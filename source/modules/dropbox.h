#ifndef MODULES_DROPBOX_H
#define MODULES_DROPBOX_H

#include <string>
#include <vector>
#include <map>

#include "../utils/curl.h"

typedef struct ListResult {
    std::string name;
    std::string path_display;
    std::string server_modified;
    std::string tag; // "file" or "folder", used by downloadPath() to recurse
} ListResult;

std::string get_dropbox_access_token(std::string refreshToken);

class Dropbox{
    public:
        Dropbox(std::string token);
        ~Dropbox(){};
        void upload(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths);
        std::vector<ListResult> list_folder(std::string path);
        void download(std::string path, std::string destPath);
        // Downloads everything found under "/<dropboxPrefix>" in Dropbox into
        // localBasePath, recreating the same relative folder structure locally.
        // Uses the existing list_folder()/download() under the hood; does not
        // upload anything and does not touch local files that aren't present
        // in Dropbox.
        void downloadPath(std::string dropboxPrefix, std::string localBasePath);
        
    private:
        std::string _token;
        Curl _curl;
        void _downloadFolderRecursive(std::string remotePath, std::string localBasePath, std::string relativePath);
};


#endif
