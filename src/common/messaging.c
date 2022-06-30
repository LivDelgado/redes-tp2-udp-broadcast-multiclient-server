#include "messaging.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *SPLITTER = " ";

struct MessageFields existingMessages[EXISTING_MESSAGES] = {
    {0, 0, 0},
    {1, 0, 0},
    {1, 0, 0},
    {0, 0, 1},
    {1, 1, 0},
    {1, 1, 1},
    {0, 1, 1},
    {0, 1, 1}};

struct Message structureMessage(char *originalMessage)
{
    char message[MAXSTRINGLENGTH] = "";
    strcpy(message, originalMessage); // copy to avoid changing the message string

    struct Message structuredMessage = {0, 0, 0, NULL};
    int counter = 0;
    puts("original message");
    puts(originalMessage);

    puts("let's do that");
    char payload[MAXSTRINGLENGTH] = "";

    // first word is the id - it exists for every message
    char *word = strtok(message, SPLITTER);
    if (word == NULL)
    {
        puts("word is null.");
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
            puts("counter 2");

            if (fields->hasSourceId == 1)
            {
                puts("hasSourceId");
                structuredMessage.sourceId = atoi(word);
            }
            else if (fields->hasDestineId == 1)
            {
                puts("hasDestineId");
                structuredMessage.destineId = atoi(word);
            }
            else if (fields->hasPayload == 1)
            {
                puts("hasPayload");
                strcat(payload, word);

                while (word != NULL)
                {
                    strcat(payload, SPLITTER);
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, word);
                }
                strcat(payload, word);
                break;
            }
            else
            {
                puts("else!");
                break;
            }
        }
        else if (counter == 3)
        {
            puts("counter 3");
            if (fields->hasSourceId == 1 && fields->hasDestineId == 1)
            {
                puts("hasDestineId and source id");
                structuredMessage.destineId = atoi(word);
            }
            else if (fields->hasPayload == 1)
            {
                puts("hasPayload");
                puts(word);
                strcat(payload, word);
                puts("passed strcat");

                puts(payload);

                while (word != NULL)
                {
                    strcat(payload, SPLITTER);
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, word);
                }
                strcat(payload, word);
                break;
            }
            else
            {
                puts("else!");
                break;
            }
        }
        else
        {
            puts("bigger counter!");
            if (fields->hasPayload == 1)
            {
                puts("hasPayload");
                strcat(payload, word);

                while (word != NULL)
                {
                    strcat(payload, SPLITTER);
                    word = strtok(NULL, " "); // takes next word
                    if (word == NULL)
                    {
                        break;
                    }
                    strcat(payload, word);
                }
                strcat(payload, word);
                break;
            }
            else
            {
                break;
            }
        }
    }

    return structuredMessage;
}

int isErrorMessage(struct Message message)
{
    return (message.messageId == 7);
}

char *getErrorMessage(struct Message message)
{
    if (message.messageId != 7)
    {
        return NULL;
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