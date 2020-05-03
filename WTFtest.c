#include <stdlib.h>

int main (int argc, char ** argv){

    //first we test the ability of both the server and client to create projects
    system("./WTF create testDirectory");
    //next we test the ability of adding files in the directory/subdirectories to the manifest
    system("./WTF add testDirectory test2.txt");
    system("./WTF add testDirectory subDirec2/subDirec1/test7.txt");
    //test the ability of adding large files with random bytes to the manifest
    system("./WTF add testDirectory subDirec2/largeFile2");
    //test the ability of committing all the added files to the server
    system("./WTF commit testDirectory");
    //test being able to send all files to the server
    system("./WTF push testDirectory");
    //now we modify one of the added files and commit and push to see if the change takes place
    system("echo \"This systems project was amazing\" >> testDirectory/test2.txt");
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");
    //now we test the remove function and see if the file is removed from the server
    system("./WTF remove testDirectory subDirec2/largeFile2");
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");
    //update and upgrade would not be useful but just to check that they work
    system("./WTF update testDirectory");
    system("./WTF upgrade testDirectory");
}   