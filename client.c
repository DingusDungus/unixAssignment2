#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

void getMatinvInp(char **argv, char *sendString)
{
  fflush(stdin);

  printf("Input for matinv:\n");
  printf("Size of array: ");
  scanf("%s", argv[2]);
  argv[1] = "-n";
  fflush(stdin);

  printf("Number of parallel threads: ");
  scanf("%s", argv[4]);
  argv[3] = "-P";
  fflush(stdin);

  printf("Type of generation [rand || fast]: ");
  scanf("%s", argv[6]);
  argv[5] = "-I";
  fflush(stdin);

  // copies into a sendstring
  int stringIndex = 0;
  for (int y = 0; y < 10; y++)
  {
    for (int x = 0; x < 100 && argv[y][x] != '\0' && argv[y][x] != '\n'; x++)
    {
      sendString[stringIndex] = argv[y][x];
      stringIndex++;
    }
    sendString[stringIndex] = ' ';
    stringIndex++;
  }
  sendString[stringIndex] = '\0';
  stringIndex++;
}

void printData(char **argv, int argc)
{
  for (int i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }
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
    char sendData[100] = "";
    char *temp = NULL;
    char recvData[1024];
    int dataIteration = 0;

    recv(sockD, recvData, sizeof(recvData), 0);
    printf("%s\n", recvData);

    fflush(stdin);
    size_t len = 0;
    ssize_t lineSize = 0;
    lineSize = getline(&temp, &len, stdin);
    strcpy(sendData, temp);
    printf("%s\n", sendData);
    
    if (res = send(sockD, sendData, sizeof(sendData), 0) == -1)
    {
      printf("Failed to send\n");
    }
    recv(sockD, recvData, sizeof(recvData), 0);
    printf("Message: %s\n", recvData);
  }

  return 0;
}
