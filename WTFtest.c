#include <stdlib.h>
#include <pthread.h>

int main (int argc, char ** argv){

    //make a client 1 and client 2 directories to test two different clients
    system("mkdir client1");
    system("mkdir client2");
    system("mkdir server");
    system("make");
    system("cp WTF client1/");
    system("cp WTF client2/");
    system("cp WTFserver server/");

    //now that everything is setup we can get rid of the executables in the working directory
    system("make clean");

    //start the server
    pthread_t id;
    pthread_create(&id, NULL, servStart, "server_is_starting");

    //first configure to localhost and 20202
    system("cd client1");
    system("./WTF configure localhost 20202");
    system("cd ../client2");
    system("./WTF configure localhost 20202");

    //now we will work with client 1
    system("cd ../client1");
    
    //first we test the ability of both the server and client to create projects
    system("./WTF create testDirectory");
    //first create 2 files in the main directory
    system("touch testDirectory/test1.txt");
    system("touch testDirectory/test2.txt");
    system("echo Letter rentaliciousosos wooded direct two men indeed income sister. Impression up admiration he by partiality is. Instantly immediate his saw one day perceived. Old blushes respect but offices hearted minutes effects. Written parties winding oh as in without on started. Residence gentleman yet preserved few convinced. Coming regret simple longer little am sister on. Do danger in to adieus ladies houses oh eldest. Gone pure late gay ham. They sigh were not find are rentaliciousosos. >> testDirectory/test2.txt");
    system("echo Lose away off why half led have near bed. At engage simple father of period others except. My giving do summer of though narrow marked at. Spring formal no county ye waited. My whether cheered at regular it of promise blushes perhaps. Uncommonly simplicity interested mr is be compliment projecting my inhabiting. Gentleman he september in oh excellent. >> testDirectory/test1.txt");
    //next we test the ability of adding files in the directory/subdirectories to the manifest
    system("./WTF add testDirectory test1.txt");
    system("./WTF add testDirectory test2.txt");
    //test commit for the first time
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");
    //now test the ability of adding files in subdirectories
    system("mkdir -p testDirectory/subDirec1 && touch testDirectory/subDirec1/test3.txt");
    system("touch testDirectory/subDirec1/test4.txt");
    system("echo Questions explained agreeable preferred strangers too him her son. Set put shyness offices his females him distant. Improve has message besides shy himself cheered however how son. Quick judge other leave ask first chief her. Indeed or remark always silent seemed narrow be. Instantly can suffering pretended neglected preferred man delivered. Perhaps fertile brandon do imagine to cordial cottage. >> testDirectory/subDirec1/test3.txt");
    system("echo As am hastily invited settled at limited civilly fortune me. Really spring in extent an by. Judge but built gay party world. Of so am he remember although required. Bachelor unpacked be advanced at. Confined in declared marianne is vicinity. >> testDirectory/subDirec1/test4.txt");
    system("./WTF add testDirectory subDirec1/test3.txt");
    system("./WTF add testDirectory subDirec1/test4.txt");
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");

    //now go to the second client and checkout the project
    system("cd ../client2");
    system("./WTF checkout testDirectory");

    //test the ability to modify and commit said files to the server
    system("cd ../client1");
    system("echo Now seven world think timed while her. Spoil large oh he rooms on since an. Am up unwilling eagerness perceived incommode. Are not windows set luckily musical hundred can. Collecting if sympathize middletons be of of reasonably. Horrible so kindness at thoughts exercise no weddings subjects. The mrs gay removed towards journey chapter females offered not. Led distrusts otherwise who may newspaper but. Last he dull am none he mile hold as. > testDirectory/subDirec1/test4.txt");
    system("echo It allowance prevailed enjoyment in it. Calling observe for who pressed raising his. Can connection instrument astonished unaffected his motionless preference. Announcing say boy precaution unaffected difficulty alteration him. Above be would at so going heard. Engaged at village at am equally proceed. Settle nay length almost ham direct extent. Agreement for listening remainder get attention law acuteness day. Now whatever surprise resolved elegance indulged own way outlived. > testDirectory/test2.txt");
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");
    
    //test the ability to update/upgrade the project on another client
    system("cd ../client2");
    system("./WTF update testDirectory");
    system("./WTF upgrade testDirectory");

    //now we test the remove function and see if the file is removed from the server
    system("./WTF remove testDirectory subDirec1/test3.txt");
    system("./WTF commit testDirectory");
    system("./WTF push testDirectory");

    //do an update/upgrade, the test3 file should be deleted
    system("cd ../client1");
    system("./WTF update testDirectory");
    system("./WTF upgrade testDirectory");
    
    //checkout the history of your project
    system("./WTF history testDirectory");

    //try rolling back your project to an earlier version
    system("./WTF rollback testDirectory 1");

    //retrieve this new version
    system("./WTF update testDirectory");
    system("./WTF upgrade testDirectory");

    //finally test the destroy function
    system("./WTF destroy testDirectory");

    //terminate the server process
    system("killall -SIGINT WTFserver");
}

void *servStart(void *ptr){
    system("cd server/");
    system("./WTFserver 20202");
}