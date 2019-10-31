/*
**  Filename : configuration.c
**
**  Made by : CAO Song Toan
**
**  Description : Manage the to parse the configuration file
*/
#ifndef __CONFIGURATION
#define __CONFIGURATION

#include <stdio.h>
#include <stdlib.h>

enum optionType{MAX_DEPTH, VERSIONNING, TYPESELECT};

union optionVal{
    int depth;          //>=0; value if the chosen option is MAX_DEPTH 
    int shift;          //value if the chosen option is VERSIONNING 
                        //off=0, on=1
    char **types;       //array of string, each string is a type 
                        //if the chosen option is TYPESELECT
};

typedef struct option{
    enum optionType type;
    union optionVal val;
}Option;

typedef struct action{
    char *name;
    char *url; 
    Option *options;    //array of Option, these options are exclusive to one action
}Action;

typedef struct timeLaunch{
    int sec;
    int min; 
    int hour;
}TimeLaunch;

typedef struct task{
    char *name;
    TimeLaunch time;
    int nbActions; 
    Action **actions;   //array of Action pointers, each action can be performed by different tasks 
                        //Therefore we only use pointers to Action as these actions are shared
}Task;

typedef struct configure{
    int nbTask;         //number of tasks in the configuration file
    Task **tasks;       //array of all tasks in this configure
}Configure;

/**
 * Initialize an Option
 * @param opt : pointer on an Option which has to be fully initialized at the end of the function
 * @param optType : type of the option
 * @param optVal: value of the option
 * @return : nothing, the option is modified directly by the pointer passed in parameters
 */
void initOption(Option *opt, enum optionType optType, union optionVal optVal);


/**
 * Initialize an Action
 * @param nameAct : name of the action
 * @param urlAct : url to scrappe of this action
 * @param optTypes: an array of types of this action's options
 * @param optVals: an array of values of this actions's options
 * @param sizeOpt : the size of optTypes and optVals
 * @return : a pointer on the initialized action
 */
Action *initAction(char *nameAct, char *urlAct, enum optionType *optTypes, union optionVal optVals, int sizeOpt);

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
Task *initTask(char *nameTask, int sec, int min, int hour, Action **allActions, char **nameActions, int totalActs, int nbActs);

/**
 * Read in a configuration 
 * @param filePath : path of the configuration file
 * @return : a pointer on the initialized configure
 */
Configure *readConfigure(char *filePath);

void delAction(Action **action);

void delTask(Task **task);

void delConfigure(Configure **config);

#endif