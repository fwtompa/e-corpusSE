#include <stdio.h>
#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cgi.h"

long int engine_open(char* doc, cursor* datafile) {
   // open a data file for reading selected blocks
   // set datafile to be the file handle
   // return the size of the file, or 0 if no file found

   char* textName;
   long int fileSize = 0;

   textName = (char*)malloc( ( strlen(TEXTLOC) + strlen(doc) + 1 )*sizeof(char) );
   strcpy(textName,TEXTLOC);
   strcat(textName,doc);
//    printf("Opening %s<br/>",textName); fflush(stdout);

   datafile->file = fopen(textName,"r");
   if (!datafile->file) cgi_error("engine_read: cannot open text file",textName);

   if (fseek(datafile->file, 0L, SEEK_END) == 0) {
      fileSize = ftell(datafile->file);
      fseek(datafile->file, 0L, SEEK_SET);
      }
   else fileSize = 0;
   datafile->begin = -1;
   datafile->end = -1;
   free(textName);
//    printf("return file size = %ld<br/>",fileSize); fflush(stdout);
   return(fileSize);
   }

void engine_close(cursor* data) {
   if (data && data->file) {
      pclose(data->file);
      free(data);
      }
}

void engine_idx_close(merge_cursor* data) {
   if (data) {
      engine_close(data->part[0]);
      engine_close(data->part[1]);
      free(data);
      }
}

char* get_match(int begin, int end, FILE* data)
{
   // return empty string if begin out of range, end < begin
   // returns (unexpectedly) short string if end > fileSize
   char* match;
   int size = 0;

   if(begin>end) match = (char*)malloc(sizeof(char));
   else {
      size = end - begin + 1;
      match = (char*)malloc(sizeof(char) * (size+1));
      if (!fseek(data, begin, SEEK_SET)) { // returns 0 if successful seek
         fread(match, sizeof(char), size, data);
         }
      else {
         printf("begin: %d, end: %d, ferror: %d, feof: %d ",begin,end,ferror(data),feof(data));
         fflush(stdout);
         }
   }
   match[size] = '\0';
   // printf("get_match (%d,%d) => (",begin,end); http_printf(match); printf(")<br/>"); fflush(stdout);
   return match;
}

int  engine_count(char* query, char* doc, char* idx_query) {
   // count number of matches for query in doc
   // idx_query = NULL iff no index query

   char* textName;
   char* command;
   int   num, xnum;
   int   count = 0;
   cursor* pipe[2];

//   printf("<br/>engine_count: query=%s, doc=%s, idx_query=%s, ",query,doc,idx_query?idx_query:"NULL"); fflush(stdout);
//   printf("TEXTLOC=%s, INDEX_PREFIX=%s<BR/>",TEXTLOC,INDEX_PREFIX); fflush(stdout);
   if (!idx_query) { // can use engine's count command
      textName = (char*)malloc( ( strlen(TEXTLOC) + strlen(doc) + 1 )*sizeof(char) );
      sprintf(textName,"%s%s",TEXTLOC,doc);
//      printf("search doc: %s<br/>",textName); fflush(stdout);

      command = (char*)malloc((strlen(COUNT_CMD) + strlen(query) +
               + strlen(textName) + 1  )*sizeof(char)); 
      sprintf(command,"%s%s%s",COUNT_CMD,query,textName);

//      printf("engine_count: command: %s\n<br/>",command); fflush(stdout); 
      pipe[0] = malloc(sizeof(cursor));
      pipe[0]->file = popen(command,"r");
      if(!pipe[0]->file) cgi_error("lookup: cannot open pipe file",command);
      // read result from count command
      num = fscanf(pipe[0]->file, "%d ", &count);  // #matches?
      if (num != 1) cgi_error("engine (1): cannot read pipe file",command);
      free(textName);
      free(command);
      }
   else {
      pipe[0] = engine_findall(query,doc,1); // extended matches
      pipe[1] = engine_findall(idx_query,doc,1); // normalized matches
      // now merge and count
      num = advanceN(pipe[0],1);
      xnum = advanceN(pipe[1],1);
      while (num && xnum) { // more matches in both lists
         count++;
         if (pipe[0]->end < pipe[1]->begin) num = advanceN(pipe[0],1);
         else if (pipe[1]->end < pipe[0]->begin) xnum = advanceN(pipe[1],1);
         // index end could appear before main begin in extraordinary circulstances
         else {  // both equal
            num = advanceN(pipe[0],1);
            xnum = advanceN(pipe[1],1);
            }
         }
      while (num) {
         count++;
         num = advanceN(pipe[0],1);
         }
      while (xnum) {
         count++;
         xnum = advanceN(pipe[1],1);
         }

//      textName = (char*)malloc( ( strlen(TEXTLOC) + strlen(doc) +
//                  strlen(INDEX_PREFIX) + 1 )*sizeof(char) );
//      sprintf(textName,"%s%s%s",TEXTLOC,INDEX_PREFIX,doc);
////      printf("search doc: %s<br/>",textName); fflush(stdout);
//
//      command = (char*)malloc((strlen(COUNT_CMD) + strlen(query) + strlen(idx_query) +
//               + strlen(textName) + 1  )*sizeof(char)); 
//      if (query[0]) { // something to count
//         sprintf(command,"%s%s%s",COUNT_CMD,query,textName);
////         printf("engine_count: command: %s\n<br/>",command); fflush(stdout); 
//         pipe[0] = malloc(sizeof(cursor));
//         pipe[0]->file = popen(command,"r");
//         if(!pipe[0]->file) cgi_error("lookup: cannot open pipe file",command);
//         // read result from count command
//         num = fscanf(pipe[0]->file, "%d ", &xnum);  // #matches?
//         if (num != 1) cgi_error("engine (2): cannot read pipe file",command);
//         }
//      else xnum = 0;
//
//      sprintf(command,"%s%s%s",COUNT_CMD,idx_query,textName);
////      printf("engine_count: command: %s\n<br/>",command); fflush(stdout); 
//      pipe[0] = malloc(sizeof(cursor));
//      pipe[0]->file = popen(command,"r");
//      if(!pipe[0]->file) cgi_error("lookup: cannot open pipe file",command);
//      // read result from count command
//      num = fscanf(pipe[0]->file, "%d ", &count);  // #matches?
//      if (num != 1) cgi_error("engine (3): cannot read pipe file",command);
//
//      count += xnum;
//      free(textName);
//      free(command);
      }
   return(count);
}

