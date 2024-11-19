#include "serverR.h"

ServerR::ServerR()
	: UDPSocket(-1)
	, membersRepository()
{
	generateRepository();
	if(!setupUDPServer())
	{
		if(DEBUG)
		{
			printf("[DEBUG]Failed to create UDP server.\n");
		}
		return;
	}
	else
	{
		printf("Server R is up and running using UDP on port %d.\n", UDP_PORT);
	}
}

ServerR::~ServerR()
{
	if(UDPSocket != -1) close(UDPSocket);
}

void ServerR::generateRepository()
{
	std::ifstream file(FILENAMES_FILE);

	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
	}

	std::string username, filename;

	while(file >> username >> filename) {
		membersRepository[username].insert(filename);
	}

	file.close();
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
			std::cout << "Waiting to close file" << std::endl;
		}
		while(tmpFile.is_open())
		{
			std::cout << "Waiting to close file" << std::endl;
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

void ServerR::lookup(const std::string& username, const sockaddr_in& clientAddr)
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
	sendUDPMessage(response, clientAddr);
}

void ServerR::deploy(const std::string& username, const sockaddr_in& clientAddr)
{
	std::string response = std::string("deploy ") + username; 
	const auto& filenames = membersRepository[username];
	for (const auto& filename : filenames)
	{
		response += std::string(" ") + filename;
	}
	
	sendUDPMessage(response, clientAddr);
	printf("Server R has finished sending the response to the main server.\n");
}

bool ServerR::sendUDPMessage(const std::string& message, const sockaddr_in& clientAddr)
{
	ssize_t bytesSent = sendto(UDPSocket, message.c_str(), message.size(), MSG_CONFIRM, 
		(sockaddr*)&clientAddr, sizeof(clientAddr));
	if(bytesSent < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG]Failed to send UDP message.\n");
		}
		return false;
	}
	if(DEBUG)
	{
		printf("[DEBUG]Sent UDP message: %s\n", message.c_str());
	}
	return true;
}

void ServerR::receiveUDPMessage()
{
	char buffer[BUFFER_SIZE];
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	ssize_t bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer)-1, MSG_WAITALL, (sockaddr*)&clientAddr, &clientAddrLen);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received from TCP Client: %s.\n", buffer);
		}
		std::istringstream iss(buffer);
		std::string command;
		iss >> command;

		if(command.compare("lookup") == 0)
		{
			std::string username;
			iss >> username;

			printf("Server R has received a lookup request from the main server.\n");
			if(!username.empty())
			{
				lookup(username, clientAddr);
				printf("Server R has finished sending the response to the main server.\n");
			}
			else
			{
				if(DEBUG)
				{
					printf("[DEBUG] No username was provided for lookup.\n");
				}
			}
		}
		else if(command.compare("push") == 0)
		{
			std::string username, filename, overwriteConfirmation;
			iss >> username >> filename >> overwriteConfirmation;
			printf("Server R has received a push request from the main server.\n");
			std::string response;
			if(!username.empty() && !filename.empty())
			{
				if(overwriteConfirmation.compare("OC") == 0)
				{
					bool removeSuccess = removeFromRepository(username, filename);
					bool addSuccess = addToRepository(username, filename);
					if(removeSuccess && addSuccess)
					{
						response = std::string("push ") + username + std::string(" ") + filename + std::string(" OK");
					}
					else
					{
						if(!removeSuccess)
						{
							std::cout << "remove failed" << std::endl;
						}
						if(!addSuccess)
						{
							std::cout << "add failed" << std::endl;
						}
						response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
					}
				}
				else if(overwriteConfirmation.compare("ONC") == 0)
				{
					response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
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
						printf("%s uploaded successfully.\n", filename.c_str());
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
						printf("[DEBUG] No username was provided for push.\n");
					}
					if(filename.empty())
					{
						printf("[DEBUG] No filename was provided for push.\n");
					}
					response = std::string("push ") + username + std::string(" ") + filename + std::string(" NOK");
				}
			}
			sendUDPMessage(response, clientAddr);
		}
		else if(command.compare("remove") == 0)
		{
			printf("Server R has received a remove request from the main server.\n");
			std::string username, filename, response;
			iss >> username >> filename;
			if(!username.empty() && !filename.empty())
			{
				std::set<std::string> filenames = membersRepository[username];
				auto searchResult = filenames.find(filename);
				if(searchResult == filenames.end())
				{
					response = std::string("remove ") + username + std::string(" ") + filename + std::string(" NOK");
					printf("%s does not exist in the repository.\n", filename.c_str());
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
					printf("Server R has received a remove request from the main server.\n");
				}
			}
			else
			{
				if(DEBUG)
				{
					response = std::string("remove ") + username + std::string(" ") + filename + std::string(" NOK");
					if(username.empty())
					{
						printf("[DEBUG] No username was provided for push.\n");
					}
					if(filename.empty())
					{
						printf("[DEBUG] No filename was provided for push.\n");
					}
				}
			}
			sendUDPMessage(response, clientAddr);
		}
		else if(command.compare("deploy") == 0)
		{
			printf("Server R has received a deploy request from the main server.\n");
			std::string username;
			iss >> username;
			deploy(username, clientAddr);
		}
		// else if(command.compare("confirm") == 0)
		// {
		// 	std::string username, filename;
		// 	iss >> username >> filename;

		// 	if(username.empty() || filename.empty())
		// 	{
		// 		printf("Overwrite denied.\n");
		// 	}
		// 	else
		// 	{
		// 		removeFromRepository(username,filename);
		// 		addToRepository(username,filename);
		// 		printf("User requested overwrite; overwrite successful.\n");
		// 	}
		// }
		else
		{
			//do nothing with invalid command
			if(DEBUG)
		 	{
		 		printf("[DEBUG] Received Invalid Command.\n");
		 	}
		}
	}
}

bool ServerR::setupUDPServer()
{
	UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocket < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to create UDP Socket.\n");
		}
		return false;
	}

	sockaddr_in ServerRddr;
	ServerRddr.sin_family = AF_INET;
    ServerRddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    ServerRddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPSocket, (struct sockaddr*)&ServerRddr, sizeof(ServerRddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to bind TCP socket.\n");
		}
		close(UDPSocket);
		UDPSocket = -1;
		return false;
	}

	printf("UDP Server is listening on port %d.\n", UDP_PORT);
	return true;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerR server;

	while(1)
	{
		server.receiveUDPMessage();
	}

	return 0;
}
