#  Makefile
# 
#  Cmake configuration file for project
#
#  @author Brian Mar
#  EE 450
#  Socket Programming Project
#

# Compiler
CC = g++
# Compiler flags
CFLAGS = -std=c++11 -Wall -Wextra -pthread

# Source files
CLIENT_SRC = client.cpp
SERVER_SRC = server.cpp
SERVERM_SRC = serverM.cpp
SERVERA_SRC = serverA.cpp
SERVERD_SRC = serverD.cpp
SERVERR_SRC = serverR.cpp

# Object files
CLIENT_OBJ = client.o
SERVER_OBJ = server.o
SERVERM_OBJ = serverM.o
SERVERA_OBJ = serverA.o
SERVERD_OBJ = serverD.o
SERVERR_OBJ = serverR.o


# Executable names
CLIENT_EXE = client
SERVERM_EXE = serverM
SERVERA_EXE = serverA
SERVERD_EXE = serverD
SERVERR_EXE = serverR

# Files Created From Executables
DEPLOYED_FILE = deployed.txt

# Default target
all: $(CLIENT_EXE) $(SERVERM_EXE) $(SERVERA_EXE) $(SERVERD_EXE) $(SERVERR_EXE)

# Rules to compile executables
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVERM_EXE): $(SERVERM_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVERA_EXE): $(SERVERA_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVERD_EXE): $(SERVERD_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVERR_EXE): $(SERVERR_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Rules to compile object files with explicit dependencies
client.o: client.cpp client.h common.h
	$(CC) $(CFLAGS) -c $<

server.o: server.cpp server.h common.h
	$(CC) $(CFLAGS) -c $<

serverM.o: serverM.cpp serverM.h server.h
	$(CC) $(CFLAGS) -c $<

serverA.o: serverA.cpp serverA.h server.h 
	$(CC) $(CFLAGS) -c $<

serverD.o: serverD.cpp serverD.h server.h 
	$(CC) $(CFLAGS) -c $<

serverR.o: serverR.cpp serverR.h server.h 
	$(CC) $(CFLAGS) -c $<

# Clean target
clean:
	rm -f $(CLIENT_OBJ) $(SERVERM_OBJ) $(SERVERA_OBJ) $(SERVERD_OBJ) $(SERVERR_OBJ) $(CLIENT_EXE) $(SERVERM_EXE) $(SERVERA_EXE) $(SERVERD_EXE) $(SERVERR_EXE) $(DEPLOYED_FILE)

.PHONY: all clean
