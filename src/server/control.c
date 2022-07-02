#include "control.h"
#include "utils.h"

int connectedEquipments[MAX_EQUIPMENTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int numberConnectedEquipments = 0;

struct ConnectedEquipment equipments[MAX_EQUIPMENTS] = {
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1},
    {-1}};

int getNextEquipmentId()
{
    for (int i = 0; i < MAX_EQUIPMENTS; i++)
    {
        if (connectedEquipments[i] == 0)
        {
            return i;
        }
    }

    return -1;
}

void setEquipmentAsConnected(int equipmentId)
{
    if (equipmentId < 0 || equipmentId >= MAX_EQUIPMENTS)
        return;
    connectedEquipments[equipmentId - 1] = 1;
}

int alreadyReachedMaxNumberOfConnections()
{
    return numberConnectedEquipments >= MAX_EQUIPMENTS;
}

int newConnection(struct sockaddr_in equipmentAddress)
{
    int equipmentIdInArray = getNextEquipmentId();
    equipments[equipmentIdInArray].equipmentId = equipmentIdInArray + 1;
    equipments[equipmentIdInArray].equipmentAddress = equipmentAddress;

    connectedEquipments[equipmentIdInArray] = 1;

    numberConnectedEquipments++;

    return equipments[equipmentIdInArray].equipmentId;
}

void removeConnection(int equipmentId)
{
    int equipmentIdInArray = equipmentId - 1;

    equipments[equipmentIdInArray].equipmentId = -1;

    numberConnectedEquipments--;

    connectedEquipments[equipmentIdInArray] = 0;
}

int getEquipment(struct sockaddr_in equipmentAddress)
{
    int equipmentId = -1;
    if (numberConnectedEquipments > 0)
    {
        for (int i = 0; i < MAX_EQUIPMENTS; i++)
        {
            if (connectedEquipments[i] == 0)
                continue;
            if (
                equipments[i].equipmentAddress.sin_addr.s_addr == equipmentAddress.sin_addr.s_addr &&
                equipments[i].equipmentAddress.sin_port == equipmentAddress.sin_port)
            {
                equipmentId = i + 1;
                break;
            }
        }
    }

    return equipmentId;
}

struct ConnectedEquipment *getEquipmentById(int equipmentId)
{
    if (equipmentId <=0 || equipmentId > MAX_EQUIPMENTS)
        return NULL;

    int equipmentIdOnArray = equipmentId - 1;
    if (connectedEquipments[equipmentIdOnArray] == 1) {
        return &equipments[equipmentIdOnArray];
    } else {
        return NULL;
    }
}

int equipmentExist(int equipmentId)
{
    if (getEquipmentById(equipmentId) == NULL)
        return -1;

    return 1;
}

char *listConnectedEquipmentsAsString()
{
    char equipmentString[5] = "";
    memset(equipmentString, 0, sizeof(equipmentString));
    char output[MAXSTRINGLENGTH] = "";
    memset(output, 0, sizeof(output));

    for (int i = 0; i < MAX_EQUIPMENTS; i++)
    {
        if (i > 0) {
            strcat(output, SPLITTER);
        }
        if (connectedEquipments[i] == 1)
        {
            int equipmentId = i + 1;
            char *zero = "";
            if (equipmentId < 10)
            {
                zero = "0";
            }
            sprintf(equipmentString, "%s%i", zero, equipmentId);
            strcat(output, equipmentString);
        }
    }

    char *connectedEquipmentsString = output;
    return connectedEquipmentsString;
}

struct sockaddr_in getEquipmentAddress(int equipmentId)
{
    return getEquipmentById(equipmentId)->equipmentAddress;
}
