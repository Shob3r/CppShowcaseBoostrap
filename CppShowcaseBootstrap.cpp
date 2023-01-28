// Most of these libraries are unused lol
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include "include/curl/curl.h"
#include "include/git2.h"
#include <winhttp.h>
#include <fstream>
#include <direct.h>
#include <string>
#include <sys/stat.h>
#include <filesystem>
#include <shellapi.h>
#include <ShlObj.h>

#pragma warning(disable:4996)
#pragma comment(lib, "WinHTTP.lib")
#pragma comment(lib, "git2.lib")

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
	git_repository *repo = NULL;
	
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;

	// Libcurl

	

	// Check if app is installed
	if (std::filesystem::exists(InstallDir) && std::filesystem::is_directory(InstallDir)) {
		// App is installed!
		std::cout << "Folder exists and app is installed!";
		int cloneCheck = git_repository_open(&repo, InstallDir.c_str());
		if (cloneCheck < 0) {
			std::cout << "Could not find the source code!";
			return 1;
		}
		git_remote *remote = NULL;

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
					int result = MessageBoxEx(NULL, L"Do you have Node.Js installed on your computer?", L"Pre-Start checks", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);

					if (result == IDYES) {
						// The user will never select yes, they don't even know what Node.Js is lmao
						MessageBoxEx(NULL, L"Good to know! this program will now download and install the source code", L"About to download", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON2 | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
					}
					else if (result == IDNO) {
						// Always going to be what the user selects lmao
						MessageBoxEx(NULL, L"This app will now download node.js to compile and run the app", L"About to download", MB_OK | MB_ICONASTERISK | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
					}

					return 0;
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