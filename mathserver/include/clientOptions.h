#ifndef CLIENT_OPTIONS
#define CLIENT_OPTIONS

#include <stdbool.h>

// Options set by the parser to decide the way the server operates
struct options
{
    int port;
    char *address;
};

#endif