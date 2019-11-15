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

#include <stdio.h>
#include <stdlib.h>
#include "configuration.h"
#include "url.h"


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