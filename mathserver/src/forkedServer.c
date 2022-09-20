#include "../include/forkedServer.h"

int forkedServer(int *clientSocket)
{
    char strData[255];
    char serMsg[255] = "Message from the server-child-process to the "
                       "client \'Hello Cunt\' ";
    send(*clientSocket, serMsg, sizeof(serMsg), 0);
    recv(*clientSocket, strData, sizeof(strData), 0);
    printf("Message: %s", strData);

    return 1;
}