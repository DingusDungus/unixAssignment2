#ifndef THREAD_HANDLER
#define THREAD_HANDLER

#include <pthread.h>
#include "thread.h"

void newRequest(int clientSocket, int calculationType, int *data[]);

#endif