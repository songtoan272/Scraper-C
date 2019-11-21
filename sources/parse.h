/*
**  Filename : parse.h
**
**  Made by : CAO Song Toan
**
**  Description :   Interface managing the parsing of website 
**                  determined by the configuration
*/
#ifndef __PARSE
#define __PARSE

#include "configuration.h"
#include "url.h"

#define NB_MIME_TYPES 62
#define BUFFER_SIZE 2000



typedef struct typeMIME{
  char *type;
  char *extension;
}TypeMIME;

extern TypeMIME* allMIMEs;

/*Each Action will be associated with its tree of URLs 
* by this wrapper. This wrapper allows us to get access
* to the initial action (its name, url and options) 
* and at the same time manipulate its tree associated.
* A WrapAction will be initiated when the Action enters 
* scrapping process and will be destroy when all scrapping
* is done.
*/
typedef struct wrapAction{
  Action *action;
  Node root;
}WrapAction;

/*LinkEasyMulti is the association between a curl easy handle 
* and a curl multi handle. A curl easy handle is to manage 
* the scrapping of a URL while the multi handle is used to
* manage all the scrapping process of a task.
*/
typedef struct linkEasyMulti{
  CURL *easy;
  CURLM *multi;
}LinkEasyMulti;



void delAllMIME(TypeMIME *allMIME);

TypeMIME *initAllMIME();

WrapAction *initWrap(Action *action, Node root);

void delWrap(WrapAction **wrapper);

LinkEasyMulti *initLink(CURL *easy, CURLM *multi);

void delLink(LinkEasyMulti *link);

int isTypeSelected(char *type, Action *action);

char *extractLastPart(char *url);

char *makeFilePath(Action *action, char *contentType, char *url);

size_t saveData(void *data, size_t size, size_t nmemb, char *dataType, char *filePath, char *url);

void getURLsFromFile(FILE *f, CURLM *cm, WrapAction *wrapper, char* URLOfFile);

void reconstructURL(char **URLRelative, char *URLDomain);

size_t write_cb(void *data, size_t size, size_t nmemb, CURL *easy);
 
void add_transfer(CURLM *cm, WrapAction *wrapper, char *url);

void parseATask(Task *task);

void parseConfig(Configure *config);

#endif