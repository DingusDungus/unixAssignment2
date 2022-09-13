#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <arpa/inet.h>

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

    int connectStatus = connect(sockD, (struct sockaddr *)&servAddr,
                                sizeof(servAddr));

    if (connectStatus == -1)
    {
        printf("Error...\n");
    }

    else
    {
        char strData[255];

        recv(sockD, strData, sizeof(strData), 0);

        printf("Message: %s\n", strData);
    }

    return 0;
}