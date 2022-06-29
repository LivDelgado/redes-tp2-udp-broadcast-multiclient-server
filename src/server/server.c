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
    in_port_t serverPort = htons((in_port_t)atoi(port));

    // to generate a random number later on
    srand(time(NULL));

    puts("INFO: Initializing the server.");

    /* Create socket for sending/receiving datagrams */
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printErrorAndExit("socket() failed");
    }

    puts("INFO: created socket");

    /* Set socket to allow broadcast */
    int broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0)
    {
        printErrorAndExit("setsockopt() failed");
    }
    puts("INFO: set socket option to allow broadcast");

    /* Construct local address structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_ANY;       /* Broadcast IP address */
    broadcastAddr.sin_port = serverPort;              /* Broadcast port */

    char *sendString = "teste";
    unsigned int sendStringLen = strlen(sendString); /* Find length of sendString */
    while (1)
    {
        /* Broadcast sendString in datagram to clients every 3 seconds*/
        if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
        {
            printErrorAndExit("sendto() sent a different number of bytes than expected");
        }

        puts("INFO: sent message");

        sleep(3); /* Avoids flooding the network */
    }
    /* NOT REACHED */
}