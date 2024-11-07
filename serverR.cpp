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

void ServerR::addToRepository(const std::string& username, const std::string& filename)
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
		return;
	}
	membersRepository[username].insert(filename);
}

void ServerR::removeFromRepository(const std::string& username, const std::string& filename)
{
	std::string target = username + " " + filename;
	std::ifstream file(FILENAMES_FILE);
	if(!file.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
		return;
	}

	std::ofstream tmpFile("tmp.txt");
	if(!tmpFile.is_open())
	{
		if(DEBUG)
		{
			printf("[PANIC] Error in opening file %s", FILENAMES_FILE.c_str());
		}
		file.close();
		return;
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
		if(std::remove(filename.c_str()) != 0)
		{
			if(DEBUG)
			{
				printf("[PANIC] Could not delete the original file.\n");
			}
		}
		else if (std::rename("tmp.txt", FILENAMES_FILE.c_str()) != 0)
		{
			if(DEBUG)
			{
				printf("[PANIC] Could not replace the original file.\n");
			}
		}
		else
		{
			if(DEBUG)
			{
				printf("[DEBUG] Line removed from the repository.\n");
			}
		}
	}
}

void ServerR::lookup(const std::string& username, const sockaddr_in& clientAddr)
{
	std::string response = std::string("lookup") + std::string(" ") + username; 
	const auto& filenames = membersRepository[username];
	for (const auto& filename : filenames)
	{
		response += std::string(" ") + filename;
	}
	
	sendUDPMessage(response, clientAddr);
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
		std::string command, username, filename;
		iss >> command >> username >> filename;

		if(command.compare("lookup") == 0 && !username.empty())
		{
			printf("Server R has received a lookup request from the main server.\n");
			lookup(username, clientAddr);
			printf("Server R has finished sending the response to the main server.\n");
		}
		else if(command.compare("push") == 0 && !username.empty() && !filename.empty())
		{
			printf("Server R has received a push request from the main server.\n");
			std::set<std::string> filenames = membersRepository[username];
			auto searchResult = filenames.find(filename);
			if(searchResult != filenames.end())
			{
				addToRepository(username, filename);
				printf("%s uploaded successfully.\n", filename.c_str());
			}
			else //file already exists
			{
				//write code to ask for overwrite
			}

		}
		else if(command.compare("remove") == 0 && !username.empty() && !filename.empty())
		{
			printf("Server R has received a remove request from the main server.\n");
		}
		else if(command.compare("deploy") == 0)
		{
			printf("Server R has received a deploy request from the main server.\n");
		}
		else
		{
			if(DEBUG)
		 	{
		 		printf("[DEBUG] Received Invalid Command.\n");
		 	}
		 	//do nothing with invalid command.
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
