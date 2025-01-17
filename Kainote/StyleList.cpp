//  Copyright (c) 2016 - 2020, Marcin Drob

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


#include "StyleList.h"
#include "FontEnumerator.h"
#include "config.h"
#include <wx/msw/winundef.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <algorithm>


wxDEFINE_EVENT(SELECTION_CHANGED, wxCommandEvent);

bool sortf(int i, int j){ return (i < j); }

StyleList::StyleList(wxWindow *parent, long id, std::vector<Styles*> *stylearray, const wxPoint &pos, const wxSize &size, long style)
	:KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
{

	stylenames = stylearray;

	font = *Options.GetFont();
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_UP, 15555);
	entries[1].Set(wxACCEL_NORMAL, WXK_DOWN, 15556);
	entries[2].Set(wxACCEL_SHIFT, WXK_UP, 15557);
	entries[3].Set(wxACCEL_SHIFT, WXK_DOWN, 15558);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);

	
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
	Height = fh;

	bmp = nullptr;
	scPos = 0;
	lastsel = 0;
	lastRow = -1;
	holding = Switchlines = false;
	sels.Add(0);
	SetMinSize(wxSize(150, 150));
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent& evt) {});
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& evt) { Refresh(false); });
	Bind(wxEVT_KILL_FOCUS, [=](wxFocusEvent& evt) { Refresh(false); });
}

StyleList::~StyleList(){
	if (bmp){ delete bmp; bmp = nullptr; }
}



void StyleList::OnPaint(wxPaintEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	int panelrows = (h / Height) + 1;
	int scrows;
	if ((scPos + panelrows) >= (int)stylenames->size() + 1){
		scrows = stylenames->size();
		scPos = (scrows - panelrows) + 1;
		if (panelrows > (int)stylenames->size() + 1){ scPos = 0; }
	}
	else{
		scrows = (scPos + panelrows);
	}
	if (SetScrollBar(wxVERTICAL, scPos, panelrows, stylenames->size() + 1, panelrows - 2)){
		GetClientSize(&w, &h);
	}

	if (bmp) {
		if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
			delete bmp;
			bmp = nullptr;
		}
	}
	if (!bmp) bmp = new wxBitmap(w, h);

	// Draw bitmap
	wxMemoryDC bdc;
	bdc.SelectObject(*bmp);

	int fw = 0, fh = 0, posX = 1, posY = 1;
	bdc.Clear();
	bdc.SetFont(font);
	const wxColour & background = Options.GetColour(STATICLIST_BACKGROUND);
	bdc.SetPen(wxPen(HasFocus()? Options.GetColour(TEXT_FIELD_BORDER_ON_FOCUS) : 
		Options.GetColour(STATICLIST_BORDER)));
	bdc.SetBrush(wxBrush(background));
	bdc.DrawRectangle(0, 0, w, h);
	wxArrayString *fonts = FontEnum.GetFonts(nullptr, [](){});

	for (int i = scPos; i < scrows; i++)
	{
		if (sels.Index(i) != -1){
			bdc.SetPen(*wxTRANSPARENT_PEN);
			bdc.SetBrush(wxBrush(Options.GetColour(STATICLIST_SELECTION)));
			bdc.DrawRectangle(posX, posY, w - 2, Height);
		}
		else{
			bdc.SetBrush(wxBrush(background));
		}
		if (fonts->Index(stylenames->at(i)->Fontname, false) == -1){
			bdc.SetTextForeground(Options.GetColour(WINDOW_WARNING_ELEMENTS));
		}
		else{
			bdc.SetTextForeground(Options.GetColour(WINDOW_TEXT));
		}


		wxRect cur(posX + 4, posY, w - 8, Height);
		bdc.SetClippingRegion(cur);
		bdc.DrawLabel(stylenames->at(i)->Name, cur, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		bdc.DestroyClippingRegion();

		posY += Height;
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &bdc, 0, 0);
}

