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

    // matinv -n 4 -P 1 -I rand

    return mainServer(clientSocket, serverOpts);
}
