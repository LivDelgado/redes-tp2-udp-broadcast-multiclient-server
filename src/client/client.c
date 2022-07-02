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

struct ClientThreadArguments
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

void createClientThread(pthread_t *newClientThread, int clientUnicastSocket, int clientBroadcastSocket, struct addrinfo *serverAddress, void *(*threadFunction) (void *))
{
    struct ClientThreadArguments *clientThreadArgs = (struct ClientThreadArguments *)malloc(sizeof(struct ClientThreadArguments));
    clientThreadArgs->clientUnicastSocket = clientUnicastSocket;
    clientThreadArgs->clientBroadcastSocket = clientBroadcastSocket;
    clientThreadArgs->serverAddress = serverAddress;

    int clientThreadStatus = pthread_create(newClientThread, NULL, threadFunction, (void *)clientThreadArgs);
    if (clientThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create thread");
    }

}

void *receiveUnicastThread(void *data) {
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

    while (1)
    {
        receiveMessageFromServer(threadData->clientUnicastSocket, threadData->serverAddress);
    }

    free(threadData);
    pthread_exit(NULL);
}

void *receiveBroadcastThread(void *data)
{
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

    while (1)
    {
        puts(receiveBroadcastMessage(threadData->clientBroadcastSocket));
    }

    free(threadData);
    pthread_exit(NULL);
}

void *sendUnicastThread(void *data)
{
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

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
    // GET SERVER ADDRESS
    //
    struct addrinfo *serverAddress = getServerAddress(serverIpAddress, serverPort);
    //
    //
    // THREADS
    //
    pthread_t unicastListenerThread = 0;
    pthread_t unicastSenderThread = 0;
    pthread_t broadcastListenerThread = 0;

    createClientThread(&unicastListenerThread, clientUnicastSocket, clientBroadcastSocket, serverAddress, receiveUnicastThread);
    createClientThread(&unicastSenderThread, clientUnicastSocket, clientBroadcastSocket, serverAddress, sendUnicastThread);
    createClientThread(&broadcastListenerThread, clientUnicastSocket, clientBroadcastSocket, serverAddress, receiveBroadcastThread);

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