#pragma once

#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <iostream>

const bool DEBUG = true;
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int UDP_PORT = 23000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 2048;
const std::string LOCALHOST = "127.0.0.1";
const std::string DEPLOYED_FILE = "deployed.txt";

class ServerD
{
public:
	ServerD();
	~ServerD();

	bool sendUDPMessage(const std::string&, const sockaddr_in&);
	void receiveUDPMessage();

private:
	bool deploy(const std::string&, const std::string&);
	bool setupUDPServer();

	int UDPSocket;
};