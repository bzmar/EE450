#pragma once

#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <mutex>

const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int TCP_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 2048;
const std::string LOCALHOST = "127.0.0.1";

class ServerM
{
	public:
		ServerM();
		~ServerM();

		//Handle TCP Communication
		void acceptTCPConnection();
		void handleTCPClient(int);
		std::string receiveTCPMessage(int clientSocket);
		bool sendTCPMessage(int clientSocket, const std::string& message);
		

		//Handle UDP Communication
		bool sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr);
		void receiveUDPMessage();

		bool setupTCPServer();
		bool setupUDPServer();

	private:

		//socket descriptors
		int TCPServerSocket;
		int UDPServerSocket;
		sockaddr_in TCPServerAddr; // serverM TCP address
		sockaddr_in UDPServerAddr; // serverM UDP address
		std::vector<int> TCPClientSockets; //member client, guest client
		std::mutex clientMutex;
};