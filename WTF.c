#include "clientHeader.h"

int main (int argc, char ** argv){
//check if we need to configure the client (setup the ip and port)
if(strcmp(argv[1], "configure")){
  //call the configure method and pass in ip and port
  if(configure(argv[2], argv[3])){
    //there was an error configuring the file
    printf("There was an error configuring the file");
  }
}


}