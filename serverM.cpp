#include "serverM.h"

ServerM::ServerM()
	: TCPSocket(-1)
	, UDPSocket(-1)
	, TCPClientSocket(-1)
{
	if(!setupTCPServer())
	{
		printf("Failed to create TCP server.\n");
		return;
	}
	if(!setupUDPServer())
	{
		printf("Failed to create UDP server.\n");
		return;
	}

	// std::thread TCPReceiveThread(&ServerM::receiveTCPMessage, this);
	// std::thread UDPReceiveThread(&ServerM::receiveUDPMessage, this);

	// TCPReceiveThread.join();
	// UDPReceiveThread.join();
};

ServerM::~ServerM()
{
	if(TCPSocket != -1) close(TCPSocket);
	if(UDPSocket != -1) close(UDPSocket);
	if(TCPClientSocket != -1) close(TCPClientSocket);
}

/*
sendTCPMessage: Function to send a 
*/
bool ServerM::sendTCPMessage(const std::string& message)
{
	if(TCPClientSocket == -1)
	{
		printf("No active TCP client connection.\n");
		return false;
	}

	ssize_t bytesSent = send(TCPClientSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		printf("Failed to send TCP message.\n");
		return false;
	}

	printf("Sent TCP message: %s\n", message.c_str());
	return true;
}

void ServerM::receiveTCPMessage()
{
	while(1)
	{
		TCPClientSocket = accept(TCPSocket, nullptr, nullptr);
		if(TCPClientSocket < 0)
		{
			printf("Failed to accept TCP Client.\n");
			continue;
		}

		char buffer[BUFFER_SIZE];
		ssize_t bytesReceived;
		while((bytesReceived = recv(TCPClientSocket, buffer, sizeof(buffer) -1, 0)) > 0)
		{
			buffer[bytesReceived] = '\0';
			printf("Received Message from TCP Client: %s.\n", buffer);
		}
		close(TCPClientSocket);
		TCPClientSocket = -1;
	}
}

bool ServerM::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
{
	ssize_t bytesSent = sendto(UDPSocket, message.c_str(), message.size(), MSG_CONFIRM, (sockaddr*)&clientAddr, sizeof(clientAddr));
	if(bytesSent < 0)
	{
		printf("Failed to send UDP message.\n");
		return false;
	}

	printf("Sent UDP message: %s\n", message.c_str());
	return true;
}

void ServerM::receiveUDPMessage()
{
	std::cout << "receiveUDPMessage" << std::endl;
	char buffer[BUFFER_SIZE];
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	ssize_t bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer)-1, 
		MSG_WAITALL, (sockaddr*)&clientAddr, &clientAddrLen);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		printf("Received Message from TCP Client: %s.\n", buffer);

		std::string response = "Message Received:" + std::string(buffer);
		sendUDPMessage(response, clientAddr);
	}
	else
	{
		std::cout << "received nothing" << std::endl;
	}
}

bool ServerM::setupTCPServer()
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
    serverAddr.sin_port = htons(TCP_PORT);

    int bindResult = bind(TCPSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(bindResult < 0)
	{
		printf("Failed to bind TCP socket.\n");
		close(TCPSocket);
		return false;
	}

	int listenResult = listen(TCPSocket, 10);
	if(listenResult < 0)
	{
		printf("TCP Socket failed to listen.\n");
		close(TCPSocket);
		return false;
	}

	printf("TCP Server is listening on port %d.\n", TCP_PORT);
	return true;
}


bool ServerM::setupUDPServer()
{
	UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocket < 0)
	{
		printf("Failed to create UDP Socket.\n");
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    serverAddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(bindResult < 0)
	{
		printf("Failed to bind TCP socket.\n");
		close(UDPSocket);
		return false;
	}

	printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerM server;
	server.receiveUDPMessage();

	return 0;
}