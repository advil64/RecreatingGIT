#include "clientHeader.h"

/*
The configure command will save the IP address (or hostname) and port of the server for use by later
commands. This command will not attempt a connection to the server, but insteads saves the IP and port number
so that they are not needed as parameters for all other commands. The IP (or hostname) and port should be
written out to a ./.configure file. All commands that need to communicate with the server should first try to get
the address information and port from the ./.configure file and must fail if configure wasn’t run before they were
called. All other commands must also fail if a connection to the server cannot be established.
Note: if you can write out to an environment variable that persists between Processes, feel free to do so, but all
recent feedback has been that security upgrades to the iLabs seem to have obviated this option.
*/

int configure(char * myIp, char * myPort){
  //open the .configure file to write ip and port
  int confFD = open(".configure", O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  //check if confFD is positive
  if(confFD < 0){
    //unsuccessful
    return 0;
  }
  //write IP/Host to the .configure file
  write(confFD, myIp, strlen(myIp)*sizeof(char));
  write(confFD, "\n", sizeof(char));
  //write the port number to the configure file
  write(confFD, myPort, strlen(myPort)*sizeof(char));
  //close the file descriptor
  close(confFD);
  //return 0 is it was successfull
  return 0;
}

/*
The checkout command will fail if the project name doesn’t exist on the server, the client can't communicate
with the server, if the project name already exists on the client side or if configure was not run on the client side.
If it does run it will request the entire project from the server, which will send over the current version of the
project .Manifest as well as all the files that are listed in it. The client will be responsible for receiving the
project, creating any subdirectories under the project and putting all files in to place as well as saving the
.Manifest.
Note: The project is stored in the working directory once it recieves all the files from the server
*/

int checkout(char * projName){
  //call the read configure file to find the IP and port
  if(readConf()){
    //if it failed print the error
    printf("Please configure your client with a port and IP first.\n");
    //return unsuccessfull
    return 1;
  }
  //check if the given project already exists
  DIR *myDirec = opendir(projName);
  if(myDirec){
    //the directory/project exists and we need to print an error
    printf("The project you want to check out already exists.\n");
    //free the pointer
    free(myDirec);
    //return unsuccessful
    return 1;
  }
  //we're good to go on the client side, we need to communicate with the server now
  printf("Attempting to connect to server...");
  //TODO: write code that actually connects to the server
  return 0;
}

/*
The update command will fail if the project name doesn’t exist on the server and if the client can not contact the
server. The update command is rather complex since it is where lots of things are compared in order to maintain
proper versioning. If update doesn't work correctly, almost nothing else will.
*/

int update(char * projName){
  //call the read configure file to find the IP and port
  if(readConf()){
    //if it failed print the error
    printf("Please configure your client with a port and IP first.\n");
    //return unsuccessfull
    return 1;
  }
  
}

/* helper method to read the configure file */
int readConf(){
  //first check to see if .configure has been created
  int confFD = open(".configure", O_RDONLY);
  //if confd is negative, return unsuccessfull
  if(confFD < 0){
    //configure does not exist, return an error
    return 1;
  }
  //the buffer to store the file in
  char * buffer;
  //call the readfile method
  readFile(confFD, &buffer);
  //in the buffer loop through and gather the needed information
  sscanf(buffer, "%s\n%d", IP, &port);
  //print the IP address and port number to the console
  printf("IP/Hostname: %s\n", IP);
  printf("Port #: %d\n", port);
  //free the buffer created and close the fd
  free(buffer);
  close(confFD);
  //return success
  return 0;
}

/*helper method to read files*/
void readFile(int fd, char ** buff){
  //if confFD is positive, configure has been created, extract the IP and Port
  int buffSize = lseek(fd, (size_t)0, SEEK_END);
  //store the contents of the config file
  *buff = malloc((buffSize+1)*sizeof(char));
  //set the offset back to the start
  lseek(fd, (size_t)0, SEEK_SET);
  //memset the buffer to null terminators
  memset(*buff, '\0', buffSize+1);
  //read the buffsize amount of bytes into the buffer
  read(fd, *buff, buffSize);
}