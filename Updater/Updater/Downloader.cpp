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
	return ((Downloader*)userdata)->WriteCallback(buffer, itemsize * numitems);
}

size_t CurlHeaderCallback(const char* buffer, size_t itemsize, size_t numitems, void* userdata) {
	return ((Downloader*)userdata)->HeaderCallback(buffer, itemsize * numitems);
}

bool Downloader::SeekHeader(const std::string& what, std::string* result)
{
	size_t position = header.find(what);
	if (position != -1) {
		position += what.size();
		size_t endposition = header.find('\n', position);
		size_t endposition1 = header.find('\r', position);
		if (endposition1 != -1 && endposition1 < endposition) {
			(*result) = header.substr(position, endposition1 - position);
		}
		else//no need to check endposition if is -1 it get rest of text
			(*result) = header.substr(position, endposition - position);

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

Downloader::Downloader(const std::string& URL, const std::wstring& _outputFile, bool _saveToFile, bool getLocationFromHeader)
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
		errorMsg = L"Could not initialize CURL";
		return;
	}

	if (getLocation) {
		if (!GetHeader(curl))
			goto failed;

		if (!SeekHeader("Location: ", &url))
			goto failed;

		curl_easy_cleanup(curl);
		curl = curl_easy_init();
		if (!curl) {
			errorMsg = L"Could not initialize CURL";
			return;
		}
	}

	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEDATA, this)) {
		errorMsg = L"Could not set write callback userdata.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback)) {
		errorMsg = L"Could not set write callback function.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1)) {
		errorMsg = L"Could not set redirect following.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1)) {
		errorMsg = L"Could not disable progress callback";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1)) {
		errorMsg = L"Could not fail on error.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_URL, url.c_str())) {
		errorMsg = L"Could not set fetch url.";
		goto failed;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlError)) {
		errorMsg = L"Could not set error buffer.";
		goto failed;
	}

	CURLcode result = curl_easy_perform(curl);
	if (result) {
		if (result == CURLE_WRITE_ERROR)
			errorMsg = L"Curl write error.";
		else {
			wchar_t werror[CURL_ERROR_SIZE];
			MultiByteToWideChar(CP_UTF8, 0, curlError, CURL_ERROR_SIZE, werror, CURL_ERROR_SIZE);
			errorMsg = std::wstring(werror);
		}
	}

	Finalize();
	downloadSucceded = true;

failed:
	curl_easy_cleanup(curl);
}

bool Downloader::GetHeader(CURL* curl)
{
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_HEADERDATA, this)) {
		errorMsg = L"Could not set header callback userdata.";
		return false;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeaderCallback)) {
		errorMsg = L"Could not set header callback function.";
		return false;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1)) {
		errorMsg = L"Could not disable progress callback";
		return false;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1)) {
		errorMsg = L"Could not fail on error.";
		return false;
	}
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_URL, url.c_str())) {
		errorMsg = L"Could not set fetch url.";
		return false;
	}
	char curlError[CURL_ERROR_SIZE];
	if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlError)) {
		errorMsg = L"Could not set error buffer.";
		return false;
	}

	CURLcode result = curl_easy_perform(curl);
	if (result) {
		if (result == CURLE_WRITE_ERROR)
			errorMsg = L"Curl write error.";
		else {
			wchar_t werror[CURL_ERROR_SIZE];
			MultiByteToWideChar(CP_UTF8, 0, curlError, CURL_ERROR_SIZE, werror, CURL_ERROR_SIZE);
			errorMsg = std::wstring(werror);
		}
	}
	return true;
}

void Downloader::Finalize()
{
	if (saveToFile) {
		std::fstream outStream(outputFile, std::ios::out | std::ios::binary);
		if (outStream.fail()) {
			errorMsg = L"Couldn't open output file: " + outputFile;
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

const std::wstring& Downloader::GetError()
{
	return errorMsg;
}