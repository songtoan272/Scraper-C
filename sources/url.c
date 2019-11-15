/*
**  Filename : url.c
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
**              - The root of the tree will have the url "." ,whose depth 
**              equals -1 and has no sibling node.
**              
**              The tree structure helps keeping track of all parsed URL without
**              consuming much of memory as each node only saves a part of the
**              entire URL.
**              Divide URLs into small parts and arrange nodes orderly helps 
**              navigating the right node quick.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "url.h"
#include "configuration.h"


/*****************CONTRUCTION************************/

/**
 * Initialize a Node
 * @param suburl : the string of sub-link of the url 
 * @param depth : the depth of this link from the initial
 * @param next : its next sibling
 * @param child : its first child
 * @return : the node itself
 */
Node initNode(char *suburl, int depth, Node next, Node child){
    if (suburl == NULL || strlen(suburl) == 0){
        fprintf(stderr, "Wrong URL\n");
        exit(1);
    }

    Node res = (Node)malloc(sizeof(struct __node));
    if (res == NULL){
        fprintf(stderr, "Allocation for new Node failed.\n");
        exit(1);
    }

    res->depth = depth;
    res->firstChild = child;
    res->nextSibling = next;
    res->url = strdup(suburl);

    return res;
}


/**
 * delete the "https://" or "http://" in the URL 
 * @param URL : the URL to be modified
 * @return : the pointer to the modified URL
 */
char *delHTTPS(char *url){
    char *partURL;
    char delim[3] = "//";

    //find the first occurence of "//" in url
    //which delimits the https part and the real
    //url part
    partURL = strstr(url, delim);
    
    if (partURL == NULL){
        //no "//" exists in URL 
        return url;
    }else{
        return partURL+2;
    }
}


/**
 * Make a tree from the very initial url 
 * @param url : the initial url of the tree
 * @return : the root of the tree
 */
Node makeTree(char *url){
    Node root, curNode;
    char *subURL, *__url;
    const char delim[2] = "/";

    //del the HTTP part from URL
    url = strdup(url);
    __url = delHTTPS(url);

    //create the root node 
    root = initNode(".", -1, NULL, NULL);
    curNode = root;

    subURL = strtok(__url, delim);
    while (subURL != NULL){
        curNode->firstChild = initNode(subURL, -1, NULL, NULL);
        subURL = strtok(NULL, delim);
        curNode = curNode->firstChild;
    }

    free(url); free(subURL);
    return root;
}


/*****************INSERTION************************/


/**
 * Compare two nodes by their url 
 * @param node1; node2 : 2 nodes to be compared
 * @return : 0 if their url are equals 
 *          -1 if node1->url < node2->url
 *           1 if node1->url > node2->url
 */
int compareNode(Node node1, Node node2){
    return strcmp(node1->url, node2->url);
}


/**
 * Divide an URL into 2 parts by the first occurence of '/'
 * @param URL : the URL to be divided
 * @param firstSubURL: address of the string to stock 
 * the first part of the URL
 * @param restURL: address of the string to stock the 
 * last part of the URL
 * @return : nothing as 2 strings are saved in the pointers
 * passed in by arguments
 */
void divideURL(char *URL, char **firstSubURL, char **restURL){
    char *idxDiviseur;

    idxDiviseur = strchr(URL, '/');
    if (idxDiviseur == NULL){
        //URL doesn't have any further sub URLs
        *firstSubURL = strdup(URL);
        *restURL = NULL;
    }else{
        *restURL = idxDiviseur + 1;
        //recopy the first part into firstSubURL
        *firstSubURL = (char*)malloc((idxDiviseur - URL) * sizeof(char));
        strncpy(*firstSubURL, URL, (idxDiviseur - URL));
    }
}


/**
 * Insert a node into the tree
 * @param upperNode : the upper Node (parent node) of those 
 * to be inserted successively (have to be != NULL)
 * @param subURLInsert : the sub url of the node to be inserted
 * @param depth : the depth of the inserted node from the initial url
 * @return : nothing as the node to be inserted is updated through upperNode
 */
void insertNode(Node upperNode, char *subURLInsert, int depth){
    if (upperNode == NULL){
        fprintf(stderr, "Parent node does not exist.\n");
        exit(1);
    }

    if (subURLInsert == NULL || strlen(subURLInsert) == 0){
        return;
    }

    //declare necessary variables
    char *firstSubURL, *restURL, *idxDiviseur; 
    Node nodeToInsert, childNode, prevNode;
    int order, inserted;


    divideURL(subURLInsert, &firstSubURL, &restURL);
    nodeToInsert = initNode(firstSubURL, -1, NULL, NULL);
    free(firstSubURL); //node initialised -> free to save memory



    //insert the node to a appropriate position 
    //regarding the alphabet order between sibling nodes
    childNode = upperNode->firstChild;
    order = compareNode(nodeToInsert, childNode);
    if (order == -1){
        //nodeToInsert < childNode
        nodeToInsert->nextSibling = childNode;
        upperNode->firstChild = nodeToInsert;
    }else if (order == 0){
        //nodeToInsert is the childNode -> alr exists
        //-> we dont insert it and process to
        //the rest part of URL
        delNode(&nodeToInsert);
        nodeToInsert = childNode;
    }else{
        //nodeToInsert > childNode
        //-> insert it in the proper position following alphabet order
        prevNode = childNode;
        childNode = childNode->nextSibling;
        inserted = 0;
        while (childNode != NULL){
            order = compareNode(nodeToInsert, childNode);
            if (order == -1){
                nodeToInsert->nextSibling = childNode;
                prevNode->nextSibling = nodeToInsert;
                inserted = 1;
                break;
            }else if (order == 1){
                prevNode = childNode;
                childNode = childNode->nextSibling;
            }else{
                //this node alr exists in the tree -> pass
                delNode(&nodeToInsert);
                nodeToInsert = childNode;
                inserted = 1;
                break;
            }
        }
        //browsed through all children nodes of upperNode 
        //-> insert to the end of the linked-list sibling nodes
        if (!inserted){
            prevNode->nextSibling = nodeToInsert;
        }        
    }


    if (restURL == NULL || strlen(restURL) == 0){
        //this is the last node to be inserted in 
        //this URL so we update its depth and stop here
        nodeToInsert->depth = depth;
        return;
    }

    //call recursively this function for the rest part
    //of the URL with the node inserted as upperNode
    insertNode(nodeToInsert, restURL, depth);

}


