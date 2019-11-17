/*
**  Filename : url.h
**
**  Made by : CAO Song Toan
**
**  Description : Manage all the URL parsed in an action by a structure of tree 
**              - Each node of the tree only stock a sub-link of the url.
**              We retrieve the full URL of a node by browsing from the root 
**              to the node and concatenate all the sublinks together.
**              - Each node can have a set of child nodes.
**              - All child nodes of a node are of the same level and 
**              linked by a structure of alphabetically ordered linked list.
**              - The root of the tree will have the url "" - an empty string,
**              whose depth equals -1 and has no sibling node.
**              
**              The tree structure helps keeping track of all parsed URL without
**              consuming much of memory as each node only saves a part of the
**              entire URL.
**              Divide URLs into small parts and arrange nodes orderly helps 
**              navigating the right node quick.
*/
#ifndef __URL
#define __URL

#include <stdio.h>
#include <stdlib.h>
#include "configuration.h"

struct __node{
    char *url;                  //the last part after a '/' of an url
    int depth;                  //the depth of this link from the initial
                                //link whose depth = 0
    struct __node *nextSibling;   //pointer to the next sibling node on the same level 
    struct __node *firstChild;    //pointer to the first child node of this node
}; 

typedef struct __node* Node; 



/**
 * Initialize a Node
 * @param suburl : the string of sub-link of the url 
 * @param depth : the depth of this link from the initial
 * @param next : its next sibling
 * @param child : its first child
 * @return : the node itself
 */
Node initNode(char *suburl, int depth, Node next, Node child);

/**
 * delete the protocol part in the URL 
 * @param URL : the URL to be modified
 * @return : the pointer to the modified URL
 */
char *delProtocol(char *url);

/**
 * Make a tree from the very initial url 
 * @param url : the initial url of the tree
 * @return : the root of the tree
 */
Node makeTree(char *url);

/**
 * Compare two nodes by their url 
 * @param node1, node2 : 2 nodes to be compared
 * @return : 0 if their url are equals 
 *          -1 if node1->url < node2->url
 *           1 if node1->url > node2->url
 */
int compareNode(Node node1, Node node2);

/**
 * Insert a node into the tree
 * @param root : the root of the tree
 * @param urlToInsert : the url of the node to be inserted
 * @param depth : the depth of the inserted node from the initial url
 * @return : nothing as the root always stay the same 
 */
void insertURL(Node root, char *URL, int depth);

/**
 * Delete (free) a node
 * @return : nothing
 * the value of the case to which pNode point turned to NULL
 */
void delNode(Node *pNode);

/**
 * Delete (free) the whole tree
 * @return : the value of the case to which pNode point turned to NULL
 */
void delTree(Node *pRoot);

/**
 * Find the node corresponding to an URL
 * (the node contains the last part of the URL in the tree)
 * @param upperNode : the curren parent node from where
 * we descend to find the last node of the URL
 * @param subURL: a part of the initial URL that we use 
 * to find the last node of the URL
 * @return : the last node if it exists in the tree
 *          NULL if not
 */
Node findNode(Node upperNode, char *subURL);

/**
 * Verify if an URL has already been parsed
 * @param root : the root of the tree
 * @param URL : the URL to be verified
 * @return : 1 if the URL was parsed
 *           0 if not
 */
int URLAlrParsed(Node root, char *URL);

/**
 * Print out the whole tree
 * @param root : the root of the tree
 * @return :
 */
void printTree(Node root);

/**
 * Browse through the tree to find and save all 
 * the parsed URLs into a file 
 * @param root : root of the tree
 * @param filePath : the path (the folder) to
 * save the file
 * @param URL : the initial URL given by the action
 * we use this URL as the name of the file
 * @return : nothing (a file is created and saved)
 */
void saveAllURLs(Node root, Action *action);
#endif