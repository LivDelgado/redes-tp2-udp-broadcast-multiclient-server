#include "threads.h"

void createClientThread(pthread_t *newClientThread, int clientUnicastSocket, int clientBroadcastSocket, struct addrinfo *serverAddress, void *(*threadFunction)(void *))
{
    struct ClientThreadArguments *clientThreadArgs = (struct ClientThreadArguments *)malloc(sizeof(struct ClientThreadArguments));
    clientThreadArgs->clientUnicastSocket = clientUnicastSocket;
    clientThreadArgs->clientBroadcastSocket = clientBroadcastSocket;
    clientThreadArgs->serverAddress = serverAddress;

    int clientThreadStatus = pthread_create(newClientThread, NULL, threadFunction, (void *)clientThreadArgs);
    if (clientThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create client thread");
    }
}

void createServerThread(
    pthread_t *newServerThread,
    int serverUnicastSocket,
    int serverBroadcastSocket,
    struct sockaddr_in *broadcastServerAddress,
    void *(*threadFunction)(void *))
{
    struct ServerThreadArguments *serverThreadArgs = (struct ServerThreadArguments *)malloc(sizeof(struct ServerThreadArguments));
    serverThreadArgs->serverUnicastSocket = serverUnicastSocket;
    serverThreadArgs->serverBroadcastSocket = serverBroadcastSocket;
    serverThreadArgs->clientAddrLen = sizeof(struct sockaddr_in);
    serverThreadArgs->broadcastServerAddress = broadcastServerAddress;

    int serverThreadStatus = pthread_create(newServerThread, NULL, threadFunction, (void *)serverThreadArgs);
    if (serverThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create server thread");
    }
}

void createServerThreadBasedOnExistingThread(
    pthread_t *newServerThread,
    struct ServerThreadArguments *threadArguments,
    void *(*threadFunction)(void *))
{
    int serverThreadStatus = pthread_create(newServerThread, NULL, threadFunction, (void *)threadArguments);
    if (serverThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create server thread");
    }
}
