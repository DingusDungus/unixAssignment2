#include "include/forkedServer.h"
#include "include/mainServer.h"

int main(int argc, char const *argv[])
{
    int *clientSocket = NULL;
    int *pid = NULL;

    // get port for server
    if (pid == NULL)
    {
        printf("Server starting!; pid: %d\n", getpid());
        if (argc > 2)
        {
            printf("Error; No port given!\n");
            return -1;
        }
        const int PORT_NUMBER = atoi(argv[1]);

        pid = malloc(sizeof(pid));
        clientSocket = malloc(sizeof(clientSocket));
        return mainServer(PORT_NUMBER, INADDR_ANY, clientSocket, pid);
    }
    else
    {
        printf("Client fork starting!; pid: %d\n", getpid());
        return forkedServer(clientSocket);
    }
    return 0;
}
