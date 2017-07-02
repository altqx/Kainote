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


#include "SubsFile.h"
#include "KaiListCtrl.h"
#include "MappedButton.h"

//pami�taj ilo�� element�w tablicy musi by� r�wn ilo�ci enum�w
wxString historyNames[] = {
	"",//pierwszy element kt�rego nie u�ywamy a musi by� ostatni enum weszed� a tak�e ochroni� nas przed potencjalnym 0
	_("Otwarcie napis�w"),
	_("Nowe napisy"),
	_("Edycja linii"),
	_("Edycja wielu linii"),
	_("Poprawa b��du pisowni w polu tekstowym"),
	_("Powielenie linijek"),
	_("Po��czenie linijek"),
	_("Po��czenie linijki z poprzedni�"),
	_("Po��czenie linijki z nast�pn�"),
	/*10*/_("Po��czenie linijek pozostawienie pierszej"),
	_("Po��czenie linijek pozostawienie ostatniej"),
	_("Wklejenie linijek"),
	_("Wklejenie kolumn"),
	_("Wklejenie t�umaczenia"),
	_("Przesuni�cie tekstu t�umaczenia"),
	_("Ustawienie czas�w linii jako ci�g�ych"),
	_("Ustawienie FPSu obliczonego z wideo"),
	_("Ustawienie w�asnego FPSu"),
	_("Zamiana linijek"),
	/*20*/_("Konwersja napis�w"),
	_("Sortowanie napis�w"),
	_("Usuni�cie linijek"),
	_("Usuni�cie tekstu"),
	_("Ustawienie czasu pocz�tkowego"),
	_("Ustawienie czasu ko�cowego"),
	_("W��czenie trybu t�umaczenia"),
	_("Wy��czenie trybu t�umaczenia"),
	_("Dodanie nowej linii"),
	_("Wstawienie linii"),
	/*30*/_("Zmiana czasu na wykresie audio"),
	_("Przyklejenie do klatki kluczowej"),
	_("Zmiana nag��wku napis�w"),
	_("Akcja zaznacz linijki"),
	_("Przesuni�cie czas�w"),
	_("Poprawa b��du pisowni"),
	_("Edycja styl�w"),
	_("Zmiana rozdzielczo�ci napis�w"),
	_("Narz�dzie pozycjonowania"),
	_("Narz�dzie ruchu"),
	/*40*/_("Narz�dzie skalowania"),
	_("Narz�dzie obrot�w w osi Z"),
	_("Narz�dzie obrot�w w osiach X i Y"),
	_("Narz�dzie wycin�w prostok�tnych"),
	_("Narz�dzie wycin�w wektorowych"),
	_("Narz�dzie rysunk�w wektorowych"),
	_("Narz�dzie zmieniacz pozycji"),
	_("Zamie�"),
	_("Zamie� wszystko"),
	_("Skrypt automatyzacji"),
};

HistoryDialog::HistoryDialog(wxWindow *parent, SubsFile *file, std::function<void(int)> func )
	: KaiDialog(parent, -1, _("Historia"),wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	wxArrayString history;
	file->GetHistoryTable(&history);
	KaiListCtrl *HistoryList = new KaiListCtrl(this, ID_HISTORY_LIST, history);
	//HistoryList->ScrollTo(file->Iter()-2);
	Bind(LIST_ITEM_DOUBLECLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_HISTORY_LIST);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *Set = new MappedButton(this, ID_SET_HISTORY, _("Ustaw"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
	}, ID_SET_HISTORY);
	MappedButton *Ok = new MappedButton(this, ID_SET_HISTORY_AND_CLOSE, _("OK"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		func(HistoryList->GetSelection());
		Hide();
	}, ID_SET_HISTORY_AND_CLOSE);
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(Set,1,wxALL,3);
	buttonSizer->Add(Ok,1,wxALL,3);
	buttonSizer->Add(Cancel,1,wxALL,3);
	main->Add(HistoryList,1,wxEXPAND|wxALL,3);
	main->Add(buttonSizer,0, wxCENTER);
	main->SetMinSize(300,400);
	SetSizerAndFit(main);
	CenterOnParent();
	HistoryList->SetSelection(file->Iter(),true);
}


File::File()
	:etidtionType(0)
	,activeLine(0)
{
}

