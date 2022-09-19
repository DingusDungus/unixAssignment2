#ifndef MAIN_SERVER
#define MAIN_SERVER

#include <netinet/in.h> // structure for storing address information
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>

#include "serverOptions.h"

int mainServer(int *clientSocket, int *pid, struct options *serverOpts);

void printHelp();

#endif