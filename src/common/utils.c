#include "utils.h"

// prints the message sent in the parameter and exit with error status
void printErrorAndExit(char *errorMessage)
{
    puts(errorMessage);
    exit(1);
}


// generate random number between 1 and 10
int getRandomNumber()
{
    return (rand() % 9) + 1;
}