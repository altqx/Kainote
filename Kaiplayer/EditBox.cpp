﻿//  Copyright (c) 2016, Marcin Drob

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


#include "EditBox.h"
#include "SubsGrid.h"
#include "KainoteApp.h"
#include <wx/regex.h>
#include <wx/tglbtn.h>
#include "FontDialog.h"
#include "Visuals.h"
#include "KaiMessageBox.h"


DescTxtCtrl::DescTxtCtrl(wxWindow *parent, int id, const wxSize &size, const wxString &desc, const wxValidator &validator)
	:KaiChoice(parent, id, "", wxDefaultPosition, size, wxArrayString(), 0, validator)
{
	description = desc;
	choiceText->Bind(wxEVT_SET_FOCUS, &DescTxtCtrl::OnFocus, this);
	choiceText->Bind(wxEVT_KILL_FOCUS, &DescTxtCtrl::OnKillFocus, this);
}

void DescTxtCtrl::ChangeValue(const wxString &val)
{
	if (val == "" && !choiceText->HasFocus()){
		SetForegroundColour(WindowTextInactive);
		SetValue(description);
		Refresh(false);
	}
	else{
		SetForegroundColour(WindowText);
		SetValue(val);
		Refresh(false);
	}
}

void DescTxtCtrl::OnFocus(wxFocusEvent &evt)
{
	if (choiceText->GetForegroundColour() == WindowTextInactive){
		SetValue("");
		SetForegroundColour(WindowText);
	}
	evt.Skip();
}
void DescTxtCtrl::OnKillFocus(wxFocusEvent &evt)
{
	if (GetValue() == ""){
		SetForegroundColour(WindowTextInactive);
		SetValue(description);
	}
	evt.Skip();
}




txtdialog::txtdialog(wxWindow *parent, int id, const wxString &txtt, const wxString &_name, int _type)
	:KaiDialog(parent, id, _("Wpisz tag ASS"))
{
	DialogSizer *siz = new DialogSizer(wxVERTICAL);
	wxBoxSizer *siz1 = new wxBoxSizer(wxHORIZONTAL);
	wxString types[3] = { _("Tag wstawiany w miejse kursora"), _("Tag wstawiany na początku tekstu"), _("Zwykły tekst") };
	type = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, types);
	type->SetSelection(_type);
	name = new KaiTextCtrl(this, -1, _name, wxDefaultPosition, wxSize(150, 25), wxTE_PROCESS_ENTER);
	txt = new KaiTextCtrl(this, -1, txtt, wxDefaultPosition, wxSize(150, 25), wxTE_PROCESS_ENTER);
	txt->SetSelection(0, txtt.Len());
	txt->SetFocus();
	siz->Add(type, 0, wxEXPAND | wxALL, 4);
	siz->Add(new KaiStaticText(this, -1, _("Nazwa przycisku")), 0, wxEXPAND | wxALL, 4);
	siz->Add(name, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
	siz->Add(new KaiStaticText(this, -1, _("Tag przycisku")), 0, wxEXPAND | wxALL, 4);
	siz->Add(txt, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
	siz1->Add(new MappedButton(this, wxID_OK, _("Zapisz tag")), 0, wxEXPAND | wxALL, 4);
	siz1->Add(new MappedButton(this, wxID_CANCEL, _("Anuluj")), 0, wxEXPAND | wxALL, 4);
	siz->Add(siz1, 0, wxEXPAND, 0);
	SetSizerAndFit(siz);
	MoveToMousePosition(this);

}
wxDEFINE_EVENT(TAG_BUTTON_EDITION, wxCommandEvent);

TagButton::TagButton(wxWindow *parent, int id, const wxString &_name, const wxString &_tag, int _type, const wxSize &size)
	: MappedButton(parent, id, _name, "", wxDefaultPosition, size, EDITBOX_HOTKEY)
{
	wxString rest;
	type = _type;
	tag = _tag;
	if (tag != ""){
		SetToolTip(tag);
	}
	name = _name;
	Bind(wxEVT_LEFT_UP, &TagButton::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &TagButton::OnMouseEvent, this);
}

void TagButton::OnMouseEvent(wxMouseEvent& event)
{
	if (event.RightUp() || (tag == "" && event.LeftUp())){
		wxCommandEvent evt(TAG_BUTTON_EDITION, GetId());
		ProcessEvent(evt);
		SetFocus();
		return;
	}
	else if (event.LeftUp()){
		bool oldclicked = clicked;
		clicked = false;
		Refresh(false);
		if (oldclicked){
			wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, GetId());
			ProcessEvent(evt);
		}
		return;
	}

	event.Skip();
}



