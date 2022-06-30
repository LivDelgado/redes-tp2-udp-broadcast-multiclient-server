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

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
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

void receiveMessageFromServer(int clientSocket)
{
    puts("INFO: receiving message from server");

    char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
    ssize_t numBytes = recv(clientSocket, buffer, MAXSTRINGLENGTH, 0);
    if (numBytes < 0)
    {
        printErrorAndExit("ERROR: recvfrom() failed");
    }

    buffer[MAXSTRINGLENGTH] = '\0';
    printf("INFO: received: %s\n", buffer);
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

        printf("> ");
        getline(&echoString, &echoStringLen, stdin); // get the message from user input
        sendMessageToServer(clientSocket, echoString, serverAddress);
        receiveMessageFromServer(clientSocket);
        //
        //
        //
    }
}