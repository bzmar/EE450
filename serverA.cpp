/*
*  serverA.cpp
* 
*  This is the derived class for a server A. It constructs a server with a UDP socket 
*  and functions to support the actions for handling authentication.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#include "serverA.h"

/*
*  Constructor for server A.
*
*  @param udpPortNumber The static port number for the UDP Socket
*/
ServerA::ServerA(int udpPortNumber)
	: Server(udpPortNumber, "A")
{
	generateMembers();

	serverMAddress.sin_family = AF_INET;
	serverMAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverMAddress.sin_port = htons(SERVER_M_UDP_PORT);
}

/*
*  Override for the virtual receiveTCPMessage(...) function as server A does not Support TCP
*/
bool ServerA::receiveTCPMessage(const int /*socket*/, std::string& /*message*/)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

/*
*  Override for the virtual sendTCPMessage(...) function as server A does not Support TCP
*/
bool ServerA::sendTCPMessage(const int /*socket*/, const std::string& /*message*/)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

/*
*  Function reads from a members.txt which contains lines space delimited
*  for a username password. Parses and stores into a std::map for faster authentication.
*/
void ServerA::generateMembers(const std::string filename)
{
	std::ifstream file(filename);
	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s.", MEMBER_FILE.c_str());
		}
	}
	std::string username, password;
	while(file >> username >> password) {
		members[username] = password;
	}
	file.close();
}


/*
*  Function to encrypt the password based on the specifications:
*  Offset each ASCII character (A-Z, a-z, 0-9) by 3
*  Cyclicaly update characters and numbers for overflow
*  Case Sensitive
*  Special characters are not modified
*
*  @param password the original password to be encrypted
*/
std::string ServerA::encryptPassword(const std::string& password)
{
	std::string encryptedPassword;
	for(char c: password)
	{
		if(c >= 'A' && c <= 'Z')
		{
			encryptedPassword += (c - 'A' + 3) % 26 + 'A';
		}
		else if(c >= 'a' && c <= 'z')
		{
			encryptedPassword += (c - 'a' + 3) % 26 + 'a';
		}
		else if(c >= '0' && c <= '9')
		{
			encryptedPassword += (c - '0' + 3) % 10 + '0';
		}
		else
		{
			encryptedPassword += c;
		}
	}
	return encryptedPassword;
}

/*
*  Function verify if the username and password is a valid member.
*
*  @param username the username to be verified
*  @param password the password to be verified
*  @return bool true if the username and password is in members.txt, false if not.
*/
bool ServerA::authenticate(const std::string& username, const std::string& password)
{
	bool authenticationResult = false;
	std::string encryptedPassword = encryptPassword(password);
	if(DEBUG)
	{
		printf("[DEBUG] Original Password:%s Encrypted Password: %s\n", password.c_str(), encryptedPassword.c_str());
	}
	auto searchResult = members.find(username);
	if(searchResult != members.end())
	{

		authenticationResult = (encryptedPassword.compare(searchResult->second) == 0);
	}

	return authenticationResult;
}

/*
*  Main function:
*  Instantiate server A, then loop as long as server is open and listen for commands
*/
int main(/*int argc, char const *argv[]*/)
{
	ServerA serverA(SERVER_A_UDP_PORT);

	while(1)
	{
		std::string message;
		sockaddr_in serverMAddress; 
		bool messageReceivedFromServerM = serverA.receiveUDPMessage(serverMAddress, message);

		if(messageReceivedFromServerM)
		{
			std::istringstream iss(message);
			std::string command, username, password;
			iss >> command >> username >> password;

			if(command.compare("login") == 0 && !username.empty() && !password.empty())
			{
				printf("ServerA received username %s and password %s \n", username.c_str(), std::string(password.length(), '*').c_str());

				bool authenticationResult = serverA.authenticate(username, password);

				if(authenticationResult)
				{
					printf("Member %s has been authenticated.\n", username.c_str());
				}
				else
				{
					printf("The username %s or password %s is incorrect\n", username.c_str(), std::string(password.length(), '*').c_str());
				}

				std::string response = authenticationResult ? "login OK" : "login NOK";
				serverA.sendUDPMessage(serverMAddress, response);
			}
		}
	}

	return 0;
}
