#include "serverM.h"

ServerM::ServerM()
	: TCPServerSocket(-1)
	, UDPServerSocket(-1)
	, TCPServerAddr()
	, UDPServerAddr()
	, TCPClientSockets()
{
	// std::thread TCPReceiveThread(&ServerM::receiveTCPMessage, this);
	// std::thread UDPReceiveThread(&ServerM::receiveUDPMessage, this);

	// TCPReceiveThread.join();
	// UDPReceiveThread.join();
};

ServerM::~ServerM()
{
	if(TCPServerSocket != -1) close(TCPServerSocket);
	if(UDPServerSocket != -1) close(UDPServerSocket);
	// if(TCPClientSocket1 != -1) close(TCPClientSocket1);
	// if(TCPClientSocket2 != -1) close(TCPClientSocket2);
}

bool ServerM::setupTCPServer()
{
	TCPServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPServerSocket < 0)
	{
		printf("Failed to create TCP Socket.\n");
		return false;
	}

	sockaddr_in TCPServerAddr;
	TCPServerAddr.sin_family = AF_INET;
    TCPServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    TCPServerAddr.sin_port = htons(TCP_PORT);

    int bindResult = bind(TCPServerSocket, (struct sockaddr*)&TCPServerAddr, sizeof(TCPServerAddr));
	if(bindResult < 0)
	{
		printf("Failed to bind TCP socket.\n");
		close(TCPServerSocket);
		return false;
	}

	int listenResult = listen(TCPServerSocket, 10);
	if(listenResult < 0)
	{
		printf("TCP Socket failed to listen.\n");
		close(TCPServerSocket);
		return false;
	}

	printf("TCP Server is listening on port %d.\n", TCP_PORT);

	return true;
}


bool ServerM::setupUDPServer()
{
	UDPServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPServerSocket < 0)
	{
		printf("Failed to create UDP Socket.\n");
		return false;
	}

	sockaddr_in UDPServerAddr;
	UDPServerAddr.sin_family = AF_INET;
    UDPServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    UDPServerAddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPServerSocket, (struct sockaddr*)&UDPServerAddr, sizeof(UDPServerAddr));
	if(bindResult < 0)
	{
		printf("Failed to bind TCP socket.\n");
		close(UDPServerSocket);
		return false;
	}

	printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

void ServerM::acceptTCPConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket;
	clientSocket = accept(TCPServerSocket, (sockaddr*)&clientAddr, (socklen_t*)&clientAddrLen);
	if(clientSocket < 0)
	{
		printf("Failed to accept TCP Client.\n");
	}
	else
	{
		printf("Connected to TCP Client.\n");
		std::lock_guard<std::mutex> lock(clientMutex);
		TCPClientSockets.push_back(clientSocket);

		std::thread clientThread(&ServerM::handleTCPClient, this, clientSocket);
		clientThread.detach();
	}
}

void ServerM::handleTCPClient(int clientSocket)
{
	receiveTCPMessage(clientSocket);

	close(clientSocket);
	clientSocket = -1;
}

bool ServerM::sendTCPMessage(int clientSocket, const std::string& message)
{
	if(clientSocket == -1)
	{
		printf("No active TCP client connection.\n");
		return false;
	}

	ssize_t bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		printf("Failed to send TCP message.\n");
		return false;
	}

	printf("Sent TCP message: %s\n", message.c_str());
	return true;
}

std::string ServerM::receiveTCPMessage(int clientSocket)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesReceived;
	while((bytesReceived = read(clientSocket, buffer, sizeof(buffer)-1)) > 0)
	{
		buffer[bytesReceived] = '\0';
		printf("Received Message from TCP Client: %s.\n", buffer);
	}

	return std::string(buffer);
}

bool ServerM::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
{
	ssize_t bytesSent = sendto(UDPServerSocket, message.c_str(), message.size(), MSG_CONFIRM, (sockaddr*)&clientAddr, sizeof(clientAddr));
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
	ssize_t bytesReceived = recvfrom(UDPServerSocket, buffer, sizeof(buffer)-1, 
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

int main(/*int argc, char const *argv[]*/)
{
	ServerM* m = new ServerM();
	
	if(!m->setupTCPServer())
	{
		printf("Failed to create TCP server.\n");
		// return;
	}
	if(!m->setupUDPServer())
	{
		printf("Failed to create UDP server.\n");
		// return;
	}

	while(1)
	{
		m->acceptTCPConnection();
	}


	return 0;
}