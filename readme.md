# Recreating GIT AsstLast
This is the last assignment in CS 214 for the spring semester of 2020. Git is a a popular current iteration of a series of version control systems. Most large, complex software projects are coded using version control. Version control can be very helpful when working on a single set of source code that multiple people are contributing to by making sure that everyone is working on the same version of the code. If people are working on code in physically separate locations, it is entirely possible that two different people have edited the same original code in two ways that are incompatible with each other. A versioning control system would not allow two different versions of the same file to exist in its central repository, enforcing that any changes made to a file are seen by everyone before they can submit additional changes to the repository.

## Thread Synchronization (Our Approach)

- Simply put, everytime a new client connected to the server, a thread was created to represent that client.  The thread would then go through the thread handler function to then begin the process where the client can send a request that needs to be accomplished.  Thread synchronization is an important aspect of this assignment as the repository needs to be locked in order to prevent conflicts that exist.  That being said, every command in our sever was locked by a mutex and was unlocked when completed so deadlocking would not occur to crash the code.  This implements thread safety and allows for synchronization to occur safely, otherwise the different threads would run havoc and essentially destroy the protection of our repository.

## Server Functions

### Requests From Client

#### Project (Checkout)
- Client asks for a project in the following manner **Proj:**
- Then Client will send the **length of the project name +1** (including the null terminator) 
- The Client will then send the **NameOfProject**
- Server should first write the **number of files** to expect including files in any and all subdirectories (DO NOT SEND THE BACKUPS HOWEVER)
- Then server should give all the files in the project by using the following protocol:
  - First send the **length of the path +1** (null terminator) of each file
  - Then send the **path of the file** 
  - Then just follow normal protocol for sending a file.

#### Create
- When you need to add a new project to the repo client will ask server to create a project in the following manner **Crea:**
- Then Client will send the **length of the project name +1** (including the null terminator)
- Then **NameOfProject** server needs to check that the project does not already exist
- Server needs to send back a 1 if the project did not exist before and has been successfully created, otherwise send a -1 if the project had already existed.
- Both client and server need to create the project and the manifest associated with the project.

#### File
- Client asks for a file in the following manner **File:** 
- Then **FilePath size +1** (including the null terminator)
- Then the Client will send **FilePath**
- Server should first send the **number of bytes +1** (including the null terminator) in the requested file
- Then send the file itself. If the file/project does not exist, send -1 instead of the fileSize.

#### Commit
- Client needs to make a commit in the following manner **Comm:**
- Then the name/path of the file will sent in the following manner, first the size: **FilePath size +1** (including the null terminator)
- Then the Client will send **FilePath-hashcode**, hashcode is appended to serve as an id to the commit file
- Server should merely store the given commit file in the appropriate project and wait until the commit has been pushed to make the changes.

#### Push (Split into multiple steps)
1. Client will ask to push the changes in the commit in the following manner **Push:**
  - Then the client will send the **Length of the Project Name +1** and then send the **Project Name**, this is to create the server backup.
  - Then the name/path of the .Commit file will sent in the following manner, first the size: **FilePath size +1** (including the null terminator)
  - Then the Client will send **.Commit FilePath-hashcode**, hashcode is appended to serve as an id to the commit file
2. The server needs to **append the contents of the commit** file to the .History file but make sure to add the **manifest version** number first and then add the contents.
3. Server should first check to see if the commit file exists and send a 1 if it does and -1 if it doesn't
4. Server should now fetch that selected .commit file and tokenize the file by new lines, then one by one it will recieve the files from client, make sure to DELETE files tagged D from the server and DO NOT recieve those, the client will not send those files.
  - The client will now send (M and A files) the **length of each file path + 1** then send the **file path** and then send the **# of bytes in the file** and send the **file** itself.
5. Lastly the client will send the updated manifest with the new manifest version number and all of the file versions incremented.
  - Client will send **Length of the Manifest Path+1** then the **Manifest File Path** then the **Number of Bytes in the Manifest File** and lastly **The Manifest File**
  - Server needs to OVERWRITE the .Manifest the is currently there with the new Manifest file from the client which will have incremented version numbers and deleted/added entries

#### Destroy
- When you need to destroy a project which is on the server, the client will ask the server to destroy in the following manner **Dest:** then **NameOfProject**, then you need to follow the directions on the description.

#### History
- When the client needs the history of a particular repository, it will follow the following protocol:
  - It will first send the following command **Hist:**
  - Then it will send the **length of the project name**
  - Then send the **Project Name**
  - The server should then retrieve the History file from the project and do the following:
    - If the project exists, send the **size of the History file**
    - Then send the **History File** to the client
    - If the project does not exist, then just send **-1** instead of the size

