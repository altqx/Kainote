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

#pragma once

#include "KaiListCtrl.h"
#include "MappedButton.h"
#include "KaiDialog.h"


class Stylelistbox: public KaiDialog
{
	public:

		Stylelistbox(wxWindow* parent, bool styles = true, int numelem = 0, 
			wxString *arr = 0, const wxPoint& pos = wxDefaultPosition, int style = 0);
		Stylelistbox(wxWindow* parent, const wxArrayString & arr, bool styles = true, 
			const wxPoint& pos = wxDefaultPosition, int style = 0);
		virtual ~Stylelistbox();

		
		MappedButton* OK;
		KaiListCtrl* CheckListBox;
		MappedButton* Cancel;
		
};

class CustomCheckListBox : public KaiDialog
{
public:

	CustomCheckListBox(wxWindow* parent, const wxArrayString &listElems, 
		const wxString &title, const wxPoint& pos = wxDefaultPosition, int style = 0);
	virtual ~CustomCheckListBox(){};

	void GetCheckedElements(wxArrayString &checkedElements);

	MappedButton* OK;
	KaiListCtrl* CheckListBox;
	MappedButton* Cancel;

};

wxString GetCheckedElements(wxWindow *parent);

class KaiListBox : public KaiDialog
{
public:
	KaiListBox(wxWindow *parent, const wxArrayString &list, const wxString &title, bool centerOnParent = false);
	virtual ~KaiListBox(){};
	KaiListCtrl *list;
	wxString GetSelection() const{return result;};
	int GetIntSelection() {return selection;}
private:
	void OnDoubleClick(wxCommandEvent& evt);
	void OnOKClick(wxCommandEvent& evt);
	wxString result;
	int selection;
};


