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
#include <set>

const bool DEBUG = true;
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int UDP_PORT = 22000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 1024;
const std::string LOCALHOST = "127.0.0.1";
const std::string FILENAMES_FILE = "filenames.txt";
const int BUFFER_CHAR_COUNT_SPLIT_NUMBER = 5;

class ServerR
{
public:
	ServerR();
	~ServerR();

	bool sendUDPMessage(const std::string&, const sockaddr_in&);
	void receiveUDPMessage();

private:
	void lookup(const std::string&, const sockaddr_in&);
	void deploy(const std::string&, const sockaddr_in&);
	void generateRepository();
	bool addToRepository(const std::string&, const std::string&);
	bool removeFromRepository(const std::string&, const std::string&);
	bool setupUDPServer();

	int UDPSocket;
	std::map<std::string, std::set<std::string>> membersRepository;
};