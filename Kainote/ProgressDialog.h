//  Copyright (c) 2016 - 2026, Marcin Drob

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

#include "MappedButton.h"
#include <wx/dialog.h>
#include <functional>


struct ITaskbarList3;
class KaiGauge;
class KaiStaticText;

class ProgresDialog : public wxDialog
{
private:
	KaiGauge *gauge;
	KaiStaticText *text;
	KaiStaticText *text1;

	bool canceled;
	int oldtime;
	void OnCancel(wxCommandEvent& event);
	void OnShow(wxThreadEvent& evt);
	void OnProgress(wxThreadEvent& evt);
	void OnTitle(wxThreadEvent& evt);
	int firsttime;

public:
	ProgresDialog(wxWindow *parent, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~ProgresDialog();

	void Progress(int num);
	void Title(wxString title);
	bool WasCancelled();
	MappedButton *cancel;
	ITaskbarList3 *taskbar;
	int result;
};

class Sink 
{
public:
	Sink() {};
	virtual ~Sink() {};
	virtual void Title(wxString title){};
	virtual bool WasCancelled() { return false; };
	virtual void Progress(int num) {};
	virtual void EndModal() {};
};
//sink for show progress dialog and blocking
//main window till task is ended
class ProgressSink : public Sink, public wxThread
{
public:
	ProgressSink(wxWindow *parent, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~ProgressSink();
	//First make ProgressSink
	//set or create thread
	//show dialog from main thread
	//and then wait for make task / thread
	//in other thread can be used only Progress, 
	//WasCancelled, Title, ShowSecondaryDialog and EndModal()
	void SetAndRunTask(std::function<int()> _task)
	{
		task = _task;
		Create();
		Run();
	}
	void ShowDialog();
	int ShowSecondaryDialog(std::function<int()> dialfunction);
	void Title(wxString title) override;
	bool WasCancelled() override;
	void Progress(int num) override;
	void EndModal() override;
private:
	wxThread::ExitCode Entry();
	std::function<int()> task;
	ProgresDialog *dlg;

};

class KainoteFrame;

//sink for showing progres only on statusbar
//and taskbar not blocking main window
class ProgressSinkSilent : public Sink, public wxEvtHandler
{
public:
	//use constructor
	//use Title, WasCancelled, Progress, from other thread
	//use EndModal from main thread
	ProgressSinkSilent(KainoteFrame* _parent, const wxString& title);
	virtual ~ProgressSinkSilent();
	
	void Title(wxString _title) override;
	bool WasCancelled() override;
	void Progress(int num) override;
	void EndModal() override;
	void OnProgress(wxThreadEvent& event);
	void OnTitle(wxThreadEvent& event);
	void OnEndModal(wxThreadEvent& event);
private:
	ITaskbarList3* taskbar = nullptr;
	KainoteFrame* parent;
	int oldTime = 0;
	int firstTime = 0;
	wxString title;
};


