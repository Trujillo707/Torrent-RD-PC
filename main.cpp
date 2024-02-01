// Torrent_to_RD_to_PC
/*MIT License

Copyright (c) 2024 Orlando Trujillo-Ortiz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cwchar>
#include <Windows.h>

#include <fcntl.h>
#include <io.h>
#include <stdio.h> // NOLINT(*-deprecated-headers)

int wmain(int argc, wchar_t* argv[])
{
    _setmode(_fileno(stdout), _O_U16TEXT); // Bless this guy for his help https://giodicanio.com/2023/06/01/how-to-print-unicode-text-to-the-windows-console-in-c-plus-plus/

    // Common sense
    if (argv[1] == nullptr) {
        std::wcout << L"ERROR: No command line arguments were passed!!!" << std::endl;
        _wsystem(L"pause");
        return 0;
    }

    // Char stars are for losers
    std::wstring torrentFilePath = argv[1];
    std::wstring currentdir = argv[0];
    currentdir = currentdir.substr(0, currentdir.find_last_of('\\'));
    //std::filesystem::path currentPath = currentdir;

    // Read settings from file, user is responsible for making it
    std::filesystem::path settingsFile(currentdir + L"\\settings.txt");
    std::wstring curlfilename = L"curlOutput" + std::to_wstring(std::time(0)) + L".txt"; // NOLINT(*-use-nullptr)
    std::filesystem::path curlOutput(currentdir + L"\\" + curlfilename);

    std::wifstream ifs(settingsFile);

    if (!ifs.is_open()) {
        std::wcout << L"ERROR: settings.txt was unable to be opened!\nPlease confirm it exists in same directory as " << currentdir << std::endl;
        _wsystem(L"pause");
        return 0;
    }
    // Tell user what should be in settings if its blank (somehow)
    if (std::filesystem::file_size(settingsFile) == 0) {
        std::wcout << L"ERROR: Settings File empty! Please configure it using the following:\n"
                   << L"token=string\n"
                   << L"outdir=string\n";
        _wsystem(L"pause");
        return 0;

    }

    // Get API-Token
    std::wstring token;
    std::getline(ifs, token);

    if ((token.substr(0, 6)) != L"token=") {
        std::wcout << L"ERROR: Please properly format first line as\ntoken=string\n";
        _wsystem(L"pause");
        return 0;
    }
    token.erase(0, 6);
    if (token.empty()) {
        std::wcout << L"ERROR: token is missing! Please provide one in settings.txt to access Real-Debrid\n";
        _wsystem(L"pause");
        return 0;
    }

    // Get Video File output directory
    std::wstring outdir;
    std::getline(ifs, outdir);
    ifs.close(); // done with file

    if ((outdir.substr(0, 7)) != L"outdir=") {
        std::wcout << L"ERROR: Please properly format first line as\noutdir=string\n";
        _wsystem(L"pause");
        return 0;
    }
    outdir.erase(0, 7);
    if (outdir.empty()) {
        std::wcout << L"ERROR: output directory is missing! Please provide one in settings.txt (it takes 2 seconds)\n";
        _wsystem(L"pause");
        return 0;
    }

    // Verify outdir is a valid PATH
    if (!std::filesystem::exists(outdir)) {
        std::wcout << L"ERROR: " << outdir << L" does not exist\n";
        _wsystem(L"pause");
        return 0;
    }

    // Upload torrent to Real-Debrid
    std::wcout << L"Uploading " + torrentFilePath + L" to Real Debrid" << std::endl;
    std::wstring curlCommand = L"curl.exe -g -H \"Authorization: Bearer " + token + L"\" \"https://api.real-debrid.com/rest/1.0/torrents/addTorrent\""
                               + L" --upload-file \"" + torrentFilePath + L"\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
    _flushall();
    _wsystem(curlCommand.c_str());
    std::wcout << std::endl;

    // Read curlOutput.txt and get the torrent id

    ifs.open(curlOutput);
    if (!ifs.is_open()) {
        std::wcout << L"ERROR: Could not open " + curlOutput.wstring() + L"\n";
        _wsystem(L"pause");
        return 0;
    }
    std::wstring torrentID;
    ifs.ignore(256, ':');
    ifs.ignore(256, '\"');
    std::getline(ifs, torrentID);
    while (ispunct(torrentID.back())) {
        torrentID.pop_back();
    }

    // Select the files using torrent id and start torrent
    ifs.close();
    std::filesystem::remove(curlOutput);// wipe it for ease
    std::wcout << L"Selecting all files from torrentID: " + torrentID << std::endl;
    curlCommand = L"curl.exe -X POST --data \"files=all\" -H \"Authorization: Bearer " + token + L"\" "
                  + L"\"https://api.real-debrid.com/rest/1.0/torrents/selectFiles/" + torrentID + L"\" --no-progress-meter";
    _wsystem(curlCommand.c_str());
    std::wcout << std::endl;

    // Get unrestricted link for the torrent file
    std::wcout << L"Attempting to get download link from torrent" << std::endl;
    curlCommand = L"curl.exe -X GET -H \"Authorization: Bearer " + token + L"\" "
                  + L"\"https://api.real-debrid.com/rest/1.0/torrents/info/" + torrentID + L"\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
    _wsystem(curlCommand.c_str());
    std::wcout << std::endl;
    ifs.open(curlOutput);
    if (!ifs.is_open()) {
        std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
        _wsystem(L"pause");
        return 0;
    }
    std::wstring restrictedLink;
    std::wstring temp;
    std::wstring torrentProgress;
    int sleepTime = 2;
    while (std::getline(ifs, temp)) { // traverse file
        if (temp.find(L"progress") != std::wstring::npos) { // get progress
            torrentProgress = temp;
        }

        if (temp.find(L"links") != std::wstring::npos) { // find download section
            std::getline(ifs, temp); // get download line
            if (temp.find(L"real-debrid.com") == std::wstring::npos) { // if no valid link, attempt to recheck for it
                ifs.close();
                std::filesystem::remove(curlOutput);
                std::wcout << L"No download link found! Attempting to wait " + std::to_wstring(sleepTime * 2) + L" seconds and will try again..." << std::endl;
                std::wcout << torrentProgress << std::endl;
                Sleep(sleepTime * 2000);
                curlCommand = L"curl.exe -X GET -H \"Authorization: Bearer " + token + L"\" "
                              + L"\"https://api.real-debrid.com/rest/1.0/torrents/info/" + torrentID + L"\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
                _wsystem(curlCommand.c_str());
                std::wcout << std::endl;
                ifs.open(curlOutput);
                if (!ifs.is_open()) {
                    std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
                    _wsystem(L"pause");
                    return 0;
                }
                sleepTime ++;
            }
            else{
                break;
            }
        }
    }
    std::erase(temp, ' '); // dunno if this does anything, not going to touch it tbh
    std::erase(temp, '\\');
    std::erase(temp, '"');
    for (int i = temp.size() - 1; i >= 0; i--) {
        if (isspace(temp[i])) {
            break;
        }
        restrictedLink.push_back(temp[i]);

    }
    std::reverse(restrictedLink.begin(), restrictedLink.end()); // clean link

    // Unrestrict the link
    ifs.close();
    std::filesystem::remove(curlOutput);
    std::wcout << L"Unrestricting " + restrictedLink << std::endl;
    curlCommand = L"curl.exe -X POST --data \"link=" + restrictedLink + L"\" -H \"Authorization: Bearer "
                  + token + L"\" \"https://api.real-debrid.com/rest/1.0/unrestrict/link\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
    _wsystem(curlCommand.c_str());
    std::wcout << std::endl;

    // Get direct link to media file (Please ensure you have the legal permission to download :P)
    ifs.open(curlOutput);
    if (!ifs.is_open()) {
        std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
        _wsystem(L"pause");
        return 0;
    }
    std::wstring downloadLink;
    temp.clear();
    while (std::getline(ifs, temp)) {
        if (temp.find(L"download") != std::wstring::npos) {
            downloadLink = temp;
            break;
        }
    }
    downloadLink.pop_back();
    int toDelete = downloadLink.find(L':') + 2;
    downloadLink.erase(0, toDelete);
    std::erase(downloadLink, '\\');

    // Grab filename since curl refuses to decode the urls
    std::filesystem::path tempPath = torrentFilePath;
    std::wstring fileName = tempPath.filename().wstring();
    if (fileName.find(L".torrent") != std::wstring::npos) {
        fileName = fileName.substr(0, fileName.find_last_of('.'));
    }

    tempPath = fileName; // meh, it works

    // Enforce that it has the right file extension in case it is missing
    std::wstring junkDownloadLink = downloadLink.substr(downloadLink.find_last_of('/'));
    junkDownloadLink.pop_back(); // links have the quotes at the end
    std::filesystem::path downloadLinkAsPath = junkDownloadLink;
    if (tempPath.extension() != downloadLinkAsPath.extension()){
        fileName.append(downloadLinkAsPath.extension().wstring());
    }

    // IGNORE: for debug
    //std::wcout << L"Download link extension; fileName extension; new fileName " +  downloadLinkAsPath.extension().wstring() + L" "  + tempPath.extension().wstring() + fileName << std::endl;
    //_wsystem(L"pause");

    // Download the file!
    std::wcout << L"Downloading " + downloadLink << std::endl; // Yeah, the link is raw, so what?
    std::wcout << std::endl;
    curlCommand = L"curl.exe " + downloadLink + L" -o \"" + fileName + L"\" --output-dir " + outdir + L" --create-dirs";
    _wsystem(curlCommand.c_str());
    std::wcout << std::endl;

    // Cleanup
    ifs.close();
    std::filesystem::remove(curlOutput); // If curlOutput remains on system that's the sign of a mistake
    std::wcout << L"Success!" << std::endl;
    return 0;
}

