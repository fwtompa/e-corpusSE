//File: lookup.c
//Location: "~/manuprobe/src/tags/"



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tags.h"

/*
 * find the tags_tbl entry for converting a tag 
 */

int tags_lookup(char *tag)
{
   int            count;

   for(count=0; count<tags_tbl_len; count++) {
	if (strcmp(tags_tbl[count].tagname, tag) == 0) return count;
	}
   return -1;
}

/*
 * find the char_tbl entry for converting a character reference 
 */

void tags_convert_char(unsigned char c1, unsigned char c2, char* out)
{
    // using UTF-8 encoding
    sprintf(out,"&#%d;", ((c1 % 4) << 6) | (c2 % 64)); // 110000xx 10xxxxxx
}

char* tags_char2ent(unsigned char* in, int quotesOK)
{
    int mo = 0;
    int mi;
    char*  out = NULL;

    
    if (!in) return in; /* no string to convert */
    out = (char*)malloc((strlen("&256;")+1)*sizeof(char)); //room for entref
    for (mi=0;mi<=strlen(in);mi++){
        if (in[mi] < 128) { // ascii
        	if (!quotesOK && in[mi]=='\"') {
			strbuf[mo] = '\0';
			strcat(strbuf, "&quot;");
			mo += strlen("&quot;");
		} else if (!quotesOK && in[mi]=='&' && in[mi+1]!='#') {
			strbuf[mo] = '\0';
			strcat(strbuf, "&#38;");
			mo += strlen("&#38;");
		} else strbuf[mo++] = in[mi]; 
		}
        else {
             tags_convert_char(in[mi],in[mi+1],out);
             mi++; // character already absorbed
             strbuf[mo] = '\0';
             strcat(strbuf, out);
             mo += strlen(out);
             }
        }
    free (out);
    if (strlen(strbuf)>=MAX_PARM_LEN)
	tags_error("char2ent: strbuf overflow",strbuf);
    out = (char*) malloc(strlen(strbuf)+1);
    strcpy(out,strbuf);
    return out;
}


//  char* tags_char2entQuote(unsigned char* in)
//  {
//      int mo = 0;
//      int mi;
//      char*  out;
//  
//      if (!in) return in; /* no string to convert */
//      for (mi=0;mi<=strlen(in);mi++){
//          if (in[mi] < 128 && in[mi]!='\"' ) strbuf[mo++] = in[mi];  //allowing quotation marks temporarily
//          else {
//               out = tags_convert_char(in[mi]);
//               if (out == "") strbuf[mo++] = in[mi]; /* not in conversion table */
//               else {
//                       strbuf[mo] = '\0';
//                       strcat(strbuf, out);
//                       mo += strlen(out);
//                    }
//               }
//          }
//      out = (char*) malloc(strlen(strbuf)+1);
//      strcpy(out,strbuf);
//      return out;
//
// char* tags_char2entQuote(unsigned char* in)
// {
//     int mo = 0;
//     int mi;
//     char*  out = NULL;
// 
//     if (!in) return in; /* no string to convert */
//    out = (char*)malloc((strlen("&256;")+1)*sizeof(char)); //room for entref
//     for (mi=0;mi<=strlen(in);mi++){
//         if (in[mi]!='\"' && in[mi]!='&') strbuf[mo++] = in[mi];  //disallow quotation marks and ampersand
//         else {
//              tags_convert_char(in[mi],in[mi+1],out);
//              if (out == "") strbuf[mo++] = in[mi]; /* not in conversion table */
//              else {
//                      strbuf[mo] = '\0';
//                      strcat(strbuf, out);
//                      mo += strlen(out);
//                   }
//              }
//         }
//     out = (char*) malloc(strlen(strbuf)+1);
//     strcpy(out,strbuf);
//     return out;
// }
// 

char* tags_char_encode(unsigned char* in)
{
	// encode all non-alnumeric characters to defeat cross-site scripting (OWASP)
	// string is just output, so no need to allocate new space -- can use strbuf
	
      int mo = 0;
      int mi;
      char*  out;
	 
     if (!in) return in; /* no string to convert */
     for (mi=0;mi<=strlen(in);mi++){
           if (!in[mi] || isalnum(in[mi])) strbuf[mo++] = in[mi];  //preserve alphanumerics and end-of-string
           else {
                snprintf(strbuf+mo,7,"&#x%02hhX;",in[mi]);
                mo += 6;
                }
           }
     return strbuf;
}

