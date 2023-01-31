#include <windows.h>
#include <winhttp.h>
#include <direct.h>
#include <string>
#include <filesystem>
#include <shellapi.h>
#include <iostream>
#include <stdio.h>
#include <CommCtrl.h>
#include <curl/curl.h>
#include <git2.h>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <stdlib.h>
#include <cstdlib>
#include <ShlObj.h>
#include <cstdlib>

#pragma warning(disable:4996)
#pragma comment(lib, "WinHTTP.lib")
#pragma comment(lib, "git2.lib")

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

using namespace std;

int main(int argc, char* argv[]) {
	// Install dir variables
	string appData = getenv("APPDATA");
	string InstallDir = appData + "\\CppShowcase";
	std::filesystem::path installPath(InstallDir);
	std::error_code errorCode;
	// for node::run
	string mainJsLoc = string(InstallDir) + "\\src\\js\\main.js";
	// Libcurl setup
	CURL* curl;
	CURLcode res;
	FILE* fp;
	const char* url = "https://nodejs.org/dist/v19.5.0/node-v19.5.0-win-x64.zip";
	char nodeJsZipfile[FILENAME_MAX];
	DWORD dwDownloaded = 0, dwTotalSize = 0;
	// Get the current directory of the C++ applications
	GetCurrentDirectoryA(sizeof(nodeJsZipfile), nodeJsZipfile);
	strcat(nodeJsZipfile, "\\tools\\node.zip");

	// Libgit2
	git_libgit2_init();
	git_repository* repo = NULL;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
	clone_options.checkout_branch = "master";


	// Check if app is installed

	if (std::filesystem::exists(InstallDir) && std::filesystem::is_directory(InstallDir)) {
		// App is installed!
		std::cout << "Folder exists and app is installed! about to update code and npm packages!" << endl;
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

		system("tools\\runnpm.bat");
		return 0;
	}

	else {
		// App is not Cloned nor installed, script proceeds as normal
		if (std::filesystem::create_directory(InstallDir, errorCode)) {
			std::cout << "folder successfully created!" << std::endl;

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
						std::cout << "Nodejs Is installed!!!" << endl;

						int repoCloneCheck = git_clone(&repo, "https://github.com/Shob3r/CppShowcase.git", InstallDir.c_str(), NULL);
						if (repoCloneCheck < 0) {
							const git_error* e = giterr_last();
							std::cerr << "Error: " << repoCloneCheck << " / " << e->message << std::endl;
							return 1;
						}

						git_repository_free(repo);
						git_libgit2_shutdown();
						system("cmd tools\\runnpm.bat");
						std::cout << "Done! the installed application is in" << InstallDir;
					}
					else {
						// Node.JS is NOT installed, Download Node.js and clone the repo after it's installed
						// Start Libcurl
						curl = curl_easy_init();
						if (curl) {
							fp = fopen(nodeJsZipfile, "wb");
							if (!fp) {
								cout << "Could not open file " << nodeJsZipfile << endl;
								return 1;
							}

							// Pre-Download checks
							curl_easy_setopt(curl, CURLOPT_URL, url);
							curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
							// Sets the file to write the data to
							curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
							// Do shit with the progress bar
							curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
							curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, [](void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) -> int {
								DWORD* params = (DWORD*)clientp;
							params[0] = (DWORD)dlnow;
							params[1] = (DWORD)dltotal;
							return 0;
								});
							curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &dwDownloaded);

							// Downloading time
							res = curl_easy_perform(curl);

							// Clean up and end the LibCurl process
							curl_easy_cleanup(curl);
							std::fclose(fp);
							// Close the prograssbar for the download

							//Check for download errors
							if (res != CURLE_OK) {
								std::cout << "Error while downloading: " << curl_easy_strerror(res) << endl;
								return 1;
							}

							// Time to clone the repository to the folder and then npm i

							int repoCloneCheck = git_clone(&repo, "https://github.com/Shob3r/CppShowcase.git", InstallDir.c_str(), NULL);
							if (repoCloneCheck < 0) {
								const git_error* e = giterr_last();
								std::cerr << "Error: " << repoCloneCheck << " / " << e->message << std::endl;
								return 1;
							}

							git_repository_free(repo);
							git_libgit2_shutdown();
							system("cmd tools\\runnpm.bat");
							std::cout << "Done! the installed application is in" << InstallDir;
							return 0;
						}
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
	}
}

// Special thanks to the internet for helping me make this