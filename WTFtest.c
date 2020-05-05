#include <stdlib.h>
#include <pthread.h>

void *servStart(void *);

int main (int argc, char ** argv){

    //make a client 1 and client 2 directories to test two different clients
    system("mkdir client1"); 
    system("mkdir client2");
    system("mkdir server");
    system("make");
    system("cp WTF client1/");
    system("cp WTF client2/");
    system("cp WTFserver server/");

    //start the server
    pthread_t id;
    pthread_create(&id, NULL, servStart, "server_is_starting");

    //first configure to localhost and 20202
    system("cd client1/ && ./WTF configure localhost 20202");
    system("cd client2/ && ./WTF configure localhost 20202");

    //now we will work with client 1
    
    //first we test the ability of both the server and client to create projects
    system("cd client1/ && ./WTF create testDirectory");
    //first create 2 files in the main directory
    system("cd client1/ && touch testDirectory/test1.txt");
    system("cd client1/ && touch testDirectory/test2.txt");
    system("cd client1/ && echo Letter rentaliciousosos wooded direct two men indeed income sister. Impression up admiration he by partiality is. Instantly immediate his saw one day perceived. Old blushes respect but offices hearted minutes effects. Written parties winding oh as in without on started. Residence gentleman yet preserved few convinced. Coming regret simple longer little am sister on. Do danger in to adieus ladies houses oh eldest. Gone pure late gay ham. They sigh were not find are rentaliciousosos. >> testDirectory/test2.txt");
    system("cd client1/ && echo Lose away off why half led have near bed. At engage simple father of period others except. My giving do summer of though narrow marked at. Spring formal no county ye waited. My whether cheered at regular it of promise blushes perhaps. Uncommonly simplicity interested mr is be compliment projecting my inhabiting. Gentleman he september in oh excellent. >> testDirectory/test1.txt");
    //next we test the ability of adding files in the directory/subdirectories to the manifest
    system("cd client1/ && ./WTF add testDirectory test1.txt");
    system("cd client1/ && ./WTF add testDirectory test2.txt");
    //test commit for the first time
    system("cd client1/ && ./WTF commit testDirectory");
    system("cd client1/ && ./WTF push testDirectory");
    //now test the ability of adding files in subdirectories
    system("cd client1/ && mkdir -p testDirectory/subDirec1 && touch testDirectory/subDirec1/test3.txt");
    system("cd client1/ && touch testDirectory/subDirec1/test4.txt");
    system("cd client1/ && echo Questions explained agreeable preferred strangers too him her son. Set put shyness offices his females him distant. Improve has message besides shy himself cheered however how son. Quick judge other leave ask first chief her. Indeed or remark always silent seemed narrow be. Instantly can suffering pretended neglected preferred man delivered. Perhaps fertile brandon do imagine to cordial cottage. >> testDirectory/subDirec1/test3.txt");
    system("cd client1/ && echo As am hastily invited settled at limited civilly fortune me. Really spring in extent an by. Judge but built gay party world. Of so am he remember although required. Bachelor unpacked be advanced at. Confined in declared marianne is vicinity. >> testDirectory/subDirec1/test4.txt");
    system("cd client1/ && ./WTF add testDirectory subDirec1/test3.txt");
    system("cd client1/ && ./WTF add testDirectory subDirec1/test4.txt");
    system("cd client1/ && ./WTF commit testDirectory");
    system("cd client1/ && ./WTF push testDirectory");

    //now go to the second client and checkout the project
    system("cd client2/ && ./WTF checkout testDirectory");

    //test the ability to modify and commit said files to the server
    system("cd client1/ && echo Now seven world think timed while her. Spoil large oh he rooms on since an. Am up unwilling eagerness perceived incommode. Are not windows set luckily musical hundred can. Collecting if sympathize middletons be of of reasonably. Horrible so kindness at thoughts exercise no weddings subjects. The mrs gay removed towards journey chapter females offered not. Led distrusts otherwise who may newspaper but. Last he dull am none he mile hold as. > testDirectory/subDirec1/test4.txt");
    system("cd client1/ && echo It allowance prevailed enjoyment in it. Calling observe for who pressed raising his. Can connection instrument astonished unaffected his motionless preference. Announcing say boy precaution unaffected difficulty alteration him. Above be would at so going heard. Engaged at village at am equally proceed. Settle nay length almost ham direct extent. Agreement for listening remainder get attention law acuteness day. Now whatever surprise resolved elegance indulged own way outlived. > testDirectory/test2.txt");
    system("cd client1/ && ./WTF commit testDirectory");
    system("cd client1/ && ./WTF push testDirectory");
    
    //test the ability to update/upgrade the project on another client
    system("cd client2/ && ./WTF update testDirectory");
    system("cd client2/ && ./WTF upgrade testDirectory");

    //now we test the remove function and see if the file is removed from the server
    system("cd client2/ && ./WTF remove testDirectory subDirec1/test3.txt");
    system("cd client2/ && ./WTF commit testDirectory");
    system("cd client2/ && ./WTF push testDirectory");

    //do an update/upgrade, the test3 file should be deleted
    system("cd client1/ && ./WTF update testDirectory");
    system("cd client1/ && ./WTF upgrade testDirectory");
    
    //checkout the history of your project
    system("cd client1/ && ./WTF history testDirectory");
    system("cd client1/ && ./WTF currentversion testDirectory");

    //try rolling back your project to an earlier version
    system("cd client1/ && ./WTF rollback testDirectory 2");

    //retrieve this new version
    system("cd client1/ && ./WTF update testDirectory");
    system("cd client1/ && ./WTF upgrade testDirectory");

    //finally test the destroy function
    system("cd client1/ && ./WTF destroy testDirectory");

    //terminate the server process
    system("killall -SIGINT WTFserver");

    //clean up our directories
    //system("rm -rf client1");
    //system("rm -rf client2");
    //system("rm -rf server");
    //system("make clean");
    return 0;
}

void *servStart(void *ptr){
    system("cd server/ && ./WTFserver 20202");
    return NULL;
}