#ifndef MESSAGING
#define MESSAGING

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define EXISTING_MESSAGES 8

struct MessageFields
{
    int hasSourceId;
    int hasDestineId;
    int hasPayload;
};

struct Message
{
    int messageId;
    int sourceId;
    int destineId;
    char *payload;
};

typedef enum {
    REQ_ADD = 1,
    REQ_REM,
    RES_ADD,
    RES_LIST,
    REQ_INF,
    RES_INF,
    ERROR,
    OK
} MessageType;

struct Message structureMessage(char *originalMessage);
struct MessageFields *initializeMessageFields();

int isErrorMessage(struct Message message);
char *getErrorMessage(struct Message message);

#endif