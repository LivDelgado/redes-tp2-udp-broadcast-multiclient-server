#include "utils.h"
#include "protocol.h"
#include "messaging.h"
#include "threads.h"

#include <pthread.h>

#define STANDARD_INPUT 0  // File Descriptor - Standard input
#define MAX_EQUIPMENTS 15 // maximum number of connected equipments

typedef struct
{
    int equipmentId;
    int listOfEquipments[MAX_EQUIPMENTS]; // works like a boolean, 0 does not exist, 1 exists
} Equipment;

Equipment self;

char *listConnectedEquipmentsAsString()
{
    char equipmentString[5] = "";
    memset(equipmentString, 0, sizeof(equipmentString));
    char output[MAXSTRINGLENGTH] = "";
    memset(output, 0, sizeof(output));

    for (int i = 0; i < MAX_EQUIPMENTS; i++)
    {
        if (i > 0)
        {
            strcat(output, SPLITTER);
        }
        if (self.listOfEquipments[i] == 1)
        {
            int equipmentId = i + 1;
            char *zero = "";
            if (equipmentId < 10)
            {
                zero = "0";
            }
            sprintf(equipmentString, "%s%i", zero, equipmentId);
            strcat(output, equipmentString);
        }
    }

    char *connectedEquipmentsString = output;
    return connectedEquipmentsString;
}

void sendReqAdd(struct addrinfo *serverAddress, int clientSocket)
{
    char *reqAddMessage = "01";
    sendMessageToServer(clientSocket, reqAddMessage, serverAddress);
}

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

void processResAdd(struct Message message)
{
    int equipmentId = atoi(message.payload);
    int equipmentIdOnArray = equipmentId - 1;

    char *zero = "";
    if (equipmentId < 10)
    {
        zero = "0";
    }

    // register new equipment on the list
    self.listOfEquipments[equipmentIdOnArray] = 1;

    if (self.equipmentId == 0) // it means that this equipment hasn't connected yet, then it should register things!
    {
        self.equipmentId = equipmentId;
        // NEEDED OUTPUT!!!
        printf("New ID: %s%i\n", zero, equipmentId);
        //
    }
    else
    { // equipment is already connected
        // NEEDED OUTPUT!!!
        printf("Equipment %s%i added\n", zero, equipmentId);
        //
    }
}

void processResList(struct Message message)
{
    char payload[MAXSTRINGLENGTH];
    memset(payload, 0, sizeof(payload));
    strcpy(payload, message.payload); // copy to avoid changing the payload

    char *word = strtok(payload, SPLITTER);
    while (word != NULL)
    {
        int equipmentId = atoi(word);
        int equipmentIdOnArray = equipmentId - 1;
        self.listOfEquipments[equipmentIdOnArray] = 1;
        word = strtok(NULL, " "); // takes next word
    }

    memset(payload, 0, sizeof(payload));
}

void processReqRem(struct Message message)
{
    int equipmentIdOnArray = message.sourceId - 1;
    self.listOfEquipments[equipmentIdOnArray] = 0;
    if (message.sourceId != self.equipmentId)
    {
        char *zero = "";
        if (message.sourceId < 10)
        {
            zero = "0";
        }
        // NEEDED OUTPUT!!!
        printf("Equipment %s%i removed\n", zero, message.sourceId);
        //
    }
}

void processOk(struct Message message)
{
    if (message.destineId == self.equipmentId)
    {
        // NEEDED OUTPUT!!!
        puts("Successful removal");
        //
    }
}

void processReqInf(struct Message message, struct ClientThreadArguments *threadData)
{
    // NEEDED OUTPUT!!!
    puts("requested information");
    //

    // generate information
    char *sourceZero = "";
    if (message.sourceId < 10)
    {
        sourceZero = "0";
    }
    char *destineZero = "";
    if (message.destineId < 10)
    {
        destineZero = "0";
    }

    char responseResInfMessage[MAXSTRINGLENGTH] = "";
    memset(responseResInfMessage, 0, sizeof(responseResInfMessage));
    sprintf(
        responseResInfMessage,
        "06 %s%i %s%i %i.%i%i",
        destineZero, message.destineId,
        sourceZero, message.sourceId,
        getRandomNumber(), getRandomNumber(), getRandomNumber());
    
    // respond RES_INF(IdEQj, IdEQi, PAYLOAD).
    sendMessageToServer(threadData->clientUnicastSocket, responseResInfMessage, threadData->serverAddress);
}

void processResInf(struct Message message)
{
    char *zero = "";
    if (message.sourceId < 10)
    {
        zero = "0";
    }
    // NEEDED OUTPUT!!!
    printf("Value from %s%i: %s\n", zero, message.sourceId, message.payload);
    //
}

void processMessage(char *message, struct ClientThreadArguments *threadData)
{
    struct Message response = structureMessage(message);

    if (isErrorMessage(response))
    {
        // NEEDED OUTPUT!!!
        puts(getErrorMessage(response));
        //
    }
    else
    {
        MessageType messageType = (MessageType)response.messageId;
        switch (messageType)
        {
        case RES_ADD:
            processResAdd(response);
            break;
        case RES_LIST:
            processResList(response);
            break;
        case REQ_REM:
            processReqRem(response);
            break;
        case OK:
            processOk(response);
            break;
        case REQ_INF:
            processReqInf(response, threadData);
            break;
        case RES_INF:
            processResInf(response);
            break;
        default:
            break;
        }
    }
}

void *receiveUnicastThread(void *data)
{
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

    while (1)
    {
        processMessage(receiveMessageFromServer(threadData->clientUnicastSocket, threadData->serverAddress), threadData);
    }

    free(threadData);
    pthread_exit(NULL);
}

void *receiveBroadcastThread(void *data)
{
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

    while (1)
    {
        processMessage(receiveBroadcastMessage(threadData->clientBroadcastSocket), threadData);
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

        if (readFromStandardInput(messageFromTerminal))
        {
            if (strncmp(messageFromTerminal, "close connection", strlen(messageFromTerminal) - 1) == 0)
            {
                if (self.equipmentId > 0)
                    sendMessageToServer(threadData->clientUnicastSocket, constructMessageWithTwoFields(2, self.equipmentId), threadData->serverAddress);
            }
            else if (strncmp(messageFromTerminal, "request information from", 24) == 0)
            {
                char destineIdStr[3];
                memset(destineIdStr, 0, sizeof(destineIdStr));
                memcpy(destineIdStr, &messageFromTerminal[strlen(messageFromTerminal) - 2], 2);
                destineIdStr[2] = '\0';

                int destineId = atoi(destineIdStr);
                sendMessageToServer(threadData->clientUnicastSocket, constructMessageWithThreeFields(5, self.equipmentId, destineId), threadData->serverAddress);
            }
            else if (strncmp(messageFromTerminal, "list equipment", 14) == 0)
            {
                puts(listConnectedEquipmentsAsString());
            }
            else
            {
                puts("command not recognized!");
            }
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

    // send first connection message
    self.equipmentId = 0;
    for (int i = 0; i < MAX_EQUIPMENTS; i++)
    {
        self.listOfEquipments[i] = 0;
    }
    sendReqAdd(serverAddress, clientUnicastSocket);

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
}