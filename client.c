#include <arpa/inet.h>
#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>

int getMatinvInp(char **argv)
{
  
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
    char serMsg[255];
    char sendData[10][100];
    char strData[1024];
    int dataIteration = 0;

    recv(sockD, strData, sizeof(strData), 0);
    printf("%s\n", strData);
    fflush(stdin);

    scanf("%s", sendData[dataIteration]);
    dataIteration++;
    while (scanf("%s", sendData[dataIteration]) != EOF)
    {
      printf("%s\n", sendData[dataIteration]);
      dataIteration++;
    }
    printf("Sending!\n");

    if (res = send(sockD, sendData, sizeof(sendData), 0) == -1)
    {
      printf("Failed to send\n");
    }
    recv(sockD, strData, sizeof(strData), 0);
    printf("Message: %s\n", strData);
  }

  return 0;
}
