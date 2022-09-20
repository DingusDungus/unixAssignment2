#include "../include/mainServer.h"

void printHelp()
{
    printf(
        "-Mathserver v.1.0-\n   -h // Prints out server options and their functionality\n   -p [port] // Allows you to give a custom port to the server\n   -d // Option to run server as a daemon\n   -s [muxbasic || muxscale] // Multiplexing options\n");
}

int mainServer(int *clientSocket, struct options *serverOpts)
{
    if (serverOpts->helpRequested)
    {
        printHelp();
        return 1;
    }
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
        res = muxBasic(clientSocket, servAddr, servSockD, serverOpts->port);
        if (res != 0)
        {
            return res;
        }
    }

    return 0;
}
