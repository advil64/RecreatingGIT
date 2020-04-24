/*importing all necessary libraries needed */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
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
int mkdir(const char * pathname, mode_t mode);

struct projNode * nameTbl[256]; // hashtable that holds all project names, easy O(1) access

int main (int argc, char ** argv) {
    memset(nameTbl, 0x0, 256 * sizeof(struct projNode *)); // setting everything in the hashtable to NULL
    int lsocket; // declaring the file descriptor for our listening (server) socket
    int csocket; // declaring the file descriptor from the respective client socket
    int caddysize = -1;
    int portnum = atoi(argv[1]);
    char crequest[1000]; // get requests from clients!
    char manSuc[2] = {'1', '\0'};
    char manFail[3] = {'-','1','\0'};
    char sCon[7] = {'f', 'u', 'c', 'k', ' ', 'u', '\0'};
    memset(crequest, '\0', 1000);
    lsocket = socket(AF_INET, SOCK_STREAM, 0); // creating the socket
    /* stuff that comprises the server addy */
    struct sockaddr_in serveraddy;
    struct sockaddr_in clientaddy;
    caddysize = sizeof(clientaddy);
    bzero((char*) &serveraddy, sizeof(serveraddy));
    serveraddy.sin_family = AF_INET;
    serveraddy.sin_port = htons(portnum); // port number must be taken from command line
    serveraddy.sin_addr.s_addr = INADDR_ANY;
    bind(lsocket, (struct sockaddr*) &serveraddy, sizeof(serveraddy)); // binding socket to address
    listen(lsocket, 0); // has 0 clients on backlog
    csocket = accept(lsocket, (struct sockaddr *) &clientaddy, &caddysize); // setting info to NULL rn
    send(csocket, sCon, sizeof(sCon), 0);
    recv(csocket, &crequest, sizeof(crequest), 0);
    printf("The client has requested the server to: %s", crequest);
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
        mkdir(name, S_IRWXU); // makes the directory
        open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
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
        mkdir(name, S_IRWXU); // makes the directory
        open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
        return 1;
    }
}
/*
int destroyer (DIR *myDirectory, int counter, int currSize, char * currDirec){ // takes in a directory that is meant to be deleted
  //stores the filepath of our subdirectories
  char filePBuff[PATH_MAX + 1];
  //in the case of recursion, update the filepath so that we do not get lost
  strcpy(filePBuff, currDirec);
  //add a forward-slash at the end to get ready to add more to the path
  strcat(filePBuff, "/");
  //the dirent struct which holds whatever readdir returns
  struct dirent *currDir;
  //loop through the contents of the directory and store in files array
  while((currDir = readdir(myDirectory)) != NULL){
    //skip the . and .. and dsstore file
    if(strcmp(currDir->d_name, ".") == 0 || strcmp(currDir->d_name, "..") == 0 || strcmp(currDir->d_name,".DS_Store") == 0){
      //skip the iteration
      continue;
    }
    //first check if the currdir is a regular file or a directory
    if(currDir->d_type == DT_DIR){
      //add the directory in question to the path
      strcat(filePBuff, currDir->d_name);
      //traverse the new directory
      counter = direcTraverse(opendir(filePBuff), counter, currSize, filePBuff);
      //we are back in the original file, get rid of the previous file path
      strcpy(filePBuff, currDirec);
      //put the forward-slash back in there
      strcat(filePBuff, "/");
      //find the new max size of the array
      currSize = ((counter%100)+1)*100;
    } else if(currDir -> d_type == DT_REG){
      //allocate space for the file path
      files[counter] = (char *)malloc((PATH_MAX+1) * sizeof(char));
      //add the file path to the array
      strcpy(files[counter],filePBuff);
      //store the names of the files in our files array
      strcat(files[counter], currDir->d_name);
      //just to test the code
      //printf("%s\n", files[counter]);
      //check if files array needs more space
      if(++counter >= currSize){
        //realloc 100 more spaces in our files array
        files = realloc(files, (currSize+100) * sizeof(char *));
        //check is the given file exists
        if(!files){
          //file does not exist it is a fatal error
          printf("FATAL ERROR: Not enough memory\n");
          //exit the code
          exit(1);
        }
        //change the curr size accordingly
        currSize += 100;
      }
    }
  }
  //return the current count
  return counter;
}
*/