#include "client.h"

Client::Client(const std::string un, const std::string pw)
	: isMember()
	, username(un)
	, password(pw)
	, TCPSocket(-1)
{
	if( ((username.compare("guest") != 0) && (password.compare("guest") != 0)) ||
		(username.empty() && password.empty()) )
	{
		getLogin();
	}
	std::cout << username << " " << password << std::endl;
};

Client::~Client()
{
	if(TCPSocket != -1) close(TCPSocket);
}

bool Client::setupTCPClient()
{
	TCPSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPSocket < 0)
	{
		printf("Failed to create TCP Socket.\n");
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    serverAddr.sin_port = htons(SERVER_PORT);

    if (connect(TCPSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Failed connection with the server.\n");
        close(TCPSocket);
        TCPSocket = -1;
        return false;
    }
    
    sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int getsock_check = getsockname(TCPSocket, (struct sockaddr*)&clientAddr, &addrlen);
    if (getsock_check == -1) {
        printf("Failed to getsockname().\n");
        close(TCPSocket);
        TCPSocket = -1;
        return false;
    }

    printf("Connected from local IP: %s on port: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    return true;
}

bool Client::receiveMessage(std::string& response)
{
	if(TCPSocket == -1)
	{
		printf("Socket is not connected.\n");
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
        	printf("Failed to getsockname().\n");
        	close(TCPSocket);
        	TCPSocket = -1;
        	return false;
    	}
    	printf("The client received the response from the main server using TCP over port %d.", ntohs(localAddr.sin_port));

		buffer[bytesReceived] = '\0';
		printf("Received Message from TCP Client: %s.\n", buffer);
		response = std::string(buffer);
		return true;
	}
	
	std::cout << "Received nothing" << std::endl;
	return false;
}

bool Client::sendMessage(const std::string& message)
{
	if(TCPSocket == -1)
	{
		printf("Socket is not connected.\n");
		return false;
	}

	ssize_t bytesSent = send(TCPSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		printf("Error Sending Message.\n");
		return false;
	}

	printf("Sent Message: %s.\n", message.c_str());
	return true;
		
}

void Client::getLogin()
{
	while(username.empty())
	{
		printf("Please enter username (usename cannot be empty): ");
		std::getline(std::cin, username);
		std::cout << "[DEBUG] User Entered username: " << username << std::endl;
	}
	while(password.empty())
	{
		printf("Please enter password (password cannot be empty): ");
		std::getline(std::cin, password);
		std::cout << "[DEBUG] User Entered password: " << password << std::endl;
	}
}


bool Client::getAuthentication()
{
	if((username.compare("guest") == 0) && (password.compare("guest") == 0))
	{
		isMember = false;
		return true;
	}
	else
	{
		const std::string command = std::string("login ") + username + std::string(" ") + password;
		sendMessage(command);

		std::string response;
		bool receivedResponse = false;
		do
		{
			receivedResponse = receiveMessage(response);
		}
		while (!receivedResponse);
	
		std::istringstream iss(response);
		std::string key, result;
		iss >> key >> result;
		if(key.compare("login") != 0)
		{
			printf("The response does not contain the command sent");
		}
		if(key.compare("login") == 0 && result.compare(std::string("OK")) == 0)
		{
			isMember = true;
			printf("You have been granted member access.\n");
			return true;
		}
	}
	username.clear();
	password.clear();
	return false;
}

bool Client::getUserCommand(std::string& command)
{
	if(isMember)
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
	if(isMember)
	{
		validCommand = (VALID_MEMBER_ACTIONS.find(action) != VALID_MEMBER_ACTIONS.end());
		if(validCommand)
		{
			if(action.compare("lookup") == 0)
			{
				if(parameter.empty())
				{
					command = action + std::string(" ") + username + std::string(" ") + username;
					printf("Username is not specified. Will lookup %s\n", username.c_str());
				}
				else
				{
					command = action + std::string(" ") + parameter + std::string(" ") + username;
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
					command = action + std::string(" ") + username + std::string(" ") + parameter;
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
					command = action + std::string(" ") + username + std::string(" ") + parameter;
					validCommand = true;
				}
			}
			else if(action.compare("deploy") == 0)
			{
				command = action + std::string(" ") + username;
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

void Client::handleUserCommand(const std::string& command)
{
	std::istringstream iss(command);
	std::string action, parameter;
	iss >> action >> parameter;

	if(action.compare("lookup") == 0)
	{
		printf("%s sent a lookup request for %s to the main server\n", username.c_str(), parameter.c_str());
	}
	else if(action.compare("push") == 0)
	{
		printf("%s sent a push request to the main server for file: %s\n", username.c_str(), parameter.c_str());
	}
	else if(action.compare("remove") == 0)
	{
		printf("%s sent a remove request to the main server for file %s\n", username.c_str(), parameter.c_str());
	}
	else if(action.compare("deploy") == 0)
	{
		printf("%s sent a deploy request to the main server\n", username.c_str());
	}
	else if(action.compare("log") == 0)
	{
		printf("%s sent a log request to the main server\n", username.c_str());
	}
	sendMessage(command);
}

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
		printf("----Start a new request----\n");
	}
	else if(action.compare("push") == 0)
	{
		std::string username, filename, result;
		iss >> username >> filename >> result;
		if(result.compare("CO") == 0)
		{
			std::string userResponse, message;
			printf("%s exists in %s's repository, do you want to overwrite (Y/N)?", filename.c_str(), username.c_str());
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
			printf("%s pushed successfully.", filename.c_str());
		}
		else
		{
			printf("%s was not pushed successfully.", filename.c_str());
		}
		
	}
	else if(action.compare("remove") == 0)
	{
		std::string username, filename, result;
		iss >> username >> filename >> result;
		if(result.compare("OK") == 0)
		{
			printf("The remove request was successful.");
		}
		else
		{
			printf("The remove request was unsuccessful.");
		}
	}
	else if(action.compare("deploy") == 0)
	{
		std::string result, username;
		iss >> username >> result;
		if(result.compare("OK") == 0)
		{
			printf("The following files in %s repository have been deployed:\n", username.c_str());
			std::string file;
			while (iss >> file)
			{
				printf("%s\n", file.c_str());
			}
		}
		else
		{
			printf("Deployment request unsuccessful.");
		}
	}
	// else if(action.compare("log") == 0)
	// {
		
	// }
	return true;
}

int main(int argc, char const *argv[])
{
	std::string username;
	std::string password;
	if(argc == 3)
	{
		username = argv[1];
		password = argv[2];
	}

	Client* c = new Client(username, password);
	
	while(!c->setupTCPClient())
	{
		printf("Failed to create TCP server.\n");
	}

	printf("The client is up and running.\n");

	bool result = false;
	do
	{
		result = c->getAuthentication();
		if(!result)
		{
			printf("The credentials are incorrect. Please try again.\n");
			c->getLogin();
		}
		
	} while(!result);

	while(1)
	{
		std::string command;
		bool validCommand = c->getUserCommand(command);
		while(!validCommand)
		{
			command.clear();
			printf("The command was entered incorrectly. Please try again.\n");
			validCommand = c->getUserCommand(command);
		}

		c->handleUserCommand(command);

		std::string response;
		bool messageReceived = false;
		do
		{
			messageReceived = c->receiveMessage(response);
		} while (!messageReceived);

		bool completedTransaction = c->handleServerResponse(response);
		while(!completedTransaction)
		{
			messageReceived = false;
			do
			{
				messageReceived = c->receiveMessage(response);
			} while (!messageReceived);
			completedTransaction = c->handleServerResponse(response);
		}
	}

	return 0;
}