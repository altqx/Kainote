//  Copyright (c) 2026, Marcin Drob
//
//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#ifndef _WIN32

#include "LinuxVaapiRenderer.h"
#include "LogHandler.h"
#include "RendererVideo.h"

#include <algorithm>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

namespace
{
	using VADisplay = void*;
	using VABufferID = unsigned int;
	using VAImageID = unsigned int;
	using VASurfaceID = unsigned int;
	using VAStatus = int;
	using Drawable = unsigned long;
	using Display = void;

	constexpr VAStatus VA_STATUS_SUCCESS = 0;
	constexpr unsigned int VA_INVALID_ID = 0xffffffffu;
	constexpr unsigned int VA_FOURCC_BGRA = 0x41524742u;
	constexpr unsigned int VA_RT_FORMAT_RGB32 = 0x00000100u;
	constexpr unsigned int VA_PROGRESSIVE = 0x1;
	constexpr int VA_LSB_FIRST = 1;

	struct VAImageFormat
	{
		unsigned int fourcc = 0;
		int byte_order = 0;
		unsigned int bits_per_pixel = 0;
		unsigned int depth = 0;
		unsigned int red_mask = 0;
		unsigned int green_mask = 0;
		unsigned int blue_mask = 0;
		unsigned int alpha_mask = 0;
		unsigned int va_reserved[4]{};
	};

	struct VAImage
	{
		VAImageID image_id = VA_INVALID_ID;
		VAImageFormat format{};
		VABufferID buf = VA_INVALID_ID;
		unsigned short width = 0;
		unsigned short height = 0;
		unsigned int data_size = 0;
		unsigned int num_planes = 0;
		unsigned int pitches[3]{};
		unsigned int offsets[3]{};
		unsigned int num_palette_entries = 0;
		unsigned int entry_bytes = 0;
		unsigned char component_order[4]{};
		unsigned int va_reserved[4]{};
	};

	template<typename T>
	T LoadSymbol(void* lib, const char* name)
	{
		return reinterpret_cast<T>(dlsym(lib, name));
	}

	void* OpenAny(const char* first, const char* second = nullptr)
	{
		void* handle = dlopen(first, RTLD_NOW | RTLD_LOCAL);
		if (!handle && second)
			handle = dlopen(second, RTLD_NOW | RTLD_LOCAL);
		return handle;
	}

	const char* SafeDlError()
	{
		const char* err = dlerror();
		return err ? err : "unknown dlopen/dlsym error";
	}
}

LinuxVaapiRenderer::LinuxVaapiRenderer() = default;

LinuxVaapiRenderer::~LinuxVaapiRenderer()
{
	Reset();
}

void LinuxVaapiRenderer::SetError(const wxString& error)
{
	m_lastError = error;
	m_available = false;
	m_disabled = true;
	if (!m_loggedUnavailable) {
		KaiLogSilent(L"VAAPI renderer disabled: " + error);
		m_loggedUnavailable = true;
	}
}

bool LinuxVaapiRenderer::EnsureDisplay(wxWindow* window)
{
	if (!window || m_disabled)
		return false;

	GtkWidget* widget = reinterpret_cast<GtkWidget*>(window->GetHandle());
	if (!widget)
		return false;

	if (!m_libGtk)
		m_libGtk = OpenAny("libgtk-3.so.0", "libgtk-3.so");
	if (!m_libGdk)
		m_libGdk = OpenAny("libgdk-3.so.0", "libgdk-3.so");
	if (!m_libVa)
		m_libVa = OpenAny("libva.so.2", "libva.so");
	if (!m_libVaX11)
		m_libVaX11 = OpenAny("libva-x11.so.2", "libva-x11.so");

	if (!m_libGtk || !m_libGdk || !m_libVa || !m_libVaX11) {
		SetError(wxString::FromUTF8(SafeDlError()));
		return false;
	}

	using gtk_widget_get_window_t = GdkWindow* (*)(GtkWidget*);
	using gdk_window_get_display_t = void* (*)(GdkWindow*);
	using gdk_x11_display_get_xdisplay_t = Display* (*)(void*);
	using gdk_x11_window_get_xid_t = unsigned long (*)(GdkWindow*);
	using vaGetDisplay_t = VADisplay (*)(Display*);
	using vaInitialize_t = VAStatus (*)(VADisplay, int*, int*);

	auto gtk_widget_get_window = LoadSymbol<gtk_widget_get_window_t>(m_libGtk, "gtk_widget_get_window");
	auto gdk_window_get_display = LoadSymbol<gdk_window_get_display_t>(m_libGdk, "gdk_window_get_display");
	auto gdk_x11_display_get_xdisplay = LoadSymbol<gdk_x11_display_get_xdisplay_t>(m_libGdk, "gdk_x11_display_get_xdisplay");
	auto gdk_x11_window_get_xid = LoadSymbol<gdk_x11_window_get_xid_t>(m_libGdk, "gdk_x11_window_get_xid");
	auto vaGetDisplay = LoadSymbol<vaGetDisplay_t>(m_libVaX11, "vaGetDisplay");
	auto vaInitialize = LoadSymbol<vaInitialize_t>(m_libVa, "vaInitialize");

	if (!gtk_widget_get_window || !gdk_window_get_display || !gdk_x11_display_get_xdisplay ||
		!gdk_x11_window_get_xid || !vaGetDisplay || !vaInitialize) {
		SetError(wxString::Format(L"missing runtime VAAPI/X11 symbol: %s", wxString::FromUTF8(SafeDlError())));
		return false;
	}

	GdkWindow* gdkWindow = gtk_widget_get_window(widget);
	if (!gdkWindow)
		return false;

	unsigned long xid = gdk_x11_window_get_xid(gdkWindow);
	Display* xDisplay = gdk_x11_display_get_xdisplay(gdk_window_get_display(gdkWindow));
	if (!xDisplay || !xid) {
		SetError(L"wxGTK is not running on an X11 window (VAAPI presentation needs libva-x11)");
		return false;
	}

	if (m_vaDisplay && m_xDisplay == xDisplay && m_xid == xid) {
		m_available = true;
		return true;
	}

	CloseDisplay();
	m_xDisplay = xDisplay;
	m_xid = xid;
	m_vaDisplay = vaGetDisplay(xDisplay);
	if (!m_vaDisplay) {
		SetError(L"vaGetDisplay returned null");
		return false;
	}
	if (vaInitialize(m_vaDisplay, &m_major, &m_minor) != VA_STATUS_SUCCESS) {
		SetError(L"vaInitialize failed");
		m_vaDisplay = nullptr;
		return false;
	}

	m_available = true;
	return true;
}

