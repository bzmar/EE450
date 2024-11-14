#pragma once

#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <algorithm>
#include <unordered_set>

using namespace std;

const std::string LOCALHOST = "127.0.0.1";
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int SERVER_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 2048;
const std::unordered_set<std::string> VALID_MEMBER_ACTIONS = {"lookup", "push", "remove", "deploy", "log"};
const std::unordered_set<std::string> VALID_GUEST_ACTIONS = {"lookup"};

class Client
{
public:
	// Client();
	Client(const std::string, const std::string);
	~Client();

	bool setupTCPClient();
	bool receiveMessage(std::string&);
	bool sendMessage(const std::string&);
	void getLogin();
	// void setUsername(const std::string);
	// void setPassword(const std::string);
	// void setAuthenticationStatus(bool);
	bool getAuthentication();
	// bool isClientAuthenticatedAsMember();
	bool getUserCommand(std::string&);
	void handleUserCommand(const std::string&);
	bool handleServerResponse(const std::string&);

private:
	bool isMember;
	std::string username;
	std::string password;
	int TCPSocket;
};