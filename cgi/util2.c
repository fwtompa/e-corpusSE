//File: util2.c
//Location: ~/manuprobe/src/cgi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "tags.h"
#include "cgi.h"

unsigned char* http_val (char* word, int m){
    int x;

    for (x=0;x<=m;x++) {
       if (!strcmp(entries[x].name,word)) return(entries[x].val[0]?entries[x].val:NULL);
    }
    return(NULL);
}

unsigned char* http_nval (char* word, int m){
    int x;
    unsigned char* val;

    val = http_val(word,m);
    if (val && strlen(val)>0) {
       x = atoi(val);
       sprintf(val,"%d",x); // make sure only numeric data is present
       return(val);
    }
    return(NULL);
}

void http_val_dump (int m, FILE* out){
    int x;

    for (x=0;x<=m;x++) {
       fprintf(out,"[%s=%s];",entries[x].name,entries[x].val);
    }
}

void http_printf (char* text) {
   register int x;

   if (!text) return;
   for (x=0;text[x];x++) {
	if (text[x] == '<') printf("&lt;");
	else if (text[x] == '&') printf("&amp;");
	else putchar(text[x]);
	}
}

void semi2comma(unsigned char *str) {
    register int x;

    for(x=0;str[x+1];x++) if(str[x] == ';' && str[x+1] == ' ') str[x] = ',';
}

char* decodeUTF8(unsigned char* str) {
    register int x;
    register int p=0;
    int len;
    char* newstr;
    char* num;

    
    for (x=0;str[x];x++) len += (str[x]>127 ? 3 : 1); // 2-char codes for 6-char encoding
    if (len == strlen(str)) return(str);
    newstr = malloc((len+1)*sizeof(char));
    num = malloc(7*sizeof(char));
    for (x=0;str[x];x++) {
       if (str[x]>127) {
          if (str[x++] == 195) // c3 in hex
             sprintf(num,"&#%u;",str[x]-128+192); // x80 -> 192
          else // c2 in hex
             sprintf(num,"&#%u;",str[x]-192+160); // xA0 -> 160
          newstr[p] = 0;
          strcat(newstr,num);
          p += 6;
          }
       else newstr[p++] = str[x];
       }
   newstr[p] = 0;
   return(newstr);
}

void writeLog(int m) {
   char* command;
   char* buffer = malloc(1000*sizeof(char));
   int bufsize = 1000;
   FILE* pipeTemp;
   int p = 0;
   int x;
   char tempChar;


   command = malloc((strlen(USAGE_LOG)+strlen("echo REMOTE_ADDR=[$REMOTE_ADDR] HTTP_REFERER=[$HTTP_REFERER])") +1)
                  *sizeof(char));
   sprintf(command,"date");
   pipeTemp = popen(command,"r");
   while(tempChar = fgetc(pipeTemp))
   {
      if(feof(pipeTemp)) break;
      buffer[p++] = (tempChar != '\n')?tempChar:' ';
      if (p > bufsize-2) {
         bufsize *= 2;
         buffer = (char *)realloc(buffer,sizeof(char)*bufsize);
      }
   }
   pclose(pipeTemp);


   sprintf(command,"echo REMOTE_ADDR=[$REMOTE_ADDR] HTTP_REFERER=[$HTTP_REFERER]");
   pipeTemp = popen(command,"r");
   while(tempChar = fgetc(pipeTemp))
   {
      if(feof(pipeTemp)) break;
      buffer[p++] = (tempChar != '\n')?tempChar:' ';
      if (p > bufsize-2) {
         bufsize *= 2;
         buffer = (char *)realloc(buffer,sizeof(char)*bufsize);
      }
   }
   pclose(pipeTemp);

   for (x=0;x<=m;x++) {
      if (p + strlen(entries[x].name) + strlen(entries[x].val) + strlen("[=];") > bufsize-2) {
         bufsize *= 2;
         buffer = (char *)realloc(buffer,sizeof(char)*bufsize);
      }
      sprintf(&buffer[p],"[%s=%s];",entries[x].name,entries[x].val);
      p += strlen(entries[x].name) + strlen(entries[x].val) + strlen("[=];");
   }

   buffer[p] = '\0';
   pipeTemp = fopen(USAGE_LOG,"a");
   fprintf(pipeTemp,"%s\n",buffer);
   fclose(pipeTemp);
}

