#include <stdio.h>
#include <errno.h>
#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "tags.h"
#include "cgi.h"
#include <ctype.h>

char* constructQuery(char* word, char* macro, int numWords,
			char* STRING_SUB_PROG, char* useMacro){

// convert a string like caput+sicut and macro name like TITLE_MATCH into
// sgrep pattern: TITLE_MATCH1("caput") equal TITLE_MATCH1("sicut")
// The command executed for running this perl script is like this:
// echo 'caput+sicut' | ./stringSub.plx TITLE_MATCH1 1
// returns NULL if nothing to search for


   int index;
   int stars=0;
   int maxIndex;
   char tempChar;
   char* tempWord;

   char* command;
   char* toReturn;
   FILE* pipeTemp;

   if (!word || word[0] == '\0') {
      toReturn = malloc(sizeof(char));
      toReturn[0] = '\0';
      return(toReturn); // nothing to search for
      }
   command = (char*)malloc( ( strlen("echo '")
		+strlen(word)
		+strlen("' | ")
		+strlen(STRING_SUB_PROG)
		+strlen(" '")
		+strlen(macro)
		+strlen("' ")
		+strlen(useMacro)
		+1 )*sizeof(char) );
   for (index=0;index<strlen(word);index++)
      if (word[index] == '*') stars++;
   maxIndex = strlen(word) 
		+ stars*strlen("INNER(\"..\")")  // * in quotes changed to ".."
		+ numWords*(strlen("WORD(\"\")") 
			+ strlen(macro) 
			+ strlen(" not equal ")) //longest operator
		+ strlen(" -e '' ");
   toReturn = (char*)malloc( (maxIndex + 1 )*sizeof(char)  ); 
   strcpy(toReturn," -e '");
   index = strlen(toReturn);
   stars = index; // save the index before command

   sprintf(command,"echo '%s' | %s '%s' %s",word,STRING_SUB_PROG,macro,useMacro);   
   // echo 'word' | perl stringSub.plx 'macro' use);
//   printf("TESTING: search expression formed from %s<br/>",command); fflush(stdout);
   pipeTemp = popen(command,"r");
   if(!pipeTemp) cgi_error("lookup: cannot open perl pipe file",command);
   if(feof(pipeTemp)) printf("unexpected end of file<br/>");

   while(tempChar = fgetc(pipeTemp))
   {
      if(feof(pipeTemp)) break;
      toReturn[index++] = tempChar;
      if (index > maxIndex-2) {
         toReturn[index] = '\0';
         printf("Command [%s] returns [%s...] ",command,toReturn);
         cgi_error("Unexpectedly large","constructQuery");
      }
   }
   if(index > stars) { // something to search for
      toReturn[index]='\''; //to append the ending single quote and space, for the sgrep query
      toReturn[index+1]=' ';
      toReturn[index+2]='\0';
   } else {
      free(toReturn);
      toReturn = NULL;
   }

//   printf("TESTING: constructQuery(%s,%s,%d,%s,%s) -> %s<br/>",word,macro,numWords,STRING_SUB_PROG,useMacro,toReturn); fflush(stdout);
   if (pclose(pipeTemp)) cgi_error("Error closing pipeTemp","");
   if(command) free(command);
   command = NULL;

   return toReturn;
}

char* adjustQuery(char* instances, char* command) {
   // insert instances into sqrep command
   // adjustQuery(" -e 'P_MATCH("veni")'") -> " -e '("veni") in P_MATCH("veni")'"
   int pin;
   int pout=0;
   int ws;
   int wl;
   char post[6] = " in (";
   int pl = strlen(post);
   int e = strlen("' ");
   char* toReturn;

   toReturn = (char*)malloc((strlen(instances)+strlen(command)+1)*sizeof(char) );
   if (instances[0] == '\0') { // nothing to adjust
      toReturn[0] = '\0';
      return(toReturn);
      }
   
   for (ws=0; instances[ws] != '\''; ws++); // find opening quote
   ws++;
   for (wl=0; instances[ws+wl] != '\''; wl++); // find closing quote

   for (pin=0; pin<ws; pin++) toReturn[pout++] = command[pin];
   for (pin=0; pin<wl; pin++) toReturn[pout++] = instances[ws+pin];
   for (pin=0; pin<pl; pin++) toReturn[pout++] = post[pin];
   for (pin=ws; pin<strlen(command)-e; pin++) toReturn[pout++] = command[pin];
   toReturn[pout++] = ')';
   for (pin; pin<=strlen(command); pin++) toReturn[pout++] = command[pin];
   return(toReturn);
}

