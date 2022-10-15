#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <unistd.h>

#include "./mathserver/include/fileTransfer.h"
#include "./mathserver/include/modeDefinitions.h"
#include "./mathserver/include/clientArgsParsing.h"
#include "mathserver/include/clientOptions.h"

void getFileName(char *fileName, int nr, char *clientNr)
{
  char nrString[100];
  char fileExtension[100] = ".txt";
  char extendedFileName[100] = "_soln";
  sprintf(nrString, "%d", nr);
  strcat(fileName, clientNr);
  strcat(fileName, extendedFileName);
  strcat(fileName, nrString);
  strcat(fileName, fileExtension);
}

int main(int argc, char const *argv[]) {
  struct options clientOptions;
  cparseArgs(argv, argc, &clientOptions);
  // Converts ip from argv to ip for computer
  struct in_addr SERVER_ADDRESS;
  int result = inet_pton(AF_INET, clientOptions.address, &SERVER_ADDRESS);
  if (result != 1) {
    printf("Error; IP-address is invalid!\n");
    return -1;
  }
  // Create socket for client
  int sockD = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in servAddr;

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(clientOptions.port);
  servAddr.sin_addr.s_addr = SERVER_ADDRESS.s_addr;
  printf("Connecting to %d\n", SERVER_ADDRESS.s_addr);

  int connectStatus =
      connect(sockD, (struct sockaddr *)&servAddr, sizeof(servAddr));

  if (connectStatus == -1) {
    printf("Error...\n");
  } else {
    int res;
    char sendData[255] = "";
    char *temp = NULL;
    char recvData[1024];
    recv(sockD, recvData, sizeof(recvData), 0);
    printf("%s\n", recvData);
    int matinvResCount = 0;
    int kmeansResCount = 0;
    char clientNr[100];
    recvPid(sockD, clientNr);
    while (1) {
      memset(sendData, 0, sizeof(sendData));

      fflush(stdin);
      size_t len = 0;
      printf("Command: ");
      getline(&temp, &len, stdin);
      strcpy(sendData, temp);
      if (strcmp(sendData, "done\n") == 0) {
        break;
      }
      int mode = getMode(sendData);
      if (mode == UNACCEPTABLE_INPUT) {
        printf("Error; No command of such syntax found! (acceptable: matinv, "
               "kmeans)\n");
        continue;
      }
      if ((res = send(sockD, sendData, sizeof(sendData), 0)) == -1) {
        printf("Failed to send\n");
      }
      if (mode == KMEANS) {
        char *inputfile = getFile(sendData);
        char outFile[100] = "kmeans_client";
        kmeansResCount++;
        getFileName(outFile, matinvResCount, clientNr);
        transferFile(sockD, 4096, inputfile);
        recvFile(sockD, outFile, "w");
        printf("Received the solution: %s\n", outFile);
      } else {
        char file[100] = "matinv_client";
        matinvResCount++;
        getFileName(file, matinvResCount, clientNr);
        recvFile(sockD, file, "w");
        printf("Received the solution: %s\n", file);
      }
    }
    send(sockD, sendData, sizeof(sendData), 0);
  }

  return 0;
}
