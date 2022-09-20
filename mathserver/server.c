#include "include/mainServer.h"
#include "include/serverArgsParsing.h"

int main(int argc, char const *argv[])
{
    int *clientSocket = malloc(sizeof(clientSocket));

    printf("Initialization...\n");
    struct options *serverOpts = malloc(sizeof(serverOpts));
    if (parseArgs(argv, argc, serverOpts) == false)
    {
        return -1;
    }

    return mainServer(clientSocket, serverOpts);
}
