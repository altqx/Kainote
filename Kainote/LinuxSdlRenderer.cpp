//  Copyright (c) 2026, Marcin Drob
//
//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#ifndef _WIN32

#include "LinuxSdlRenderer.h"
#include "LogHandler.h"

#include <SDL.h>
#include <algorithm>
#include <dlfcn.h>

namespace
{
	using GtkWidget = ::GtkWidget;
	using GdkWindow = ::GdkWindow;

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

LinuxSdlRenderer::LinuxSdlRenderer() = default;

LinuxSdlRenderer::~LinuxSdlRenderer()
{
	Reset();
}

void LinuxSdlRenderer::SetError(const wxString& error, bool disable)
{
	m_lastError = error;
	m_available = false;
	if (disable)
		m_disabled = true;
	if (!m_loggedUnavailable) {
		KaiLogSilent(L"SDL video renderer disabled: " + error);
		m_loggedUnavailable = true;
	}
}

unsigned long LinuxSdlRenderer::GetXid(wxWindow* window)
{
	if (!window)
		return 0;

	GtkWidget* widget = reinterpret_cast<GtkWidget*>(window->GetHandle());
	if (!widget)
		return 0;

	if (!m_libGtk)
		m_libGtk = OpenAny("libgtk-3.so.0", "libgtk-3.so");
	if (!m_libGdk)
		m_libGdk = OpenAny("libgdk-3.so.0", "libgdk-3.so");
	if (!m_libGtk || !m_libGdk) {
		SetError(wxString::FromUTF8(SafeDlError()));
		return 0;
	}

	using gtk_widget_get_window_t = GdkWindow* (*)(GtkWidget*);
	using gdk_x11_window_get_xid_t = unsigned long (*)(GdkWindow*);
	auto gtk_widget_get_window = LoadSymbol<gtk_widget_get_window_t>(m_libGtk, "gtk_widget_get_window");
	auto gdk_x11_window_get_xid = LoadSymbol<gdk_x11_window_get_xid_t>(m_libGdk, "gdk_x11_window_get_xid");
	if (!gtk_widget_get_window || !gdk_x11_window_get_xid) {
		SetError(wxString::Format(L"missing GTK/GDK X11 symbol for SDL fallback: %s", wxString::FromUTF8(SafeDlError())));
		return 0;
	}

	GdkWindow* gdkWindow = gtk_widget_get_window(widget);
	if (!gdkWindow)
		return 0;
	return gdk_x11_window_get_xid(gdkWindow);
}

bool LinuxSdlRenderer::EnsureWindow(wxWindow* window)
{
	if (!window || m_disabled)
		return false;

	unsigned long xid = GetXid(window);
	if (!xid)
		return false;

	if (m_sdlRenderer && m_sdlWindow && xid == m_xid) {
		m_available = true;
		return true;
	}

	CloseWindow();

	if (!m_sdlInitialized) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			SetError(wxString::FromUTF8(SDL_GetError()));
			return false;
		}
		m_sdlInitialized = true;
	}

	m_sdlWindow = SDL_CreateWindowFrom(reinterpret_cast<void*>(xid));
	if (!m_sdlWindow) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}

	m_sdlRenderer = SDL_CreateRenderer(static_cast<SDL_Window*>(m_sdlWindow), -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!m_sdlRenderer) {
		// Some Xvfb/VM environments expose only a software SDL renderer.  This is still
		// a useful fallback before wxImage because SDL owns texture upload/scaling.
		m_sdlRenderer = SDL_CreateRenderer(static_cast<SDL_Window*>(m_sdlWindow), -1, SDL_RENDERER_SOFTWARE);
	}
	if (!m_sdlRenderer) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		CloseWindow();
		return false;
	}

	m_xid = xid;
	m_available = true;
	return true;
}

bool LinuxSdlRenderer::EnsureTexture(int width, int height)
{
	if (!m_sdlRenderer || width <= 0 || height <= 0)
		return false;
	if (m_sdlTexture && width == m_width && height == m_height)
		return true;

	CloseTexture();
	m_sdlTexture = SDL_CreateTexture(static_cast<SDL_Renderer*>(m_sdlRenderer),
		SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!m_sdlTexture) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}
	SDL_SetTextureBlendMode(static_cast<SDL_Texture*>(m_sdlTexture), SDL_BLENDMODE_NONE);
	m_width = width;
	m_height = height;
	return true;
}

bool LinuxSdlRenderer::RenderBgra(wxWindow* window, const unsigned char* frame, int width, int height,
	int pitch, const RECT& targetRect)
{
	if (!frame || width <= 0 || height <= 0 || pitch < width * 4)
		return false;
	if (!EnsureWindow(window) || !EnsureTexture(width, height))
		return false;

	SDL_Texture* texture = static_cast<SDL_Texture*>(m_sdlTexture);
	SDL_Renderer* renderer = static_cast<SDL_Renderer*>(m_sdlRenderer);
	if (SDL_UpdateTexture(texture, nullptr, frame, pitch) != 0) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}

	SDL_Rect dst{};
	dst.x = static_cast<int>(targetRect.left);
	dst.y = static_cast<int>(targetRect.top);
	dst.w = std::max(0L, targetRect.right - targetRect.left);
	dst.h = std::max(0L, targetRect.bottom - targetRect.top);
	if (dst.w <= 0 || dst.h <= 0)
		return false;

	SDL_Rect clip = dst;
	SDL_RenderSetClipRect(renderer, &clip);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &dst);
	if (SDL_RenderCopy(renderer, texture, nullptr, &dst) != 0) {
		SDL_RenderSetClipRect(renderer, nullptr);
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}
	SDL_RenderSetClipRect(renderer, nullptr);
	SDL_RenderPresent(renderer);
	m_available = true;
	return true;
}

void LinuxSdlRenderer::CloseTexture()
{
	if (m_sdlTexture) {
		SDL_DestroyTexture(static_cast<SDL_Texture*>(m_sdlTexture));
		m_sdlTexture = nullptr;
	}
	m_width = 0;
	m_height = 0;
}

void LinuxSdlRenderer::CloseWindow()
{
	CloseTexture();
	if (m_sdlRenderer) {
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(m_sdlRenderer));
		m_sdlRenderer = nullptr;
	}
	if (m_sdlWindow) {
		SDL_DestroyWindow(static_cast<SDL_Window*>(m_sdlWindow));
		m_sdlWindow = nullptr;
	}
	m_xid = 0;
	m_available = false;
}

void LinuxSdlRenderer::Reset()
{
	CloseWindow();
	if (m_sdlInitialized) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		m_sdlInitialized = false;
	}
	if (m_libGdk) { dlclose(m_libGdk); m_libGdk = nullptr; }
	if (m_libGtk) { dlclose(m_libGtk); m_libGtk = nullptr; }
	m_disabled = false;
}

#endif
