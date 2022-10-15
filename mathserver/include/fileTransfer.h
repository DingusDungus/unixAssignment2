#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "./modeDefinitions.h"

// Takes file after -f flag
char *getFile(char *argString);

// Gets mode from client command, either matinv, kmeans or not acceptable
// Returns 1 if mode is matinv, 2 if kmeans, 0 if not acceptable
// NOTE: command buffer must be bigger than size 10
int getMode(char *command);

// Send client pid to client
int sendPid(int socket);

// Recieves pid for client
int recvPid(int socket, char *pid);

// Gets total useful data from a buffer, disregarding trash-values
int getWorkSize(char *buf, int size);

// Copies sub-string from a buffer given an interval
int copy2chunk(char *chunk, char *resBuf, int bufSize, const int MAX_CHUNK, int iteration, int chunkNr);

// Sends chunks given by transferFile to send to recvFile
void sendChunks(FILE *fs, char *resBuf, int bufSize, const int MAX_CHUNK, int socket);

// Transfers file to socket in chunks
int transferFile(int socket, int chunkSize, char* fileName);

// Recieves file from socket in chunks
int recvFile(int socket, char *fileName, char *fileMode);

#endif