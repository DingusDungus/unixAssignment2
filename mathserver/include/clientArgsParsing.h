#ifndef CLIENT_ARGS_PARSING
#define CLIENT_ARGS_PARSING

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "clientOptions.h"

/* Function parses server args and sets options for server, 
if function returns false an error in the command-line options has been found and server shall close with error code */
bool parseArgs(const char **argv, int argc, struct options *clientOpts);

#endif