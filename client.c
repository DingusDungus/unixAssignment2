#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <unistd.h>

#include "./mathserver/include/fileTransfer.h"
#include "mathserver/include/modeDefinitions.h"

int main(int argc, char const *argv[]) {
  // get port for server
  if (argc > 3) {
    printf("Error; Not enough arguments (require port and IP)!\n");
    return -1;
  }
  const int PORT_NUMBER = atoi(argv[1]);

  // Get IP for server
  struct in_addr SERVER_ADDRESS;
  int result = inet_pton(AF_INET, argv[2], &SERVER_ADDRESS);
  if (result != 1) {
    printf("Error; IP-address is invalid!\n");
    return -1;
  }
  // Create socket for client
  int sockD = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in servAddr;

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT_NUMBER);
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
        char outFile[100] = "kmeansOutputFile.txt";
        transferFile(sockD, 4096, inputfile);
        recvFile(sockD, outFile, "w");
        printf("Kmeans results in: %s\n", outFile);
      } else {
        recvFile(sockD, "matinvOutputFile.txt", "a");
      }
    }
    send(sockD, sendData, sizeof(sendData), 0);
  }

  return 0;
}
