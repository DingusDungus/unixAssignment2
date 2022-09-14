#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <pthread.h>

#include "include/threadHandler.h"

int main(int argc, char const *argv[])
{
    // get port for server
    if (argc > 2)
    {
        printf("Error; No port given!\n");
        return -1;
    }
    const int PORT_NUMBER = atoi(argv[1]);
    printf("Running on address %d and port %d", INADDR_ANY, PORT_NUMBER);

    // create server socket
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    // string store data to send to client

    // define server address
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT_NUMBER);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to the specified IP and port
    bind(servSockD, (struct sockaddr *)&servAddr,
         sizeof(servAddr));

    // listen for connections
    listen(servSockD, 1);
    while (1)
    {
        // integer to hold client socket.
        int clientSocket = accept(servSockD, NULL, NULL);
        int **data = malloc(69);
        int calculationType = 0;
        // Create thread to handle client
        newRequest(clientSocket, calculationType, data);
    }

    return 0;
}