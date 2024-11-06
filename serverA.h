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
#include <map>
#include <iostream>

const bool DEBUG = true;
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int UDP_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVERM_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 1024;
const std::string LOCALHOST = "127.0.0.1";
const std::string MEMBER_FILE = "members.txt";

class ServerA
{
public:
	ServerA();
	~ServerA();

	bool sendUDPMessage(const std::string&, const sockaddr_in&);
	void receiveUDPMessage();

private:
	std::string encryptPassword(const std::string&);
	void generateMembers();
	bool setupUDPServer();

	int UDPSocket;
	std::map<std::string, std::string> members;
};