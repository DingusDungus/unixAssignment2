#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <string.h>



int main()
{
    char *temp = NULL;
    size_t len;
    int lineSize = getline(&temp, &len, stdin);
    char command[100] = "./../";
    strcat(command, temp);
    system(command);
    printf("Done\n");
    
    return 0;
}
