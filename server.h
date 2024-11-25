/*
*  server.h
* 
*  This is header file to declare the base class of server.
*  See server.cpp for the implementation.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#pragma once

#include "common.h"

class Server
{
public:
	Server(const int udpPortNumber, const int tcpPortNumber, const std::string& name);
	Server(int udpPortNumber, const std::string& name);

	virtual ~Server();

	virtual bool receiveTCPMessage(const int socket, std::string& message);
	virtual bool receiveUDPMessage(sockaddr_in& srcAddr, std::string& message);
	virtual bool sendTCPMessage(const int socket, const std::string& message);
	virtual bool sendUDPMessage(const sockaddr_in& destAddr, const std::string& message);

protected:
	bool setupUDPSocket(int udpPortNumber, const std::string& name);
	bool setupTCPSocket(int tcpPortNumber, const std::string& name);
	int getSocketPort(int socket);

	int TCPServerSocket;
	int UDPServerSocket;
};