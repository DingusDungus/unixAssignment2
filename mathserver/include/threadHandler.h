#ifndef THREAD_HANDLER
#define THREAD_HANDLER

#include "thread.h"
#include <pthread.h>

void newRequest(int clientSocket, int calculationType, int *data[]);

#endif
