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

TypeMIME *allMIMEs;

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

/**
 * Create an array of all commun MIME types
 * by browsing through a website.
**/
TypeMIME *initAllMIME(){
  CURL *curl;
  FILE *fp;
  CURLcode res;
  TypeMIME *typesMime;
  char *ext = NULL, *typeMime = NULL, *startExt, *endExt, *startTypeMime, *endTypeMime;
  char buffer[BUFFER_SIZE];
  char *url = "https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types";
  char *outfilename = "MIME_Types.txt";
  int nb_types = 0;

  // Download the content of the website who refer the list of mime types
  curl = curl_easy_init();
  if (curl) {
    fp = fopen(outfilename,"wb");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
  }
  // Open our file to parse and get type mime and its extension
  fp = fopen(outfilename,"r");
  if(fp == NULL) {
    printf("Impossible d'accéder au fichier\n");
    exit(1);
  }

  typesMime = (TypeMIME*)malloc(NB_MIME_TYPES * sizeof(TypeMIME));
  // Get each line of html content
  while((fgets(buffer,BUFFER_SIZE, fp) != NULL)) {
    // Get line where we find an <td><code>. 
    //-> the line where we can find the extension
    if (strstr(buffer, "<td><code>.") != NULL) {
      ext = strstr(buffer, "<td><code>.");
      if (strstr(buffer, "<br>") == NULL){
        //there is no breakline 
        //-> only one extension for this MIME type
        startExt = ext + strlen("<td><code>");
        endExt = strstr(startExt, "</code></td>");
      }else{
        //There is a break line
        //-> there are 2 extensions possible for this MIME type
        //We take the second one in the following line
        fgets(buffer, BUFFER_SIZE, fp);
        startExt = strchr(buffer, '.');
        endExt = strstr(startExt, "</code></td>");
      }
      ext = strndup(startExt, endExt - startExt);
    }
    // Same use as for extension but we check here if we dont have a "."
    else if(strstr(buffer, "<td><code>") != NULL) {
      typeMime = strstr(buffer, "<td><code>");
      startTypeMime = typeMime + strlen("<td><code>");
      endTypeMime = strstr(startTypeMime, "</code>");
      typeMime = strndup(startTypeMime, endTypeMime - startTypeMime);
    }else if (strstr(buffer, "</tr>") != NULL && ext != NULL && typeMime != NULL){
      typesMime[nb_types].extension = ext;
      typesMime[nb_types].type = typeMime;
      // printf("%d. ext = %s\n",nb_types+1, typesMime[nb_types].extension);
      // printf("%d. type = %s\n", nb_types+1, typesMime[nb_types].type);
      nb_types++;
    // When we get the end of tbody we stop the process
    }else if(strstr(buffer, "</tbody>") != NULL) {
      break;
    }
  }
  fclose(fp);
  return typesMime;
}


void delAllMIME(TypeMIME *allMIME){
  for (int i = 0; i < NB_MIME_TYPES; i++){
    free(allMIME[i].extension);
    free(allMIME[i].type);
  }free(allMIME);
}

/**
 * From contentType, look for the corresponding extension
 * in the table allMIMEs.
 * (allMIMEs contains only the commun MIME types)
**/
char *getExtensionFromCt(char *contentType){
  for (int i = 0; i < NB_MIME_TYPES; i++){
    if (strstr(contentType, allMIMEs[i].type) != NULL){
      return allMIMEs[i].extension;
    }
  }
  return NULL;
}

/**
 * This function fix the extension in the name of a file
 * according to its type. 
 * If the file name alr has a valid extension then 
 * *fileName won't be changed.
 * Else, we create a new name with valid extension 
 * then assign this name to the string pointed by fileName
 * @param fileName: pointer to the string of fileName
 * @param contentType : the MIME type of the content 
 * to be saved
 * @return : this function does not return anything
 * the filename fixed (or not) will be assigned back
 * to the pointer passed in argument.
 **/ 
void fixExtension(char **fileName, char *contentType){
  char *validExt = getExtensionFromCt(contentType);
  char *pointExt;
  char *newName;

  if (validExt == NULL) {
    fprintf(stderr, "File %s is not of a commun MIME type.", *fileName);
    return;
  }

  pointExt = strstr(*fileName, validExt);
  if (pointExt != NULL){
    //the extension exists in fileName
    newName = strndup(*fileName, pointExt + strlen(validExt) - *fileName);
  }else{
    //the current fileName does not contain valid extension
    newName = (char*)malloc((strlen(*fileName) + strlen(validExt)) * sizeof(char));
    strcpy(newName, *fileName);
    strcat(newName, validExt);
  }
  free(*fileName);
  *fileName = newName;
}

