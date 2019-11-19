/*
**  Filename : parse.c
**
**  Made by : CAO Song Toan
**
**  Description :   Interface managing the parsing of website 
**                  determined by the configuration
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "configuration.h"
#include "url.h"
#include "parse.h"

/** 
 * Initialize the WrapAction
 */
WrapAction *initWrap(Action *action, Node root){
  WrapAction *res = (WrapAction*)malloc(sizeof(WrapAction));
  res->action = action;
  res->root = root;
  return res;
}

void delWrap(WrapAction **wrapper){
  delTree(&((*wrapper)->root));
  free(*wrapper);
  *wrapper = NULL;
}

LinkEasyMulti *initLink(CURL *easy, CURLM *multi){
  LinkEasyMulti *res = (LinkEasyMulti*)malloc(sizeof(LinkEasyMulti));
  res->easy = easy;
  res->multi = multi;
  return res;
}

void delLink(LinkEasyMulti *link){
  free(link);
}

int getMaxDepth(Action *action){
  int res = 0;
  for (int i = 0; i < action->nbOptions; ++i){
    switch (action->options[i].type){
      case MAX_DEPTH:
        res = action->options[i].val.depth;
        break;
      default:
        break;
    }
  }
  return res;
}

int getVersionning(Action *action){
  int res = 0;
  for (int i = 0; i < action->nbOptions; ++i){
    switch (action->options[i].type){
      case VERSIONNING:
        res = action->options[i].val.shift;
        break;
      default:
        break;
    }
  }
  return res;
}

int isTypeSelected(char *type, Action *action){
  char **typesSelected = NULL;
  int nbTypes = 0;

  for (int i = 0; i < action->nbOptions; ++i){
    switch (action->options[i].type){
      case TYPESELECT:
        typesSelected = action->options[i].val.type.types;
        nbTypes = action->options[i].val.type.nbTypes;
        break;
      default:
        break;
    }
  }

  if (nbTypes == 0){
    //this action has no option TYPESELECT
    //save all type of data
    return 1;
  }else{
    for (int i = 0; i < nbTypes; ++i){
      if (strstr(type, typesSelected[i]) != NULL){
        return 1;
      }
    }
    return 0;
  }
}

char *extractLastPart(char *url){
  char *mark, *prevMark, *res;
  int host = 1; 
  char prohitbited[] = "*:\\/<>|\"?[];=+&£$.!@#^() ";


  mark = strchr(url, '/');
  while (mark != NULL && *(mark+1) != '\0'){
    prevMark = mark;
    mark = strchr(mark+1, '/');
    host = 0; 
  }
  if (host == 0){ // www.abc.com/sub1/sub2...
    if (mark != NULL) {
      res = strndup(prevMark+1, strlen(prevMark)-2);  
    }
    else res = strdup(prevMark+1);
  }else{ // www.abc.com(/)
    res = strndup(url, strlen(url)-1);
  }

  //replace prohibited character by file naming convention in host url by _
  mark = res;
  for (int i = 0; i < strlen(res); ++i){
      if (strchr(prohitbited, res[i]) != NULL){
        res[i] = '_';
      }
  }
  return res;
}

int hasExtension(char *fileName){
  char *dot = strrchr(fileName, '.');
  if (!dot || dot == fileName) return 0;
  return 1;
}

char *getExtFromDataType(char *dataType){
    char *slash, *plus, *semicolon;
    slash = strchr(dataType, '/');
    plus = strchr(dataType, '+');
    semicolon = strchr(dataType, ';');
    if (slash == NULL){
        fprintf(stderr, "Wrong MIME type.\n");
        exit(1);
    }else{
        if (semicolon != NULL){
            if (plus != NULL){
                return strndup(slash+1, plus - slash - 1);
            }else{
                return strndup(slash+1, semicolon - slash - 1);
            }
        }else{
            if (plus != NULL){
                return strndup(slash+1, plus - slash - 1);
            }else{
                return strdup(slash+1);
            }
        }
    }
}

size_t saveData(void *data, size_t size, size_t nmemb, char *dataType, char *filePath, char *url){
  FILE *f;
  //use last part after '/' of the url as name of saved file
  char *fileName = extractLastPart(url);
  char *extension;
  char *fullPath;
  size_t res;

  extension = getExtFromDataType(dataType);
  fullPath = (char*)malloc((strlen(filePath) + strlen(fileName) + strlen(extension) + 3) * sizeof(char));
  strcpy(fullPath, filePath);
  strcat(fullPath, "/");
  strcat(fullPath, fileName);
  strcat(fullPath, ".");
  strcat(fullPath, extension);

  f = fopen(fullPath, "a");
  res = fwrite(data, size, nmemb, f);
  fclose(f);
  free(fullPath);
  free(fileName);
  free(extension);
  return res;
}

char *getURL(char *data, char **dataLeft){
  if (data == NULL || strlen(data) == 0) return NULL;

  char *href, *src, *startURL, *endURL, *url;
  href = strstr(data, "href=\"");
  src = strstr(data, "src=\"");

  if (href != NULL){
    if (src != NULL){
      if (href < src){
        //find a href tag before a src tag
        startURL = href + strlen("href=\"");
        endURL = strstr(startURL, "\"");
      }else{
        startURL = src + strlen("src=\"");
        endURL = strstr(startURL, "\"");
      }
    }else{
      startURL = href + strlen("href=\"");
      endURL = strstr(startURL, "\"");
    }
  }else{
    if (src != NULL){
      startURL = src + strlen("src=\"");
      endURL = strstr(startURL, "\"");
    }else{
      return NULL;
    }
  }
  
  url = strndup(startURL, endURL-startURL);
  *dataLeft = endURL;
  return url;
}

