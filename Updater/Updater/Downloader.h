#pragma once

#include <string>
#include <thread>
#include "curl/curl.h"


class Downloader {
private:
	std::thread thread;
	std::string url;
	std::string header;
	std::string outputBuffer;
	std::wstring outputFile;
	std::wstring errorMsg;
	bool saveToFile, getLocation;
	bool threadJoined = false;
	bool downloadSucceded = false;
	bool SeekHeader(const std::string& what, std::string *result);
	void ThreadProcess();
	bool GetHeader(CURL* curl);
	void Finalize();
public:
	Downloader(const std::string &URL, const std::wstring &outputFile, bool saveToFile = true, bool getLocationFromHeader = true);
	~Downloader();

	
	bool JoinThread();
	const std::string& GetOutput();
	const std::wstring& GetError();

	//int ProgressCallback(curl_off_t totalDL, curl_off_t currentDL);
	size_t WriteCallback(const char* buffer, size_t size);
	size_t HeaderCallback(const char* buffer, size_t size);
};