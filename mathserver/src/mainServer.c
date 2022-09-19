#include "../include/mainServer.h"

void printHelp()
{
    printf(
        "-Mathserver v.1.0-\n   -h // Prints out server options and their functionality\n   -p [port] // Allows you to give a custom port to the server\n   -d // Option to run server as a daemon\n   -s [muxbasic || muxscale] // Multiplexing options\n");
}

void getLocalNetworkAddress(char *output)
{
    struct ifaddrs *ifap;
    char *addr;
    struct sockaddr_in *sa;

    getifaddrs(&ifap);
    for (struct ifaddrs *ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            // Tests to pick the wifi or ethernet based local-address, whichever comes first
            // As lo stands for the localhost ip, which is almost always 172.0.0.1
            if ((ifa->ifa_name[0] != 'l' && ifa->ifa_name[1] != 'o'))
            {
                sa = (struct sockaddr_in *)ifa->ifa_addr;
                const char* temp = inet_ntoa(sa->sin_addr);
                memset(output, '\0', sizeof(output));
                strcpy(output, temp);
                printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, output);
                break;
            }
        }
    }

    freeifaddrs(ifap);
}

int mainServer(int *clientSocket, int *pid, struct options *serverOpts)
{
    if (serverOpts->helpRequested)
    {
        printHelp();
        return 1;
    }
    // create server socket
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    // string store data to send to client

    // define server address
    struct sockaddr_in servAddr;
    char servAddrString[256];
    getLocalNetworkAddress(servAddrString);
    servAddr.sin_port = htons(serverOpts->port);
    servAddr.sin_family = AF_INET;
    int result = inet_pton(AF_INET, servAddrString, &servAddr);
    if (result != 1)
    {
        printf("Error; Network error!\n");
        return 1;
    }

    // bind socket to the specified IP and port
    bind(servSockD, (struct sockaddr *)&servAddr, sizeof(servAddr));

    // listen for connections
    listen(servSockD, 1);
    printf("Listening on local ip %s with port %d!\n", servAddrString, (int)servAddr.sin_port);
    while (1 && *pid != 0)
    {
        // integer to hold client socket.
        *clientSocket = accept(servSockD, NULL, NULL);
        printf("Accepted client!\n");
        if (fork() == 0)
        {
            printf("Child process pid: %d\n", getpid());
            return 0;
        }
    }

    return 0;
}
