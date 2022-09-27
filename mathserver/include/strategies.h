#ifndef FORK_STRATEGY
#define FORK_STRATEGY

#include <netinet/in.h> // structure for storing address information
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/poll.h>

#include "forkedServer.h"

int forkStrategy(int *clientSocket, struct sockaddr_in servAddr, int servSockD, int port);

int muxBasic(struct sockaddr_in servAddr, int servSockD, int port);

#endif