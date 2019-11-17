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

WrapAction *initWrap(Action *action, Node root);

void delWrap(WrapAction **wrapper);

LinkEasyMulti *initLink(CURL *easy, CURLM *multi);

void delLink(LinkEasyMulti *link);

int isTypeSelected(char *type, Action *action);

char *extractLastPart(char *url);

int hasExtension(char *fileName);

size_t saveData(char *data, char *dataType, char *filePath, char *url);

char *getURL(char *data, char **dataLeft);

void reconstructURL(char **URLRelative, char *URLDomain);

size_t write_cb(char *data, size_t n, size_t l, LinkEasyMulti *linkHandles);
 
void add_transfer(CURLM *cm, WrapAction *wrapper, char *url);

/**
 * Initialize a Node
 * @param suburl : the string of sub-link of the url 
 * @param depth : the depth of this link from the initial
 * @param next : its next sibling
 * @param child : its first child
 * @return : the node itself
 */
void parseAction(Action *action);

/**
 * Make a tree from the very initial url 
 * @param url : the initial url of the tree
 * @return : the root of the tree
 */
void parseTask(Task *task);

/**
 * Compare two nodes by their url 
 * @param node1, node2 : 2 nodes to be compared
 * @return : 0 if their url are equals 
 *          -1 if node1->url < node2->url
 *           1 if node1->url > node2->url
 */
void parseConfigure(Configure *config);



#endif