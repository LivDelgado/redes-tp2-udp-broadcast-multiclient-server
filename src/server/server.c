#include "utils.h"
#include "protocol.h"
#include "messaging.h"
#include "control.h"
#include "threads.h"

#include <pthread.h>

void *receiveUnicastThread(void *args)
{
    struct ServerThreadArguments *threadData = (struct ServerThreadArguments *)args;

    int connection_id = threadData->clientAddrIn.sin_port;
    struct sockaddr_in from = threadData->clientAddrIn;
    char *ip = inet_ntoa(from.sin_addr);
    printf("INFO: created new thread to handle client request %s:%d.\n", ip, connection_id);

    while (1)
    {
        receiveMessage(threadData->serverUnicastSocket, threadData->buffer, &threadData->clientAddrIn, threadData->clientAddrLen);
        puts("received message!");
        puts(threadData->buffer);
        memset(threadData->buffer, 0, sizeof(threadData->buffer));
    }
    
    free(threadData);
    pthread_exit(NULL);
}

void *sendUnicastThread(void *args)
{
    struct ServerThreadArguments *threadData = (struct ServerThreadArguments *)args;

    int connection_id = threadData->clientAddrIn.sin_port;
    struct sockaddr_in from = threadData->clientAddrIn;
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

            char messageToSend[MAXSTRINGLENGTH];
            memset(messageToSend, 0, sizeof(messageToSend));
            sprintf(messageToSend, "03 %s%i ", zero, equipmentId);
            message = messageToSend;
        }
    }
    */

    // sendMessage(threadData->buffer, threadData->serverUnicastSocket, &threadData->clientAddrIn, threadData->clientAddrLen);

    memset(threadData->buffer, 0, sizeof(threadData->buffer));
    free(threadData);
    return NULL;
}

void *sendBroadcastThread(void *args)
{
    struct ServerThreadArguments *threadData = (struct ServerThreadArguments *)args;

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
    // Start multithreading server!
    //
    pthread_t unicastListenerThread = 0, unicastSenderThread = 0, broadcastSenderThread = 0;

    createServerThread(&unicastListenerThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, receiveUnicastThread);
    createServerThread(&unicastSenderThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, sendUnicastThread);
    createServerThread(&broadcastSenderThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, sendBroadcastThread);

    pthread_join(unicastListenerThread, NULL);
    // pthread_join(unicastSenderThread, NULL);
    pthread_join(broadcastSenderThread, NULL);
}