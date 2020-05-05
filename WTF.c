#include "clientHeader.h"

int main (int argc, char ** argv){
//check if we need to configure the client (setup the ip and port)
if(strcmp(argv[1], "configure") == 0){
  //first check that there are the right number of arguments
  if(argc != 4){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the configure method and pass in ip and port
  if(configure(argv[2], argv[3])){
    printf("There was an error configuring the file.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "currentversion") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the checkout method and pass in the project name to be checked out
  if(currentversion(argv[2])){
    printf("There was an error checking out the project.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "checkout") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the checkout method and pass in the project name to be checked out
  if(checkout(argv[2])){
    printf("There was an error checking out the project.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "update") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the update method and give the project name
  if(update(argv[2])){
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "upgrade") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the upgrade method 
  if(upgrade(argv[2])){
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "commit") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the commit method 
  if(commit(argv[2])){
    printf("There was an error while updating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "add") == 0){
  //first check that there are the right number of arguments
  if(argc != 4){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the commit method 
  if(add(argv[2], argv[3])){
    //print an error
    printf("There was an error while adding %s to the %s project.\n", argv[3], argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "create") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    //print an error
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the create method 
  if(create(argv[2])){
    //print an error
    printf("There was an error while creating the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "destroy") == 0){

  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }

  //read the configure file and get the socket
  connectToServer();
  send(sfd, "Dest:", 5, 0);
  int len = strlen(argv[2])+1;
  send(sfd, &len, sizeof(int), 0);
  send(sfd, argv[2], strlen(argv[2]), 0);
  close(sfd);
} else if(strcmp(argv[1], "remove") == 0){
  //first check that there are the right number of arguments
  if(argc != 4){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //call the remove method 
  if(removeMan(argv[2], argv[3])){
    printf("There was an error while removing %s from the %s project.\n", argv[3], argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "history") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //we need to call history method
  if(history(argv[2])){
    printf("There was an error while fetching the history for the %s project.\n", argv[2]);
    exit(0);
  }
} else if(strcmp(argv[1], "push") == 0){
  //first check that there are the right number of arguments
  if(argc != 3){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //now push the commit
  if(push(argv[2])){
    printf("Unable to push the latest version of this project.\n");
    exit(0);
  }
} else if(strcmp(argv[1], "rollback") == 0){
  //check if there are indeed 4 arguments
  if(argc != 4){
    printf("Insufficient arguments.\n");
    exit(0);
  }
  //connect to the server
  connectToServer();
  //now follow the rollback protocol
  send(sfd, "Roll:", 5, 0);
  int len = strlen(argv[2]) + 1;
  send(sfd, &len, sizeof(int), 0);
  send(sfd, argv[2], len, 0);
  int version = atoi(argv[3]);
  send(sfd, &version, sizeof(int), 0);
  int status;
  recv(sfd, &status, sizeof(int), MSG_WAITALL);
  if(status < 0){
    printf("There was an error trying to rollback your project, try again.\n");
    exit(0);
  }
} else {
  printf("Unknown command %s, please consult the readme for the commands.", argv[1]);
  exit(0);
}
printf("Your %s command was successfully executed.\n", argv[1]);
return 0;
}