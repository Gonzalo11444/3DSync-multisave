// PC/WSL test harness for Dropbox::downloadPath() (custom paths download).
// Mirrors the existing test.cpp pattern used for Citra saves.
//
// Build (from repo root, inside WSL):
//   g++ -o test_download test_download.cpp source/modules/dropbox.cpp source/utils/curl.cpp -lcurl -ljson-c
//
// Before building, comment out the VERSION_STRING line in
// source/utils/curl.cpp (line 8), same as the README instructs for test.cpp.
//
// Run:
//   export DROPBOX_REFRESH_TOKEN=<your Dropbox refresh token>
//   ./test_download <dropboxPrefix> <localBasePath>
//
// Example (Windows path exposed through WSL):
//   ./test_download "roms/nds/saves" "/mnt/c/3ds-sdmc-dev/roms/nds/saves"

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>

#include "source/modules/dropbox.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <dropboxPrefix> <localBasePath>" << std::endl;
        std::cout << "Example: " << argv[0] << " roms/nds/saves /mnt/c/3ds-sdmc-dev/roms/nds/saves" << std::endl;
        return 1;
    }

    std::string dropboxPrefix = argv[1];
    std::string localBasePath = argv[2];

    auto refreshToken = std::getenv("DROPBOX_REFRESH_TOKEN");
    if (refreshToken == nullptr) {
        std::cout << "Set DROPBOX_REFRESH_TOKEN env var first" << std::endl;
        return 1;
    }

    std::string dropboxToken = get_dropbox_access_token(refreshToken);
    if (dropboxToken == "") {
        std::cout << "Failed to receive Dropbox access token, exiting" << std::endl;
        return 1;
    }

    Dropbox dropbox(dropboxToken);
    dropbox.downloadPath(dropboxPrefix, localBasePath);

    return 0;
}
