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
#include <cstdlib>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/image.h>

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

bool LinuxSdlRenderer::EnsureSdl()
{
	if (m_disabled)
		return false;
	if (m_sdlInitialized)
		return true;
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}
	m_sdlInitialized = true;
	return true;
}

bool LinuxSdlRenderer::RenderBgra(wxWindow* window, const unsigned char* frame, int width, int height,
	int pitch, const RECT& targetRect)
{
	if (std::getenv("KAINOTE_DISABLE_SDL_RENDER"))
		return false;
	if (!window || !frame || width <= 0 || height <= 0 || pitch < width * 4)
		return false;
	if (!EnsureSdl())
		return false;

	const int targetWidth = static_cast<int>(targetRect.right - targetRect.left);
	const int targetHeight = static_cast<int>(targetRect.bottom - targetRect.top);
	if (targetWidth <= 0 || targetHeight <= 0)
		return false;

	const size_t sourceSize = static_cast<size_t>(width) * height * 3;
	if (m_sourceRgb.size() != sourceSize || m_sourceWidth != width || m_sourceHeight != height) {
		m_sourceRgb.resize(sourceSize);
		m_sourceWidth = width;
		m_sourceHeight = height;
	}
	for (int y = 0; y < height; ++y) {
		const unsigned char* src = frame + (y * pitch);
		unsigned char* dst = m_sourceRgb.data() + (static_cast<size_t>(y) * width * 3);
		for (int x = 0; x < width; ++x) {
			// FFMS2 output is BGRA. SDL surface scaling works reliably with an
			// opaque RGB24 source surface and avoids X11 child-window stacking issues.
			dst[(x * 3) + 0] = src[(x * 4) + 2];
			dst[(x * 3) + 1] = src[(x * 4) + 1];
			dst[(x * 3) + 2] = src[(x * 4) + 0];
		}
	}

	const size_t scaledSize = static_cast<size_t>(targetWidth) * targetHeight * 3;
	if (m_scaledRgb.size() != scaledSize || m_scaledWidth != targetWidth || m_scaledHeight != targetHeight) {
		m_scaledRgb.resize(scaledSize);
		m_scaledWidth = targetWidth;
		m_scaledHeight = targetHeight;
	}

	SDL_Surface* source = SDL_CreateRGBSurfaceWithFormatFrom(m_sourceRgb.data(), width, height, 24,
		width * 3, SDL_PIXELFORMAT_RGB24);
	if (!source) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}
	SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormatFrom(m_scaledRgb.data(), targetWidth, targetHeight, 24,
		targetWidth * 3, SDL_PIXELFORMAT_RGB24);
	if (!scaled) {
		SDL_FreeSurface(source);
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}

	SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);
	SDL_SetSurfaceBlendMode(scaled, SDL_BLENDMODE_NONE);
	SDL_FillRect(scaled, nullptr, SDL_MapRGB(scaled->format, 0, 0, 0));
	const int blitResult = SDL_BlitScaled(source, nullptr, scaled, nullptr);
	SDL_FreeSurface(scaled);
	SDL_FreeSurface(source);
	if (blitResult != 0) {
		SetError(wxString::FromUTF8(SDL_GetError()));
		return false;
	}

	wxImage image(targetWidth, targetHeight, m_scaledRgb.data(), true);
	if (!image.IsOk())
		return false;
	wxBitmap bitmap(image);
	if (!bitmap.IsOk())
		return false;
	wxClientDC dc(window);
	dc.DrawBitmap(bitmap, targetRect.left, targetRect.top, false);
	m_available = true;
	return true;
}

void LinuxSdlRenderer::Reset()
{
	m_sourceRgb.clear();
	m_scaledRgb.clear();
	m_sourceWidth = 0;
	m_sourceHeight = 0;
	m_scaledWidth = 0;
	m_scaledHeight = 0;
	m_available = false;
	m_disabled = false;
	if (m_sdlInitialized) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		m_sdlInitialized = false;
	}
}

#endif
