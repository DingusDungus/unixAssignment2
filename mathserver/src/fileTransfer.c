#include "../include/fileTransfer.h"
#include <string.h>
#include <sys/socket.h>

char *getFile(char *argString) {
  char *fileName = (char *)malloc(sizeof(char) * 100);
  int index = 0;
  bool flagFound = false;
  for (int i = 0; i < 255; i++) {
    if (argString[i] == '-' && argString[i + 1] == 'f') {
      flagFound = true;
      i = i + 2;
    }
    else if (argString[i] != ' ' && flagFound && 
    argString[i] != '\n') {
      fileName[index] = argString[i];
      index++;
    } else if (index > 0) {
      break;
    }
  }
  return fileName;
}

bool isEqual(char *s1, char *s2, int s1_size, int s2_size) {
  for (int i = 0; i < s1_size && i < s2_size; i++) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  return true;
}

int getMode(char *command) {
  char buf[10];
  int i;
  for (i = 0; i < 9 && command[i] != ' '; i++) {
    buf[i] = command[i];
  }
  buf[i] = '\0';
  if (isEqual(buf, "matinv", 10, 6)) {
    return 1;
  } else if (isEqual(buf, "kmeans", 10, 6)) {
    return 2;
  } else {
    return 0;
  }
}

int getWorkSize(char *buf, int size) {
  int workSize = 0;
  for (int i = 0; i < size && buf[i] != '\0'; i++) {
    workSize++;
  }
  return workSize;
}

int copy2chunk(char *chunk, char *resBuf, int bufSize, const int MAX_CHUNK,
               int iteration, int chunkNr) {
  int chunk_index = 0;
  int i;
  memset(chunk, 0, sizeof(char) * MAX_CHUNK);
  for (i = iteration; i < (MAX_CHUNK * chunkNr) && i < bufSize;
       i++, chunk_index++) {
    chunk[chunk_index] = resBuf[i];
  }

  return i;
}

void sendChunks(FILE *fs, char *resBuf, int bufSize, const int MAX_CHUNK,
                int socket) {
  char chunk[MAX_CHUNK];
  int iteration = 0;
  int chunkNr = 1;
  char *doneMessage = "done";

  if (send(socket, &(MAX_CHUNK), sizeof(MAX_CHUNK), 0) == -1) {
    printf("Send unsuccessful, tried to send chunk size\n");
    return;
  }

  for (; iteration < bufSize; chunkNr++) {
    iteration =
        copy2chunk(chunk, resBuf, bufSize, MAX_CHUNK, iteration, chunkNr);
    if (send(socket, chunk, sizeof(char) * MAX_CHUNK, 0) == -1) {
      printf("Send unsuccessful; chunk nr: %d\n", chunkNr);
    }
  }

  if (send(socket, doneMessage, sizeof(doneMessage), 0) == -1) {
    printf("Send unsuccessful, tried to send done message\n");
  }
}

int sendPid(int socket)
{
  char pid[100];
  sprintf(pid, "%d", getpid());
  send(socket, pid, sizeof(char) * 100, 0);
  return 0;
}

int recvPid(int socket, char *pid)
{
  recv(socket, pid, sizeof(char) * 100, 0);
  return 0;
}

int transferFile(int socket, int chunkSize, char *filename, bool failure) {
  // Error handling for server
  if (failure)
  {
    char *failureMessage = "Failure";
    if (send(socket, failureMessage, sizeof(char) * 20, 0) == -1) {
      printf("Send unsuccessful, tried to send failure message\n");
    }
    return 0;
  } else {
    char *failureMessage = "Non-failure";
    if (send(socket, failureMessage, sizeof(char) * 20, 0) == -1) {
      printf("Send unsuccessful, tried to send non-failure message\n");
    }
  }
  FILE *resFile = fopen(filename, "r");
  const int MAX_CHUNK = chunkSize; // Max bytes allowed for server to send per
                                   // send (buffer might discard if too big)
  fseek(resFile, 0L, SEEK_END);
  int size = ftell(resFile);
  fseek(resFile, 0L, SEEK_SET);
  char resBuf[size];
  memset(resBuf, 0, sizeof(resBuf));

  fread(resBuf, sizeof(char), size, resFile);
  fclose(resFile);
  size = getWorkSize(resBuf, size);
  sendChunks(resFile, resBuf, size, MAX_CHUNK, socket);
  return 0;
}

int recvFile(int socket, char *filename, char *fileMode) {
  int chunkSize;
  char failureMessage[20];
  int res;
  // Capturing failure data
  if ((res = recv(socket, &failureMessage, sizeof(char) * 20, 0) == -1)) {
    printf("Error reading results\n");
    return 1;
  }
  if (strcmp(failureMessage, "Failure") == 0)
  {
    printf("Error; Command failed on server-side\n");
    return 1;
  }
  // Capturing size of data
  if ((res = recv(socket, &chunkSize, sizeof(chunkSize), 0) == -1)) {
    printf("Error reading results\n");
    return 1;
  }
  FILE *resFile = fopen(filename, fileMode);
  char *recvData = (char *)malloc(sizeof(char) * chunkSize);
  // Recieves chunks from server and appends them to result file
  while (strcmp(recvData, "done")) {
    memset(recvData, 0, chunkSize);
    if ((res = recv(socket, recvData, sizeof(char) * chunkSize, 0)) == 0) {
      printf("Sender disconnected\n");
      return 1;
    }
    if (isEqual(recvData, "done", 100, 4))
    {
      break;
    }
    int size = getWorkSize(recvData, chunkSize);
    fwrite(recvData, 1, sizeof(char) * size, resFile);
  }

  fclose(resFile);
  free(recvData);
  return 0;
}