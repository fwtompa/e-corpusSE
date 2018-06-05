Create CGI script to search the corpus
 
Uses Dragoman to display tagged text according to various styles.
   NOTE: file tags.h should be a softlink to ../tags/tags.h rather than a copy.

// functions in main.c
   static void header();
      // print out web page header
   static void trailer(int lang);
      // print out web page trailer
   void   main(int argc, char *argv[]);
      // Search for entries containing a word or phrase.
      // ---initialize http protocol.
      // ---settings of search string, display format, manuscript to be searched in,
      //    number of entries to return.
      // ---form Sgrep command according to the previous settings.
      // ---read in the designated match (using get_match)
      // ---convert it to HTML using tags_convert
      // ---prepare for follow-on query

// functions in error.c
   void  cgi_error(char* msg, char* y);
      // print error message and halt
   char* msgs_lookup(char* msg_id, int lang);
      // find error message in table of messages
   
// functions in  util.c
      // Source: NCSA HTTPd

   void          getword(char *word, char *line, char stop);
   char          *makeword(char *line, char stop);
   unsigned char *fmakeword(FILE *f, char stop, int *cl);
   unsigned char x2c(unsigned char *what);
   void          unescape_url(unsigned char *url);
   void          plustospace(unsigned char *str);
   void          spacetoplus(char *str);
   int           rind(char *s, char c);
   void          send_fd(FILE *f, FILE *fd)
   int           ind(char *s, char c);
   void          escape_shell_cmd(char *cmd);

   char*         url_encode (unsigned char* s);
      //  Source: http://www.w3.org/International/URLUTF8Encoder.java 

// functions in util2.c 
   unsigned char* http_val (char* word, int m);
      // read the value of an http parameter
   void           http_val_dump (int m, FILE* out);
      // read all http parameter-value pairs
   void           http_printf (char* text);
      // print something through http (encoding appropriately)
   void           semi2comma(unsigned char *str);
      // change all semicolons to commas in str
   char*          decodeUTF8(unsigned char* str);
      // convert UTF8 to ascii or  character entrefs
   void           writeLog(int m);
      // write a log record showing access
   int            numWords(char* word);
      // return the number of words, ie, ones delimited with op codes, +,-,|
   char*          orthonormal(char* in, int align);
      // convert word to normalized form using Latin rules
   void           adjustOrtho(char* word);
      // handle searches such as "philosophia" to also match "philosophiae" (normalized)
   char*          normalize(char* word, int indexed);
      // remove all the spaces, except the ones in between double quotes or braces,
      // convert everything to lower case
      // convert parentheses inside quotes and all commas and apostrophes to codes
      // if searching an indexed text, apply orthographic normalization
   int           nestingCheck(char* word,int* numWords);
      // checks whether the double quotes and brackets are balanced,
      // op chars not nested and properly placed, ignores spaces

// functions in queryAssist.c
   char* constructQuery(char* word, char* macro, int numWords, char* STRING_SUB_PROG, char* useMacro);
      // convert a string like caput+sicut and macro name like TITLE_MATCH into
      //  sgrep pattern: TITLE_MATCH1("caput") equal TITLE_MATCH1("sicut")
      //  The command executed for running this perl script is like this:
      //  echo 'caput+sicut' | ./stringSub.plx TITLE_MATCH1 1
      // returns NULL if nothing to search for
   char* adjustQuery(char* instances, char* command);
      // insert instances into sqrep command
      //  e.g., adjustQuery(" -e 'P_MATCH("veni")'") -> " -e '("veni") in P_MATCH("veni")'"
   int   unfinishedXML(char* letters);
      // returns i - if finds an unfinished XML tag starting at letters[i-1]
      //         0 - if can't find any such tag
   char* untag(char* text, int pos);
      // remove start tag symbol at text[pos-1]
   int   unfinishedTag(int begin, int end, cursor* hit);
      // returns the index of the end of the tag if match found WITHIN a tag
      //         0 otherwise 
   char* constructRegion(int begin, int end, cursor* hit, int fnum);
      // insert match tags
   char* completeXML(char* tagged);
      // prepend opening tags on the front and append closing tags on the end to make fragment well-formed XML
   void  encompassWord(cursor* doc, int size);
      // move cursor's begin and end to surround whole word but not past size
   
// functions in engine.c
   long int      engine_open(char* doc, cursor* datafile);
      // prepare to search file
   void          engine_close(cursor* datafile);
      // close search file
   void          engine_idx_close(merge_cursor* datafile);
      // close secondary file
   char*         get_match(int begin, int end, FILE* data);
      // retreive the data from begin to end
   int           engine_count(char* wordQuery, char* doc, char* idx_query);
      // return how many matches; use supplementary file if idx_query <> NULL
   cursor*       engine_findall(char* wordQuery, char* doc, int indexed);
      // find all matches to query
   merge_cursor* engine_idx_findall(char* wordQuery, char* doc, char* idx_query);
      // find all matches to query using secondary file too
   int           advanceN(cursor* matches, int count);
      // move forward "count" matches; return 0 if not found
   int           advanceMergedN(merge_cursor* matches, int count);
      // move forward "count" matches is main or secondary file; return 0 if not found
   int           advanceToIncludeOrBeyond(cursor* matches, int target);
      // move forward to match that includes target (or beyond); return 0 if not found
   int           advanceMergedToIncludeOrBeyond(merge_cursor* matches, int target);
      // move forward to match that includes target of beyond in context of secondary file;
      // return 0 if not found
   int           advanceToBeyond(cursor* matches, int target, int* prevBegin, int* prevEnd);
      // move cursor past the target; return 0 if not found
   int           advanceMergedToBeyond(merge_cursor* matches, int target);
      // move merged cursor past the target; return 0 if not found
   
// functions in initialize.c
      // read data from files for table-driven behaviour
   void  msgs_initialize ();
   void  texts_initialize ();
   void  scopes_initialize();
   void  directories_initialize();
   void  browser_initialize();
   // helper routines
   char* converter_read_in(char *fname)
      // read the file
   char* converter_chop(char **s, char c)
      // break on tabs and newlines
   
   int   http_initialize (void);
      // prepare for http reading and writing
   int initialize_get (void);
   int initialize_post (void);
   void initialize_output (void);
      // print "content" line for http

