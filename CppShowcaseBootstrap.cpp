// Libraries
#include <windows.h> // The windows API, only really used for popups if something fails
#include <winhttp.h> // WinHttp can make http requests, it is used to check for the internet in this application
#include <direct.h> // Interacts with system directories
#include <string> // For string variables
#include <filesystem> // Interacts with the filesystem
#include <iostream> // Logs things in the console
#include <curl/curl.h> // Library for Curl, an application which downloads files
#include <git2.h> // Library for git, which is the most used software version control software
#include <fstream> // More file/folder stuff

#pragma warning(disable:4996) // Disable the warning of a variable being "unsafe"
#pragma comment(lib, "WinHTTP.lib") // Include the Winhttp library at compile time, compiling would not be successful without these 2 lines
#pragma comment(lib, "git2.lib") // Include the libgit2 library at compile time

// For Libcurl
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

using namespace std;
// Main process
int main(int argc, char* argv[]) {
	// Install dir variables
	string appData = getenv("APPDATA");
	string InstallDir = appData + "\\CppShowcase";
	std::filesystem::path installPath(InstallDir);
	std::error_code errorCode;

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

	// Libgit2 setup
	git_libgit2_init();
	git_repository* repo = NULL;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
	clone_options.checkout_branch = "master";

	// Check if the app is installed

	if (std::filesystem::exists(InstallDir) && std::filesystem::is_directory(InstallDir)) {
		// App is installed!
		std::cout << "Folder exists and app is installed! about to update code and npm packages!" << endl;
		// Attempt to connect to the repository
		int cloneCheck = git_repository_open(&repo, InstallDir.c_str());
		if (cloneCheck < 0) {
			// If the connection fails, the program tells the user as much
			std::cout << "Could not find the source code!";
			return 1;
		}
		git_remote* remote = NULL;
		// Connect to the branch of development
		cloneCheck = git_remote_lookup(&remote, repo, "origin");
		if (cloneCheck < 0) {
			// If looking for the branch fails, tell the user that it can't find the branch
			std::cout << "Could find the branch!";
			return 1;
		}
		// Pull from the branch
		cloneCheck = git_remote_fetch(remote, NULL, NULL, NULL);
		if (cloneCheck < 0) {
			// If it fails, tells the user
			std::cout << "Could not sync with the repository!";
			return 1;
		}

		// Shudown libgit2
		git_remote_free(remote);
		git_repository_free(repo);
		git_libgit2_shutdown();
		// Run an npm i command in batch to update any packages
		system("tools\\runnpm.bat");
		return 0;
	}

	else {
		// App is not Cloned nor installed, script proceeds as normal
		// Create the installation directory in the appdata folder
		if (std::filesystem::create_directory(InstallDir, errorCode)) {
			std::cout << "folder successfully created!" << std::endl;
			// Check if the user has an active internet connection
			HINTERNET hSession = WinHttpOpen(L"PersonalPInternetChecker",
				WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0);
			if (hSession) {
				// If WinHttp can start, it will attempt to connect to 1.1.1.1
				HINTERNET hConnect = WinHttpConnect(hSession, L"1.1.1.1",
					INTERNET_DEFAULT_HTTPS_PORT, 0);
				if (hConnect) {
					/* If connection to 1.1.1.1 is successful, continue with the script
					Checks if Node.js is installed in its default directory */

					string NodeJsInstallDir = "C:\\Program Files\\nodejs";

					if (std::filesystem::exists(NodeJsInstallDir)) {
						// Node.JS is installed, Continue to cloning the repo.
						std::cout << "Nodejs Is installed!!!" << endl;
						// Clone the repository to the user's appdata folder
						int repoCloneCheck = git_clone(&repo, "https://github.com/Shob3r/CppShowcase.git", InstallDir.c_str(), NULL);
						if (repoCloneCheck < 0) {
							// If it fails, spit out an error
							const git_error* e = giterr_last();
							std::cerr << "Error: " << repoCloneCheck << " / " << e->message << std::endl;
							return 1;
						}
						// End libgit and run an npm command in a seperate batch file, then tell the install directory if the application
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
								// Spit out an error it the clone fails
								const git_error* e = giterr_last();
								std::cerr << "Error: " << repoCloneCheck << " / " << e->message << std::endl;
								return 1;
							}
							// End libgit and run an npm command in a seperate batch file, then tell the install directory if the application
							git_repository_free(repo);
							git_libgit2_shutdown();
							system("cmd tools\\runnpm.bat");
							std::cout << "Done! the installed application is in" << InstallDir;
							return 0;
						}
					}
				}
				else {
					// Create a windows.h popup to tell the user that it can't connect to the internet
					MessageBoxEx(NULL, L"Could not connect to the internet! run this file again once you have connected to the internet", L"Error!", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
					return 1;
				}
			}
			else {
				// Create a windows.h popup to tell the user that it can't start WinHttp.h
				MessageBoxEx(NULL, L"Could not start WinHttp! this error is probably related to you still being on Windows 95 or you have something really wrong with your PC", L"Error!", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, LANG_NEUTRAL);
				return 1;
			}
		}
	}
}