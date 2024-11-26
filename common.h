/*
*  common.h
* 
*  This is a header file for common includes and constants.
*
*  @author Brian Mar
*  EE 450
*  Socket Programming Project
*/

#pragma once

#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <string>

const bool DEBUG = false;
const int MY_ID_NUMBER_LAST_THREE_DIGITS = 209;
const std::string LOCALHOST = "127.0.0.1";
const int BUFFER_SIZE = 2048;