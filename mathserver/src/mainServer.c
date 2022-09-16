#include "../include/mainServer.h"

int mainServer(int port, int address, int *clientSocket, int *pid)
{

    // create server socket
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    // string store data to send to client

    // define server address
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to the specified IP and port
    bind(servSockD, (struct sockaddr *)&servAddr, sizeof(servAddr));

    // listen for connections
    listen(servSockD, 1);
    while (1)
    {
        // integer to hold client socket.
        *clientSocket = accept(servSockD, NULL, NULL);
        *pid = fork();
    }

    return 0;
}