void StyleList::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void StyleList::OnScroll(wxScrollWinEvent& event)
{
	int newPos = 0;
	if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
	{
		newPos = scPos - 1;
		if (newPos < 0){ newPos = 0; return; }
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos = scPos + 1;
		if (newPos >= (int)stylenames->size()){ newPos = (int)stylenames->size() - 1; return; }
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size = GetClientSize();
		newPos = scPos;
		newPos -= (size.y / Height - 1);
		newPos = MAX(0, newPos);
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size = GetClientSize();
		newPos = scPos;
		newPos += (size.y / Height - 1);
		newPos = MIN(newPos, (int)stylenames->size() - 1);
	}
	else{ newPos = event.GetPosition(); }
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void StyleList::OnMouseEvent(wxMouseEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	bool shift = event.ShiftDown();
	bool alt = event.AltDown();
	bool ctrl = event.CmdDown();
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY = event.GetY();
	int row = (curY / Height) + scPos;

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}

	if (left_up && holding) {
		if (Switchlines){ wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId()); AddPendingEvent(evt); }
		holding = false;
		ReleaseMouse();
	}

	if (row < scPos || row >= (int)stylenames->size()) {
		return;
	}

	if (event.GetWheelRotation() != 0) {
		int step = scPos - ((event.GetWheelRotation() / event.GetWheelDelta()));
		if (step < 0){ step = 0; }
		if (step > (int)stylenames->size() - 1){ step = stylenames->size() - 1; }
		if (scPos != step){
			scPos = step;
			Refresh(false);
		}
		return;
	}



	if (click) {
		holding = true;
		if (!shift) { lastRow = row; }
		lastsel = -1;
		//wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		//AddPendingEvent(evt);
		Refresh(false);
		CaptureMouse();
	}

	if (dclick){
		wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, GetId());
		AddPendingEvent(evt);
	}

	if (holding && alt && !shift && lastsel != row)
	{
		if (lastsel != -1) {

			int diff = (row - lastsel);
			if (diff > 0){
				for (int i = sels.size() - 1; i >= 0; i--){
					if ((sels[i] + diff) >= (int)stylenames->size()){ break; }
					Styles *tmp = stylenames->at(sels[i]);
					stylenames->erase(stylenames->begin() + sels[i]);
					stylenames->insert(stylenames->begin() + sels[i] + diff, tmp);
					sels[i] += diff;
				}
			}
			else{
				for (int i = 0; i < (int)sels.size(); i++){
					if ((sels[i] + diff) < 0){ break; }
					
					Styles *tmp = stylenames->at(sels[i]);
					stylenames->erase(stylenames->begin() + sels[i]);
					stylenames->insert(stylenames->begin() + sels[i] + diff, tmp);
					sels[i] += diff;
				}

			}
			Refresh(false);
			Switchlines = true;
		}
		lastsel = row;
		//return;
	}

	if ((click && shift && !alt) || (holding && !ctrl && !alt && !shift && lastsel != row)) {
		if (lastRow != -1) {
			// Keyboard selection continues from where the mouse was last used
			//extendRow = lastRow;

			// Set boundaries
			int i1 = row;
			int i2 = lastRow;
			if (i1 > i2) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Toggle each
			bool notFirst = false;
			for (int i = i1; i <= i2; i++) {
				if (!notFirst){ sels.clear(); }
				sels.Add(i);
				notFirst = true;
			}
			lastsel = row;
			Refresh(false);
			SendSelectionEvent();
		}
		return;
	}

	// Toggle selected
	if (click && ctrl && !shift && !alt) {
		int idx = sels.Index(row);
		if (idx != -1){
			sels.RemoveAt(idx);
		}
		else{
			sels.Add(row);
		}
		std::sort(sels.begin(), sels.end(), sortf);
		Refresh(false);
		SendSelectionEvent();
		return;
	}

	if (!shift && !ctrl && !alt) {
		if (click && !dclick) {
			sels.clear();

			sels.Add(row);
			Refresh(false);
			SendSelectionEvent();
			return;
		}
	}

	if (holding) {
		// Find direction
		int minVis = scPos + 1;
		int maxVis = scPos + (h / Height) - 2;
		int delta = 0;
		if (row < minVis && row != 0) delta = -1;
		if (row > maxVis) delta = 1;

		if (delta) {
			scPos = (MID(row - (h / Height), scPos + delta, row));
			Refresh(false);
		}
	}


}