bool LinuxVaapiRenderer::EnsureSurface(int width, int height)
{
	if (!m_vaDisplay || width <= 0 || height <= 0)
		return false;
	if (m_surface != VA_INVALID_ID && m_image != VA_INVALID_ID && m_width == width && m_height == height)
		return true;

	CloseSurface();

	using vaCreateSurfaces_t = VAStatus (*)(VADisplay, unsigned int, unsigned int, unsigned int, VASurfaceID*, unsigned int, void*, unsigned int);
	using vaCreateImage_t = VAStatus (*)(VADisplay, VAImageFormat*, int, int, VAImage*);
	auto vaCreateSurfaces = LoadSymbol<vaCreateSurfaces_t>(m_libVa, "vaCreateSurfaces");
	auto vaCreateImage = LoadSymbol<vaCreateImage_t>(m_libVa, "vaCreateImage");
	if (!vaCreateSurfaces || !vaCreateImage) {
		SetError(L"missing VA surface/image creation symbols");
		return false;
	}

	VASurfaceID surface = VA_INVALID_ID;
	if (vaCreateSurfaces(m_vaDisplay, VA_RT_FORMAT_RGB32, width, height, &surface, 1, nullptr, 0) != VA_STATUS_SUCCESS) {
		SetError(L"vaCreateSurfaces(RGB32) failed");
		return false;
	}

	VAImageFormat format{};
	format.fourcc = VA_FOURCC_BGRA;
	format.byte_order = VA_LSB_FIRST;
	format.bits_per_pixel = 32;
	format.depth = 32;
	format.red_mask = 0x00ff0000;
	format.green_mask = 0x0000ff00;
	format.blue_mask = 0x000000ff;
	format.alpha_mask = 0xff000000;

	VAImage image{};
	if (vaCreateImage(m_vaDisplay, &format, width, height, &image) != VA_STATUS_SUCCESS) {
		m_surface = surface;
		CloseSurface();
		SetError(L"vaCreateImage(BGRA) failed");
		return false;
	}

	m_surface = surface;
	m_image = image.image_id;
	m_imageBuffer = image.buf;
	m_imagePitch = image.pitches[0] ? image.pitches[0] : static_cast<unsigned int>(width * 4);
	m_imageOffset = image.offsets[0];
	m_width = width;
	m_height = height;
	return true;
}

