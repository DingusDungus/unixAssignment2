#include "../include/mainServer.h"

int mainServer(int *clientSocket, struct options *serverOpts)
{
    // create server socket
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    // string store data to send to client

    // define server address
    struct sockaddr_in servAddr;
    servAddr.sin_port = htons(serverOpts->port);
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res = 0;

    if (serverOpts->fork)
    {
        res = forkStrategy(clientSocket, servAddr, servSockD, serverOpts->port);
        if (res != 0)
        {
            return res;
        }
    }
    else if (serverOpts->muxbasic)
    {
        res = muxBasic(servAddr, servSockD, serverOpts->port);
        if (res != 0)
        {
            return res;
        }
    }

    return 0;
}