/**
 * Get the value of max-depth option of the action
 * If the action does not have a max-depth option then return 0
**/
int getMaxDepth(Action *action){
  int res = 0;
  for (int i = 0; i < action->nbOptions; i++){
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

/**
 * Return the value of versionning of the action
 * If the action does not have versionning option, 
 * versionning will be considered "off".
**/ 
int getVersionning(Action *action){
  int res = 0;
  for (int i = 0; i < action->nbOptions; i++){
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

/**
 * Check if type (content type of an url) is one 
 * of the selected types of the action
 **/
int isTypeSelected(char *type, Action *action){
  char **typesSelected = NULL;
  int nbTypes = 0;

  for (int i = 0; i < action->nbOptions; i++){
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
    for (int i = 0; i < nbTypes; i++){
      if (strstr(type, typesSelected[i]) != NULL){
        return 1;
      }
    }
    return 0;
  }
}


/**
 * In a html script, there may be relative link 
 * which will direct back to a file in host link.
 * We need to reconstruct the relative url 
 * before initialize a curl_easy for it
 **/
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
  
//   url = strndup(startURL, endURL-startURL);
//   *dataLeft = endURL;
//   return url;
// }

/**
 * Reading through file f, retrieve all URLs and 
 * add these URLs to curl multi cm if the depth 
 * of URLOfFile < max-depth of the action
 **/
void getURLsFromFile(FILE *f, CURLM *cm, WrapAction *wrapper, char* URLOfFile){
  char buffer[BUFFER_SIZE], *copyBuffer;
  char *href, *src, *startURL, *endURL, *url;
  int currDepth, maxdepth;

  currDepth = findNode(wrapper->root, URLOfFile)->depth;
  maxdepth = getMaxDepth(wrapper->action);

  if (currDepth >= maxdepth) return;

  while (fgets(buffer, BUFFER_SIZE, f) != NULL){
    copyBuffer = buffer;
    href = strstr(copyBuffer, "href=\"");
    src = strstr(copyBuffer, "src=\"");
    while (href != NULL || src != NULL){
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
          break;
        }
      }
      if (startURL == NULL || endURL == NULL) break;
      url = strndup(startURL, endURL-startURL);
      reconstructURL(&url, URLOfFile);
      url = delProtocol(url);

      if (!URLAlrParsed(wrapper->root, url)){
        add_transfer(cm, wrapper, url);
        insertURL(wrapper->root, url, currDepth+1);
      }

      href = strstr(endURL+1, "href=\"");
      src = strstr(endURL+1, "src=\"");
    }
    
  }
}

/**
 * Extract the last part after '/' of url
 * This part will be used to name the file that
 * are saved the content of @param url
 **/
char *extractLastPart(char *url){
  char *slash = strrchr(url, '/'); //last occurence of / in url
  int size;
  if (slash == url + strlen(url) - 1){
    // www.abc.com/ the last / is useless
    //we have to retrieve the / before this one
    for (int i = strlen(url) - 2; i >= 0; --i){
      if (*(url + i) == '/'){
        slash = url + i;
        size = (url + strlen(url) - 1) - slash - 1;
        break;
      }
    }
  }else{
    size = (url + strlen(url)) - slash - 1;
  }

  return strndup(slash + 1, size);
}


/**
 * Generate a path to save the content returned by libcurl
 * Create directories if necessary 
 * Path will be of format: 
 * scrapper/data/name of Action/type of content (text, image,..)/name of file with extension
 * This function return the pointer to the opened file
 **/
char *makeFilePath(Action *action, char *contentType, char *url){
  char *filePath, *nameFile, *type, *actionName, command[200];
  actionName = strdup(action->name);
  strcpy(command, "mkdir -p ");
  for (char *c = actionName; *c != '\0'; c++){
    if (*c == ' ') *c = '_';
  }

  nameFile = extractLastPart(url);
  type = strchr(contentType, '/');
  type = strndup(contentType, type - contentType);
  fixExtension(&nameFile, contentType);
  filePath = (char*)malloc((strlen("../data/") + strlen(actionName) + strlen(type) + strlen(nameFile) + 3) * sizeof(char));
  strcpy(filePath, "../data/");
  strcat(filePath, actionName);
  strcat(filePath, "/");
  strcat(filePath, type);
  strcat(filePath, "/");
  //create directories
  strcat(command, filePath);
  system(command);

  strcat(filePath, nameFile);

  free(type);
  free(nameFile);
  free(actionName);
  return filePath;
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
size_t write_cb(void *data, size_t size, size_t nmemb, CURL *easy){
  size_t res;
  FILE *f;
  char *contentType;
  char *currURL;
  char *filePath;
  WrapAction *wrapper;

  //retrieve the content type 
  curl_easy_getinfo(easy, CURLINFO_CONTENT_TYPE, &contentType);

  //retrieve the URL of the curl that called write_cb
  curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &currURL);

  //retrieve the wrapper
  curl_easy_getinfo(easy, CURLINFO_PRIVATE, &wrapper);

  //if the content type is one of those selected to save 
  //defined in the option of action then save data 
  //we also save all html file even if text/html is not
  //a selected type in order to find URLs in it after
  if (isTypeSelected(contentType, wrapper->action) || strstr(contentType, "text/html") != NULL){
    filePath = makeFilePath(wrapper->action, contentType, currURL);
    f = fopen(filePath, "a");
    if (f == NULL) return 0; 
    res = fwrite(data, size, nmemb, f);
    free(filePath);
    fclose(f);
  }
  return res;
}
 
void add_transfer(CURLM *cm, WrapAction *wrapper, char *url)
{
  CURL *eh;
  if (url == NULL) url = wrapper->action->url;
  
  eh = curl_easy_init();
  if (eh){
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, eh);
    curl_easy_setopt(eh, CURLOPT_URL, url);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, (void*)wrapper);
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
    curl_multi_add_handle(cm, eh);
  }
}


