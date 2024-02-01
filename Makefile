CXX      = g++
CFLAGS   = -g -Wall -pedantic
CXXFLAGS = $(CFLAGS)
LDFLAGS  =
LIBS     = -lpthread -lm
RM       = rm -rf

SRC_DIR   = src
OBJ_DIR   = obj
EXEC_DIR  = .
INCLUDES  = -I$(SRC_DIR)

SOCKET_SRC = $(SRC_DIR)/socket.cpp
CLIENT_SRC = $(SRC_DIR)/client.cpp
SERVEUR_SRC = $(SRC_DIR)/serveur.cpp
CLIENT_MAIN_SRC = $(SRC_DIR)/client_main.cpp
SERVEUR_MAIN_SRC = $(SRC_DIR)/serveur_main.cpp

SOCKET_OBJ = $(OBJ_DIR)/socket.o
CLIENT_OBJ = $(OBJ_DIR)/client.o
SERVEUR_OBJ = $(OBJ_DIR)/serveur.o
CLIENT_MAIN_OBJ = $(OBJ_DIR)/client_main.o
SERVEUR_MAIN_OBJ = $(OBJ_DIR)/serveur_main.o

EXEC = client_main serveur_main

all: $(EXEC)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

client_main: $(CLIENT_OBJ) $(CLIENT_MAIN_OBJ) $(SOCKET_OBJ)
	$(CXX) $(LDFLAGS) -o $(EXEC_DIR)/$@ $^ $(LIBS)

serveur_main: $(SERVEUR_OBJ) $(CLIENT_OBJ) $(SOCKET_OBJ) $(SERVEUR_MAIN_OBJ)
	$(CXX) $(LDFLAGS) -o $(EXEC_DIR)/$@ $^ $(LIBS)

clean:
	$(RM) $(OBJ_DIR)/*.o $(EXEC_DIR)/$(EXEC)
