#include "../include/strategies.h"

int forkStrategy(int *clientSocket, struct sockaddr_in servAddr, int servSockD, int port)
{
    // bind socket to the specified IP and port
    if (bind(servSockD, (struct sockaddr *)&servAddr, sizeof(servAddr)) != 0)
    {
        printf("Error; Failed to bind\n");
        return 2;
    }

    // listen for connections
    if (listen(servSockD, 1) != 0)
    {
        printf("Error; Failed to start listening!\n");
        return 2;
    }

    printf("Listening on %s (binary format: %d) with port %d!\n", inet_ntoa(servAddr.sin_addr), servAddr.sin_addr.s_addr, port);
    while (1)
    {
        // integer to hold client socket.
        *clientSocket = accept(servSockD, NULL, NULL);
        printf("Accepted client!\n");
        if (fork() == 0)
        {
            printf("Child process pid: %d\n", getpid());
            return 0;
        }
    }
    printf("Client fork starting!; pid: %d\n", getpid());
    forkedServer(clientSocket);
}

int muxBasic(int *clientSocket, struct sockaddr_in servAddr, int servSockD, int port)
{
}
