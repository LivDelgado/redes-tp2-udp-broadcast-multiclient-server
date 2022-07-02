#include "messaging.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *SPLITTER = " ";

struct MessageFields existingMessages[EXISTING_MESSAGES] = {
    {0, 0, 0}, // REQ_ADD
    {1, 0, 0}, // REQ_REM
    {0, 0, 1}, // RES_ADD
    {0, 0, 1}, // RES_LIST
    {1, 1, 0}, // REQ_INF
    {1, 1, 1}, // RES_INF
    {0, 1, 1}, // ERROR
    {0, 1, 1}  // OK
};

struct Message structureMessage(char *originalMessage)
{
    char message[MAXSTRINGLENGTH];
    memset(message, 0, sizeof(message));
    strcpy(message, originalMessage); // copy to avoid changing the message string

    struct Message structuredMessage = {0, 0, 0, NULL};
    int counter = 0;

    // first word is the id - it exists for every message
    char *word = strtok(message, SPLITTER);
    if (word == NULL)
    {
        return structuredMessage;
    }
    int messageId = atoi(word);
    if (messageId <= 0 || messageId > EXISTING_MESSAGES)
    {
        return structuredMessage;
    }

    structuredMessage.messageId = messageId;
    counter++;

    struct MessageFields *fields = &existingMessages[structuredMessage.messageId - 1];
    char payload[MAXSTRINGLENGTH];
    memset(payload, 0, sizeof(payload));

    while (word != NULL)
    {
        word = strtok(NULL, " "); // takes next word
        if (word == NULL)
        {
            break;
        }

        counter++;

        if (counter == 2)
        {
            if (fields->hasSourceId == 1)
            {
                structuredMessage.sourceId = atoi(word);
            }
            else if (fields->hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (fields->hasPayload == 1)
            {
                strcat(payload, word);
                while (word != NULL)
                {
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, SPLITTER);
                    strcat(payload, word);
                }

                break;
            }
            else
            {
                break;
            }
        }
        else if (counter == 3)
        {
            if (fields->hasSourceId == 1 && fields->hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (fields->hasPayload == 1)
            {
                strcat(payload, word);
                while (word != NULL)
                {
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, SPLITTER);
                    strcat(payload, word);
                }
                break;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (fields->hasPayload == 1)
            {
                strcat(payload, word);
                while (word != NULL)
                {
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, SPLITTER);
                    strcat(payload, word);
                }

                break;
            }
            else
            {
                break;
            }
        }
    }

    structuredMessage.payload = payload;
    return structuredMessage;
}

int isErrorMessage(struct Message message)
{
    return (message.messageId == 7) || (message.messageId == 0);
}

char *getErrorMessage(struct Message message)
{
    if (message.messageId != 7)
    {
        return "ERROR - invalid message";
    }

    int messageCode = atoi(message.payload);
    if (messageCode < 1 || messageCode > 5)
    {
        return NULL;
    }

    switch (messageCode)
    {
    case 1:
        return "Equipment not found";
        break;
    case 2:
        return "Source equipment not found";
        break;
    case 3:
        return "Target equipment not found";
        break;
    case 4:
        return "Equipment limit exceeded";
        break;
    }

    return NULL;
}