void parseATask(Task *task){
  CURLM *cm;
  CURLMsg *msg;
  CURLMcode res;
  CURL *ce; 
  int msgs_left = -1;
  int still_alive = 1;
  WrapAction *wrappers[task->nbActions];
  WrapAction *wrapper;
  int currDepth, maxDepth;
  long timeout;
  char *url, *contentType, *filePath;
  FILE *f;

  curl_global_init(CURL_GLOBAL_ALL);
  cm = curl_multi_init();

  if (cm == NULL){
    fprintf(stderr, "Cannot initialize curl_multi.\n");
    exit(1);
  }

  //Limit the amount of simultaneous connections curl should allow:
  curl_multi_setopt(cm, CURLOPT_MAXCONNECTS, 10 * task->nbActions);

  //add URLs from actions of the task to curl_multi handle
  for (int i = 0; i < task->nbActions; i++){
    wrappers[i] = initWrap(task->actions[i], makeTree(task->actions[i]->url));
    add_transfer(cm, wrappers[i], NULL);
  }

  do {
    res = curl_multi_perform(cm, &still_alive);
    if(res != CURLM_OK) {
      fprintf(stderr, "curl_multi failed, code %d.\n", res);
      break;
    }
    while ((msg = curl_multi_info_read(cm, &msgs_left))){
      if (msg->msg == CURLMSG_DONE) {
        //retrieve needed infos
        ce = msg->easy_handle;
        curl_easy_getinfo(ce, CURLINFO_PRIVATE, &wrapper);
        curl_easy_getinfo(ce, CURLINFO_CONTENT_TYPE, &contentType);
        curl_easy_getinfo(ce, CURLINFO_EFFECTIVE_URL, &url);
        //print out message
        fprintf(stderr, "R: %d - %s <%s>\n",
                msg->data.result, curl_easy_strerror(msg->data.result), url);

        if (msg->data.result != 0) continue;
        //if the content type is text/html 
        //then we need to parse the saved data 
        //to retrieve all URLs
        if (strstr(contentType, "text/html")){
          url = delProtocol(url);
          maxDepth = getMaxDepth(wrapper->action);
          currDepth = findNode(wrapper->root, url)->depth;
          if (currDepth < maxDepth){
            filePath = makeFilePath(wrapper->action, contentType, url);
            f = fopen(filePath, "r");
            if (f == NULL) continue;
            getURLsFromFile(f, cm, wrapper, url);
            fclose(f);
            
            //if the content type (text/html) is actually
            //not a selected type then delte the file
            if (!isTypeSelected(contentType, wrapper->action)){
              remove(filePath);
            }
            free(filePath);
          }
        }
        curl_multi_remove_handle(cm, ce);
        curl_easy_cleanup(ce);
      }
      else{
        fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
        curl_multi_remove_handle(cm, msg->easy_handle);
        curl_easy_cleanup(msg->easy_handle);
      }
    }
    curl_multi_timeout(cm, &timeout);
    if (timeout < 0) curl_multi_wait(cm, NULL, 0, 1000, NULL);
    else if (timeout == 0) curl_multi_perform(cm, &still_alive);
    else curl_multi_wait(cm, NULL, 0, (int)timeout, NULL);
  }while (still_alive);

  //clean up and free space
  for (int i = 0; i < task->nbActions; i++) delWrap(&(wrappers[i]));
  curl_multi_cleanup(cm);
  curl_global_cleanup();
}

void parseConfig(Configure *config){
  for (int i = 0; i < config->nbTask; i++){
    parseATask(config->tasks[i]);
  }
}

