/*
**  Filename : configuration.c
**
**  Made by : CAO Song Toan
**
**  Description : Manage the to parse the configuration file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"


/*****************CONTRUCTION************************/
/**
 * Initialize an Option
 * @param opt : pointer on an Option which has to be fully initialized at the end of the function
 * @param optType : type of the option
 * @param optVal: value of the option
 * @return : nothing, the option is modified directly by the pointer passed in parameters
 */
void initOption(Option *opt, OptionType optType, OptionVal optVal){
    if (opt == NULL){
        fprintf(stderr, "Option has not been initialised correctly.\n");
        exit(1);
    }

    opt->type = optType;
    switch (opt->type){
        case MAX_DEPTH:
            opt->val.depth = optVal.depth;
            break;
        case VERSIONNING:
            opt->val.shift = optVal.shift;
            break;
        case TYPESELECT:
            opt->val.type.nbTypes = optVal.type.nbTypes;
            opt->val.type.types = optVal.type.types;
            break;
        default:
            fprintf(stderr, "Invalid type of option.\n");
            break;
    }    
}


/**
 * Initialize an Action
 * @param nameAct : name of the action
 * @param urlAct : url to scrappe of this action
 * @param optTypes: an array of types of this action's options
 * @param optVals: an array of values of this actions's options
 * @param sizeOpt : the size of optTypes and optVals
 * @return : a pointer on the initialized action
 */
Action *initAction(char *nameAct, char *urlAct, OptionType *optTypes, OptionVal *optVals, int sizeOpt){
    if (nameAct == NULL || strlen(nameAct) == 0){
        fprintf(stderr, "Name of action unavailable.\n");
        exit(1);
    }

    if (urlAct == NULL || strlen(urlAct) == 0){
        fprintf(stderr, "URL of action unavailable.\n");
        exit(1);
    }

    Action *res = (Action*)malloc(sizeof(Action));

    res->name = strdup(nameAct); 
    res->url = strdup(urlAct);
    res->nbOptions = sizeOpt;
    res->options = (Option*)malloc(sizeOpt * sizeof(Option));

    //fill in the options of this action
    for (int i = 0; i < sizeOpt; i++){
        initOption(res->options + i, optTypes[i], optVals[i]);
    }

    return res;
}

/**
 * Initialize an Task
 * @param nameTask : name of the task
 * @param sec, min, hour : time parameters to initialize the property time of the task
 * @param allActions: an array of all actions in the configuration
 * @param nameActions: an array of names of actions which are executed by this task
 * @param totalActs : the total number of actions in the configuration file
 * @param nbActs: the number of action this task executes = size of nameActions
 * @return : a pointer on the initialized task
 */
Task *initTask(char *nameTask, int sec, int min, int hour, Action **allActions, char **nameActions, int totalActs, int nbActs){
    if (nameTask == NULL || strlen(nameTask) == 0){
        fprintf(stderr, "Name of task unavailable.\n");
        exit(1);
    }

    if (nameActions == NULL){
        fprintf(stderr, "Names of actions unavailable to create a task.\n");
        exit(1);
    }

    if (allActions == NULL){
        fprintf(stderr, "No actions available at all.\n");
        exit(1);
    }

    if (nameActions == NULL){
        fprintf(stderr, "No actions available to create a task.\n");
        exit(1);
    }

    if (sec < 0 || min < 0 || hour < 0){
        fprintf(stderr, "Time invalid to create task.\n");
        exit(1);
    }

    if (nbActs > totalActs){
        fprintf(stderr, "The number of actions to be executed by this task exceeds the total number of actions available.\n");
        exit(1);
    }

    Task *res = (Task*)malloc(sizeof(Task));

    res->name = strdup(nameTask);
    res->nbActions = nbActs;
    res->actions = (Action**)malloc(res->nbActions * sizeof(Action*));
    res->time.hour = hour;
    res->time.min = min;
    res->time.sec = sec;

    //fill in the array of actions executed by this task
    for (int i = 0; i < res->nbActions; i++){
        for (int j = 0; j < totalActs; j++){
            if (strcmp(nameActions[i], allActions[j]->name) == 0){
                res->actions[i] = allActions[j];
                break;
            }
        }
    }

    //free nameActions as it will not be needed afterward
    for (int i = 0; i < res->nbActions; i++){
        free(nameActions[i]);
    }
    free(nameActions);

    return res;
}


