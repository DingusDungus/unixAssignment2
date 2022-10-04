#include "../include/forkedServer.h"

int getWorkSize(char *buf, int size)
{
    int workSize = 0;
    for (int i = 0; i < size && buf[i] != '\0'; i++)
    {
        workSize++;
    }
    return workSize;
}

int copy2chunk(char *chunk, char *resBuf, int bufSize, const int MAX_CHUNK, int iteration, int chunkNr)
{
    int chunk_index = 0;
    int i;
    for (i = iteration; i < (MAX_CHUNK * chunkNr) && i < bufSize; i++, chunk_index++)
    {
        chunk[chunk_index] = resBuf[i];
    }

    return i;
}

void sendChunks(FILE *fs, char *resBuf, int bufSize, const int MAX_CHUNK, int clientSocket)
{
    char chunk[MAX_CHUNK];
    int iteration = 0;
    int chunkNr = 1;
    char *doneMessage = "done";

    if (send(clientSocket, &(MAX_CHUNK), sizeof(MAX_CHUNK), 0) == -1)
    {
        printf("Send unsuccessful, tried to send chunk size\n");
    }

    for (; iteration < bufSize; chunkNr++)
    {
        iteration = copy2chunk(chunk, resBuf, bufSize, MAX_CHUNK, iteration, chunkNr);
        if (send(clientSocket, chunk, sizeof(char) * MAX_CHUNK, 0) == -1)
        {
            printf("Send unsuccessful; chunk nr: %d\n", chunkNr);
        }
    }

    if (send(clientSocket, doneMessage, sizeof(doneMessage), 0) == -1)
    {
        printf("Send unsuccessful, tried to send done message\n");
    }
}

void readFileSendFile(char *fileName, int clientSocket)
{
    FILE *resFile = fopen(fileName, "r");
    const int MAX_CHUNK = 4096; // Max bytes allowed for server to send per send (buffer might discard if too big)

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
    sendChunks(resFile, resBuf, size, MAX_CHUNK, clientSocket);
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
        readFileSendFile(fileName, *clientSocket);
    }

    close(*clientSocket);

    return 1;
}