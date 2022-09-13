#include "../include/threadHandler.h"

void initThreadHandler(struct threadHandler handler)
{
    handler.arraySize = 0;
    handler.currentMaxSize = 10;
    handler.threads = malloc(sizeof(pthread_t)*10);
}