#include "../include/serverArgsParsing.h"

bool parseArgs(const char **argv, int argc,struct options *serverOpts)
{
    // Sets default values for options
    serverOpts->fork = true;
    serverOpts->port = 3000;

    serverOpts->daemonRequested = false;
    serverOpts->helpRequested = false;
    serverOpts->muxbasic = false;
    serverOpts->muxscale = false;
    for (int i = 0;i < argc;i++)
    {
        const char* commandLine = argv[i];
        if (strcmp(commandLine, "-h") == 0)
        {
            // If parser finds that -h was written in commandline, no further options will be parsed as server will only print help text
            serverOpts->helpRequested = true;
            break;
        }
        else if (strcmp(commandLine, "-p") == 0)
        {
            if ((i + 1) >= argc)
            {
                printf("Error; no port given after -p\n");
                return false;
            }
            serverOpts->port = atoi(argv[i + 1]);
            // Checks if no valid integer was given, which returns 0 when atoi is being called on it
            if (serverOpts->port == 0)
            {
                printf("Error; no port given after -p\n");
                return false;
            }
        }
        else if (strcmp(commandLine, "-s") == 0)
        {
            serverOpts->fork = false;
            if ((i + 1) >= argc)
            {
                printf("Error; no option given after -s\n");
                return false;
            }

            const char *commandLineExtra = argv[i + 1];
            if (strcmp(commandLine, "muxbasic"))
            {
                serverOpts->muxbasic = true;
            }
            if (strcmp(commandLine, "muxscale"))
            {
                serverOpts->muxscale = true;
            }
            else
            {
                printf("Error; no valid option given after -s\n");
                return false;
            }
        }
        else if (strcmp(commandLine, "-d"))
        {
            serverOpts->daemonRequested = true;
        }
    }
    return true;
}
