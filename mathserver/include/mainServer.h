#ifndef MAIN_SERVER
#define MAIN_SERVER

#include <netinet/in.h> // structure for storing address information
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <unistd.h>

int mainServer(int port, int address, int *clientSocket, int *pid);


#endif