File::~File()
{
	dials.clear();
	styles.clear();
	sinfo.clear();
	ddials.clear();
	dstyles.clear();
	dsinfo.clear();
	//wxLogStatus("Clearing");
}
void File::Clear()
{
	
	for(std::vector<Dialogue*>::iterator it = ddials.begin(); it != ddials.end(); it++)
	{
		
		delete (*it);
		
	}
	
	for(std::vector<Styles*>::iterator it = dstyles.begin(); it != dstyles.end(); it++)
	{
		delete (*it);
		
	}
	
	for(std::vector<SInfo*>::iterator it = dsinfo.begin(); it != dsinfo.end(); it++)
	{
		delete (*it);
		
	}
}



File *File::Copy()
{
	File *file=new File();
	file->dials = dials;
	file->styles= styles;
	file->sinfo = sinfo;
	return file;
}

SubsFile::SubsFile()
{
    iter=0;
	edited=false;
	subs = new File();
}

SubsFile::~SubsFile()
{
	subs->Clear();
	delete subs;
	for(std::vector<File*>::iterator it = undo.begin(); it != undo.end(); it++)
	{
		(*it)->Clear();
		delete (*it);
	}
    undo.clear();
}


void SubsFile::SaveUndo(unsigned char editionType, int activeLine)
{
	int size=maxx();
	if(iter!=size){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
	subs->activeLine = activeLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
	iter++;
	edited=false;
}


void SubsFile::Redo()
{
    if(iter<maxx()){
		iter++;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
	}
}

void SubsFile::Undo()
{
    if(iter>0){
		iter--;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
	}
}

bool SubsFile::SetHistory(int _iter)
{
    if(_iter < undo.size() && _iter>=0){
		iter = _iter;
		subs->Clear();
		delete subs;
		subs=undo[iter]->Copy();
		return false;
	}
	return true;
}

void SubsFile::DummyUndo()
{
	subs->Clear();
	delete subs;
	subs=undo[iter]->Copy();
}

void SubsFile::DummyUndo(int newIter)
{
	if(newIter < 0 || newIter >= undo.size()){return;}
	subs->Clear();
	delete subs;
	subs=undo[newIter]->Copy();
	iter = newIter;
	if(iter < undo.size() - 1){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
}

bool SubsFile::IsNotSaved()
{
    if((subs->ddials.size()==0 && subs->dstyles.size()==0 && subs->dsinfo.size()==0 && !edited)){return false;}
    return true;
}

int SubsFile::maxx()
{
    return undo.size()-1;
}

int SubsFile::Iter()
{
    return iter;
}

Dialogue *SubsFile::CopyDial(int i, bool push, bool keepstate)
{
	Dialogue *dial=subs->dials[i]->Copy(keepstate);
	subs->ddials.push_back(dial);
	if(push){subs->dials[i]=dial;}
	return dial;
}
	
Styles *SubsFile::CopyStyle(int i, bool push)
{
	Styles *styl=subs->styles[i]->Copy();
	subs->dstyles.push_back(styl);
	if(push){subs->styles[i]=styl;}
	return styl;
}
	
SInfo *SubsFile::CopySinfo(int i, bool push)
{
	SInfo *sinf=subs->sinfo[i]->Copy();
	subs->dsinfo.push_back(sinf);
	if(push){subs->sinfo[i]=sinf;}
	return sinf;
}

void SubsFile::EndLoad(unsigned char editionType, int activeLine)
{
	subs->activeLine = activeLine;
	subs->etidtionType = editionType;
	undo.push_back(subs);
	subs=subs->Copy();
}

void SubsFile::RemoveFirst(int num)
{
	//uwaga pierwszym elementem tablicy s� napisy zaraz po wczytaniu dlatego te� nie nale�y go usuwa�
	for(std::vector<File*>::iterator it = undo.begin()+1; it != undo.begin()+num; it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.erase(undo.begin()+1, undo.begin()+num);
	iter-=(num-1);
}

void SubsFile::GetURStatus(bool *_undo, bool *_redo)
{
	*_redo= (iter<(int)undo.size()-1);
	*_undo= (iter>0);
}

File *SubsFile::GetSubs()
{
	return subs;
}

void SubsFile::GetHistoryTable(wxArrayString *history)
{
	for(size_t i = 0; i < undo.size(); i++){
		history->push_back(historyNames[undo[i]->etidtionType] + 
			wxString::Format(_(", aktywna linia %i"), undo[i]->activeLine));
	}
}

void SubsFile::ShowHistory(wxWindow *parent, std::function<void(int)> functionAfterChangeHistory)
{
	HistoryDialog HD(parent, this, functionAfterChangeHistory);
	HD.ShowModal();
}