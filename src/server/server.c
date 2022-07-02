#include <time.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "protocol.h"
#include "messaging.h"
#include "control.h"

#define MAXTHREADS 15

int numberOfThreads = 0;

struct ThreadArgs
{
    int serverBroadcastSocket;
    int serverUnicastSocket;
    struct sockaddr_in *broadcastServerAddress;
    struct sockaddr_in clientAddrIn;
    socklen_t clientAddrLen;
    char buffer[MAXSTRINGLENGTH];
};

void *receiveUnicastThread(void *args)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)args;

    int connection_id = threadData->clientAddrIn.sin_port;
    struct sockaddr_in from = threadData->clientAddrIn;
    char *ip = inet_ntoa(from.sin_addr);
    printf("INFO: created new thread to handle client request %s:%d.\n", ip, connection_id);

    while (1)
    {
        receiveMessage(threadData->serverUnicastSocket, threadData->buffer, &threadData->clientAddrIn, threadData->clientAddrLen);
        puts("received message!");
        puts(threadData->buffer);
    }

    free(threadData);
    pthread_exit(NULL);
}

void *sendUnicastThread(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;

    int connection_id = threadArgs->clientAddrIn.sin_port;
    struct sockaddr_in from = threadArgs->clientAddrIn;
    char *ip = inet_ntoa(from.sin_addr);
    printf("INFO: created new thread to handle client request %s:%d.\n", ip, connection_id);

    /*
    char *message = "";

    if (getEquipment(from) < 0) // equipment is not connected, will try to connect!
    {
        if (alreadyReachedMaxNumberOfConnections())
        {
            message = "07 00 04";
        }
        else
        {
            int equipmentId = newConnection(from);

            char *zero = "";
            if (equipmentId < 10)
            {
                zero = "0";
            }
            printf("Equipment %s%i added\n", zero, equipmentId);

            char messageToSend[MAXSTRINGLENGTH] = "";
            sprintf(messageToSend, "03 %s%i ", zero, equipmentId);
            message = messageToSend;
        }
    }
    */

    // sendMessage(threadArgs->buffer, threadArgs->serverUnicastSocket, &threadArgs->clientAddrIn, threadArgs->clientAddrLen);

    free(threadArgs);
    numberOfThreads--;
    return NULL;
}

void *sendBroadcastThread(void *args)
{
    struct ThreadArgs *threadData = (struct ThreadArgs *)args;

    char *sendString = "teste";
    while (1)
    {
        sendMessageTo(*(threadData->broadcastServerAddress), threadData->serverBroadcastSocket, sendString);
        puts("sent! waiting 5");
        sleep(5);
    }

    free(threadData);
    pthread_exit(NULL);
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

    //
    // CREATING SOCKETS
    //
    int serverBroadcastSocket = createUdpSocket();
    int serverUnicastSocket = createUdpSocket();
    setSocketPermissionToBroadcast(serverBroadcastSocket);
    //

    //
    // PREPARING SERVER ADDRESS
    //
    createAddress(serverUnicastSocket, port);
    struct sockaddr_in broadcastServerAddress = createBroadcastAddress(BROADCAST_PORT);
    //

    //
    // THREADS
    //
    pthread_t unicastListenerThread, unicastSenderThread, broadcastSenderThread;

    struct ThreadArgs *unicastListenerThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    unicastListenerThreadArgs->serverUnicastSocket = serverUnicastSocket;
    unicastListenerThreadArgs->serverBroadcastSocket = serverBroadcastSocket;
    unicastListenerThreadArgs->clientAddrLen = sizeof(struct sockaddr_in);
    unicastListenerThreadArgs->broadcastServerAddress = &broadcastServerAddress;
    int unicastListenerThreadStatus = pthread_create(&unicastListenerThread, NULL, receiveUnicastThread, (void *)unicastListenerThreadArgs);
    if (unicastListenerThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create unicastListenerThread");
    }

    struct ThreadArgs *unicastSenderThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    unicastSenderThreadArgs->serverUnicastSocket = serverUnicastSocket;
    unicastSenderThreadArgs->serverBroadcastSocket = serverBroadcastSocket;
    unicastSenderThreadArgs->clientAddrLen = sizeof(struct sockaddr_in);
    unicastSenderThreadArgs->broadcastServerAddress = &broadcastServerAddress;
    int unicastSenderThreadStatus = pthread_create(&unicastSenderThread, NULL, sendUnicastThread, (void *)unicastSenderThreadArgs);
    if (unicastSenderThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create unicastSenderThread");
    }

    struct ThreadArgs *broadcastSenderThreadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    broadcastSenderThreadArgs->serverUnicastSocket = serverUnicastSocket;
    broadcastSenderThreadArgs->serverBroadcastSocket = serverBroadcastSocket;
    broadcastSenderThreadArgs->clientAddrLen = sizeof(struct sockaddr_in);
    broadcastSenderThreadArgs->broadcastServerAddress = &broadcastServerAddress;
    int broadcastSenderThreadStatus = pthread_create(&broadcastSenderThread, NULL, sendBroadcastThread, (void *)broadcastSenderThreadArgs);
    if (broadcastSenderThreadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create broadcastSenderThread");
    }


    pthread_join(unicastListenerThread, NULL);
    //pthread_join(unicastSenderThread, NULL);
    pthread_join(broadcastSenderThread, NULL);


    // receiveMessage(serverUnicastSocket, threadArgs->buffer, &threadArgs->clientAddrIn, threadArgs->clientAddrLen);
    // createThreadToHandleReceivedMessage(threads, threadArgs);


    /*
    //
    // UNICAST
    //
    createAddress(serverUnicastSocket, port);
    while (1)
    {
        puts("receiving message");
        receiveMessageAndRespond(serverUnicastSocket);
    }
    //
    //
    //
    puts("received and responded");
    */

    /*
    //
    // BROADCAST
    //
    char *sendString = "teste";
    struct sockaddr_in serverAddress = createBroadcastAddress(BROADCAST_PORT);
    while (1)
    {
        sendMessageTo(serverAddress, serverBroadcastSocket, sendString);
        puts("sent! waiting 5");
        sleep(5);
    }
    //
    //
    */
}