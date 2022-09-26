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
            return forkedServer(clientSocket);
        }
    }
    printf("Client fork starting!; pid: %d\n", getpid());
    return 0;
}

int muxBasic(struct sockaddr_in servAddr, int servSockD, int port)
{
    // Muxing variables
    struct pollfd fds[200];
    int fdsArraySize = 1;

    // Networking variables
    int clientSocket = 0;
    struct sockaddr_in clientAddr;
    int on = 1;
    int res;

    // Buffers
    char clientBuf[1024];
    char sendBuf[1024] = "Hello client";
    memset(&clientBuf, 0, sizeof(clientBuf));

    // Setup
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ioctl(servSockD, FIONBIO, (char *)&on);
    memset(fds, 0, sizeof(fds));

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
        exit(2);
    }

    // Setup poll
    fds[0].fd = servSockD;
    fds[0].events = POLLIN;

    printf("Multiplexed server started!\n");
    printf("Listening on %s (binary format: %d) with port %d!\n", inet_ntoa(servAddr.sin_addr), servAddr.sin_addr.s_addr, port);

    while (1)
    {
        clientSocket = 0;
        if ((res = poll(fds, fdsArraySize, -1)) == -1)
        {
            perror("Error; Select from server failed!\n");
            exit(1);
        }

        printf("Going through filedescriptors!, fds array size: %d\n", fdsArraySize);
        int size = fdsArraySize;
        printf("Server socket ready!\n");
        while (clientSocket != -1)
        {
            clientSocket = accept(servSockD, NULL, NULL);
            if (clientSocket != -1)
            {
                printf("Client accepted! socket: %d\n", clientSocket);
                fds[fdsArraySize].fd = clientSocket;
                fds[fdsArraySize].events = POLLIN;
                fdsArraySize++;
            }
        }
        printf("Looping through descriptors!\n");
        for (int i = 1; i < size; i++)
        {
            printf("Iteration: %d, sock %d, servsock: %d\n", i, fds[i].fd, servSockD);
            if (fds[i].revents == 0)
            {
                // If there are no events on this descriptor, continue to next
                continue;
            }
            if (fds[i].fd == -1)
            {
                // This descriptor is marked for removal
                continue;
            }
            if (fds[i].fd != servSockD && fds[i].events == POLLIN)
            {
                printf("Client ready to be read!\n");
                if (res = recv(fds[i].fd, clientBuf, sizeof(clientBuf), 0) == -1)
                {
                    continue;
                }
                printf("Message: %s", clientBuf);
                res = send(fds[i].fd, sendBuf, sizeof(sendBuf), 0);
                // Closes connection
                close(fds[i].fd);
                fds[i].fd = -1;
            }
        }
    }
}
