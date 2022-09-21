#include "../include/forkedServer.h"

int forkedServer(int *clientSocket)
{
    printf("Sending payload!");
    char serMsg[255] = "Message from the server-child-process to the "
                       "client \'Hello Cunt\' ";
    char strData[255];
    recv(*clientSocket, strData, sizeof(strData), 0);
    send(*clientSocket, serMsg, sizeof(serMsg), 0);
    printf("Message: %s", strData);

    return 1;
}