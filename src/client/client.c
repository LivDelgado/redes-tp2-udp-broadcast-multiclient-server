#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "utils.h"
#include "protocol.h"
#include "messaging.h"

#define STANDARD_INPUT 0 // File Descriptor - Standard input

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

int readFromStandardInput(char *message)
{
    struct timeval timeInterval;
    timeInterval.tv_sec = 0;
    timeInterval.tv_usec = 30000;

    fd_set readFileDescriptor;
    FD_ZERO(&readFileDescriptor);
    FD_SET(STANDARD_INPUT, &readFileDescriptor);

    select(STANDARD_INPUT + 1, &readFileDescriptor, NULL, NULL, &timeInterval);

    if (FD_ISSET(STANDARD_INPUT, &readFileDescriptor))
    {
        read(STANDARD_INPUT, message, MAXSTRINGLENGTH - 1);
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
        char messageFromTerminal[MAXSTRINGLENGTH];
        memset(messageFromTerminal, 0, sizeof(messageFromTerminal));

        if (readFromStandardInput(messageFromTerminal)) {
            puts("INFO: sending message received from terminal");
            sendMessageToServer(threadData->clientUnicastSocket, messageFromTerminal, threadData->serverAddress);
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

    //
    // CREATING SOCKETS
    //
    int clientUnicastSocket = createUdpSocket();

    int clientBroadcastSocket = createUdpSocket();
    bindToBroadcasterServer(clientBroadcastSocket, serverPort);
    //
    //
    //

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
    int broadcastListenetThreadStatus = pthread_create(&broadcastListenerThread, NULL, receiveBroadcastThread, (void *)broadcastListenetThreadArgs);
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