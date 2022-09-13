#ifndef THREAD_HANDLER
#define THREAD_HANDLER

#include <pthread.h>
#include "thread.h"

struct threadHandler
{
    pthread_t *threads;
    int arraySize;
    int currentMaxSize;
};

void newThread(struct threadHandler handler, int clientSocket, int calculationType, int *data[]);
void jointhreads(struct threadHandler handler);
void initHandler(struct threadHandler handler);

#endif