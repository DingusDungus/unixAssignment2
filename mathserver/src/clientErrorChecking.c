#include "../include/clientErrorChecking.h"
#include <string.h>
bool checkIfFlagHasValue(char *command, int index) {
  char temp[100];
  int tempi = 0;
  for (int i = index; command[i] != ' ' && command[i] != 0; i++, tempi++) {
    temp[tempi] = command[i];
  }
  if (strcmp(temp, "-n") == 0) {
    return false;
  } else if (strcmp(temp, "-I") == 0) {
    return false;
  } else if (strcmp(temp, "-k") == 0) {
    return false;
  } else if (strcmp(temp, "-f") == 0) {
    return false;
  }
  return true;
}

bool matinvCheck(char *command, int index) {
  bool haveN = false;
  bool haveI = false;
  char temp[100];
  int tempi = 0;
  for (int i = index; command[i] != 0; i++, tempi++) {
    if (command[i] == 32) {
      if (strcmp(temp, "-n") == 0) {
        if (checkIfFlagHasValue(command, i + 1)) {
          haveN = true;
        }
      } else if (strcmp(temp, "-I") == 0) {
        if (checkIfFlagHasValue(command, i + 1)) {
          haveI = true;
        }
      }
      printf("Temp: %s\n", temp);
      temp[tempi] = command[i];
      memset(temp, 0, sizeof(char) * 100);
      tempi = 0;
    }
    if (haveN == true && haveI == true) {
      return true;
    }
  }
  return false;
}

bool kmeansCheck(char *command, int index) {
  bool haveK = false;
  bool haveF = false;
  char temp[100];
  for (int i = index; command[i] != 0; i++) {
    if (command[i] == ' ') {
      if (strcmp(temp, "-k") == 0) {
        if (checkIfFlagHasValue(command, i + 1)) {
          haveK = true;
        }
      } else if (strcmp(temp, "-f") == 0) {
        if (checkIfFlagHasValue(command, i + 1)) {
          haveF = true;
        }
      }
      memset(temp, 0, sizeof(char) * 100);
    }
    temp[i] = command[i];
    if (haveK == true && haveF == true) {
      return true;
    }
  }
  return false;
}

bool checkCommandValidity(char *command) {
  char prequel[10];
  int i;
  for (i = 0; i < 10 && command[i] != ' '; i++) {
    prequel[i] = command[i];
  }
  if (strcmp(prequel, "matinv") == 0) {
    return matinvCheck(command, i + 1);
  } else if (strcmp(prequel, "kmeans") == 0) {
    return kmeansCheck(command, i + 1);
  }
  return false;
}
