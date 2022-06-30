#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXSTRINGLENGTH 255

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
}

int createUdpSocket()
{
    /* Create socket for sending/receiving datagrams */
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printErrorAndExit("socket() failed");
    }

    return sock;
}

void setSocketPermissionToBroadcast(int serverSocket)
{
    /* Set socket to allow broadcast */
    int broadcastPermission = 1;
    int setSocketPermission = setsockopt(serverSocket, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission));
    if (setSocketPermission < 0)
    {
        printErrorAndExit("setsockopt() failed");
    }
}

struct sockaddr_in createBroadcastAddress(char *port)
{
    in_port_t serverPort = htons((in_port_t)atoi(port));

    /* Construct local address structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_port = serverPort;              /* Broadcast port */
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    return broadcastAddr;
}

void createAddress(int serverSocket, char *port)
{
    // Construct the server address structure
    struct addrinfo addrCriteria;                   // Criteria for address
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_INET;               // IPV4
    addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
    addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(NULL, port, &addrCriteria, &servAddr);
    if (rtnVal != 0)
        printErrorAndExit("getaddrinfo() failed");

    // Bind to the local address
    if (bind(serverSocket, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
        printErrorAndExit("bind() failed");

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);
}

void sendMessageTo(struct sockaddr_in serverAddress, int serverSocket, char *message)
{
    unsigned int sendStringLen = strlen(message); /* Find length of sendString */
    size_t numberBytesSent = sendto(serverSocket, message, sendStringLen, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (numberBytesSent != sendStringLen)
    {
        printErrorAndExit("sendto() sent a different number of bytes than expected");
    }
}

void receiveMessageAndRespond(int serverSocket)
{
    puts("receiving messages!");
    struct sockaddr_storage clntAddr; // Client address

    // Block until receive message from a client
    char buffer[MAXSTRINGLENGTH]; // I/O buffer

    socklen_t clntAddrLen = sizeof(&clntAddr);
    // Size of received message
    ssize_t numBytesRcvd = recvfrom(serverSocket, buffer, MAXSTRINGLENGTH, 0,
                                    (struct sockaddr *)&clntAddr, &clntAddrLen);
    if (numBytesRcvd < 0)
    {
        printErrorAndExit("ERROR: recvfrom failed");
    }

    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(serverSocket, buffer, MAXSTRINGLENGTH, 0,
                                  (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed");
    }
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

    int serverSocket = createUdpSocket();
    setSocketPermissionToBroadcast(serverSocket);

    // char *sendString = "teste";

    //
    // BROADCAST
    //
    // struct sockaddr_in serverAddress = createBroadcastAddress(port);
    // sendMessageTo(serverSocket, port, sendString);
    //
    //

    //
    // UNICAST
    //
    createAddress(serverSocket, port);
    while (1)
    {
        receiveMessageAndRespond(serverSocket);
    }
    //
    //
    //
}