/*****************READ CONFIGURATION FILE************************/
/**
 * Count the number of actions and tasks in a configuration file
 * @param f : pointer to configuration file whose position is at the beginning of the file
 * @param nbTasks : pointer to an int pointer representing the number of tasks
 * @param nbActions : pointer to an int pointer representing the number of actions
 * @return : nothing, the results are saved in 2 int pointers passed by argument
 */
void countNbTasksActions(FILE *f, int *nbTasks, int *nbActions){
    char buffer[2100];
    while (fgets(buffer, 2100, f) != NULL){
        if (strstr(buffer, "==") != NULL){
            (*nbTasks)++;
        }else if (strchr(buffer, '=') != NULL){
            (*nbActions)++;
        }
    }
}

/**
 * Get the key and value from a pair of association
 * @param buffer : the string from where retrieved are the key and the value
 * @param key : pointer to a string representing the key (alr statically allocated)
 * @param value : pointer to a string representing the value (alr statically allocated)
 * @return : nothing, the results are saved in 2 string passed by argument
 */
void getKeyValue(char *buffer, char *key, char *value){
    char *startKey = strchr(buffer, '{') + 1;
    char *endKey = strchr(buffer, ' ');
    char *startVal = strchr(buffer, '>') + 2;
    char *endVal = strchr(buffer, '}');
    int lengthKey = endKey - startKey;
    int lengthVal = endVal - startVal;

    //reset the strings key and value
    //before copy new values to them
    memset(key, '\0', 20);
    memset(value, '\0', 2000);

    //copy from buffer to key and value
    strncpy(key, startKey, lengthKey);
    strncpy(value, startVal, lengthVal);
}

/**
 * Verify the time launch whether it is valid or not
 * @param h : int pointer to the number of hours
 * @param m : int pointer to the number of minutes
 * @param s : int pointer to the number of seconds
 * @return : nothing, modify directly 3 pointers
 */
void verifyTime(int *h, int *m, int *s){
    if (*h < 0) *h = 0;
    if (*m < 0) *m = 0;
    if (*s < 0) *s = 0;

    if (*s > 60){
        *m += (*s / 60);
        *s %= 60;
    }

    if (*m > 60){
        *h += (*m / 60);
        *m %= 60;
    }
}

/**
 * Split a string into several substrings by certain delimiters
 * This function is only used to split string of format (sub1, sub2,...)
 * @param str : the string to get splited (unchanged when exiting the function)
 * @param nbElem : the int pointer to save the number of substrings found
 * @return : an array of substrings
 */
char **splitString(char *str, int *nbElem){
    char **res = NULL;
    char *buffer;
    const char delims[4] = ",()";
    char *token;
    int idx = 0;

    *nbElem = 0;
    //get the 1st token
    buffer = strdup(str);
    token = strtok(buffer, delims);

    //walk through all tokens to count 
    //the number of elements in str
    while (token != NULL){
        char *c;
        for (c = token; (*c) == ' ' || (*c) == '\n' || (*c) == '\r'; c++);
        if (strlen(c) != 0) (*nbElem)++;
        token = strtok(NULL, delims);
    }
    
    //allocate the array of substrings
    res = (char**)malloc((*nbElem) * sizeof(char*));

    //fill the array of substrings
    free(buffer); free(token);
    buffer = strdup(str);
    token = strtok(buffer, delims);
    while (token != NULL){
        //remove the heading spaces 
        char *c;
        for (c = token; (*c) == ' ' || (*c) == '\n' || (*c) == '\r'; c++);
        if (strlen(c) != 0){
            res[idx] = strdup(c);
            idx++;
        }
        token = strtok(NULL, delims);
    }

    free(buffer); free(token);
    return res;
}

