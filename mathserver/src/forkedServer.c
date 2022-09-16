#include "../include/forkedServer.h"

int forkedServer(int *clientSocket)
{
    char serMsg[255] = "Message from the server-child-process to the "
                       "client \'Hello Cunt\' ";
    send(*clientSocket, serMsg, sizeof(serMsg), 0);

    return 1;
}