bool LinuxVaapiRenderer::UploadBgra(const unsigned char* frame, int width, int height, int pitch)
{
	if (!frame || !m_vaDisplay || m_image == VA_INVALID_ID || m_surface == VA_INVALID_ID)
		return false;

	using vaMapBuffer_t = VAStatus (*)(VADisplay, VABufferID, void**);
	using vaUnmapBuffer_t = VAStatus (*)(VADisplay, VABufferID);
	using vaPutImage_t = VAStatus (*)(VADisplay, VASurfaceID, VAImageID, int, int, unsigned int, unsigned int, int, int, unsigned int, unsigned int);
	auto vaMapBuffer = LoadSymbol<vaMapBuffer_t>(m_libVa, "vaMapBuffer");
	auto vaUnmapBuffer = LoadSymbol<vaUnmapBuffer_t>(m_libVa, "vaUnmapBuffer");
	auto vaPutImage = LoadSymbol<vaPutImage_t>(m_libVa, "vaPutImage");
	if (!vaMapBuffer || !vaUnmapBuffer || !vaPutImage) {
		SetError(L"missing VA image upload symbols");
		return false;
	}

	void* mapped = nullptr;
	if (vaMapBuffer(m_vaDisplay, m_imageBuffer, &mapped) != VA_STATUS_SUCCESS || !mapped) {
		SetError(L"vaMapBuffer(image) failed");
		return false;
	}

	const int rowBytes = width * 4;
	unsigned char* dst = static_cast<unsigned char*>(mapped) + m_imageOffset;
	for (int y = 0; y < height; ++y)
		std::memcpy(dst + (y * m_imagePitch), frame + (y * pitch), rowBytes);

	vaUnmapBuffer(m_vaDisplay, m_imageBuffer);

	if (vaPutImage(m_vaDisplay, m_surface, m_image, 0, 0, width, height, 0, 0, width, height) != VA_STATUS_SUCCESS) {
		SetError(L"vaPutImage(BGRA -> surface) failed");
		return false;
	}
	return true;
}

bool LinuxVaapiRenderer::RenderBgra(wxWindow* window, const unsigned char* frame, int width, int height,
	int pitch, const RECT& targetRect)
{
	if (!frame || width <= 0 || height <= 0 || pitch < width * 4)
		return false;
	if (!EnsureDisplay(window) || !EnsureSurface(width, height) || !UploadBgra(frame, width, height, pitch))
		return false;

	using vaPutSurface_t = VAStatus (*)(VADisplay, VASurfaceID, Drawable, short, short, unsigned short, unsigned short,
		short, short, unsigned short, unsigned short, void*, unsigned int, unsigned int);
	auto vaPutSurface = LoadSymbol<vaPutSurface_t>(m_libVaX11, "vaPutSurface");
	if (!vaPutSurface) {
		SetError(L"missing vaPutSurface symbol");
		return false;
	}

	const int dstX = targetRect.left;
	const int dstY = targetRect.top;
	const int dstW = std::max(0L, targetRect.right - targetRect.left);
	const int dstH = std::max(0L, targetRect.bottom - targetRect.top);
	if (dstW <= 0 || dstH <= 0)
		return false;

	VAStatus status = vaPutSurface(m_vaDisplay, m_surface, m_xid,
		0, 0, static_cast<unsigned short>(width), static_cast<unsigned short>(height),
		static_cast<short>(dstX), static_cast<short>(dstY), static_cast<unsigned short>(dstW), static_cast<unsigned short>(dstH),
		nullptr, 0, VA_PROGRESSIVE);
	if (status != VA_STATUS_SUCCESS) {
		SetError(L"vaPutSurface failed");
		return false;
	}
	m_available = true;
	return true;
}

void LinuxVaapiRenderer::CloseSurface()
{
	if (!m_libVa || !m_vaDisplay)
		return;
	using vaDestroyImage_t = VAStatus (*)(VADisplay, VAImageID);
	using vaDestroySurfaces_t = VAStatus (*)(VADisplay, VASurfaceID*, int);
	auto vaDestroyImage = LoadSymbol<vaDestroyImage_t>(m_libVa, "vaDestroyImage");
	auto vaDestroySurfaces = LoadSymbol<vaDestroySurfaces_t>(m_libVa, "vaDestroySurfaces");
	if (m_image != VA_INVALID_ID && vaDestroyImage)
		vaDestroyImage(m_vaDisplay, m_image);
	if (m_surface != VA_INVALID_ID && vaDestroySurfaces)
		vaDestroySurfaces(m_vaDisplay, &m_surface, 1);
	m_surface = VA_INVALID_ID;
	m_image = VA_INVALID_ID;
	m_imageBuffer = VA_INVALID_ID;
	m_imagePitch = 0;
	m_imageOffset = 0;
	m_width = 0;
	m_height = 0;
}

void LinuxVaapiRenderer::CloseDisplay()
{
	CloseSurface();
	if (m_libVa && m_vaDisplay) {
		using vaTerminate_t = VAStatus (*)(VADisplay);
		auto vaTerminate = LoadSymbol<vaTerminate_t>(m_libVa, "vaTerminate");
		if (vaTerminate)
			vaTerminate(m_vaDisplay);
	}
	m_vaDisplay = nullptr;
	m_xDisplay = nullptr;
	m_xid = 0;
}

void LinuxVaapiRenderer::Reset()
{
	CloseDisplay();
	if (m_libVaX11) { dlclose(m_libVaX11); m_libVaX11 = nullptr; }
	if (m_libVa) { dlclose(m_libVa); m_libVa = nullptr; }
	if (m_libGdk) { dlclose(m_libGdk); m_libGdk = nullptr; }
	if (m_libGtk) { dlclose(m_libGtk); m_libGtk = nullptr; }
	m_available = false;
	m_disabled = false;
}

#endif