void reconstructURL(char **URLRelative, char *URLHost){
  char *slash, *res;

  //special case URL relative = #
  if (**URLRelative == '#'){
    URLHost = delProtocol(URLHost);
    slash = strchr(URLHost, '/');
    res = (char*)malloc((slash - URLHost + strlen(*URLRelative) + 1) * sizeof(char));
    strncpy(res, URLHost, (slash-URLHost)/sizeof(char));
    res[slash-URLHost] = '\0';
    strcat(res, "/");
    strcat(res, *URLRelative);
    free(URLHost);
  }else{
    //check if URLRelative is really a relative url
    if (**URLRelative != '/'){ //not relative
      res = strdup(*URLRelative);    
    }else{
      URLHost = delProtocol(URLHost);
      slash = strchr(URLHost, '/');
      res = (char*)malloc((slash - URLHost + strlen(*URLRelative)) * sizeof(char));
      strncpy(res, URLHost, (slash-URLHost)/sizeof(char));
      res[slash-URLHost] = '\0';
      strcat(res, *URLRelative);
      free(URLHost);
    }
  }
  free(*URLRelative);

  
  *URLRelative = res;
}

/*  
* Get content type to know if this should be saved or not: easy handle, action options type selected
* Save the content if its type satisfy the condition: action 
* Parse the data to retrieve all URLs (data)
* for each URLs retrieve, check if it exists in tree to know if it should be insert or not to the tree: tree, depth
* new depth can be refound by find the handle's url in tree and then +1 
* If it's a new URL, need to create a new handle for it and add to the multi handle of task: multi handle
* After that cleanup the ez handle in argument
* 
*/ 
size_t write_cb(void *data, size_t size, size_t nmemb, LinkEasyMulti *linkHandles){

  size_t res;
  char *contentType;
  char *urlFound, *currURL;
  char *__data;
  int currDepth, max_depth;
  WrapAction *wrapper;

  //retrieve the content type 
  curl_easy_getinfo(linkHandles->easy, CURLINFO_CONTENT_TYPE, &contentType);

  //retrieve the URL of the curl that called write_cb
  curl_easy_getinfo(linkHandles->easy, CURLINFO_EFFECTIVE_URL, &currURL);

  //retrieve the wrapper
  curl_easy_getinfo(linkHandles->easy, CURLINFO_PRIVATE, &wrapper);

  //retrieve the depth of the current curl request
  currURL = delProtocol(currURL);
  currDepth = findNode(wrapper->root, currURL)->depth;

  //retrieve the max_depth of action
  max_depth = getMaxDepth(wrapper->action);

  //if the content type is one of those selected to save 
  //defined in the option of action then save data 
  if (isTypeSelected(contentType, wrapper->action)){
    res = saveData(data, size, nmemb, contentType, ".", currURL);
  }

  // find others URL from the content only if the content 
  // is of type text/html
  if (strstr(contentType, "text/html") != NULL && currDepth < max_depth){
    __data = (char*)data;    
    while ((urlFound = getURL(__data, &__data)) != NULL){
      reconstructURL(&urlFound, currURL);
      urlFound = delProtocol(urlFound);
      //check if the url found was parsed or not 
      //if not, make an easy curl for it 
      //add that curl to the multi curl of the task 
      //and insert the url into the tree 
      if (!URLAlrParsed(wrapper->root, urlFound)){
        insertURL(wrapper->root, urlFound, currDepth+1);
        add_transfer(linkHandles->multi, wrapper, urlFound);
      }free(urlFound);
    }
  }
  // curl_multi_remove_handle(linkHandles->multi, linkHandles->easy);
  // curl_easy_cleanup(linkHandles->easy);
  //delLink(linkHandles);
  return res;
}
 
void add_transfer(CURLM *cm, WrapAction *wrapper, char *url)
{
  CURL *eh;
  LinkEasyMulti *linkCurls;
  if (url == NULL) url = wrapper->action->url;
  
  eh = curl_easy_init();
  if (eh){
    linkCurls = initLink(eh, cm);
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, linkCurls);
    curl_easy_setopt(eh, CURLOPT_URL, url);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, (void*)wrapper);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
    curl_multi_add_handle(cm, eh);
  }
}


/**
 * Initialize a Node
 * @param suburl : the string of sub-link of the url 
 * @param depth : the depth of this link from the initial
 * @param next : its next sibling
 * @param child : its first child
 * @return : the node itself
 */
// void parseAction(Action *action){
//     Node tree;
//     int max_depth = -1;
//     int versionning = 0;
//     char **types = NULL;
//     int nbTypes = -1;

//     //retrieve options of the action 
//     for (int i = 0; i < action->nbOptions; ++i){
//         switch (action->options[i].type){
//             case MAX_DEPTH:
//                 max_depth = action->options[i].val.depth;
//                 break;
//             case VERSIONNING:
//                 versionning = action->options[i].val.shift;
//                 break;
//             case TYPESELECT:
//                 types = action->options[i].val.type.types;
//                 nbTypes = action->options[i].val.type.nbTypes;
//                 break;
//             default:
//                 break;
//         }
//     }
    
//     tree = makeTree(action->url);

//     parseURLRecursive(action->url, tree, 0, max_depth, versionning, types, nbTypes);

//     //save all URLs parsed
//     saveAllURLs(tree, action);
//     delTree(&tree);

// }

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


