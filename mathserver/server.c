#include "include/forkedServer.h"
#include "include/mainServer.h"

int main(int argc, char const *argv[])
{
    int *clientSocket = NULL;
    int *pid = NULL;
    printf("Initialization...\n");
    // get port for server
    printf("Server starting!; pid: %d\n", getpid());
    if (argc > 2)
    {
        printf("Error; No port given!\n");
        return -1;
    }
    const int PORT_NUMBER = atoi(argv[1]);

    pid = malloc(sizeof(pid));
    *pid = 1;
    clientSocket = malloc(sizeof(clientSocket));
    mainServer(PORT_NUMBER, INADDR_ANY, clientSocket, pid);

    // Section which a child process comes to naturally
    printf("Client fork starting!; pid: %d\n", getpid());
    forkedServer(clientSocket);

    printf("Done\n");
    return 0;
}
