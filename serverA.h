#pragma once

#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int UDP_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int serverA_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 1024;
const std::string LOCALHOST = "127.0.0.1";

class serverA
{
public:
	ServerA();
	~ServerA();

	bool sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr);
	void receiveUDPMessage();

private:
	void generateMembers();
	bool setupUDPServer();

	int UDPSocket;
}