#pragma once

#include <vector>
#include <mutex>
#include "common.h"

const int TCP_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVERA_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVERR_PORT = 22000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVERD_PORT = 23000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
// const int BUFFER_SIZE = 2048;

class ServerM
{
	public:
		ServerM();
		~ServerM();

		//Handle TCP Communication
		void acceptTCPConnection();
		void handleTCPClient(int);
		void processReceivedMessageFromClient(int, const std::string&);
		bool getResponseFromServer(std::string&);
		std::string receiveTCPMessage(int clientSocket);
		bool sendTCPMessage(int, const std::string&);
		

		//Handle UDP Communication
		bool sendUDPMessage(const std::string&, const sockaddr_in&);
		bool receiveUDPMessage(std::string&);

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
