/*importing all necessary libraries needed */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

int main (int argc, char ** argv) {
    int lsocket; // declaring the file descriptor for our listening (server) socket
    int csocket; // declaring the file descriptor from the respective client socket
    lsocket = socket(AF_INET, SOCK_STREAM, 0); // creating the socket

    /* stuff that comprises the server addy */
    struct sockaddr_in serveraddy;
    serveraddy.sin_family = AF_INET;
    serveraddy.sin_port = argv[1]; // port number must be taken from command line
    serveraddy.sin_addr.s_addr = INADDR_ANY;

    /*binding */
    bind(lsocket, (struct sockaddr*) &serveraddy, sizeof(serveraddy));

    /*listening */
    listen(lsocket, 10); // has 10 clients on backlog

    /* accepting shit from clients */
    csocket = accept(lsocket, NULL, NULL); // setting info to NULL rn
    return 0;
}