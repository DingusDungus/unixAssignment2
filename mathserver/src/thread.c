#include "../include/thread.h"

void *threadInit(void *threadparam) {
  char serMsg[255] = "Message from the server-thread to the "
                     "client \'Hello Cunt\' ";

  struct threadParam *parameters = (struct threadParam *)threadparam;
  // Client socket used to send finished data
  int socket = parameters->clientSocket;
  // Type of calculation (i.e., matrix inversion or k-means)
  int calcType = parameters->calculationType;
  // Data used for calculation
  int **threadData = parameters->data;

  int **result = threadHandle(calcType, threadData);
  send(socket, result, sizeof(result), 0);
  send(socket, serMsg, sizeof(serMsg), 0);
  threadKamikaze();
  return NULL;
}

int **threadHandle(int calculationType, int *data[]) {
  if (calculationType == 0) {
    return matrixInteversionAlgorithm(data);
  } else {
    return kMeansAlgorithm(data);
  }
  return NULL;
}

int **matrixInteversionAlgorithm(int *data[]) { return NULL; }

int **kMeansAlgorithm(int *data[]) { return NULL; }

void threadKamikaze() {
  // Thread kills itself when job is done
  pthread_exit(NULL);
}