/**
 * Read in and create all actions in the configuration file
 * This function works theoritically even if actions and tasks are written alternatively
 * @param f : the configuration file
 * @param nbActions : the number of actions counted from the configuration file
 * @param allActions : an array of action pointers where we save all the pointers found
 * @return : nothing, the results are saved in allActions
 */
void readInActions(FILE *f, int nbActions, Action **allActions){
    int noCurrAct = 0; 
    int isAnAction = 0; 
    int isOpt = 0;
    char buffer[2000];
    char key[20];
    char value[2000];
    char *name = NULL;
    char *url = NULL;
    //each action can have maximum 3 options
    OptionType currOptTypes[3];
    OptionVal currOptVal[3];
    int nbOpts;

    while (fgets(buffer, 2100, f) != NULL && noCurrAct < nbActions){
        if (strstr(buffer, "==") != NULL){
            //found the beginning of a task
            //save the last action parsed if there is one
            if (isAnAction){
                allActions[noCurrAct] = initAction(name, url, currOptTypes, currOptVal, nbOpts);
                free(name); name = NULL;
                free(url); url = NULL;
                nbOpts = 0;
                isOpt = 0;
                noCurrAct++;
            }            
            isAnAction = 0; 
            continue;
        }else if (strchr(buffer, '=') != NULL){
            //found the beginning of an action
            if (isAnAction){
                //alr parsed through a preceding action
                //save this action and reset all parameter variables
                allActions[noCurrAct] = initAction(name, url, currOptTypes, currOptVal, nbOpts);
                free(name); name = NULL;
                free(url); url = NULL;
                nbOpts = 0;
                isOpt = 0;
                noCurrAct++;
            }
            isAnAction = 1;
            continue;
        }else if (strchr(buffer, '+') != NULL && isAnAction){
            //found the beginning of options bloc
            // countNbOptions(f, &nbOpts);
            isOpt = 1;
            continue;
        }else if (strchr(buffer, '{') != NULL && isAnAction){
            getKeyValue(buffer, key, value);
            if (isOpt == 0){
            //a pair of key-value but not an option
                if (strcmp(key, "name") == 0){
                    name = strdup(value);
                }
                else if (strcmp(key, "url") == 0){
                    url = strdup(value);
                }else{
                    fprintf(stderr, "Undefined champ of an action: {%s -> %s}\n", key, value);
                    exit(1);
                }
            }else if (isOpt == 1){
                //a pair of key-value which is an option
                if (strcmp(key, "max-depth") == 0){
                    currOptTypes[nbOpts] = MAX_DEPTH;
                    currOptVal[nbOpts].depth = atoi(value); //an int
                    nbOpts++;
                }else if (strcmp(key, "versionning") == 0){
                    currOptTypes[nbOpts] = VERSIONNING;
                    currOptVal[nbOpts].shift = strcmp(value, "on") == 0 ? 1:0;
                    nbOpts++;
                }else if (strcmp(key, "type") == 0){
                    currOptTypes[nbOpts] = TYPESELECT;
                    int nbTypes;
                    char **allTypes = splitString(value, &nbTypes);
                    currOptVal[nbOpts].type.nbTypes = nbTypes;
                    currOptVal[nbOpts].type.types = allTypes;
                    nbOpts++;
                }else{
                    fprintf(stderr, "Undefined option of an action: {%s -> %s}\n", key, value);
                    exit(1);
                }
            }
            continue;        
            
        }
    }
    if (name != NULL){
        allActions[noCurrAct] = initAction(name, url, currOptTypes, currOptVal, nbOpts);
        free(name); name = NULL;
        free(url); url = NULL;
    }
}

/**
 * Read in and create all tasks in the configuration file
 * This function works theoritically even if actions and tasks are written alternatively
 * @param f : the configuration file
 * @param allTasks: an array of tasks pointers where we save all the tasks found
 * @param nbTasks: the number of tasks counted from the configuration file
 * @param allActions : an array of action pointers where we save all the pointers found
 * @param nbActions : the number of actions counted from the configuration file
 * @return : nothing, the results are saved in allActions
 */
