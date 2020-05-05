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

struct tNode {
  pthread_t thread;
  struct tNode * next;
}; //thread node definition

void handler (int signa);
int creator (char * name);
int checkers (char * name);
int fyleBiter (int fd, char ** buffer);
int mkdir(const char * pathname, mode_t mode);
int traverser (DIR * myDirectory, int counter, int currSize, char * currDirec, char ** files);
int writeFile(char * path);
void * tstart (void * sock); // thread handler

int lsocket; //declaring the file descriptor for our listening (Server) socket
int x = 42; // while loop handler in honor of Jackie Robinson
struct tNode * root = NULL;
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;  // global mutex

int main (int argc, char ** argv) {
    signal(SIGINT, handler);
    int csocket; // declaring the file descriptor from the respective client socket
    int caddysize = -1;
    int portnum = atoi(argv[1]);
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
    listen(lsocket, 3); // has 0 clients on backlog
    while (x != 6900) { // while loop that creates threads from new clients being accepted
      csocket = accept(lsocket, (struct sockaddr *) &clientaddy, (socklen_t *) &caddysize);
      printf("New client has connected!\n");
      pthread_t pthread = NULL;
      int * socketp = (int *) malloc(sizeof(int));
      *socketp = csocket;
      if (root == NULL) {
        root = (struct tNode *) malloc(sizeof(struct tNode));
        root->thread = pthread;
        root->next = NULL;
      }
      else {
        struct tNode * temp = root;
        while (temp->next != NULL) {
          temp = temp ->next;
        }
        temp->next = (struct tNode *) malloc(sizeof(struct tNode));
        temp->next->thread = pthread;
        temp->next->next = NULL;

      }
      pthread_create(&pthread, NULL, &tstart, socketp);
    }
      void * val;
      while (root != NULL) {
        pthread_join(root->thread, &val);
        root = root -> next;
      }
      pthread_mutex_destroy(&locker); // destroying our mutex

    return 0;
}


