// Most of these libraries are unused lol
#include <windows.h>
#include <winhttp.h>
#include <direct.h>
#include <string>
#include <filesystem>
#include <shellapi.h>
#include <iostream>
#include <stdio.h>
#include <CommCtrl.h>
#include "include/curl/curl.h"
#include "include/git2.h"

#pragma warning(disable:4996)
#pragma comment(lib, "WinHTTP.lib")
#pragma comment(lib, "git2.lib")

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	// This function will be called by libcurl as it receives data from the download.
	// We can use this opportunity to update the progress bar.

	// First, calculate the total number of bytes received so far.
	int total_bytes = size * nmemb;

	// Next, update the progress bar.
	// The progress bar is represented by a HWND (a handle to a window).
	// We can use the SendMessage function to send a message to the progress bar
	// telling it to update its position.
	SendMessage((HWND)userp, PBM_SETPOS, total_bytes, 0);

	// Return the number of bytes received, to let libcurl know how much data was processed.
	return total_bytes;
}

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

using namespace std;

int main() {
	// Install dir variables
	string appData = getenv("APPDATA");
	string InstallDir = appData + "\\CppShowcase";
	std::filesystem::path installPath(InstallDir);
	std::error_code errorCode;

	// Libgit2
	git_libgit2_init();
	git_repository* repo = NULL;

	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;

	// Check if app is installed
	if (std::filesystem::exists(InstallDir) && std::filesystem::is_directory(InstallDir)) {
		// App is installed!
		std::cout << "Folder exists and app is installed!";
		int cloneCheck = git_repository_open(&repo, InstallDir.c_str());
		if (cloneCheck < 0) {
			std::cout << "Could not find the source code!";
			return 1;
		}
		git_remote* remote = NULL;

		cloneCheck = git_remote_lookup(&remote, repo, "origin");
		if (cloneCheck < 0) {
			std::cout << "Could find the branch!";
			return 1;
		}

		cloneCheck = git_remote_fetch(remote, NULL, NULL, NULL);
		if (cloneCheck < 0) {
			std::cout << "Could not sync with the repository!";
			return 1;
		}
		git_remote_free(remote);
		git_repository_free(repo);
		git_libgit2_shutdown();

		return 0;
	}

	else {
		// App is not Cloned nor installed, script proceeds as normal
		if (std::filesystem::create_directory(InstallDir, errorCode)) {
			std::cout << "folder successfully created!";

			HINTERNET hSession = WinHttpOpen(L"PersonalPInternetChecker",
				WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0);
			if (hSession) {
				HINTERNET hConnect = WinHttpConnect(hSession, L"1.1.1.1",
					INTERNET_DEFAULT_HTTPS_PORT, 0);
				if (hConnect) {
					string NodeJsInstallDir = "C:\\Program Files\\nodejs";

					if (std::filesystem::exists(NodeJsInstallDir)) {
						// Node.JS is installed, Continue to cloning the repo.
						// No need to add anything in this section
					}
					else {
						// Node.JS is NOT installed, Download Node.js and clone the repo after it's installed
						// Start Libcurl to download Node.Js
						curl_global_init(CURL_GLOBAL_ALL);
						CURL* curl = curl_easy_init();
						curl_easy_setopt(curl, CURLOPT_URL, "https://nodejs.org/dist/v19.5.0/node-v19.5.0-x64.msi");

						// Do the funny progressbar thing
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

						HWND DownloadProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
							WS_CHILD | WS_VISIBLE,
							10, 10, 200, 20, NULL,
							NULL, GetModuleHandle(NULL), NULL);
						SendMessage(DownloadProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, DownloadProgress);

						// Download nodejs with libcurl fr this time
						CURLcode res = curl_easy_perform(curl);

						// Clean up libcurl, and run the installer file
						curl_easy_cleanup(curl);
						curl_global_cleanup();
					}
				}
				else {
					MessageBoxEx(NULL, L"Could not connect to the internet! run this file again once you have connected to the internet", L"Error!", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
					return 1;
				}
			}
			else {
				MessageBoxEx(NULL, L"Could not start WinHttp! this error is probably related to you still being on Windows 95 or you have something really wrong with your PC", L"Error!", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
				return 1;
			}
		}
		else {
			std::cout << "Error while creating the directory: " << errorCode.message();
		}
	}
}

// Special thanks to the internet for helping me make this