void readInTasks(FILE *f, Task **allTasks, int nbTasks, Action **allActions, int nbActions){ 
    int noCurrTask = 0; 
    int isATask = 0; 
    int nbOpts = 0;
    char buffer[2100];
    char key[20];
    char value[2000];
    char *name = NULL;
    int h = 0, m = 0, s = 0;
    char **nameActions; 

    while (fgets(buffer, 2100, f) != NULL && noCurrTask < nbTasks){
        if (strstr(buffer, "==") != NULL){
            //found the beginning of a task
            if (isATask){
                //not the first task parsed
                //save the preceding parsed task
                //then reset all the parameter variables
                verifyTime(&h, &m, &s);
                allTasks[noCurrTask] = initTask(name, s, m, h, allActions, nameActions, nbActions, nbOpts);
                free(name); name = NULL;
                h = 0, m = 0, s = 0;
                nbOpts = 0;
                noCurrTask++;
            }
            isATask = 1; 
            continue;
        }else if (strchr(buffer, '=') != NULL){
            //found the beginning of an action
            if (isATask){
                //alr parsed through a preceding action
                //save this action and reset all parameter variables
                verifyTime(&h, &m, &s);
                allTasks[noCurrTask] = initTask(name, s, m, h, allActions, nameActions, nbActions, nbOpts);
                free(name); name = NULL;
                h = 0, m = 0, s = 0;
                nbOpts = 0;
                noCurrTask++;
            }
            isATask = 0; 
            continue;
        }else if (isATask){ //only execute if sure that this is in a bloc of a task
            if (strchr(buffer, '+') != NULL){
                //found the beginning of options bloc
                fgets(buffer, 2100, f); //read the following line
                nameActions = splitString(buffer, &nbOpts);
                continue;
            }else if (strchr(buffer, '{') != NULL){
                //found a champ of the current task
                getKeyValue(buffer, key, value);
                if (strcmp(key, "name") == 0){
                    name = strdup(value);
                }else if (strcmp(key, "hour") == 0){
                    h = atoi(value);
                }else if (strcmp(key, "minute") == 0){
                    m = atoi(value);
                }else if (strcmp(key, "second") == 0){
                    s = atoi(value);
                }else{
                    fprintf(stderr, "Undefined champ of a task: {%s -> %s}.\n", key, value);
                    exit(1);
                }continue;        
            }
        }
    }
    if (name != NULL){
        verifyTime(&h, &m, &s);
        allTasks[noCurrTask] = initTask(name, s, m, h, allActions, nameActions, nbActions, nbOpts);
        free(name); name = NULL;
    }
}

/**
 * Read in a configuration 
 * @param filePath : path of the configuration file
 * @return : a pointer on the initialized configure
 */
Configure *readConfigure(char *filePath){
    FILE *configuration;
    Configure *res;
    int nbTasks = 0;
    int nbActions = 0;

    configuration = fopen(filePath, "r");
    if (configuration == NULL){
        fprintf(stderr, "Failed to open configuration file.\n");
        exit(1);
    }

    res = (Configure*)malloc(sizeof(Configure));

    //count the nb of actions and tasks in configuration file
    countNbTasksActions(configuration, &nbTasks, &nbActions);

    //allocate memory for arrays of tasks and of actions
    res->nbActions = nbActions;
    res->actions = (Action**)malloc(res->nbActions * sizeof(Action*));
    res->nbTask = nbTasks;
    res->tasks = (Task**)malloc(res->nbTask * sizeof(Task*));

    //read in actions and tasks
    fseek(configuration, 0, SEEK_SET);
    readInActions(configuration, nbActions, res->actions);
    fseek(configuration, 0, SEEK_SET);
    readInTasks(configuration, res->tasks, nbTasks, res->actions, nbActions);
    

    fclose(configuration);
    return res;
}


/*****************DELETION************************/

void delAction(Action **action){
    free((*action)->name);
    free((*action)->url);
    for (int i = 0; i < (*action)->nbOptions; ++i){
        switch ((*action)->options[i].type){
            case TYPESELECT:
                for (int j = 0; j < (*action)->options[i].val.type.nbTypes; ++j){
                    free((*action)->options[i].val.type.types[j]);
                }
                free((*action)->options[i].val.type.types);
                break;
            default:
                break;
        }
    }
    free((*action)->options);
    free((*action));
    *action = NULL;
}

