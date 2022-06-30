#include "messaging.h"
#include "utils.h"


static const char *SPLITTER = " ";

struct MessageFields *initializeMessageFields()
{
    struct MessageFields *messageFields = (struct MessageFields *)malloc(sizeof(struct MessageFields) * EXISTING_MESSAGES);

    struct MessageFields REQ_ADD, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK;

    REQ_ADD.hasSourceId = 0;
    REQ_ADD.hasDestineId = 0;
    REQ_ADD.hasPayload = 0;

    REQ_REM.hasSourceId = 1;
    REQ_REM.hasDestineId = 0;
    REQ_REM.hasPayload = 0;

    RES_ADD.hasSourceId = 0;
    RES_ADD.hasDestineId = 0;
    RES_ADD.hasPayload = 1;

    RES_LIST.hasSourceId = 0;
    RES_LIST.hasDestineId = 0;
    RES_LIST.hasPayload = 1;

    REQ_INF.hasSourceId = 1;
    REQ_INF.hasDestineId = 1;
    REQ_INF.hasPayload = 0;

    RES_INF.hasSourceId = 1;
    RES_INF.hasDestineId = 1;
    RES_INF.hasPayload = 1;

    ERROR.hasSourceId = 0;
    ERROR.hasDestineId = 1;
    ERROR.hasPayload = 1;

    OK.hasSourceId = 0;
    OK.hasDestineId = 1;
    OK.hasPayload = 1;

    messageFields = (struct MessageFields[EXISTING_MESSAGES]){REQ_ADD, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK};
    return messageFields;
}

struct Message structureMessage(char *originalMessage)
{
    char message[MAXSTRINGLENGTH] = "";
    strcpy(message, originalMessage); // copy to avoid changing the message string
    struct MessageFields *messageFields = initializeMessageFields();

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
    structuredMessage.payload = "";
    counter++;

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
            if (messageFields[structuredMessage.messageId - 1].hasSourceId == 1)
            {
                structuredMessage.sourceId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
                break;
            }
            else
            {
                break;
            }
        }
        else if (counter == 3)
        {
            if (messageFields[structuredMessage.messageId - 1].hasDestineId == 1)
            {
                structuredMessage.destineId = atoi(word);
            }
            else if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
                break;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (messageFields[structuredMessage.messageId - 1].hasPayload == 1)
            {
                strncat(word, message, strlen(message));
                strncpy(structuredMessage.payload, word, strlen(word));
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
