#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <pthread.h>

#define STDIN 0 // file descriptor for standard input

#include "utils.h"
#include "protocol.h"
#include "messaging.h"

void sendReqAdd(struct addrinfo *serverAddress, int clientSocket)
{
    char *reqAddMessage = "01";
    sendMessageToServer(clientSocket, reqAddMessage, serverAddress);
    struct Message response = structureMessage(receiveMessageFromServer(clientSocket, serverAddress));

    if (isErrorMessage(response))
    {
        printErrorAndExit(getErrorMessage(response));
    }
    else
    {
        int equipmentId = atoi(response.payload);

        char *zero = "";
        if (equipmentId < 10)
        {
            zero = "0";
        }
        printf("New ID: %s%i\n", zero, equipmentId);
    }
}

struct ThreadArgs
{
    struct addrinfo* serverAddress;
    int clientUnicastSocket;
    int clientBroadcastSocket;
};

int nonBlockRead(char *message)
{
    //size_t messageLen;
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    select(STDIN + 1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN, &readfds))
    {
        read(STDIN, message, MAXSTRINGLENGTH - 1);
        // getline(&message, &messageLen, stdin); // get the message from user input
        return 1;
    }
    else
    {
        fflush(stdout);
    }
    return 0;
}

void *receiveUnicastThread(void *data) {
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;

    while (1)
    {
        receiveMessageFromServer(threadData->clientUnicastSocket, threadData->serverAddress);
    }

    free(threadData);
    pthread_exit(NULL);
}

void *receiveBroadcastThread(void *data)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;

    while (1)
    {
        puts(receiveBroadcastMessage(threadData->clientBroadcastSocket));
    }

    free(threadData);
    pthread_exit(NULL);
}

void *sendUnicastThread(void *data)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)data;

    while (1)
    {
        char buffer[MAXSTRINGLENGTH];
        memset(buffer, 0, sizeof(buffer));
        if (nonBlockRead(buffer)) {
            puts("INFO: sending message");
            sendMessageToServer(threadData->clientUnicastSocket, buffer, threadData->serverAddress);
        }
    }

    free(threadData);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the client: client <server address> <server port>");
    }

    char *serverIpAddress = argv[1]; // first argument is server address
    char *serverPort = argv[2];      // second argument is server port

    int clientUnicastSocket = createUdpSocket();
    int clientBroadcastSocket = createUdpSocket();
    bindToBroadcasterServer(clientBroadcastSocket, serverPort);
    struct addrinfo *serverAddress = getServerAddress(serverIpAddress, serverPort);


    //
    // PREPARING THREADS
    //
    pthread_t unicastListenerThread, unicastSenderThread, broadcastListenerThread;

    struct ThreadArgs *unicastListenerThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    unicastListenerThreadArgs->clientUnicastSocket = clientUnicastSocket;
    unicastListenerThreadArgs->clientBroadcastSocket = clientBroadcastSocket;
    unicastListenerThreadArgs->serverAddress = serverAddress;
    int unicastListenerThreadStatus = pthread_create(&unicastListenerThread, NULL, receiveUnicastThread, (void *)unicastListenerThreadArgs);
    if (unicastListenerThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create unicastListenerThread");
    }

    struct ThreadArgs *unicastSenderThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    unicastSenderThreadArgs->clientUnicastSocket = clientUnicastSocket;
    unicastSenderThreadArgs->clientBroadcastSocket = clientBroadcastSocket;
    unicastSenderThreadArgs->serverAddress = serverAddress;
    int unicastSenderThreadStatus = pthread_create(&unicastSenderThread, NULL, sendUnicastThread, (void *)unicastSenderThreadArgs);
    if (unicastSenderThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create unicastSenderThread");
    }
    
    struct ThreadArgs *broadcastListenetThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    broadcastListenetThreadArgs->clientUnicastSocket = clientUnicastSocket;
    broadcastListenetThreadArgs->clientBroadcastSocket = clientBroadcastSocket;
    broadcastListenetThreadArgs->serverAddress = serverAddress;
    int broadcastListenetThreadStatus = pthread_create(&broadcastListenerThread, NULL, sendUnicastThread, (void *)broadcastListenetThreadArgs);
    if (broadcastListenetThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create broadcastListenerThread");
    }

    pthread_join(unicastListenerThread, NULL);
    pthread_join(unicastSenderThread, NULL);
    pthread_join(broadcastListenerThread, NULL);

    //
    //
    //

    // send first connection message
    // sendReqAdd(serverAddress, clientUnicastSocket);

    /*
    //
    // UNICAST
    //
    while(1)
    {
        char *echoString = NULL; // create new message
        size_t echoStringLen;

        getline(&echoString, &echoStringLen, stdin); // get the message from user input
        sendMessageToServer(clientUnicastSocket, echoString, serverAddress);
        puts("sent message");
        puts(receiveMessageFromServer(clientUnicastSocket));
        puts("received response");
    }
    //
    //
    //
    */

    /*
    while (1)
    {
        //
        // BROADCAST
        //
        puts(receiveBroadcastMessage(clientSocket));
        //
        //
        //
    }
    */
}