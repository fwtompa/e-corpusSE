#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tags/tags.h"
#include "cgi/util.h"
#include "cgi/cgi.h"
void directories_initialize();

void main(int argc, char *argv[]) {
   char* buffer;
   int bp = 0;
   int bsize = 102400;

   if (argc != 1) {
      printf("Usage: index <[text-in] >[text-out]\n");
      exit;
   }
   buffer = (char*) malloc(bsize*sizeof(char));
   while (1) {
      buffer[bp] = (char)fgetc(stdin);
      if (buffer[bp] == EOF) {
         buffer[bp] = '\0';    // should check for file read error???
         break;
      }
      if(++bp == bsize-1) {
         buffer[bsize] = '\0';
         bsize += 102400;
         buffer = (char*) realloc (buffer, bsize*sizeof(char));
      }
   }
   directories_initialize(); // define CHAR_CODES
   tags_initchars(CHAR_CODES);
   printf("%s",orthonormal(buffer,1));
   exit(0);
}
