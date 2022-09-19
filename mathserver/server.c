#include "include/forkedServer.h"
#include "include/mainServer.h"
#include "include/serverArgsParsing.h"

int main(int argc, char const *argv[])
{
    int *pid = malloc(sizeof(pid));
    *pid = 1;
    int *clientSocket = malloc(sizeof(clientSocket));
    printf("Initialization...\n");
    struct options *serverOpts = malloc(sizeof(serverOpts));    
    if (parseArgs(argv, argc, serverOpts) == false)
    {
        return -1;
    }

    // If options like -h is given the server will stop prematurely
    if (mainServer(clientSocket, pid, serverOpts))
    {
        return 0;
    }

    // Section which a child process comes to naturally
    printf("Client fork starting!; pid: %d\n", getpid());
    forkedServer(clientSocket);

    printf("Done\n");
    return 0;
}
