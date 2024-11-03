#pragma once

#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int TCP_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 1024;
const std::string LOCALHOST = "127.0.0.1";

class ServerM
{
	public:
		ServerM();
		~ServerM();

		//Handle TCP Communication
		bool sendTCPMessage(const std::string& message);
		void receiveTCPMessage();

		//Handle UDP Communication
		bool sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr);
		void receiveUDPMessage();

	private:
		//setup servers
		bool setupTCPServer();
		bool setupUDPServer();

		//socket descriptors
		int TCPSocket;
		int UDPSocket;
		int TCPClientSocket;
};