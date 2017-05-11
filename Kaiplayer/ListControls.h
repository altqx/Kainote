//  Copyright (c) 2016, Marcin Drob

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

#ifndef _LIST_CONTROLS_
#define _LIST_CONTROLS_

#include <map>
#include <wx/wx.h>
#include <wx/popupwin.h>
#include <wx/msw/popupwin.h>

class KaiTextCtrl;
class KaiScrollbar;

class PopupList : public wxPopupWindow{
	friend class KaiChoice;
public:

	PopupList(wxWindow *DialogParent, wxArrayString *list, std::map<int, bool> *disabled);
	~PopupList();
	void Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem);
	void CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize);
	void EndPartialModal(int ReturnId);
	void SetSelection(int pos);
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnKeyPress(wxKeyEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();}};
	int sel;
	int scPos;
	
protected:
	wxBitmap *bmp;
	wxArrayString *itemsList;
	std::map<int, bool> *disabledItems;
	wxWindow *Parent;
	int orgY;
	KaiScrollbar *scroll;
	DECLARE_EVENT_TABLE()
};

class KaiChoice :public wxWindow
{
	friend class PopupList;
public:
	KaiChoice(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
        const wxSize& size, const wxArrayString &choices,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	KaiChoice(wxWindow *parent, int id, const wxString &comboBoxText, const wxPoint& pos,
        const wxSize& size, const wxArrayString &choices,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	virtual ~KaiChoice();
	void SetSelection(int sel, bool changeText=true);
	void Clear();
	/*void Prepend(wxString what);*/
	void Insert(const wxString &what, int position);
	wxString GetString(int pos){
		if(pos<0 || pos >= (int)list->size()){
			return "";
		}
		return (*list)[pos];
	}
	int GetSelection(){return choice;};
	void SetValue(const wxString &text);
	wxString GetValue();
	void Select(int sel){choice=sel;}
	int GetCount();
	int FindString(const wxString &text, bool caseSensitive = false);
	void EnableItem(int numItem, bool enable=true);
	int Append(const wxString &item);
	void Append(const wxArrayString &itemsArray);
	void PutArray(wxArrayString *arr);
	void Delete(int num);
	void SetToolTip(const wxString &tooltip="");
	void Sort();
	bool SetBackgroundColour(const wxColour &col);
	bool SetForegroundColour(const wxColour &col);
	bool HasFocus();
	KaiTextCtrl *choiceText;
private:
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void OnEraseBackground(wxEraseEvent &event){}
	void ShowList();
	void SendEvent(int choice);
	void SetSelectionByPartialName(const wxString &PartialName, bool changeText=false);
	void SelectChoice(int sel, bool select = true, bool sendEvent=true);
	inline void CalcMaxWidth(wxSize *result, bool changex, bool changey);
	void OnArrow(wxCommandEvent &evt);
	void OnActivate(wxFocusEvent &evt);
	bool enter;
	bool clicked;
	wxArrayString *list;
	std::map<int, bool> *disabled;
	wxBitmap *bmp;
	int choice;
	bool listIsShown;
	bool focusSet;
	wxString toolTip;
	PopupList *itemList;
	wxString txtchoice;
	bool useFgCol;

	wxDECLARE_ABSTRACT_CLASS(KaiChoice);
	DECLARE_EVENT_TABLE()
};

enum{
	KAI_COMBO_BOX = 1,
	KAI_SCROLL_ON_FOCUS
};

#endif