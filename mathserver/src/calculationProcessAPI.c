#include "../include/calculationProcessAPI.h"
#include <stdbool.h>
#include <string.h>
#include <sys/ucontext.h>

// Needed to remove newline when user ends input with pressing enter
void rmNewLine(char *argString) {
  for (int i = 0; i < 255; i++) {
    if (argString[i] == '\n') {
      argString[i] = '\0';
      break;
    }
  }
}

// Splits away file defined by user, only needing defined number of clusters
char *getK(char *argString) {
  char *k = (char *)malloc(sizeof(char) * 100);
  int index = 0;
  bool kFound = false;
  for (int i = 0; i < 255; i++) {
    if (argString[i] == '-' && argString[i + 1] == 'k') {
      kFound = true;
    }
    if (argString[i] >= 48 && argString[i] < 58 && kFound) {
      k[index] = argString[i];
      index++;
    } else if (index > 0) {
      break;
    }
  }
  return k;
}

char *createOutPutFile() {
  char fileExtension[100] = ".txt";
  char resFile[100] = "outFile";
  char pid[100];
  char createResFile[100] = "touch ";
  sprintf(pid, "%d", getpid());
  printf("%s\n", pid);
  strcat(resFile, pid);
  strcat(resFile, fileExtension);
  strcat(createResFile, resFile);
  system(createResFile);
  char *res = (char *)malloc(sizeof(char) * 100);
  strcpy(res, resFile);

  return res;
}

char *createInPutFile() {
  char fileExtension[100] = ".txt";
  char resFile[100] = "inFile";
  char pid[100];
  char createResFile[100] = "touch ";
  sprintf(pid, "%d", getpid());
  printf("%s\n", pid);
  strcat(resFile, pid);
  strcat(resFile, fileExtension);
  strcat(createResFile, resFile);
  system(createResFile);
  char *res = (char *)malloc(sizeof(char) * 100);
  strcpy(res, resFile);

  return res;
}

char *getCommand(char *argString) {
  char command[100] = "./../";
  strcat(command, argString);
  char *res = (char *)malloc(sizeof(char) * 100);
  strcpy(res, command);

  return res;
}

char *matinvMode(char *argString, int socket) {
  char *pipeFile = createOutPutFile();
  char *command = getCommand(argString);

  // As matinv does not write to a resultfile it must be piped to one
  char pipeString[100] = " > ";

  strcat(pipeString, pipeFile);
  strcat(command, pipeString);
  printf("%s\n", command);

  if (system(command) != 0) {
    return 0;
  }
  transferFile(socket, 4096, pipeFile);

  free(pipeFile);
  return 0;
}

char *kmeansMode(char *argString, int socket) {
  char parsedCommand[255] = "kmeans";
  char *k = getK(argString);
  printf("%s\n", k);
  char *inputFile = createInPutFile();
  char *outputFile = createOutPutFile();
  char fileFlag[100] = " -f ";
  char kFlag[100] = " -k ";
  char oFlag[100] = " -o ";
  strcat(fileFlag, inputFile);
  strcat(oFlag, outputFile);
  strcat(kFlag, k);
  strcat(parsedCommand, kFlag);
  strcat(parsedCommand, fileFlag);
  strcat(parsedCommand, oFlag);
  char *command = getCommand(parsedCommand);
  printf("%s\n", command);
  recvFile(socket, inputFile);
  if (system(command) != 0) {
    return 0;
  }
  transferFile(socket, 4096, outputFile);
  free(inputFile);
  free(command);
  return 0;
}

char *initCalculation(char *argString, int mode, int socket) {
  rmNewLine(argString);

  if (mode == MATINV) {
    return matinvMode(argString, socket);
  } else if (mode == KMEANS) {
    return kmeansMode(argString, socket);
  }
  return 0;
}