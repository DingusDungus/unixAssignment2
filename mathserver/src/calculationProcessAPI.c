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

void createOutPutFile(char *file, int nr) {
  char fileExtension[100] = ".txt";
  char extended[100] = "_soln";
  char pid[100];
  char s_nr[100];
  char createResFile[100] = "touch ";
  sprintf(pid, "%d", getpid());
  sprintf(s_nr, "%d", nr);
  strcat(file, pid);
  strcat(file, extended);
  strcat(file, s_nr);
  strcat(file, fileExtension);
  strcat(createResFile, file);
  system(createResFile);
}

char *createInPutFile() {
  char fileExtension[100] = ".txt";
  char resFile[100] = "inFile";
  char pid[100];
  char createResFile[100] = "touch ";
  sprintf(pid, "%d", getpid());
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

int matinvMode(char *argString, int socket, int sol) {
  char pipeFile[100] = "matinv_client";
  createOutPutFile(pipeFile, sol);
  char *command = getCommand(argString);

  // As matinv does not write to a resultfile it must be piped to one
  char pipeString[100] = " > ";

  strcat(pipeString, pipeFile);
  strcat(command, pipeString);

  if (system(command) != 0) {
    return 1;
  }
  printf("Sending solution: %s\n", pipeFile);
  transferFile(socket, 4096, pipeFile);

  return 0;
}

int kmeansMode(char *argString, int socket, int sol) {
  char outputFile[100] = "kmeans_client";
  char parsedCommand[255] = "kmeans";
  char *k = getK(argString);
  char *inputFile = createInPutFile();
  createOutPutFile(outputFile, sol);
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
  recvFile(socket, inputFile, "w");
  if (system(command) != 0) {
    return 1;
  }
  transferFile(socket, 4096, outputFile);
  remove(inputFile);
  remove(outputFile);
  free(inputFile);
  free(command);
  return 0;
}

int initCalculation(char *argString, int mode, int socket, int sol) {
  rmNewLine(argString);

  if (mode == MATINV) {
    return matinvMode(argString, socket, sol);
  } else if (mode == KMEANS) {
    return kmeansMode(argString, socket, sol);
  }
  return 0;
}