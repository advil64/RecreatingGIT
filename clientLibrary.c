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
  write(confFD, "IP/Hostname: ", 13*sizeof(char));
  write(confFD, myIp, strlen(myIp)*sizeof(char));
  write(confFD, "\n", sizeof(char));
  //write the port number to the configure file
  write(confFD, "Port: ", 6*sizeof(char));
  write(confFD, myPort, strlen(myPort)*sizeof(char));
  //return 0 is it was successfull
  return 0;
}