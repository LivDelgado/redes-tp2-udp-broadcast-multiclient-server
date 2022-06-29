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

struct addrinfo *getServerAddress(char *serverIpAddress, char *serverPort)
{
    //
    // ADDRESS - Get foreign address for server:
    //
    struct addrinfo addressCriteria;                      // Criteria for address match
    memset(&addressCriteria, 0, sizeof(addressCriteria)); // Zero out structure
    addressCriteria.ai_family = AF_INET;                  // IPV4
    addressCriteria.ai_socktype = 0;
    addressCriteria.ai_protocol = 0;

    struct addrinfo *serverAddress; // List of server addresses
    int rtnVal = getaddrinfo(serverIpAddress, serverPort, &addressCriteria, &serverAddress);
    if (rtnVal != 0)
    {
        printErrorAndExit("ERROR: getaddrinfo failed");
    }

    return serverAddress;
}

int setupTcpSocket(struct addrinfo *serverAddress)
{
    int tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSocket < 0)
    {
        printErrorAndExit("ERROR: tcp socket() failed");
    }

    if (connect(tcpSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printErrorAndExit("ERROR: failed to connect to server");
    }

    return tcpSocket;
}

int setupUdpSocket(struct addrinfo *serverAddress)
{
    int udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket < 0)
    {
        printErrorAndExit("ERROR: udp socket() failed");
    }

    /*
    int bindedClientToPort = bind(udpSocket, , sizeof(serverAddress));

    if (bindedClientToPort < 0)
    {
        printErrorAndExit("ERROR: Couldn't bind to the port\n");
    }
    puts("INFO: bind to server succeeded (UDP)");
    */

    return udpSocket;
}

void sendMessageOverTcp(int clientSocket, char *message)
{
    size_t messageLength = strlen(message); // get the message size
    ssize_t numberOfBytesBeingSent = send(clientSocket, message, messageLength, 0);
    if (numberOfBytesBeingSent < 0)
    {
        printErrorAndExit("ERROR: failed to send message.");
    }
    else if (numberOfBytesBeingSent != messageLength)
    {
        printErrorAndExit("ERROR: could not send the correct message to the server.");
    }
}

void sendMessageOverUdp(int clientSocket, struct addrinfo *serverAddress, char *message)
{
    size_t messageLength = strlen(message); // get the message size
    ssize_t numberOfBytesBeingSent = sendto(
        clientSocket, message, messageLength, 0, serverAddress->ai_addr, serverAddress->ai_addrlen);
    if (numberOfBytesBeingSent < 0)
    {
        printErrorAndExit("ERROR: failed to send message.");
    }
    else if (numberOfBytesBeingSent != messageLength)
    {
        printErrorAndExit("ERROR: could not send the correct message to the server.");
    }
}

void receiveMessageOverTcp(int clientSocket)
{
    unsigned int totalBytesReceived = 0;
    ssize_t numberOfBytesReceived;
    char buffer[MAXSTRINGLENGTH] = ""; // I/O buffer

    // Receive up to the buffer size bytes from the sender
    numberOfBytesReceived = recv(clientSocket, buffer, MAXSTRINGLENGTH - 1, 0);

    if (numberOfBytesReceived < 0)
    {
        printErrorAndExit("ERROR: failed to receive response from server.");
    }
    else if (numberOfBytesReceived == 0)
    {
        printErrorAndExit("WARNING: server closed the connection!");
    }

    totalBytesReceived += numberOfBytesReceived;
    buffer[numberOfBytesReceived] = '\0';

    printf("< %s", buffer); // prints message received from the server
}

void receiveMessageOverUdp(int clientSocket)
{
    //
    // RECEIVE - get response from server
    //
    struct sockaddr_storage fromAddr; // Source address of server
    // Set length of from address structure (in-out parameter)
    socklen_t fromAddrLen = sizeof(fromAddr);
    char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
    ssize_t numberOfBytesReceived = recvfrom(clientSocket, buffer, MAXSTRINGLENGTH, 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
    if (numberOfBytesReceived < 0)
    {
        printErrorAndExit("ERROR: recvfrom() failed");
    }

    buffer[numberOfBytesReceived] = '\0';
    printf("Received: %s\n", buffer);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the client: client <server address> <server port>");
    }

    char *serverIpAddress = argv[1]; // first argument is server address
    char *serverPort = argv[2];      // second argument is server port

    struct addrinfo *serverAddress = getServerAddress(serverIpAddress, serverPort);

    int udpSocket = setupUdpSocket(serverAddress);

    while (1)
    {
        char *echoString = NULL; // create new message
        size_t echoStringLen;

        printf("> ");
        getline(&echoString, &echoStringLen, stdin); // get the message from user input

        sendMessageOverUdp(udpSocket, serverAddress, echoString);
        receiveMessageOverUdp(udpSocket);
    }
}