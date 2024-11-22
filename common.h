#pragma once

#include <iostream>
#include <sstream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

bool DEBUG = false;

int getSockInfo(int socket)
{
	sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    
    if(getsockname(socket, (struct sockaddr*)&addr, &addrlen) == -1)
    {
    	if(DEBUG)
    	{
    		printf("[PANIC] Error in getsockname(...): %s", std::strerror(errno));
    	}
    	return -1;
    }

    return ntohs(addr.sin_port);
}