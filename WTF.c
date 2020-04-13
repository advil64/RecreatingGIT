#include "clientHeader.h"

int main (int argc, char ** argv){
//check if we need to configure the client (setup the ip and port)
if(strcmp(argv[1], "configure") == 0){
  //first check that there are the right number of arguments
  if(argc != 4){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the configure method and pass in ip and port
  if(configure(argv[2], argv[3])){
    //there was an error configuring the file
    printf("There was an error configuring the file.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "checkout") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the checkout method and pass in the project name to be checked out
  if(checkout(argv[2])){
    //there was some sort of problem
    printf("There was an error checking out the project.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "update") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the update method 
  if(update(argv[2])){
    //print an error
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "upgrade") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the upgrade method 
  if(upgrade(argv[2])){
    //print an error
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "commit") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the commit method 
  if(commit(argv[2])){
    //print an error
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "push") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the commit method 
  if(push(argv[2])){
    //print an error
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
}
return 0;
}