/*
*  serverM.cpp
* 
*  This is the derived class for a server M. It constructs a server with a UDP socket 
*  and TCP Socket. It also has functions to support the actions for handling requests
*  from the client to facilitating the communication of these requests to
*  the appropriate server A/R/D.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#include "serverM.h"

/*
*  Constructor for server M.
*
*  @param udpPortNumber The static port number for the UDP Socket
*  @param tcpPortNumber The static port number for the TCP Socket
*/
ServerM::ServerM(const int udpPortNumber, const int tcpPortNumber)
	: Server(udpPortNumber, tcpPortNumber, "M")
{
	serverAAddress.sin_family = AF_INET;
	serverAAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverAAddress.sin_port = htons(SERVER_A_UDP_PORT);

	serverDAddress.sin_family = AF_INET;
	serverDAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverDAddress.sin_port = htons(SERVER_D_UDP_PORT);

	serverRAddress.sin_family = AF_INET;
	serverRAddress.sin_addr.s_addr = inet_addr(LOCALHOST.c_str());
	serverRAddress.sin_port = htons(SERVER_R_UDP_PORT);
};

/*
*  Function for accepting a TCP connection through the TCPServerSocket.
*  It will create a thread for processing the request so multiple connections can be run
*  in parallel.
*/
void ServerM::acceptTCPConnectionAndProcessClientRequest()
{
	sockaddr_in clientAddr;
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

		std::thread clientThread(&ServerM::processClientRequest, this, clientSocket);
		clientThread.detach();
	}
}

/*
*  Function to receive the message from the clientSocket then parse it call
*  the correct function to act on the request.
*
*  @param clientSocket the client socket
*/
void ServerM::processClientRequest(int clientSocket)
{
	while(1)
	{
		std::string message;
		bool messageReceived = receiveTCPMessage(clientSocket, message);
		while(!messageReceived)
		{
			message.clear();
			messageReceived = receiveTCPMessage(clientSocket, message);
		}
	
		std::istringstream iss(message);
		std::string action;
		iss >> action;
		if(action.compare("login") == 0)
		{
			handleLoginRequest(clientSocket, message);
		}
		else if(action.compare("lookup") == 0)
		{
			handleLookupRequest(clientSocket, message);
		}
		else if(action.compare("push") == 0)
		{
			handlePushRequest(clientSocket, message);
		}
		else if(action.compare("remove") == 0)
		{
			handleRemoveRequest(clientSocket, message);
		}
		else if(action.compare("deploy") == 0)
		{
			handleDeployRequest(clientSocket, message);
		}
		else if(action.compare("log") == 0)
		{
			// handleLogRequest(clientSocket, message);
		}
	}
	close(clientSocket);
	clientSocket = -1;
}

/*
*  Function to handle the authentication request from the socket. It will parse the information from the request
*  then it will send the request to server A, wait for the response from the server and forward
*  the response back to the client
*
*  @param clientSocket the client socket
*  @param message contains the received message and its contents for the authentication request
*/
void ServerM::handleLoginRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
	std::string action, username, password, serverResponse;
	iss >> action >> username >> password;
	printf("Server M has received username %s and password %s.\n", username.c_str(), std::string(password.length(), '*').c_str());
	bool sendStatus = sendUDPMessage(serverAAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverAAddress, message);
	}
	printf("Server M has sent authentication request to Server A.\n");
	bool serverResponseReceived = receiveUDPMessage(serverAAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverAAddress, serverResponse);
	}
	printf("The main server has received the response from server A using UDP over port %d.\n", getSocketPort(UDPServerSocket));
	sendStatus = sendTCPMessage(clientSocket, serverResponse);
	while(!sendStatus)
	{
		sendStatus = sendTCPMessage(clientSocket, serverResponse);
	}
	printf("The main server has sent the response from server A to client using TCP over port %d\n", getSocketPort(TCPServerSocket));
}

/*
*  Function to handle the lookup request from the socket. It will parse the information from the request
*  then it will send the request to server R, wait for the response from the server and forward
*  the response back to the client
*
*  @param clientSocket the client socket
*  @param message contains the received message and its contents for the authentication request
*/
void ServerM::handleLookupRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
	std::string action, targetUser, requestingUser, serverResponse;
	iss >> action >> targetUser >> requestingUser;
	printf("The main server has received a lookup request from %s to lookup %s's repository using TCP over port %d.\n", requestingUser.c_str(), targetUser.c_str(), getSocketPort(TCPServerSocket));
	bool sendStatus = sendUDPMessage(serverRAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverRAddress, message);
	}
	printf("The main server has sent the lookup request to server R.\n");
	bool serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	}
	printf("The main server has received the response from server R using UDP over port %d.\n", getSocketPort(UDPServerSocket));
	sendStatus = sendTCPMessage(clientSocket, serverResponse);
	while(!sendStatus)
	{
		sendStatus = sendTCPMessage(clientSocket, serverResponse);
	}
	printf("The main server has sent the response to the client.\n");
}

