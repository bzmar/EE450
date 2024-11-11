#pragma once

#include <iostream>

using namespace std;

const std::string LOCALHOST = "127.0.0.1";
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int SERVER_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 2048;

class Client
{
public:
	bool receiveMessage(std::string&)
	bool sendMessage(std::string&)

private:
	bool setupTCPClient();
	void promptLogin(std::string&, std::string&)

	bool isAuthenticated;
	std::string username;
	std::string password;
	int TCPSocket;
};