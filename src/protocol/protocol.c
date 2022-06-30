#include "protocol.h"

int createUdpSocket()
{
    /* Create a best-effort datagram socket using UDP */
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printErrorAndExit("socket() failed");
    }

    return sock;
}

void bindToBroadcasterServer(int clientSocket, char *serverPort)
{
    /* Construct bind structure */
    struct sockaddr_in broadcastAddr;                 /* Broadcast Address */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr)); /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;               /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_ANY;       /* Any incoming interface */
    broadcastAddr.sin_port = htons(atoi(serverPort)); /* Broadcast port */

    int broadcast = 1;
    int setBroadcast = setsockopt(clientSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (setBroadcast < 0)
    {
        printErrorAndExit("ERROR: failed to set broadcast socket option");
    }
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(broadcast));

    /* Bind to the broadcast port */
    if (bind(clientSocket, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
    {
        printErrorAndExit("bind() failed");
    }
}

char *receiveBroadcastMessage(int clientSocket)
{
    /* Receive a single datagram from the server */
    char recvString[MAXSTRINGLENGTH + 1]; /* Buffer for received string */
    int recvStringLen = recvfrom(clientSocket, recvString, MAXSTRINGLENGTH, 0, NULL, 0);
    if (recvStringLen < 0)
    {
        printErrorAndExit("recvfrom() failed");
    }

    recvString[recvStringLen] = '\0';
    char *output = recvString;
    return output;
}

struct addrinfo *getServerAddress(char *serverIpAddress, char *serverPort)
{
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

    return servAddr;
}

void sendMessageToServer(int clientSocket, char *message, struct addrinfo *servAddr)
{
    size_t messageLen = strlen(message);
    ssize_t numBytes = sendto(clientSocket, message, messageLen, 0, servAddr->ai_addr, servAddr->ai_addrlen);
    if (numBytes < 0)
    {
        printErrorAndExit("ERROR: failed to send to the server");
    }
    else if (numBytes != messageLen)
    {
        printErrorAndExit("ERROR: sent unexpected number of bytes");
    }

    puts("INFO: sent message to server");
}

char *receiveMessageFromServer(int clientSocket)
{
    puts("INFO: receiving message from server");

    char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
    ssize_t numBytes = recv(clientSocket, buffer, MAXSTRINGLENGTH, 0);
    if (numBytes < 0)
    {
        printErrorAndExit("ERROR: recvfrom() failed");
    }

    char *returnMessage = buffer;
    return returnMessage;
}


void receiveMessageAndRespond(int serverSocket, struct sockaddr_in clntAddr)
{
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

    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(serverSocket, buffer, MAXSTRINGLENGTH, 0,
                                  (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed");
    }
}


char *receiveMessage(int serverSocket, char *buffer, struct sockaddr_in *clientAddrIn, socklen_t clientAddrLen)
{
    int numBytesRcvd = recvfrom(
        serverSocket, buffer, MAXSTRINGLENGTH, 0, (struct sockaddr *)clientAddrIn, &clientAddrLen);
    if (numBytesRcvd < 0)
    {
        printErrorAndExit("ERROR: recvfrom failed");
    }
    
    char *output = buffer;
    return output;
}


void sendMessage(char *response, int serverSocket, struct sockaddr_in *clientAddrIn, socklen_t clientAddrLen)
{
    // Send received datagram back to the client
    ssize_t numBytesSent = sendto(serverSocket, response, MAXSTRINGLENGTH, 0, (struct sockaddr *)clientAddrIn, clientAddrLen);
    if (numBytesSent < 0)
    {
        printErrorAndExit("sendto() failed");
    }
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


