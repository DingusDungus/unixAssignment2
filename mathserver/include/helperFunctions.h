#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <stdbool.h>
#include <stdio.h>

bool fileExists(char string[100]);
bool stringIsNumber(char string[100]);
void transformIntoArgV(char *command, char argv[10][100]);


#endif