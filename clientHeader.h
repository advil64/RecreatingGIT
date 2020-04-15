#ifndef _CLIENTHEADER_H_
#define _CLIENTHEADER_H_

//libraries to import
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include<math.h>
#include<netdb.h>
#include<pthread.h>
#include<signal.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <openssl/sha.h>

//struct declarations, this one is for the contents of the manifest
struct entry {
    char * filePath;
    int fileVer;
    char * fileHash;
    struct entry * next;
    struct entry * prev;
};

//global variables
int port;
char IP[100];

//functions
int configure(char *, char *);
int checkout(char *);
int readConf();
int readFile(int, char **);
struct entry ** populateManifest(char *);
int update(char *);
int charComparator (void*, void*);
int insertionSortHelper(void*, int(*comparator)(void*, void*));
struct entry ** insertionSort(void*, int(*comparator)(void*, void*));
#endif