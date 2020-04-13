#include "clientHeader.h"

int main (int argc, char ** argv){
//check if we need to configure the client (setup the ip and port)
if(strcmp(argv[1], "configure") == 0){
  //call the configure method and pass in ip and port
  if(configure(argv[2], argv[3])){
    //there was an error configuring the file
    printf("There was an error configuring the file.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "checkout") == 0){
  //call the checkout method and pass in the project name to be checked out
  if(checkout(argv[2])){
    //there was some sort of problem
    printf("There was an error checking out the project.\n");
    exit(0);
  }
}
return 0;
}