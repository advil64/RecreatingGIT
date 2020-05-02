# Recreating GIT AsstLast
This is the last assignment in CS 214 for the spring semester of 2020. Git is a a popular current iteration of a series of version control systems. Most large, complex software projects are coded using version control. Version control can be very helpful when working on a single set of source code that multiple people are contributing to by making sure that everyone is working on the same version of the code. If people are working on code in physically separate locations, it is entirely possible that two different people have edited the same original code in two ways that are incompatible with each other. A versioning control system would not allow two different versions of the same file to exist in its central repository, enforcing that any changes made to a file are seen by everyone before they can submit additional changes to the repository.

## Testing Procedure

1. First test the create 

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

*Check:* This command is merely to see if a given project exists on the server. The client will will ask the following **Chec:** then **NameOfProject** and the server should return a 1 is the project exists and a -1 if the project does not exist.

~~*Manifest project number:* Client asks for a file in the following manner **manV:**, server should retrieve the manifest version from the most recent manifest and send the number. (This might be deleted, so ignore it for now)~~

## Client Functions

### Push


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