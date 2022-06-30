#include <string.h>

#include "utils.h"
#include "protocol.h"
#include "messaging.h"

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