void delTask(Task **task){
    free((*task)->name);
    free((*task)->actions);
    free((*task));
    *task = NULL;
}

void delConfigure(Configure **config){
    for (int i = 0; i < (*config)->nbActions; ++i){
        delAction((*config)->actions+i);
    }
    free((*config)->actions);

    for (int i = 0; i < (*config)->nbTask; ++i){
        delTask((*config)->tasks+i);
    }
    free((*config)->tasks);

    free((*config));
    *config = NULL;
}


/*****************PRINT************************/

void printAction(Action *action){
    printf("Name: %s\n", action->name);
    printf("URL: %s\n", action->url);
    printf("This action has %d options:\n", action->nbOptions);
    for (int i = 0; i < action->nbOptions; ++i){
        switch (action->options[i].type){
            case MAX_DEPTH:
                printf("\tmax-depth = %d\n", action->options[i].val.depth);
                break;
            case VERSIONNING:
                printf("\tversionning = %s\n", action->options[i].val.shift == 0 ? "off":"on");
                break;
            case TYPESELECT:
                printf("\ttype = {");
                for (int j = 0; j < action->options[i].val.type.nbTypes - 1; ++j){
                    printf("%s, ", action->options[i].val.type.types[j]);
                }
                //print the last type
                printf("%s}\n", action->options[i].val.type.types[action->options[i].val.type.nbTypes-1]);
        }
    }
}

void printTask(Task *task){
    printf("Name: %s\n", task->name);
    printf("Time launch: %dh %dm %ds\n", task->time.hour, task->time.min, task->time.sec);
    printf("This task executes %d actions:\n", task->nbActions);
    for (int i = 0; i < task->nbActions; ++i){
        printf("\t%s\n", task->actions[i]->name);
    }
}

void printConfig(Configure *config){
    printf("There are %d actions in the configure:\n", config->nbActions);
    for (int i = 0; i < config->nbActions; ++i){
        printf("%d.\n", i+1);
        printAction(config->actions[i]);
    }

    printf("\n\nThere are %d tasks in the configure:\n", config->nbTask);
    for (int i = 0; i < config->nbTask; ++i){
        printf("%d.\n", i+1);
        printTask(config->tasks[i]);
    }
}



/*****************WRITE CONFIGURATION************************/

/**Clear the newline character in stdin buffer**/
void clearNewline(){
    getc(stdin);
}

/**
 * This function allows users to set up their own configuration file.
 * @return : the name of the configuration file (.sconf) that are written on hard disk
 */
