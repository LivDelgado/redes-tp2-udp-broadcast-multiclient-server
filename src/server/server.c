#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>

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

    puts("INFO: Initializing the server.");

    // to generate a random number later on
    srand(time(NULL));

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

    // Create socket for incoming connections
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                      servAddr->ai_protocol);
    if (sock < 0)
        printErrorAndExit("socket() failed");

    // Bind to the local address
    if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
        printErrorAndExit("bind() failed");

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    puts("INFO: accepting client connections");

    for (;;)
    {                                     // Run forever
        struct sockaddr_storage clntAddr; // Client address
        // Set Length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Block until receive message from a client
        char buffer[MAXSTRINGLENGTH]; // I/O buffer
        // Size of received message
        ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
                                        (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (numBytesRcvd < 0)
        {
            printErrorAndExit("ERROR: recvfrom failed");
        }

        puts("Handling new client!");

        // Send received datagram back to the client
        ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0,
                                      (struct sockaddr *)&clntAddr, sizeof(clntAddr));
        if (numBytesSent < 0)
            printErrorAndExit("sendto() failed)");
        else if (numBytesSent != numBytesRcvd)
            printErrorAndExit("sendto() sent unexpected number of bytes");
    }
}