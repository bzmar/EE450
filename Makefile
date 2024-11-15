# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread

# Source files
CLIENT_SRC = client.cpp
SERVERM_SRC = serverM.cpp
SERVERA_SRC = serverA.cpp
SERVERD_SRC = serverD.cpp
SERVERR_SRC = serverR.cpp

# Object files
CLIENT_OBJ = client.o
SERVERM_OBJ = serverM.o
SERVERA_OBJ = serverA.o
SERVERD_OBJ = serverD.o
SERVERR_OBJ = serverR.o

# Executable names
CLIENT_EXE = Client
SERVERM_EXE = ServerM
SERVERA_EXE = ServerA
SERVERD_EXE = ServerD
SERVERR_EXE = ServerR

# Default target
all: $(CLIENT_EXE) $(SERVERM_EXE) $(SERVERA_EXE) $(SERVERD_EXE) $(SERVERR_EXE)

# Rules to compile executables
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVERM_EXE): $(SERVERM_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVERA_EXE): $(SERVERA_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVERD_EXE): $(SERVERD_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVERR_EXE): $(SERVERR_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rules to compile object files with explicit dependencies
client.o: client.cpp client.h
	$(CXX) $(CXXFLAGS) -c $<

serverM.o: serverM.cpp serverM.h
	$(CXX) $(CXXFLAGS) -c $<

serverA.o: serverA.cpp serverA.h
	$(CXX) $(CXXFLAGS) -c $<

serverD.o: serverD.cpp serverD.h
	$(CXX) $(CXXFLAGS) -c $<

serverR.o: serverR.cpp serverR.h
	$(CXX) $(CXXFLAGS) -c $<

# Clean target
clean:
	rm -f $(CLIENT_OBJ) $(SERVERM_OBJ) $(SERVERA_OBJ) $(SERVERD_OBJ) $(SERVERR_OBJ) $(CLIENT_EXE) $(SERVERM_EXE) $(SERVERA_EXE) $(SERVERD_EXE) $(SERVERR_EXE)

.PHONY: all clean
