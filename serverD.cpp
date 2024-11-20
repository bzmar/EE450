#include "serverD.h"

ServerD::ServerD()
	: UDPSocket(-1)
{
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
		printf("Server D is up and running using UDP on port %d.\n", UDP_PORT);
	}
}

ServerD::~ServerD()
{
	if(UDPSocket != -1) close(UDPSocket);
}

bool ServerD::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
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

bool ServerD::deploy(const std::string& username, const std::string& files)
{
	std::ofstream file(DEPLOYED_FILE, std::ios::app);

	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", DEPLOYED_FILE.c_str());
		}
		return false;
	}

	std::istringstream iss(files);
	std::string filename;

	while(iss >> filename)
	{
		file << filename << '\n';
	}

	file.close();

	printf("Server D has deployed the user %s's repository.\n", username.c_str());
	return true;
}

void ServerD::receiveUDPMessage()
{
	char buffer[BUFFER_SIZE];
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	ssize_t bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer)-1, MSG_WAITALL, (sockaddr*)&clientAddr, &clientAddrLen);
	std::string response;
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received from TCP Client: %s.\n", buffer);
		}
		std::istringstream iss(buffer);
		std::string command, username;
		iss >> command >> username;

		if(command.compare("deploy") == 0)
		{
			std::string files;
			std::getline(iss >> std::ws, files);
			bool deploySuccess = deploy(username, files);
			
			if(deploySuccess)
			{
				response = std::string("deploy ") + username + std::string(" OK");
				std::istringstream iss(files);
				std::string filename;
				while(iss >> filename)
				{
					response += std::string(" ") + filename;
				}
			}
			else
			{
				response = std::string("deploy ") + username + std::string(" NOK");
			}
		}
		else
		{
			if(DEBUG)
			{
				printf("[DEBUG] Received Invalid Command.\n");
				response = std::string("deploy ") + username + std::string(" NOK");
			}
			//do nothing with invalid command.
		}
	}

	sendUDPMessage(response, clientAddr);
}

bool ServerD::setupUDPServer()
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

	sockaddr_in ServerDddr;
	ServerDddr.sin_family = AF_INET;
    ServerDddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerDddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPSocket, (struct sockaddr*)&ServerDddr, sizeof(ServerDddr));
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

	//printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerD server;

	while(1)
	{
		server.receiveUDPMessage();
	}

	return 0;
}
