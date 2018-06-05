#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi.h"

/*
 * Print error message and halt
 */

void cgi_error(char* msg, char* fname)
{
   printf("<br><br>Error: cgi_%s in %s\n", msg, fname);
   fflush(stdout);
   exit (1);
}

char* msgs_lookup(char *msg_id, int lang)
{
   int count;

   for(count=0; count<msgs_tbl_len; count++) {
        if (strcmp(msgs_tbl[count].internal_name, msg_id) == 0)
         return msgs_tbl[count].display_text[lang];
   }
   return "no message matched";
}

