#ifndef FORKED_SERVER
#define FORKED_SERVER

#include <netinet/in.h> // structure for storing address information
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>

#include "calculationProcessAPI.h"
#include "fileTransfer.h"

int forkedServer(int *clientSocket);

#endif