int numWords(char* word) //returns an number of words, ie, ones delimited with op codes, +,-,|
{
	int numWords=0;
	int aIndex=0; // a=array, arrayIndex
	int wIndex=0; // w=word , wordIndex
	int size=0; //current part size
	int lastEnd=0; //the end of last part

	int i;
	int j;

	if(!word) return 0;

	while(word[wIndex] != '\0'){

		char currChar=word[wIndex];

		if(currChar=='+' || currChar=='-' || currChar=='|' ){

			numWords++;
		}
		wIndex++;
	}

	//for the last word
	numWords++;

	return (numWords);
}


/*  N.B. using perl version in constructQueryNew for now
char* constructQuery(char* word,char* macro,int numWords) {

// construct query for SGREP

	char* toReturn = (char*)malloc( (strlen(word)
		+ numWords*( max(strlen(" not equal "),strlen(" -e ''"))
				+ strlen(macro))
		+ 1) * sizeof(char);
	// "not equal" represents the longest sgrep key word used

	int wIndex=0;
	int lastEnd;
	char currChar;
	char nextChar;
	int lastCheckIndex;
	int macroStart=0;

	char* opening = "(\"";
	char* closing = "\")";


	char* tempStr = (char*)malloc(1*sizeof(char));


	strcpy(toReturn," -e '");  // opening sequence for search
	while(word[wIndex] != '\0')
	{
		currChar = word[wIndex];
		nextChar = word[wIndex+1];

		//printf("wIndex=%d : currChar=%c : nextChar=%c<br>\n",wIndex,currChar,nextChar); fflush(stdout);

		if( currChar=='+' || currChar=='|' || currChar=='-' )
		{
			if(macroStart){
				strcat(toReturn,closing);
				macroStart=0;
			}

			//replacing the op char with any of these
			if(currChar=='+') strcat(toReturn," equal ");
			else if(currChar=='-') strcat(toReturn," not equal ");
			else if(currChar=='|') strcat(toReturn," or ");

		}else if( currChar=='(' )
		{
			if( nextChar != '(' ){
				strcat(toReturn,"(");
				strcat(toReturn,macro);
				strcat(toReturn,opening);
				macroStart=1;
			}else
				strcat(toReturn,"(");

		}else if( currChar==')' ){

			if( lastCheckIndex==(wIndex-1) ) {
				strcat(toReturn,closing); //if the last char wasn't any of the parenthesis, or operators
				strcat(toReturn,")");
				macroStart=0;
			}else
				strcat(toReturn,")");
		}else{

			sprintf(tempStr,"%c",currChar);

			if(!macroStart){
				strcat(toReturn,macro);
				strcat(toReturn,opening);
				macroStart=1;
			}

			strcat(toReturn,tempStr);
			lastCheckIndex=wIndex;

		}

		if(lastCheckIndex != wIndex) lastCheckIndex=-1;  //to check in the next loop, whether the last char was an ending of the word or not

		wIndex++;
		//printf("toReturn=%s<br>\n",toReturn); fflush(stdout);
	}

	//for the ending only
	if(macroStart){
		strcat(toReturn,"\")' ");
		macroStart=0;
	}else
		strcat(toReturn,"' ");

	return toReturn;
}
*/