int unfinishedXML(char* letters){

// returns i - if finds an unfinished XML tag at letters[i-1]
//         0 - if can't find any such tag

   int i; //index

   for(i=strlen(letters);i;){
      if(letters[--i] == '<')     // ends in middle of tag
         return i+1;  // to ensure it's not a 0 being returned
      else if(letters[i] == '>')  // last tag complete
         return 0;
      }
   return 0; // no opening or closing tags
}

char* untag(char* text, int pos) {

// remove start tag symbol at text[pos-1]

   register int x;
   register int count = 0;
   char* newText;


   if (!text) return(text);
   if (text[--pos] != '<') { // account for zero-indexing and assert pre-condition
      printf("in text %s expected to find '<' at offset %d;",text,pos);
      cgi_error("Violated precondition","untag");
      }
   newText =(char*) malloc((strlen(text) + strlen("&#60") + 1)*sizeof(char));
   // accommodates &#60; because one char already counted in text
   text[pos] = '\0'; // insert end of string
   strcpy(newText,text);
   strcat(newText,"&#60;"); // replace '<' by equivalent
   strcat(newText,text+pos+1); // append remainder of text
   free (text); // text is replaced by newText
   return (newText);
}

int unfinishedTag(int begin, int end, cursor* hit){

// returns the index of the end of the tag if match found WITHIN a tag
//         0 otherwise 

   int tags = 0; //when no opening angle bracket seen '<', it has seen no tags
   int toReturn = 0;

   char* p1=NULL;
   char* match=NULL;
   char* p2=NULL;

   int i; //index
   char currChar;

   match = get_match(hit->begin,hit->end,hit->file); //match region
   p1 = get_match(begin,hit->begin-1,hit->file); //pre-match region
   if (unfinishedXML(p1)) { // tag overlaps start of match
      for(i=0;i<strlen(match) && match[i]!='>';i++); // look for tag end
      if(i==strlen(match)) {
         p2 = get_match(hit->end+1,end,hit->file);// post-match region
         for(i=0;i<strlen(p2) && p2[i]!='>';i++); // look for tag end
         i += hit->end + 1; // index of end of tag
         }
      else {
         i += hit->begin; // index of end of tag
         }
      }
   else if (unfinishedXML(match)) { // tag overlaps end of match
      p2 = get_match(hit->end+1,end,hit->file);// post-match region
      for(i=0;i<strlen(p2) && p2[i]!='>';i++); // look for tag end
      i += hit->end + 1; // index of end of tag
      }
   else {
      i = 0; // match not in the middle of a tag
      }

   if (p1) free(p1);
   if (match) free(match);
   if (p2) free(p2);

   return i;
}


char* constructRegion(int begin, int end, cursor* hit, int fnum) {

// insert match tags

   char* p;
   char* p1;
   char* match;
   int i;

   p1 = get_match(begin,end,hit->file); // empty string if end < begin

   match = get_match(hit->begin,hit->end,hit->file);

   if (i = unfinishedXML(match)) match = untag(match,i);

   p = (char*) malloc( (strlen(p1) + strlen(match)
            + 2*strlen(MATCH_TAG) + strlen("< pos=\"\" chosenTexts=\"\">[]</>")
            + 2*strlen("999999") + 1)*sizeof(char) );
   if(!p)cgi_error("constructRegion: failed memory allocation","match");

   if (end >= hit->begin)  // pre-region overlaps start, end, or all of match
      sprintf(p,"%s<%s pos=\"%d\" chosenTexts=\"%d\">[%s]</%s>",p1,MATCH_TAG,hit->begin,fnum,match,MATCH_TAG);
   else sprintf(p,"%s<%s pos=\"%d\" chosenTexts=\"%d\">%s</%s>",p1,MATCH_TAG,hit->begin,fnum,match,MATCH_TAG);

   if(p1) free(p1); //since they are malloced in the function : get_match
   if(match) free(match);

   return p;
}

