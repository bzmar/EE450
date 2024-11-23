#include "server.h"

Server::Server(const int udpPortNumber, const int tcpPortNumber, const std::string& name)
	: TCPServerSocket(-1)
	, UDPServerSocket(-1)
{
	if(DEBUG)
	{
		printf("Booting Server %s", name.c_str());
	}
	bool socketCreated = false;
	do
	{
		socketCreated = setupUDPSocket(udpPortNumber, name);
		if(!socketCreated)
		{
			if(DEBUG)
			{
				printf(".");
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	} while(!socketCreated);

	socketCreated = false;
	do
	{
		setupTCPSocket(tcpPortNumber, name);
		if(!socketCreated)
		{
			if(DEBUG)
			{
				printf(".");
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	} while(!socketCreated);
}

Server::Server(int udpPortNumber, const std::string& name)
	: TCPServerSocket(-1)
	, UDPServerSocket(-1)
{
	if(DEBUG)
	{
		printf("Booting Server %s", name.c_str());
	}
	bool socketCreated = false;
	do
	{
		socketCreated = setupUDPSocket(udpPortNumber, name);
		if(!socketCreated)
		{
			if(DEBUG)
			{
				printf(".");
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	} while(!socketCreated);
}

Server::~Server()
{
	if(TCPServerSocket != -1) close(TCPServerSocket);
	if(UDPServerSocket != -1) close(UDPServerSocket);
}

bool Server::receiveTCPMessage(const int clientSocket, std::string& message)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesReceived = read(clientSocket, buffer, sizeof(buffer)-1);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		message = std::string(buffer);
		if(DEBUG)
		{
			printf("[DEBUG] Received TCP message (port %d): %s\n", getSockPort(clientSocket), buffer);
		}
	}
	else
	{
		if(DEBUG)
		{
			// printf("[WARN] No data was received from TCP Socket.");
		}
		return false;
	}
	return true;
}
bool Server::receiveUDPMessage(sockaddr_in& srcAddr, std::string& message)
{
	char buffer[BUFFER_SIZE];
	socklen_t srcAddrLen = sizeof(srcAddr);
	ssize_t bytesReceived = recvfrom(UDPServerSocket, buffer, sizeof(buffer)-1, 
		MSG_DONTWAIT, (sockaddr*)&srcAddr, &srcAddrLen);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received UDP Message(port %d): %s.\n", getSockPort(UDPServerSocket), buffer);
		}
		message = std::string(buffer);
	}
	else
	{
		if(DEBUG)
		{
			// printf("[WARN] No data was received from UDP Socket.");
		}
		return false;
	}
	return true;
}


bool Server::sendTCPMessage(const int clientSocket, const std::string& message)
{
	if(clientSocket == -1)
	{
		if(DEBUG)
		{
			printf("[ERR] No active TCP client connection.\n");
		}
		return false;
	}

	ssize_t bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
	if(bytesSent > 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Sent TCP message (port %d): %s\n", getSockPort(clientSocket), message.c_str());
		}
	}
	else
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to send TCP message: %s\n", std::strerror(errno));
		}
		return false;
	}
	
	return true;

}
bool Server::sendUDPMessage(const sockaddr_in& destAddr, const std::string& message)
{
	ssize_t bytesSent = sendto(UDPServerSocket, message.c_str(), message.size(), MSG_DONTWAIT, (sockaddr*)&destAddr, sizeof(destAddr));
	if(bytesSent > 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Sent UDP message (port %d): %s\n", getSockPort(UDPServerSocket), message.c_str());
		}
	}
	else
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to send UDP message: %s\n", std::strerror(errno));
		}
		return false;
	}
	return true;
}

bool Server::setupUDPSocket(const int udpPortNumber, const std::string& name)
{
	UDPServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPServerSocket < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to create UDP Socket: %s\n", std::strerror(errno));
		}
		return false;
	}

	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerAddr.sin_port = htons(udpPortNumber);

    int bindResult = bind(UDPServerSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to bind TCP socket: %s\n", std::strerror(errno));
		}
		close(UDPServerSocket);
		UDPServerSocket = -1;
		return false;
	}

	printf("\nServer %s is up and running using UDP on port %d.\n", name.c_str(), getSockPort(UDPServerSocket));
	return true;
}

bool Server::setupTCPSocket(const int tcpPortNumber, const std::string& name)
{
	TCPServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPServerSocket < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to create TCP Socket: %s.\n", std::strerror(errno));
		}
		return false;
	}

	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerAddr.sin_port = htons(tcpPortNumber);

    int bindResult = bind(TCPServerSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] Failed to bind TCP socket: %s.\n", std::strerror(errno));
		}
		close(TCPServerSocket);
		TCPServerSocket = -1;
		return false;
	}

	int listenResult = listen(TCPServerSocket, 5);
	if(listenResult < 0)
	{
		if(DEBUG)
		{
			printf("[ERR] TCP Socket failed to listen: %s.\n", std::strerror(errno));
		}
		close(TCPServerSocket);
		TCPServerSocket = -1;
		return false;
	}

	printf("Server %s is up and running using TCP on port %d.\n", name.c_str(), getSockPort(TCPServerSocket));
	return true;
}

int Server::getSockPort(int socket)
{
	sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    
    if(getsockname(socket, (struct sockaddr*)&addr, &addrlen) == -1)
    {
    	if(DEBUG)
    	{
    		printf("[PANIC] Error in getsockname(...): %s\n", std::strerror(errno));
    	}
    	return -1;
    }

    return ntohs(addr.sin_port);
}