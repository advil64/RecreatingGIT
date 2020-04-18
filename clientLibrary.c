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
  //check if the given project already exists
  DIR *myDirec = opendir(projName);
  if(myDirec){
    printf("The project you want to check out already exists.\n");
    closedir(myDirec);
    return 1;
  }
  //we're good to go on the client side, we need to communicate with the server now
  printf("Attempting to connect to server...");
  if(connectToServer()){
    printf("We're having difficulties connecting to the server at the given IP address and port.\n");
    return 1;
  }
  //now that we connected, we need to ask server for the project
  //TODO: write code that actually connects to the server
  return 0;
}

/* Function to connect to the server on the client side (copied from Francisco's lecture)*/
int connectToServer(){
  //call the read configure file to find the IP and port
  if(readConf()){
    printf("Please configure your client with a port and IP first.\n");
    return 1;
  }
  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sfd < 0){
    printf("Unable to setup the socket, try again.\n");
    return 1;
  }
  struct hostent* result = gethostbyname(IP);
  if(result == NULL){
    printf("No such host by that given name try a different IP.\n");
    return 1;
  }
  struct sockaddr_in serverAddress;
  bzero((char *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  bcopy((char *)result->h_addr_list[0],(char *)&serverAddress.sin_addr.s_addr, result->h_length);
  if(connect(sfd, (struct sockaddr *)&serverAddress, sizeOf(serverAddress))){
    return 1;
  } else{
    printf("Client has successfully connected to the server.\n");
    return 0;
  }
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
  //check if the given project exists on the client (closed)
  DIR *myDirec = opendir(projName);
  if(!myDirec){
    printf("The project you want does not exist.\n");
    return 1;
  }
  //this method connects to the server at the given IP address and populates the global hostent
  if(connectToServer()){
    printf("We're having difficulties connecting to the server at the given IP address and port.\n");
    return 1;
  }
  int servManSize;
  char * servMan;
  char * clientMan;
  int clientProjVer;
  int servProjVer;
  //given the project name, we need to create a path to the manifest
  char manPath[PATH_MAX];
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Manifest");
  //we need to get the .manifest file in the directory (closed)
  int manFD = open(manPath, O_RDONLY);
  //if manFD is negative, return unsuccessfull
  if(manFD < 0){
    printf("Manifest does not exist within the project.\n");
    close(sfd);
    return 1;
  }
  //pass in the client man to be populated 
  readFile(manFD, &clientMan);
  //follow protocol to retrieve the .Manifest from the server, first ask it for the manifest then read it
  write(sfd, "File: ", 6);
  write(sfd, manPath, strlen(manPath));
  read(sfd, &servManSize, sizeof(int));
  if(servManSize < 0){
    printf("The project you are looking for does not exist.\n");
    return 1;
  }
  servMan = (char *)malloc((servManSize+1)*sizeof(char));
  read(sfd, servMan, servManSize);
  servMan[servManSize] = '\0';
  /* clientman -> holds client's manifest... servman -> holds server's manifest now compare the two
  .Manifest outline <project version> (once) <path> <file version> <hash> (for each file)
  first check if the project version is the same then create the update and conflict accordingly*/
  sscanf(clientMan, "%d\n", &clientProjVer);
  sscanf(servMan, "%d\n", &servProjVer);
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Update");
  int updateFD = open(manPath, O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Conflict");
  int conflictFD = open(manPath, O_APPEND | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  int hasConflict = FALSE;
  //if they are the same, then update is done
  if(clientProjVer == servProjVer){
    printf("Up To Date");
  } else{
    //the project versions are not the same and we need to traverse the two manifests
    populateManifest(servMan, &servManHead);
    populateManifest(clientMan, &clienManHead);
    insertionSort(&servManHead, charComparator);
    insertionSort(&clienManHead, charComparator);
    //given the two heads, we need to figure out which files are different and need to be updated
    struct entry * servCurr = servManHead;
    struct entry * clienCurr = clienManHead;
    //this stores the new hash to check for conflicts
    unsigned char hash[SHA_DIGEST_LENGTH+1];
    //for each entry in the server manifest, loop through the client manifest
    while(servCurr != NULL || clienCurr != NULL){
      //find the corresponding entry in the client
      if(strcmp(servCurr ->filePath, clienCurr ->filePath) == 0){
        //memset before we do anything with the hash
        memset(hash, '\0', SHA_DIGEST_LENGTH+1);
        //to check for conflicts, first calculate the sha1 of the file in the client
        SHA1((unsigned char *)clientMan, strlen(clientMan), hash);
        //then check if the client and server hashes are the same
        if(strcmp(clienCurr ->fileHash, servCurr ->fileHash) != 0){
          //check if the client hash is also up to date
          if(strcmp((char *)hash, clienCurr -> fileHash) == 0){
            //we need to modify the file on the client side
            write(updateFD, "M ", 2);
            write(updateFD, servCurr -> filePath, strlen(servCurr -> filePath));
            write(updateFD, " ", 1);
            write(updateFD, servCurr->fileHash, strlen(servCurr->fileHash));
            write(updateFD, " ", 1);
            write(updateFD, &(servCurr->fileVer), sizeof(int));
            write(updateFD, "\n", 1);
            printf("M %s", servCurr -> filePath);
          } else{
            //we have a conflict
            hasConflict = TRUE;
            write(conflictFD, "C ", 2);
            write(conflictFD, servCurr -> filePath, strlen(servCurr -> filePath));
            write(conflictFD, " ", 1);
            write(conflictFD, hash, strlen((char *)hash));
            write(conflictFD, "\n", 1);
            printf("Conflicts were found and must be resolved before the project can be updated.\n");
          }
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
        write(updateFD, " ", 1);
        write(updateFD, &(servCurr->fileVer), sizeof(int));
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
        write(updateFD, " ", 1);
        write(updateFD, &(servCurr->fileVer), sizeof(int));
        write(updateFD, "\n", 1);
        //print the deletion to terminal
        printf("D %s", clienCurr -> filePath);
        //we increment only the server pointer
        clienCurr = clienCurr -> next;
      }
    }
  }
  //check for conflicts
  if(!hasConflict){
    remove(manPath);
  }
  //free the buffers
  free(clientMan);
  free(servMan);
  //close the file descriptors
  close(manFD);
  close(updateFD);
  close(conflictFD);
  close(sfd);
  closedir(myDirec);
  freeLL(servManHead);
  freeLL(clienManHead);
  //return success
  return 0;
}

/*This method frees linked lists*/
void freeLL(struct entry * head){
  struct entry * temp = NULL;
  struct entry * curr = head;
  while(curr != NULL){
    temp = curr -> next;
    free(curr);
    curr = temp;
  }
}

//returns 1 is first string comes before second string in the dictionary
int charComparator (char * thing1, char * thing2){
  //we are dealing with strings
  char * arr1 = thing1;
  char * arr2 = thing2;
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
void insertionSortHelper(struct entry ** head,struct entry * toSort, int(*comparator)(char *, char *)){
  //declarations used for the sorting, sortee will be moved 
  struct entry * sortee = toSort;
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
    *head = sortee;
  }
}

/* How insertion sort basically works is that the first node is ALREADY SORTED in theory! */
int insertionSort(struct entry ** toSort, int(*comparator)(char *, char *)){
  //declarations used for the sorting, sortee will be moved
  struct entry * sortee = *toSort;
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
    insertionSortHelper(toSort, sortee, comparator);
    //change sortee's to its next
    sortee = sortee -> next;
  }
  //we're finished
  return 0;
}

/*helper method to populate the manifest linked lists*/
void populateManifest(char * buffer, struct entry ** head){
  //loop through the given buffer and extract the info
  struct entry * prev = NULL;
  struct entry * next = NULL;
  struct entry * curr = NULL;
  //this stores every line
  char * line;
  line = strtok(buffer, "\n");
  line = strtok(NULL, "\n");
  //loop through and do repeat
  while(line != NULL){
    //allocate a new node (freed)
    struct entry * curr = (struct entry *)malloc(sizeof(struct entry));
    //memsets so that we don't run into bugs
    memset(curr -> filePath, '\0', PATH_MAX);
    memset(curr -> fileHash, '\0', SHA_DIGEST_LENGTH+1);
    //now store the items in the line in their correct positions
    sscanf(line, "%s %d %s", curr -> filePath, &(curr -> fileVer), curr -> fileHash);
    //make curr's next equal to the next from above
    curr -> next = next;
    //make next equal curr
    next = curr;
    //tokenize a new line
    line = strtok(NULL, "\n");
  }
  //next is the node at the front
  *head = next;
  //one we are done populating the list we need to go through the list again and get previouses
  curr = *head;
  //loop through the list
  while(curr != NULL){
    //assign the previouses
    curr -> prev = prev;
    prev = curr;
    curr = curr -> next;
  }
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
  int newVersionNumber;
  //given the project name, we need to create a path to the manifest
  char manPath[PATH_MAX];
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Conflict");
  //check to see if a .conflict file exists
  int conflictFD = open(manPath, O_RDONLY);
  if(conflictFD > 0){
    printf("Please resolve conflicts before continuing.\n");
    close(conflictFD);
    return 1;
  }
  //this method connects to the server at the given IP address and populates the global hostent
  if(connectToServer()){
    printf("We're having difficulties connecting to the server at the given IP address and port.\n");
    return 1;
  }
  //get the version number from the server
  write(sfd, "manifestVersion", 16);
  read(sfd, &newVersionNumber, sizeof(int));
  //path to the .update file
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Update");
  //check to see if a .update file exists on the client side
  int updateFD = open(manPath, O_RDONLY);
  //if confd is negative, return unsuccessfull
  if(updateFD < 0){
    printf("There are no updates at this time, please try again later.\n");
    return 1;
  }
  //buffer to store the update file
  char * updates;
  //read in the update file
  int updateBytes = readFile(updateFD, &updates);
  //check if there are any updates
  if(updateBytes == 0){
    printf("There are no updates at this time, please try again later.\n");
    remove(manPath);
    return 1;
  }
  //tokenize through the updates
  char * line;
  line = strtok(updates, "\n");
  //store the instructions
  char instruction;
  char path[PATH_MAX];
  char hash[SHA_DIGEST_LENGTH+1];
  //memset those things to prevent bugs
  memset(path, '\0', PATH_MAX);
  memset(hash, '\0', SHA_DIGEST_LENGTH+1);
  //path to the manifest
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Manifest");
  //open the .Manifest file and read it
  int manFD = open(manPath, O_RDONLY);
  //read the file
  char * manifestBuffer;
  readFile(manFD, &manifestBuffer);
  //also populate the client manifests to make the changes to files
  populateManifest(manifestBuffer, &clienManHead);
  struct entry * curr;
  char * serverFile;
  int fileFD;
  int fileSize;
  int version;
  //loop through the updates
  while(line != NULL){
    //curr starts off at the head
    curr = clienManHead;
    //read each line
    sscanf(line, "%c %s %s %d", &instruction, path, hash, &version);
    //first check for deletions
    if(instruction == 'D'){
      //find that node in the client linked list
      while(curr != NULL){
        //check if the path matches
        if(strcmp(curr -> filePath, path) == 0){
          //edge cases
          if(curr -> prev == NULL && curr -> next == NULL){
            clienManHead = NULL;
          } else if(curr -> prev == NULL && curr -> next != NULL){
            clienManHead = curr -> next;
            clienManHead -> prev = NULL;
          } else if(curr -> prev != NULL && curr -> next == NULL){
            curr -> prev -> next = NULL;
          } else{
            curr -> prev -> next = curr -> next;
            curr -> next -> prev = curr -> prev;
          }
          free(curr);
          //now delete the selected file
          //remove(path); ask professor about this line
          break;
        }
        curr = curr -> next;
      }
    } else if(instruction == 'A'){
      //we would need to retrieve the file from the server and add it to the manifest
      fileFD = open(path, O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
      //follow protocol to retrieve the selected file from the server
      write(sfd, "File: ", 6);
      write(sfd, path, strlen(path));
      read(sfd, &fileSize, sizeof(int));
      if(fileSize < 0){
        printf("The project/file you are looking for does not exist.\n");
        return 1;
      }
      serverFile = (char *)malloc((fileSize+1)*sizeof(char));
      read(sfd, serverFile, fileSize);
      serverFile[fileSize] = '\0';
      //Then write that shit to the file we just opened
      write(fileFD, serverFile, fileSize);
      //now we add a new node to the client linked list
      curr  = (struct entry *)malloc(sizeof(struct entry));
      curr -> next = clienManHead;
      curr -> prev = NULL;
      strcpy(curr -> filePath, path);
      curr ->fileVer = version;
      strcpy(curr -> fileHash, hash);
      clienManHead -> prev = curr;
      clienManHead = curr;
      //then close the file descriptor
      close(fileFD);
    } else if(instruction == 'M'){
      //we would need to retrieve the file from the server and add it to the manifest
      fileFD = open(path, O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
      //follow protocol to retrieve the selected file from the server
      write(sfd, "File: ", 6);
      write(sfd, path, strlen(path));
      read(sfd, &fileSize, sizeof(int));
      if(fileSize < 0){
        printf("The project/file you are looking for does not exist.\n");
        return 1;
      }
      serverFile = (char *)malloc((fileSize+1)*sizeof(char));
      read(sfd, serverFile, fileSize);
      serverFile[fileSize] = '\0';
      //Then write that shit to the file we just opened
      write(fileFD, serverFile, fileSize);
      //loop through to find the entry to be modified and modify it
      while(curr != NULL){
        if(strcmp(curr -> filePath, path) == 0){
          curr -> fileVer = version;
          strcpy(curr -> fileHash, hash);
          break;
        }
        curr = curr -> next;
      }
    }
  }
  rewriteManifest(clienManHead, manPath, newVersionNumber);
  //now we remove the .Update file from the client
  memset(manPath, '\0', PATH_MAX);
  strcpy(manPath, projName);
  strcat(manPath, "/.Update");
  remove(manPath);
  //close the files
  close(updateFD);
  close(manFD);
  close(sfd);
  //free the buffer
  free(updates);
  free(manifestBuffer);
  freeLL(clienManHead);
  return 0;
}

/*thie method takes in a manifest linked list and rewrites the manifest*/
void rewriteManifest(struct entry * head, char * path, int version){
  //first open the manifest
  int manFD = open(path, O_TRUNC | O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR);
  //write the newest version number on the first line
  write(manFD, &version, sizeof(int));
  write(manFD, '\n', 1);
  struct entry * curr = head;
  //loop through the linked list and write accordingly
  while(curr != NULL){
    write(manFD, curr -> filePath, strlen(curr -> filePath));
    write(manFD, " ", 1);
    write(manFD, curr->fileHash, strlen(curr->fileHash));
    write(manFD, " ", 1);
    write(manFD, &(curr->fileVer), sizeof(int));
    write(manFD, "\n", 1);
    curr = curr -> next;
  }
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
  return 0;
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
  return 0;
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

/*helper method to read files returns the number of bytes in the buffer*/
int readFile(int fd, char ** buff){
  //if confFD is positive, configure has been created, extract the IP and Port
  int buffSize = lseek(fd, (size_t)0, SEEK_END);
  //store the contents of the config file (freed)
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