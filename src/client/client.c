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

int main(int argc, char *argv[])
{
    /* Create a best-effort datagram socket using UDP */
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printErrorAndExit("socket() failed");
    }

    int broadcast = 1;
    int setBroadcast = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (setBroadcast < 0) {
        printErrorAndExit("ERROR: failed to set broadcast socket option");
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(broadcast));
    puts("INFO: client can now receive messages");

    /* Construct bind structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast Address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_ANY;       /* Any incoming interface */
    broadcastAddr.sin_port = 0;                       /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
    {
        printErrorAndExit("bind() failed");
    }
    puts("INFO: bind complete");

    /* Receive a single datagram from the server */
    while (1)
    {
        char recvString[MAXSTRINGLENGTH + 1]; /* Buffer for received string */
        int recvStringLen = recvfrom(sock, recvString, MAXSTRINGLENGTH, 0, NULL, 0);
        if (recvStringLen < 0)
        {
            printErrorAndExit("recvfrom() failed");
        }
        puts("INFO: received string");

        recvString[recvStringLen] = '\0';
        printf("Received: %s\n", recvString); /* Print the received string */
    }
    close(sock);
    exit(0);
}