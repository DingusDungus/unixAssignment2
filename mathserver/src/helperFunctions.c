#include "../include/helperFunctions.h"

bool stringIsNumber(char string[100])
{
    printf("char: %c\n", string[0]);
    if (string[0] != '+' && string[0] != '-' && ( string[0] < '0' || string[0] > '9'))
    {
        return false;
    }
    for (int i = 1;string[i] != 0;i++)
    {
        if (string[i] == 0)
        {
            break;
        }
        printf("Char: %c\n", string[i]);
        if (string[i] < '0' || string[i] > '9')
        {
            printf("Invalid\n");
            return false;
        }
    }
    return true;
}