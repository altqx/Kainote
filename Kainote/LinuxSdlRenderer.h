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

// SDL2 fallback presenter for Linux/wxGTK video frames.
//
// Order on Linux is now:
//   VAAPI/X11 presenter -> SDL2 renderer -> wxImage/wxBitmap software blit.
// SDL keeps the decoded/subtitle-composited BGRA frame path intact, but uses SDL's
// accelerated renderer/texture upload when VAAPI is unavailable.
class LinuxSdlRenderer
{
public:
	LinuxSdlRenderer();
	~LinuxSdlRenderer();

	LinuxSdlRenderer(const LinuxSdlRenderer&) = delete;
	LinuxSdlRenderer& operator=(const LinuxSdlRenderer&) = delete;

	bool RenderBgra(wxWindow* window, const unsigned char* frame, int width, int height,
		int pitch, const RECT& targetRect);
	void Reset();
	bool IsAvailable() const { return m_available; }
	const wxString& LastError() const { return m_lastError; }

private:
	bool EnsureWindow(wxWindow* window);
	bool EnsureTexture(int width, int height);
	unsigned long GetXid(wxWindow* window);
	void CloseTexture();
	void CloseWindow();
	void SetError(const wxString& error, bool disable = true);

	void* m_libGtk = nullptr;
	void* m_libGdk = nullptr;
	void* m_sdlWindow = nullptr;
	void* m_sdlRenderer = nullptr;
	void* m_sdlTexture = nullptr;
	unsigned long m_xid = 0;
	int m_width = 0;
	int m_height = 0;
	bool m_sdlInitialized = false;
	bool m_available = false;
	bool m_disabled = false;
	bool m_loggedUnavailable = false;
	wxString m_lastError;
};

#endif
