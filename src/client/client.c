#include <string.h>

#include "utils.h"
#include "protocol.h"

#define EXISTING_MESSAGES 8
static const char *SPLITTER = " "; // character that represents the separation between words in the commands

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
    char message[MAXSTRINGLENGTH] = "";
    strcpy(message, originalMessage); // copy to avoid changing the message string
    struct MessageFields *messageFields = initializeMessageFields();

    struct Message structuredMessage = {0, 0, 0, NULL};
    int counter = 0;

    // first word is the id - it exists for every message
    char *word = strtok(message, SPLITTER);
    if (word == NULL)
    {
        return structuredMessage;
    }
    int messageId = atoi(word);
    if (messageId <= 0 || messageId > EXISTING_MESSAGES)
    {
        return structuredMessage;
    }

    structuredMessage.messageId = messageId;
    structuredMessage.payload = "";
    counter++;

    while (word != NULL)
    {
        word = strtok(NULL, " "); // takes next word
        if (word == NULL)
        {
            break;
        }

        counter++;
        if (counter == 2)
        {
            if (messageFields[structuredMessage.messageId - 1].hasSourceId == 1)
            {
                structuredMessage.sourceId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
                break;
            }
            else
            {
                break;
            }
        }
        else if (counter == 3)
        {
            if (messageFields[structuredMessage.messageId - 1].hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
                break;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
                break;
            }
            else
            {
                break;
            }
        }
    }

    return structuredMessage;
}

void sendReqAdd(struct addrinfo *serverAddress, int clientSocket)
{
    char *reqAddMessage = "01";
    sendMessageToServer(clientSocket, reqAddMessage, serverAddress);
    char *responseFromServer = receiveMessageFromServer(clientSocket);
    puts(responseFromServer);
    struct Message response = structureMessage(responseFromServer);

    printf("response id: %d", response.messageId);
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