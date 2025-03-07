Name: Brian Mar
Student ID: 5332229209

Supported Features:
- User Input Prompt (Extra Credit)
- Authentication
- Lookup
- Push
- Remove
- Deploy
- Log (Extra credit)
- All functions have block code commenting how the functions work. Code is written in a maintainable way by being human readable so you can be able to understand what is going on.

Tested and Verified on Ubuntu 20.04

Prerequisites:
Ubuntu 20.04
CMake: sudo apt-get install make
pthread: sudo apt-get install libpthread-stubs0-dev

Files required:
filenames.txt (must include file in the directory)
members.txt (must include file in the directory)

/Project
|---Makefile 		# CMake configuration file
|---common.h 		# common includes header
|---client.h 		# client class header
|---client.cpp 		# client class implementation
|---server.h 		# server base class header
|---server.cpp 		# server base class implementation
|---serverA.h 		# server A derived class header
|---serverA.cpp 	# server A derived class implementation
|---serverD.h 		# server D derived class header
|---serverD.cpp 	# server D derived class implementation
|---serverM.h 		# server M derived class header
|---serverM.cpp 	# server M derived class implementation
|---serverR.h 		# server R derived class header
|---serverR.cpp 	# server R derived class implementation


How to run:
1)Compile all executables by running 'make all'
2)Run Server M by './serverM'
3)Run Server A by './serverA'
4)Run Server R by './serverR'
5)Run Server D by './serverD'
6)Run Client by './client <username> <password>'
                './client' will start the client by prompting for login's.
                './client guest guest' will start the client in guest mode.
You'll need to be authenticated as a member or guest to run commands. Once authenticated you'll be prompted to enter commands.
**'Make Clean' will erase all files from compile, executables and generated program files: deployed.txt and logs.txt

Design Documentation:
                                       Server A
Client <--TCP--> Server M  <---UDP---> Server R
                                       Server D     
Client -> Server M (TCP Communication messages to send user requests to Main Server) 
Authentication: "login [username] [password] [overwrite flag]"  #overwrite flag indicates if the use wants to overwrite (Yes/No = OC/NOC)
Lookup: "lookup [target username] [client username]"
Push: "push [client username] [filename]"
Remove: "remove [client username] [filename]"
Deploy: "deploy [client username]"
Log: "log [client username]"

Server M -> Server A (UDP communication forwarding request) : Authentication
Server M -> Server R (UDP communication forwarding request) : Lookup, Push, Remove, Deploy
Server M -> Server D (UDP communication forwarding request) : Deploy

Server A -> Server M (UDP Communication messages responses):
Authentication: "login [success flag]" # success flag indicates if authenticated (Yes/No = OK/NOK)

Server R -> Server M (UDP Communication messages responses):
Lookup: "lookup [target username] [result] [file #1] [...] [file #n]" #result is flag for if a user has a file in repository (Yes/No = UF/UNF)
Push: "push [client username] [filename] [result]" #result is flag for if successfully pushed (Yes/No = OK/NOK), if overwrite is required result flag is set to "CO"
Remove: "remove [client username] [filename] [result]" #result is flag for if successfully remove (Yes/No = OK/NOK)
Deploy: "deploy [client username] [file #1] [...] [files #n] # files are appended to the command for deploy

Server D -> Server M (UDP)
Deploy: "deploy [client username] [result]" #result is flag if the successfully deployed (Yes/NO = OK/NOK), if empty repository flag is set to NO_FILES_DEPLOYED

Server M -> Client (TCP communication forwarding responses)
Log: "log [client username]"
All other messages are forwarded from the server A/R/D to server M. Please reference the design messages from lines 51-61.

Idiosyncrasies:
- If you boot servers while zombie process exists, it will not fully be able to create the sockets. Solution: ps -aux | kill -9 [zombie process id] or wait about 1 minute and it should resolve.
- Deployed follows the design specifications of the assignment. It is a assumed that one deployment per user. The first deployment will create the deployed.txt file with line <username> <deployed filename>. The subsequent deployments will append to this file. If you duplicate a deploy for a user it will duplicated the lines from the previous deployment(s).

Works Cited:
Server and Client Socket Reference (https://geeksforgeeks.org/udp-server-client-implementation-c)
Network programming(https://beej.us/guide/bgnet/html/split)
Cmake(https://cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html)
Threading (https://geeksforgeeks.org/how-to-detach-a-thread-in-cpp)