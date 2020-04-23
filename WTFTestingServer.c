/*importing all necessary libraries needed */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <limits.h>

struct projNode { // this is a node that holds the project name and what not
    char * projName;
    struct projNode * next;
};

int creator (char * name);

struct projNode * nameTbl[256]; // hashtable that holds all project names, easy O(1) access

int main (int argc, char ** argv) {
    memset(nameTbl, NULL, 256 * sizeof(struct projNode *)); // setting everything in the hashtable to NULL
    int lsocket; // declaring the file descriptor for our listening (server) socket
    int csocket; // declaring the file descriptor from the respective client socket
    char crequest[1000]; // get requests from clients!
    char manSuc[2] = {'1', '\0'};
    char manFail[2] = {'-1','\0'};
    memset(crequest, '\0', 1000);
    lsocket = socket(AF_INET, SOCK_STREAM, 0); // creating the socket
    /* stuff that comprises the server addy */
    struct sockaddr_in serveraddy;
    serveraddy.sin_family = AF_INET;
    serveraddy.sin_port = argv[1]; // port number must be taken from command line
    serveraddy.sin_addr.s_addr = INADDR_ANY;
    bind(lsocket, (struct sockaddr*) &serveraddy, sizeof(serveraddy)); // binding socket to address
    listen(lsocket, 10); // has 10 clients on backlog
    csocket = accept(lsocket, NULL, NULL); // setting info to NULL rn
    recv(csocket, &crequest, sizeof(crequest), 0);
    if (crequest[1] == 'r') { // this means you know you have to create something
      char projName[100];
      int count = 7;
      int creRet; // what creator returns!
      memset(projName, '\0', 100);
      while(crequest[count] != '\0') { // this will put the future project name into its own string
        projName[count - 7] = crequest[count];
        count++;
      }
      creRet = creator(projName);
      if (creRet == 1) { // the file was created successfully
        send(csocket, manSuc, sizeof(manSuc), 0); // send 1 to the client
      }
      else {
        send(csocket, manFail, sizeof(manFail), 0);
      }
    }

    return 0;
}


int creator (char * name) { // will see if the name of the project is there or not, whatever
    int sum = 0;
    int k;
    int mfd; // manifest file descriptor
    char manPath[PATH_MAX];
    memset(manPath, '\0', PATH_MAX);
    strcpy(manPath, name);
    strcat(manPath, "/.Manifest");
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
        mfd = open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
        return 1;
    }
    else { // this means that the index isnt empty, however 2 proj names may hash to same index in a linked list
        struct projNode * temp = nameTbl[index];
        while (temp != NULL) {
            if(strcmp(temp->projName, name) == 0) {
                return -1; // sadly the project name does exist, therefore this is a problem
            }
            else {
                temp = temp ->next; // moving the nodes along the linked list
            }
        }
        // this means that we have reached the end of the linked list and we have to create the project since it does not exist
        temp = (struct projNode *)malloc(sizeof(struct projNode));
        temp -> projName = name;
        temp -> next = NULL;
        mkdir(name); // makes the directory
        mfd = open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
        return 1;
    }
}