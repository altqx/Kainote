//  Copyright (c) 2026, Marcin Drob
//
//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once

#ifndef _WIN32

#include <wx/window.h>

#include "RendererVideo.h"

// Small runtime-loaded VAAPI/X11 presenter for the wxGTK video path.
//
// The Linux port must keep building on machines that do not have libva development
// headers/packages installed, so this class deliberately avoids including <va/va.h>
// and resolves libva/libva-x11/GDK entry points with dlopen().  When VAAPI/X11 is
// unavailable (Wayland session, missing driver, no render-capable device, etc.) every
// method returns false and callers keep using the existing wxImage software path.
class LinuxVaapiRenderer
{
public:
	LinuxVaapiRenderer();
	~LinuxVaapiRenderer();

	LinuxVaapiRenderer(const LinuxVaapiRenderer&) = delete;
	LinuxVaapiRenderer& operator=(const LinuxVaapiRenderer&) = delete;

	bool RenderBgra(wxWindow* window, const unsigned char* frame, int width, int height,
		int pitch, const RECT& targetRect);
	void Reset();
	bool IsAvailable() const { return m_available; }
	const wxString& LastError() const { return m_lastError; }

private:
	bool EnsureDisplay(wxWindow* window);
	bool EnsureSurface(int width, int height);
	bool UploadBgra(const unsigned char* frame, int width, int height, int pitch);
	void CloseSurface();
	void CloseDisplay();
	void SetError(const wxString& error);

	void* m_libVa = nullptr;
	void* m_libVaX11 = nullptr;
	void* m_libGtk = nullptr;
	void* m_libGdk = nullptr;
	void* m_xDisplay = nullptr;
	void* m_vaDisplay = nullptr;
	unsigned long m_xid = 0;
	unsigned int m_surface = 0xffffffffu;
	unsigned int m_image = 0xffffffffu;
	unsigned int m_imageBuffer = 0xffffffffu;
	unsigned int m_imagePitch = 0;
	unsigned int m_imageOffset = 0;
	int m_width = 0;
	int m_height = 0;
	int m_major = 0;
	int m_minor = 0;
	bool m_available = false;
	bool m_disabled = false;
	bool m_loggedUnavailable = false;
	wxString m_lastError;
};

#endif