void StyleList::SetArray(std::vector<Styles*> *stylearray)
{
	stylenames = stylearray;
	Refresh(false);
}

void StyleList::SendSelectionEvent()
{
	wxCommandEvent evt(SELECTION_CHANGED, GetId());
	evt.SetInt(sels.size());
	ProcessEvent(evt);
}

//This method adding to selection or reset
//and select new elements and refresh list
void StyleList::SetSelection(int _sel, bool reset)
{

	if (_sel == -1 || reset){ sels.clear(); }
	if (_sel != -1){ sels.Add(_sel); }
	if (sels.size() > 0) {
		scPos = MAX(0, sels[0] - 2);
		lastRow = sels[0];
	}
	else
		lastRow = 0;

	Refresh(false);
	SendSelectionEvent();
}

void StyleList::SetSelections(const wxArrayInt &_sels)
{
	sels.clear();
	for (size_t i = 0; i < _sels.size(); i++){
		sels.push_back(_sels[i]);
	}
	if (sels.size() > 0) {
		scPos = MAX(0, sels[0] - 2);
		lastRow = sels[0];
	}
	else
		lastRow = 0;
	
	Refresh(false);
	SendSelectionEvent();
}


int StyleList::GetSelections(wxArrayInt &_sels)
{
	if (stylenames->size() < 1){
		sels.Clear();
		SendSelectionEvent();
	}
	std::sort(sels.begin(), sels.end(), sortf);
	_sels = sels;
	return sels.size();
}

void StyleList::Scroll(int step)
{
	scPos += step;
	if (scPos < 0 || scPos >(int)stylenames->size()){ return; }
	Refresh(false);
}

void StyleList::OnArrow(wxCommandEvent& event)
{
	int id = event.GetId();
	if (id == 15555 || id == 15557)//up -> shift up
	{
		if (lastsel == 0){ return; }
		if (id == 15557 && sels[0] != lastsel){
			lastsel = sels[0];
		}
		lastsel--;
		if (lastsel < 0){ lastsel = (sels.size() > 0) ? sels[0] : 0; }
		if (id == 15555){ sels.clear(); }
		if (scPos > lastsel){ scPos--; }
		sels.Add(lastsel);
	}
	else if (id == 15556 || id == 15558){//down -> shift down
		if (id == 15558 && sels[sels.size() - 1] != lastsel){
			lastsel = sels[sels.size() - 1];
		}
		lastsel++;
		if (lastsel >= (int)stylenames->size()){ lastsel = stylenames->size() - 1; return; }

		if (id == 15556){ sels.clear(); }
		wxSize size = GetClientSize();
		if ((scPos + (size.y / Height)) <= lastsel){ scPos++; }
		sels.Add(lastsel);
	}
	std::sort(sels.begin(), sels.end(), sortf);
	Refresh(false);
}

bool StyleList::SetFont(const wxFont &_font)
{
	font = _font;
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
	Height = fh;
	Refresh(false);
	return true;
}

BEGIN_EVENT_TABLE(StyleList, KaiScrolledWindow)
EVT_PAINT(StyleList::OnPaint)
EVT_SIZE(StyleList::OnSize)
EVT_SCROLLWIN(StyleList::OnScroll)
EVT_MOUSE_EVENTS(StyleList::OnMouseEvent)
EVT_MENU_RANGE(15555, 15558, StyleList::OnArrow)
EVT_MOUSE_CAPTURE_LOST(StyleList::OnLostCapture)
END_EVENT_TABLE()
