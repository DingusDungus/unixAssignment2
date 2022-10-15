#include "../include/clientArgsParsing.h"

void printHelp() {
  printf("-lient v.1.0-\n   -h // Prints out client options and their "
         "functionality\n   -p [port] // Port to server "
         "\n   -ip // Address to server\n");
}

bool parseArgs(const char **argv, int argc, struct options *clientOpts) {
  // Sets default values for options
  clientOpts->port = 3000;
  clientOpts->address = "0.0.0.0";
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
      clientOpts->port = atoi(argv[i + 1]);
      // Checks if no valid integer was given, which returns 0 when atoi is
      // being called on it
      if (clientOpts->port == 0) {
        printf("Error; no port given after -p\n");
        exit(3);
      }
    } else if (strcmp(commandLine, "-ip") == 0) {
        if ((i + 1) >= argc) {
        printf("Error; no address given after -ip\n");
        exit(3);
      }
      clientOpts->address = (char *)argv[i + 1];
    }
  }
  return true;
}