cursor* engine_findall(char* query, char* doc, int indexed) {
   // find all matches (start/end offsets) for query in doc
   // indexed = 1 iff there is an index

   char* textName;
   char* command;
   cursor* pipe = malloc(sizeof(cursor));

   if (query && query[0]) { // something to search for
      textName = (char*)malloc( ( strlen(TEXTLOC) + strlen(doc) +
                  (indexed?strlen(INDEX_PREFIX):0) + 1 )*sizeof(char) );
      sprintf(textName,"%s%s%s",TEXTLOC,(indexed?INDEX_PREFIX:""),doc);

      command = (char*)malloc((strlen(SEARCH_CMD) + strlen(query) +
                  + strlen(textName) + 1  )*sizeof(char)); 
      sprintf(command,"%s%s%s",SEARCH_CMD,query,textName);

//      printf("engine_findall: command: %s\n<br/>",command); fflush(stdout); 
      pipe->file = popen(command,"r");
      if(!pipe->file) cgi_error("lookup: cannot open pipe file",command);
      free(textName);
      free(command);
      }
   else pipe->file = NULL;
   pipe->begin = -1;
   pipe->end = -1;
   return(pipe);
}

merge_cursor* engine_idx_findall(char* query, char* doc, char* idx_query) {
   merge_cursor* pipes = malloc(sizeof(merge_cursor));

   pipes->begin = -1;
   pipes->end = -1;
   pipes->part[0] = engine_findall(query,doc,idx_query?1:0);
   advanceN(pipes->part[0],1); // initialize each queue
   if (idx_query) { // index provided
      pipes->part[1] = engine_findall(idx_query,doc,1);
      advanceN(pipes->part[1],1);
      }
   else { // signal that there is nothing in the index queue
      pipes->part[1] = NULL;
      }
   return(pipes);
}

int advanceN(cursor* matches, int count) {
   // move match "cursor" forward count matches
   // set start and end to values of resulting match
   // return 0 iff failed to match
   int num=2;
   int i;
   if (!matches || !matches->file) // nothing to advance
      return(0);
   for (i=0; i<count && num==2; i++){
      num = fscanf(matches->file, "%d %d ", &matches->begin, &matches->end);
      }
   if (num==2) {
//      printf("advanceN: vals: %d, %d \n<br>",matches->begin,matches->end); fflush(stdout);
      return(1);
      }
   else {
      matches->begin = -1;
      matches->end = -1;
//      printf("advanceN: EOF"); fflush(stdout);
      return(0);
      }
}

