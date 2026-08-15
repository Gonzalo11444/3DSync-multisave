#include "curl.h"

Curl::Curl(){
    curl_global_init(CURL_GLOBAL_ALL);
    _curl = curl_easy_init();
    if(!_curl) printf("Failed to init libcurl.\n");
    // Comment out next line for non-3DS builds
    //curl_easy_setopt(_curl, CURLOPT_USERAGENT, "3DSync/" VERSION_STRING);
    curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 50L);
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(_curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(_curl, CURLOPT_PIPEWAIT, 1L);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _write_callback);
    #ifdef DEBUG
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);
    #endif
}

Curl::~Curl(){
    curl_easy_cleanup(_curl);
}

void Curl::setURL(std::string URL){
    curl_easy_setopt(_curl, CURLOPT_URL, URL.c_str());
}

void Curl::setHeaders(curl_slist *headers){
    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);
}

void Curl::setReadData(void *pointer){
    curl_easy_setopt(_curl, CURLOPT_READFUNCTION, _read_callback);
    curl_easy_setopt(_curl, CURLOPT_READDATA, pointer);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, NULL);
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
}

void Curl::setBody(const void *pointer) {
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, pointer);
}

int Curl::perform(){
    CURLcode rescode = curl_easy_perform(_curl);
    curl_easy_strerror(rescode);
    return rescode;
}

long Curl::getHTTPCode() {
    long http_code = 0;
    curl_easy_getinfo (_curl, CURLINFO_RESPONSE_CODE, &http_code);
    return http_code;
}

void Curl::setWriteData(void *pointer) {
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _write_string_callback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, pointer);
}

void Curl::setWriteFile(FILE *fp) {
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _write_file_callback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, fp);
}

void Curl::setHeaderData(void *pointer) {
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, _header_callback);
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, pointer);
}

void Curl::setCustomRequestPost() {
    curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, "POST");
}

size_t Curl::_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata){
    FILE *readhere = (FILE *)userdata;
    curl_off_t nread;
    size_t retcode = fread(ptr, size, nmemb, readhere);
    nread = (curl_off_t)retcode;
    if(nread > 0){
        printf("Sent %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
    }
    return retcode;
}

size_t Curl::_write_callback(void *data, size_t size, size_t nmemb, void* userdata){
    size_t newLength = size*nmemb;
    return newLength;
}

size_t Curl::_write_string_callback(const char* data, size_t size, size_t nmemb, std::string* userdata){
    size_t newLength = size*nmemb;
    userdata->append(data, newLength);
    return newLength;
}

size_t Curl::_header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    std::string *headers = (std::string*) userdata;
    headers->append(buffer, nitems * size);
    return nitems * size;
}

size_t Curl::_write_file_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
