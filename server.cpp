/*
*  server.cpp
* 
*  This is the base class for a server. It constructs a server with either a UDP socket 
*  or a server with UDP socket and TCP socket. The server class implements a send and receive
*  function for the socket.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*
*  References used to develop UDP and TCP socket code below:
*  Server and Client Socket Reference (https://geeksforgeeks.org/udp-server-client-implementation-c)
*  Network programming(https://beej.us/guide/bgnet/html/split)
*  Threading (https://geeksforgeeks.org/how-to-detach-a-thread-in-cpp)
*/

#include "server.h"

/*
*  Constructor for server for support of TCP and UDP comunication
*
*  @param udpPortNumber The static port number for the UDP Socket
*  @param tcpPortNumber The static port number for the TCP Socket
*  @param name The string for the letter of the name of the server (e.g. ServerA = "A")
*/
Server::Server(const int udpPortNumber, const int tcpPortNumber, const std::string& name)
	: TCPServerSocket(-1)
	, UDPServerSocket(-1)
{
	if(DEBUG)
	{
		printf("Booting Server %s ...", name.c_str());
	}
	bool socketCreated = setupUDPSocket(udpPortNumber, name);
	while(!socketCreated)
	{
		if(DEBUG)
		{
				printf(".");
		}
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	socketCreated = setupTCPSocket(tcpPortNumber, name);
	while(!socketCreated)
	{
		if(DEBUG)
		{
			printf(".");
		}
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}

/*
*  Constructor for server for support of UDP comunication
*
*  @param udpPortNumber The static port number for the UDP Socket
*  @param name The string for the letter of the name of the server (e.g. ServerA = "A")
*/
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

/*
*  Destructor for server - closes server sockets
*/
Server::~Server()
{
	if(TCPServerSocket != -1) close(TCPServerSocket);
	if(UDPServerSocket != -1) close(UDPServerSocket);
}

/*
*  Function used to receiving a message from a client TCP socket
*  and updates the message param for the consumer to use.
*
*  @param socket The socket for the client to communicate with.
*  @param message The string to store the received message from the socket.
*  @return bool Returns true if a message was received or false if the message was empty
*/
bool Server::receiveTCPMessage(const int socket, std::string& message)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesReceived = read(socket, buffer, sizeof(buffer)-1);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		message = std::string(buffer);
		if(DEBUG)
		{
			printf("[DEBUG] Received TCP message (port %d): %s\n", getSocketPort(socket), buffer);
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

/*
*  Function used to receiving a message from a client UDP socket
*  and updates the message param for the consumer to use.
*
*  @param srcAddr The address struct which the address of the source will be updated to.
*  @param message The string to store the received message from the socket.
*  @return bool Returns true if a message was received or false if the message was empty.
*/
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
			printf("[DEBUG] Received UDP Message(port %d): %s.\n", getSocketPort(UDPServerSocket), buffer);
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

/*
*  Function used to sending a message to a client TCP socket.
*
*  @param clientSocket The socket for target receiver.
*  @param message The string of the message to send.
*  @return bool Returns true if a message was sent or false if the send was unsuccessful.
*/
bool Server::sendTCPMessage(const int socket, const std::string& message)
{
	if(socket == -1)
	{
		if(DEBUG)
		{
			printf("[ERR] No active TCP client connection.\n");
		}
		return false;
	}

	ssize_t bytesSent = send(socket, message.c_str(), message.size(), 0);
	if(bytesSent > 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Sent TCP message (port %d): %s\n", getSocketPort(socket), message.c_str());
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

/*
*  Function used to sending a message to a client UDP socket.
*
*  @param destAddr The address for the target receiver.
*  @param message The string of the message to send.
*  @return bool Returns true if a message was sent or false if the send was unsuccessful.
*/
bool Server::sendUDPMessage(const sockaddr_in& destAddr, const std::string& message)
{
	ssize_t bytesSent = sendto(UDPServerSocket, message.c_str(), message.size(), MSG_DONTWAIT, (sockaddr*)&destAddr, sizeof(destAddr));
	if(bytesSent > 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Sent UDP message (port %d): %s\n", getSocketPort(UDPServerSocket), message.c_str());
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


/*
*  Function used to create a UDP socket and assign it an address.
*
*  @param udpPortNumber The UDP port for the socket.
*  @param name The string for the letter of the name of the server (e.g. ServerA = "A").
*  @return bool Returns true if socket was created and bounded or false if failed.
*/
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

	printf("\nServer %s is up and running using UDP on port %d.\n", name.c_str(), getSocketPort(UDPServerSocket));
	return true;
}

/*
*  Function used to create a TCP socket and assign it an address.
*
*  @param tcpPortNumber The TCP port for the socket.
*  @param name The string for the letter of the name of the server (e.g. ServerA = "A").
*  @return bool Returns true if socket was created and bounded or false if failed.
*/
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

	printf("Server %s is up and running using TCP on port %d.\n", name.c_str(), getSocketPort(TCPServerSocket));
	return true;
}

/*
*  Function used to get the assigned port number of a socket.
*
*  @param udpPortNumber The socket.
*  @return int The port number.
*/
int Server::getSocketPort(int socket)
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