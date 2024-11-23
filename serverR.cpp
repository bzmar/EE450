#include "serverR.h"

ServerR::ServerR(int udpPortNumber)
	: Server(udpPortNumber, "R")
{
	generateRepository();
	
	serverMAddress.sin_family = AF_INET;
	serverMAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverMAddress.sin_port = htons(SERVER_M_UDP_PORT);
}

bool ServerR::receiveTCPMessage(const int socket, std::string& message)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

bool ServerR::sendTCPMessage(const int socket, const std::string& message)
{
	throw std::runtime_error("Server A does not support TCP functionality");

	return false;
}

void ServerR::generateRepository()
{
	std::ifstream file(FILENAMES_FILE);

	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[ERR] Error in opening file %s", FILENAMES_FILE.c_str());
		}
	}

	std::string username, filename;

	while(file >> username >> filename) {
		membersRepository[username].insert(filename);
	}

	file.close();
}

void ServerR::parseAndExecuteCommand(const std::string& message)
{
	std::istringstream iss(message);
	std::string command, username, password;
	iss >> command;
	
	if(command.compare("lookup") == 0)
	{
		printf("Server R has received a lookup request from the main server.\n");
		std::string username;
		iss >> username;
		if(!username.empty())
		{
			lookup(username);
		}
		else
		{
			if(DEBUG)
			{
				printf("[ERR] No username was provided for lookup.\n");
			}
		}
	}
	else if(command.compare("push") == 0)
	{
		printf("Server R has received a push request from the main server.\n");
		std::string username, filename, overwrite;
		iss >> username >> filename >> overwrite;
		push(username, filename, overwrite);
	}
	else if(command.compare("remove") == 0)
	{
		printf("Server R has received a remove request from the main server.\n");
		std::string username, filename, response;
		iss >> username >> filename;
		remove(username, filename);
	}
	else if(command.compare("deploy") == 0)
	{
		printf("Server R has received a deploy request from the main server.\n");
		std::string username;
		iss >> username;
		deploy(username);
	}
}

void ServerR::lookup(const std::string& username)
{
	std::string response = std::string("lookup ") + username;
	const auto& search = membersRepository.find(username);
	if (search == membersRepository.end())
	{
		response += std::string(" UNF");
	}
	else
	{
		response += std::string(" UF");
		const auto& filenames = membersRepository[username];
		for (const auto& filename : filenames)
		{
			response += std::string(" ") + filename;
		}
	}
	sendUDPMessage(serverMAddress, response);
	printf("Server R has finished sending the response to the main server.\n");
}

void ServerR::push(const std::string& username, const std::string& filename, const std::string& overwrite)
{
	printf("Server R has received a push request from the main server.\n");
	std::string response;
	if(!username.empty() && !filename.empty())
	{
		if(overwrite.compare("OC") == 0)
		{
			bool removeSuccess = removeFromRepository(username, filename);
			bool addSuccess = addToRepository(username, filename);
			if(removeSuccess && addSuccess)
			{
				printf("%s requested overwrite; overwrite successful.\n", username.c_str());
				response = std::string("push ") + username + std::string(" ") + filename + std::string(" OK");
			}
			else
			{
				if(!removeSuccess)
				{
					printf("%s requested overwrite; overwrite unsuccessful, could not remove existing file.\n", username.c_str());
				}
				else if(!addSuccess)
				{
					printf("%s requested overwrite; overwrite unsuccessful, could not add file.\n", username.c_str());
				}
				response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
			}
		}
		else if(overwrite.compare("NOC") == 0)
		{
			response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
			printf("%s declined overwrite.\n", username.c_str());
		}
		else
		{
			std::set<std::string> filenames = membersRepository[username];
			auto searchResult = filenames.find(filename);
			if(searchResult == filenames.end())
			{
				bool addSuccess = addToRepository(username, filename);
				if(addSuccess)
				{
					response = std::string("push ") + username + std::string(" ") + filename + std::string(" OK");
				}
				else
				{
					response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
				}
				printf("%s uploaded successfully to %s's repository.\n", filename.c_str(), username.c_str());
			}
			else
			{
				response = std::string("push ") + username + std::string(" ") + filename + std::string(" CO");
				printf("%s exists in %s's repository; requesting overwrite confirmation.\n", filename.c_str(), username.c_str());
			}
		}
	}
	else
	{
		if(DEBUG)
		{
			if(username.empty())
			{
				printf("[ERR] No username was provided for push.\n");
			}
			if(filename.empty())
			{
				printf("[ERR] No filename was provided for push.\n");
			}
			response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
		}
	}
	sendUDPMessage(serverMAddress, response);
}

