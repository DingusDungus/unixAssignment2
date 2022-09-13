#include "../include/thread.h"

void *threadInit(void *clientSocket, void *calculationT, void* data)
{
    char serMsg[255] = "Message from the server-thread to the "
                       "client \'Hello Cunt\' ";
    // Client socket used to send finished data
    int socket = (int)clientSocket;
    // Type of calculation (i.e., matrix inversion or k-means)
    int calcType = (int)calculationT;
    // Data used for calculation
    int threadData = (int**)data;

    int result = threadHandle(calcType, threadData);
    send(clientSocket, serMsg, sizeof(serMsg), 0);
    threadKamikaze();
}

int **threadHandle(int calculationType, int *data[])
{
    if (calculationType == 0)
    {
        return matrixInteversionAlgorithm(data);
    }
    else
    {
        return kMeansAlgorithm(data);
    }

}

int **matrixInteversionAlgorithm(int *data[])
{

}

int **kMeansAlgorithm(int *data[])
{

}

void threadKamikaze()
{
    // Thread kills itself when job is done
    pthread_exit(NULL);
}

