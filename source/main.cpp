#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>

#include <3ds.h>

#include <curl/curl.h>

#include "libs/inih/INIReader/INIReader.h"
#include "modules/dropbox.h"
#include "modules/citra.h"
#include "modules/time.h"

std::vector<std::string> recurse_dir(std::string basepath, std::string additionalpath=""){
    std::vector<std::string> paths;
    DIR *dir;
    struct dirent *ent;
    std::string path(basepath + additionalpath);
    if((dir = opendir(path.c_str())) != NULL){
        while((ent = readdir(dir)) != NULL){
            std::string readpath(path + "/" + ent->d_name);
            std::vector<std::string> recurse = recurse_dir(basepath, additionalpath + "/" + ent->d_name);
            paths.insert(paths.end(), recurse.begin(), recurse.end());
        }
    } else {
        if(additionalpath != "") paths.push_back(additionalpath);
        else printf("Folder %s not found\n", basepath.c_str());
    }
    closedir(dir);
    return paths;
}

// Reads the [Paths] section of the INI (excluding well-known keys like
// "Checkpoint") into (localBasePath, dropboxPrefix) pairs. Extracted from
// the block that used to live inline in main() so upload and the new
// download flow can share it without duplicating the INI parsing logic.
std::vector<std::pair<std::string, std::string>> getCustomPathPairs(INIReader &reader){
    std::vector<std::pair<std::string, std::string>> pairs;
    std::map<std::string, std::string> values = reader.GetValues();
    for(auto value : values){
        if(value.first.rfind("paths=", 0) == 0){
            pairs.push_back(std::make_pair(value.second, value.first.substr(6)));
        }
    }
    return pairs;
}

// Download-only flow: Dropbox -> custom paths -> 3DS.
// Deliberately does NOT call upload() at any point, so it can never turn
// into Dropbox -> 3DS -> Upload -> Dropbox and overwrite what's stored
// remotely.
void doDownload(std::string dropboxToken, std::vector<std::pair<std::string, std::string>> pathPairs){
    if(pathPairs.empty()){
        std::cout << "No custom paths configured in 3DSync.ini, nothing to download." << std::endl;
        return;
    }

    Dropbox dropbox(dropboxToken);
    for(auto pair : pathPairs){
        std::string localBasePath = pair.first;
        std::string dropboxPrefix = pair.second;
        dropbox.downloadPath(dropboxPrefix, localBasePath);
    }

    std::cout << "Download from Dropbox finished." << std::endl;
}
void doUpload(std::string dropboxToken, std::vector<std::pair<std::string, std::string>> pathPairs){
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths;

    for(auto pair : pathPairs){
        paths[pair] = recurse_dir(pair.first);
    }

    Dropbox dropbox(dropboxToken);

    if((int)paths.size() > 0){
        dropbox.upload(paths);
    }

    std::cout << "Upload to Dropbox finished." << std::endl;
}
void printMenu(){
    printf("\n\n--- 3DSync ---");
    printf("\n[A]      Download from Dropbox");
    printf("\n[B]      Upload to Dropbox");
    printf("\n[START]  Exit\n");
}

bool componentsInit(){
    bool result = true;
    gfxInitDefault();

    consoleInit(GFX_BOTTOM, NULL);
    printf(CONSOLE_RED "\n 3DSync " VERSION_STRING " by Kyraminol" CONSOLE_RESET);
    printf("\n\n\n\n\n\n  Sync your saves with another 3DS,\n   a PC or even a cloud.");
    printf("\n\n\n\n\n\n Commit: " CONSOLE_BLUE REVISION_STRING CONSOLE_RESET);

    consoleInit(GFX_TOP, NULL);
    printf("Initializing components...\n\n");
    
    APT_SetAppCpuTimeLimit(30);
    cfguInit();
    romfsInit();
    pxiDevInit();
    amInit();
    acInit();

    u32* socketBuffer = (u32*)memalign(0x1000, 0x100000);
    if(socketBuffer == NULL){printf("Failed to create socket buffer.\n"); result = false;}
    if(socInit(socketBuffer, 0x100000)){printf("socInit failed.\n"); result = false;}

    httpcInit(0);
    sslcInit(0);
    return result;
}

void componentsExit(){
    sslcExit();
    httpcExit();
    socExit();
    acExit();
    pxiDevExit();
    romfsExit();
    cfguExit();
    gfxExit();
}

int main(int argc, char** argv){
    if(!componentsInit()) componentsExit();

    INIReader reader("/3ds/3DSync/3DSync.ini");

    // Declared here (instead of only inside the block below) so the menu
    // loop can offer "Download from Dropbox" using the same token/paths
    // that were already parsed, without re-reading the INI or re-doing auth.
    std::string dropboxToken = "";
    std::vector<std::pair<std::string, std::string>> pathPairs;

    if(reader.ParseError() < 0){
        printf("Can't load configuration\n");
    } else {
        std::string refreshToken = reader.Get("Dropbox", "RefreshToken", "");
        
        if(refreshToken != ""){
            dropboxToken = get_dropbox_access_token(refreshToken);
            if (dropboxToken == "") {
                std::cout << "Failed to receive Dropbox access token, exiting" << std::endl;
                return 1;
            }

            // Download Citra saves to local Checkpoint directory
            std::string checkpointPath = reader.Get("Paths", "Checkpoint", "");
            if (checkpointPath != "") {
                downloadCitraSaves(dropboxToken, checkpointPath);
                std::cout << "Finished downloading Citra saves!" << std::endl << std::endl;
            }

            // Parse custom paths for the manual upload/download actions.
pathPairs = getCustomPathPairs(reader);
        } else {
            printf("Can't load Dropbox token from 3DSync.ini\n");
        }

    }

    printMenu();
    while (aptMainLoop()){
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break;
        if ((kDown & KEY_A) && dropboxToken != ""){
            doDownload(dropboxToken, pathPairs);
            printMenu();
        }
        if ((kDown & KEY_B) && dropboxToken != ""){
            doUpload(dropboxToken, pathPairs);
            printMenu();
        }
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    componentsExit();
    return 0;
}
