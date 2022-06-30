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
    int serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket < 0)
    {
        printErrorAndExit("socket() failed");
    }

    return serverSocket;
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

struct sockaddr_in createServerAddress(int broadcast, char *port)
{
    in_port_t serverPort = htons((in_port_t)atoi(port));

    /* Construct local address structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_port = serverPort;              /* Broadcast port */

    if (broadcast == 1)
    {
        broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
    }
    else
    {
        broadcastAddr.sin_addr.s_addr = INADDR_ANY;
    }

    return broadcastAddr;
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

void sendMessage(int serverSocket, char *port, char *message, int broadcast)
{
    struct sockaddr_in serverAddress = createServerAddress(broadcast, port);
    sendMessageTo(serverAddress, serverSocket, message);
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

    char *sendString = "teste";

    sendMessage(serverSocket, port, sendString, 1);
}