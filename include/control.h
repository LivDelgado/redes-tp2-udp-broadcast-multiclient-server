#ifndef CONTROL
#define CONTROL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_EQUIPMENTS 15

struct ConnectedEquipment {
    int equipmentId;
    struct sockaddr_in *equipmentAddress;
};

//
// PRIVATE METHODS
//
int getNextEquipmentId();
void setEquipmentAsConnected(int equipmentId);

//
// PUBLIC METHODS
//

// check if we already reached the maximum number of available connections
int alreadyReachedMaxNumberOfConnections();

// creates new connection, by setting a new ConnectedEquipment and adding it to the global variables
int newConnection(struct sockaddr_in *equipmentAddress);

// removes equipment connection
void removeConnection(int equipmentId);

// get equipment id if we have connected it before. if not, return -1.
int getEquipment(struct sockaddr_in *equipmentAddress);

#endif