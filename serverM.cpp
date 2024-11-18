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
	std::string clientMessage = receiveTCPMessage(clientSocket);


	close(clientSocket);
	clientSocket = -1;
}

bool ServerM::getResponseFromServer(std::string& response)
{
	bool responseFromServerReceived = false;
	std::cout << (responseFromServerReceived ? "true" : "false") << std::endl;
	do
	{
		std::cout << "this should print once" << std::endl;
		responseFromServerReceived = receiveUDPMessage(response);
	}
	while(!responseFromServerReceived);

	return responseFromServerReceived;
}

void ServerM::processReceivedMessageFromClient(int clientSocket, const std::string& message)
{
	std::istringstream iss(message.c_str());
	std::string action;
	iss >> action;
	sockaddr_in targetUDPAddr;
	targetUDPAddr.sin_family = AF_INET;
	targetUDPAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());

	if(action.compare("login") == 0)
	{
		targetUDPAddr.sin_port = htons(SERVERA_PORT);
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerA;
		bool responseServerAReceived = getResponseFromServer(responseServerA);
		sendTCPMessage(clientSocket, responseServerA);
	}
	else if(action.compare("lookup") == 0)
	{
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		sendTCPMessage(clientSocket, responseServerR);
	}
	else if(action.compare("push") == 0)
	{
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		sendTCPMessage(clientSocket, responseServerR);
	}
	else if(action.compare("remove") == 0)
	{
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		sendTCPMessage(clientSocket, responseServerR);
	}
	else if(action.compare("deploy") == 0)
	{
		
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);

		targetUDPAddr.sin_port = htons(SERVERD_PORT);
		sendUDPMessage(responseServerR, targetUDPAddr);
		std::string responseServerD;
		bool responseServerDReceived = getResponseFromServer(responseServerD);

		sendTCPMessage(clientSocket, responseServerD);
	}
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

		processReceivedMessageFromClient(clientSocket, std::string(buffer));
	}

	return std::string(buffer);
}

bool ServerM::sendUDPMessage(const std::string& message, const sockaddr_in& serveraddr)
{
	ssize_t bytesSent = sendto(UDPServerSocket, message.c_str(), message.size(), MSG_CONFIRM, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if(bytesSent < 0)
	{
		printf("Failed to send UDP message.\n");
		return false;
	}

	printf("Sent UDP message: %s\n", message.c_str());
	return true;
}

bool ServerM::receiveUDPMessage(std::string& response)
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
		printf("Received UDPMessage: %s.\n", buffer);

		response = std::string(buffer);
		// sendUDPMessage(response, clientAddr);
		// processReceivedMessageFromClient(clientAddr, std::string(buffer));
		return true;
	}
	else
	{
		std::cout << "received nothing" << std::endl;
	}
	return false;
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