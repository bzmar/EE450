/*
*  serverD.h
* 
*  This is the header file for the declaration of Server D, derived class.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#pragma once

#include "server.h"

const int SERVER_D_UDP_PORT = 23000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVER_M_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const std::string DEPLOYED_FILE = "deployed.txt";

class ServerD : public Server
{
public:
	ServerD(int udpPortNumber);
	
	void handleReceivedMessage(const std::string& message);
	bool receiveTCPMessage(const int socket, std::string& message) override;
	bool sendTCPMessage(const int socket, const std::string& message) override;

private:
	bool deploy(const std::string& username, const std::string& files);
	void respondToServer(const bool deploymentStatus, const std::string& username, const std::string& files);

	sockaddr_in serverMAddress;
};