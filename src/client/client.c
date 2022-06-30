#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXSTRINGLENGTH 500
#define EXISTING_MESSAGES 8
static const char *SPLITTER = " "; // character that represents the separation between words in the commands

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
}

struct MessageFields
{
    int hasSourceId;
    int hasDestineId;
    int hasPayload;
};

struct Message
{
    int messageId;
    int sourceId;
    int destineId;
    char *payload;
};

struct MessageFields *initializeMessageFields()
{
    struct MessageFields *messageFields = (struct MessageFields *)malloc(sizeof(struct MessageFields) * EXISTING_MESSAGES);

    struct MessageFields REQ_ADD, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK;

    REQ_ADD.hasSourceId = 0;
    REQ_ADD.hasDestineId = 0;
    REQ_ADD.hasPayload = 0;

    REQ_REM.hasSourceId = 1;
    REQ_REM.hasDestineId = 0;
    REQ_REM.hasPayload = 0;

    RES_ADD.hasSourceId = 0;
    RES_ADD.hasDestineId = 0;
    RES_ADD.hasPayload = 1;

    RES_LIST.hasSourceId = 0;
    RES_LIST.hasDestineId = 0;
    RES_LIST.hasPayload = 1;

    REQ_INF.hasSourceId = 1;
    REQ_INF.hasDestineId = 1;
    REQ_INF.hasPayload = 0;

    RES_INF.hasSourceId = 1;
    RES_INF.hasDestineId = 1;
    RES_INF.hasPayload = 1;

    ERROR.hasSourceId = 0;
    ERROR.hasDestineId = 1;
    ERROR.hasPayload = 1;

    OK.hasSourceId = 0;
    OK.hasDestineId = 1;
    OK.hasPayload = 1;

    messageFields = (struct MessageFields[EXISTING_MESSAGES]){REQ_ADD, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK};
    return messageFields;
}

struct Message structureMessage(char *originalMessage)
{
    struct MessageFields *messageFields = initializeMessageFields();

    struct Message structuredMessage;
    int counter = 0;
    char message[MAXSTRINGLENGTH] = "";
    strcpy(message, originalMessage); // copy to avoid changing the message string

    // first word is the id - it exists for every message
    char *word = strtok(message, SPLITTER);
    structuredMessage.messageId = atoi(word);
    counter++;

    while (word != NULL)
    {
        word = strtok(NULL, " "); // takes next word
        if (word == NULL)
            break;
    }
}

char *destructureMessage(struct Message originalMessage)
{
}

int createUdpSocket()
{
    /* Create a best-effort datagram socket using UDP */
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printErrorAndExit("socket() failed");
    }

    return sock;
}

void bindToBroadcasterServer(int clientSocket, char *serverPort)
{
    /* Construct bind structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast Address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_ANY;       /* Any incoming interface */
    broadcastAddr.sin_port = htons(atoi(serverPort)); /* Broadcast port */

    int broadcast = 1;
    int setBroadcast = setsockopt(clientSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (setBroadcast < 0)
    {
        printErrorAndExit("ERROR: failed to set broadcast socket option");
    }
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(broadcast));

    /* Bind to the broadcast port */
    if (bind(clientSocket, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
    {
        printErrorAndExit("bind() failed");
    }
}

void receiveBroadcastMessage(int clientSocket)
{
    /* Receive a single datagram from the server */
    char recvString[MAXSTRINGLENGTH + 1]; /* Buffer for received string */
    int recvStringLen = recvfrom(clientSocket, recvString, MAXSTRINGLENGTH, 0, NULL, 0);
    if (recvStringLen < 0)
    {
        printErrorAndExit("recvfrom() failed");
    }
    puts("INFO: received string");

    recvString[recvStringLen] = '\0';
    printf("Received: %s\n", recvString); /* Print the received string */
}

struct addrinfo *getServerAddress(char *serverIpAddress, char *serverPort)
{
    struct addrinfo addrCriteria;                   // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_INET;               // IPV4
    addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(serverIpAddress, serverPort, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        printErrorAndExit("ERROR: getaddrinfo failed");
    }

    return servAddr;
}

void sendMessageToServer(int clientSocket, char *message, struct addrinfo *servAddr)
{
    size_t messageLen = strlen(message);
    ssize_t numBytes = sendto(clientSocket, message, messageLen, 0, servAddr->ai_addr, servAddr->ai_addrlen);
    if (numBytes < 0)
    {
        printErrorAndExit("ERROR: failed to send to the server");
    }
    else if (numBytes != messageLen)
    {
        printErrorAndExit("ERROR: sent unexpected number of bytes");
    }

    puts("INFO: sent message to server");
}

char *receiveMessageFromServer(int clientSocket)
{
    puts("INFO: receiving message from server");

    char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
    ssize_t numBytes = recv(clientSocket, buffer, MAXSTRINGLENGTH, 0);
    if (numBytes < 0)
    {
        printErrorAndExit("ERROR: recvfrom() failed");
    }

    return buffer;
}

void sendReqAdd(struct addrinfo *serverAddress, int clientSocket)
{
    char *reqAddMessage = "01";
    sendMessageToServer(clientSocket, reqAddMessage, serverAddress);
    char *responseFromServer = receiveMessageFromServer(clientSocket);
    struct Message response = structureMessage(responseFromServer);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the client: client <server address> <server port>");
    }

    char *serverIpAddress = argv[1]; // first argument is server address
    char *serverPort = argv[2];      // second argument is server port

    int clientSocket = createUdpSocket();

    // bindToBroadcasterServer(clientSocket, serverPort);

    struct addrinfo *serverAddress = getServerAddress(serverIpAddress, serverPort);

    // send first connection message
    sendReqAdd(serverAddress, clientSocket);

    while (1)
    {
        /*
        //
        // BROADCAST
        //
        puts("INFO: receiving message");
        receiveBroadcastMessage(clientSocket);
        //
        //
        //
        */

        //
        // UNICAST
        //
        char *echoString = NULL; // create new message
        size_t echoStringLen;

        getline(&echoString, &echoStringLen, stdin); // get the message from user input
        sendMessageToServer(clientSocket, echoString, serverAddress);
        receiveMessageFromServer(clientSocket);
        //
        //
        //
    }
}