char *writeConfig(){
    char nameFile[100], fullPath[200];
    int key, choice, valueInt; 
    char value[2000];
    FILE *config;

    strcpy(fullPath, "../configure/");
    //Ask the name of the configuration
    printf("How do you want to name this configuration?\n");
    scanf("%[^\n]", nameFile);
    strcat(nameFile, ".sconf");
    strcat(fullPath, nameFile);
    clearNewline();

    //Open the configuration file to write in
    config = fopen(nameFile, "w");
    if (config == NULL){
        fprintf(stderr, "Cannot open file to write in configure.\n");
        exit(1);
    }

    do{
        printf("Do you want to set up an Action(1) or a Task(2) or terminate(0)?\nPlease fill in the corresponding number:");
        scanf("%1d", &choice);
        clearNewline();
        switch (choice){
            case 0: break;
            case 1: //write an action
                fprintf(config, "%s\n", "=");
                //ask for name of action
                printf("How do you want to name this action?\n");
                scanf("%[^\n]", value);
                fprintf(config, "{name -> %s}\n", value);
                clearNewline();
                //ask for url of action
                printf("What URL to be parsed by this action?\n");
                scanf("%[^\n]", value);
                fprintf(config, "{url -> %s}\n", value);
                clearNewline();
                //ask for options of this action
                printf("Do you want to add options to this action?\n (1 = YES, else = NO)\n");
                scanf("%1d", &key);
                clearNewline();
                switch (key){
                    case 1:
                        fprintf(config, "%s\n", "+");

                        int nbOptions;
                        printf("How many options do you want to set up for this action?\n");
                        scanf("%d", &nbOptions);
                        clearNewline();

                        for (int i = 0; i < nbOptions; ++i){
                            printf("Which type of option is the option %d?\n", i+1);
                            printf("Please enter the corresponding number.\n");
                            printf("1. max-depth\n");
                            printf("2. versionning\n");
                            printf("3. type\n");
                            scanf("%1d", &key);
                            clearNewline();
                            switch (key){
                                case 1:
                                    printf("What is the value of max-depth?\n");
                                    scanf("%d", &valueInt);
                                    fprintf(config, "{max-depth -> %d}\n", valueInt);
                                    clearNewline();
                                    break;
                                case 2:
                                    printf("What is the value of versionning? (1 = ON, else = OFF)\n");
                                    scanf("%d", &valueInt);
                                    fprintf(config, "{versionning -> %s}\n", valueInt == 1 ? "on":"off");
                                    clearNewline();
                                    break;
                                case 3:
                                    printf("How many types do you want to select?\n");
                                    scanf("%d", &valueInt); clearNewline();
                                    printf("Please note that only MIME types are accepted.\n");
                                    printf("Any violation will result in crashing the scrapper.\n");

                                    //read from stdin the types selected by user
                                    fprintf(config, "{type -> (");
                                    for (int j = 0; j < valueInt; ++j){
                                        printf("Type %d = ", j+1);
                                        scanf("%[^\n]", value);
                                        fprintf(config, "%s, ", value);
                                        clearNewline();
                                    }
                                    //rewind 2 steps in the config file
                                    fseek(config, -2, SEEK_CUR);
                                    fprintf(config, ")}\n");
                                    break;
                                default:
                                    printf("Wrong value, no options available.\n");
                                    --i;
                                    break;
                            }
                        }
                        break;
                    default:
                        printf("No options will be added to this action.\n");
                        break;
                }
                //print a newline mark the end of the action
                fprintf(config, "\n");
                break;
            
            case 2: //write a task
                fprintf(config, "%s\n", "==");
                //ask for name of task
                 printf("How do you want to name this task?\n");
                scanf("%[^\n]", value);
                fprintf(config, "{name -> %s}\n", value);
                clearNewline();
                //ask for time between requests
                printf("How much time do you want to rest between 2 requests?\n");
                printf("Hour = ");
                scanf("%d", &valueInt); clearNewline();
                if (valueInt > 0) fprintf(config, "{hour -> %d}\n", valueInt);
                printf("Minute = ");
                scanf("%d", &valueInt); clearNewline();
                if (valueInt > 0) fprintf(config, "{minute -> %d}\n", valueInt);
                printf("Second = ");
                scanf("%d", &valueInt); clearNewline();
                if (valueInt > 0) fprintf(config, "{second -> %d}\n", valueInt);
                //ask for actions to execute
                fprintf(config, "%s\n", "+");
                printf("How many actions do you want to be executed by this task?\n");
                scanf("%d", &valueInt);
                clearNewline();
                //read from stdin actions to be executed
                printf("Please note that the action you ask this task to execute must be present in this configuration file.\n");
                printf("Any unknown actions could result in the malfunction of the scrapper.\n");
                fputc('(', config);
                for (int i = 0; i < valueInt; ++i){
                    printf("Action %d = ", i+1);
                    scanf("%[^\n]", value);
                    fprintf(config, "%s, ", value);
                    clearNewline();
                }
                //rewind 2 steps in the configuration file
                fseek(config, -2, SEEK_CUR);
                fputs(")\n", config);
                //print a newline mark the end of the task
                fprintf(config, "\n");
                break;
            
            default:
                printf("Wrong value! Type in 0 or 1 or 2, how hard could that be?\n");
                break;
        }
    }while (choice != 0);

    fclose(config);
    return strdup(fullPath);
}