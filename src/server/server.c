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

struct sockaddr_in createAddress(int serverSocket, char *port)
{
    in_port_t serverPort = htons((in_port_t)atoi(port));

    /* Construct local address structure */
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
    servAddr.sin_family = AF_INET;          /* Internet address family */
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = serverPort;

    // Bind to the local address
    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr) < 0))
    {
        printErrorAndExit("bind() failed");
    }

    return servAddr;
}

struct sockaddr_in createServerAddress(int serverSocket, char *port, int broadcast)
{
    if (broadcast == 1)
    {
        return createBroadcastAddress(port);
    }
    else
    {
        return createAddress(serverSocket, port);
    }
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

void receiveMessage(int serverSocket, char *buffer, struct sockaddr_storage clntAddr)
{
    socklen_t clntAddrLen = sizeof(clntAddr);
    // Size of received message
    ssize_t numBytesRcvd = recvfrom(serverSocket, buffer, MAXSTRINGLENGTH, 0,
                                    (struct sockaddr *)&clntAddr, &clntAddrLen);
    if (numBytesRcvd < 0)
    {
        printErrorAndExit("ERROR: recvfrom failed");
    }
}

void sendMessageToClient(int serverSocket, char *message, struct sockaddr_storage clntAddr)
{
    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(serverSocket, message, MAXSTRINGLENGTH, 0,
                                  (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed)");
    }
}

void receiveMessageAndRespond(int serverSocket)
{
    struct sockaddr_storage clntAddr; // Client address

    // Block until receive message from a client
    char buffer[MAXSTRINGLENGTH]; // I/O buffer

    receiveMessage(serverSocket, buffer, clntAddr);
    sendMessageToClient(serverSocket, buffer, clntAddr);
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
    // struct sockaddr_in serverAddress = createServerAddress(serverSocket, port, 1);
    // sendMessageTo(serverSocket, port, sendString);
    //
    //

    while (1)
    {
        createServerAddress(serverSocket, port, 0);
        receiveMessageAndRespond(serverSocket);
    }
}