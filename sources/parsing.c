#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "parsing.h"

#define BUFFER_SIZE 1000

// > This functions allow to remove a substring in a string
char *strRemove(char *str, const char *sub) {
    char *p, *q, *r;
    if ((q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            while (p < r)
                *q++ = *p++;
        }
        while ((*q++ = *p++) != '\0')
            continue;
    }
    return str;
}

// > This function allow to parse the content of the first url and get all the hyperlink that we can find in this content
void getUrl(char *outfilename) {
    char buffer[BUFFER_SIZE];
    FILE * fp;
    FILE * fUrl;
    char *href, *startUrl, *endUrl, *url;
    fUrl = fopen("urlList.txt","w+");
    fp = fopen(outfilename,"r");
    if(fUrl == NULL || fp == NULL){
        printf("Impossible d'accéder au fichier\n");
        exit(EXIT_SUCCESS);
    }
    //get each line of html content
    while((fgets(buffer,BUFFER_SIZE, fp) != NULL)){
        //get line where we find an href
        if (strstr(buffer, "href") != NULL) {
            href = strstr(buffer,"href=");
            startUrl = href + strlen("href=\"");
            endUrl = strstr(startUrl,"\"");
            url = strRemove(startUrl,endUrl);
            //write url in a new file
            fputs(url,fUrl);
            fputs("\n",fUrl);
        }
    }
    fclose(fp);
    fclose(fUrl);
}

// > This function allow to check if the content of our url file match with the different type mime present in the configuration file, then
// we download all of this hyperlink and placed in a unique folder
void parseUrlFile(char **types, int sizeTypes) {
    CURL *curl;
    FILE *fp;
    FILE *fUrl;
    CURLcode res;
    char bufferUrl[300];
    char *folderName, *fileName, *contentType;
    fUrl = fopen("urlList.txt", "r");
    if (fUrl == NULL) {
        printf("Impossible d'accéder au fichier\n");
        exit(EXIT_SUCCESS);
    }
    // get each url
    while ((fgets(bufferUrl, 300, fUrl) != NULL)) {
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, bufferUrl);
            res = curl_easy_perform(curl);
            if(!res){
                // > Extract the content type
                contentType = NULL;
                res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
                if (!res && contentType) {
                    printf("Content-Type: %s\n", contentType);
                    // > compare each type with current link
                    for (int i = 0; i < sizeTypes; i++) {
                        if (strstr(contentType, types[i]) != NULL) {
                            // > if the type of link match with our type we can write the content in a new file named with the link content
                            /*char *startUrl = strstr(bufferUrl,"://");
                            char *endUrl = strstr(startUrl,"\"");
                            char *outfilename = strRemove(startUrl,endUrl);
                            strcat(outfilename,".txt");
                            printf("%s\n",outfilename);
                            fp = fopen(outfilename, "w");
                            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                            res = curl_easy_perform(curl);*/
                            printf("Le type de cet hyperlien est %s.\n",types[i]);
                            break;
                        } else {
                            printf("Pas de type correspondant\n");
                        }
                    }
                }
            }
        }

    }
    //fclose(fp);
    fclose(fUrl);
    curl_easy_cleanup(curl);
}