#include "../include/helperFunctions.h"

// Transform command string into argv vector
void transformIntoArgV(char *command, char argv[10][100])
{
  int vIndex1 = 0;
  int vIndex2 = 0;
  for (int i = 0;command[i] != 0 && command[i] != '\n';i++)
  {
    if (command[i] == ' ')
    {
      argv[vIndex1][vIndex2] = 0;
      vIndex1++;
      vIndex2 = 0;
      continue;
    }
    argv[vIndex1][vIndex2] = command[i];
    vIndex2++;
  }
  argv[vIndex1][vIndex2] = 0;
  vIndex1++;
  argv[vIndex1][0] = 0;
}

bool fileExists(char string[100])
{
    FILE *file;
    if ((file = fopen(string, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}

bool stringIsNumber(char string[100])
{
    if (string[0] != '+' && string[0] != '-' && ( string[0] < '0' || string[0] > '9'))
    {
        return false;
    }
    for (int i = 1;string[i] != 0 && string[i] != '\n';i++)
    {
        if (string[i] == 0 || string[i] == '\n')
        {
            break;
        }
        if (string[i] < '0' || string[i] > '9')
        {
            printf("Invalid\n");
            return false;
        }
    }
    return true;
}