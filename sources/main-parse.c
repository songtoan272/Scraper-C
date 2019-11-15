#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "parsing.h"

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

    int main(void) {
        CURL *curl;
        FILE *fp;
        CURLcode res;
        char *types[] = {"text/html", "image/png", "application/xml"};
        int numTypes = 0;
        char *url = "https://stackoverflow.com";
        char outfilename[FILENAME_MAX] = "stackoverflow.txt";
        curl = curl_easy_init();
        if (curl) {
            fp = fopen(outfilename,"wb");
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
        }
        // get the number of type
        numTypes = sizeof(types)/sizeof(types[0]);
        // call the function to get all url of our file
        getUrl(outfilename);
        // call the function to parse our url list
        parseUrlFile(types, numTypes);
        return 0;
}

