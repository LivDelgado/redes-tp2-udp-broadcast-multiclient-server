#ifndef THREAD_MANIPULATION
#define THREAD_MANIPULATION

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#include "utils.h"

struct ClientThreadArguments
{
    struct addrinfo* serverAddress;
    int clientUnicastSocket;
    int clientBroadcastSocket;
};

struct ServerThreadArguments
{
    int serverBroadcastSocket;
    int serverUnicastSocket;
    struct sockaddr_in *broadcastServerAddress;
    struct sockaddr_in clientAddrIn;
    socklen_t clientAddrLen;
    char buffer[MAXSTRINGLENGTH];
};

void createServerThread(
    pthread_t *newServerThread,
    int serverUnicastSocket,
    int serverBroadcastSocket,
    struct sockaddr_in *broadcastServerAddress,
    void *(*threadFunction)(void *));

void createServerThreadBasedOnExistingThread(
    pthread_t *newServerThread,
    struct ServerThreadArguments *threadArguments,
    void *(*threadFunction)(void *));

void createClientThread(
    pthread_t *newClientThread,
    int clientUnicastSocket,
    int clientBroadcastSocket,
    struct addrinfo *serverAddress,
    void *(*threadFunction) (void *));

#endif