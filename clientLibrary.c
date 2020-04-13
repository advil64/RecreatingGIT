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
    //close the directory
    closedir(myDirec);
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
Update's purpose is to fetch the server's .Manifest for the specified project, compare every entry in it to the
client's .Manifest and see if there are any changes on the server side for the client. If there are, it adds a line to
a .Update file to reflect the change and outputs some information to STDOUT to let the user know what needs to
change/will be changed. This is done for every difference discovered. If there is an update but the user changed
the file that needs to be updated, update should write instead to a .Conflict file and delete any .Update file (if
there is one). If the server has no changes for the client, update can stop and does not have to do a line-by-line
analysis of the .Manifest files, and should blank the .Update file and delete any .Conflict file (if there is one),
since there are no server updates.
*/

int update(char * projName){
  //call the read configure file to find the IP and port
  if(readConf()){
    //if it failed print the error
    printf("Please configure your client with a port and IP first.\n");
    //return unsuccessfull
    return 1;
  }
  //check if the given project exists on the client
  DIR *myDirec = opendir(projName);
  if(!myDirec){
    //the directory/project exists and we need to print an error
    printf("The project you want does not exist.\n");
    //return unsuccessful
    return 1;
  }
  //TODO: write code that actually connects to the server
  //this buffer ideally holds the server's .Manifest file, right now it is a placeholder
  int servManSize;
  char * servMan = NULL;
  char * clientMan;
  //given the project name, we need to create a path to the manifest
  char manPath[PATH_MAX];
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Manifest");
  //we need to get the .manifest file in the directory
  int manFD = open(manPath, O_RDONLY);
  //if manFD is negative, return unsuccessfull
  if(manFD < 0){
    //the manifest does not exist, return an error
    return 1;
  }
  //pass in the client man to be populated
  int clientManSize = readFile(manFD, &clientMan);
  //TODO: finish the rest of this function in a later time

  //free the buffers
  free(clientMan);
  free(servMan);
  //close the file descriptors
  close(manFD);
  closedir(myDirec);
  //return success
  return 0;
}

/*
The upgrade command will fail if the project name doesn’t exist on the server, if the server can not be
contacted, if there is no .Update on the client side or if .Conflict exists. The client will apply the changes listed
in the .Update to the client's local copy of the project. It will delete the entry from the client's .Manifest for all
files tagged with a “D”, fetch from the server and then write or overwrite all files on the client side that are
tagged with a “M” or “A”, respectively. When it is done processing all updates listed in it, the client should
delete the .Update file. Note that the client does not make any changes to files in the the project directory that are
not listed in the .Update. If the .Update is empty, the client need only inform the user that the project is up to
date and delete the empty .Update file. If no .Update file exists, the client should tell the user to first do an
update. If .Conflict exists, the client should tell the user to first resolve all conflicts and update.
*/

int upgrade(char * projName){
  //call the read configure file to find the IP and port
  if(readConf()){
    //if it failed print the error
    printf("Please configure your client with a port and IP first.\n");
    //return unsuccessfull
    return 1;
  }
  //check to see if a .conflict file exists
  int conflictFD = open(".Conflict", O_RDONLY);
  //if confd is positive, return unsuccessfull
  if(conflictFD > 0){
    printf("Please resolve conflicts before continuing.\n");
    //close the file descriptor
    close(conflictFD);
    //update does not exist, return an error
    return 1;
  }
  //check to see if a .update file exists on the client side
  int updateFD = open(".Update", O_RDONLY);
  //if confd is negative, return unsuccessfull
  if(updateFD < 0){
    printf("There are no updates at this time, please try again later.\n");
    //update does not exist, return an error
    return 1;
  }
  //buffer to store the update file
  char * updates;
  //read in the update file
  readFile(updateFD, &updates);

  //TODO: add code which looks for the D M and A commands

  //close the files
  close(updateFD);
  //free the buffer
  free(updates);
}

/*
The commit command will fail if the project name doesn’t exist on the server, if the server can not be contacted,
if the client can not fetch the server's .Manifest file for the project, if the client has a .Update file that isn't empty
(no .Update is fine) or has a .Conflict file. After fetching the server's .Manifest, the client should should first
check to make sure that the .Manifest versions match. If they do not match, the client can stop immediatley and
ask the user to update its local project first. If the versions match, the client should run through its own .Manifest
and compute a live hash for each file listed in it. Every file whose live hash is different than the stord hash saved
in the client's local .Manifest should have an entry written out to a .Commit with its file version number
incremented.
*/

int commit(char * projName){
  //call the read configure file to find the IP and port
  if(readConf()){
    //if it failed print the error
    printf("Please configure your client with a port and IP first.\n");
    //return unsuccessfull
    return 1;
  }
  //TODO: finish the rest of this function
}

/*
The push command will fail if the project name doesn’t exist on the server, if the client can not communicate
with the server or if the client has no .Commit file. The client should send its .Commit and all files listed in it to
the server. The server should first lock the repository so no other command can be run on it. While the repository
is locked, the server should check to see if it has a stored .Commit for the client and that it is the same as the
.Commit the client just sent. If this is the case, the server should expire all other .Commits pending for any other
clients, duplicate the project directory, write all the files the client sent to the newly-copied directory (or remove
files, as indicated in the .Commit), update the new project directory's .Manifest by replacing corresponding
entries for all files uploaded (and removing entries for all files removed) with the information in the .Commit the
client sent, and increasing the project's version. The server should then unlock the repository and send a success
message to the client. If there is a failure at any point in this process, the server should delete any new files or
directories created, unlock the repository and send a failure message to the client. The client should erase its
.Commit on either response from the server.
*/

int push(char * projName){
  //first check to see if .configure has been created
  int confFD = open(".configure", O_RDONLY);
  //if confd is negative, return unsuccessfull
  if(confFD < 0){
    //configure does not exist, return an error
    return 1;
  }
  //TODO: Finish the rest of this function
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
int readFile(int fd, char ** buff){
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
  //return the size of the file
  return buffSize;
}