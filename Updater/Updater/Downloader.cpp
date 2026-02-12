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
#include <fstream>
#include "Downloader.h"

size_t CurlWriteCallback(const char* buffer, size_t itemsize, size_t numitems, void* userdata) {
	((Downloader*)userdata)->WriteCallback(buffer, itemsize * numitems);
}

size_t CurlHeaderCallback(const char* buffer, size_t itemsize, size_t numitems, void* userdata) {
	((Downloader*)userdata)->HeaderCallback(buffer, itemsize * numitems);
}

bool Downloader::SeekHeader(const std::string& what, std::string* result)
{
	size_t position = header.find(what);
	if (position != -1) {
		position += what.size() + 1;
		size_t endposition = header.find('\n', position);
		size_t endposition1 = header.find('\r', position);
		if (endposition1 != -1 && endposition1 < endposition) {
			result->substr(position, endposition1);
		}
		else//no need to check endposition if is -1 it get rest of text
			result->substr(position, endposition);

		return true;
	}
	return false;
}

//int Downloader::ProgressCallback(curl_off_t totalDL, curl_off_t currentDL)
//{
//
//}

size_t Downloader::WriteCallback(const char* buffer, size_t size)
{
	outputBuffer.append(buffer);
	return size;
}

size_t Downloader::HeaderCallback(const char* buffer, size_t size)
{
	header.append(buffer);
	return size;
}

Downloader::Downloader(const std::string& URL, const std::string& _outputFile, bool _saveToFile, bool getLocationFromHeader)
{
	url = URL;
	outputFile = _outputFile;
	saveToFile = _saveToFile;
	getLocation = getLocationFromHeader;

	thread = std::thread(&Downloader::ThreadProcess, this);
}

Downloader::~Downloader()
{
}

void Downloader::ThreadProcess()
{
	char curlError[CURL_ERROR_SIZE];
	CURL* curl = curl_easy_init();
	if (!curl) {
		errorMsg = "Could not initialize CURL";
		return;
	}


	if (getLocation) {
		if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_HEADERDATA, this)) {
			errorMsg = "Could not set header callback userdata.";
			goto failed;
		}
		if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeaderCallback)) {
			errorMsg = "Could not set header callback function.";
			goto failed;
		}
	}

	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEDATA, this)) {
		errorMsg = "Could not set write callback userdata.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback)) {
		errorMsg = "Could not set write callback function.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1)) {
		errorMsg = "Could not set redirect following.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1)) {
		errorMsg = "Could not disable progress callback";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1)) {
		errorMsg = "Could not fail on error.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_URL, url.c_str())) {
		errorMsg = "Could not set fetch url.";
		goto failed;
	}
	
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlError)) {
		errorMsg = "Could not set error buffer.";
		goto failed;
	}

	CURLcode result = curl_easy_perform(curl);
	if (result) {
		if(result == CURLE_WRITE_ERROR)
			errorMsg = "Curl write error.";
		else
			errorMsg = std::string(curlError);
	}

	Finalize();
	downloadSucceded = true;

failed:
	curl_easy_cleanup(curl);
}

void Downloader::Finalize()
{
	if (saveToFile) {
		std::fstream outStream(outputFile, std::ios::out | std::ios::binary);
		if (outStream.fail()) {
			errorMsg = "Couldn't open output file: " + outputFile;
			downloadSucceded = false;
			return;
		}

		outStream << outputBuffer;
		outStream.close();
	}
}

bool Downloader::JoinThread()
{
	if (!threadJoined)
		thread.join();

	return downloadSucceded;
}

const std::string& Downloader::GetOutput()
{
	return outputBuffer;
}
