#pragma once

#include "server.h"
#include <vector>
#include <mutex>

const int SERVER_A_UDP_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_R_UDP_PORT = 22000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_D_UDP_PORT = 23000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_M_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_M_TCP_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;


class ServerM : public Server
{
	public:
		ServerM(int udpPortNumber, int tcpPortNumber);

		//Handle TCP Communication
		// void acceptTCPConnection();
		// void handleTCPClient(int);
		// void processReceivedMessageFromClient(int, const std::string&);
		// bool getResponseFromServer(std::string&);
		// std::string receiveTCPMessage(int clientSocket);
		// bool sendTCPMessage(int, const std::string&);
		

		// //Handle UDP Communication
		// bool sendUDPMessage(const std::string&, const sockaddr_in&);
		// bool receiveUDPMessage(std::string&);

		// bool setupTCPServer();
		// bool setupUDPServer();

	private:

		sockaddr_in serverAAddress;
		sockaddr_in serverDAddress;
		sockaddr_in serverRAddress;
		std::vector<int> TCPClientSockets; //member client, guest client
		std::mutex clientMutex;
};
