// > This functions allow to remove a substring in a string
char *strRemove(char *str, const char *sub);

// > This function allow to parse the content of the first url and get all the hyperlink that we can find in this content
void getUrl(char *outfilename);

// > This function allow to check if the content of our url file match with the different type mime present in the configuration file, then
// we download all of this hyperlink and placed in a unique folder
void parseUrlFile(char **types, int sizeTypes);
