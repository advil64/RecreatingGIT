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
  int clientProjVer;
  int servProjVer;
  //given the project name, we need to create a path to the manifest
  char manPath[PATH_MAX];
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Manifest");
  //we need to get the .manifest file in the directory
  int manFD = open(manPath, O_RDONLY);
  //if manFD is negative, return unsuccessfull
  if(manFD < 0){
    printf("Manifest does not exist within the project.\n");
    //the manifest does not exist, return an error
    return 1;
  }
  //pass in the client man to be populated
  int clientManSize = readFile(manFD, &clientMan);
  //clientman -> holds client's manifest... servman -> holds server's manifest now compare the two
  //.Manifest outline <project version> (once) <path> <file version> <hash> (for each file)
  //first check if the project version is the same
  sscanf(clientMan, "%d\n", &clientProjVer);
  sscanf(servMan, "%d\n", &servProjVer);
  //.Update file tells us what to upgrade
  int updateFD = open(".Update", O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  int conflictFD = open(".Update", O_APPEND | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  //if they are the same, then update is done
  if(clientProjVer == servProjVer){
    //finished update
    printf("Up To Date");
  } else{
    //the project versions are not the same and we need to traverse the two manifests
    //call on a method which reads both manifests line by line and populates the linked lists
    struct entry * servManHead = *populateManifest(servMan);
    struct entry * clienManHead = *populateManifest(clientMan);
    //given the two heads, we need to figure out which files are different and need to be updated
    //first get traversal nodes for both the linked lists
    struct entry * servCurr = servManHead;
    struct entry * clienCurr = clienManHead;
    //sort the two linked lists, there are definately lots of bugs in the sorting functions
    servManHead = *insertionSort(servManHead, charComparator);
    clienManHead = *insertionSort(clienManHead, charComparator);
    //this stores the new hash to check for conflicts
    unsigned char hash[SHA_DIGEST_LENGTH];
    //for each entry in the server manifest, loop through the client manifest
    while(servCurr != NULL || clienCurr != NULL){
      //find the corresponding entry in the client
      if(strcmp(servCurr ->filePath, clienCurr ->filePath) == 0){
        //to check for conflicts, first calculate the sha1 of the file in the client
        SHA1(clientMan, strlen(clientMan), hash);
        //then check if the client and server hashes are the same
        if(strcmp(clienCurr ->fileHash, servCurr ->fileHash) != 0){
          //check if the client hash is also up to date
          if(strcmp(hash, clienCurr -> fileHash) == 0){
            //we need to modify the file on the client side
            write(updateFD, "M ", 2);
            write(updateFD, servCurr -> filePath, strlen(servCurr -> filePath));
            write(updateFD, " ", 1);
            write(updateFD, servCurr->fileHash, strlen(servCurr->fileHash));
            write(updateFD, "\n", 1);
            //print the modification to terminal
            printf("M %s", servCurr -> filePath);
          } else{
            //we have a conflict
            write(conflictFD, "C ", 2);
            write(conflictFD, servCurr -> filePath, strlen(servCurr -> filePath));
            write(conflictFD, " ", 1);
            write(conflictFD, hash, strlen(hash));
            write(conflictFD, "\n", 1);
            //print a warning to the terminal
            printf("Conflicts were found and must be resolved before the project can be updated.\n");
          }
          //we need to download the modified 
        }
        //we increment both the currs
        servCurr = servCurr -> next;
        clienCurr = clienCurr -> next;
      } else if(strcmp(clienCurr ->filePath, servCurr ->filePath) > 0){
        //server has a file that client does not have, we need to add the file to client
        //this seems to be a new file write to the update fd
        write(updateFD, "A ", 2);
        write(updateFD, servCurr -> filePath, strlen(servCurr -> filePath));
        write(updateFD, " ", 1);
        write(updateFD, servCurr->fileHash, strlen(servCurr->fileHash));
        write(updateFD, "\n", 1);
        //print the appendage to terminal
        printf("A %s", servCurr -> filePath);
        //we increment only the client pointer
        servCurr = servCurr -> next;
      } else if(strcmp(servCurr ->filePath, clienCurr ->filePath) > 0){
        //client has a file that server does not have, we need to delete the file from client
        //this seems to be a new file write to the update fd
        write(updateFD, "D ", 2);
        write(updateFD, clienCurr -> filePath, strlen(clienCurr -> filePath));
        write(updateFD, " ", 1);
        write(updateFD, clienCurr->fileHash, strlen(clienCurr->fileHash));
        write(updateFD, "\n", 1);
        //print the deletion to terminal
        printf("D %s", clienCurr -> filePath);
        //we increment only the server pointer
        clienCurr = clienCurr -> next;
      }
    }
  }
  //free the buffers
  free(clientMan);
  free(servMan);
  //close the file descriptors
  close(manFD);
  closedir(myDirec);
  //return success
  return 0;
}

//returns 1 is first string comes before second string in the dictionary
int charComparator (void* thing1, void* thing2){
  //we are dealing with strings
  char * arr1 = (char *)thing1;
  char * arr2 = (char *)thing2;
  //we are working with strings or characters
  int counter = 0;
  //loop through both the strings
  while(arr1[counter] != '\0' && arr2[counter] != '\0'){
    //compare the characters
    if(arr1[counter] > arr2[counter]){
      return 1;
    } else if(arr1[counter] < arr2[counter]){
      return -1;
    }
    //increment counter
    counter++;
  }
  //both strings have the same starting chars but arr1 is shorter
  if(strlen(arr1) < strlen(arr2)){
    //arr1 is still greater
    return -1;
  } else if(strlen(arr2) < strlen(arr1)){
    //arr2 is now greater
    return 1;
  } else{
    //they are both the same thing
    return 0;
  }
}

//helps insert the given node into the right position
int insertionSortHelper(void* toSort, int(*comparator)(void*, void*)){
  //declarations used for the sorting, sortee will be moved
  struct entry * sortee = (struct entry *)toSort;
  struct entry * headNode = sortee;
  //extra nodes to do the sorting
  struct entry * temp1;
  struct entry * temp2;
  struct entry * temp3;
  //loop through the nodes before the sortee while the sortee is less than its previous
  while(sortee -> prev != NULL && comparator(sortee->filePath, sortee->prev->filePath) == -1){
    //stores the sortee's previous value (cannot be null)
    temp1 = sortee->prev;
    //this one may be null
    temp2 = sortee->prev->prev;
    //this one also may be null
    temp3 = sortee->next;
    //sortee's new next should be the old previous
    sortee -> next = temp1;
    //sortee and the node before it are getting switched
    sortee -> prev = temp2;
    //the previous node's previous is now sortee
    temp1 -> prev = sortee;
    //the old previous node's next is now sortee's next
    temp1 -> next = temp3;
    //if temp 2 is not null, make its next the sortee
    if(temp2 != NULL){temp2 -> next = sortee;}
    //if sortee's next is not null, make its previoous temp 1
    if(temp3 != NULL){
      temp3 -> prev = temp1;
    }
  }
  //check if sortee is the new head node
  if(sortee-> prev == NULL){
    //set the headnode to be sortee
    headNode = sortee;
  }
  //return the sorted headnode
  return &headNode;
}

/* How insertion sort basically works is that the first node is ALREADY SORTED in theory! */
struct entry ** insertionSort(void* toSort, int(*comparator)(void*, void*)) {
  //declarations used for the sorting, sortee will be moved
  struct entry * sortee = toSort;
  //if the sortee is null, there is nothing in the linked list
  if(sortee == NULL){
    //the list is null
    return 0;
  } else if(sortee->prev == NULL && sortee->next == NULL) {
    //its the only node in the linked list 
    return 0;
  }
  //because the headnode is technically already sorted, progress to the next node
  sortee = sortee->next;
  //compare the current value with its next value unless sortee is at the end
  while(sortee != NULL) {
    //insert the given sortee into the correct position of the linked list
    insertionSortHelper(sortee, comparator);
    //change sortee's to its next
    sortee = sortee -> next;
  }
  //we're finished
  return 0;
}

/*helper method to populate the manifest linked lists*/
struct entry ** populateManifest(char * buffer){
  //loop through the given buffer and extract the info
  struct entry * prev = NULL;
  struct entry * next = NULL;
  struct entry * curr = NULL;
  //this stores every line
  char * line;
  line = strtok(buffer, "\n");
  //loop through and do repeat
  while(line != NULL){
    //allocate a new node
    struct entry * curr = (struct entry *)malloc(sizeof(struct entry));
    //tokenize a new line
    line = strtok(buffer, "\n");
    //now store the items in the line in their correct positions
    sscanf(line, "%s %d %s", curr -> filePath, &(curr -> fileVer), curr -> fileHash);
    //make curr's next equal to the next from above
    curr -> next = next;
    //make next equal curr
    next = curr;
  }
  //one we are done populating the list we need to go through the list again and get previouses
  curr = next;
  //loop through the list
  while(curr != NULL){
    //assign the previouses
    curr -> prev = prev;
    prev = curr;
    curr = curr -> next;
  }
  //return the final next which becomes the head
  return &next;
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