#include "serverM.h"

ServerM::ServerM()
	: TCPServerSocket(-1)
	, UDPServerSocket(-1)
	, TCPServerAddr()
	, UDPServerAddr()
	, TCPClientSockets()
	, clientMutex()
{
	bool UDPServerReady = false;
	bool TCPServerReady = false;
	printf("Booting Server M ..");
	do
	{
		printf(".");
		if(!UDPServerReady)
		{
			UDPServerReady = setupUDPServer();
		}
		if(!TCPServerReady)
		{
			TCPServerReady = setupTCPServer();
		}
		if(!UDPServerReady || !TCPServerReady)
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}while(!UDPServerReady || !TCPServerReady);
};

ServerM::~ServerM()
{
	if(TCPServerSocket != -1) close(TCPServerSocket);
	if(UDPServerSocket != -1) close(UDPServerSocket);
}

bool ServerM::setupTCPServer()
{
	TCPServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPServerSocket < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to create TCP Socket.\n");
		}
		return false;
	}

	sockaddr_in TCPServerAddr;
	TCPServerAddr.sin_family = AF_INET;
    TCPServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    TCPServerAddr.sin_port = htons(TCP_PORT);

    int bindResult = bind(TCPServerSocket, (struct sockaddr*)&TCPServerAddr, sizeof(TCPServerAddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to bind TCP socket.\n");
		}
		close(TCPServerSocket);
		return false;
	}

	int listenResult = listen(TCPServerSocket, 5);
	if(listenResult < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] TCP Socket failed to listen.\n");
		}
		close(TCPServerSocket);
		return false;
	}

	printf("Server M is up and running using TCP on port %d.\n", TCP_PORT);
	return true;
}


bool ServerM::setupUDPServer()
{
	UDPServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPServerSocket < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to create UDP Socket.\n");
		}
		return false;
	}

	sockaddr_in UDPServerAddr;
	UDPServerAddr.sin_family = AF_INET;
    UDPServerAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    UDPServerAddr.sin_port = htons(UDP_PORT);

    int bindResult = bind(UDPServerSocket, (struct sockaddr*)&UDPServerAddr, sizeof(UDPServerAddr));
	if(bindResult < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to bind TCP socket.\n");
		}
		close(UDPServerSocket);
		return false;
	}

	printf("\nServer M is up and running using UDP on port %d.\n", UDP_PORT);
	return true;
}

void ServerM::acceptTCPConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket;
	clientSocket = accept(TCPServerSocket, (sockaddr*)&clientAddr, (socklen_t*)&clientAddrLen);
	if(clientSocket < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to accept TCP Client.\n");
		}
	}
	else
	{
		if(DEBUG)
		{
			printf("[DEBUG] Connected to TCP Client.\n");
		}
		std::lock_guard<std::mutex> lock(clientMutex);
		TCPClientSockets.push_back(clientSocket);

		std::thread clientThread(&ServerM::handleTCPClient, this, clientSocket);
		clientThread.detach();
	}
}

void ServerM::handleTCPClient(int clientSocket)
{
	std::string clientMessage = receiveTCPMessage(clientSocket);


	close(clientSocket);
	clientSocket = -1;
}

bool ServerM::getResponseFromServer(std::string& response)
{
	bool responseFromServerReceived = false;
	do
	{
		responseFromServerReceived = receiveUDPMessage(response);
	}
	while(!responseFromServerReceived);

	return responseFromServerReceived;
}

