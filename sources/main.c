
// #include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>
#include "url.h"
#include "configuration.h"
#include "parse.h"

 
#define MAX_PARALLEL 5
#define NUM_URLS sizeof(urls)/sizeof(char *)

 
int main(void)
{
  Configure *config = readConfigure("../sources/configure.sconf");
  Task *task = config->tasks[2];
  struct callback_data;
  CURLM *cm;
  CURLMsg *msg;
  unsigned int transfers = 0;
  int msgs_left = -1;
  int still_alive = 1;

  curl_global_init(CURL_GLOBAL_ALL);
  cm = curl_multi_init();
 
  /* Limit the amount of simultaneous connections curl should allow: */ 
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);
 
  // for(transfers = 0; transfers < task->nbActions; transfers++)
  //   add_transfer(cm, task->actions[transfers]);
  WrapAction *wrapper1 = initWrap(task->actions[0], makeTree(task->actions[0]->url));
  add_transfer(cm, wrapper1, NULL);
  do {
    curl_multi_perform(cm, &still_alive);
 
    while((msg = curl_multi_info_read(cm, &msgs_left))) {
      if(msg->msg == CURLMSG_DONE) {
        char *url;
        CURL *e = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);
        fprintf(stderr, "R: %d - %s <%s>\n",
                msg->data.result, curl_easy_strerror(msg->data.result), url);
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);

      }
      else {
        fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
      }
    }
    // if(still_alive)
    //   curl_multi_wait(cm, NULL, 0, 1000, NULL);
 
  } while(still_alive);
 
  curl_multi_cleanup(cm);
  curl_global_cleanup();


  return 0;
} 