void ServerR::remove(const std::string& username, const std::string& filename)
{
	std::string response;
	if(!username.empty() && !filename.empty())
	{
		std::set<std::string> filenames = membersRepository[username];
		auto searchResult = filenames.find(filename);
		if(searchResult == filenames.end())
		{
			response = std::string("remove ") + username + std::string(" ") + filename + std::string(" NOK");
			printf("%s does not exist in %s repository.\n", filename.c_str(), username.c_str());
		}
		else
		{
			bool removeSuccess = removeFromRepository(username, filename);
			if(removeSuccess)
			{
				response = std::string("remove ") + username + std::string(" ") + filename + std::string(" OK");
			}
			else
			{
				response = std::string("remove ") + username + std::string(" ") + filename + std::string(" NOK");	
			}
			printf("%s has been removed from %s's repository.\n", filename.c_str(), username.c_str());
		}
	}
	else
	{
		if(DEBUG)
		{
			response = std::string("remove ") + username + std::string(" ") + filename + std::string(" NOK");
			if(username.empty())
			{
				printf("[ERR] No username was provided for push.\n");
			}
			if(filename.empty())
			{
				printf("[ERR] No filename was provided for push.\n");
			}
		}
	}
	sendUDPMessage(serverMAddress, response);
}

void ServerR::deploy(const std::string& username)
{
	std::string response = std::string("deploy ") + username; 
	const auto& filenames = membersRepository[username];
	for (const auto& filename : filenames)
	{
		response += std::string(" ") + filename;
	}
	
	sendUDPMessage(serverMAddress, response);
	printf("Server R has finished sending the response to the main server.\n");
}

bool ServerR::addToRepository(const std::string& username, const std::string& filename)
{
	std::string entry = username + " " + filename;
	std::ofstream file(FILENAMES_FILE, std::ios::app);
	if(file.is_open())
	{
		file << entry << '\n';
		file.close();
	}
	else
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
		return false;
	}
	membersRepository[username].insert(filename);
	return true;
}

bool ServerR::removeFromRepository(const std::string& username, const std::string& filename)
{
	std::string target = username + " " + filename;
	std::ifstream file(FILENAMES_FILE);
	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
		return false;
	}

	std::ofstream tmpFile("tmp.txt");
	if(!tmpFile.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
		file.close();
		return false;
	}

	std::string line;
	bool found = false;

	while(getline(file, line))
	{
		if(line.compare(target) == 0)
		{
			found = true;
		}
		else
		{
			tmpFile << line << '\n';
		}
	}

	file.close();
	tmpFile.close();

	if(found)
	{
		while(file.is_open())
		{
			if(DEBUG)
			{
				printf("[DEBUG] Waiting to close original file.\n");
			}
		}
		while(tmpFile.is_open())
		{
			if(DEBUG)
			{
				printf("[DEBUG] Waiting to close temp file.\n");
			}
		}
		if(std::remove(FILENAMES_FILE.c_str()) != 0)
		{
			if(DEBUG)
			{
				printf("[PANIC] Could not delete the original file. (%s)\n", std::strerror(errno));
			}
			return false;
		}
		else if (std::rename("tmp.txt", FILENAMES_FILE.c_str()) != 0)
		{
			if(DEBUG)
			{
				printf("[PANIC] Could not replace the original file. (%s)\n", std::strerror(errno));
			}
			return false;
		}
		else
		{
			membersRepository[username].erase(filename);
			if(DEBUG)
			{
				printf("[DEBUG] Line removed from the repository.\n");
			}
		}
	}
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerR serverR(SERVER_R_UDP_PORT);

	while(1)
	{
		std::string message;
		sockaddr_in serverMAddress; 
		bool messageReceivedFromServerM = serverR.receiveUDPMessage(serverMAddress, message);

		if(messageReceivedFromServerM)
		{
			serverR.parseAndExecuteCommand(message);
		}
	}

	return 0;
}
