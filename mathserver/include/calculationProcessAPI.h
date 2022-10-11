#ifndef CALCULATION_PROCESS_API
#define CALCULATION_PROCESS_API

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <string.h>

#include "./modeDefinitions.h"
#include "./fileTransfer.h"

char* initCalculation(char *argString, int mode, int socket);

#endif