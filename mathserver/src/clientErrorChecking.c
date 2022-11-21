#include "../include/clientErrorChecking.h"
#include <string.h>
#include <ctype.h>

// Checks if argv meet requirements for matinv
bool matinvCheck(char argv[10][100])
{
  for (int i = 1;i < 10 && argv[i][0] != 0;i++)
  {
    if (strcmp(argv[i], "-n") == 0)
    {
      printf("-n found\n");
      if ((i + 1) >= 10 || !stringIsNumber(argv[i+1]))
      {
        printf("Error; No value given to -n, ex. -n 9\n");
        return false;
      }
    }
    else if (strcmp(argv[i], "-I") == 0)
    {
      printf("-I found\n");
      if ((i + 1) >= 10 || ((strcmp(argv[i + 1], "rand") != 0 && strcmp(argv[i + 1], "fast") != 0)))
      {
        printf("Error; No value given to -I (acceptable \"rand\", \"fast\")\n");
        return false;
      }
    }
  }
  return true;
}

// Checks if argv meet requirements for kmeans
bool kmeansCheck(char argv[10][100])
{
  bool fFlagGiven = false;
  for (int i = 1;i < 10 && argv[i][0] != 0;i++)
  {
    if (strcmp(argv[i], "-k") == 0)
    {
      if ((i + 1) >= 10 || !stringIsNumber(argv[i+1]))
      {
        printf("Error; -k flag is not given a number\n");
        return false;
      }
      continue;
    }

    if (strcmp(argv[i], "-f") == 0)
    {
      if ((i + 1) < 10 && argv[i+1][0] != 0)
      {
        if (!fileExists(argv[i + 1]))
        {
          printf("Error; Given file does not exist!\n");
          return false;
        }
      } else {
        printf("Error; No file was given after -f flag\n");
        return false;
      }
      fFlagGiven = true;
    }
  }
  if (!fFlagGiven) {
    printf("Error; No -f flag given, no file given, required for kmeans!\n");
    return false;
  }
  return true;
}

// Checks if client input is valid
bool checkCommandValidity(char *command) {
  char argv[10][100];
  transformIntoArgV(command, argv);
  if (strcmp(argv[0], "matinv") == 0)
  {
    return matinvCheck(argv);
  }
  else if (strcmp(argv[0], "kmeans") == 0)
  {
    return kmeansCheck(argv);
  }
  else if (strcmp(argv[0], "done") == 0)
  {
    return true;
  }
  return false;
}
