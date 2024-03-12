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
#include <random>

#include <fcntl.h>
#include <io.h>
#include <stdio.h> // NOLINT(*-deprecated-headers)
#include <map>
#include "RdFile.h"

static std::map<wchar_t, unsigned char>  hexDigits = {{'0',0x00},{'1',0x01},{'2',0x02},{'3',0x03},
                                                      {'4',0x04},{'5',0x05},{'6',0x06},{'7',0x07},
                                                      {'8',0x08},{'9',0x09},{'a',0x0a},{'b',0x0b},
                                                      {'c',0x0c},{'d',0x0d},{'e',0x0e},{'f',0x0f}};

wchar_t uniToWchar(std::wstring toParse){
    wchar_t result;
    short theHex = 0x0000;
    if (toParse.length() != 4){
        return '0';
    }
    for (int i = 0; i < 4; i++) {
        theHex = theHex | hexDigits[toParse[i]];
        if (i < 3){
            theHex = theHex << 4;
        }
    }
    result = (wchar_t) theHex;
    return result;
}

void decodeUni(std::wstring& toParse){
    for (int i = 0; i < toParse.length(); i++) {
        if(toParse.substr(i,2) == L"\\u"){
            toParse.replace(i,6, 1, uniToWchar(toParse.substr(i+2,4)));
        }
    }
}

