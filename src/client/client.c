#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXSTRINGLENGTH 255

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printErrorAndExit("ERROR: Invalid arguments. To run the client: client <server address> <server port>");
    }

    char *serverIpAddress = argv[1]; // first argument is server address
    char *serverPort = argv[2];      // second argument is server port

    //
    // ADDRESS - Get foreign address for server:
    //
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

    //
    // SOCKET - Create datagram socket
    //
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
    if (sock < 0)
    {
        printErrorAndExit("ERROR: socket() failed");
    }

    while (1)
    {
        char *echoString = NULL; // create new message
        size_t echoStringLen;

        printf("> ");
        getline(&echoString, &echoStringLen, stdin); // get the message from user input

        //
        // SEND - send the string to the server
        //
        ssize_t numBytes = sendto(sock, echoString, echoStringLen, 0, servAddr->ai_addr, servAddr->ai_addrlen);
        if (numBytes < 0)
        {
            printErrorAndExit("ERROR: failed to send to the server");
        }
        else if (numBytes != echoStringLen)
        {
            printErrorAndExit("ERROR: sent unexpected number of bytes");
        }
        //
        // RECEIVE - get response from server
        //
        struct sockaddr_storage fromAddr; // Source address of server
        // Set length of from address structure (in-out parameter)
        socklen_t fromAddrLen = sizeof(fromAddr);
        char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
        numBytes = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
        if (numBytes < 0)
        {
            printErrorAndExit("ERROR: recvfrom() failed");
        }

        buffer[echoStringLen] = '\0';
        printf("Received: %s\n", buffer);
    }
}