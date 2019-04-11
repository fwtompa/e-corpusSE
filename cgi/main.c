//FILE: main.c
//LOCATION: "~/codeBase/src/cgi/"

#include <stdio.h>
#include <errno.h>
#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "util.h"
#include "cgi.h"

#include <ctype.h>

static void header()
{
        // NB. Mime requires blank line after content type
        printf("Content-type: text/html; charset=utf-8\n\n");
		//mainly for the char representation for Albertus
        printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n");
        printf("<HTML>\n<BODY>\n"); fflush(stdout);
}

static void trailer(int lang)
{
        printf("<!-- begin trailer -->\n");
        printf("<hr>\n");
        printf("<p>%s<br/>\n", msgs_lookup("trailer",lang));
        printf("%s %s %s\n",msgs_lookup("compiled",lang), __DATE__, __TIME__);
        printf("<!-- end trailer -->\n");
        printf("</BODY>\n</HTML>\n"); fflush(stdout);
}


void main(int argc, char *argv[]) {

   int i; // temp
   int m = 0;

#ifdef timing
   clock_t start_time, text_start_time;
   struct timeval  wall_start_time, wall_text_start_time;
   struct timeval  wall_end_time, wall_text_end_time;
#endif



   int findex = 1, count, q = 0, s=1;
   int printstate;
   char* p = NULL; //used to represent the matched text
   char* snippet = NULL;

   char* p1;///
   char* match;
   char* p2;

   unsigned char* format;
   char* word = NULL;
   int wIndex;  //..added, for word character index
   char* pos = NULL;

   char* save_word = NULL;

   char* scope;
   int sindex; // index of the scope (macro) to use for searching
   char* macro; // points to macro name for scope of search

   char* start;
   char* newstart;
   char* scroll;
   char* quantity;
   char* search;

   int begin=-1;
   int end=-1;
   int total=0;
   int foundRegion=0;
   int foundMatch=0;

   int matchMoved;
   int nextMatchNum;
   int nextMatchBegin=-1;
   int nextMatchEnd=-1;//

   int num = 0;
   int lang = 0;
   char* language;
   int excl = 0;
   char* exclude;
   char* include;
   char* normalized;
   int norm = 0; // boolean to indicate that normalization should be used

   cursor* datafile = malloc(sizeof(cursor)); //..added
   long int fileSize;


   //added variables
   cursor * pipeCount = NULL;  //..to contain the "count" of matches
   merge_cursor * pipeRegions = NULL; //..to contain the pairs of region matches(offsets)
   merge_cursor * pipeActual = NULL; //..to contain the pairs of term matches
   cursor * pipePage = NULL; //..to contain the page, if browsing
   cursor * pipeLocTags = NULL; //..and pairs of matches of tags with loc attributes
   cursor * pipeLocVals = NULL; //..and pairs of matches of loc values

   char* location = NULL; //..to store the location string,everytime
   int foundLoc=0;
   int prevBegin = -1;
   int prevEnd = -1;
   int printBegin;
   int printEnd;

   int tempIndex;
   int pstart;
   int pend;
   int textsSelected=0;
   int currTextIndex=0;

   char *show = NULL; // identifies the text to display in whole
   char *chosenTexts; // identifies the text(s) to search or browse
   char* browse = NULL; // identifies locVal within text for browsing
      //		non-null		null
      // chosenTexts	which text(s)		just counts
      // browse		where in chosenTexts	search
   char* browseWord; // search term to use to find page when browsing

   char* regionQuery = NULL; // all regions including matches
   char* termQuery = NULL; // all matches anywhere
   char* instQuery = NULL; // all matches with regions
   char* browseQuery = NULL;
   char* regionxQuery = NULL; // corresponding queries for use against index
   char* termxQuery = NULL;
   char* instxQuery = NULL;

   char *matchedTexts = NULL; // identifies the text(s) matched
   int numTextsMatched = 0;
   int totalMatches = 0;
   int* textMatches = NULL; // to collect numbers of matches
   int totalInstMatches = 0;
   int* textInstMatches = NULL; // to collect numbers of matching instances
   int moreMatches = 0;
   int numWords=0;
   char* canonWord = NULL; // canonical form of word
   char* normxWord = NULL; // normalized index word

   int lastLoop=0;   //boolean check variable used at one place only
   int lastActualMatch=0; //boolean check variable used at one place only
   int lastIncomplete = 0;

   int errNum = 0;





// start main code

   // ---------------------------------------------------------------
   // initialize data
   // ---------------------------------------------------------------

#ifdef timing
   start_time = clock();
   gettimeofday(&wall_start_time, NULL);
#endif

   header();
   directories_initialize(); //..to initialize the REQUIRED directory locations
   freopen(LOG_FILE, "w", stderr ); // supress error messages!
   msgs_initialize();    // initialize the message/dialog table
				//..fills in -> converter* msgs_tbl
   texts_initialize();    // initialize the text files table
				//..fills in converter_2* texts_tbl
   m = http_initialize();      //.. fills in: name_val entries[MAX_ENTRIES]
   scopes_initialize(); //..fills in: cgi_scope_val *cgi_scopes

   writeLog(m);  // writre out a log record

   char *textID;
   int textIndexCheck[texts_tbl_len];  // must be after "texts_initialize()"

   tags_initspecs(SPEC_NAMES,SPECS_DIR);
   tags_initchars(CHAR_CODES); 





   // ---------------------------------------------------------------
   //  read parameters
   // ---------------------------------------------------------------

   // read HTTP parameter values
//   printf("TESTING: "); http_val_dump(m,stdout); printf("<BR/>\n"); fflush(stdout);

   language = http_nval("language",m); // numeric //..frm util.h - util2.c
   /* ignore language */ lang = /*(language) ? atoi(language) :*/ 0; // use English (first language) by default

   // choose which format to use for displaying text
   format = tags_char2ent(http_val("format",m),1);
   if (!format) {
      format = (char*) malloc (sizeof(char));
      format[0] = '\0';
      }
   for(findex=0; (strcmp(format,tags_formats[findex].descr[lang])) 
      && (strcmp(tags_formats[findex].descr[lang],""));findex++) ;

   exclude = http_nval("exclude",m); // numeric //..frm util.h - util2.c
   excl = (exclude) ? atoi(exclude) : 0;
   if (excl != 0) excl = 1; // can only be 0 or 1
   include = http_nval("include",m); 
   if (include) excl = 1 - excl; // complement the exclusion

   normalized = http_nval("normalized",m); // numeric //..frm util.h - util2.c
   norm = (normalized) ? atoi(normalized) : 0;
   if (norm != 0) norm = 1; // can only be 0 or 1

   newstart = http_nval("newstart",m); // numeric

   search = tags_char2ent(http_val("search",m),1);  // used a search button
   if (search && strcmp(search,msgs_lookup("cont",lang)) == 0) scroll = search;
   else {
          scroll = tags_char2ent(http_val("scroll",m),1);
          if (scroll) {
              if (strcmp(scroll,msgs_lookup("back",lang)) != 0) scroll = msgs_lookup("more",lang);
          }
   }

   if (!search) {
      browse = tags_char2ent(http_val("browse",m),1); // else browse
      if (!browse && scroll && strcmp(scroll,msgs_lookup("more",lang)) == 0) // perhaps forward browsing only
         browse = tags_char2ent(http_val("browsefor",m),1);
      }
   else if (strcmp(search,msgs_lookup("exactMatch",lang)) == 0) norm = 0;
   else if (strcmp(search,msgs_lookup("normalize",lang)) == 0) norm = 1;
   else search = msgs_lookup("cont",lang);
   // else continue button or no button selected => leave normaization unchanged

   start = http_nval("start",m); // numeric
   s = (start && !browse) ? atoi(start) : 1;

   quantity = http_nval("quantity",m); // numeric
   q = (browse) ? 1 : ( (quantity) ? atoi(quantity) : 10 );
   if (q<=0||q>50) q = 10;

   if (scroll) {
      if (!browse) { // browse always starts with 1
         if (strcmp(scroll,msgs_lookup("back",lang)) == 0) s -= q;
         else if (strcmp(scroll,msgs_lookup("more",lang)) == 0) s += q;
         else s = (newstart) ? atoi(newstart) : 1;
         if (s < 1) s = 1; // do not allow negative starts
         }
      else { // if browsing forward, use the forward position
         if (strcmp(scroll,msgs_lookup("more",lang)) == 0) {
            free(browse);
            browse = tags_char2ent(http_val("browsefor",m),1); // ensure forward
            }
         }
      }

   if (search && 
	(strcmp(search,msgs_lookup("exactMatch",lang)) == 0   // starting a new search
	 || strcmp(search,msgs_lookup("normalize",lang)) == 0) )
	s = 1;

   scope = tags_char2ent(http_val("scope",m),1);
   if (!scope) { // make sure scope is defined
//     printf("TESTING: default scope<br/>"); fflush(stdout);
      scope = (char*) malloc (sizeof(char));
      scope[0] = '\0';
      }
   // else plustospace(scope);
   // and set the scope number
   for(sindex=0; (strcmp(scope,cgi_scopes[sindex].descr[lang])) 
      && (strcmp(cgi_scopes[sindex].descr[lang],""));sindex++) ;
   macro = cgi_scopes[sindex].name; // use that scope for searches

   pos = http_nval("pos",m); // offset of search in text
   if (pos) { // cannot have both pos and word
      i = atoi(pos); // make sure it's an integer
      word = malloc((strlen("\"[(,)]\"") + 2*strlen(pos) + 1)*sizeof(char));
      sprintf(word,"\"[(%d,%d)]\"",i,i);
      }
   else word = tags_char2ent(http_val("word",m),1);

   show = http_val("show",m); // name of text, if displaying outline
   if (show) { // specific text to display
      chosenTexts = decodeUTF8(show);
      if (chosenTexts != show) { // check whether the old char* needs to be freed
         free(show);
         show = chosenTexts;
         }
      for(currTextIndex=0;currTextIndex<texts_tbl_len;currTextIndex++) {
         if (!strcmp(show,texts_tbl[currTextIndex].display_text[lang])) break;
         }
//      printf("TESTING: currTextIndex=%d,texts_tbl_len=%d ",currTextIndex,texts_tbl_len); fflush(stdout);
      if (currTextIndex < texts_tbl_len) {  // valid text name
         chosenTexts = malloc(4*sizeof(char));
         sprintf(chosenTexts,"%d",currTextIndex);
         for(sindex=0; strcmp(cgi_scopes[sindex].descr[lang],""); sindex++) /* use default scope -- should cover whole text */;
//         printf("TESTING: currTextIndex=%d, chosenTexts=%s, format=%s, findex=%d, sindex=%d<BR/>\n",currTextIndex,chosenTexts,format,findex,sindex); fflush(stdout);
         }
      else {
         printf("%s: %s<br/>\n",msgs_lookup("invalidText",lang),tags_char_encode(show));
         free(show);
         show = NULL;
         }
      }
   else chosenTexts = tags_char2ent(http_val("chosenTexts",m),1); // sequence of numerics

   if (chosenTexts) {
//     printf("TESTING: chosenTexts = %s<br/>",chosenTexts);
      if (chosenTexts[strlen(chosenTexts)-1] != ';') { // just one text
         currTextIndex = atoi(chosenTexts);
         for(tempIndex=0;tempIndex<texts_tbl_len;tempIndex++) 
            textIndexCheck[tempIndex] = (tempIndex==currTextIndex);
         textsSelected = 1;
      } else { 
         for(tempIndex=0;tempIndex<texts_tbl_len;tempIndex++) 
            textIndexCheck[tempIndex] = 0;
         for(pstart=0;pstart<strlen(chosenTexts);pstart++) {
            // pick off text indexes
            tempIndex = atoi(chosenTexts+pstart);
            textIndexCheck[tempIndex] = 1;
            textsSelected++;
            for(pstart+=1;chosenTexts[pstart]!=';' && chosenTexts[pstart];pstart++); // advance to end
            }
      }
   } else {  //searching multiple texts
      if (newstart) { // show snippets if used button on snippets page
         chosenTexts = malloc((texts_tbl_len*strlen("999;")+1)*sizeof(char)); 
         chosenTexts[0] = '\0'; // prepare to collect matched text numbers
      }
      textID = (char*)malloc((strlen("textID[999]")+1)*sizeof(char));
      for(tempIndex=0;tempIndex<texts_tbl_len;tempIndex++) {
         sprintf(textID,"textID[%d]",tempIndex);
         if(http_val(textID,m)){
            textIndexCheck[tempIndex] = 1;
            textsSelected++;
            if (newstart) {
               sprintf(textID,"%d;",currTextIndex);
               strcat(chosenTexts,textID);
            } else; // nothing
         }else
            textIndexCheck[tempIndex] = 0;
      }
      if(textsSelected==0){ // search all texts if none explicitly selected
         for(tempIndex=0;tempIndex<texts_tbl_len;tempIndex++){
            textIndexCheck[tempIndex] = 1;
            textsSelected++;
         }
      }
   }








   // ---------------------------------------------------------------
   // Start producing the page .....
   // ---------------------------------------------------------------

   printf("%s\n", msgs_lookup("header",lang));
   fflush(stdout);

   if (word) {
      save_word = (char*) malloc ((strlen(word)+1)*sizeof(char));
      strcpy(save_word,word);
   } else {
      save_word = (char*) malloc (sizeof(char));
      save_word[0] = '\0';
   }

   printf("<TITLE>%s: %s</TITLE>\n",msgs_lookup("title",lang),(word&&!pos)?tags_char_encode(word):"??");//ff

   // determine what to search for
   if (browse) { // a specific page
      browseWord = (char*)malloc((strlen(browse)+strlen("\"\"")+1)*sizeof(char));
      sprintf(browseWord,"\"%s\"",browse);
//      printf("TESTING: browseWord = %s<br/>",browseWord);fflush(stdout);
      }
   if ((!word) || (strlen(word) == 0) ) {
      if (!browse && !show) {
         printf("<P align=center><em>%s</em></P>\n",msgs_lookup("instruct",lang));  //ff
         printf("<P>%s", msgs_lookup("searchOptions",lang));
         printf("<TABLE>");
         printf("<TR><TD VALIGN='TOP'>1.</TD><TD>%s</TD></TR>", msgs_lookup("unnormalized",lang));
         printf("<TR><TD VALIGN='TOP'>2.</TD><TD>%s</TD></TR>", msgs_lookup("normalized",lang));
         printf("</TABLE></P>\n");
         }
      if (word) free(word);
      word = NULL;
      }
   else if (pos) {
      numWords = 1; // of form [(ddd,ddd)] but treat as one lookup
      }
   else if (!nestingCheck(word,&numWords)) {
		// set numWords as a side effect (!!)
      printf("<P>%s: %s\n",tags_char_encode(word), msgs_lookup("nesting",lang));//ff
      free(word);
      word = NULL;
      }
   else { // searching texts
         count = strlen(word)-1;   //position of the last character
         if (2*count+22+4 > MAX_SEARCH) {
            if (count+24+2 > MAX_SEARCH) {
                  printf("<P>%s: %s\n",tags_char_encode(word), msgs_lookup("tooLong",lang));//ff
			// FWT: correction doesn't work with quotes and ops
                  word[MAX_SEARCH-2-24+1] = '*';  // .. add these parts
                  word[MAX_SEARCH-2-24+2] = '\0';
                  count = strlen(word)-1;
            }else {
                  word[++count] = '*';
                  word[count+1] = '\0';
            }
         }
   }






   // ---------------------------------------------------------------
   // prepare to search and display matches
   // ---------------------------------------------------------------

   if ((browse || word) && !include) { // only search if there's something specified
      tags_initialize(show?OUTLINE_FORMAT:tags_formats[findex].name,SPECS_DIR);
//      printf("TESTING: tags initialized;<br/>"); fflush(stdout);
      printstate = printoff;   //save printing initial state (from tags.h)

      if (pos) { // do not normalize position requests
         canonWord = malloc((strlen(word)+1)*sizeof(char));
         strcpy(canonWord,word);
         }
      else if (word) {
         if (norm) {
            normxWord = (char*)normalize(word,1);
            canonWord = (char*)normalize(word,2);
            }
         else canonWord = (char*)normalize(word, 0);
         }
      else canonWord = NULL;  // browsing with no word

      if (browse) {
         // construct query to find page to view
         for(sindex=0; strcmp(cgi_scopes[sindex].descr[lang],""); sindex++) /* find default scope -- should cover whole text */;
         browseQuery = (char*)constructQuery(browseWord,cgi_scopes[sindex].name,1,STRING_SUB_PROG,"1");
//         printf("TESTING: browseQuery=%s<br>",browseQuery); fflush(stdout);
         }

      if (canonWord) {
         regionQuery = (char*)constructQuery(canonWord,macro,numWords,STRING_SUB_PROG,"1");
         if (norm) regionxQuery = (char*)constructQuery(normxWord,macro,numWords,STRING_SUB_PROG,"1");

         if (pos) { // remove quotation marks from ...("[(ddd,dddd)]")...
            for(i=0;i<strlen(regionQuery);i++)
               if (regionQuery[i] == '"') regionQuery[i] = ' ';
            }
//         printf("TESTING: canonWord=%s, regionQuery=%s<BR/>",canonWord,regionQuery); fflush(stdout);
//         if (norm) printf("TESTING: normxWord=%s, regionxQuery=%s<BR/>",normxWord,regionxQuery); fflush(stdout);
         }

   // ---------------------------------------------------------------
   //  now search
   // ---------------------------------------------------------------

      if (regionQuery) { // get the objects to search for
         termQuery = (char*)constructQuery(canonWord,SEARCH_MATCHES,numWords,STRING_SUB_PROG,"0");
         if (norm) termxQuery = (char*)constructQuery(normxWord,SEARCH_MATCHES,numWords,STRING_SUB_PROG,"0");

         if (pos) { // remove quotation marks
            for(i=0;i<strlen(termQuery);i++)
               if (termQuery[i] == '"') termQuery[i] = ' ';
            }
         instQuery =adjustQuery(termQuery,regionQuery); // insert the instance match piece
         if (norm) instxQuery =adjustQuery(termxQuery,regionxQuery); // insert the instance match piece
//         printf("TESTING: termQuery=%s, instQuery = %s<br/>",termQuery,instQuery); fflush(stdout);
//         if (norm) printf("TESTING: termxQuery=%s, instxQuery = %s<br/>",termxQuery,instxQuery); fflush(stdout);

         if(norm) printf("<P>%s</P>\n", msgs_lookup("normalized",lang)); // headings
         else printf("<P>%s</P>\n", msgs_lookup("unnormalized",lang));

         if(canonWord) free(canonWord);
         canonWord = NULL;
         if(normxWord) free(normxWord);
         normxWord = NULL;
         }

   // ---------------------------------------------------------------
   // first, code for searching with no snippets, just counts
   // ---------------------------------------------------------------
      if(regionQuery && !chosenTexts) { // if nothing to count, skip counts with no snippets 

         for (currTextIndex=0;currTextIndex<texts_tbl_len;currTextIndex++)
            printf("_");
         printf("<br/>");
         matchedTexts = malloc((textsSelected*strlen("999;")+1)*sizeof(char)); 
         matchedTexts[0] = '\0'; // prepare to collect matched text numbers
         textMatches = malloc(texts_tbl_len*sizeof(int));
         textInstMatches = malloc(texts_tbl_len*sizeof(int));

         currTextIndex = 0; // start from first text
         for(tempIndex=0;tempIndex<textsSelected;tempIndex++) {
//            printf("TESTING: matched text number = %d ",tempIndex); fflush(stdout);
            for(currTextIndex;!textIndexCheck[currTextIndex];currTextIndex++) {
   										// find next text to search
               printf("*"); fflush(stdout);
               textMatches[currTextIndex] = 0; // mark no matches in this text
               textInstMatches[currTextIndex] = 0; // mark no matches in this text
               }
//            printf("TESTING: text %d %s<br/>",currTextIndex,texts_tbl[currTextIndex].extraInfo); fflush(stdout);
            if (excl && strcmp(texts_tbl[currTextIndex].extraInfo,"excludable")==0) {
               textMatches[currTextIndex] = 0; // mark no matches in this text
               textInstMatches[currTextIndex] = 0; // mark no matches in this text
               printf("*"); fflush(stdout);
               currTextIndex++;
               continue;
               }

            // ---------------------------------------------------------------
            // count number of matching regions and number of instances
            // ---------------------------------------------------------------
#ifdef timing
            text_start_time = clock();
            gettimeofday(&wall_text_start_time, NULL);
#endif
            textMatches[currTextIndex] = engine_count(regionQuery, texts_tbl[currTextIndex].internal_name, regionxQuery);
#ifdef timing
            gettimeofday(&wall_text_end_time, NULL);
            texts_tbl[currTextIndex].CPU_time = ((double)(clock() - text_start_time)) / CLOCKS_PER_SEC;
            texts_tbl[currTextIndex].wall_time = (double) (wall_text_end_time.tv_usec - wall_text_start_time.tv_usec) / 1000000 
                                                 + (double) (wall_text_end_time.tv_sec - wall_text_start_time.tv_sec);
#endif

            if (textMatches[currTextIndex]) { // some matches 
               numTextsMatched++;
               totalMatches += textMatches[currTextIndex]; // accumulate count               
               textInstMatches[currTextIndex] = engine_count(instQuery, texts_tbl[currTextIndex].internal_name, instxQuery);
               totalInstMatches += textInstMatches[currTextIndex]; // accumulate count               
               sprintf(textID,"%d;",currTextIndex);
               strcat(matchedTexts,textID);
               }
            else textInstMatches[currTextIndex] = 0;

            currTextIndex++;  //loop ends
            printf("*"); fflush(stdout);

            } // end of multi-text search loop
         for (currTextIndex; currTextIndex < texts_tbl_len; currTextIndex++){
            printf("*"); fflush(stdout);
            textMatches[currTextIndex] = 0; // no matches in remainder of texts
            textInstMatches[currTextIndex] = 0; // no matches in remainder of texts
            }
         printf("</tt>");
         if (!numTextsMatched)
            printf("<p>%s %s</p>\n",msgs_lookup("noMatch",lang),msgs_lookup("inAllTexts",lang));
         else {
            printf("<br/>");
            if (numTextsMatched > 1) { // matches in more than one text 
               printf("<p><i>%d (%d) %s",totalMatches, totalInstMatches, msgs_lookup("nMatches",lang));
               printf(" <a href=\"%s?chosenTexts=%s",getenv("SCRIPT_NAME"),matchedTexts);
               printf("&normalized=%d&exclude=%d&language=%d&word=%s&newstart=%d",
   					norm,excl,lang,url_encode(save_word),s); 
               printf("&quantity=%s&scope=%s&format=%s\"",
   					quantity,url_encode(scope),url_encode(format)); 
               printf(" target=\"_blank\">");
               printf("%s</a></i>:</p>\n", msgs_lookup("inAllTexts",lang));
            }
            for(currTextIndex=0;currTextIndex<texts_tbl_len;currTextIndex++) {
               if (textMatches[currTextIndex]) {
#ifdef timing
                  printf("t=%4.2f sec & ",texts_tbl[currTextIndex].CPU_time);
                  printf("T=%4.2f sec: ",texts_tbl[currTextIndex].wall_time);
#endif
                  if (textMatches[currTextIndex] == 1)
                     printf("1(%d) %s",textInstMatches[currTextIndex],msgs_lookup("oneMatch",lang));
                  else
                     printf("%d (%d) %s",textMatches[currTextIndex],textInstMatches[currTextIndex],
					msgs_lookup("nMatches",lang));
                  printf(" <a href=\"%s?chosenTexts=%d",
   					getenv("SCRIPT_NAME"),currTextIndex);
                  printf("&normalized=%d&exclude=%d&language=%d&word=%s&newstart=%d",
   					norm,excl,lang,url_encode(save_word),s); 
                  printf("&quantity=%s&scope=%s&format=%s\"",
   				quantity,url_encode(scope),url_encode(format)); 
                  printf(" target=\"_blank\">");
                  printf("%s</a><br/>\n",
   				texts_tbl[currTextIndex].display_text[lang]);
               }
            }
         }
         if (textMatches) {
            free (textMatches);
            textMatches = NULL;
            free (textInstMatches);
            textInstMatches = NULL;
         }
      } // end code for searching multiple texts with no snippets




   // ---------------------------------------------------------------
   // now code for showing snippets
   // ---------------------------------------------------------------
      if (chosenTexts && (browseQuery || regionQuery)) { // skip if nothing at all to display
         currTextIndex = 0; // start from first text
         // ... and process the number of texts that are selected
         for(tempIndex=0;tempIndex<textsSelected;tempIndex++) {
//            printf("TESTING: check matched text #%d ",tempIndex); fflush(stdout);
            for(currTextIndex;!textIndexCheck[currTextIndex];currTextIndex++);
   			// find next text to search
            if (excl && strcmp(texts_tbl[currTextIndex].extraInfo,"excludable")==0) {
               currTextIndex++;
               continue;
               }
//            printf("TESTING: search in text %d %s<br/>",currTextIndex,texts_tbl[currTextIndex].extraInfo); fflush(stdout);


            // first open the display file
            fileSize = engine_open(texts_tbl[currTextIndex].internal_name, datafile);
//            printf("TESTING: Display file: %s (size %ld)<BR/>", texts_tbl[currTextIndex].internal_name,fileSize); fflush(stdout);

            // ---------------------------------------------------------------
            // get the number of matching regions
            // ---------------------------------------------------------------
            if (!browse && !show) { // find how many regions match
               total = engine_count(regionQuery, texts_tbl[currTextIndex].internal_name, regionxQuery);
               totalInstMatches = engine_count(instQuery, texts_tbl[currTextIndex].internal_name, instxQuery);

               if (total < s) { // nothing to print
                  currTextIndex++;
                  continue;
               } else {
                  if (total == 1)
                     printf("%d (%d) %s",total,totalInstMatches, msgs_lookup("oneMatch",lang));
                  else printf("%d (%d) %s",total, totalInstMatches, msgs_lookup("nMatches",lang));
                  printf(" <b>%s</b><br/>\n",texts_tbl[currTextIndex].display_text[lang]);
                  moreMatches |= (total >= s+q); // more matches to print
               }
            }
            else if (show) { // show matches in all pages
               browseQuery = (char*)constructQuery("loc=",cgi_scopes[sindex].name,1,STRING_SUB_PROG,"1"); /* search for all pages */
               total = engine_count(browseQuery, texts_tbl[currTextIndex].internal_name, NULL); // get count of all units
               s = 1; // print all pages starting from the first
               q = total; 
            }
            else  {
               total = 1; // when browsing there is only one matching region
               // get the matching page (no normalization of page reference)
               pipePage = engine_findall(browseQuery, texts_tbl[currTextIndex].internal_name, 0);
               if (!advanceN(pipePage, 1)) {
                  printf("No page found for browsing with %s in %s%s<BR/>", tags_char_encode(browseQuery),
                                   (norm?"normalized ":""), texts_tbl[currTextIndex].internal_name);
                  cgi_error("aborting","main");
               }
//               printf("TESTING: Query: [%s] Browsing page: %d to %d<BR/>", browseQuery, pipePage->begin, pipePage->end);
            }
            fflush(stdout);

            // get all location tags
            pipeLocTags = engine_findall(SEARCH_LOC,texts_tbl[currTextIndex].internal_name,0);
            pipeLocVals = engine_findall(SEARCH_LOC_VAL,texts_tbl[currTextIndex].internal_name,0);

            if (regionQuery) {// get all the matching words (for marking them)
               pipeActual = engine_idx_findall(instQuery, texts_tbl[currTextIndex].internal_name, norm?instxQuery:NULL);
//               printf("TESTING: pipeActual: [%s] pipexActual: [%s] <BR/>", instQuery,norm?instxQuery:"n/a");

               // get the actual matching regions
               if (show) {
                  pipeRegions = engine_idx_findall(browseQuery, texts_tbl[currTextIndex].internal_name, NULL); // get the units themselves
//                  printf("TESTING: pipeRegions: [%s] pipexMatches: [%s] <BR/>", browseQuery,"n/a");
                  }
               else {
                  pipeRegions = engine_idx_findall(regionQuery, texts_tbl[currTextIndex].internal_name, norm?regionxQuery:NULL);
//                  printf("TESTING: pipeRegions: [%s] pipexRegions: [%s] <BR/>", regionQuery,norm?regionxQuery:"n/a");
                  }
               }


            // print results

//           printf("TESTING: ready to print %d matches<br/>",total); fflush(stdout);
            if (!browse && !show) { // skip the search results if merely browsing or outlining
               printf("<ol start=%d>\n",s);
//               printf("TESTING: preparing %d of %d;\n<br/>",s,q); fflush(stdout);
               // get the s^th requested matching scope
               foundRegion = advanceMergedN(pipeRegions, s);
//               printf("TESTING: starting at #%d found %d from %d to %d;\n<br/>",s, foundRegion, pipeRegions->begin, pipeRegions->end); fflush(stdout);
               }
            else if (show) {
               foundRegion = advanceMergedN(pipeRegions, 1); // find the first page
               foundMatch = advanceMergedN(pipeActual, 1);
               printoff = printstate; // start with print on or off as indicated
               printf("<TABLE><TR><TD>\n");
               }
            else /* (browse) */ if (regionQuery) { // find the first matching region overlapping browsing page (if any)
               // to be interesting, must have a match within that region and on that page as well
               foundRegion = advanceMergedToIncludeOrBeyond(pipeRegions, pipePage->begin);
//               printf("TESTING: starting at #%d found %d from %d to %d;\n<br/>",s, foundRegion, pipeRegions->begin, pipeRegions->end); fflush(stdout);
               if (foundRegion) foundMatch = advanceMergedToBeyond(pipeActual, pipeRegions->begin);
               else foundMatch = 0;
//               printf("TESTING: foundMatch=%d from %d to %d;\n<br/>",foundMatch, pipeActual->begin, pipeActual->end); fflush(stdout);

               while (foundRegion && pipeActual->end < pipePage->begin) { // match comes before the page
                  foundMatch = advanceMergedN(pipeActual, 1);
//                  printf("TESTING: foundMatch=%d from %d to %d;\n<br/>",foundMatch, pipeActual->begin, pipeActual->end); fflush(stdout);
                  if (!foundMatch) foundRegion = 0;
                  else if (pipeActual->begin > pipeRegions->end) { // match now out of region
                     // find next matching region and match within it
                     foundRegion = advanceMergedN(pipeRegions, 1);
                     if (foundRegion) foundMatch = advanceMergedToBeyond(pipeActual, pipeRegions->begin);
                     else foundMatch = 0;
                     }
                  } // at loop exit, match is first one overlapping page, or beyond it if no matches on page

               }
            else foundMatch = 0;

            if (browse || show || foundRegion) {  // there's something to print
               for (count=s; count<s+q; count++) {                  
//                  printf("TESTING: region # %d; ",count); fflush(stdout);
                  if (!show) { // get location information
                     printoff = printstate; // start printing appropriately each time
   
         	          // get the locations of the matches themselves and loc information, if available
      
                     if (browse) { // actual match already found, if there is one
                        // tag format:   .... <page ... loc=.../> .....
                        // N.B. The start of the page, but not the end of the page, is within the LOC region
                        foundLoc = advanceToIncludeOrBeyond(pipeLocTags, pipePage->begin);
                        if (foundLoc) foundLoc = advanceToBeyond(pipeLocVals, pipeLocTags->begin, &prevBegin, &prevEnd);
                     } else { // in searching there will always be a match within the region
                        foundMatch = advanceMergedToBeyond(pipeActual, pipeRegions->begin);
                        foundLoc = advanceToIncludeOrBeyond(pipeLocTags, pipeRegions->begin);
                        if (foundLoc) foundLoc = advanceToBeyond(pipeLocVals, pipeLocTags->begin,&prevBegin, &prevEnd);
                     }
//                    printf("<br>TESTING: begin=%d end=%d locValBegin=%d locValEnd=%d pipeActual->begin=%d pipeActual->end=%d <br/>\n",regionQuery?pipeRegions->begin:-1,regionQuery?pipeRegions->end:-1,pipeLocVals->begin,pipeLocVals->end,regionQuery?pipeActual->begin:-1,regionQuery?pipeActual->end:-1); fflush(stdout);
                     } // end if (!show)
   
                  if (!browse && !show) { // do not print location info for browsing or outline
                     printf("<li>");
                     if(foundLoc) { // location information available
                        location = get_match(pipeLocVals->begin,pipeLocVals->end,datafile->file);
                        printf("<a href=\"%s?browse=%s&chosenTexts=%d",
      			   getenv("SCRIPT_NAME"),url_encode(location),currTextIndex);
                        printf("&normalized=%d&exclude=%d&language=%d&word=%s&newstart=%d",
                           norm,excl,lang,pos?"":url_encode(save_word),s);
                        printf("&quantity=%s&scope=%s&format=%s\">",
                           quantity,url_encode(scope),url_encode(format));
                        semi2comma(location); // commas in citation format but not in search string
                        printf("%s%s</a><br/>\n",
      				texts_tbl[currTextIndex].display_text[lang],
      				tags_convert(location));
                        if(location) free(location); // malloc'd in get_match
                        location = NULL;
                        }
                     else;  // print nothing else if no location available
                     }
                  else if (browse) {
                     semi2comma(browse); // use commas for printing browsing location
                     printf("<b>%s</b>",texts_tbl[currTextIndex].display_text[lang]);
                     printf("%s<br/>\n",tags_char_encode(browse)); // print browsing location
                     printf("<TABLE><TR><TD>");
                     }
                  fflush(stdout);
                  // Now print actual snippet (first match within region already found if it exists)
                  
                  if (browse) {
                     printBegin = pipePage->begin;
                     printEnd = pipePage->end;
                     }
                   else {
                     printBegin = pipeRegions->begin;
                     printEnd = pipeRegions->end;
                     }
                  while(printBegin <= printEnd) { // each loop prints to the end of each match
                     if(foundMatch && pipeActual->begin < printEnd) { // another match within the printing region
//                        printf("TESTING: found match: (%d,%d) within (%d,%d)<br/>",pipeActual->begin,pipeActual->end,pipeRegions->begin,pipeRegions->end); fflush(stdout);
                        if (pipeActual->begin < printBegin) pipeActual->begin = printBegin; //show a half match if needed when browsing
                        if (pipeActual->end > printEnd) pipeActual->end = printEnd; // truncate if needed when browsing

                        datafile->begin = pipeActual->begin;
                        datafile->end = pipeActual->end;
                        encompassWord(datafile,fileSize); // move matches to encompass whole words
                        // and now make sure it's outside a tag
      		        matchMoved = unfinishedTag(printBegin,printEnd,datafile);
//                        printf("TESTING: moved match begin=%d end=%d datafile->begin=%d datafile->end=%d matchMoved=%d<br>",printBegin,printEnd,datafile->begin,datafile->end,matchMoved); fflush(stdout);
                        if (matchMoved) { // match was inside a tag
                           p = constructRegion(printBegin,matchMoved,datafile,currTextIndex);
                           printBegin = matchMoved + 1; // start again after the tag
                           }
                        else {
                           p = constructRegion(printBegin,datafile->begin-1,datafile,currTextIndex);
                           printBegin = datafile->end + 1; // start again after the match
                           }
			while(foundMatch && pipeActual->begin < printBegin) // perhaps multiple string matches within one matched word
                           foundMatch = advanceMergedN(pipeActual, 1); // get next match
                        /*if (browse && pipeActual->begin > printEnd) { // match now out of page -- so why do we still care?
                           // find next matching region and match within it
                           foundRegion = advanceMergedN(pipeRegions, 1);
                           if (foundRegion)
                              foundMatch = advanceMergedToBeyond(pipeActual, pipeRegions->begin);
                           else foundMatch = 0;
                           }*/
                        }
                     else { // no printable matches in (remainder of) region
                        p = get_match(printBegin,printEnd,datafile->file);
                        printBegin = printEnd + 1; // terminate the loop to print out the region
                        }
                     // collect parts into snippet
                     if (p) {
                        if (snippet) {
                           snippet = realloc(snippet,(strlen(snippet)+strlen(p)+1)*sizeof(char));
                           strcat(snippet,p); // catenate the new piece to end of snippet
                           free(p);
                           }
                        else {
                           snippet = p;
                           }
                        p = NULL;
                        }
                     }//while
   
                  // now print, but first close off all xml tags
                  if (snippet) {
                     if (!show) {
                        completeXML(snippet);  // result is in tags_buffer
                        free(snippet);
                        snippet = malloc((strlen(tags_buffer)+1)*sizeof(char));
                        strcpy(snippet,tags_buffer);  // need to move string out of tags_buffer so that tags_convert can work
                        }
                     printf("%s",tags_convert(snippet)); fflush(stdout);
                     free(snippet);
                     snippet = NULL;
                     }
                  if (browse) {
                     printf("</TD></TR></TABLE>\n");
                     break;  // browsing has only one region, with no headers
                     }
                  if (!show) printf("</li>\n");
                  fflush(stdout);
                  foundRegion =  advanceMergedN(pipeRegions, 1); //get next region
                  if (!foundRegion) break;
                  } // for loop for each region

               }
            if (!browse && !show) printf("</ol>\n"); // no headers for browsing
            else if (show) printf("</TD></TR></TABLE>\n");
            fflush(stdout);

            engine_idx_close(pipeRegions);
            pipeRegions = NULL;
            engine_idx_close(pipeActual);
            pipeActual = NULL;
            currTextIndex++;
            }

         } // end case for searching with snippets




      if(word) free(word);
      word = NULL;
      if(regionQuery) free(regionQuery);
      regionQuery = NULL;
      if(instQuery) free(instQuery);
      instQuery = NULL;
      if(termQuery) free(termQuery);
      termQuery = NULL;
      if(regionxQuery) free(regionxQuery);
      regionxQuery = NULL;
      if(instxQuery) free(instxQuery);
      instxQuery = NULL;
      if(termxQuery) free(termxQuery);
      termxQuery = NULL;
   } // end of processing a specified search or browse request




   // ---------------------------------------------------------------
   // create form to collect parameters for next search
   // ---------------------------------------------------------------
   // putenv("SCRIPT_NAME");
   printf("<FORM METHOD=\"post\" ACTION=\"%s\">\n",getenv("SCRIPT_NAME"));//ff
   printf("<input type=\"hidden\" name=\"start\" value=%d>", s);//ff
   printf("<input type=\"hidden\" name=\"language\" value=%d>", lang);//ff
   printf("<input type=\"hidden\" name=\"exclude\" value=%d>", excl);//ff
   printf("<input type=\"hidden\" name=\"normalized\" value=%d>", norm);//ff

   // output scrolling buttons
   if(!include && chosenTexts) { // no scroll buttons if no text being shown
      if ( s > 1 || (browse && pipeLocTags->begin > 1)) { // able to search or browse backwards
         if (browse) {
            location = get_match(prevBegin,prevEnd,datafile->file);  // loc actually within prior page
            printf("<input type=\"hidden\" name=\"browse\" value=\"%s\">",location);
            free(location);
            location = NULL;
            }
         printf("<input type=\"submit\" name=\"scroll\" value=\"%s\">&nbsp;",msgs_lookup("back",lang));
         }
      if ( moreMatches || (browse && pipePage->end<fileSize)) {  // search or browse forwards
         if (browse) {
            foundLoc = advanceToBeyond(pipeLocVals, pipePage->end, &prevBegin, &prevEnd);
            if (foundLoc) { // there are more locations
               location = get_match(pipeLocVals->begin,pipeLocVals->end,datafile->file);  // next loc
               printf("<input type=\"hidden\" name=\"browsefor\" value=\"%s\">",location);
               free(location);
               location = NULL;
               printf("<input type=\"submit\" name=\"scroll\" value=\"%s\">",msgs_lookup("more",lang));
               }
            engine_close(pipePage);
            pipePage = NULL;
            }
         else
            printf("<input type=\"submit\" name=\"scroll\" value=\"%s\">",msgs_lookup("more",lang));
         }
   
/* HERE -- these pclose stmts cause sgrep to find a broken pipe */
      engine_close(pipeLocTags);
      pipeLocTags = NULL;
      engine_close(pipeLocVals);
      pipeLocVals = NULL;
      engine_close(datafile);
      datafile = NULL;
      }
   
   printf("<HR/><TABLE>");//ff

   // Word
   printf("<TR>");//ff
   printf("<TD>%s</TD>",msgs_lookup("word",lang));//ff
   printf("<TD><input type=\"text\" name=\"word\" value=\"%s\" size=90>&nbsp;",pos?"":tags_char_encode(save_word) );  // disallow quote marks and ampersands
   if (norm)
      printf("%s",msgs_lookup("usingNorm",lang));
   else
      printf("%s",msgs_lookup("usingExact",lang));
   printf("</TR>\n");//ff

   // scope
   printf("<TR>");//ff
   printf("<TD>%s</TD>",msgs_lookup("scope",lang));//ff
   printf("<TD><select name=\"scope\">");//ff

   for(count=0;(strcmp(cgi_scopes[count].descr[lang],""));count++) {
          printf("<option");//ff
           if (count==sindex) {
              printf(" selected");//ff
           }
           printf(">%s\n",cgi_scopes[count].descr[lang]);//ff
   }
   printf("</select></TD>");//ff
   printf("</TR>\n");//ff

   // style sheet
   printf("<TR>");//ff
   printf("<TD>%s</TD>",msgs_lookup("format",lang));//ff
   printf("<TD><select name=\"format\">");//ff

   for(count=0;(strcmp(tags_formats[count].descr[lang],""));count++) {
        printf("<option");//ff
        if (count==findex && !show) { // but don't pass on the outline format
           printf(" selected");//ff
        }
        printf(">%s\n",tags_formats[count].descr[lang]);//ff
   }
   printf("</select></TD>");//ff
   printf("</TR>\n");//ff

   // number of matches
   if (chosenTexts && !show && !browse) { // only makes sense if snippets being shown and not browsing
      printf("<TR>");//ff
      printf("<TD>%s</TD>",msgs_lookup("quantity",lang));//ff
      printf("<TD><input type=\"text\" name=\"quantity\" value=%d size=2></TD></TR>\n",q);//ff

   // continuing from ...
      printf("<TR>");
      printf("<TD>%s</TD>",msgs_lookup("start",lang));//ff
      if (browse) s = (newstart) ? atoi(newstart) : 1;
      printf("<TD><input type=\"text\" name=\"newstart\" value=%d size=2>",((moreMatches) ? s+q : s));
      if (!save_word || (strlen(save_word) == 0)) printf("</TD>");
      else
         printf("&nbsp;<input type=\"submit\" name=\"search\" value=\"%s\"></TD>",msgs_lookup("cont",lang));//ff
      printf("</TR>\n");//ff
   }

   // start buttons
   printf("<TR><TD>&nbsp;</TD>");//ff
   printf("<TD><input type=\"submit\" name=\"search\" value=\"%s\">&nbsp;</TD></TR>\n",msgs_lookup("exactMatch",lang));//ff
   printf("<TR><TD>&nbsp;</TD>");//ff
   printf("<TD><input type=\"submit\" name=\"search\" value=\"%s\">&nbsp;</TD></TR>\n",msgs_lookup("normalize",lang));//ff

   // corpora
   printf("<TR>");//ff
   printf("<TD valign=\"top\">%s</TD>",msgs_lookup("text",lang));//ff

   printf("<TD>\n");

      char *currCheck;
      int csize;
      int rpt;
      int cols = 2; // number of columns for listing texts

      if(!chosenTexts || textsSelected >1) { // no snippets or multiple texts
         csize = texts_tbl_len/cols + ((texts_tbl_len%cols)?1:0); //use columns
         printf("%s",msgs_lookup("allTexts",lang));
         if (excl)
            printf(" <input type=\"submit\" name=\"include\" value=\"%s\">&nbsp;",msgs_lookup("include",lang));
         else
            printf(" <input type=\"submit\" name=\"include\" value=\"%s\">&nbsp;",msgs_lookup("exclude",lang));
         printf("<table>");	// nested table for two-column list
         }
      else { 
         csize = texts_tbl_len; // use 1 listing
         printf("<select name=\"chosenTexts\">");
         }

      for(count=0;count<csize;count++) {
         if(!chosenTexts || textsSelected >1) { // for multiple texts
            printf("<tr>");
            for(rpt = count;rpt<texts_tbl_len;rpt+=csize) {
               if(textIndexCheck[rpt]==1 && textsSelected<texts_tbl_len)
                  currCheck = " checked";
               else currCheck = "";
               printf("<td>");
               if (excl && strcmp(texts_tbl[rpt].extraInfo,"excludable")==0) {
                  printf("<font color=\"LightGray\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s</font>",
                           texts_tbl[rpt].display_text[lang]);
                  }
               else {
                  printf("<input type=\"checkbox\" name=\"textID[%d]\" value=\"%d\"%s>",rpt,rpt,currCheck);
                  printf("<input type=\"submit\" name=\"show\" value=\"%s\">",texts_tbl[rpt].display_text[lang]);
                  }
               printf("</td>\n");
               }
            printf("</tr>");
            }
         else {
            if(textIndexCheck[count]==1 && textsSelected<texts_tbl_len)
               currCheck = " selected";
            else currCheck = "";
            if (!excl || strcmp(texts_tbl[count].extraInfo,"excludable")!=0) 
               printf("<option value=\"%d\"%s> %s<br/>\n", count,currCheck,texts_tbl[count].display_text[lang]);
            }
         }
      if (!chosenTexts || textsSelected > 1) printf("</table>");
      else {
         printf("</select>");
         if (excl)
            printf("<input type=\"submit\" name=\"include\" value=\"%s\">&nbsp;",msgs_lookup("include",lang));
         else
            printf("<input type=\"submit\" name=\"include\" value=\"%s\">&nbsp;",msgs_lookup("exclude",lang));
      }
   printf("</TD>\n"); 
   printf("</TR>\n");

   printf("</TABLE>\n");//ff
   printf("</FORM>\n");//ff

   trailer(lang);
#ifdef timing
   gettimeofday(&wall_end_time, NULL);
   printf("<br/>t=%4.2f sec ; T=%4.2f sec",((double)(clock()-start_time))/CLOCKS_PER_SEC,
                                      (double) (wall_end_time.tv_usec - wall_start_time.tv_usec) / 1000000 
                                                 + (double) (wall_end_time.tv_sec - wall_start_time.tv_sec));
#endif
   fflush(stdout);
   exit(0);

}//end main
