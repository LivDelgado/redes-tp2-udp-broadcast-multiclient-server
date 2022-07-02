#include <string.h>

#include "utils.h"
#include "protocol.h"
#include "messaging.h"

void sendReqAdd(struct addrinfo *serverAddress, int clientSocket)
{
    char *reqAddMessage = "01";
    sendMessageToServer(clientSocket, reqAddMessage, serverAddress);
    struct Message response = structureMessage(receiveMessageFromServer(clientSocket, serverAddress));

    if (isErrorMessage(response)) {
        printErrorAndExit(getErrorMessage(response));
    } else {
        int equipmentId = atoi(response.payload);

        char *zero = "";
        if (equipmentId < 10) {
            zero = "0";
        }
        printf("New ID: %s%i\n", zero, equipmentId);
        
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the client: client <server address> <server port>");
    }

    char *serverIpAddress = argv[1]; // first argument is server address
    char *serverPort = argv[2];      // second argument is server port

    int clientUnicastSocket = createUdpSocket();
    int clientBroadcastSocket = createUdpSocket();
    bindToBroadcasterServer(clientBroadcastSocket, serverPort);
    struct addrinfo *serverAddress = getServerAddress(serverIpAddress, serverPort);

    // send first connection message
    sendReqAdd(serverAddress, clientUnicastSocket);

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