#include "serverD.h"

ServerD::ServerD(int udpPortNumber)
	: Server(udpPortNumber, "D")
{
	serverMAddress.sin_family = AF_INET;
	serverMAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverMAddress.sin_port = htons(SERVER_M_UDP_PORT);
}

bool ServerD::receiveTCPMessage(const int socket, std::string& message)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

bool ServerD::sendTCPMessage(const int socket, const std::string& message)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

void ServerD::handleReceivedMessage(const std::string& message)
{
	std::istringstream iss(message);
	std::string command, username;
 	iss >> command >> username;
	if(command.compare("deploy") == 0)
 	{
 		std::string files;
		std::getline(iss >> std::ws, files);
		bool deploySuccess = deploy(username, files);

		respondToServer(deploySuccess, username, files);
	}
	else
	{
		if(DEBUG)
		{
			printf("[DEBUG] Received Invalid Command.\n");
		}
		respondToServer(false, "INVALID", "");
 	}
}

bool ServerD::deploy(const std::string& username, const std::string& files)
{
	printf("Server D has received a deploy request from the main server.\n");
	std::ofstream file(DEPLOYED_FILE, std::ios::app);

	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", DEPLOYED_FILE.c_str());
		}
		return false;
	}

	std::istringstream iss(files);
	std::string filename;

	while(iss >> filename)
	{
		file << filename << '\n';
	}

	file.close();

	printf("Server D has deployed the user %s's repository.\n", username.c_str());
	return true;
}

void ServerD::respondToServer(const bool deploymentStatus, const std::string& username, const std::string& files)
{
	std::string response;
	if(deploymentStatus)
	{	
		response = std::string("deploy ") + username + std::string(" OK");
		std::istringstream iss(files);
		std::string filename;
		while(iss >> filename)
		{
			response += std::string(" ") + filename;
		}
		if(files.empty())
		{
			response += std::string(" NO_FILES_DEPLOYED");
		}
	}
	else
	{
		response = std::string("deploy ") + username + std::string(" NOK");
	}
	sendUDPMessage(serverMAddress, response);
}

int main(/*int argc, char const *argv[]*/)
{
	ServerD serverD(SERVER_D_UDP_PORT);

	while(1)
	{
		std::string message;
		sockaddr_in serverMAddress;
		bool messageReceivedFromServerM = serverD.receiveUDPMessage(serverMAddress, message);

		if(messageReceivedFromServerM)
		{
			serverD.handleReceivedMessage(message);
		}
	}

	return 0;
}
