#include "serverA.h"

ServerA::ServerA()
	: UDPSocket(-1)
{
	if(!setupUDPServer())
	{
		printf("Failed to create UDP server.\n");
		return;
	}
	std::thread UDPReceiveThread(&ServerA::receiveUDPMessage, this);
	UDPReceiveThread.join();
}

ServerA::~ServerA()
{
	if(UDPSocket != -1) close (UDPSocket);
}

// void ServerA::generateMembers()
// {
// 	ifstream file(MEMBER_FILE);

// 	if(!file.is_open())
// 	{
// 		printf("[PANIC] Error in opening file %s", MEMBER_FILE.c_str());
// 	}

// 	string username;
// 	string password;

// 	while(file >> username >> password) {
// 		members[username] = password;
// 	}

// 	file.close();
// }

bool ServerA::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
{
	ssize_t bytesSent = sendto(UDPSocket, message.c_str(), message.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	if(bytesSent < 0)
	{
		printf("Failed to send UDP message.\n");
		return false;
	}

	printf("Sent UDP message: %s\n", message.c_str());
	return true;
}

void ServerA::receiveUDPMessage()
{
	while(1)
	{
		char buffer[BUFFER_SIZE];
		sockaddr_in clientAddr;
		clientAddr.sin_family = AF_INET;
    	clientAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    	clientAddr.sin_port = htons(ServerA_UDP_PORT);
		socklen_t clientAddrLen = sizeof(clientAddr);
		ssize_t bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer)-1, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if(bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0';
			printf("Received Message from TCP Client: %s.\n", buffer);

			std::string response = "Message Received:" + std::string(buffer);
			sendUDPMessage(response, clientAddr);
		}
	}
}

bool ServerA::setupUDPServer()
{
	UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocket < 0)
	{
		printf("Failed to create UDP Socket.\n");
		return false;
	}

	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerAddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPSocket, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if(bindResult < 0)
	{
		printf("Failed to bind TCP socket.\n");
		close(UDPSocket);
		UDPSocket = -1;
		return false;
	}

	printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	return 0;
}