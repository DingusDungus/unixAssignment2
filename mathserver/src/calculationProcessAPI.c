#include "../include/calculationProcessAPI.h"

void rmNewLine(char *argString)
{
    for (int i = 0;i < 255;i++)
    {
        if (argString[i] == '\n')
        {
            argString[i] = '\0';
            break;
        }
    }
}

char *initCalculation(char *argString)
{
    rmNewLine(argString);

    printf("Init calculation\n");
    char fileExtension[100] = ".txt";
    char pipeFile[100] = "resultFile";
    printf("%s\n", pipeFile);

    char pipeString[100] = " > ";
    char command[100] = "./../";
    char pid[100];
    char createResFile[100] = "touch ";

    sprintf(pid, "%d", getpid());
    printf("%s\n", pid);
    strcat(pipeFile, pid);
    printf("%s\n", pipeFile);
    strcat(pipeFile, fileExtension);
    strcat(createResFile, pipeFile);
    system(createResFile);
    strcat(pipeString, pipeFile);
    strcat(command, argString);
    strcat(command, pipeString);
    printf("%s\n", command);

    if (system(command) != 0)
    {
        return 0;
    }
    char *res = (char *)malloc(sizeof(char) * 100);
    strcpy(res, pipeFile);
    return res;
}