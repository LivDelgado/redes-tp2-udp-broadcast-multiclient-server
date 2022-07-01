#include <time.h>
#include <pthread.h>

#include "utils.h"
#include "protocol.h"

#define MAXTHREADS 15

int numberOfThreads = 0;

struct ThreadArgs
{
    int serverSocket;
    struct sockaddr_in clientAddrIn;
    socklen_t clientAddrLen;
    char buffer[MAXSTRINGLENGTH];
};

void *ThreadMain(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;

    int connection_id = threadArgs->clientAddrIn.sin_port;
    struct sockaddr_in from = threadArgs->clientAddrIn;
    char *ip = inet_ntoa(from.sin_addr);
    printf("INFO: created new thread to handle client request %s:%d.\n", ip, connection_id);

    // testing error message
    sendMessage("07 01 04", threadArgs->serverSocket, &threadArgs->clientAddrIn, threadArgs->clientAddrLen);

    free(threadArgs);
    numberOfThreads--;
    return NULL;
}

void createThreadToHandleReceivedMessage(pthread_t *threads, struct ThreadArgs *threadArgs)
{
    int threadStatus = pthread_create(&threads[numberOfThreads], NULL, ThreadMain, (void *)threadArgs);
    if (threadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create thread");
    }
    else
    {
        numberOfThreads++;
    }
}

int main(int argc, char *argv[])
{
    // validate the number of arguments
    // address is mandatory, port is optional (it uses the default one if empty)
    if (argc < 1 || argc > 2)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the server: server <port (optional)>");
    }

    // set the server port
    char *port = "51511";
    if (argc == 3)
    {
        port = argv[2]; // second argument (optional) -> port
    }

    // to generate a random number later on
    srand(time(NULL));

    int serverSocket = createUdpSocket();
    setSocketPermissionToBroadcast(serverSocket);

    //
    // THREADS
    //
    pthread_t threads[MAXTHREADS];

    createAddress(serverSocket, port);
    while (1)
    {
        // Create separate memory for client argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        threadArgs->serverSocket = serverSocket;
        threadArgs->clientAddrLen = sizeof(struct sockaddr_in);

        puts("INFO: waiting for the first contact from the client");

        receiveMessage(serverSocket, threadArgs->buffer, &threadArgs->clientAddrIn, threadArgs->clientAddrLen);
        createThreadToHandleReceivedMessage(threads, threadArgs);
    }

    /*
    //
    // BROADCAST
    //
    char *sendString = "teste";
    struct sockaddr_in serverAddress = createBroadcastAddress(port);
    while (1) {
        sendMessageTo(serverAddress, serverSocket, sendString);
    }
    //
    //
    */

    /*
    //
    // UNICAST
    //
    createAddress(serverSocket, port);
    while (1)
    {
        receiveMessageAndRespond(serverSocket);
    }
    //
    //
    //
    */
}