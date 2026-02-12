//  Copyright (c) 2018, Marcin Drob

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

#include <string>
#include <stdio.h>
#include <iostream>
#include "ZipHandler.h"
#include "Downloader.h"


using namespace std;

int wmain(int argc, wchar_t **argv)
{
	std::string url = "https://drive.google.com/uc?id=1ECqsrLo5d1jPoz-FKvJrS0279YeTKrmS&export=download";
	std::wstring path;
	if (argc) {
		path.assign(argv[0]);
		size_t position = path.find_last_of(L'\\');
		//check / slashes?
		if (position != -1) {
			path = path.substr(0, position - 1);
		}//if cannot find slash means that there is only file name
		else {
			path.clear();
		}
	}
	if (path.empty()) {
		wcout << L"Could not get program path";
		system("pause");
		return 0;
	}


	wchar_t targetfilename1[] = L"J:\\Easeus 11 22_24\\Nowy(F)\\Lost Files\\#2 Lost Partition(NTFS)\\Anime";

	wcout << L"Podaj sciezke for zero size seeking: ";
	wcout << targetfilename1;
	
	ZipHandler zh;
	//zh.ZipFolder(filesDir, zipPatch, excludes, 3);
	////zh.UnZipFile(zipPatch, targetfilename);
	size_t dirs=0;
	size_t files=0;
	zh.CheckFiles(targetfilename1, &dirs, &files, NULL);
	wcout << L"location: " << targetfilename1 << L"\n";
	wcout << L"dirs: " << dirs<<L"\n";
	wcout << L"files: " << files << L"\n";
	if (zh.found.size()){
		wcout << L"found:\n";
		wcout << zh.found;
	}
	system("pause");
	return 0;
}