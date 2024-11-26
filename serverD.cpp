/*
*  serverD.cpp
* 
*  This is the derived class for a server D. It constructs a server with a UDP socket 
*  and functions to support the actions for "deployment" of a repository.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#include "serverD.h"

/*
*  Constructor for server D.
*
*  @param udpPortNumber The static port number for the UDP Socket
*/
ServerD::ServerD(int udpPortNumber)
	: Server(udpPortNumber, "D")
{
	serverMAddress.sin_family = AF_INET;
	serverMAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverMAddress.sin_port = htons(SERVER_M_UDP_PORT);
}

/*
*  Override for the virtual receiveTCPMessage(...) function as server D does not Support TCP
*/
bool ServerD::receiveTCPMessage(const int /*socket*/, std::string& /*message*/)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

/*
*  Override for the virtual sendTCPMessage(...) function as server D does not Support TCP
*/
bool ServerD::sendTCPMessage(const int /*socket*/, const std::string& /*message*/)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

/*
*  Function parses the received command to deploy, take actin to deploy,
*  generate a response and send it to the main server.
*
*  @param message the received message of the deploy command from the main server
*/
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

/*
*  Function executes the deployment. It opens or creates a deployed.txt file
*  and appends the username and the filenames for the user's repository.
*  
*  @param username the username of the repo deployed from
*  @param message the received message of the deploy command from the main server
*  @return bool true if able to open and write to file, false if not successful.
*/
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
		file << username << " " << filename << '\n';
	}

	file.close();

	printf("Server D has deployed the user %s's repository.\n", username.c_str());
	return true;
}

/*
*  Function sends the response to the main server
*  
*  @param deploymentStatus true if successfully deployed, false if unsuccessfully deployed.
*  @param username the username of the repo deployed from
*  @param files the list of files deployed, space delimited
*/
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

/*
*  Main function:
*  Instantiate server D, then loop as long as server is open and listen and process commands
*/
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
