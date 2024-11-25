/*
*  client.h
* 
*  This is the header file for the declaration of the client class.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#pragma once

#include "common.h"
#include <algorithm>
#include <unordered_set>

using namespace std;

const int SERVER_PORT = 25000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const std::unordered_set<std::string> VALID_MEMBER_ACTIONS = {"lookup", "push", "remove", "deploy", "log"};
const std::unordered_set<std::string> VALID_GUEST_ACTIONS = {"lookup"};

class Client
{
public:
	// Client();
	Client(const std::string, const std::string);
	~Client();

	bool getUserCommand(std::string&);
	bool receiveMessage(std::string&);
	bool sendMessage(const std::string&);
	void handleUserCommand(const std::string&);
	bool handleServerResponse(const std::string&);

private:
	bool setupTCPSocket();
	void getLogin();
	bool getAuthentication();
	
	bool IsMember;
	std::string Username;
	std::string Password;
	int TCPSocket;
};