#include "utils.h"
#include "protocol.h"
#include "messaging.h"
#include "threads.h"

#include <pthread.h>

#define STANDARD_INPUT 0 // File Descriptor - Standard input
#define MAX_EQUIPMENTS 15 // maximum number of connected equipments

typedef struct {
    int equipmentId;
    int listOfEquipments[MAX_EQUIPMENTS]; // works like a boolean, 0 does not exist, 1 exists
} Equipment;

Equipment self;

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
        printf("New ID: %s%i\n", zero, equipmentId);
    }
    else { // equipment is already connected
        printf("Equipment %s%i added\n", zero, equipmentId);
    }
    
}

void processResList(struct Message message)
{
    char payload[MAXSTRINGLENGTH];
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


void processMessage(char *message)
{
    struct Message response = structureMessage(message);

    if (isErrorMessage(response))
    {
        printErrorAndExit(getErrorMessage(response));
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
        processMessage(receiveMessageFromServer(threadData->clientUnicastSocket, threadData->serverAddress));
    }

    free(threadData);
    pthread_exit(NULL);
}

void *receiveBroadcastThread(void *data)
{
    struct ClientThreadArguments *threadData = (struct ClientThreadArguments *)data;

    while (1)
    {
        processMessage(receiveBroadcastMessage(threadData->clientBroadcastSocket));
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

    // send first connection message
    puts("INFO: connecting to the server...");
    self.equipmentId = 0;
    for (int i=0; i < MAX_EQUIPMENTS; i++)
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