/*
*  Function to handle the push request from the socket. It will parse the information from the request
*  then it will send the request to server R, wait for the response from the server and forward
*  the response back to the client
*
*  Legend:  NOC - Overwrite Rejected Response
*           OC  - Overwrite Accepted Response
*			EMPTY - Push Request (Initial)
*           CO - Confirm Overwrite (Ask Client)
*
*  @param clientSocket the client socket
*  @param message contains the received message and its contents for the authentication request
*/
void ServerM::handlePushRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
	std::string action, username, filename, overwrite, serverResponse;
	iss >> action >> username >> filename >> overwrite;
	if( (overwrite.compare("NOC") == 0) || (overwrite.compare("OC") == 0) )
	{
		printf("The main server has received the overwrite confirmation from %s using TCP over port %d.\n", username.c_str(), getSocketPort(TCPServerSocket));
	}
	else if(overwrite.empty())
	{
		printf("The main server has received a push request from %s, using TCP over port %d.\n", username.c_str(), getSocketPort(TCPServerSocket));
	}
	bool sendStatus = sendUDPMessage(serverRAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverRAddress, message);
	}
	if( (overwrite.compare("NOC") == 0) || (overwrite.compare("OC") == 0) )
	{
		printf("The main server has sent the overwrite confirmation response to server R.\n");
	}
	else if(overwrite.empty())
	{
		printf("The main server has rsent the push request to serverR.\n");
	}
	bool serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	}
	sendStatus = sendTCPMessage(clientSocket, serverResponse);
	while(!sendStatus)
	{
		sendStatus = sendTCPMessage(clientSocket, serverResponse);
	}
	if(overwrite.compare("CO") == 0)
	{
		printf("The main server has sent the overwrite confirmation request to the client.\n");
	}
	else
	{
		printf("The main server has sent the push response to the client.\n");
	}
}

/*
*  Function to handle the remove request from the socket. It will parse the information from the request
*  then it will send the request to server R, wait for the response from the server and forward
*  the response back to the client
*
*  @param clientSocket the client socket
*  @param message contains the received message and its contents for the authentication request
*/
void ServerM::handleRemoveRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
	std::string action, username, serverResponse;
	iss >> action >> username;
	printf("The main server has received a remove request from member %s over TCP port %d.\n", username.c_str(), getSocketPort(TCPServerSocket));
	bool sendStatus = sendUDPMessage(serverRAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverRAddress, message);
	}
	printf("The main server has sent the remove request to server R.\n");
	bool serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	}
	printf("The main server has received confirmation of the remove request done by server R.\n");
	sendStatus = sendTCPMessage(clientSocket, serverResponse);
	while(!sendStatus)
	{
		sendStatus = sendTCPMessage(clientSocket, serverResponse);
	}
	printf("The main server has sent the remove response to the client.\n");
}

/*
*  Function to handle the remove request from the socket. It will parse the information from the request
*  then it will send the request to server R, wait for the response from the server, after receiving the 
*  response it will forward the information to server D to deploy, wait for the response and forward
*  the response back to the client
*
*  @param clientSocket the client socket
*  @param message contains the received message and its contents for the authentication request
*/
void ServerM::handleDeployRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
	std::string action, username, serverResponse;
	iss >> action >> username;
	printf("The main server has received a deploy request from member %s over TCP port %d.\n", username.c_str(), getSocketPort(TCPServerSocket));
	bool sendStatus = sendUDPMessage(serverRAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverRAddress, message);
	}
	printf("The main server has sent the lookup request to server R.\n");
	bool serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	}
	printf("The main server has received the lookup response from server R.\n");
	sendStatus = sendUDPMessage(serverDAddress, message);
	while(!sendStatus)
	{
		sendStatus = sendUDPMessage(serverDAddress, message);
	}
	printf("The main server has sent the deploy request to server D.\n");
	serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	while(!serverResponseReceived)
	{
		serverResponseReceived = receiveUDPMessage(serverRAddress, serverResponse);
	}
	printf("The main server has received the deploy response from server D.\n");
	sendStatus = sendTCPMessage(clientSocket, serverResponse);
	while(!sendStatus)
	{
		sendStatus = sendTCPMessage(clientSocket, serverResponse);
	}
	std::istringstream iss2(serverResponse);
	std::string result;
	iss >> result >> result >> result;
	if(result.compare("OK") == 0)
	{
		printf("The user %s's repository has been deployed at Server D.\n", username.c_str());
	}
	printf("The main server has sent the deploy response to the client.\n");
}

/*
*  Function to handle the log request from the socket.
*/
void handleLogRequest(int clientSocket, const std::string& message)
{
	std::istringstream iss(message);
}

/*
*  Main function:
*  Instantiate server M, then loop as long as server is accept TCP connections and process requests
*/
int main(/*int argc, char const *argv[]*/)
{
	ServerM serverM(SERVER_M_UDP_PORT, SERVER_M_TCP_PORT);

	while(1)
	{
		serverM.acceptTCPConnectionAndProcessClientRequest();
	}


	return 0;
}