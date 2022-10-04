#include "../include/forkedServer.h"

int getWorkSize(char *buf, int size)
{
    int workSize = 0;
    for (int i = 0;i < size && buf[i] != '\0';i++)
    {
        workSize++;
    }
    return workSize;
}

int forkedServer(int *clientSocket)
{
    char sendMsg[255] = "Welcome to mathserver API\n"
                        "Options:\n"
                        "    matinv -n [size of matrix] -P [parallel, 1-] -I [rand || fast]\n"
                        "-------------------------\n"
                        "When finished write [done]\n";
    char recvData[255];
    send(*clientSocket, sendMsg, sizeof(sendMsg), 0);
    while (1)
    {
        memset(recvData, 0, sizeof(recvData));
        recv(*clientSocket, recvData, sizeof(recvData), 0);
        if (strcmp(recvData, "done\n") == 0)
        {
            printf("Client disconnected from process: %d\n", getpid());
            break;
        }
        printf("%s", recvData);
        char *fileName = initCalculation(recvData);

        FILE *resFile = fopen(fileName, "r");
        if (resFile == NULL)
        {
            return 1;
        }

        fseek(resFile, 0L, SEEK_END);
        int size = ftell(resFile);
        fseek(resFile, 0L, SEEK_SET);
        char resBuf[size];
        memset(resBuf, 0, sizeof(resBuf));

        fread(resBuf, sizeof(char), size, resFile);
        fclose(resFile);
        printf("%s\n", resBuf);
        size = getWorkSize(resBuf, size);
        remove(fileName);
        int res;
        // Sending size of data
        if ((res = send(*clientSocket, &(size), sizeof(size), 0)) == -1)
        {
            printf("Send unsuccessful\n");
        }

        // Sending data
        if ((res = send(*clientSocket, resBuf, sizeof(char) * size, 0)) == -1)
        {
            printf("Send unsuccessful\n");
        }
        printf("Bytes sent: %d\n", res);
    }

    close(*clientSocket);

    return 1;
}