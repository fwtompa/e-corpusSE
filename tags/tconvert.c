//FILE: converter.c
//Location: "~/manuprobe/src/tags/"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tags.h"
/*
 * encode XML-like tagged text based on tag translation table
 */
char *tags_convert(char *tagged)
    /*tagged is the matched string*/
{
   int          pin = 0, pout = 0;
   int          which_tag, which, len;
   char         tag[200];
   char        *tagstr;              //stores for each tag
   char         attrnm[200];
   char         attrval[200];
   int          moreattr = 0;
   int          empty;

   //added for compatability with Margot project - for its location tags
   if(strlen(tagged)==0){ //if it is an empty string
      strcpy(tags_buffer,"");
      return tags_buffer;
      }

   tags_buffer[pout] = 0; // start with the empty string
   while (tagged[pin]) 
   {
     if (tagged[pin] == TBEGIN) 
     {
        moreattr = tags_get_tag(tag,tagged,&pin);
        if (tag[0] == TCLOSE) 
           tagstr = &tag[1];
        else 
           tagstr = tag;
        if (tag[strlen(tag)-1] == TCLOSE) {
           tag[strlen(tag)-1] = '\0';
           empty = 1;
        } else 
           empty = 0;
        which_tag = tags_lookup(tagstr);
        if (which_tag >= 0) {
           if (tag[0] != TCLOSE) {
              len = strlen(tagstr);
              while( moreattr && tagged[pin] != TEND) {
                 while(tagged[pin] == ' ' || tagged[pin]== NEWLINE || tagged[pin] == TAB) 
                    pin++; // skip over any whitespace
                 if (tagged[pin] == TCLOSE) {
                    break;
                 }
                 if (tagged[pin] == TEND) break;
                 moreattr = tags_get_attr(attrnm,attrval,tagged,&pin);
                 if (!attrval) break; // something wrong in tag syntax -- detect?
                 strcat(tagstr,".");
                 strcat(tagstr,attrnm);
                 which = tags_lookup(tagstr);
                 if (which >= 0) {
                    tags_handle_start(which, &pin, &pout, tagged, attrval);
                    /* process the attrval */
                    if (printoff <= 0) {
                       tags_fix_buffer(pout+strlen(attrval));
                       strcat(tags_buffer, attrval);
                       pout += strlen(attrval);
                       /* copy from attrval to tags_buffer */
                    }
                    tags_handle_end(which, &pout);
                 }//if
                 tagstr[len] = '\0'; // now ignore attribute names
              }//if
              tags_handle_start(which_tag, &pin, &pout, tagged, NULL);
              if (tagged[pin] == TCLOSE) {
                 tags_handle_end(which_tag, &pout);
                 pin++;
              }//if
              if (tagged[pin] == TEND) pin++;
           }else { /* close tag */
              tags_handle_end(which_tag, &pout);
           }//if
        }else if (printoff <= 0) {
           tags_fix_buffer(pout+strlen(tag)+8);
           strcpy(&tags_buffer[pout], "&lt;");
           strcpy(&tags_buffer[pout+4], tag);
           if (moreattr) strcpy(&tags_buffer[pout+4+strlen(tag)], " ");
           else strcpy(&tags_buffer[pout+4+strlen(tag)], ">");
           pout += strlen(tag)+5;
        }//if
     }//if
     if (tagged[pin] == EBEGIN) {
        tags_get_entref(tag,tagged,&pin);
        if (printoff <= 0) {
           if (tag[1] == NUMCHARREF) { // &#ddd;
              strcpy(&tags_buffer[pout], tag);
              pout += strlen(tag);
           } else {
              which = tags_lookup(tag);
              if (which >= 0) {
                 tagstr = tags_tbl[which].rbegin+1;  /* skip " mark */
                 tags_fix_buffer(pout+strlen(tagstr));
                 strcpy(&tags_buffer[pout], tagstr);
                 pout += strlen(tagstr);
              }else {
                 tags_fix_buffer(pout+strlen(tag)+4);
                 strcpy(&tags_buffer[pout], "&amp;");
                 strcpy(&tags_buffer[pout+5], &tag[1]);
                 pout += strlen(tag)+4;
              }
           }
        }
     }else {   /* plain text data */
        if (printoff <= 0) {
           tags_fix_buffer(pout+strlen(&tagged[pin]));
           tags_get_data(tags_buffer,&pout,tagged,&pin);
//printf("+%s",&tagged[pin]); fflush(stdout);
        }
        else {
           pin = tags_offset(tagged,pin,TBEGIN);
           if (pin < 0) pin = strlen(tagged);
        }
     }
   }
return(tags_buffer);
}

void tags_handle_start(int which, int *pin, int *pout, char *tagged, char* attrval)
/* fix variable references */
{
   int len;
   char* tagstr;
   char tag[20];
   tagstr = tags_tbl[which].rbegin;
   if (tagstr[0] == '+') {
       printoff--;
      }
   if (printoff <= 0 || tagstr[0] == '*') {
      tags_fix_buffer(*pout+strlen(tagstr+1));
      strcpy(&tags_buffer[*pout], tagstr+1);
      *pout += strlen(tagstr+1);
   }
   if (printoff <= 0 && tagstr[0] == '&') { /* treat content */
      if (!attrval) {
         len = tags_offset(tagged,*pin,TBEGIN);
         if ((len)-(*pin)<20) { /* don't overrun tag space */
            len = 0;
            tags_get_data(tag,&len,tagged,pin);
         //printf("::%s::",tag); fflush(stdout);
         }
         else strcpy(tag, "NOT A TAG");
      }
      else {
         strcpy(tag, attrval);
         attrval[0] = '\0';
      }
      which = tags_lookup(tag);
      if (which >= 0) tags_handle_start(which, pin, pout, tagged, NULL);
   }
   if (tagstr[0] == '-') {
      printoff++;
      }
}

void tags_handle_end(int which, int *pout)
{
   char * tagstr;
   tagstr = tags_tbl[which].rend;
   if (tagstr[0] == '+') {
      printoff--;
      }
   if(printoff <= 0 || tagstr[0] == '*') {
      tags_fix_buffer(*pout+strlen(tagstr+1));
      strcpy(&tags_buffer[*pout], tagstr+1);
      *pout += strlen(tagstr+1);
      }
   if (tagstr[0] == '-') {
      printoff++;
      }
}

void tags_fix_buffer(int needs)
{
   if (needs+1 > tags_buffer_len) {
     tags_buffer_len = needs+1024;
     tags_buffer = (char *)realloc(tags_buffer,sizeof(char)*tags_buffer_len);
     if (!tags_buffer) tags_error("Cannot realloc space","tags_fix_buffer");
     }
}