char* orthonormal(char* in, int align){

// adjust text to normalize Latin spelling and remove quotes
/* replacements:
        convert to lower case
        remove diacritics and ligatures (using charcode_list)
        normalize spelling
 */
// but preserve spacing so that offsets match source text
/* pad shortened words with '\037' (Unit separator char)
   to avoid matching with blanks
*/

	int ip;
	int op = 0;
	char* out;
        int i, j;
        char c;
        char pad = '\037';
        int inTag = 0, inTagVal = 0;
	
	out = (char*) malloc((strlen(in)+1)*sizeof(char));
	for (ip=0; in[ip]; ip++) {
           c = '\0';
           if ((!inTag || inTagVal) && isalpha(in[ip])) c = tolower(in[ip]);
	   else if(in[ip] == ' ') {
              out[op++] = in[ip]; // copy the blank itself, then pad
	      while (align && op <= ip) out[op++] = pad;
	   }
	   else if (in[ip] == '<') {             // preserve tag and attribute names 
              inTag = 1;
	      while (op < ip) out[op++] = pad;  // first align
              out[op++] = in[ip];
           }
	   else if (inTag && !inTagVal && in[ip] == '>') { 
              inTag = 0;
              out[op++] = in[ip];
           }
	   else if (inTag  && !inTagVal && in[ip] == '"') { // start attribute value => normalize
              inTagVal = 1;
              out[op++] = in[ip];
           }
	   else if (inTag  && inTagVal && in[ip] == '"') { // end attribute vale => align
              inTagVal = 0;
	      while (op < ip) out[op++] = pad;  // first align
              out[op++] = in[ip];
           }
           else if (inTag && !inTagVal) out[op++] = in[ip]; // preserve chars in tags and outside attr vals
           else if (in[ip] == '&')  { // check table for normalizations
              for (j=0; j < char_tbl_len - 1; j++)  {  // check every string
                 if ((i = strncmp(char_tbl[j].charref,in+ip,strlen(char_tbl[j].charref))) == 0) {
                    ip += strlen(char_tbl[j].charref) - 1;
                    break;
                    }
                 if (i < 0) { // ordering => no later string can match
                    j = char_tbl_len - 1; // signal no match by pointing to empty string
                    break;
                    }
              }
              if (j < char_tbl_len - 1) { // found a match
                 c = char_tbl[j].norm;
              }
              else {
                 while (in[ip] != ';')  out[op++] = in[ip++];  // no normalization available
                 out[op++] = ';';
              }
           }
           else out[op++] = in[ip];
           if (c != '\0') { // found alpha character
            // ae      e
            // ci      ti
            // d       t	(added July 2017)
            // h
            // j       i
            // k       c
            // m       n
            // oe      e
            // ph      f
            // v       u
            // y       i
              switch(c) {
              case 'd':
                 out[op++] = 't';
                 break;
              case 'e': 
                 if (op > 0 && (out[op-1] == 'a' || out[op-1] == 'o')) {
                    while (op > 1 && (out[op-2] == 'a' || out[op-2] == 'o')) --op; // leave only one 'a' or 'o'
                    out[op-1] = 'e'; // replace ae or oe by e.
                 }
                 else out[op++] = 'e';
                 break;
              case 'h':
                 if (op > 0 && out[op-1] == 'p') out[op-1] = 'f';
                 else /* do nothing */;
                 break;
              case 'i': case 'j': case 'y':
                 if (op > 0 && out[op-1] == 'c') {
                    if (op > 1 && out[op-2] == 't') op--;  // handle "...tci..."
                    else  out[op-1] = 't';
                 }
                 out[op++] = 'i';
                 break;
              case 'k':
                 out[op++] = 'c';
                 break;
              case 'm':
                 out[op++] = 'n';
                 break;
              case 'v':
                 out[op++] = 'u';
                 break;
              default:
                out[op++] = c;
              }
	      if (op > 1 && out[op-1] == out[op-2]) op--;   // \l+ -> \l
           }
	}
	while (align && op < ip) out[op++] = pad;       // pad with pad chars
	out[op] = '\0';
        if (!align) { // do not insert alignment chars at the end of query
           for (op -= 1; op>0 && out[op] == pad; op--) out[op] = '\0';
           }
//        printf("<pre>orthonormal: '%s' -> '%s'</pre>",in,out);fflush(stdout);

	return out;
}

void adjustOrtho(char* word) {
   // handle searches such as "philosophia" to also match "philosophiae" (normalized)
            // ae      e
            // ci      ti
            // oe      e
            // ph      f
   int pos = strlen(word)-1;
   switch (word[pos]) {
      case 'a': case 'o':
         word[pos] = 'e';
         break;
      case 'c':  // need to search for ti as well
         word[pos++] = 't';
         word[pos++] = 'i';
         word[pos] = '\0';  
      case 'p':
         word[pos] = 'f';
         break;
      default:
         word[0] = '\0'; // no other word to find
      }
   return;
}

