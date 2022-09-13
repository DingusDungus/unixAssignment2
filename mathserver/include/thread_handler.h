#ifndef THREAD_HANDLER
#define THREAD_HANDLER

#include <pthread.h>

struct threadHandler
{
    pthread_t *threads;
    int arraySize;
    int currentMaxSize;
    void newThread(int clientSocket, int calculationType, int *data[]);
    void jointhreads();
    threadHandler();
};

#endif