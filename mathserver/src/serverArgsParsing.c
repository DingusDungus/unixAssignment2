#include "../include/serverArgsParsing.h"

void printHelp() {
  printf("-Mathserver v.1.0-\n   -h // Prints out server options and their "
         "functionality\n   -p [port] // Allows you to give a custom port to "
         "the server\n   -d // Option to run server as a daemon\n   -s "
         "[muxbasic || muxscale] // Multiplexing options\n");
}

bool parseArgs(const char **argv, int argc, struct options *serverOpts) {
  // Sets default values for options
  serverOpts->fork = true;
  serverOpts->port = 3000;

  serverOpts->daemonRequested = false;
  serverOpts->helpRequested = false;
  serverOpts->muxbasic = false;
  serverOpts->muxscale = false;
  for (int i = 0; i < argc; i++) {
    const char *commandLine = argv[i];
    if (strcmp(commandLine, "-h") == 0) {
      // If parser finds that -h was written in commandline, no further options
      // will be parsed as server will only print help text
      printHelp();
      exit(0);
    } else if (strcmp(commandLine, "-p") == 0) {
      if ((i + 1) >= argc) {
        printf("Error; no port given after -p\n");
        exit(3);
      }
      serverOpts->port = atoi(argv[i + 1]);
      // Checks if no valid integer was given, which returns 0 when atoi is
      // being called on it
      if (serverOpts->port == 0) {
        printf("Error; no port given after -p\n");
        exit(3);
      }
    } else if (strcmp(commandLine, "-s") == 0) {
      serverOpts->fork = false;
      if ((i + 1) >= argc) {
        printf("Error; no option given after -s\n");
        exit(3);
      }

      const char *commandLineExtra = argv[i + 1];
      if (strcmp(commandLine, "muxbasic")) {
        printHelp();
        exit(3);
      }
      if (strcmp(commandLine, "muxscale")) {
        printHelp();
        exit(3);
      } else {
        printf("Error; no valid option given after -s\n");
        exit(3);
      }
    } else if (strcmp(commandLine, "-d") == 0) {
      serverOpts->daemonRequested = true;
      printHelp();
      exit(3);
    }
  }
  return true;
}