#### Rollback
- When the client wants to rollback a repository to the chosen project version, it will follow the following protocol:
  - It will first send the following command **Roll:**
  - Then the client will send **length of the project name**
  - Then send the **Project Name**
  - Then client will send the **Version** that it wants the server to rollback to
  - The server only needs to send a success *1* or failure *-1* to indicate to the client that the job is complete

## Client Functions

### Commit
* [x] Check if the given project exists on the client, checks to see if the .Configure has been set up properly
* [x] First requests the project Manifest from the server to see if the client project is up to date, if it's not then clien prints an error.
* [x] If there are any conflicts Client warns the user to resolve before continuing.
* [x] Load both manifests into linked lists and start comparing the entries. If there is a file on the server or if the file versions are out of sync, warn the user to update.
* [x] Then traverse the client manifest, if the tag is either 'A' or 'D' (tag is assigned during add or remove) write that entry to the commit file
* [x] Otherwise calculate the hash for the current client entry and mark it as modified in the commit file.
* [x] Once all entries in the client manifest have been recorded in the .Commit, generate a hashcode for the commit file, this will serve as its unique identifier which will let the push function on the server side know which commit to use.
* [x] Send the server the commit command, send the commit file and free everything else.

### Push
* [x] Check if the given project exists on the client, checks to see if the .Configure has been set up properly
* [x] Checks if the given project has a .Commit file, if it does not the user needs to commit the changes first.
* [x] Read the commit file and store it in a buffer. Then calculate the commit file's hashcode so that the server knows which commit file to push the changes of on the server.
* [x] Once the unique id is generated, send the push command to the server to tell it to get ready to push the commit file's changes.
* [x] After push, send the name of the project as well as the commit file name and contents to the server so that backups and changes can be applied.
* [x] Updates the manifest by tokenizing the commit file and replacing old versions and hash codes with the new ones form commit.
* [x] If the commit calls to delete a file, client erases the line from the manifest. Otherwise the client sends the new versions of the files to the server
* [x] Once the manifest has been updated send the manifest to the server to rewrite the one there
* [x] Free everything that has been mallocd and close the file descriptors

### Update
* [x] Checks to see if .Configure has been setup correctly and the client has stored the IP and Port number
* [x] Checks to see that the project name exists on the client and prints an error if it's not there
* [x] Sets up a client socket and using the ip and port, checks to see if the project exists on the server and prints an error if it's not there
* [x] Retrieves the .Manifest file from the server and stores the contents in a local buffer
* [x] Retrieves the .Manifest file from the client and stores the contents in a local buffer
* [x] Enters both contents into a Linked List and sorts both to be compared also gets both project versions
* [x] If both project versions are the same, prints up to date. Otherwise edits the .Update after traversing both linked lists and checking if any files need to be Added/Modified/Deleted from the client
* [x] Frees and closes the allocated linked lists and file descriptors

### Upgrade
* [x] Checks to see if .Configure has been setup correctly and the client has stored the IP and Port number
* [x] Sets up a client socket and using the ip and port, checks to see if the project exists on the server and prints an error if it's not there
* [x] Checks to see that both .Update exists and .Conflict does not exist on the client side and prints an error of either is true
* [x] Reads both the .Update and .Manifest files on the client side and stores both in buffers, it then populates a linked list for the client .Manifest
* [x] For every deletion in the .Update file, finds the corresponding entry in the Client and removes from the Linked List
* [x] For every addition and modification, retrieved said file from the server project and writes/overwrites the file contents to a corresponding file in the Client project
* [x] ] For additions where there are new subdirectories, checks to see if they exist and if not, creates all additional subdirectories before writing the contets to the new file
* [x] For additions and modifications, adds/edits the manifest linked list by including the file's hash and file version
* [x] Writes the finalized linked list back into the client's .Manifest file with the new manifest version number reflecting the server's manifest number
* [x] Frees the memory allocated for linked lists and closes file descriptors from the client's update, conflict and manifest
* [x] Get the version number from the server rather than pulling the whole server manifest
* [x] Set the tags for the manifest entries to 'U'

### Checkout
* [x] Checks to see if the project already exists locally, if the configure was not run yet, client is unable to connect to server, and if the project exists on the server
* [x] Asks the server for the project and clones every file in the project locally, including the manifest and others

### Add
* [x] Prepends the project name onto the given file name
* [x] Checks to see if the file exists, if it does, just rewrites the existing hash and increments the file version otherwise calculates a new hash and makes the file version 1
* [x] Writes changes to the .Manifest


	*This repository is the work of Advith Chegu and Savan Patel, please adhere to the Rutgers University Academic Integrity Policy for any future use, have fun!*