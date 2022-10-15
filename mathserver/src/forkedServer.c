#include "../include/forkedServer.h"
#include <unistd.h>

int forkedServer(int *clientSocket) {
  char sendMsg[255] =
      "Welcome to mathserver API\n"
      "Options:\n"
      "    matinv -n [size of matrix] -I [rand || fast]\n"
      "    kmeans -k [number of clusters] -f [local file to read data from]\n"
      "-------------------------\n"
      "When finished write [done]\n";
  char recvData[255];
  send(*clientSocket, sendMsg, sizeof(sendMsg), 0);
  while (1) {
    memset(recvData, 0, sizeof(recvData));
    if (recv(*clientSocket, recvData, sizeof(recvData), 0) == 0) {
      printf("Client disconnected from process: %d\n", getpid());
      break;
    }
    if (strcmp(recvData, "done\n") == 0) {
      printf("Client disconnected from process: %d\n", getpid());
      break;
    }
    int mode = getMode(recvData);
    if (initCalculation(recvData, mode, *clientSocket) == 1)
    {
      printf("User (%d) input illegal code\n", getpid());
      break;
    }

  }
  close(*clientSocket);
  return 1;
}