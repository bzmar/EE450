/*
*  server.cpp
* 
*  This is the class for a client which uses TCP communication. It collects user input
*  requests, forwards them to the main server, waits for a response and outputs 
*  messages for the client.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#include "client.h"

/*
*  Constructor for client using TCP comunication
*
*  @param username The username if provided from program startup, otherwise it will be empty
*  @param password The password if provided from program startup, otherwise it will be empty
*/
Client::Client(const std::string username, const std::string password)
	: IsMember(false)
	, Username(username)
	, Password(password)
	, TCPSocket(-1)
{
	//setup TCP socket
	bool setupStatus = setupTCPSocket();
	while(!setupStatus)
	{
		printf("[ERR]Failed to create TCP server.\n");
	}
	
	// Authenticate for the login provided from terminal command ./Client username passsword
	if(!Username.empty() && !Password.empty())
	{
		bool result = false;
		result = getAuthentication();
		while(!result)
		{
			printf("The credentials are incorrect. Please try again.\n");
			getLogin(); // get new login
			result = getAuthentication(); // try to authenticate again
		}		
	}
	else
	{
		// no login was provided so collect login and authenticate
		getLogin();
		bool result = false;
		result = getAuthentication();
		while(!result)
		{
			printf("The credentials are incorrect. Please try again.\n");
			getLogin(); // get new login
			result = getAuthentication(); // authenticate
		}		
	}
};

/*
*  Destructor for client using TCP comunication
*  Close opened socket on exit if not already closed.
*/
Client::~Client()
{
	if(TCPSocket != -1) close(TCPSocket);
}


/*
*  Function used to create a TCP Socket. Not bound will dynamically get a port number
*/
bool Client::setupTCPSocket()
{
	TCPSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPSocket < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to create TCP Socket.\n");
		}
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    serverAddr.sin_port = htons(SERVER_PORT);

    if (connect(TCPSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    	if(DEBUG)
		{
        	printf("[ERR] Failed connection with the server.\n");
        }
        close(TCPSocket);
        TCPSocket = -1;
        return false;
    }
    
    sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int getsock_check = getsockname(TCPSocket, (struct sockaddr*)&clientAddr, &addrlen);
    if (getsock_check == -1) 
    {
    	if(DEBUG)
		{
        	printf("[ERR]Failed to getsockname().\n");
        }
        close(TCPSocket);
        TCPSocket = -1;
        return false;
    }

    if(DEBUG)
	{
    	printf("Connected from local IP: %s on port: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }
    printf("The client is up and running.\n");
    return true;
}

/*
*  Function used to receive a message from the TCP Socket and store in response string buffer
*
*  @param response buffer to store the message from the TCP socket
*  @return bool true if message was received, false if no message was received.
*/
bool Client::receiveMessage(std::string& response)
{
	if(TCPSocket == -1)
	{
		if(DEBUG)
		{
			printf("Socket is not connected.\n");
		}
		return false;
	}

	char buffer[BUFFER_SIZE];
	ssize_t bytesReceived = recv(TCPSocket, buffer, sizeof(buffer) - 1, 0);
	if(bytesReceived > 0)
	{
		sockaddr_in localAddr;
    	socklen_t addrlen = sizeof(localAddr);
    	int getsock_check = getsockname(TCPSocket, (struct sockaddr*)&localAddr, &addrlen);
    	if (getsock_check == -1) 
    	{
    		if(DEBUG)
			{
        		printf("Failed to getsockname().\n");
        	}
        	close(TCPSocket);
        	TCPSocket = -1;
        	return false;
    	}
    	printf("The client received the response from the main server using TCP over port %d.\n", ntohs(localAddr.sin_port));

		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received TCP Message: %s.\n", buffer);
		}
		response = std::string(buffer);
		return true;
	}
	if(DEBUG)
	{
		printf("[DEBUG] Received Empty TCP Message.\n");
	}
	return false;
}

/*
*  Function used to send a message to the Server over TCP Socket from string buffer message
*
*  @param message the string of the message to be sent to the TCP Server
*  @return bool true if message was sent successfully, false if no message was not sent successfully.
*/
bool Client::sendMessage(const std::string& message)
{
	if(TCPSocket == -1)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Socket is not connected.\n");
		}
		return false;
	}

	ssize_t bytesSent = send(TCPSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Error Sending Message.\n");
		}
		return false;
	}

	if(DEBUG)
	{
		printf("[DEBUG] Sent TCP Message: %s.\n", message.c_str());
	}
	return true;
		
}

/*
*  Function collect user input from the console. Will collect a username and a password and
*  verify that input is not empty.
*/
void Client::getLogin()
{
	while(Username.empty())
	{
		printf("Please enter username (usename cannot be empty): ");
		std::getline(std::cin, Username);
		if(DEBUG)
		{
			printf("[DEBUG] User Entered username: %s.\n", Username.c_str());
		}
	}
	while(Password.empty())
	{
		printf("Please enter password (password cannot be empty): ");
		std::getline(std::cin, Password);
		if(DEBUG)
		{
			printf("[DEBUG] User Entered username: %s.\n", Password.c_str());
		}
	}
}

