#ifndef PROTOCOL_COMMON
#define PROTOCOL_COMMON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utils.h"

int createUdpSocket();
void setSocketPermissionToBroadcast(int serverSocket);

// CLIENT FUNCTIONS
void bindToBroadcasterServer(int clientSocket, char *serverPort);
char *receiveBroadcastMessage(int clientSocket);
struct addrinfo *getServerAddress(char *serverIpAddress, char *serverPort);
void sendMessageToServer(int clientSocket, char *message, struct addrinfo *servAddr);
char *receiveMessageFromServer(int clientSocket);

// SERVER FUNCTIONS
void receiveMessageAndRespond(int serverSocket);
char *receiveMessage(int serverSocket, char *buffer, struct sockaddr_in *clientAddrIn, socklen_t clientAddrLen);
void sendMessage(char *response, int serverSocket, struct sockaddr_in *clientAddrIn, socklen_t clientAddrLen);

struct sockaddr_in createBroadcastAddress(char *port);
void createAddress(int serverSocket, char *port);
void sendMessageTo(struct sockaddr_in serverAddress, int serverSocket, char *message);

#endif