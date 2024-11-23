#pragma once

#include "server.h"

const int SERVER_A_UDP_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_R_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
// const int BUFFER_SIZE = 1024;

const std::string MEMBER_FILE = "members.txt";

class ServerA : public Server
{
public:
	ServerA(int udpPortNumber);

	bool authenticate(const std::string& username, const std::string& password);
	sockaddr_in getServerRAddress();
	bool receiveTCPMessage(const int socket, std::string& message) override;
	bool sendTCPMessage(const int socket, const std::string& message) override;

private:
	std::string encryptPassword(const std::string&);
	void generateMembers(const std::string filename = MEMBER_FILE);
	bool pingServerM();

	std::map<std::string, std::string> members;
	sockaddr_in serverRAddress;
};