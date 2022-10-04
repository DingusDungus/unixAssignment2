#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

void printData(char *recvData, int size)
{
  for (int i = 0; i < 10; i++)
  {
    if (recvData[i] != '\0')
    {
      printf("%s", recvData[i]);
    }
    else
    {
      printf("Null");
    }
  }
}

// Takes in a data buf and a dest buf with value NULL, also size of data buf
// This is needed for this project as the buffert recieves a lot of noise from the socket
int sizeOfData(char *data, int size)
{
  int workSize = 0; // Size of buf which has meaningful data in it
  for (int i = 0; data[i] != '\0' && i < size; i++)
  {
    workSize++;
  }
  return workSize;
}

void recvFile(int sockD)
{
  int chunkSize;
  if (recv(sockD, &chunkSize, sizeof(chunkSize), 0) == -1)
  {
    printf("Error reading results\n");
    return;
  }
  char filename[] = "clientResultFile.txt";
  FILE *resFile = fopen(filename, "a");
  char *recvData = (char *)malloc(sizeof(char) * chunkSize);
  int res = 0;
  // Recieves chunks from server and appends them to result file
  while (strcmp(recvData, "done"))
  {
    memset(recvData, 0, chunkSize);
    if ((res = recv(sockD, recvData, sizeof(char) * chunkSize, 0)) == -1)
    {
      printf("Error reading results\n");
    }
    int size = sizeOfData(recvData, chunkSize);
    fwrite(recvData, 1, sizeof(char) * size, resFile);
  }
  printf("Results appended to file [%s]\n", filename);

  fclose(resFile);
  free(recvData);
}

int main(int argc, char const *argv[])
{
  // get port for server
  if (argc > 3)
  {
    printf("Error; Not enough arguments (require port and IP)!\n");
    return -1;
  }
  const int PORT_NUMBER = atoi(argv[1]);

  // Get IP for server
  struct in_addr SERVER_ADDRESS;
  int result = inet_pton(AF_INET, argv[2], &SERVER_ADDRESS);
  if (result != 1)
  {
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

  if (connectStatus == -1)
  {
    printf("Error...\n");
  }
  else
  {
    int res;
    char sendData[255] = "";
    char *temp = NULL;
    char recvData[1024];
    int dataIteration = 0;
    recv(sockD, recvData, sizeof(recvData), 0);
    printf("%s\n", recvData);
    char filename[] = "resFile";

    while (1)
    {
      memset(sendData, 0, sizeof(sendData));

      fflush(stdin);
      size_t len = 0;
      ssize_t lineSize = 0;
      printf("Command: ");
      lineSize = getline(&temp, &len, stdin);
      strcpy(sendData, temp);
      if (strcmp(sendData, "done\n") == 0)
      {
        break;
      }

      if (res = send(sockD, sendData, sizeof(sendData), 0) == -1)
      {
        printf("Failed to send\n");
      }
      recvFile(sockD);
    }
    send(sockD, sendData, sizeof(sendData), 0);
  }

  return 0;
}