int wmain(int argc, wchar_t* argv[])
{
    // Some important stuff
    std::vector<std::shared_ptr<RdFile>> theFiles;


    _setmode(_fileno(stdout), _O_U16TEXT); // Bless this guy for his help https://giodicanio.com/2023/06/01/how-to-print-unicode-text-to-the-windows-console-in-c-plus-plus/
    std::random_device rd;

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


    // Read settings from file, user is responsible for making it
    std::filesystem::path settingsFile;
    if (argv[2] == nullptr){
        settingsFile= (currentdir + L"\\settings.txt");
    }
    else{
        settingsFile = argv[2];
    }

    std::wstring curlfilename = L"curlOutput" + std::to_wstring(rd()) + L".txt"; // NOLINT(*-use-nullptr)
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


    std::wstring rootDownloadFolder; // In future refactoring put this in an easier to find place (aka the top likely)
    std::wstring filePath;
    std::wstring temp;
    int sleepTime = 1;
    std::wstring lastProg;
    // before this check for the download "filename". Use this for multi-file non-archives as the name for the master folder all files end up in irregardless of lower
    while (std::getline(ifs, temp)){
        auto theIndex = temp.find(L"\"filename\":");
        if (theIndex != std::wstring::npos){
            rootDownloadFolder = temp.substr(theIndex + 13, temp.size());
            rootDownloadFolder.pop_back();
            rootDownloadFolder.pop_back(); // works well enough
            decodeUni(rootDownloadFolder);
            break;
        }
    }

    while (std::getline(ifs, temp)){ // stop the looping if the links are ready
        if (temp.find(L"\"progress\":") != std::wstring::npos){
            if (temp.find(L"100") != std::wstring::npos){
                std::wcout << std::endl << L"Torrent is complete..." << std::endl << std::endl;
                break;
            }
            else{
                ifs.close();
                std::filesystem::remove(curlOutput);
                sleepTime = (temp == lastProg) ? sleepTime + 1 : 1; // Seems like a reasonable use of time
                std::wcout << std::flush << L"Torrent download to servers is not done. Please wait... -> "<< temp << L"    \r" << std::flush;
                Sleep(sleepTime * 2000);
                curlCommand = L"curl.exe -X GET -H \"Authorization: Bearer " + token + L"\" "
                              + L"\"https://api.real-debrid.com/rest/1.0/torrents/info/" + torrentID + L"\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";// NOLINT(*-inefficient-string-concatenation)
                _wsystem(curlCommand.c_str());
                ifs.open(curlOutput);
                if (!ifs.is_open()) {
                    std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
                    _wsystem(L"pause");
                    return 1;
                }

            }
            lastProg = temp;
        }
    }

    // check if the downloaded torrent is still being uploaded
    // TODO: eliminate code redundancy and make this and previous while block a function of sorts
    sleepTime = 0;
    while (std::getline(ifs, temp)){
        if (temp.find(L"\"status\":") != std::wstring::npos){
            if (temp.find(L"downloaded") != std::wstring::npos){
                std::wcout << std::endl << L"Download links are ready..." << std::endl << std::endl;
                break;
            }
            else{
                ifs.close();
                std::filesystem::remove(curlOutput);
                std::wcout << L"Torrent upload to servers is not done. Please wait briefly...\r" << std::flush;
                Sleep(sleepTime * 1000);
                curlCommand = L"curl.exe -X GET -H \"Authorization: Bearer " + token + L"\" "
                              + L"\"https://api.real-debrid.com/rest/1.0/torrents/info/" + torrentID + L"\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";// NOLINT(*-inefficient-string-concatenation)
                _wsystem(curlCommand.c_str());
                ifs.open(curlOutput);
                if (!ifs.is_open()) {
                    std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
                    _wsystem(L"pause");
                    return 1;
                }
            }
        }
    }


        //std::wcout << L"Saving filename(s)..." << std::endl << std::endl; //debug

    while (std::getline(ifs, temp)){ // save the links into the correct RdFile
        if (temp.find(L"links") != std::wstring::npos){
            //std::wcout << L"End of filename(s)..." << std::endl << std::endl; // debug
            break;
        }
        auto currIndex = temp.find(L"\"path\":"); // pardon the auto, it works though
        if (currIndex != std::wstring::npos){
            filePath = temp.substr(currIndex + 7, temp.size()); // currIndex is start of "path:", so jump 7
            filePath.pop_back(); // delete that final comma since its currently: "foobar",
            decodeUni(filePath);
            std::erase(filePath, '\\');
            theFiles.push_back(std::make_shared<RdFile>(filePath));
            //std::wcout << L"Saved filename: " << filePath <<  std::endl; //for debug
        }
    }

    int index = 0;
    while (std::getline(ifs, temp)) { // traverse file
        if (temp.find(L"],") != std::wstring::npos){ // stop once we reach end of the list of links
            break;
        }
        std::wstring restrictedLink;
        for (int i = 0; i < temp.length(); i++) {
            if (isspace(temp[i]) || temp[i] == '\\' || temp[i] == ',') {
                continue;
            }
            restrictedLink.push_back(temp[i]);
        }
        theFiles.at(index)->rawLink = restrictedLink; // yeah, the names are annoying
        index++;
    }

    // Unrestrict the link
    ifs.close();
    std::filesystem::remove(curlOutput);

    // Edge Case: Multiple file paths reported, but only one download link available. Treat as an archive
    if (theFiles.size() > 1 && theFiles.at(1)->rawLink.empty()){
        std::wcout << L"Multiple file paths reported, yet only one undecoded link...\nTreating as an archive" << std::endl;
        curlCommand = L"curl.exe -X POST --data \"link=" + theFiles.at(0)->rawLink + L"\" -H \"Authorization: Bearer "
                      + token + L"\" \"https://api.real-debrid.com/rest/1.0/unrestrict/link\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
        _wsystem(curlCommand.c_str());
        std::wcout << std::flush;

        ifs.open(curlOutput);
        if (!ifs.is_open()) {
            std::wcout << L"ERROR: Could not open " + curlOutput.wstring() << std::endl;
            _wsystem(L"pause");
            return 0;
        }

        std::wstring archiveName;
        std::wstring tempString;
        while (std::getline(ifs, tempString)) {
            if (tempString.find(L"filename") != std::wstring::npos) {
                archiveName= tempString;
                break;
            }
        }

        archiveName.pop_back();
        auto toDelete = archiveName.find(L':') + 2;
        archiveName.erase(0, toDelete);

        decodeUni(archiveName);

        theFiles.at(0)->thePath = archiveName;
        std::wcout<< theFiles.at(0)->thePath << std::endl << std::endl;

        theFiles.resize(1);

        ifs.close();
        std::filesystem::remove(curlOutput);
    }
    else if(theFiles.size() > 1){ // put all downloads in some main root directory
        for (int i = 0; i < theFiles.size(); i++){
            auto current = theFiles.at(i);
            // not too sure why my substr needs to start at 2 but it works
            current->thePath = L"\"/" + rootDownloadFolder + current->thePath.substr(2,current->thePath.size());
        }
    }

    for (int i = 0; i < theFiles.size(); i++) {
        auto current = theFiles.at(i);
        //std::wcout << L"Unrestricting " + current->rawLink << std::endl;
        curlCommand = L"curl.exe -X POST --data \"link=" + current->rawLink + L"\" -H \"Authorization: Bearer "
                      + token + L"\" \"https://api.real-debrid.com/rest/1.0/unrestrict/link\" -o " + curlOutput.wstring() + L" --create-dirs --no-progress-meter";
        _wsystem(curlCommand.c_str());
        std::wcout << std::flush;

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
        current->decodedLink = downloadLink;

        ifs.close();
        std::filesystem::remove(curlOutput);
    }

    for (const auto &curr: theFiles) {
        // Download the file!
        std::wcout << L"Downloading " + curr->thePath << std::endl; // Yeah, the link is raw, so what?
        std::wcout << std::endl;
        curlCommand = L"curl.exe " + curr->decodedLink + L" -o " + curr->thePath + L" --output-dir " + outdir + L" --create-dirs";
        _wsystem(curlCommand.c_str());
        std::wcout << std::endl;
    }

    // Cleanup
    ifs.close();
    std::filesystem::remove(curlOutput); // If curlOutput remains on system that's the sign of a mistake
    std::wcout << L"Success!" << std::endl;
    return 0;
}

