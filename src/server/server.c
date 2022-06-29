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

struct addrinfo *constructServerAddress(char *port)
{
    // Construct the server address structure
    struct addrinfo addressCriteria;                      // Criteria for address
    memset(&addressCriteria, 0, sizeof(addressCriteria)); // Zero out structure
    addressCriteria.ai_family = AF_INET;                  // IPV4
    addressCriteria.ai_flags = AI_PASSIVE;                // Accept on any address/port
    addressCriteria.ai_socktype = SOCK_DGRAM;             // Only datagram socket
    addressCriteria.ai_protocol = IPPROTO_UDP;            // Only UDP socket

    struct addrinfo *serverAddress; // List of server addresses
    int rtnVal = getaddrinfo(NULL, port, &addressCriteria, &serverAddress);
    if (rtnVal != 0)
        printErrorAndExit("getaddrinfo() failed");

    return serverAddress;
}

int setupUdpSocket(struct addrinfo *serverAddress) {
    // Create socket for incoming connections
    int serverSocket = socket(serverAddress->ai_family, serverAddress->ai_socktype,
                      serverAddress->ai_protocol);
    if (serverSocket < 0)
        printErrorAndExit("socket() failed");

    // Bind to the local address
    if (bind(serverSocket, serverAddress->ai_addr, serverAddress->ai_addrlen) < 0)
        printErrorAndExit("bind() failed");

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(serverAddress);

    return serverSocket;
}

void setBroadcastOption(int serverUdpSocket, char *port, int broadcast, struct sockaddr_storage* destineAddressStorage) {
    int setBroadcast = setsockopt(serverUdpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (setBroadcast < 0) {
        printErrorAndExit("ERROR: failed to set broadcast socket option");
    }

    in_port_t serverPort = htons((in_port_t) atoi(port));

    struct sockaddr_in *clientAddress = (struct sockaddr_in *) &destineAddressStorage;
    clientAddress->sin_family = AF_INET;
    clientAddress->sin_port = serverPort;

    if (broadcast == 1) {
        clientAddress->sin_addr.s_addr = INADDR_BROADCAST;
    } else {
        clientAddress->sin_addr.s_addr = INADDR_ANY;
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

    puts("INFO: Initializing the server.");

    // to generate a random number later on
    srand(time(NULL));

    struct addrinfo *serverAddress = constructServerAddress(port);
    int serverUdpSocket = setupUdpSocket(serverAddress);
    puts("INFO: accepting client connections");

    while (1)
    {
        struct sockaddr_storage destineAddressStorage;
        memset(&destineAddressStorage, 0, sizeof(destineAddressStorage));
        // Set Length of client address structure (in-out parameter)
        //socklen_t destineAddressLength = sizeof(destineAddressStorage);

        //struct sockaddr *clientAddress = (struct sockaddr *)&destineAddressStorage;
        size_t addressSize = sizeof(struct sockaddr_in);

        // Block until receive message from a client
        //char buffer[MAXSTRINGLENGTH]; // I/O buffer

        /*
        // Size of received message
        ssize_t numberOfBytesReceived = recvfrom(serverUdpSocket, buffer, MAXSTRINGLENGTH, 0, clientAddress, &destineAddressLength);
        if (numberOfBytesReceived < 0)
        {
            printErrorAndExit("ERROR: recvfrom failed");
        }
        */

        ssize_t numBytesSent;

        /*
        // Send received datagram back to the client
        ssize_t numBytesSent = sendto(serverUdpSocket, buffer, MAXSTRINGLENGTH, 0, clientAddress, addressSize);
        if (numBytesSent < 0) {
            printErrorAndExit("sendto() failed");
        }
        */

        setBroadcastOption(serverUdpSocket, port, 1, &destineAddressStorage);
        char *echoString = NULL; // create new message
        size_t echoStringLen;

        printf("> ");
        getline(&echoString, &echoStringLen, stdin); // get the message from user input

        numBytesSent = sendto(serverUdpSocket, echoString, echoStringLen, 0, (struct sockaddr *)&destineAddressStorage, addressSize);
        if (numBytesSent < 0) {
            printErrorAndExit("sendto() failed)");
        }
    }
}