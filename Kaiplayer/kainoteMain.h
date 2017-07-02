/***************************************************************
 * Name:      kainoteMain.h
 * Purpose:   Defines Application Frame
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
 * License:

 * Kainote is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Kainote is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

 **************************************************************/

#pragma once

#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include "KaiFrame.h"
#include "Tabs.h"
#include <vector>
#include "TabPanel.h"
#include "SpellChecker.h"
#include "StyleStore.h"
#include "FindReplace.h"
#include "SelectLines.h"
#include "Automation.h"
#include "Toolbar.h"
#include "KaiStatusBar.h"
class FontCollector;

class kainoteFrame: public KaiFrame
{
    public:
        kainoteFrame(const wxPoint &pos, const wxSize &size);
        virtual ~kainoteFrame();

		Notebook* Tabs;

		wxBoxSizer *mains;
        void Save(bool dial, int wtab=-1, bool changeLabel = true);
		void SaveAll();

        bool OpenFile(wxString filename,bool fulls=false);
		void Label(int iter=0,bool video=false, int wtab=-1);
		
		void SetRecent(short what=0);
		void AppendRecent(short what=0,Menu *Menu=0);
		void SetAccels(bool all=true);
		wxString FindFile(wxString fn,bool video,bool prompt=true);

		Menu* VidsRecMenu;
		Menu* SubsRecMenu;
		Menu* AudsRecMenu;
		MenuBar* Menubar;

		KaiStatusBar* StatusBar;
		KaiToolbar *Toolbar;
		wxArrayString subsrec;
		wxArrayString videorec;
		wxArrayString audsrec;
		void OnMenuSelected(wxCommandEvent& event);
		void OnMenuSelected1(wxCommandEvent& event);
		void OnRecent(wxCommandEvent& event);

		TabPanel* GetTab();
		
		void InsertTab(bool sel=true);
		void OpenFiles(wxArrayString,bool intab=false, bool nofreeze=false, bool newtab=false);
		void OpenAudioInTab(TabPanel *pan, int id, const wxString &path);
		void HideEditor();
		bool SavePrompt(char mode=1, int wtab=-1);
		void UpdateToolbar();
		void OnOpenAudio(wxCommandEvent& event);
		void OnMenuClick(wxCommandEvent &event);
		void SetStatusText(const wxString &label, int field){StatusBar->SetLabelText(field, label);}
		wxString GetStatusText(int field){return StatusBar->GetStatusText(field);}
		void SetSubsResolution(bool dialog=false);
		void SetVideoResolution(int w, int h, bool dialog=false);
		void ShowBadResolutionDialog(const wxSize &videoRes, const wxSize &subsRes);
		FindReplace *FR;
		SelectLines *SL;
		Auto::Automation *Auto;
		FontCollector *fc;
    private:

		
        void OnConversion(char form);
        void OnAssProps();
		void OnMenuOpened(MenuEvent& event);
		void OnP5Sec(wxCommandEvent& event);
		void OnM5Sec(wxCommandEvent& event);
		void OnAudioSnap(wxCommandEvent& event);
		void OnPageChanged(wxCommandEvent& event);
		void OnPageChange(wxCommandEvent& event);
		void OnPageAdd(wxCommandEvent& event);
		void OnPageClose(wxCommandEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnRunScript(wxCommandEvent& event);
		void OnChangeLine(wxCommandEvent& event);
		void OnDelete(wxCommandEvent& event);
		void OnClose1(wxCloseEvent& event);
		static void OnOutofMemory();
        Menu* ConvMenu;
        Menu* FileMenu;
		Menu* HelpMenu;
        Menu* SubsMenu;
		Menu* EditMenu;
		Menu* VidMenu;
		Menu* AudMenu;
		Menu* ViewMenu;
		Menu* AutoMenu;
		
		wxLogWindow *mylog;
		bool badResolution;
		/*int fontlastmodif;
		int fontlastmodifl;*/
		
};




enum{
	ID_ADDPAGE=6900,
	ID_CLOSEPAGE,
	ID_TABS,
	ID_STATUSBAR1,
	ID_CONV
};