char* completeXML(char* tagged) {
// prepend opening tags on the front and append closing tags on the end to make fragment well-formed XML
   int    pin = 0, pout = 0;
   int    i,j, needs = 0;
   int    closing;
   int    found[200], ftop = 0;    // stack of tags found
   int    closed[200], ctop = 0;   // stack of unmatched closing tags found
   int    errors[200], etop = 0;   // list of mis-matched closing tags found

   if(strlen(tagged)==0){ //if it is an empty string
      strcpy(tags_buffer,"");
      return tags_buffer;
      }

   for (pin; tagged[pin]; pin++) {
      if (tagged[pin] == TBEGIN) {      // found <genid...>, <genid.../>, or </genid>
         if (tagged[++pin] == TCLOSE) { // it's a closing tag </genid>
            closed[ctop] = ++pin;       // mark start of genid for closing tag
            closing = 1;
            }
         else { 
            found[ftop] = pin;          // mark start of genid for opening or empty tag
            closing = 0;
            }
         while (tagged[pin] && tagged[pin] != ' ' && tagged[pin] != TEND) pin++;  //find end of genid
         if (closing) closed[ctop+1] = pin - closed[ctop];  // and save length
         else found[ftop+1] = pin - found[ftop];
 
         while (tagged[pin] && tagged[pin] != TEND) pin++; // find end of tag
         if (tagged[pin-1] == TCLOSE) {   // empty tag -- ignore it
            continue;
            }

         if (closing) {                 // check for matching opening tag on stack
            if (ftop && found[ftop-1] == closed[ctop+1] // tags are same length
		&& strncmp(tagged+found[ftop-2],tagged+closed[ctop],closed[ctop+1]) == 0) { // match
               ftop -= 2;    // pop opening tag
               }
            else if(!ftop)  ctop += 2;  // save unmatched closing tag only if outside tags
            else {  // mis-matched tag, likely problem in ordering of opening and closing tags
               errors[etop++] = closed[ctop];  // move tag to list of errors
               errors[etop++]= closed[ctop]+1;
               ctop -= 2;
               }
            }
         else ftop += 2;     // leave opening tag on stack
         }
      }
   // now all tags left on stack are unmatched
   for (i=ctop-2; i>=0; i-=2)
      needs += closed[i+1] + 2;
   for (i=ftop-2; i>=0; i-=2) { // some of thse opening tags might match errors in closing tags
      needs += found[i+1] + 3;
      }
   tags_fix_buffer(strlen(tagged)+needs); // make sure there's enough space for text

   for (ctop-=2; ctop>= 0; ctop-=2) { // first write all needed opening tags
      tags_buffer[pout++] = TBEGIN;
      strncpy(tags_buffer+pout,tagged+closed[ctop],closed[ctop+1]);  
      pout += closed[ctop+1];
      tags_buffer[pout++] = TEND;
      }
   strcpy(tags_buffer+pout,tagged);  // now the text fragment itself
   pout += strlen(tagged);
   for (ftop-=2; ftop>=0; ftop-=2) { // finally all the needed closing tags
      tags_buffer[pout++] = TBEGIN;
      tags_buffer[pout++] = TCLOSE;
      strncpy(tags_buffer+pout,tagged+found[ftop],found[ftop+1]);  
      pout += found[ftop+1];
      tags_buffer[pout++] = TEND;
      }
   tags_buffer[pout] = '\0';  /// ... and the string delimiter
   return(tags_buffer);
}

void encompassWord(cursor* doc, int size) {
   // move begin and end to surround whole word
   int i;
   char* p;
   int pre=20;
   int post=20;
   char* skipPunc = "'&#;";
   int begin = doc->begin;
   int end = doc->end;

   if (doc->begin < pre) pre = doc->begin; // do not try to read before begin of file
   if (doc->end + post >= size) post = size - doc->end - 1; //... nor beyond end of file
   p = get_match(doc->begin-pre, doc->end+post, doc->file);
   for (i=(doc->end-doc->begin)+pre;  // begin here in case search term ends with a stopping character (blank)
        i<(doc->end-doc->begin)+pre+post+1 && (isalnum(p[i]) || strchr(skipPunc,p[i]));
        i++) (doc->end)++; // move to end of word
   for (i=pre;
        i>=0 && (isalnum(p[i]) || strchr(skipPunc,p[i]));
        i--) (doc->begin)--; // move to begin of word
   (doc->begin)++;  // now go back one to avoid stopping character
   (doc->end)--;
   if (doc->begin > begin) doc->begin = begin;  // do not let matched region shrink
   if (doc->end < end) doc->end = end; 
}
 
