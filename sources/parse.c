/*
**  Filename : parse.c
**
**  Made by : CAO Song Toan
**
**  Description :   Interface managing the parsing of website 
**                  determined by the configuration
*/
#ifndef __PARSE
#define __PARSE

#include <stdio.h>
#include <stdlib.h>
#include "configuration.h"
#include "url.h"



void parseActionRecursive(char *URLToParse, Node root, int depth, int max_depth, int versionning, char **types, int nbTypes){
    if (depth > max_depth) return;
    
    if (URLToParse == NULL || strlen(URLToParse) == 0) return;

    if (root == NULL) return;


}

/**
 * Initialize a Node
 * @param suburl : the string of sub-link of the url 
 * @param depth : the depth of this link from the initial
 * @param next : its next sibling
 * @param child : its first child
 * @return : the node itself
 */
void parseAction(Action *action){
    Node tree;
    int max_depth = -1;
    int versionning = 0;
    char **types = NULL;
    int nbTypes = -1;

    //retrieve options of the action 
    for (int i = 0; i < action->nbOptions; ++i){
        switch (action->options[i].type){
            case MAX_DEPTH:
                max_depth = action->options[i].val.depth;
                break;
            case VERSIONNING:
                versionning = action->options[i].val.shift;
                break;
            case TYPESELECT:
                types = action->options[i].val.type.types;
                nbTypes = action->options[i].val.type.nbTypes;
                break;
            default:
                break;
        }
    }
    
    tree = makeTree(action->url);

    parseURLRecursive(action->url, tree, 0, max_depth, versionning, types, nbTypes);

    //save all URLs parsed
    saveAllURLs(tree, "../data/", action->url);
    delTree(tree);

}

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