char* normalize(char* word, int indexed){

// remove all the spaces, except the ones in between double quotes or braces,
// convert everything to lower case
// convert parentheses inside quotes and all commas and apostrophes to codes
// if searching an indexed text, apply orthographic normalization
//    indexed = 2 => apply orthographic normalization on extended word

	char* toReturn;
	char currChar;
	int wIndex=0;
	int rIndex=0;
	int inQuote=0;
	int in, out=-1;
	int size = strlen(word) + 2; // maximum size of returned string
	char* chars = "(),'";
	int charCodes[4]     // codes for chars above 
		= {40, 41, 44, 39};  // in same order as chars
	char* codePos;
	int codeOffset;
	char encoded[6]; // room for "\#44;" for example
	char* normword;

	for(wIndex=0;word[wIndex];wIndex++)
		if (strchr(chars,word[wIndex])) size += 4; // add 4 for each
	toReturn = malloc(size*sizeof(char));
        normword = indexed ? orthonormal(word,0) : word;
	//if (indexed){
	    //normword = orthonormal(word);
            // now make sure extra spaces do not hinder search
	    //for(in=0;in <= strlen(normword);in++) { // remove duplicate blanks
	       //if(in > 0 && normword[in]==' ' && normword[out] == ' ') continue;
               //normword[++out] = normword[in];
	    //}
//	    printf("TESTING: orthonormal: '%s' (%d chars) -> '%s' (%d chars)<BR/>\n",word,(int)strlen(word),normword,(int)strlen(normword)); fflush(stdout);
	//}
	//else normword = word;

	for(wIndex=0;normword[wIndex];wIndex++){
		currChar=tolower(normword[wIndex]);
		if(currChar=='\"')
			inQuote = 1 - inQuote; // flip the bit
		if(currChar!=' ' || inQuote) {
			codePos = strchr(chars,currChar);
			codeOffset = codePos ? codePos - chars : 0;
			if((inQuote && codePos && (codeOffset<2))|| codeOffset>1) {
				toReturn[rIndex] = '\0';
				sprintf(encoded,"\\#%d;",charCodes[codeOffset]);
				strcat(toReturn,encoded);
				rIndex += 5;
				}
			else toReturn[rIndex++] = currChar;
		}
        }
	toReturn[rIndex] = '\0'; // terminate the string
        if (indexed == 2) { // perhaps need to adjust word ending
            adjustOrtho(toReturn);
            }
//        printf("TESTING: normalized search term: %s\n",toReturn); fflush(stdout);
	if (indexed) free(normword);  // free space for normalized form of word
	return toReturn;
}

int nestingCheck(char* word,int* numWords) {

// checks whether the double quotes and brackets are balanced,
// op chars not nested and properly placed, ignores spaces

	int inBracket=0;
	int inQuote=0;
	int inWord=0;
	int wIndex=0;
	char currChar;

	int lastOpen=-1;  // index of the last opening bracket
	int lastClose=-1; // index of last closing bracket
	int lastOpr=-1;   // index of the last operator
	int lastQuote=-1; // index of last quotation mark
	int lastChar=-1;  // index of any other non-blank character

	*numWords = 1; // count number of words (1 + number of ops)
	for(wIndex=0; word[wIndex]; wIndex++){
		currChar=word[wIndex];
		if(currChar=='\"'){
			if( (!inQuote) && lastChar>lastOpr )
				return 0; // e.g., + word "word"
			inQuote = 1 - inQuote; // flip the bit
			lastQuote=wIndex;
		}else if(currChar=='('){
			if(lastChar>lastOpr)
				return 0; // e.g., + word (
			inBracket++;
			lastOpen=wIndex;
		}else if(currChar==')'){
			inBracket--;
			lastClose=wIndex;
			if (inBracket < 0)
				return 0; // too many right brackets
		}else if( !inQuote &&
			  (currChar=='+' || currChar=='-' || currChar=='|') ){
			if(lastChar<0)
				return 0; // e.g., + word (at start of string)
			if(lastOpr>lastChar)
				return 0; // e.g., + +
			if(lastOpen>lastChar)
				return 0; // e.g., word ( +
			lastOpr=wIndex;
			(*numWords)++;
		}else if(currChar!=' ') {
			if(lastClose>lastOpr)
				return 0; // e.g., + ) word
			if( !inQuote && lastQuote>lastOpr)
				return 0; // e.g., + "word" word
			if( !inQuote && !inWord && lastChar>lastOpr)
				return 0; // e.g., + word word
			inWord = 1;
			lastChar=wIndex;
		}else if(!inQuote) inWord = 0;

		//printf("currChar=%c ----------<br>lastOpr=%d<br>lastOpen=%d<br>lastClose=%d<br>lastChar=%d<br>inQuote=%d<br>lastQuote=%d<br>\n",currChar,lastOpr,lastOpen,lastClose,lastChar,inQuote,lastQuote); fflush(stdout);

	}
	//final conditions 
	if( lastOpr>lastChar )
		return 0; // e.g., word + (at end of string)
	if( lastChar==-1 && lastQuote==-1 )
		return 0; // no words at all (nor quoted blanks)
	return (!inBracket && !inQuote);
}