/**
 * Insert a URL into the tree
 * @param root : the root of the tree
 * @param URL : the url to be inserted in the tree
 * @param depth : the depth of the inserted node from the initial url
 * @return : nothing as the root always stay the same 
 */
void insertURL(Node root, char *URL, int depth){
    insertNode(root, URL, depth);
}


/*****************DELETION************************/


/**
 * Delete (free) a node
 * @return : nothing
 * the value of the case to which pNode point turned to NULL
 */
void delNode(Node *pNode){
    free((*pNode)->url);
    free(*pNode);
    *pNode = NULL;
}

/**
 * Delete (free) the whole tree
 * @return : the value of the case to which pNode point turned to NULL
 */
void delTree(Node *pRoot){
    if (*pRoot == NULL) return;
    Node curNode = *pRoot;
    Node nextSiblingNode;
    Node childNode; 

    childNode = curNode->firstChild;
    nextSiblingNode = curNode->nextSibling;
    delNode(&curNode);
    delTree(&childNode);
    delTree(&nextSiblingNode);
}


/*****************SEARCH************************/


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
Node findNode(Node upperNode, char *subURL){
    if (subURL == NULL || strlen(subURL) == 0){
        return upperNode;
    }

    int order;
    char *firstPartURL, *restURL;
    Node childNode = upperNode->firstChild;

    divideURL(subURL, &firstPartURL, &restURL);
    while (childNode != NULL){
        order = strcmp(firstPartURL, childNode->url);
        if (order == 0){
            free(firstPartURL);
            return findNode(childNode, restURL);
        }else if(order == 1){
            childNode = childNode->nextSibling;
        }else{
            break;
        }
    }
    free(firstPartURL);
    return NULL;
}


/**
 * Verify if an URL has already been parsed
 * @param root : the root of the tree
 * @param URL : the URL to be verified
 * @return : 1 if the URL was parsed
 *           0 if not
 */
int URLAlrParsed(Node root, char *URL){
    Node nodeOfURL = findNode(root, URL);
    if (nodeOfURL == NULL){
        //this URL has not been inserted into the tree
        return 1;
    }else{
        if (nodeOfURL->depth == -1){
            //this URL was inserted but has not been parsed
            return 0;
        }else{
            //inserted and parsed
            return 1;
        }
    }
}


/*****************PRINT TREE************************/


/**
 * Print out a node in format "(url, depth)"
 * @param node : the node to be printed out
 * @return :
 */
void printNode(Node node){

}


/**
 * Print out the whole tree
 * @param root : the root of the tree
 * @return :
 */
void printTree(Node root){

}


/*****************SAVE ALL URLS************************/


/**
 * Construct and write the parsed URL to the file
 * @param upperNode : the current parent node from where
 * descend to retrieve other URLs
 * @param prefixURL : the prefix part of the URL 
 * @param f : the file to write the URL on
 * @return : 0 if their url are equals 
 *          -1 if node1->url < node2->url
 *           1 if node1->url > node2->url
 */
void writeURL(Node upperNode, char *prefixURL, FILE *f){
    if (upperNode == NULL || prefixURL == NULL){
        return;
    }

    Node childNode;
    char *URLConstructed = NULL;
    int size, sizePrefix;

    sizePrefix = strlen(prefixURL);

    for (childNode = upperNode->firstChild; childNode != NULL; 
    childNode = childNode->nextSibling){
        size = sizePrefix + strlen(childNode->url) + 2;
        URLConstructed = (char*)malloc(size * sizeof(char));
        strcpy(URLConstructed, prefixURL);
        strcat(URLConstructed, "/");
        strcat(URLConstructed, childNode->url);

        if (childNode->depth != -1){
            //the URL constructed was parsed -> save it
            fprintf(f, "%s\n", URLConstructed);
        }
        writeURL(childNode, URLConstructed, f);
        free(URLConstructed);
    }
}


/**
 * Browse through the tree to find and save all 
 * the parsed URLs into a file 
 * @param root : root of the tree
 * @param action : the action executed
 * @return : nothing (a file is created and saved)
 */
void saveAllURLs(Node root, Action *action){
    FILE *f;
    int size;
    char *fullPath;

    size = strlen("data/") + strlen(action->name) + strlen("/hyperlinks.txt")+ 1;
    fullPath = (char*)malloc(size * sizeof(char));
    strcpy(fullPath, "data/");
    strcat(fullPath, action->name);
    strcat(fullPath, "/hyperlinks.txt");

    f = fopen(fullPath, "w");
    if (f == NULL){
        fprintf(stderr, "Cannot open file to save URLs\n");
        exit(1);
    }
    writeURL(root, "", f);

    free(fullPath);
    fclose(f);
}