int advanceMergedN(merge_cursor* matches, int count) {
   // move match "cursor" forward count matches
   // return 0 iff failed to match
   int i;
   int more;

   for (i=0; i<count; i++) {
      if (!matches->part[0] || matches->part[0]->begin < 0) {
//         printf("advanceMergedN: no more main<br/>"); fflush(stdout);
         if (!matches->part[1]) { // no index file
            matches->begin = -1;
            matches->end = -1;
            return(0);
            }
         if (count - i > 1) more = advanceN(matches->part[1],count-i-1);
         else more =  (matches->part[1]->begin >= 0);
         if (more) {
            matches->begin = matches->part[1]->begin;
            matches->end = matches->part[1]->end;
            advanceN(matches->part[1],1);
            return(1);
            }
         else { 
            matches->begin = -1;
            matches->end = -1;
            return(0);
            }
         }
      if (!matches->part[1] || matches->part[1]->begin < 0) {
//         printf("advanceMergedN: no more index<br/>"); fflush(stdout);
         if (count - i > 1) more = advanceN(matches->part[0],count-i-1);
         else more =  1;
         if (more) {
            matches->begin = matches->part[0]->begin;
            matches->end = matches->part[0]->end;
            advanceN(matches->part[0],1);
            return(1);
            }
         else { 
            matches->begin = -1;
            matches->end = -1;
            return(0);
            }
         }
      if (matches->part[0]->end < matches->part[1]->begin) {
//         printf("advanceMergedN: advance main<br/>"); fflush(stdout);
         matches->begin = matches->part[0]->begin;
         matches->end = matches->part[0]->end;
         advanceN(matches->part[0],1);
         }
      
      else if (matches->part[1]->end < matches->part[0]->begin) {
         // index match could end before main start in extraordinary circumstances
//         printf("advanceMergedN: advance index<br/>"); fflush(stdout);
         matches->begin = matches->part[1]->begin;
         matches->end = matches->part[1]->end;
         advanceN(matches->part[1],1);
         }
      else {
//         printf("advanceMergedN: advance both<br/>"); fflush(stdout);
         matches->begin = matches->part[1]->begin;
         matches->end = matches->part[0]->end<matches->part[1]->end?matches->part[0]->end:matches->part[1]->end;
         advanceN(matches->part[0],1);
         advanceN(matches->part[1],1);
         }
      }
   return(1);
}

int advanceToIncludeOrBeyond(cursor* matches, int target) {
   // move match "cursor" forward until target between begin and end
   // set begin and end to values of resulting match
   // return 0 iff failed to match
   int more = 1;

   while(matches->end<target && more){
      more = advanceN(matches,1);
//      printf("advanceToIncludeOrBeyond: target %d: %d, %d \n<br>",target,matches->begin,matches->end); fflush(stdout);
      }
   return(more);
}

int advanceMergedToIncludeOrBeyond(merge_cursor* matches, int target) {
   // move match "cursor" forward until target between begin and end
   // set begin and end to values of resulting match
   // return 0 iff failed to match
   int more = 1;

   while(matches->end<target && more){
      more = advanceMergedN(matches,1);
//      printf("advanceMergedToIncludeOrBeyond: target %d: %d, %d \n<br>",target,matches->begin,matches->end); fflush(stdout);
      }
   return(more);
}

int advanceToBeyond(cursor* matches, int target, int* prevBegin, int* prevEnd) {
   // move match "cursor" forward until begin beyond target
   // set begin and end to values of resulting match
   // return 0 iff failed to match
   // set prevBegin and preEnd to previous match, or to -1 if already beyond
   *prevBegin = -1;
   *prevEnd = -1;

   int more = 1;
   while(matches->begin<target && more) {
      *prevBegin = matches->begin;
      *prevEnd = matches->end;
      more = advanceN(matches,1);
//      printf("advanceToBeyond: target %d: %d, %d \n<br>",target,matches->begin,matches->end); fflush(stdout);
      }
   return(more);
}

int advanceMergedToBeyond(merge_cursor* matches, int target) {
   // move match "cursor" forward until begin beyond target
   // set begin and end to values of resulting match
   // return 0 iff failed to match

   int more = 1;
   while(matches->begin<target && more) {
      more = advanceMergedN(matches,1);
//      printf("advanceMergedToBeyond: target %d: %d, %d \n<br>",target,matches->begin,matches->end); fflush(stdout);
      }
   return(more);
}
