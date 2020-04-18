/*importing all necessary libraries needed */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

struct projNode { // this is a node that holds the project name and what not
    char * projName;
    struct projNode * next;
};

struct projNode * nameTbl[256]; // hashtable that holds all project names, easy O(1) access

int main (int argc, char ** argv) {
    memset(nameTbl, NULL, 256 * sizeof(struct projNode *)); // setting everything in the hashtable to NULL
    int lsocket; // declaring the file descriptor for our listening (server) socket
    int csocket; // declaring the file descriptor from the respective client socket
    lsocket = socket(AF_INET, SOCK_STREAM, 0); // creating the socket

    /* stuff that comprises the server addy */
    struct sockaddr_in serveraddy;
    serveraddy.sin_family = AF_INET;
    serveraddy.sin_port = argv[1]; // port number must be taken from command line
    serveraddy.sin_addr.s_addr = INADDR_ANY;
    bind(lsocket, (struct sockaddr*) &serveraddy, sizeof(serveraddy)); // binding socket to address
    listen(lsocket, 10); // has 10 clients on backlog
    csocket = accept(lsocket, NULL, NULL); // setting info to NULL rn
    return 0;
}


int creator (char * name) { // will see if the name of the project is there or not, whatever
    int sum = 0;
    int k;
    for (k = 0; k < strlen (name); k++) { // calculate the hash code for the name of the project
        sum = sum + (int)name[k];
    }
    int index = sum % 256;
    if (index < 0) {
        index *= -1; // if index is negative, make it positive
    }
    if(nameTbl[index] == NULL) { // project does not exist!!!
        struct projNode * newNode = (struct projNode *)malloc(sizeof(struct projNode));
        newNode -> projName = name;
        newNode -> next = NULL;
        nameTbl[index] = newNode;
        mkdir(name); // makes the directory
        return 1;
    }
    else { // this means that the index isnt empty, however 2 proj names may hash to same index in a linked list
        struct projNode * temp = nameTbl[index];
        while (temp != NULL) {
            if(strcmp(temp->projName, name) == 0) {
                return -1; // sadly the project name does exist, therefore this is a problem
            }
            else {
                temp = temp ->next // moving the nodes along the linked list
            }
        }
        // this means that we have reached the end of the linked list and we have to create the project since it does not exist
        temp = (struct projNode *)malloc(sizeof(struct projNode));
        temp -> projName = name;
        temp -> next = NULL;
        mkdir(name); // makes the directory
        return 1;
    }
}