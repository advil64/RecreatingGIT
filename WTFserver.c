/*importing all necessary libraries needed */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
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
int checkers (char * name);
int fyleBiter (char * path, char * buffer);
int mkdir(const char * pathname, mode_t mode);
int destroyer (DIR * myDirectory, int counter, int currSize, char * currDirec);

char ** files; // global variable that holds all the files within a directory!

int main (int argc, char ** argv) {
    int lsocket; // declaring the file descriptor for our listening (server) socket
    int csocket; // declaring the file descriptor from the respective client socket
    int caddysize = -1;
    int portnum = atoi(argv[1]);
    char crequest[6]; // get requests from clients!
    int manSuc = 1;
    int manFail = -1;
    char sCon[7] = {'f', 'u', 'c', 'k', ' ', 'u', '\0'};
    memset(crequest, '\0', 6);
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
    csocket = accept(lsocket, (struct sockaddr *) &clientaddy, (socklen_t *) &caddysize); // setting info to NULL rn
    send(csocket, sCon, sizeof(sCon), 0);
    recv(csocket, &crequest, sizeof(crequest), 0);
    printf("The client has requested the server to: %s", crequest);
    if (crequest[1] == 'r') { // this means you know you have to create the project (Will come in as Crea:)
      int projLen;
      recv(csocket, &projLen, sizeof(int), 0);
      char projName[projLen];
      int creRet; // what creator returns!
      memset(projName, '\0', projLen);
      recv(csocket, &projName, sizeof(projName), 0);
      creRet = creator(projName);
      if (creRet == 1) { // the file was created successfully
        send(csocket, &manSuc, sizeof(int), 0); // send 1 to the client
      }
      else { // file already existed yo
        send(csocket, &manFail, sizeof(int), 0);
      }
    }
    else if(crequest[0] == 'F') { // FILE:Path command
      int numBytes; // the number of bytes in a file
      int pathSize;
      recv(csocket, &pathSize, sizeof(int), 0);
      char pathName[pathSize]; // the path where the file to be opened resides
      char * fileBuf = NULL; // storing bytes in a file
      memset(pathName, '\0', pathSize);
      recv(csocket, &pathName, sizeof(pathName), 0);
      numBytes = fyleBiter(pathName, fileBuf);
      send(csocket,&numBytes, sizeof(int), 0);
      send(csocket, fileBuf, sizeof(fileBuf), 0);
    }
    else if(crequest[0] == 'D') { // destroying a directory!!
      files = (char **) malloc(100 * sizeof(char *));
      char dName[30]; // name of
      memset(dName, '\0', 30);
      recv(csocket, &dName, sizeof(dName), 0); // getting directory to be DESTROYED
      DIR * myDirec = opendir(dName); // making direct struct for the directory to be DESTROYED
      destroyer(myDirec, 0, 100, dName);
      char success[8] = {'s', 'u', 'c', 'c', 'e', 's', 's', '\0'};
      send(csocket, success, sizeof(success), 0);
    }
    else if (crequest[1] == 'h') { // checks to see if file exists
      int returnstat;
      int fileNameS;
      recv(csocket, &fileNameS, sizeof(int), 0);
      char project[fileNameS];
      memset(project, '\0', fileNameS);
      recv(csocket, &project, sizeof(project), 0);
      returnstat = checkers(project);
      if(returnstat == 1) { // success!
        send(csocket, &manSuc, sizeof(int), 0);
      }
      else {
        send(csocket, &manFail, sizeof(int), 0);
      }
    }
    return 0;
}


int creator (char * name) { // will see if the name of the project is there or not, whatever
    char manPath[PATH_MAX];
    char newMan[1] = {'1'};
    memset(manPath, '\0', PATH_MAX);
    strcpy(manPath, name);
    strcat(manPath, "/.Manifest");
    int mfd; // manpage file descriptor
    DIR * proj = opendir(name);
    if (proj == NULL) { // the directory does not exist in this world
      mkdir(name, S_IRWXU); // makes the directory
      mfd = open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
      write(mfd, newMan, 1);
      return 1;
    }
    else {
      return -1; // the project already existed!!
    } 
}

int checkers (char * name) { // almost like the opposite of create in my opinion, check to see if project exists!
  DIR * proj = opendir(name);
  if(proj == NULL) { // project doesnt exist!
    return -1;
  }
  else {
    return 1; // the project exists!
  }
}

int fyleBiter (char * path, char * buffer) { // given a file path, it will find how many bytes are within a file!!
  int fd; // file descriptor for the file we wanna read
  int buffsize = 100;
  buffer = (char*) malloc(101 * sizeof(char));
  char * temp; // temporary buffer
  memset(buffer, '\0', 101);
  fd = open(path, O_RDONLY); // opening the file given the path name with read only permission
  int status = 1;
  int readIN = 0;
   while(status > 0){ 
    //read buff size number of chars and store in myBuffer
    do{
      status = read(fd, buffer+readIN, 100);
      readIN += status;
    }while(readIN < buffsize && status > 0);
    //check if there are more chars left
    if(status > 0){
      //increase the array size by 100
      buffsize += 100;
      //store the old values in the temp pointer
      temp = buffer;
      //malloc the new array to myBuffer
      buffer = (char *)malloc(buffsize*sizeof(char));
      //check if the pointer is null
      if(buffer == NULL){
        //print an error
        printf("ERROR: Not enough space on heap\n");
        //exit
        return -1;
      }
      //set everything in the buffer to the null terminator
      memset(buffer, '\0', buffsize);
      //copy the old memory into the new buffer
      memcpy(buffer, temp, readIN);
      //free the old memory that was allocated temporarily
      free(temp);
    }
   }
   return readIN;
}

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
      counter = destroyer(opendir(filePBuff), counter, currSize, filePBuff);
      //we are back in the original file, get rid of the previous file path
      strcpy(filePBuff, currDirec);
      //put the forward-slash back in there
      strcat(filePBuff, "/");
      rmdir(filePBuff);
      //find the new max size of the array
      currSize = ((counter%100)+1)*100;
    } else if(currDir -> d_type == DT_REG){
      //allocate space for the file path
      files[counter] = (char *)malloc((PATH_MAX+1) * sizeof(char));
      //add the file path to the array
      strcpy(files[counter],filePBuff);
      //store the names of the files in our files array
      strcat(files[counter], currDir->d_name);
      remove(files[counter]);
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