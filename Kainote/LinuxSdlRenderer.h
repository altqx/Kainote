//  Copyright (c) 2026, Marcin Drob
//
//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#pragma once
#ifndef _WIN32

#include <vector>
#include <wx/window.h>
#include "RendererVideo.h"

// SDL2 fallback presenter for Linux/wxGTK video frames.
//
// Opt-in Linux order:
//   KAINOTE_ENABLE_SDL_RENDER=1 enables SDL2 software surface scaling before
//   the normal wxImage/wxBitmap fallback. If KAINOTE_ENABLE_VAAPI_RENDER is set,
//   VAAPI/X11 is tried first. SDL handles BGRA->RGB conversion/scaling, while
//   wxGTK does the final blit so GTK cannot repaint over an embedded SDL/X11
//   child window.
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
	bool EnsureSdl();
	void SetError(const wxString& error, bool disable = true);

	std::vector<unsigned char> m_sourceRgb;
	std::vector<unsigned char> m_scaledRgb;
	int m_sourceWidth = 0;
	int m_sourceHeight = 0;
	int m_scaledWidth = 0;
	int m_scaledHeight = 0;
	bool m_sdlInitialized = false;
	bool m_available = false;
	bool m_disabled = false;
	bool m_loggedUnavailable = false;
	wxString m_lastError;
};

#endif
