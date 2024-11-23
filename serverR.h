#pragma once

#include "server.h"

const int SERVER_R_UDP_PORT = 22000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_M_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const std::string FILENAMES_FILE = "filenames.txt";

class ServerR : public Server
{
public:
	ServerR(int udpPortNumber);

	bool receiveTCPMessage(const int socket, std::string& message) override;
	bool sendTCPMessage(const int socket, const std::string& message) override;
	void parseAndExecuteCommand(const std::string& message);

private:
	void lookup(const std::string& username);
	void push(const std::string& username, const std::string& filename, const std::string& overwrite = "");
	void remove(const std::string& username, const std::string& filename);
	void deploy(const std::string& username);
	void generateRepository();
	bool addToRepository(const std::string&, const std::string&);
	bool removeFromRepository(const std::string&, const std::string&);

	std::map<std::string, std::set<std::string>> membersRepository;
	sockaddr_in serverMAddress;
};