int creator (char * name) { // will see if the name of the project is there or not, whatever
    char manPath[PATH_MAX];
    char hisPath[PATH_MAX];
    char newMan[1] = {'1'};
    memset(manPath, '\0', PATH_MAX);
    memset(hisPath, '\0', PATH_MAX);
    strcpy(manPath, name);
    strcpy(hisPath, name);
    strcat(manPath, "/.Manifest");
    strcat(hisPath, "/.History" );
    int mfd; // manpage file descriptor
    DIR * proj = opendir(name);
    if (!proj) { // the directory does not exist in this world
      mkdir(name, S_IRWXU); // makes the directory
      mfd = open(manPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates manifest in directory
      open(hisPath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // creates the history in the directory also
      write(mfd, newMan, 1);
      return 1;
    }
    else {
      return -1; // the project already existed!!
    } 
}

int checkers (char * name) { // almost like the opposite of create in my opinion, check to see if project exists!
  DIR * proj = opendir(name);
  if(!proj) { // project doesnt exist!
    return -1;
  }
  else {
    return 1; // the project exists!
  }
}

int fyleBiter (int fd, char ** buffer) { // given a file path, it will find how many bytes are within a file!!
  int buffSize = lseek(fd, (size_t)0, SEEK_END);
  *buffer = malloc((buffSize+1)*sizeof(char)); //store the contents of the config file (freed)
  lseek(fd, (size_t)0, SEEK_SET);   //set the offset back to the start
  memset(*buffer, '\0', buffSize+1);   //memset the buffer to null terminators
  read(fd, *buffer, buffSize);   //read the buffsize amount of bytes into the buffer
  return buffSize + 1;   //return the size of the file

}


int traverser (DIR * myDirectory, int count, int currSize, char * currDirec, char ** files){ // takes in a directory and gets files!
  char filePBuff[PATH_MAX + 1]; //stores the filepath of our subdirectories
  strcpy(filePBuff, currDirec); //in the case of recursion, update the filepath so that we do not get lost
  strcat(filePBuff, "/"); //add a forward-slash at the end to get ready to add more to the path
  struct dirent * currDir; //the dirent struct which holds whatever readdir returns
  while((currDir = readdir(myDirectory)) != NULL){ //loop through the contents of the directory and store in files array
    if(strcmp(currDir->d_name, ".") == 0 || strcmp(currDir->d_name, "..") == 0 || strcmp(currDir->d_name,".DS_Store") == 0){ //skip the . and .. and dsstore file
      continue; //skip the iteration
    }
    if(currDir->d_type == DT_DIR) { //directory is a directory
      strcat(filePBuff, currDir->d_name); //add the directory in question to the path
      count = traverser(opendir(filePBuff), count, currSize, filePBuff, files); //traverse the new directory
      strcpy(filePBuff, currDirec); //we are back in the original file, get rid of the previous file path
      strcat(filePBuff, "/"); //put the forward-slash back in there
      currSize = ((count%100)+1)*100; //find the new max size of the array
    } 
    else if(currDir -> d_type == DT_REG){ // directory is a file
      files[count] = (char *)malloc((PATH_MAX+1) * sizeof(char)); //allocate space for the file path
      strcpy(files[count],filePBuff); //add the file path to the array
      strcat(files[count], currDir->d_name); //store the names of the files in our files array
      if(++count >= currSize){   //check if files array needs more space
        //realloc 100 more spaces in our files array
        files = realloc(files, (currSize+100) * sizeof(char *));  //realloc 100 more spaces in our files array
        currSize += 100;  //change the curr size accordingly
      }
    }
  }
  return count; //return the current count
}

int writeFile(char * path){
  /*line: holds the part of the string to append, next is used to make sure that we do not mkdir the final
  piece wich is the file, appendage string holds the parts, file size stores the number of bytes to be read
  server file holds the file that is retrieved from the server*/
  char original[PATH_MAX];
  strcpy(original, path);
  char * notLine = strtok(original, "/");
  char * next = strtok(NULL, "/");
  char appendageString[PATH_MAX];
  memset(appendageString, '\0', PATH_MAX);
  DIR *myDirec;
  //loop through and get all the subdirectories
  while(next != NULL){
    strcat(appendageString, notLine);
    //check is subdirectory exists
    myDirec = opendir(appendageString);
    if(!myDirec){
      mkdir(appendageString, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    } else{
      free(myDirec);
    }
    //traverse
    notLine = next;
    next = strtok(NULL, "/");
    strcat(appendageString, "/");
  }
  strcat(appendageString, notLine);
  return 0;
}

void handler (int signa) {
  x = 6900; // sets while loop handler to the number that will BREAK IT
  close(lsocket);
  return; 
}

void * tstart (void * sock) {

  int manSuc = 1;
  int manFail = -1;
  int * socketp = (int *) sock;
  int csocket = * socketp;
  char sCon[7] = {'h', 'e', 'l', 'l','o','o', '\0'};
  send(csocket, sCon, 7, 0); // success message!
  char crequest[6]; // get requests from clients!
  memset(crequest, '\0', 6);
  while (recv(csocket, crequest, 5, 0)) {
    printf("The client has requested the server to: %s\n", crequest);
  if (crequest[3] == 'a') { // this means you know you have to create the project (Will come in as Crea:)
    pthread_mutex_lock(&locker);
    int projLen = 0;
    recv(csocket, &projLen, sizeof(int), MSG_WAITALL);
    char projName[projLen];
    int creRet; // what creator returns!
    memset(projName, '\0', projLen);
    recv(csocket, projName, projLen, MSG_WAITALL);
    creRet = creator(projName);
    if (creRet == 1) { // the file was created successfully
      send(csocket, &manSuc, sizeof(int), 0); // send 1 to the client
    }
    else { // file already existed yo
      send(csocket, &manFail, sizeof(int), 0);
    }
    pthread_mutex_unlock(&locker); 
  }
  else if(crequest[0] == 'F') { // FILE:Path command
    pthread_mutex_lock(&locker);
    int numBytes; // the number of bytes in a file
    int pfd; // file descriptor for Path
    int pathSize;
    recv(csocket, &pathSize, sizeof(int), 0);
    char pathName[pathSize]; // the path where the file to be opened resides
    char * fileBuf; // storing bytes in a file
    memset(pathName, '\0', pathSize);
    recv(csocket, &pathName, pathSize, MSG_WAITALL);
    pfd = open(pathName, O_RDONLY, S_IRUSR | S_IWUSR);
    if(pfd == -1) {
      send(csocket, &manFail, sizeof(int), 0);
    }
    numBytes = fyleBiter(pfd, &fileBuf);
    send(csocket,&numBytes, sizeof(int), 0);
    send(csocket, fileBuf, numBytes, 0);
    close(pfd);
    pthread_mutex_unlock(&locker);
  }
  else if(crequest[0] == 'D') { // destroying a directory!!
    pthread_mutex_lock(&locker);
    int dsize;
    recv(csocket, &dsize, sizeof(int), MSG_WAITALL);
    char dName[dsize]; // name of
    memset(dName, '\0', dsize);
    recv(csocket, &dName, dsize, MSG_WAITALL); // getting directory to be DESTROYED
    DIR * urmom = opendir(dName);
    if(!urmom) { // said directory does not exist
      pthread_mutex_unlock(&locker);
      return NULL;
    }
    char deletor[dsize + 7];
    memset(deletor, '\0', dsize + 7);
    strcat(deletor, "rm -rf ");
    strcat(deletor, dName);
    system(deletor); // using the system call to do this
    pthread_mutex_unlock(&locker);  
  }
  else if (crequest[1] == 'h') { // checks to see if file exists
    pthread_mutex_lock(&locker);
    int returnstat;
    int fileNameS;
    recv(csocket, &fileNameS, sizeof(int), MSG_WAITALL);
    char project[fileNameS];
    memset(project, '\0', fileNameS);
    recv(csocket, &project, fileNameS, MSG_WAITALL);
    returnstat = checkers(project);
    if(returnstat == 1) { // success!
      send(csocket, &manSuc, sizeof(int), 0);
    }
    else {
      send(csocket, &manFail, sizeof(int), 0);
    }
    pthread_mutex_unlock(&locker);
  }
  else if(crequest[3] == 'j') { // Proj: protocol
    pthread_mutex_lock(&locker);
    char ** filer; // the array that holds the files necessary, stuff in the manifest
    filer = (char **) malloc(999 * sizeof(char *));
    char * fileboof; // buffer to store file into
    int nbytes; // number of bytes in a file
    int lenProjName; // length of project name
    int bfg; // file descriptors for each file!
    int md; // manifest file descriptor
    int i = 0; // first index of filer!
    recv(csocket, &lenProjName, sizeof(int), MSG_WAITALL); // getting lentgh of proj name + 1 from client
    char pject[lenProjName]; // making buffer
    memset(pject, '\0', lenProjName); // presetting it beforehand
    recv(csocket, &pject, lenProjName, MSG_WAITALL); // getting project name from client
    int numfiles = 0; // number of files within said project
    // need to send client number of files from the MANIFEST! Not the directory itself!
    char manFilePath[PATH_MAX]; // file path of manifest
    memset(manFilePath, '\0', PATH_MAX); // setting all of it to null terminator
    strcpy(manFilePath, pject); // adding project name to manifest file path
    strcat(manFilePath, "/.Manifest"); // making it the manifest file path
    char * manBoof;
    char * line; // the tokenized line
    char checksPath[PATH_MAX]; // path of each file we need to send
    int fpVer; // file version 
    char hash[40 + 1]; // hash of the file
    char tag; // tag of the character
    md = open(manFilePath, O_RDONLY, S_IRUSR | S_IWUSR); // opening manifest with proper permissions
    fyleBiter(md, &manBoof); // using adviths method to load manifest into buffer!
    line = strtok(manBoof, "\n"); // getting the real line we need!
    line = strtok(NULL, "\n"); // tokenizing again
    while(line != NULL) { // traversing the buffer
      memset(checksPath, '\0', PATH_MAX);
      sscanf(line, "%s %d %s %c", checksPath, &fpVer, hash, &tag); // scanning each line and storing that shit
      filer[i] = malloc((strlen(checksPath)+1)*sizeof(char));
      strcpy(filer[i], checksPath); // loading the said filePath into the filer array
      numfiles++; // increasing numFiles by 1
      i++; // increasing i to traverse the array
      line = strtok(NULL, "\n"); // tokenizing again
    }
    close(md);
    numfiles = numfiles + 1; // increasing numfiles
    filer[i] = manFilePath; // adding manifest in files to send to client
    send(csocket, &numfiles, sizeof(int), 0); // send client number of files
    i = 0; // resetting array index to 0
    int len = 0; // lenth of filer
    char tempPath[PATH_MAX]; // path of file
    while (i < numfiles) { // traverse files array
      len = strlen(filer[i]) + 1; // getting length of filepath + null terminator
      send(csocket, &len, sizeof(int), 0); // sending client length of file
      send(csocket, filer[i], len, 0); // send path of file
      recv(csocket, tempPath, 5, MSG_WAITALL); // getting file:
      recv(csocket, &len, sizeof(int), MSG_WAITALL); // sending length of projname 
      recv(csocket, tempPath, len, MSG_WAITALL); // getting file path itself
      bfg = open(filer[i], O_RDONLY, S_IRUSR | S_IWUSR); // open the file
      nbytes = fyleBiter(bfg, &fileboof); // how many bytes in said file
      send(csocket, &nbytes, sizeof(int), 0); // sending number of bytes to client
      send(csocket, fileboof, nbytes, 0); // sending buffer to client
      i++;
      close(bfg);
      free(fileboof);
      }
    i = 0;
    while (i < numfiles - 1) {
      free(filer[i]);
      i++;
    }
    free(filer);
    pthread_mutex_unlock(&locker);
  }
  else if(crequest[3] == 'm') { // commit!!
    pthread_mutex_lock(&locker);
    int fps; // the file path size including null terminator
    int cfd; // file descriptor for the commit file that will be given to us
    int eb; // expected bytes for the committ
    recv(csocket, &fps, sizeof(int), MSG_WAITALL); // get file path size including null from client and store into fps
    char fcomm[fps]; // making char array that the file path will be loaded into!
    memset(fcomm, '\0', fps); // setting everything to null terminator beforehand
    recv(csocket, fcomm, fps, MSG_WAITALL); // getting the actual file path from the client, includes the hashcode
    cfd = open(fcomm, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // making the commit file in the proper directory
    recv(csocket, &eb, sizeof(int), MSG_WAITALL); // getting number of bytes contained in committ
    char cbuff[eb]; // making a buffer for the commit, based on bytes in it
    memset(cbuff, '\0', eb); // setting everything in buffer to null terminator
    recv(csocket, cbuff, eb, MSG_WAITALL); // loading the contents of the commit into said buffer
    write(cfd, cbuff, eb - 1); // writing to said commit file the contents, write eb number of bytes(whole cbuff)
    close(cfd);
    pthread_mutex_unlock(&locker);
  }
  else if(crequest[3] == 'h') { // the push command
    pthread_mutex_lock(&locker);
    int filePathSize; // file path size including null terminator
    int projNL; // the project name length!
    recv(csocket, &projNL, sizeof(int), MSG_WAITALL); // getting projNamelen from client
    char projName[projNL]; // making string for project name
    memset(projName, '\0', projNL); // memsetting all to null terminator
    recv(csocket, projName, projNL, MSG_WAITALL); // GETTING project name from client
    int fcd; // file descriptor of commit
    int md; // manifest file descriptor
    int hd; // file descriptor of history
    int mfv = 0; // manifest file version
    char * comBuf; // the buffer file thatll hold the bytes in the commit!
    char * manBuf; // the buffer thatll hold the bytes in the manifest
    recv(csocket, &filePathSize, sizeof(int), MSG_WAITALL);
    char commfp[filePathSize]; // initializing the buffet thatll hold the file path to the specific commit
    memset(commfp, '\0', filePathSize); // setting them all to 0
    recv(csocket, commfp, filePathSize, MSG_WAITALL); // getting the said file path of the commit!
    fcd = open(commfp, O_RDONLY, S_IRUSR | S_IWUSR); // open the commit, with read only permission
    if (fcd == -1) { // file does not exist!
      send(csocket, &manFail, sizeof(int), 0);
      pthread_mutex_unlock(&locker);
      return NULL;
    }
    else { // the file exists!
      send(csocket, &manSuc, sizeof(int), 0);
    }
    fyleBiter(fcd, &comBuf); // storing the commit into the buffer 
    char histfp[PATH_MAX]; // file path of the projects history
    memset(histfp, '\0', PATH_MAX); // setting everything to null terminator
    strcpy(histfp, projName); // adding projdirectory part to history file path
    strcat(histfp, "/.History"); // adding history part to history file path
    char manfp[PATH_MAX]; // creating file path for manifest
    memset(manfp, '\0', PATH_MAX); // presetting everything in buffer to null terminator
    strcpy(manfp, projName); // setting project/ to file path
    strcat(manfp, "/.Manifest"); // adding the .Manifest file to it;
    md = open(manfp, O_RDONLY, S_IRUSR | S_IWUSR); // open the manifest
    fyleBiter(md, &manBuf); // loading manifest into the buffer
    close(md);
    sscanf(manBuf, "%d\n", &mfv); // getting project version from manifest buffer
    char pvBUF[999];
    memset(pvBUF, '\0', 999);
    sprintf(pvBUF, "%d", mfv); // setting project version into the buffer
    char backup[8] = {'b','a','c','k','u','p','_','\0'};
    char bName[1500];
    memset(bName, '\0', 100);
    strcpy(bName, backup); // "backup_"
    strcat(bName, projName); // "backup_projName"
    strcat(bName, "_"); // "backup_projName_"
    strcat(bName, pvBUF); // ex: "backup_projName_2"
    mkdir(bName, S_IRWXU);
    char bBuff[projNL + strlen(bName) + 100];
    memset(bBuff, '\0', projNL + strlen(bName) + 100);
    strcat(bBuff, "cp -R ");
    strcat(bBuff, projName);
    strcat(bBuff, "/. ");
    strcat(bBuff, bName);
    system(bBuff); // storing the stuff from the old directory into the new directory! (duplication)
    strcat(pvBUF, "\n");
    hd = open(histfp, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR); // opening the history with the 
    write(hd, pvBUF, strlen(pvBUF)); // writing the manifest version number to the history
    write(hd, comBuf, strlen(comBuf)); // writing the commit to the history!
    char * line; // the particular line in the commit
    char * commCopy = malloc(strlen(comBuf)+1);
    memset(commCopy, '\0', strlen(comBuf)+1);
    strcpy(commCopy, comBuf);
    line = strtok(commCopy, "\n"); // tokenizing it by new line
    int i = 0;
    int x = 0;
    char instruction;
    char hash[40+1]; 
    int fileVer;
    int currFD;
    char checksPath[PATH_MAX];
    int fplen;
    int byter; 
    while(line != NULL){
      memset(checksPath, '\0', PATH_MAX);
      sscanf(line, "%c %s %s %d", &instruction, checksPath, hash, &fileVer);
      if(instruction == 'A') {
        recv(csocket, &fplen, sizeof(int), MSG_WAITALL);
        char fpName[fplen]; // creating buffer for file path
        memset(fpName, '\0', fplen);
        recv(csocket, fpName, fplen, MSG_WAITALL);
        writeFile(fpName);
        currFD = open(fpName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // create the file with read and write permissions
        recv(csocket, &byter, sizeof(int), MSG_WAITALL); // getting bytes in file
        char fb[byter]; // buffer for the file
        recv(csocket, fb, byter, MSG_WAITALL);
        write(currFD, fb, byter);
        close(currFD);
      }
      else if(instruction == 'M') {
        recv(csocket, &fplen, sizeof(int), MSG_WAITALL);
        char fpName[fplen]; // creating buffer for file path
        memset(fpName, '\0', fplen);
        recv(csocket, fpName, fplen, MSG_WAITALL);
        currFD = open(fpName, O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR); // open the file but get rid of everything in it since its modified
        recv(csocket, &byter, sizeof(int), MSG_WAITALL); // getting bytes in file
        char fb[byter]; // buffer for the file
        recv(csocket, fb, byter, MSG_WAITALL);
        write(currFD, fb, byter);
        close(currFD);
      }
      else if(instruction == 'D') { // delete file
        remove(checksPath); // we dont get anything from the client!
      }
      strcpy(commCopy, comBuf);
      line = strtok(commCopy, "\n");
      while(x <= i && line != NULL){
        line = strtok(NULL, "\n");
        x++;
      }
      x = 0;
      i++;
    }

    int mlen;
    int nmd; // new manifest fd
    recv(csocket, &mlen, sizeof(int), MSG_WAITALL);
    char mP[mlen]; // manifest path
    memset(mP, '\0', mlen);
    recv(csocket, mP, mlen, MSG_WAITALL); // populates manifest path
    nmd = open(mP, O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR); // open manifest but also clear it and give read write permission
    int mbytes; // number of bytes in manifest
    recv(csocket, &mbytes, sizeof(int), MSG_WAITALL);
    char manBoof[mbytes];
    memset(manBoof, '\0', mbytes);
    recv(csocket, manBoof, mbytes, MSG_WAITALL); // populate the manifest buffer
    write(nmd, manBoof, mbytes);
    int nfiles; // number of files in the directory
    DIR * projtrav = opendir(projName); // creating a dirent struct from the project directory
    char ** commitdel;
    commitdel = (char **) malloc(100 * sizeof(char *));
    nfiles = traverser(projtrav, 0, 100, projName, commitdel); // getting number of files and storing stuff into commitdel
    int j = 0;
    while (j < nfiles) { // while loop removing all commits
      if (strlen(commitdel[j]) == (48 + projNL)) {
        remove(commitdel[j]); // gettinf rid of the commit
      }
      j++;
    }
    close(fcd);
    close(nmd);
    close(hd);
    pthread_mutex_unlock(&locker);
  }
  else if(crequest[0] == 'R') { // rollback method!
    pthread_mutex_lock(&locker);
    int projNameLen; // length of project name
    int pv; // current proj version
    recv(csocket, &projNameLen, sizeof(int), MSG_WAITALL); // getting length of project name from client
    char yName[projNameLen]; // making buffer for project name
    memset(yName, '\0', projNameLen); // memsetting everything to null terminator
    recv(csocket, yName, projNameLen, MSG_WAITALL); // getting project name from client
    int version; // the version the client wants to rollback to
    recv(csocket, &version, sizeof(int), MSG_WAITALL); // getting version number from client
    char verNum[999]; // making a buffer for the verNum for the project
    memset(verNum, '\0', 999); // setting everything to null terminator
    sprintf(verNum, "%d", version); // setting string to hold the string version of requested version number
    char backup[8] = {'b','a','c','k','u','p','_','\0'};
    char bName[1500];
    memset(bName, '\0', 1500);
    strcpy(bName, backup);
    strcat(bName, yName);
    strcat(bName, "_");
    strcat(bName, verNum);
    char manifestFilePath[PATH_MAX];
    memset(manifestFilePath, '\0', PATH_MAX);
    strcpy(manifestFilePath, yName);
    strcat(manifestFilePath, "/.Manifest"); // manifest file path
    int m; // file descriptor for manifest
    m = open(manifestFilePath, O_RDONLY, S_IRUSR | S_IWUSR);
    char * mb;
    fyleBiter(m, &mb); // loading manifest into the buffer
    close(m);
    sscanf(mb, "%d\n", &pv); // getting project version from manifest buffer
    pv = pv - 1;
    char bup[8] = {'b','a','c','k','u','p','_','\0'};
    char vn[999];
    while (pv > version) { // destroy any backups in between
      char bN[1500];
      memset(bN, '\0', 1500);
      strcpy(bN, bup);
      strcat(bN, yName);
      strcat(bN, "_");
      memset(vn, '\0', 999);
      sprintf(vn, "%d", pv);
      strcat(bN, vn);
      system("rm -rf bName");
      pv--; // decreasing pv
    }
    system("rm -rf yName");
    rename(bName, yName); // old project name becomes regular project name
    pthread_mutex_unlock(&locker);
  }
  }
  return NULL;
}