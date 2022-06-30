#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>    /* for close() */
#include <arpa/inet.h> /* for sockaddr_in */
#include <pthread.h>

#define MAXSTRINGLENGTH 255
#define MAXTHREADS 15

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
}

int numberOfThreads = 0;

struct ThreadArgs
{
    int serverSocket;
    struct sockaddr_in clientAddrIn;
    socklen_t clientAddrLen;
    char buffer[MAXSTRINGLENGTH];
};

void receiveMessage(int serverSocket, struct ThreadArgs *threadArgs)
{
    int numBytesRcvd = recvfrom(
        serverSocket, threadArgs->buffer, MAXSTRINGLENGTH, 0, (struct sockaddr *)&threadArgs->clientAddrIn, &threadArgs->clientAddrLen);
    if (numBytesRcvd < 0)
    {
        printErrorAndExit("ERROR: recvfrom failed");
    }
    puts("INFO: client sent the first message. Creating new channel to communicate!");
}

void sendMessage(char *response, int serverSocket, struct ThreadArgs *threadArgs)
{
    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(threadArgs->serverSocket, response, MAXSTRINGLENGTH, 0,
                                  (struct sockaddr *)&threadArgs->clientAddrIn, threadArgs->clientAddrLen);
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed");
    }
}

void *ThreadMain(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;

    int connection_id = threadArgs->clientAddrIn.sin_port;
    struct sockaddr_in from = threadArgs->clientAddrIn;
    char *ip = inet_ntoa(from.sin_addr);
    printf("INFO: created new thread to handle client request %s:%d.\n", ip, connection_id);

    sendMessage(threadArgs->buffer, threadArgs->serverSocket, threadArgs);

    free(threadArgs);
    numberOfThreads--;
    return NULL;
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

void receiveMessageAndRespond(int serverSocket, struct sockaddr_in clntAddr)
{
    puts("receiving messages!");
    // struct sockaddr_storage clntAddr; // Client address

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
    puts("INFO: received message from client. Responding!");

    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(serverSocket, buffer, MAXSTRINGLENGTH, 0,
                                  (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed");
    }
}

void createThreadToHandleReceivedMessage(pthread_t *threads, struct ThreadArgs *threadArgs)
{
    int threadStatus = pthread_create(&threads[numberOfThreads], NULL, ThreadMain, (void *)threadArgs);
    if (threadStatus != 0)
    {
        printErrorAndExit("ERROR: failed to create thread");
    }
    else
    {
        numberOfThreads++;
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

    //
    // THREADS
    //
    pthread_t threads[MAXTHREADS];

    createAddress(serverSocket, port);
    while (1)
    {
        // Create separate memory for client argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        threadArgs->serverSocket = serverSocket;
        threadArgs->clientAddrLen = sizeof(struct sockaddr_in);

        puts("INFO: waiting for the first contact from the client");

        receiveMessage(serverSocket, threadArgs);
        createThreadToHandleReceivedMessage(threads, threadArgs);
    }

    /*
    //
    // BROADCAST
    //
    char *sendString = "teste";
    struct sockaddr_in serverAddress = createBroadcastAddress(port);
    while (1) {
        sendMessageTo(serverAddress, serverSocket, sendString);
    }
    //
    //
    */

    /*
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
    */
}