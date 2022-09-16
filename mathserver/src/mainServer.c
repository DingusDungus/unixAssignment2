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
    printf("Listening on %d!\n", INADDR_ANY);
    while (1 && *pid != 0)
    {
        // integer to hold client socket.
        *clientSocket = accept(servSockD, NULL, NULL);
        printf("Accepted client!\n");
        *pid = fork();
        printf("Child process pid: %d\n", *pid);
    }

    return 0;
}
