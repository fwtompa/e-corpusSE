#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Print error message and halt
 */

void tags_error(char* msg, char* fname)
{
   printf("Error: tags_%s in %s\n", msg, fname);
   exit (1);
}