/*
*  Function for authenticate login. If both the username and password are guest,
*  then automatically grant guest access. Otherwise send the request to the main server
*  for login authentication and wait for the response from the main server. (Main server will
*  send to server A to verify)
*/
bool Client::getAuthentication()
{
	if((Username.compare("guest") == 0) && (Password.compare("guest") == 0))
	{
		printf("You have been granted guest access.\n");
		IsMember = false;
		return true;
	}
	else
	{
		const std::string command = std::string("login ") + Username + std::string(" ") + Password;
		sendMessage(command);

		std::string response;
		bool receivedResponse = receiveMessage(response);
		while (!receivedResponse)
		{
			receivedResponse = receiveMessage(response);
		}
	
		std::istringstream iss(response);
		std::string key, result;
		iss >> key >> result;
		if(key.compare("login") != 0)
		{
			printf("The response does not contain the command sent");
		}
		if(key.compare("login") == 0 && result.compare(std::string("OK")) == 0)
		{
			IsMember = true;
			printf("You have been granted member access.\n");
			return true;
		}
	}
	Username.clear();
	Password.clear();
	return false;
}

/*
*  Function for collecting user request in the screen based on member or non member. 
*  The user request will be created in the form of a command to send to the server and 
*  stored in the command buffer.
*  
*  @param command The string buffer to hold the command for sending to server.
*  @return bool returns if the command was a valid command for the guest/member privilege.
*/
bool Client::getUserCommand(std::string& command)
{
	if(IsMember)
	{
		printf("Please enter the command:\n<lookup <username>>\n<push <filename>>\n<remove <filename>>\n<deploy>\n<log>\n");
	}
	else //guest
	{
		printf("Please enter the command:\n<lookup <username>>\n");
	}
	std::getline(std::cin, command);
	command.erase(std::remove(command.begin(), command.end(), '<'), command.end());
	command.erase(std::remove(command.begin(), command.end(), '>'), command.end());
	std::istringstream iss(command);
	std::string action, parameter;
	iss >> action >> parameter;
	std::transform( action.begin(), action.end(), action.begin(),
				[] (unsigned char c){ return std::tolower(c); } );

	bool validCommand = false;
	if(IsMember)
	{
		validCommand = (VALID_MEMBER_ACTIONS.find(action) != VALID_MEMBER_ACTIONS.end());
		if(validCommand)
		{
			if(action.compare("lookup") == 0)
			{
				if(parameter.empty())
				{
					command = action + std::string(" ") + Username + std::string(" ") + Username;
					printf("Username is not specified. Will lookup %s\n", Username.c_str());
				}
				else
				{
					command = action + std::string(" ") + parameter + std::string(" ") + Username;
				}
				validCommand = true;
			}
			else if(action.compare("push") == 0)
			{
				if(parameter.empty())
				{
					validCommand = false;
					printf("Error: Filename is required. Please specify a filename to push.\n");
				}
				else
				{
					command = action + std::string(" ") + Username + std::string(" ") + parameter;
					validCommand = true;
				}
			}
			else if(action.compare("remove") == 0)
			{
				if(parameter.empty())
				{
					validCommand = false;
					printf("Error: Filename is required. Please specify a filename to remove.\n");
				}
				else
				{
					command = action + std::string(" ") + Username + std::string(" ") + parameter;
					validCommand = true;
				}
			}
			else if(action.compare("deploy") == 0)
			{
				command = action + std::string(" ") + Username;
				validCommand = true;
			}
		}
	}
	else
	{
		validCommand = (VALID_GUEST_ACTIONS.find(action) != VALID_GUEST_ACTIONS.end());
		if(validCommand)
		{
			if(parameter.empty())
			{
				validCommand = false;
				printf("Error: Username is required. Please specify a username to lookup.");
			}
			else
			{
				command = action + std::string(" ") + parameter + std::string(" guest");
				validCommand = true;
			}
		}
	}

	return validCommand;
}

/*
*  Function for collecting to send the command to the main server and output console
*  information for the client.
*  
*  @param command The string of command for sending to server.
*/
void Client::handleUserCommand(const std::string& command)
{
	std::istringstream iss(command);
	std::string action, username, parameter;
	iss >> action >> username >> parameter;

	if(action.compare("lookup") == 0)
	{
		printf("%s sent a lookup request to the main server\n", Username.c_str());
	}
	else if(action.compare("push") == 0)
	{
		if(DEBUG)
		{
			printf("%s sent a push request to the main server for file: %s\n", Username.c_str(), parameter.c_str());
		}
		else
		{
			printf("%s sent a push request to the main server.\n", Username.c_str());
		}
	}
	else if(action.compare("remove") == 0)
	{
		if(DEBUG)
		{
			printf("%s sent a remove request to the main server for file %s\n", Username.c_str(), parameter.c_str());
		}
		else
		{
			printf("%s sent a remove request to the main server.\n", Username.c_str());
		}
	}
	else if(action.compare("deploy") == 0)
	{
		printf("%s sent a deploy request to the main server\n", Username.c_str());
	}
	else if(action.compare("log") == 0)
	{
		printf("%s sent a log request to the main server\n", Username.c_str());
	}
	sendMessage(command);
}

