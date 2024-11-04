#pragma once


const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const int UDP_PORT = 21000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int SERVERM_UDP_PORT = 24000 + MY_ID_NUMBER_LAST_THREE_DIGITS;
const int BUFFER_SIZE = 1024;
const std::string LOCALHOST = "127.0.0.1";

class ServerA
{
public:
	ServerA();
	~ServerA();

	bool sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr);
	void receiveUDPMessage();

private:
	void generateMembers();
	bool setupUDPServer();

	int UDPSocket;
}