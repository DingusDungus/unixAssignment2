#include "../include/forkedServer.h"
#include <unistd.h>

int forkedServer(int *clientSocket) {
  int matinvSol = 0;
  int kmeansSol = 0;
  char sendMsg[255] =
      "Welcome to mathserver API\n"
      "Options:\n"
      "    matinv -n [size of matrix] -I [rand || fast]\n"
      "    kmeans -k [number of clusters] -f [local file to read data from]\n"
      "    example input: kmeans -k 9 -f data.txt\n"
      "-------------------------\n"
      "When finished write [done]\n";
  char recvData[255];
  send(*clientSocket, sendMsg, sizeof(sendMsg), 0);
  sendPid(*clientSocket);
  while (1) {
    memset(recvData, 0, sizeof(recvData));
    if (recv(*clientSocket, recvData, sizeof(recvData), 0) == 0) {
      printf("Client disconnected from process: %d\n", getpid());
      break;
    }
    printf("Client %d commanded %s", getpid(), recvData);
    if (strcmp(recvData, "done\n") == 0) {
      printf("Client disconnected from process: %d\n", getpid());
      break;
    }
    int mode = getMode(recvData);
    if (mode == KMEANS) {
      kmeansSol++;
      if (initCalculation(recvData, mode, *clientSocket, kmeansSol) == 1) {
        printf("Client (%d) input illegal code\n", getpid());
        transferFile(*clientSocket, 0, 0, true);
      }
    }
    else if (mode == MATINV) {
      matinvSol++;
      if (initCalculation(recvData, mode, *clientSocket, matinvSol) == 1) {
        printf("Client (%d) input illegal code\n", getpid());
        transferFile(*clientSocket, 0, 0, true);
      }
    }
  }
  close(*clientSocket);
  return 1;
}