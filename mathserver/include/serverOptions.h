#ifndef SERVER_OPTIONS
#define SERVER_OPTIONS

#include <stdbool.h>

// Options set by the parser to decide the way the server operates
struct options
{
    int port;
    bool helpRequested;
    bool daemonRequested;
    bool muxbasic;
    bool muxscale;
    bool regulerServer;
};

#endif