/*
*  Function for receiving the response from the main server, parsing and processing
*  the response.
*
*  Legend: UNF - User not Found
*          UF  - User found
*          CO  - confirm overwrite (ask user)
*  
*  @param response The string of response from the server.
*/
bool Client::handleServerResponse(const std::string& response)
{
	std::istringstream iss(response);
	std::string action;
	iss >> action;

	if(action.compare("lookup") == 0)
	{
		std::string result, un;
		iss >> un >> result;
		if(result.compare("UNF") == 0)
		{
			printf("%s does not exist. Please try again.\n", un.c_str());
		}
		else if(result.compare("UF") == 0)
		{
			std::string file;
			bool isEmpty = true;
			while (iss >> file)
			{
				if(isEmpty)
				{
					printf("%s's repository:\n", un.c_str());
				}
				isEmpty = false;
				printf("%s\n", file.c_str());
			}
			if(isEmpty)
			{
				printf("Empty Repository.\n");
			}
		}
	}
	else if(action.compare("push") == 0)
	{
		std::string username, filename, result;
		iss >> username >> filename >> result;
		if(result.compare("CO") == 0)
		{
			std::string userResponse, message;
			printf("%s exists in %s's repository, do you want to overwrite (Y/N)?\n", filename.c_str(), Username.c_str());
			std::getline(std::cin, userResponse);
			std::transform( userResponse.begin(), userResponse.end(), userResponse.begin(),
				[] (unsigned char c){ return std::tolower(c); } );
			if( (userResponse.compare("y") == 0) || (userResponse.compare("yes") == 0) )
			{
				message = std::string("push ") + username + std::string(" ") + filename + std::string(" OC");
			}
			else if( (userResponse.compare("n") == 0) || (userResponse.compare("no") == 0) )
			{
				message = std::string("push ") + username + std::string(" ") + filename + std::string(" NOC");
			}
			sendMessage(message);
			return false;
		}
		else if(result.compare("OK") == 0)
		{
			printf("%s pushed successfully.\n", filename.c_str());
		}
		else
		{
			printf("%s was not pushed successfully.\n", filename.c_str());
		}
		
	}
	else if(action.compare("remove") == 0)
	{
		std::string username, filename, result;
		iss >> username >> filename >> result;
		if(result.compare("OK") == 0)
		{
			printf("The remove request was successful.\n");
		}
		else
		{
			printf("The remove request was unsuccessful.\n");
		}
	}
	else if(action.compare("deploy") == 0)
	{
		std::string result, username;
		iss >> username >> result;
		if(result.compare("OK") == 0)
		{
			printf("The following files in %s repository have been deployed:\n", Username.c_str());
			std::string file;
			while (iss >> file)
			{
				if(file.compare("NO_FILES_DEPLOYED") == 0)
				{
					printf("\n");
					break;
				}
				printf("%s\n", file.c_str());
			}
		}
		else
		{
			printf("Deployment request unsuccessful.\n");
		}
	}
	// else if(action.compare("log") == 0)
	// {
		
	// }
	return true;
}

/*
*  Main function runs the initialization of the client. If arguments are provided for username and password
*  then assign them else they will be empty. Runs the loop to get a user request, listens to server responses,
*  call to handle the response.
*  
*  @param argc the count of arguments
*  @param argv the arguments vector
*/
int main(int argc, char const *argv[])
{
	std::string username;
	std::string password;
	if(argc == 3)
	{
		username = argv[1];
		password = argv[2];
	}
	Client client(username, password); 

	while(1)
	{
		std::string command;
		bool validCommand = client.getUserCommand(command);
		while(!validCommand)
		{
			command.clear();
			printf("The command was entered incorrectly. Please try again.\n");
			validCommand = client.getUserCommand(command);
		}
		client.handleUserCommand(command);

		std::string response;
		bool messageReceived = false;
		do
		{
			messageReceived = client.receiveMessage(response);
		} while (!messageReceived);

		bool completedTransaction = client.handleServerResponse(response);
		while(!completedTransaction)
		{
			messageReceived = false;
			do
			{
				messageReceived = client.receiveMessage(response);
			} while (!messageReceived);
			completedTransaction = client.handleServerResponse(response);
		}
		printf("----Start a new request----\n");
	}

	return 0;
}