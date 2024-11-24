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
	
		void acceptTCPConnectionAndProcessClientRequest();
		void processClientRequest(int clientSocket);

	private:
		void handleLoginRequest(int clientSocket, const std::string& message);
		void handleLookupRequest(int clientSocket, const std::string& message);
		void handlePushRequest(int clientSocket, const std::string& message);
		void handleRemoveRequest(int clientSocket, const std::string& message);
		void handleDeployRequest(int clientSocket, const std::string& message);
		void handleLogRequest(int clientSocket, const std::string& message);
		int getSocketPort(int socket);

		sockaddr_in serverAAddress;
		sockaddr_in serverDAddress;
		sockaddr_in serverRAddress;
		std::vector<int> TCPClientSockets; //member client, guest client
		std::mutex clientMutex;
};