void ServerM::processReceivedMessageFromClient(int clientSocket, const std::string& message)
{
	std::istringstream iss(message.c_str());
	std::string action;
	iss >> action;
	sockaddr_in targetUDPAddr;
	targetUDPAddr.sin_family = AF_INET;
	targetUDPAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	int clientSocketPort = getSockInfo(clientSocket);
	int serverUDPPort = getSockInfo(UDPServerSocket);

	if(action.compare("login") == 0)
	{
		std::string username, password;
		iss >> username >> password;
		printf("Server M has received username: %s and password: %s\n", username.c_str(), std::string(password.length(), '*').c_str());
		targetUDPAddr.sin_port = htons(SERVERA_PORT);
		bool messageSent = sendUDPMessage(message, targetUDPAddr);
		if(messageSent)
		{
			printf("Server M has sent authentication request to Server A.\n");
		}
		// else
		// {
		// 	std::cout << "message not sent" << std::endl;
		// }
		std::string responseServerA;
		bool responseServerAReceived = getResponseFromServer(responseServerA);
		if(responseServerAReceived)
		{
			printf("The main server has received the response from server A using UDP over port %d.\n", serverUDPPort);
		}
		sendTCPMessage(clientSocket, responseServerA);
		printf("The main server has sent the response from server A to client using TCP over port %d.\n", clientSocketPort);
	}
	else if(action.compare("lookup") == 0)
	{
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		std::string targetUser, user;
		iss >> targetUser >> user;
		printf("The main server has received a lookup request from %s to lookup %s's repository using TCP over port %d.\n", user.c_str(), targetUser.c_str(), clientSocketPort);
		sendUDPMessage(message, targetUDPAddr);
		printf("The main server has sent the lookup request to server R.\n");
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		if(responseServerRReceived)
		{
			printf("The main server has received the response from server R using UDP over port %d.\n", serverUDPPort);
		}
		sendTCPMessage(clientSocket, responseServerR);
		printf("The main server has sent the lookup response to the client.\n");
	}
	else if(action.compare("push") == 0)
	{
		std::string username, filename, OWC;
		iss >> username >> filename >> OWC;
		std::cout << username << ", " << filename << ", " << OWC << std::endl;
		if( (OWC.compare("NOC") == 0) || (OWC.compare("OC") == 0) )
		{
			printf("The main server has received the overwrite confirmation from %s using TCP over port %d.\n", username.c_str(), clientSocketPort);
		}
		else if(OWC.empty())
		{
			printf("The main server has received a push request from %s, using TCP over port %d.\n", username.c_str(), clientSocketPort);
		}
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		if( (OWC.compare("NOC") == 0) || (OWC.compare("OC") == 0) )
		{
			printf("The main server has sent the overwrite confirmation response to server R.\n");
		}
		else if(OWC.empty())
		{
			printf("The main server has sent the push request to server R.\n");
		}
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		if(responseServerRReceived)
		{
			if(OWC.compare("CO") == 0)
			{
				printf("The main server has received the response from server R using UDP over port %d, asking for overwrite confirmation.\n", serverUDPPort);	
			}
			else
			{
				printf("The main server has received the response from server R using UDP over port %d.\n", serverUDPPort);
			}
		}
		sendTCPMessage(clientSocket, responseServerR);
		if(OWC.compare("CO") == 0)
		{
			printf("The main server has sent the overwrite confirmation request to the client.\n");	
		}
		else
		{
			printf("The main server has sent the push response to the client.\n");	
		}
	}
	else if(action.compare("remove") == 0)
	{
		std::string username;
		iss >> username;
		printf("The main server has received a remove request from member %s over TCP port %d\n", username.c_str(), clientSocketPort);
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		sendUDPMessage(message, targetUDPAddr);
		printf("The main server has sent the remove request to server R.\n");
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		if(responseServerRReceived)
		{
			printf("The main server has received confirmation of the remove request done by server R.\n");
		}
		sendTCPMessage(clientSocket, responseServerR);
		printf("The main server has sent the remove response to the client.\n");
	}
	else if(action.compare("deploy") == 0)
	{
		std::string username;
		iss >> username;
		printf("The main server has received a deploy request from member %s over TCP port %d.\n", username.c_str(), clientSocketPort);
		targetUDPAddr.sin_port = htons(SERVERR_PORT);
		printf("The main server has sent the lookup request to server R.\n");
		sendUDPMessage(message, targetUDPAddr);
		std::string responseServerR;
		bool responseServerRReceived = getResponseFromServer(responseServerR);
		if(responseServerRReceived)
		{
			printf("The main server has received the lookup response from server R.\n");
		}

		targetUDPAddr.sin_port = htons(SERVERD_PORT);
		sendUDPMessage(responseServerR, targetUDPAddr);
		printf("The main server has sent the deploy request to server D.\n");
		std::string responseServerD;
		bool responseServerDReceived = getResponseFromServer(responseServerD);
		if(responseServerDReceived)
		{
			printf("The main server has received the deploy response from server D.\n");
		}

		sendTCPMessage(clientSocket, responseServerD);
		std::istringstream iss2(responseServerD.c_str());
		std::string result;
		iss2 >> action >> username >> result;
		if(result.compare("OK") == 0)
		{
			printf("The user %s's repository has been deployed at Server D.\n", username.c_str());
		}
		printf("The main server has sent the deploy response to the client.\n");
	}
}

bool ServerM::sendTCPMessage(int clientSocket, const std::string& message)
{
	if(clientSocket == -1)
	{
		if(DEBUG)
		{
			printf("[DEBUG] No active TCP client connection.\n");
		}
		return false;
	}

	ssize_t bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		if(DEBUG)
		{
			printf("[DEBUG] Failed to send TCP message.\n");
		}
		return false;
	}
	if(DEBUG)
	{
		printf("[DEBUG] Sent TCP message: %s\n", message.c_str());
	}
	return true;
}

std::string ServerM::receiveTCPMessage(int clientSocket)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesReceived;
	while((bytesReceived = read(clientSocket, buffer, sizeof(buffer)-1)) > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received from TCP Client: %s.\n", buffer);
		}
		processReceivedMessageFromClient(clientSocket, std::string(buffer));
	}

	return std::string(buffer);
}

bool ServerM::sendUDPMessage(const std::string& message, const sockaddr_in& serveraddr)
{
	ssize_t bytesSent = sendto(UDPServerSocket, message.c_str(), message.size(), MSG_DONTWAIT, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if(bytesSent < 0)
	{
		printf("Failed to send UDP message.\n");
		return false;
	}

	// printf("Sent UDP message: %s\n", message.c_str());
	return true;
}

bool ServerM::receiveUDPMessage(std::string& response)
{
	char buffer[BUFFER_SIZE];
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	ssize_t bytesReceived = recvfrom(UDPServerSocket, buffer, sizeof(buffer)-1, 
		MSG_DONTWAIT, (sockaddr*)&clientAddr, &clientAddrLen);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		if(DEBUG)
		{
			printf("[DEBUG] Received from UDP Server: %s.\n", buffer);
		}

		response = std::string(buffer);
		return true;
	}
	else
	{
		printf("[DEBUG] No response from UDP Server.\n");
	}
	return false;
}

int main(/*int argc, char const *argv[]*/)
{
	ServerM* m = new ServerM();

	while(1)
	{
		m->acceptTCPConnection();
	}


	return 0;
}