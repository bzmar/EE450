#include "serverA.h"

ServerA::ServerA()
	: UDPSocket(-1)
	, members()
{
	generateMembers();
	if(!setupUDPServer())
	{
		if(DEBUG)
		{
			printf("[DEBUG]Failed to create UDP server.\n");
		}
		return;
	}
	else
	{
		printf("Server A is up and running using UDP on port %d.\n", UDP_PORT);
	}
}

ServerA::~ServerA()
{
	if(UDPSocket != -1) close(UDPSocket);
}

void ServerA::generateMembers()
{
	std::ifstream file(MEMBER_FILE);

	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", MEMBER_FILE.c_str());
		}
	}

	std::string username, password;

	while(file >> username >> password) {
		members[username] = password;
	}

	file.close();
}

std::string ServerA::encryptPassword(const std::string& password)
{
	std::string encryptedPassword;

	for(char c: password)
	{
		if(c >= 'A' && c <= 'Z')
		{
			encryptedPassword += (c - 'A' + 3) % 26 + 'A';
		}
		else if(c >= 'a' && c <= 'z')
		{
			encryptedPassword += (c - 'a' + 3) % 26 + 'a';
		}
		else if(c >= '0' && c <= '9')
		{
			encryptedPassword += (c - '0' + 3) % 10 + '0';
		}
		else
		{
			encryptedPassword += c;
		}
	}

	return encryptedPassword;
}

bool ServerA::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
{
	ssize_t bytesSent = sendto(UDPSocket, message.c_str(), message.size(), MSG_CONFIRM, 
		(sockaddr*)&clientAddr, sizeof(clientAddr));
	if(bytesSent < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG]Failed to send UDP message.\n");
		}
		return false;
	}
	if(DEBUG)
	{
		printf("[DEBUG]Sent UDP message: %s\n", message.c_str());
	}
	return true;
}

void ServerA::receiveUDPMessage()
{
	char buffer[BUFFER_SIZE];
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	ssize_t bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer)-1, MSG_WAITALL, (sockaddr*)&clientAddr, &clientAddrLen);
	bool authenticationResult = false;
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received from TCP Client: %s.\n", buffer);
		}
		std::istringstream iss(buffer);
		std::string command, username, password;
		iss >> command >> username >> password;

		if(command.compare("./client") == 0 && !username.empty() && !password.empty())
		{
			printf("ServerA received username %s and password %s \n", username.c_str(), std::string(password.length(), '*').c_str());
			std::string encryptedPassword = encryptPassword(password);

			
			auto searchResult = members.find(username);
			if(searchResult != members.end())
			{
				authenticationResult = (encryptedPassword.compare(searchResult->second) == 0);
			}
		}
		else
		{
			if(DEBUG)
			{
				printf("[DEBUG] Received Invalid Command.\n");
			}
			//do nothing with invalid command.
		}
	}

	if(authenticationResult)
	{
		printf("Member %s has been authenticated.\n", username.c_str());
	}
	else
	{
		printf("The username %s or password %s is incorrect\n", username.c_str(), std::string(password.length(), '*').c_str());
	}
	std::string response = authenticationResult ? "./client OK" : "./client NOK";
	sendUDPMessage(response, clientAddr);
}

bool ServerA::setupUDPServer()
{
	UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocket < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to create UDP Socket.\n");
		}
		return false;
	}

	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerAddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPSocket, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to bind TCP socket.\n");
		}
		close(UDPSocket);
		UDPSocket = -1;
		return false;
	}

	printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerA server;

	while(1)
	{
		server.receiveUDPMessage();
	}

	return 0;
}
