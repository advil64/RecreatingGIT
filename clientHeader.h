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

//global variables
int port;
char IP[100];

//functions
int configure(char *, char *);
int checkout(char *);
int readConf();
void readFile(int, char **);
#endif