EditBox::EditBox(wxWindow *parent, SubsGrid *grid1, int idd)
	: wxWindow(parent, idd, wxDefaultPosition, wxDefaultSize/*, wxBORDER_SIMPLE*/)//|wxCLIP_CHILDREN
	, EditCounter(1)
	, ABox(NULL)
	, line(NULL)
	, lastVisible(true)
	, CurrentDoubtful(0)
	, CurrentUntranslated(0)
	, TagButtonManager(NULL)
{

	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	SetFont(wxFont(9, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, "Tahoma"));
	currentLine = 0;
	grid = grid1;
	grid->Edit = this;
	isdetached = splittedTags = false;
	Visual = 0;

	wxArrayString ans;
	ans.Add("an1");
	ans.Add("an2");
	ans.Add("an3");
	ans.Add("an4");
	ans.Add("an5");
	ans.Add("an6");
	ans.Add("an7");
	ans.Add("an8");
	ans.Add("an9");


	Bfont = new MappedButton(this, EDITBOX_CHANGE_FONT, "", _("Wybór czcionki"), wxDefaultPosition, wxSize(24, 24));
	Bfont->SetBitmap(wxBITMAP_PNG("FONTS"));
	Bcol1 = new MappedButton(this, EDITBOX_CHANGE_COLOR_PRIMARY, "", _("Kolor podstawowy"), wxDefaultPosition, wxSize(24, 24));
	Bcol1->SetBitmap(wxBITMAP_PNG("Kolor1"));
	Bcol1->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol2 = new MappedButton(this, EDITBOX_CHANGE_COLOR_SECONDARY, "", _("Kolor zastępczy do karaoke"), wxDefaultPosition, wxSize(24, 24));
	Bcol2->SetBitmap(wxBITMAP_PNG("Kolor2"));
	Bcol2->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol3 = new MappedButton(this, EDITBOX_CHANGE_COLOR_OUTLINE, "", _("Kolor obwódki"), wxDefaultPosition, wxSize(24, 24));
	Bcol3->SetBitmap(wxBITMAP_PNG("Kolor3"));
	Bcol3->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol4 = new MappedButton(this, EDITBOX_CHANGE_COLOR_SHADOW, "", _("Kolor cienia"), wxDefaultPosition, wxSize(24, 24));
	Bcol4->SetBitmap(wxBITMAP_PNG("Kolor4"));
	Bcol4->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bbold = new MappedButton(this, PutBold, "", _("Pogrubienie"), wxDefaultPosition, wxSize(24, 24));
	Bbold->SetBitmap(wxBITMAP_PNG("BOLD"));
	Bital = new MappedButton(this, PutItalic, "", _("Pochylenie"), wxDefaultPosition, wxSize(24, 24));
	Bital->SetBitmap(wxBITMAP_PNG("ITALIC"));
	Bund = new MappedButton(this, EDITBOX_CHANGE_UNDERLINE, "", _("Podkreślenie"), wxDefaultPosition, wxSize(24, 24));
	Bund->SetBitmap(wxBITMAP_PNG("UNDER"));
	Bstrike = new MappedButton(this, EDITBOX_CHANGE_STRIKEOUT, "", _("Przekreślenie"), wxDefaultPosition, wxSize(24, 24));
	Bstrike->SetBitmap(wxBITMAP_PNG("STRIKE"));
	Ban = new KaiChoice(this, ID_AN, wxDefaultPosition, wxSize(48, 24), ans);
	Ban->Select(1);

	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer4->Add(Bfont, 0, wxALL, 2);
	BoxSizer4->Add(Bbold, 0, wxALL, 2);
	BoxSizer4->Add(Bital, 0, wxALL, 2);
	BoxSizer4->Add(Bund, 0, wxALL, 2);
	BoxSizer4->Add(Bstrike, 0, wxALL, 2);
	BoxSizer4->Add(Bcol1, 0, wxALL, 2);
	BoxSizer4->Add(Bcol2, 0, wxALL, 2);
	BoxSizer4->Add(Bcol3, 0, wxALL, 2);
	BoxSizer4->Add(Bcol4, 0, wxALL, 2);
	BoxSizer4->Add(Ban, 0, wxALL, 2);

	SetTagButtons();

	TlMode = new KaiCheckBox(this, ID_TLMODE, _("Tryb tłumaczenia"));
	TlMode->SetValue(false);
	TlMode->Enable(false);
	LineNumber = new KaiStaticText(this, -1, _("Linia: 0"));
	Chars = new KaiStaticText(this, -1, _("Łamania: 0/86"));
	Chtime = new KaiStaticText(this, -1, _("Znaki na sekundę: 0<=15"));
	bool asFrames = Options.GetBool(EDITBOX_TIMES_TO_FRAMES_SWITCH);
	Times = new KaiRadioButton(this, ID_TIMES_FRAMES, _("Czas"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	Times->SetValue(!asFrames);
	Times->Enable(false);
	Frames = new KaiRadioButton(this, ID_TIMES_FRAMES, _("Klatki"));
	Frames->Enable(false);
	if (asFrames){
		Frames->SetValue(asFrames);
	}

	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &EditBox::OnChangeTimeDisplay, this, ID_TIMES_FRAMES);

	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer5->Add(LineNumber, 0, wxALIGN_CENTER | wxLEFT | wxEXPAND, 2);
	BoxSizer5->Add(Chars, 0, wxALIGN_CENTER | wxLEFT | wxEXPAND, 6);
	BoxSizer5->Add(Chtime, 0, wxALIGN_CENTER | wxLEFT | wxEXPAND, 6);
	BoxSizer5->Add(TlMode, 0, wxALIGN_CENTER | wxLEFT, 6);
	BoxSizer5->Add(Times, 0, wxALIGN_CENTER | wxLEFT, 2);
	BoxSizer5->Add(Frames, 0, wxALIGN_CENTER | wxLEFT, 2);


	Bcpall = new MappedButton(this, EDITBOX_PASTE_ALL_TO_TRANSLATION, _("Wklej wszystko"), EDITBOX_HOTKEY);
	Bcpall->Hide();
	Bcpsel = new MappedButton(this, EDITBOX_PASTE_SELECTION_TO_TRANSLATION, _("Wklej zaznaczone"), EDITBOX_HOTKEY);
	Bcpsel->Hide();
	Bhide = new MappedButton(this, EDITBOX_HIDE_ORIGINAL, _("Ukryj oryginał"), EDITBOX_HOTKEY);
	Bhide->Hide();
	DoubtfulTL = new ToggleButton(this, ID_DOUBTFULTL, _("Niepewne"));
	DoubtfulTL->Hide();
	AutoMoveTags = new ToggleButton(this, ID_AUTOMOVETAGS, _("Przenoszenie tagów"));
	AutoMoveTags->Hide();

	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer6->Add(Bcpsel, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(Bcpall, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(Bhide, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(DoubtfulTL, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(AutoMoveTags, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);


	TextEdit = new TextEditor(this, 16667, Options.GetBool(SpellcheckerOn), wxDefaultPosition, wxSize(-1, 30));
	TextEdit->EB = this;

	TextEditOrig = new TextEditor(this, 16667, false, wxDefaultPosition, wxSize(-1, 30));
	TextEditOrig->EB = this;

	TextEditOrig->Hide();
	Comment = new KaiCheckBox(this, ID_COMMENT, _("Komentarz"), wxDefaultPosition, wxSize(82, -1));
	Comment->SetValue(false);
	LayerEdit = new NumCtrl(this, 16668, "", -10000000, 10000000, true, wxDefaultPosition, wxSize(50, -1));
	StartEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(82, -1), wxTE_CENTER);
	EndEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(82, -1), wxTE_CENTRE);
	DurEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(82, -1), wxTE_CENTRE);
	wxArrayString styles;
	styles.Add("Default");
	StyleChoice = new KaiChoice(this, ID_STYLE, wxDefaultPosition, wxSize(100, -1), styles);//wxSize(145,-1)
	StyleEdit = new MappedButton(this, 19989, _("Edytuj"), EDITBOX_HOTKEY, wxDefaultPosition, wxSize(45, -1));
	//druga linia
	wxTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	valid.SetCharExcludes(",");
	ActorEdit = new DescTxtCtrl(this, 16658, wxSize(90, -1), _("Aktor"), valid);
	MarginLEdit = new NumCtrl(this, 16668, "", 0, 9999, true, wxDefaultPosition, wxSize(42, -1), wxTE_CENTRE);
	MarginREdit = new NumCtrl(this, 16668, "", 0, 9999, true, wxDefaultPosition, wxSize(42, -1), wxTE_CENTRE);
	MarginVEdit = new NumCtrl(this, 16668, "", 0, 9999, true, wxDefaultPosition, wxSize(42, -1), wxTE_CENTRE);
	EffectEdit = new DescTxtCtrl(this, 16658, wxSize(90, -1), _("Efekt"), valid);

	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer2->Add(Comment, 0, wxLEFT | wxALIGN_CENTER, 2);
	BoxSizer2->Add(LayerEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(StartEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(EndEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(DurEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(StyleChoice, 4, wxLEFT | wxEXPAND, 2);
	BoxSizer2->Add(StyleEdit, 0, wxLEFT | wxRIGHT, 2);
	BoxSizer2->Add(ActorEdit, 3, wxEXPAND);
	BoxSizer2->Add(MarginLEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(MarginREdit, 0, wxLEFT, 2);
	BoxSizer2->Add(MarginVEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(EffectEdit, 3, wxLEFT | wxRIGHT | wxEXPAND, 2);
	
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(BoxSizer4, 0, wxLEFT | wxRIGHT | wxTOP, 2);
	BoxSizer1->Add(BoxSizer5, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);
	BoxSizer1->Add(TextEditOrig, 3, wxEXPAND | wxLEFT | wxRIGHT, 4);
	BoxSizer1->Add(BoxSizer6, 0, wxLEFT | wxRIGHT, 2);
	BoxSizer1->Add(TextEdit, 3, wxEXPAND | wxLEFT | wxRIGHT, 4);
	BoxSizer1->Add(BoxSizer2, 0, wxEXPAND | wxALL, 2);
	//BoxSizer1->Add(BoxSizer3,0,wxLEFT | wxRIGHT | wxBOTTOM,2);
	BoxSizer3 = NULL;

	SetSizer(BoxSizer1);

	Connect(ID_COMMENT, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(ID_TLMODE, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&EditBox::OnTlMode); //16658
	Connect(16658, wxEVT_COMMAND_COMBOBOX_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(ID_STYLE, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(PutBold, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnBoldClick);
	Connect(PutItalic, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnItalicClick);
	Connect(EDITBOX_CHANGE_UNDERLINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnUnderlineClick);
	Connect(EDITBOX_CHANGE_STRIKEOUT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnStrikeClick);
	Connect(ID_AN, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&EditBox::OnAnChoice);
	Connect(EDITBOX_CHANGE_FONT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnFontClick);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnColorClick, this, EDITBOX_CHANGE_COLOR_PRIMARY, EDITBOX_CHANGE_COLOR_SHADOW);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnStyleEdit, this, 19989);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnCopyAll, this, EDITBOX_PASTE_ALL_TO_TRANSLATION);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnCopySelection, this, EDITBOX_PASTE_SELECTION_TO_TRANSLATION);
	Connect(EDITBOX_HIDE_ORIGINAL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnHideOriginal);
	Connect(ID_DOUBTFULTL, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&EditBox::OnDoubtfulTl);
	Connect(ID_AUTOMOVETAGS, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&EditBox::OnAutoMoveTags);
	Connect(EDITBOX_COMMIT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(EDITBOX_COMMIT_GO_NEXT_LINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnNewline);
	Connect(FindNextDoubtful, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::FindNextDoubtfulTl);
	Connect(FindNextUntranslated, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::FindNextUnTranslated);
	Connect(SetDoubtful, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnDoubtfulTl);
	Connect(SplitLine, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnSplit);
	Connect(StartDifference, EndDifference, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnPasteDifferents);
	Connect(wxEVT_SIZE, (wxObjectEventFunction)&EditBox::OnSize);
	if (!Options.GetBool(DisableLiveVideoEditing)){
		Connect(16668, NUMBER_CHANGED, (wxObjectEventFunction)&EditBox::OnEdit);
		Connect(16667, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&EditBox::OnEdit);
	}
	Connect(16667, CURSOR_MOVED, (wxObjectEventFunction)&EditBox::OnCursorMoved);
	DoTooltips();
	if (asFrames){
		grid->ChangeTimeDisplay(asFrames);
		StartEdit->ShowFrames(asFrames);
		EndEdit->ShowFrames(asFrames);
		DurEdit->ShowFrames(asFrames);
	}
}

EditBox::~EditBox()
{
	wxDELETE(line);
}

void EditBox::SetLine(int Row, bool setaudio, bool save, bool nochangeline, bool autoPlay)
{
	//if (!grid->GetCount()){ Enable(false); return; }
	//else if (!IsEnabled()){ Enable(); }
	TabPanel* tab = (TabPanel*)GetParent();
	bool rowChanged = currentLine != Row;
	if (nochangeline && !rowChanged){ goto done; }
	if (Options.GetInt(GridSaveAfterCharacterCount) > 1 && rowChanged && save){
		Send(EDITBOX_LINE_EDITION, false);
	}
	Dialogue *prevDial = grid->GetDialogue(currentLine);
	if (prevDial && prevDial->Start.mstime > prevDial->End.mstime){
		prevDial->End = prevDial->Start;
		grid->Refresh(false);
	}
	if (StartEdit->changedBackGround){
		StartEdit->SetForegroundColour(WindowText);
	}
	if (EndEdit->changedBackGround){
		EndEdit->SetForegroundColour(WindowText);
	}
	if (DurEdit->changedBackGround){
		DurEdit->SetForegroundColour(WindowText);
	}
	currentLine = Row;
	grid->markedLine = Row;
	grid->currentLine = Row;
	wxDELETE(line);
	line = grid->GetDialogue(currentLine)->Copy();
	LineNumber->SetLabelText(wxString::Format(_("Linia: %i"), currentLine + 1));
	Comment->SetValue(line->IsComment);
	LayerEdit->SetInt(line->Layer);
	StartEdit->SetTime(line->Start, false, 1);
	EndEdit->SetTime(line->End, false, 2);
	if (grid->showFrames){
		STime durationTime = EndEdit->GetTime() - StartEdit->GetTime();
		durationTime.orgframe++;
		DurEdit->SetTime(durationTime);
	}
	else{
		DurEdit->SetTime(line->End - line->Start);
	}
	StyleChoice->SetSelection(StyleChoice->FindString(line->Style, true));
	ActorEdit->ChangeValue(line->Actor);
	MarginLEdit->SetInt(line->MarginL);
	MarginREdit->SetInt(line->MarginR);
	MarginVEdit->SetInt(line->MarginV);
	EffectEdit->ChangeValue(line->Effect);
	TextEdit->SetState((!line->IsComment) ? 0 : (line->Effect->StartsWith("template")) ? 2 : (line->Effect->StartsWith("code")) ? 3 : 1);
	SetTextWithTags();

	if (DoubtfulTL->IsShown()){
		DoubtfulTL->SetValue(line->IsDoubtful());
	}

	if (setaudio && ABox && ABox->IsShown()){ ABox->audioDisplay->SetDialogue(line, currentLine); }

	//ustawia znaki na sekundę i ilość linii
	UpdateChars((TextEditOrig->IsShown() && line->TextTl != "") ? line->TextTl : line->Text);

done:
	VideoCtrl *vb = tab->Video;
	int pas = vb->vToolbar->videoPlayAfter->GetSelection();
	int vsa = vb->vToolbar->videoSeekAfter->GetSelection();

	if (vsa == 1 && pas < 2 && !nochangeline && rowChanged){
		if (vb->GetState() != None){
			if (vb->GetState() == Playing){ vb->Pause(); }
			vb->Seek(line->Start.mstime);
		}
		//return;
	}

	if (pas > 0 && autoPlay){
		if (pas == 1){
			if (ABox){
				wxWindow *focused = wxWindow::FindFocus();
				wxCommandEvent evt; ABox->OnPlaySelection(evt);
				focused->SetFocus();
			}
		}
		else{
			if (tab->Video->IsShown() || tab->Video->isFullscreen){
				Dialogue *next = grid->GetDialogue(MIN(currentLine + 1, grid->GetCount() - 1));
				int ed = line->End.mstime, nst = next->Start.mstime;
				int playend = (nst > ed && pas > 2) ? nst : ed;
				tab->Video->PlayLine(line->Start.mstime, tab->Video->GetPlayEndTime(playend));
			}
		}
	}

	if (Visual > CHANGEPOS && rowChanged && !nochangeline){
		tab->Video->SetVisual(false, true, true);
	}
	
	//ustawia czas i msy na polu tekstowym wideo
	if (tab->Video->IsShown() && tab->Video->GetState() != None && vsa == 0){
		tab->Video->RefreshTime();
	}

}

void EditBox::UpdateChars(const wxString &text)
{

	if (line->IsComment){
		Chars->SetLabelText("");
		Chtime->SetLabelText("");
	}
	else{
		wxString result;
		bool isbad = false;
		int ilzn = grid->CalcChars(text, &result, &isbad);
		Chars->SetLabelText(_("Łamania: ") + result + "43");
		Chars->SetForegroundColour((isbad) ? WindowWarningElements : WindowText);
		int chtime = ilzn / ((line->End.mstime - line->Start.mstime) / 1000.0f);
		if (chtime < 0 || chtime>999){ chtime = 999; }
		Chtime->SetLabelText(wxString::Format(_("Znaki na sekundę: %i<=15"), chtime));
		Chtime->SetForegroundColour((chtime > 15) ? WindowWarningElements : WindowText);
	}
	BoxSizer5->Layout();
	Frames->Refresh(false);
	Frames->Update();
	Times->Refresh(false);
	Times->Update();
}

//Pobieranie danych z kontrolek editboxa
//selline przechodzi do następnej linii
//dummy nie zapisuje linii do grida
//visualdummy nie odświeża klatki wideo wykorzystywane przy visualu clipów
void EditBox::Send(unsigned char editionType, bool selline, bool dummy, bool visualdummy)
{
	long cellm = 0;
	if (!dummy && !visualdummy && StartEdit->changedBackGround){
		StartEdit->SetForegroundColour(WindowText);//StartEdit->Refresh(false);
	}
	if (!dummy && !visualdummy && EndEdit->changedBackGround){
		EndEdit->SetForegroundColour(WindowText);//EndEdit->Refresh(false);
	}
	if (!dummy && !visualdummy && DurEdit->changedBackGround){
		DurEdit->SetForegroundColour(WindowText);//DurEdit->Refresh(false);
	}
	if (line->IsComment != Comment->GetValue()){
		line->IsComment = !line->IsComment;
		cellm |= COMMENT;
	}

	if (LayerEdit->IsModified()){
		line->Layer = LayerEdit->GetInt();
		cellm |= LAYER;
		LayerEdit->SetModified(dummy);
	}

	if (StartEdit->IsModified() || StartEdit->HasFocus()){
		line->Start = StartEdit->GetTime(1);
		//if(!visualdummy && line->Start.mstime>line->End.mstime){line->End=line->Start; cellm |= END;}
		cellm |= START;
		StartEdit->SetModified(dummy);
	}
	if (EndEdit->IsModified() || EndEdit->HasFocus()){
		line->End = EndEdit->GetTime(2);
		//if(!visualdummy && line->Start.mstime>line->End.mstime){line->End=line->Start; cellm |= START;}
		cellm |= END;
		EndEdit->SetModified(dummy);
	}
	if (DurEdit->IsModified()){
		line->End = EndEdit->GetTime();
		//if(line->Start.mstime>line->End.mstime){line->End=line->Start;}
		cellm |= END;
		DurEdit->SetModified(dummy);
	}

	wxString checkstyle = StyleChoice->GetString(StyleChoice->GetSelection());
	if (line->Style != checkstyle && checkstyle != "" || StyleChoice->HasFocus()){
		line->Style = checkstyle;
		cellm |= STYLE;
	}
	if (ActorEdit->choiceText->IsModified()){
		line->Actor = ActorEdit->GetValue();
		cellm |= ACTOR;
		ActorEdit->choiceText->SetModified(dummy);
	}
	if (MarginLEdit->IsModified()){
		line->MarginL = MarginLEdit->GetInt();
		cellm |= MARGINL;
		MarginLEdit->SetModified(dummy);
	}
	if (MarginREdit->IsModified()){
		line->MarginR = MarginREdit->GetInt();
		cellm |= MARGINR;
		MarginREdit->SetModified(dummy);
	}
	if (MarginVEdit->IsModified()){
		line->MarginV = MarginVEdit->GetInt();
		cellm |= MARGINV;
		MarginVEdit->SetModified(dummy);
	}
	if (EffectEdit->choiceText->IsModified()){
		line->Effect = EffectEdit->GetValue();
		cellm |= EFFECT;
		EffectEdit->choiceText->SetModified(dummy);
	}

	if (TextEdit->IsModified() || splittedTags){
		if (TextEditOrig->IsShown()){
			line->TextTl = TextEdit->GetValue();
			cellm |= TXTTL;
		}
		else{
			line->Text = TextEdit->GetValue();
			cellm |= TXT;
		}
		TextEdit->SetModified(dummy);
	}
	if (TextEditOrig->IsShown() && (TextEditOrig->IsModified() || splittedTags)){
		line->Text = TextEditOrig->GetValue();
		cellm |= TXT;
		TextEditOrig->SetModified(dummy);
	}

	if (cellm){
		if (currentLine < grid->GetCount() && !dummy){
			grid->ChangeLine(editionType, line, currentLine, cellm, selline, visualdummy);
			if (cellm & ACTOR || cellm & EFFECT){
				RebuildActorEffectLists();
			}
		}
	}
	else if (selline){ grid->NextLine(); }
}


void EditBox::PutinText(const wxString &text, bool focus, bool onlysel, wxString *texttoPutin)
{
	bool oneline = (grid->file->SelectionsSize() < 2);
	if (oneline && !onlysel){
		long whre;
		wxString txt = TextEdit->GetValue();
		TextEditor *Editor = TextEdit;
		if (grid->hasTLMode && txt == ""){
			txt = TextEditOrig->GetValue();
			Editor = TextEditOrig;
		}
		if (!InBracket){
			txt.insert(Placed.x, "{" + text + "}");
			whre = cursorpos + text.Len() + 2;
		}
		else{
			if (Placed.x < Placed.y){
				txt.erase(txt.begin() + Placed.x, txt.begin() + Placed.y + 1);
				whre = (focus) ? cursorpos + text.Len() - (Placed.y - Placed.x) : Placed.x;
			}
			else{ whre = (focus) ? cursorpos + 1 + text.Len() : Placed.x; }
			txt.insert(Placed.x, text);
		}
		if (text == ""){ txt.Replace("{}", ""); }
		if (texttoPutin){
			*texttoPutin = txt;
			return;
		}
		Editor->SetTextS(txt, true);
		if (focus){ Editor->SetFocus(); }
		Editor->SetSelection(whre, whre);//}else{Placed.x=whre;}CopyDialogueByKey
	}
	else{
		wxString tmp;
		wxArrayInt sels;
		grid->file->GetSelectionsAsKeys(sels);
		for (size_t i = 0; i < sels.size(); i++){
			Dialogue *dialc = grid->CopyDialogueByKey(sels[i]);
			wxString txt = (grid->hasTLMode && dialc->TextTl != "") ? dialc->TextTl : dialc->Text;
			FindVal(lasttag, &tmp, txt);

			if (InBracket && txt != ""){
				if (Placed.x < Placed.y){ txt.erase(txt.begin() + Placed.x, txt.begin() + Placed.y + 1); }
				txt.insert(Placed.x, text);
				if (grid->hasTLMode && dialc->TextTl != ""){
					dialc->TextTl = txt;
				}
				else{ dialc->Text = txt; }
			}
			else{
				if (grid->hasTLMode && dialc->TextTl != ""){
					dialc->TextTl->Prepend("{" + text + "}");
				}
				else{ dialc->Text->Prepend("{" + text + "}"); }
			}
		}
		grid->SetModified(EDITBOX_MULTILINE_EDITION);
		grid->Refresh(false);
	}

}

void EditBox::PutinNonass(const wxString &text, const wxString &tag)
{
	if (grid->subsFormat == TMP)return;
	long from, to, whre;
	size_t start = 0, len = 0;
	bool match = false;
	TextEdit->GetSelection(&from, &to);
	wxString txt = TextEdit->GetValue();
	bool oneline = (grid->file->SelectionsSize() < 2);
	if (oneline){//zmiany tylko w editboxie
		if (grid->subsFormat == SRT){

			wxRegEx srttag("\\</?" + text + "\\>", wxRE_ADVANCED | wxRE_ICASE);
			if (srttag.Matches(txt.SubString(from - 4, from + 4))){
				srttag.GetMatch(&start, &len, 0);
				if (len + start >= 4 && start <= 4)
				{
					whre = from - 4 + start;
					txt.Remove(whre, len);
					txt.insert(whre, "<" + tag + ">");
					whre += 3;
					match = true;
				}
			}
			if (!match){ txt.insert(from, "<" + tag + ">"); from += 3; to += 3; whre = from; }
			if (from != to){
				match = false;
				if (srttag.Matches(txt.SubString(to - 4, to + 4))){
					srttag.GetMatch(&start, &len, 0);
					if (len + start >= 4 && start <= 4)
					{
						txt.Remove(to - 4 + start, len);
						txt.insert(to - 4 + start, "</" + tag + ">");
						whre = to + start;
						match = true;
					}
				}
				if (!match){ txt.insert(to, "</" + tag + ">"); whre = to + 4; }
			}

		}
		else if (grid->subsFormat == MDVD){


			wxRegEx srttag("\\{" + text + "}", wxRE_ADVANCED | wxRE_ICASE);
			int wheres = txt.SubString(0, from).Find('|', true);
			if (wheres == -1){ wheres = 0; }
			if (srttag.Matches(txt.Mid(wheres))){
				if (srttag.GetMatch(&start, &len, 0))
				{
					whre = wheres + start;
					txt.Remove(whre, len);
					txt.insert(whre, "{" + tag + "}");
					match = true;
				}
			}
			if (!match){ txt.insert(wheres, "{" + tag + "}"); }

		}

		TextEdit->SetTextS(txt, true);
		TextEdit->SetFocus();
		TextEdit->SetSelection(whre, whre);
	}
	else
	{//zmiany wszystkich zaznaczonych linijek
		wxString chars = (grid->subsFormat == SRT) ? "<" : "{";
		wxString chare = (grid->subsFormat == SRT) ? ">" : "}";
		wxArrayInt sels;
		grid->file->GetSelectionsAsKeys(sels);
		for (size_t i = 0; i < sels.size(); i++)
		{
			Dialogue *dialc = grid->file->CopyDialogueByKey(sels[i]);
			wxString txt = dialc->Text;
			//dialc->spells.Clear();
			if (txt.StartsWith(chars))
			{
				wxRegEx rex(chars + tag + chare, wxRE_ADVANCED | wxRE_ICASE);
				rex.ReplaceAll(&txt, "");
				dialc->Text = chars + text + chare + txt;
			}
			else
			{
				dialc->Text->Prepend(chars + text + chare);
			}
		}
		grid->SetModified(EDITBOX_MULTILINE_EDITION);
		grid->Refresh(false);
	}

}

void EditBox::OnFontClick(wxCommandEvent& event)
{
	char form = grid->subsFormat;
	Styles *mstyle = (form < SRT) ? grid->GetStyle(0, line->Style)->Copy() : new Styles();
	wxString tmp;
	int tmpIter = grid->file->Iter();
	if (form < SRT){

		if (FindVal("b(0|1)", &tmp)){ if (mstyle->Bold&&tmp == "0"){ mstyle->Bold = false; } else if (!mstyle->Bold&&tmp == "1"){ mstyle->Bold = true; } }
		if (FindVal("i(0|1)", &tmp)){ if (mstyle->Italic&&tmp == "0"){ mstyle->Italic = false; } else if (!mstyle->Italic&&tmp == "1"){ mstyle->Italic = true; } }
		if (FindVal("u(0|1)", &tmp)){ if (mstyle->Underline&&tmp == "0"){ mstyle->Underline = false; } else if (!mstyle->Underline&&tmp == "1"){ mstyle->Underline = true; } }
		if (FindVal("s(0|1)", &tmp)){ if (mstyle->StrikeOut&&tmp == "0"){ mstyle->StrikeOut = false; } else if (!mstyle->StrikeOut&&tmp == "1"){ mstyle->StrikeOut = true; } }
		if (FindVal("fs([0-9]+)", &tmp)){ mstyle->Fontsize = tmp; }
		if (FindVal("fn(.*)", &tmp)){ mstyle->Fontname = tmp; }
	}
	FontDialog *FD = FontDialog::Get(this, mstyle);
	FD->Bind(FONT_CHANGED, &EditBox::OnFontChange, this, FD->GetId());
	if (FD->ShowModal() == wxID_OK) {
		wxString txt = TextEdit->GetValue();
		TextEditor * Editor = TextEdit;
		if (grid->hasTLMode && txt == ""){
			Editor = TextEditOrig;
			txt = TextEditOrig->GetValue();
		}

		if (txt[Placed.x] != '}'){
			int bracketPos = txt.find("}", Placed.x);
			if (bracketPos != -1){ Placed.x = Placed.y = bracketPos + 1; }
		}
		Editor->SetSelection(Placed.x, Placed.x);
		Editor->SetFocus();
	}
	else{
		grid->DummyUndo(tmpIter);
	}

}

void EditBox::OnFontChange(wxCommandEvent& event){
	FontDialog *FD = (FontDialog *)event.GetClientData();
	if (!FD)
		return;
	Styles * input = NULL;
	Styles * output = NULL;
	FD->GetStyles(&input, &output);
	if (input && output)
		ChangeFont(output, input);
}

void EditBox::ChangeFont(Styles *retStyle, Styles *editedStyle)
{
	char form = grid->subsFormat;
	wxString tmp;
	if (retStyle->Fontname != editedStyle->Fontname)
	{
		if (form < SRT){
			FindVal("fn(.*)", &tmp);
			PutinText("\\fn" + retStyle->Fontname, false);
		}
		else{ PutinNonass("F:" + retStyle->Fontname, "f:([^}]*)"); }
	}
	if (retStyle->Fontsize != editedStyle->Fontsize)
	{
		if (form < SRT){
			FindVal("fs([0-9]+)", &tmp);
			PutinText("\\fs" + retStyle->Fontsize, false);
		}
		else{ PutinNonass("S:" + retStyle->Fontname, "s:([^}]*)"); }
	}
	if (retStyle->Bold != editedStyle->Bold)
	{
		if (form < SRT){
			wxString bld = (retStyle->Bold) ? "1" : "0";
			FindVal("b(0|1)", &tmp);
			PutinText("\\b" + bld, false);
		}
		else{ PutinNonass("y:b", (retStyle->Bold) ? "Y:b" : ""); }
	}
	if (retStyle->Italic != editedStyle->Italic)
	{
		if (form < SRT){
			wxString ital = (retStyle->Italic) ? "1" : "0";
			FindVal("i(0|1)", &tmp);
			PutinText("\\i" + ital, false);
		}
		else{ PutinNonass("y:i", (retStyle->Italic) ? "Y:i" : ""); }
	}
	if (retStyle->Underline != editedStyle->Underline)
	{
		FindVal("u(0|1)", &tmp);
		wxString under = (retStyle->Underline) ? "1" : "0";
		PutinText("\\u" + under, false);
	}
	if (retStyle->StrikeOut != editedStyle->StrikeOut)
	{
		FindVal("s(0|1)", &tmp);
		wxString strike = (retStyle->StrikeOut) ? "1" : "0";
		PutinText("\\s" + strike, false);
	}
}

void EditBox::AllColorClick(int numColor, bool leftClick /*= true*/)
{
	//check if need to switch hotkeys
	if (Options.GetBool(COLORPICKER_SWITCH_CLICKS))
		leftClick = !leftClick;

	wxString tmptext = TextEdit->GetValue();
	TextEditor *Editor = TextEdit;
	int tmpIter = grid->file->Iter();
	if (grid->hasTLMode && tmptext == ""){
		tmptext = TextEditOrig->GetValue();
		Editor = TextEditOrig;
	}
	
	AssColor actualColor = AssColor(wxString("#FFFFFF"));
	GetColor(&actualColor, numColor);

	if (leftClick){
		DialogColorPicker *ColourDialog = DialogColorPicker::Get(this, actualColor.GetWX(), numColor);
		MoveToMousePosition(ColourDialog);
		ColourDialog->Connect(11111, COLOR_CHANGED, (wxObjectEventFunction)&EditBox::OnColorChange, 0, this);
		ColourDialog->Bind(COLOR_TYPE_CHANGED, [=](wxCommandEvent &evt){
			AssColor col; 
			GetColor(&col, evt.GetInt());
			ColourDialog->SetColor(col, 0, false);
		}, 11111);
		if (ColourDialog->ShowModal() == wxID_OK) {
			//wywołane tylko by dodać kolor do recent;
			ColourDialog->GetColor();
			wxString txt = Editor->GetValue();
			if (txt[Placed.x] != '}'){
				int bracketPos = txt.find("}", Placed.x);
				if (bracketPos != -1){ Placed.x = Placed.y = bracketPos + 1; }
			}
			Editor->SetSelection(Placed.x, Placed.x);
		}
		else{
			grid->DummyUndo(tmpIter);
		}
	}
	else{
		SimpleColorPicker scp(this, actualColor, numColor);
		SimpleColorPickerDialog *scpd = scp.GetDialog();
		int spcdId = scpd->GetId();
		scpd->Bind(COLOR_CHANGED, &EditBox::OnColorChange, this, spcdId);
		scpd->Bind(COLOR_TYPE_CHANGED, [=](wxCommandEvent &evt){
			AssColor col;
			GetColor(&col, evt.GetInt());
			scpd->SetColor(col);
		}, spcdId);

		AssColor ret;
		if (scp.PickColor(&ret)){
			wxString txt = Editor->GetValue();
			if (txt[Placed.x] != '}'){
				int bracketPos = txt.find("}", Placed.x);
				if (bracketPos != -1){ Placed.x = Placed.y = bracketPos + 1; }
			}
			Editor->SetSelection(Placed.x, Placed.x);
		}
		else{
			grid->DummyUndo(tmpIter);
		}
	}
	Editor->SetFocus();
}

void EditBox::GetColor(AssColor *actualColor, int numColor)
{
	if ((grid->subsFormat < SRT)){
		wxString colorNumber;
		colorNumber << numColor;
		wxString retTag;
		wxString tag = (numColor == 1) ? "?c&(.*)" : "c&(.*)";
		/*wxString taga = (numColor == 1) ? "?a&(.*)" : "a&(.*)";
		wxString tagal = "alpha(.*)";*/
		Styles *style = grid->GetStyle(0, line->Style);
		*actualColor = (numColor == 1) ? style->PrimaryColour :
			(numColor == 2) ? style->SecondaryColour :
			(numColor == 3) ? style->OutlineColour :
			style->BackColour;
		if (FindVal(colorNumber + tag, &retTag)){
			actualColor->Copy(AssColor("&" + retTag));
		}
		//when knowing about alpha tag will be needed You must change it like in method OnColorChange
		if (FindVal(colorNumber + L"a&|alpha(.*)", &retTag)){ actualColor->SetAlphaString(retTag); }
		//else if (FindVal(tagal, &retTag)){ actualColor->SetAlphaString(retTag); return true; }
	}
	//return false;
}

void EditBox::OnColorClick(wxCommandEvent& event)
{
	AllColorClick(event.GetId() - EDITBOX_CHANGE_COLOR_PRIMARY + 1);
}

void EditBox::OnColorRightClick(wxMouseEvent& event)
{
	AllColorClick(event.GetId() - EDITBOX_CHANGE_COLOR_PRIMARY + 1, false);
}

void EditBox::OnCommit(wxCommandEvent& event)
{
	TabPanel* pan = (TabPanel*)GetParent();
	pan->Video->blockpaint = true;
	if (splittedTags && (TextEdit->IsModified() || TextEditOrig->IsModified())){
		TextEdit->SetModified(); TextEditOrig->SetModified(); splittedTags = false;
	}
	Send(EDITBOX_LINE_EDITION, false, false, Visual != 0);
	if (event.GetId() == ID_COMMENT){
		TextEdit->SetState((!line->IsComment) ? 0 : (line->Effect->StartsWith("template")) ? 2 : (line->Effect->StartsWith("code")) ? 3 : 1, true);
	}
	if (Visual){
		pan->Video->SetVisual(false, true);
	}
	if (StyleChoice->HasFocus() || Comment->HasFocus()){ grid->SetFocus(); }
	if (ABox){ ABox->audioDisplay->SetDialogue(line, currentLine); }
	pan->Video->blockpaint = false;
}

void EditBox::OnNewline(wxCommandEvent& event)
{
	if (splittedTags && (TextEdit->IsModified() || TextEditOrig->IsModified())){ TextEdit->SetModified(); TextEditOrig->SetModified(); }
	bool noNewLine = !(StartEdit->HasFocus() || EndEdit->HasFocus() || DurEdit->HasFocus()) || !Options.GetBool(NoNewLineAfterTimesEdition);
	if (!noNewLine && ABox){ ABox->audioDisplay->SetDialogue(line, currentLine); }
	Send(EDITBOX_LINE_EDITION, noNewLine);
	if (Visual == CHANGEPOS){
		TabPanel *tab = (TabPanel *)GetParent();
		tab->Video->SetVisual(false, true, true);
	}
	splittedTags = false;
}

void EditBox::OnBoldClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString wart = (mstyle->Bold) ? "0" : "1";
		bool issel = true;
		if (FindVal("b(0|1)", &wart, "", &issel)){ wart = (wart == "1") ? "0" : "1"; }
		PutinText("\\b" + wart);
		if (!issel)return;
		wart = (mstyle->Bold) ? "1" : "0";
		if (FindVal("b(0|1)", &wart)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\b" + wart);
	}
	else if (grid->subsFormat == SRT){ PutinNonass("b", "b"); }
	else { PutinNonass("y:b", "Y:b"); }
}

void EditBox::OnItalicClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString wart = (mstyle->Italic) ? "0" : "1";
		bool issel = true;
		if (FindVal("i(0|1)", &wart, "", &issel)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\i" + wart);
		if (!issel)return;
		wart = (mstyle->Italic) ? "1" : "0";
		if (FindVal("i(0|1)", &wart)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\i" + wart);
	}
	else if (grid->subsFormat == SRT){ PutinNonass("i", "i"); }
	else if (grid->subsFormat == MDVD){ PutinNonass("y:i", "Y:i"); }
	else{ PutinNonass("/", "/"); }
}

void EditBox::OnUnderlineClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString wart = (mstyle->Underline) ? "0" : "1";
		bool issel = true;
		if (FindVal("u(0|1)", &wart, "", &issel)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\u" + wart);
		if (!issel)return;
		wart = (mstyle->Underline) ? "1" : "0";
		if (FindVal("u(0|1)", &wart)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\u" + wart);
	}
	else if (grid->subsFormat == SRT){ PutinNonass("u", "u"); }
}

void EditBox::OnStrikeClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString wart = (mstyle->StrikeOut) ? "0" : "1";
		bool issel = true;
		if (FindVal("s(0|1)", &wart, "", &issel)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\s" + wart);
		if (!issel)return;
		wart = (mstyle->StrikeOut) ? "0" : "1";
		if (FindVal("s(0|1)", &wart)){ if (wart == "1"){ wart = "0"; } else{ wart = "1"; } }
		PutinText("\\s" + wart);
	}
	else if (grid->subsFormat == SRT){ PutinNonass("s", "s"); }
}

void EditBox::OnAnChoice(wxCommandEvent& event)
{
	TextEdit->SetSelection(0, 0);
	if (grid->hasTLMode){ TextEditOrig->SetSelection(0, 0); }
	lasttag = "an([0-9])";
	wxString tag;
	FindVal("an([0-9])", &tag);
	PutinText("\\" + Ban->GetString(Ban->GetSelection()), true);
}

void EditBox::OnTlMode(wxCommandEvent& event)
{
	bool show = !TextEditOrig->IsShown();
	if (grid->SetTlMode(show)){ TlMode->SetValue(true); return; }
	SetTl(show);
	SetLine(currentLine);
}

void EditBox::SetTl(bool tl)
{
	TextEditOrig->Show(tl);
	Bcpall->Show(tl);
	Bcpsel->Show(tl);
	Bhide->Show(tl);
	DoubtfulTL->Show(tl);
	AutoMoveTags->Show(tl);
	AutoMoveTags->SetValue(Options.GetBool(AutoMoveTagsFromOriginal));
	BoxSizer1->Layout();
	if (TlMode->GetValue() != tl){ TlMode->SetValue(tl); }
	KainoteFrame *Kai = (KainoteFrame*)Notebook::GetTabs()->GetParent();
	Kai->Toolbar->UpdateId(SaveTranslation, tl);
}

void EditBox::OnCopyAll(wxCommandEvent& event)
{
	TextEdit->SetTextS(TextEditOrig->GetValue(), true);
	TextEdit->SetFocus();
}

void EditBox::OnCopySelection(wxCommandEvent& event)
{
	long from, to, fromtl, totl;
	TextEditOrig->GetSelection(&from, &to);
	if (from != to){
		wxString txt = TextEditOrig->GetValue();
		wxString txt1 = TextEdit->GetValue();
		TextEdit->GetSelection(&fromtl, &totl);
		wxString txtt = txt.SubString(from, to - 1);
		txt1.insert(fromtl, txtt);
		TextEdit->SetTextS(txt1, true);
		TextEdit->SetFocus();
		long whre = txtt.Len();
		TextEdit->SetSelection(fromtl + whre, fromtl + whre);
	}
}



void EditBox::RefreshStyle(bool resetline)
{
	StyleChoice->Clear();
	for (int i = 0; i < grid->StylesSize(); i++){
		StyleChoice->Append(grid->GetStyle(i)->Name);
	}
	int selection = grid->FindStyle(line->Style);
	StyleChoice->SetSelection(selection);

	if (resetline){
		if (grid->GetCount() > 0){
			SetLine(0);
		}
		else{ currentLine = 0; }
	}
}


void EditBox::DoTooltips()
{
	Ban->SetToolTip(_("Położenie tekstu"));
	TlMode->SetToolTip(_("Tryb tłumaczenia wyświetla i zapisuje zarówno tekst obcojęzyczny, jak i tekst tłumaczenia"));
	Bcpall->SetToolTip(_("Kopiuje cały tekst obcojęzyczny do pola z tłumaczeniem"));
	Bcpsel->SetToolTip(_("Kopiuje zaznaczony tekst obcojęzyczny do pola z tłumaczeniem"));
	//TextEdit->SetToolTip(_("Tekst linijki / tekst tłumaczenia, gdy tryb tłumaczenia jest włączony."));
	//TextEditOrig->SetToolTip(_("tekst obcojęzyczny."));
	Comment->SetToolTip(_("Ustawia linijkę jako komentarz. Komentarze nie są wyświetlane"));
	LayerEdit->SetToolTip(_("Warstwa linijki, wyższe warstwy są na wierzchu"));
	StartEdit->SetToolTip(_("Czas początkowy linijki"));
	EndEdit->SetToolTip(_("Czas końcowy linijki"));
	DurEdit->SetToolTip(_("Czas trwania linijki"));
	StyleChoice->SetToolTip(_("Styl linijki"));
	StyleEdit->SetToolTip(_("Umożliwia szybką edycję stylu linijki"));
	ActorEdit->SetToolTip(_("Oznaczenie aktora linijki. Nie wpływa na wygląd napisów"));
	MarginLEdit->SetToolTip(_("Margines lewy linijki"));
	MarginREdit->SetToolTip(_("Margines prawy linijki"));
	MarginVEdit->SetToolTip(_("Margines górny i dolny linijki"));
	EffectEdit->SetToolTip(_("Efekt linijki. Służy do oznaczania linijek, na których zastosowane ma być karaoke bądź efekty VSFiltra"));
	Chars->SetToolTip(_("Ilość znaków w każdej linijce.\nNie więcej niż 43 znaki na linijkę (maksymalnie 2 linijki)"));
	Chtime->SetToolTip(_("Znaki na sekundę.\nNie powinny przekraczać 15 znaków na sekundę"));
}

void EditBox::OnSize(wxSizeEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	if (isdetached && w > 800){
		BoxSizer1->Detach(BoxSizer3);

		for (int i = 5; i >= 0; i--){ BoxSizer3->Detach(i); }
		BoxSizer2->Insert(0, Comment, 0, wxLEFT | wxALIGN_CENTER, 4);
		BoxSizer2->Add(ActorEdit, 5, wxEXPAND);
		BoxSizer2->Add(MarginLEdit, 0, wxLEFT, 2);
		BoxSizer2->Add(MarginREdit, 0, wxLEFT, 2);
		BoxSizer2->Add(MarginVEdit, 0, wxLEFT, 2);
		BoxSizer2->Add(EffectEdit, 5, wxLEFT | wxRIGHT | wxEXPAND, 2);
		delete BoxSizer3; BoxSizer3 = NULL;
		SetSizer(BoxSizer1);

		isdetached = false;
	}
	else if (!isdetached && w <= 800)
	{
		for (int i = 11; i >= 7; i--){ BoxSizer2->Detach(i); }
		BoxSizer2->Detach(0);
		BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
		BoxSizer3->Add(Comment, 0, wxLEFT | wxALIGN_CENTER, 4);
		BoxSizer3->Add(ActorEdit, 5, wxEXPAND | wxLEFT, 2);
		BoxSizer3->Add(MarginLEdit, 0, wxLEFT, 2);
		BoxSizer3->Add(MarginREdit, 0, wxLEFT, 2);
		BoxSizer3->Add(MarginVEdit, 0, wxLEFT, 2);
		BoxSizer3->Add(EffectEdit, 5, wxLEFT | wxRIGHT | wxEXPAND, 2);
		BoxSizer1->Add(BoxSizer3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
		SetSizer(BoxSizer1);

		isdetached = true;
	}

	if (ABox){
		wxSize aboxSize = ABox->GetClientSize();
		int minEBSize = (TextEditOrig->IsShown()) ? 200 : 150;
		if ((h - aboxSize.y) < minEBSize){
			ABox->SetMinSize(wxSize(-1, h - minEBSize));
			Options.SetInt(AudioBoxHeight, h - minEBSize);
		}
	}

	event.Skip();

}

void EditBox::HideControls()
{
	bool state = grid->subsFormat < SRT;

	Ban->Enable(state);
	Bcol2->Enable(state);
	Bcol3->Enable(state);
	Bcol4->Enable(state);
	Comment->Enable(state);
	LayerEdit->Enable(state);
	StyleChoice->Enable(state);
	StyleEdit->Enable(state);
	ActorEdit->Enable(state);
	MarginLEdit->Enable(state);
	MarginREdit->Enable(state);
	MarginVEdit->Enable(state);
	EffectEdit->Enable(state);

	state = grid->subsFormat < SRT || grid->subsFormat == MDVD;
	Bcol1->Enable(state);
	Bfont->Enable(state);

	state = grid->subsFormat <= SRT || grid->subsFormat == MDVD;
	Bbold->Enable(state);

	state = grid->subsFormat <= SRT;
	Bund->Enable(state);
	Bstrike->Enable(state);

	state = grid->subsFormat != TMP;
	EndEdit->Enable(state);
	DurEdit->Enable(state);
	Bital->Enable(state);
	if (state){ BoxSizer4->Layout(); }
}

void EditBox::ClearErrs()
{
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++)
	{
		nb->Page(i)->Grid->SpellErrors.clear();
	}
	grid->Refresh(false);
}

void EditBox::OnSplit(wxCommandEvent& event)
{
	wxString Splitchar = (grid->subsFormat <= SRT) ? "\\N" : "|";
	bool isOriginal = (grid->hasTLMode && TextEdit->GetValue() == "" && !TextEdit->HasFocus());
	//Editor
	TextEditor *tedit = (isOriginal) ? TextEditOrig : TextEdit;
	wxString txt = tedit->GetValue();
	long strt, ennd;
	tedit->GetSelection(&strt, &ennd);
	if (strt > 0 && txt[strt - 1] == ' '){ strt--; }
	if (ennd < (int)txt.Len() && txt[ennd] == ' '){ ennd++; }

	if (strt != ennd){ txt.Remove(strt, ennd - strt); }
	txt.insert(strt, Splitchar);
	tedit->SetTextS(txt, true);
	long whre = strt + Splitchar.Len();
	tedit->SetSelection(whre, whre);
}

void EditBox::OnHideOriginal(wxCommandEvent& event)
{
	wxString texttl = TextEditOrig->GetValue();
	texttl = "{" + texttl + "}";
	TextEdit->SetFocus();
	TextEditOrig->SetTextS(texttl, true);
}

void EditBox::OnPasteDifferents(wxCommandEvent& event)
{
	if (Notebook::GetTab()->Video->GetState() == None){ wxBell(); return; }
	int idd = event.GetId();
	int vidtime = Notebook::GetTab()->Video->Tell();
	if (vidtime < line->Start.mstime || vidtime > line->End.mstime){ wxBell(); return; }
	int diff = (idd == StartDifference) ? vidtime - ZEROIT(line->Start.mstime) : abs(vidtime - ZEROIT(line->End.mstime));
	long poss, pose;
	TextEdit->GetSelection(&poss, &pose);
	wxString kkk;
	kkk << diff;
	TextEdit->Replace(poss, pose, kkk);
	int npos = poss + kkk.Len();
	TextEdit->SetSelection(npos, npos);
}
//znajduje tagi w polu tekstowym
//w wyszukiwaniu nie używać // a także szukać tylko do końca taga, nie do następnego taga
bool EditBox::FindVal(const wxString &tag, wxString *Found, const wxString &text, bool *endsel, bool fromStart)
{
	lasttag = tag;
	long from = 0, to = 0;
	bool brkt = true;
	bool inbrkt = true;
	bool fromOriginal = false;
	wxString txt;
	if (text == ""){
		txt = TextEdit->GetValue();
		if (grid->hasTLMode && txt == ""){
			fromOriginal = true;
			txt = TextEditOrig->GetValue();
		}
	}
	else{ txt = text; }
	if (txt == ""){ Placed.x = 0; Placed.y = 0; InBracket = false; cursorpos = 0; if (endsel){ *endsel = false; } return false; }
	if (grid->file->SelectionsSize() < 2){
		TextEditor *Editor = (fromOriginal) ? TextEditOrig : TextEdit;
		if (!fromStart){ Editor->GetSelection(&from, &to); }
	}

	if (endsel && from == to){ *endsel = false; }
	wxRegEx rex("^" + tag, wxRE_ADVANCED);


	int klamras = txt.SubString(0, from).Find('{', true);
	int klamrae = txt.SubString(0, (from - 2 < 1) ? 1 : (from - 2)).Find('}', true);
	if (klamras == -1 || (klamras < klamrae&&klamrae != -1)){ InBracket = false; inbrkt = false; klamrae = from; brkt = false; }
	else{
		InBracket = true;
		int tmpfrom = from - 2;
		do{
			klamrae = txt.find('}', (tmpfrom < 1) ? 1 : tmpfrom);
			tmpfrom = klamrae + 1;
		} while (klamrae != -1 && klamrae < (int)txt.Len() - 1 && txt[klamrae + 1] == '{');
		if (klamrae < 0){ klamrae = txt.Len() - 1; }
	}

	Placed.x = klamrae;
	Placed.y = klamrae;
	if (endsel && *endsel){
		cursorpos = to;
		if (InBracket){ cursorpos--; }
	}
	else{
		cursorpos = klamrae;
	}
	bool isT = false;
	bool firstT = false;
	bool hasR = false;
	int endT;
	int lslash = endT = klamrae + 1;
	int lastTag = -1;
	wxString found[2];
	wxPoint fpoints[2];
	if (klamrae == txt.Len()){ klamrae--; }

	for (int i = klamrae; i >= 0; i--){
		wxUniChar ch = txt[i];
		if (ch == '\\' && brkt){
			if (lastTag < 0){ lastTag = lslash; }
			wxString ftag = txt.SubString(i + 1, lslash - 1);
			if (ftag == "r"){
				hasR = true;
			}
			if (ftag.EndsWith(")")){
				if (ftag.Find('(') == -1 || ftag.Freq(')') >= 2 || ftag.StartsWith("t(")){
					isT = true;
					endT = lslash - 1;
				}
			}
			if (ftag.StartsWith("t(")){

				if (i <= from && from < endT){

					if (found[1] != "" && fpoints[1].y <= endT){
						Placed = fpoints[1]; *Found = found[1]; return true;
					}
					else if (found[0] != ""){
						if (fpoints[0].y <= endT){ break; }
					}
					else{
						Placed.x = endT; Placed.y = Placed.x; InBracket = true; return false;
					}

				}
				isT = false;
				lslash = i;
				continue;
			}

			int reps = rex.ReplaceAll(&ftag, "\\1");
			if (reps > 0){

				if ((ftag.EndsWith(")") && !ftag.StartsWith("(")) || ftag.EndsWith("}")){ ftag.RemoveLast(1); lslash--; }

				if (found[0] == ""&&!isT){ found[0] = ftag; fpoints[0].x = i; fpoints[0].y = lslash - 1; }
				else{ found[1] = ftag; fpoints[1].x = i; fpoints[1].y = lslash - 1; }
				if (!isT && found[0] != ""){
					break;
				}
			}

			lslash = i;
		}
		else if (ch == '{' && i > 0){
			wxString textBeforeBracket = txt.SubString(0, i - 1);
			int startBracket = textBeforeBracket.Find('{', true);
			int endBracket = textBeforeBracket.Find('}', true);
			if (endBracket >= startBracket){
				brkt = false;
				if (txt[i - 1] != '}'){ inbrkt = false; if (hasR){ break; } }
			}
			else{
				lslash = i - 1;
			}
		}
		else if (ch == '}' && i > 0){
			wxString textBeforeBracket = txt.SubString(0, i - 1);
			int startBracket = textBeforeBracket.Find('{', true);
			int endBracket = textBeforeBracket.Find('}', true);
			if (endBracket < startBracket){
				lslash = i;
				brkt = true;
			}
		}

	}

	if (!isT && found[0] != ""){
		if (inbrkt){ Placed = fpoints[0]; } *Found = found[0]; return true;
	}
	else if (lastTag >= 0 && InBracket){
		Placed.x = lastTag;
		Placed.y = lastTag;
	}



	return false;
}


void EditBox::OnEdit(wxCommandEvent& event)
{
	//subs preview will switch grid to preview grid that's why we need to change its video, we dont want to change subtitles
	TabPanel* panel = (TabPanel*)grid->GetParent();
	bool startEndFocus = StartEdit->HasFocus() || EndEdit->HasFocus();
	bool durFocus = DurEdit->HasFocus();

	bool visible = true;
	if (startEndFocus){
		line->End = EndEdit->GetTime(2);
		line->Start = StartEdit->GetTime(1);
		if (line->Start > line->End){
			if (StartEdit->HasFocus()){
				StartEdit->SetForegroundColour(WindowWarningElements);
				StartEdit->changedBackGround = true;
			}
			else{
				EndEdit->SetForegroundColour(WindowWarningElements);
				EndEdit->changedBackGround = true;
			}
		}
		else if (StartEdit->changedBackGround){
			StartEdit->SetForegroundColour(WindowText);
		}
		else if (EndEdit->changedBackGround){
			EndEdit->SetForegroundColour(WindowText);
		}

		STime durTime = line->End - line->Start;
		if (durTime.mstime < 0){ durTime.mstime = 0; }
		DurEdit->SetTime(durTime, false, 1);
	}
	else if (durFocus){
		line->End = line->Start + DurEdit->GetTime();
		EndEdit->SetTime(line->End, false, 2);
		EndEdit->MarkDirty();
	}
	if (durFocus || startEndFocus){
		if (ABox && ABox->IsShown()){ ABox->audioDisplay->SetDialogue(line, currentLine); }
		UpdateChars((TextEditOrig->IsShown() && line->TextTl != "") ? line->TextTl : line->Text);
	}

	int saveAfter = Options.GetInt(GridSaveAfterCharacterCount);
	if (saveAfter && EditCounter >= saveAfter){
		Send(EDITBOX_LINE_EDITION, false, false, true);
		if (hasPreviewGrid){
			TabPanel* thisTab = (TabPanel*)GetParent();
			thisTab->Grid->RefreshPreview();
		}
		EditCounter = 1;
		if (ABox && ABox->audioDisplay->hasKara && event.GetId()>0)
			ABox->audioDisplay->SetDialogue(line, currentLine);
	}
	else{ EditCounter++; }

	if (hasPreviewGrid)
		return;

	if (Visual > 0){
		panel->Video->SetVisual(false, true);
		return;
	}

	wxString *text = NULL;
	if (panel->Video->GetState() != None){
		//visible=true;
		text = grid->GetVisible(&visible);
		if (!visible && lastVisible != visible){ visible = true; lastVisible = false; }
		else{ lastVisible = visible; }
		//make sure that dummy edition is true when line is not visible
		if (!visible)
			panel->Video->hasDummySubs = true;
	}

	if (visible && (panel->Video->IsShown() || panel->Video->isFullscreen)){
		panel->Video->OpenSubs(text);
		if (Visual > 0){ panel->Video->ResetVisual(); }
		else if (panel->Video->GetState() == Paused){ panel->Video->Render(); }
	}
	else if (text){ delete text; }

}

void EditBox::OnColorChange(ColorEvent& event)
{
	AssColor choosenColor = event.GetColor();
	wxString choosenColorAsString = choosenColor.GetAss(false, true);
	if (grid->subsFormat < SRT){
		int intColorNumber = event.GetColorType();
		wxString colorNumber;
		colorNumber << intColorNumber;
		wxString colorString;
		wxString tag = (colorNumber == "1") ? "?c&(.*)&" : "c&(.*)&";
		Styles *style = grid->GetStyle(0, line->Style);
		AssColor col = (colorNumber == "1") ? style->PrimaryColour :
			(colorNumber == "2") ? style->SecondaryColour :
			(colorNumber == "3") ? style->OutlineColour :
			style->BackColour;

		FindVal(colorNumber + tag, &colorString);
		if (colorString != choosenColorAsString){
			PutinText("\\" + colorNumber + "c" + choosenColorAsString + "&", false);
		}

		if (FindVal(L"(" +colorNumber + L"a&|alpha.*)", &colorString)){
			if (colorString.StartsWith(colorNumber + "a&"))
				colorString = colorString.Mid(2);
			else{
				colorString = colorString.Mid(5);
				Placed.y++;
				Placed.x = Placed.y;
			}
			col.SetAlphaString(colorString);
		}

		if (col.a != choosenColor.a){
			PutinText("\\" + colorNumber + wxString::Format("a&H%02X&", choosenColor.a), false);
		}

	}
	else{ PutinNonass("C:" + choosenColorAsString.Mid(2), "C:([^}]*)"); }
	//OnEdit(wxCommandEvent());
}

void EditBox::OnButtonTag(wxCommandEvent& event)
{
	wxString type;
	wxString tag = Options.GetString((CONFIG)(event.GetId() - EDITBOX_TAG_BUTTON1 + EditboxTagButton1)).BeforeFirst('\f', &type);
	if (tag.IsEmpty()){ wxBell(); return; }
	type = type.BeforeFirst('\f');

	if (type != "2"){
		//if(type=="1"){TextEdit->SetSelection(0,0);}
		if (!tag.StartsWith("\\")){ tag.Prepend("\\"); }
		wxString delims = "1234567890-&()[]";
		bool found = false;
		wxString findtag;
		for (int i = 2; i < (int)tag.Len(); i++)
		{
			if (delims.Find(tag[i]) != -1)
			{
				found = true;
				findtag = tag.SubString(1, i - 1);
				break;
			}
		}
		if (!found){ findtag = tag.AfterFirst('\\'); }
		wxString result;
		//fn and r do not have number value we have to treat it special \r works good only when return itself as a value
		//we did not need this value at all.
		wxString pattern = (findtag.StartsWith("fn")) ? L"fn(.*)" : (findtag.StartsWith("r")) ? L"(r.*)" : findtag + "([0-9\\(&-].*)";
		FindVal(pattern, &result, "", 0, type == "1");

		PutinText(tag);
	}
	else{
		bool oneline = (grid->file->SelectionsSize() < 2);

		long from, to;
		wxString txt = TextEdit->GetValue();
		TextEditor *Editor = TextEdit;
		if (grid->hasTLMode && txt == ""){ txt = TextEditOrig->GetValue(); Editor = TextEditOrig; }
		Editor->GetSelection(&from, &to);
		if (oneline){
			if (from != to){
				txt.erase(txt.begin() + from, txt.begin() + to);
			}
			int klamras = txt.Mid(from).Find('{');
			int klamrae = txt.Mid(from).Find('}');

			if (klamrae != -1 && (klamras == -1 || klamras > klamrae) && klamras<from && klamrae>from){
				from += klamrae + 1;
			}
			txt.insert(from, tag);
			from += tag.Len();
			Editor->SetTextS(txt, true);
			Editor->SetSelection(from, from);
		}
		else{
			wxArrayInt sels;
			grid->file->GetSelectionsAsKeys(sels);
			for (size_t i = 0; i < sels.size(); i++){
				long cpyfrom = from;
				Dialogue *dialc = grid->CopyDialogueByKey(sels[i]);
				wxString &txt = dialc->Text.CheckTlRef(dialc->TextTl, grid->hasTLMode && dialc->TextTl != "");
				int klamras = txt.Mid(from).Find('{');
				int klamrae = txt.Mid(from).Find('}');

				if (klamrae != -1 && (klamras == -1 || klamras > klamrae) && klamras<from && klamrae>from){
					cpyfrom += klamrae + 1;
				}
				if (cpyfrom >= txt.size())
					cpyfrom = txt.size();
				txt.insert(cpyfrom, tag);
			}
			grid->SetModified(EDITBOX_MULTILINE_EDITION);
			grid->Refresh(false);

		}

	}

}

class NumTagButtons : public KaiDialog
{
public:
	NumTagButtons(wxWindow *parent)
		:KaiDialog(parent, -1, _("Zmień ilość przycisków"))
	{
		DialogSizer *sizer = new DialogSizer(wxVERTICAL);
		wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
		numTagButtons = new NumCtrl(this, -1, Options.GetString(EditboxTagButtons), 0, 10, true);
		MappedButton *ok = new MappedButton(this, wxID_OK, "OK");
		MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
		buttonSizer->Add(ok, 1, wxEXPAND | wxALL, 4);
		buttonSizer->Add(cancel, 1, wxEXPAND | wxALL, 4);
		sizer->Add(numTagButtons, 1, wxEXPAND | wxALL, 4);
		sizer->Add(buttonSizer, 1, wxEXPAND, 0);
		sizer->SetMinSize(wxSize(200, -1));
		SetSizerAndFit(sizer);
		MoveToMousePosition(this);
	}

	NumCtrl *numTagButtons;
};

void EditBox::OnEditTag(wxCommandEvent &event)
{
	int id = event.GetId();
	if (id == 16000){
		NumTagButtons ntb(this);
		if (ntb.ShowModal() == wxID_CANCEL){ return; }
		Options.SetInt(EditboxTagButtons, ntb.numTagButtons->GetInt());
		SetTagButtons();
		return;
	}
	wxWindow *win = FindWindow(id);
	if (!win){ return; }
	TagButton *tb = (TagButton*)win;
	if (!tb){ return; }

	txtdialog tagtxt(tb, -1, tb->tag, tb->name, tb->type);

	if (tagtxt.ShowModal() == wxID_OK){
		tb->tag = tagtxt.txt->GetValue();
		tb->type = tagtxt.type->GetSelection();
		wxString newname = tagtxt.name->GetValue();
		if (newname != tb->name){
			tb->SetLabelText(newname); tb->name = newname;
			Menu *menu = TagButtonManager->GetMenu();
			MenuItem *item = menu->FindItem(id);
			if (item){
				item->label = newname;
			}
		}
		wxString svtag = tb->tag;
		Options.SetString((CONFIG)(id - EDITBOX_TAG_BUTTON1 + EditboxTagButton1), svtag << "\f" << tb->type << "\f" << tb->name);
		Options.SaveOptions(true, false);
		if (tb->tag != ""){ tb->SetToolTip(tb->tag); }
	}

}

void EditBox::OnAutoMoveTags(wxCommandEvent& event)
{
	SetTextWithTags(true);
	Options.SetBool(AutoMoveTagsFromOriginal, AutoMoveTags->GetValue());
	Options.SaveOptions();
}

void EditBox::SetTextWithTags(bool RefreshVideo)
{
	if (grid->hasTLMode && line->TextTl == "" && AutoMoveTags->GetValue()){
		wxString Text = line->Text;
		Text.Replace("}{", "");
		int getr = Text.Find('}');
		if (getr > -1){
			int brackets = Text.find("{");
			wxString restText;
			if (Text.Len() > (size_t)getr + 1){ restText = Text.Mid(getr + 1); }
			int pos = 0;
			wxString txtOrg;
			wxString txtTl;
			if (Text.StartsWith("{")){
				txtTl = Text.substr(0, getr + 1);
				pos = txtTl.Len();
			}
			else if (brackets > 0){
				txtOrg = Text.substr(0, brackets - 1);
				txtTl = Text.SubString(brackets, getr);
			}
			else{
				txtOrg = Text.substr(0, getr + 1);
			}

			while (1){
				brackets = restText.find("{");
				getr = restText.Find('}');
				if (brackets != -1 && getr != -1){
					txtOrg += restText.substr(0, brackets);
					txtTl += restText.SubString(brackets, getr);
					if (restText.Len() > (size_t)getr + 1){ restText = restText.Mid(getr + 1); }
					else{ break; }
				}
				else{
					txtOrg += restText;
					break;
				}
			}



			TextEdit->SetTextS(txtTl, TextEdit->IsModified(), true);
			TextEditOrig->SetTextS(txtOrg, TextEditOrig->IsModified(), true);
			splittedTags = true;

			TextEdit->SetSelection(pos, pos);
			TextEdit->SetFocus();
			goto done;
		}
	}
	if (splittedTags){ delete line; line = grid->GetDialogue(currentLine)->Copy(); }
	splittedTags = false;
	TextEdit->SetTextS((TextEditOrig->IsShown()) ? line->TextTl : line->Text, TextEdit->IsModified(), true);
	if (TextEditOrig->IsShown()){ TextEditOrig->SetTextS(line->Text, TextEditOrig->IsModified(), true); }
done:
	if (RefreshVideo){
		VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
		if (vb->GetState() != None){
			vb->OpenSubs(grid->GetVisible());
			vb->Render();
		}
	}
}

void EditBox::OnCursorMoved(wxCommandEvent& event)
{
	if (Visual == SCALE || Visual == ROTATEZ || Visual == ROTATEXY || Visual == CLIPRECT){
		//here grid has nothing to do if someone would want to make effect by visual on this video, 
		//he can without problems
		TabPanel* pan = (TabPanel*)GetParent();
		pan->Video->ResetVisual();
	}
}

void EditBox::OnChangeTimeDisplay(wxCommandEvent& event)
{
	bool frame = Frames->GetValue();
	grid->ChangeTimeDisplay(frame);
	wxDELETE(line);
	line = grid->GetDialogue(currentLine)->Copy();
	StartEdit->ShowFrames(frame);
	EndEdit->ShowFrames(frame);
	DurEdit->ShowFrames(frame);
	StartEdit->SetTime(line->Start, false, 1);
	EndEdit->SetTime(line->End, false, 2);
	if (frame){
		STime durationTime = EndEdit->GetTime() - StartEdit->GetTime();
		durationTime.orgframe++;
		DurEdit->SetTime(durationTime);
	}
	else{
		DurEdit->SetTime(line->End - line->Start);
	}

	grid->RefreshColumns(START | END);
	Options.SetBool(EDITBOX_TIMES_TO_FRAMES_SWITCH, frame);
}

bool EditBox::SetBackgroundColour(const wxColour &col)
{
	if (ABox){ ABox->SetBackgroundColour(col); }
	wxWindow::SetBackgroundColour(col);
	return true;
}

//shows style editing window without style manager, window is non modal.
void EditBox::OnStyleEdit(wxCommandEvent& event)
{
	StyleStore::ShowStyleEdit();
}

bool EditBox::IsCursorOnStart()
{
	if (grid->file->SelectionsSize() > 1){ return true; }
	/*if(Visual == CLIPRECT || Visual == MOVE){return true;}
	wxString txt=TextEdit->GetValue();
	MTextEditor *Editor = TextEdit;
	if(grid->transl && txt=="" ){
	txt = TextEditOrig->GetValue();
	Editor = TextEditOrig;
	}
	long from=0, to=0;
	Editor->GetSelection(&from, &to);
	if(from == 0 || txt.StartsWith("{")){
	txt.Replace("}{","");
	int endBracket = txt.Find('}');
	if(endBracket == -1 || endBracket <= from+1){
	return true;
	}
	}*/
	return false;
}


void EditBox::OnDoubtfulTl(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
	line->ChangeState(4);
	wxArrayInt sels;
	grid->file->GetSelectionsAsKeys(sels);
	for (size_t i = 0; i < sels.size(); i++){
		Dialogue *dial = grid->file->CopyDialogueByKey(sels[i]);
		dial->ChangeState(4);
	}
	if (event.GetId() == SetDoubtful){
		grid->NextLine();
	}
	else{
		grid->Refresh(false);
	}
}

void EditBox::FindNextDoubtfulTl(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
SeekDoubtful:
	for (int i = CurrentDoubtful; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (dial->IsDoubtful()){
			SetLine(i);
			grid->SelectRow(i);
			grid->ScrollTo(i, true);
			CurrentDoubtful = i + 1;
			return;
		}
	}
	if (CurrentDoubtful == 0){ KaiMessageBox(_("Nie znaleziono niepewnych")); return; }
	CurrentDoubtful = 0;
	goto SeekDoubtful;
}

void EditBox::FindNextUnTranslated(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
SeekUntranslated:
	for (int i = CurrentUntranslated; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (dial->TextTl == ""/* && !dial->IsComment*/){
			SetLine(i);
			grid->SelectRow(i);
			grid->ScrollTo(i, true);
			CurrentUntranslated = i + 1;
			return;
		}
	}
	if (CurrentUntranslated == 0){ KaiMessageBox(_("Nie znaleziono nieprzetłumaczonych")); return; }
	CurrentUntranslated = 0;
	goto SeekUntranslated;
}

void EditBox::SetTagButtons()
{
	//dziesięć przycisków + nasz ostatni ze strzałką
	int numofButtons = BoxSizer4->GetItemCount() - 11;
	int numTagButtons = Options.GetInt(EditboxTagButtons);
	if (numTagButtons > numofButtons){
		Menu *menu = new Menu();
		for (int i = 0; i < numTagButtons; i++)
		{
			wxArrayString tagOption;
			Options.GetTable((CONFIG)(i + EditboxTagButton1), tagOption, "\f", wxTOKEN_RET_EMPTY_ALL);
			wxString name;
			wxString tag;
			int type = 0;
			if (tagOption.size() > 2){ name = tagOption[2]; }
			else{ name = wxString::Format("T%i", i + 1); }
			if (tagOption.size() > 1){
				type = wxAtoi(tagOption[1]);
			}
			if (tagOption.size() > 0){
				tag = tagOption[0];
			}
			if (i >= numofButtons){
				if (!TagButtonManager){
					BoxSizer4->Add(new TagButton(this, EDITBOX_TAG_BUTTON1 + i, name, tag, type, wxSize((name.Len()) > 2 ? -1 : 24, 24)), 0, wxALL, 2);
				}
				else if (i >= numofButtons){
					BoxSizer4->Insert(10 + i, new TagButton(this, EDITBOX_TAG_BUTTON1 + i, name, tag, type, wxSize((name.Len()) > 3 ? -1 : 24, 24)), 0, wxALL, 2);
				}
				Connect(EDITBOX_TAG_BUTTON1 + i, TAG_BUTTON_EDITION, (wxObjectEventFunction)&EditBox::OnEditTag);
				Connect(EDITBOX_TAG_BUTTON1 + i, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnButtonTag);
			}
			menu->Append(EDITBOX_TAG_BUTTON1 + i, name);
		}
		menu->Append(16000, _("Zmień ilość przycisków"));
		if (!TagButtonManager){
			TagButtonManager = new MenuButton(this, -1, _("Zarządzaj przyciskami tagów"), wxDefaultPosition, wxSize(23, 24));
			BoxSizer4->Add(TagButtonManager, 0, wxALIGN_CENTER | wxALL, 2);
			Connect(16000, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnEditTag);
		}
		TagButtonManager->PutMenu(menu);
		Layout();
	}
	else{
		Menu *menu = TagButtonManager->GetMenu();
		for (int i = numTagButtons; i < numofButtons; i++)
		{
			menu->Delete(numTagButtons);
			wxSizerItem *item = BoxSizer4->GetItem(numTagButtons + 10);
			wxWindow *win = item->GetWindow();
			BoxSizer4->Remove(numTagButtons + 10);
			win->Destroy();
		}
		Layout();
	}




}

void EditBox::SetActiveLineToDoubtful()
{
	CurrentDoubtful = currentLine;
	CurrentUntranslated = currentLine;
};

void EditBox::RebuildActorEffectLists()
{
	ActorEdit->Clear();
	EffectEdit->Clear();
	for (int i = 0; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (!dial->Actor.empty() && ActorEdit->FindString(dial->Actor, true) < 0){
			ActorEdit->Append(dial->Actor);
		}
		if (!dial->Effect.empty() && EffectEdit->FindString(dial->Effect, true) < 0){
			EffectEdit->Append(dial->Effect);
		}
	}
	ActorEdit->Sort();
	EffectEdit->Sort();
}

void EditBox::SetGrid(SubsGrid *_grid, bool isPreview){
	if (grid != _grid){
		grid = _grid;
		hasPreviewGrid = isPreview;
		RebuildActorEffectLists();
		RefreshStyle();
	}
}