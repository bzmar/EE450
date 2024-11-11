#include "client.h"

Client::Client()
	: isAuthenticated()
	, username()
	, password()
	, TCPSocket(-1)
{
	if(!setupTCPServer())
	{
		printf("Failed to create TCP server.\n");
		return;
	}
	else
	{
		printf("The client is up and running.\n");
	}
};

Client::~Client()
{
	if(TCPSocket != -1) close(TCPSocket);
}

bool Client::setupTCPClient()
{
	TCPSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPSocket < 0)
	{
		printf("Failed to create TCP Socket.\n");
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
    serverAddr.sin_port = htons(SERVER_PORT);

    if (connect(TCPSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Failed connection with the server.\n");
        close(TCPSocket);
        TCPSocket(-1);
        return false;
    }
    
    sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int getsock_check = getsockname(TCPSocket, (struct sockaddr*)&clientAddr, &addrlen);
    if (getsock_check == -1) {
        printf("Failed to getsockname().\n");
        close(TCPSocket);
        TCPSocket = -1;
        return false;
    }

    printf("Connected from local IP: %s on port: %s", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    return true;
}

bool Client::receiveMessage(std::string& response)
{
	if(TCPSocket == -1)
	{
		printf("Socket is not connected.\n");
		return false;
	}

	char buffer[BUFFER_SIZE];
	ssize_t bytes_received = recv(TCPSocket, buffer, sizeof(buffer) - 1, 0);
	if(bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		printf("Received Message from TCP Client: %s.\n", buffer);
		response = "Message Received:" + std::to_string(buffer);
		return true;
	}
	
	std::cout << "Received nothing" << std::endl;
	return false;
}

bool Client::sendMessage(const std::string& message)
{
	if(TCPSocket == -1)
	{
		printf("Socket is not connected.\n");
		return false;
	}

	ssize_t bytesSent = send(TCPSocket, message.c_str(), message.size(), 0);
	if(bytesSent < 0)
	{
		printf("Error Sending Message.\n");
		return false;
	}

	printf("Sent Message: %s.\n", message);
		
}

void Client::promptLogin(string &username, string &password)
{
	while(username.empty())
	{
		printf("Please enter username (usename cannot be empty): ");
		std::getline(std::cin, username)
	}
	while(password.empty())
	{
		printf("Please enter password (password cannot be empty): ");
		std::getline(std::cin, password)
	}
}


int main(int argc, char const *argv[])
{
	Client c;
	if(argc == 3)
	{
	 	c.username = argv[1];
		c.password = argv[2];
	}
	else
	{
		c.promptLogin(c.username, c.password)
	}

	//authentication

	return 0;
}