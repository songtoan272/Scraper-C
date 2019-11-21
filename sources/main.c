#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>
#include "url.h"
#include "configuration.h"
#include "parse.h"

 
int main(void)
{
  char *configName = writeConfig();
  Configure *config = readConfigure(configName);
  allMIMEs = initAllMIME();

  parseConfig(config);

  delConfigure(&config);
  delAllMIME(allMIMEs);
  free(configName);
  
  return 0;
} 