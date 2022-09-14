#include "../include/threadHandler.h"

// Creates a thread for the new client and then detaches it from the main-thread
void newRequest(int clientSocket, int calculationType, int *data[])
{

    struct threadParam *parameters = malloc(sizeof(parameters));
    pthread_t *newCLientThread = malloc(sizeof(newCLientThread));
    parameters->calculationType = calculationType;
    parameters->clientSocket = clientSocket;
    parameters->data = data;

    pthread_create(newCLientThread, NULL, threadInit, (void*)parameters);
    pthread_detach(*newCLientThread);
}