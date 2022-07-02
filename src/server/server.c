#include "utils.h"
#include "protocol.h"
#include "messaging.h"
#include "control.h"
#include "threads.h"

#include <pthread.h>

void processNewConnection(struct ServerThreadArguments *threadData)
{
    int equipmentId = newConnection(threadData->clientAddrIn);

    char *zero = "";
    if (equipmentId < 10)
    {
        zero = "0";
    }
    printf("Equipment %s%i added\n", zero, equipmentId);

    sendMessageTo(*(threadData->broadcastServerAddress), threadData->serverBroadcastSocket, constructMessageWithTwoFields(3, equipmentId));

    char reslistOutput[MAXSTRINGLENGTH] = "";
    strcat(reslistOutput, "04");
    strcat(reslistOutput, SPLITTER);
    strcat(reslistOutput, listConnectedEquipmentsAsString());
    sendMessage(reslistOutput, threadData->serverUnicastSocket, &threadData->clientAddrIn, threadData->clientAddrLen);
}

void processReqAdd(struct ServerThreadArguments *threadData, struct Message message)
{
    if (getEquipment(threadData->clientAddrIn) < 0) // equipment is not connected, will try to connect!
    {
        if (alreadyReachedMaxNumberOfConnections())
        {
            sendMessage(constructMessageWithThreeFields(7, 0, 4), threadData->serverUnicastSocket, &threadData->clientAddrIn, threadData->clientAddrLen);
        }
        else
        {
            processNewConnection(threadData);
        }
    }
}

void processReqRem(struct ServerThreadArguments *threadData, struct Message message)
{
    if (getEquipmentById(message.sourceId) < 0)
    { // check if equipment exists
        sendMessage(
            constructMessageWithThreeFields(7, message.sourceId, 1),
            threadData->serverUnicastSocket, &threadData->clientAddrIn, threadData->clientAddrLen);
    }
    else
    {
        removeConnection(message.sourceId);

        char *zero = "";
        if (message.sourceId < 10)
        {
            zero = "0";
        }
        printf("Equipment %s%i removed\n", zero, message.sourceId);

        // send ok message
        sendMessage(constructMessageWithThreeFields(8, message.sourceId, 1), threadData->serverUnicastSocket, &threadData->clientAddrIn, threadData->clientAddrLen);

        // broadcast equipment removed message
        sendMessageTo(*(threadData->broadcastServerAddress), threadData->serverBroadcastSocket, threadData->buffer);
    }
}

void *processMessageThread(void *args)
{
    struct ServerThreadArguments *threadData = (struct ServerThreadArguments *)args;

    struct Message messageReceived = structureMessage(threadData->buffer);
    MessageType messageType = (MessageType)messageReceived.messageId;
    switch (messageType)
    {
    case REQ_ADD:
        processReqAdd(threadData, messageReceived);
        break;
    case REQ_REM:
        processReqRem(threadData, messageReceived);
        break;
    default:
        break;
    }

    free(threadData);
    pthread_exit(NULL);
}

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

        struct ServerThreadArguments *newThreadArgs = (struct ServerThreadArguments *)malloc(sizeof(struct ServerThreadArguments));
        *newThreadArgs = *threadData;

        pthread_t processorThread = 0;
        createServerThreadBasedOnExistingThread(&processorThread, newThreadArgs, processMessageThread);
        pthread_join(processorThread, NULL);

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
    pthread_t unicastListenerThread = 0;
    pthread_t unicastSenderThread = 0;
    // pthread_t broadcastSenderThread = 0;

    createServerThread(&unicastListenerThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, receiveUnicastThread);
    createServerThread(&unicastSenderThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, sendUnicastThread);
    // createServerThread(&broadcastSenderThread, serverUnicastSocket, serverBroadcastSocket, &broadcastServerAddress, sendBroadcastThread);

    pthread_join(unicastListenerThread, NULL);
    // pthread_join(unicastSenderThread, NULL);
    // pthread_join(broadcastSenderThread, NULL);
}