//  Copyright (c) 2017 - 2026, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.


#include "UtilsWindows.h"
#include "LogHandler.h"
#include <wx/msw/private.h>
#include <wx/mstream.h>
#include <wx/dc.h>
#include <vector>





int CALLBACK MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::pair<std::vector<RECT>, bool> *pair = (std::pair<std::vector<RECT>, bool> *)dwData;
	WinStruct<MONITORINFO> monitorinfo;

	if (!GetMonitorInfo(hMonitor, &monitorinfo)) {
		KaiLog(_("Nie można pobrać informacji o monitorze"));
		return TRUE;
	}
	//podstawowy monitor ma być pierwszy w tablicy
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY) {
		pair->first.insert(pair->first.begin(), (pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
		return TRUE;
	}
	pair->first.push_back((pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
	return TRUE;
}

wxRect GetMonitorWorkArea(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea) {
	std::vector<RECT> MonRects;
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, workArea);
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonitorRects)
		*MonitorRects = MonRects;
	if (MonRects.size() == 0) {
		return { 0, 0, 1920, 1040 };
	}
	wxRect rt(MonRects[0].left, MonRects[0].top,
		abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1) {
		return rt;
	}
	else if (wmonitor == 0) {
		int x = position.x;
		int y = position.y;
		for (size_t i = 0; i < MonRects.size(); i++) {
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom) {
				return wxRect(MonRects[i].left, MonRects[i].top,
					abs(MonRects[i].right - MonRects[i].left), abs(MonRects[i].bottom - MonRects[i].top));
			}
		}
	}
	else if (wmonitor < MonRects.size()) {
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top,
			abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

wxRect GetMonitorRect1(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxRect &programRect) {
	std::vector<RECT> MonRects;
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, false);
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonitorRects)
		*MonitorRects = MonRects;
	if (MonRects.size() == 0) {
		return { 0, 0, 1920, 1080 };
	}
	wxRect rt(MonRects[0].left, MonRects[0].top, abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1) { return rt; }
	else if (wmonitor == 0) {
		int x = (programRect.width / 2) + programRect.x;
		int y = (programRect.height / 2) + programRect.y;
		for (size_t i = 0; i < MonRects.size(); i++) {
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom) {
				return wxRect(MonRects[i].left, MonRects[i].top, abs(MonRects[i].right - MonRects[i].left), abs(MonRects[i].bottom - MonRects[i].top));
			}
		}
	}
	else {
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top, abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

int FindMonitor(std::vector<tagRECT> *MonitorRects, const wxPoint &pos) {
	//skip monitor 0 it's primary we already on it
	for (size_t i = 1; i < MonitorRects->size(); i++) {
		if ((*MonitorRects)[i].left <= pos.x && pos.x <= (*MonitorRects)[i].right &&
			(*MonitorRects)[i].top <= pos.y && pos.y <= (*MonitorRects)[i].bottom) {
			return i;
		}
	}
	return -1;
}

bool IsMonitorRect(std::vector<tagRECT>* MonitorRects, const wxRect& rect)
{
	for (size_t i = 0; i < MonitorRects->size(); i++) {
		if ((*MonitorRects)[i].left == rect.x && rect.width == ((*MonitorRects)[i].right - (*MonitorRects)[i].left) &&
			(*MonitorRects)[i].top == rect.y && rect.height == ((*MonitorRects)[i].bottom - (*MonitorRects)[i].top)) {
			return true;
		}
	}
	return false;
}

bool GetMonitorWithSize(std::vector<tagRECT>* MonitorRects, wxRect* monrect)
{
	for (size_t i = 0; i < MonitorRects->size(); i++) {
		if (monrect->width == ((*MonitorRects)[i].right - (*MonitorRects)[i].left) &&
			monrect->height == ((*MonitorRects)[i].bottom - (*MonitorRects)[i].top)) {
			monrect->x = (*MonitorRects)[i].left;
			monrect->y = (*MonitorRects)[i].top;
			return true;
		}
	}
	return false;
}

bool GetLineTextExtents(const wxString& text, Styles* style, float* width, float* height, float* descent, float* extlead)
{
	float fwidth = 0, fheight = 0, fdescent = 0, fextlead = 0;
	float fontsize = style->GetFontSizeDouble() * 64.f;
	float spacing = wxAtof(style->Spacing) * 64.f;


	size_t thetextlen = text.length();
	if (!thetextlen) {
		*width = 0;
		*height = 0;
		if (descent)
			*descent = 0;
		if (extlead)
			*extlead = 0;
		return true;
	}

	const wchar_t* thetext = text.wc_str();


	SIZE sz;
	HDC thedc = CreateCompatibleDC(0);
	if (!thedc) return false;
	SetMapMode(thedc, MM_TEXT);

	LOGFONTW lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = (LONG)fontsize;
	lf.lfWeight = style->Bold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = style->Italic;
	lf.lfUnderline = style->Underline;
	lf.lfStrikeOut = style->StrikeOut;
	lf.lfCharSet = wxAtoi(style->Encoding);
	lf.lfOutPrecision = OUT_TT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcsncpy(lf.lfFaceName, style->Fontname.wc_str(), 32);

	HFONT thefont = CreateFontIndirect(&lf);
	if (!thefont) return false;
	SelectObject(thedc, thefont);

	if (spacing != 0) {
		fwidth = 0;
		for (size_t i = 0; i < thetextlen; i++) {
			GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
			fwidth += sz.cx + spacing;
			fheight = sz.cy;
		}
	}
	else {
		GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
		fwidth = sz.cx;
		fheight = sz.cy;
	}


	TEXTMETRIC tm;
	GetTextMetrics(thedc, &tm);
	fdescent = tm.tmDescent;
	fextlead = tm.tmExternalLeading;

	DeleteObject(thedc);
	DeleteObject(thefont);
	float scalex = wxAtof(style->ScaleX) / 100.f;
	float scaley = wxAtof(style->ScaleY) / 100.f;

	*width = scalex * (fwidth / 64.f);
	*height = scaley * (fheight / 64.f);
	if (descent)
		*descent = scaley * (fdescent / 64.f);
	if (extlead)
		*extlead = scaley * (fextlead / 64.f);

	return true;
}


#ifdef _M_IX86

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadID;
	DWORD dwFlags;
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#else
typedef struct tagTHREADNAME_INFO
{
	size_t dwType;
	LPCSTR szName;
	size_t dwThreadID;
	size_t dwFlags;
} THREADNAME_INFO;

void SetThreadName(size_t dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(size_t), (size_t*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#endif