//FILE: initialize.c
//Location: ~/testSource/src/cgi

#include <stdio.h>
#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif
#include <string.h>
#include "util.h"
#include "cgi.h"
#include "tags.h"
#include <sys/types.h>
#include <sys/stat.h>

/* Declare external variables */

name_val entries[MAX_ENTRIES];  //extern variablefrom util .h
converter     *msgs_tbl = NULL; //extern starting from cgi.h
int      msgs_tbl_len = 0;
converter_2     *texts_tbl = NULL;
int      texts_tbl_len = 0;
int      texts_max_len = 500;

//..adding
cgi_scope_val *cgi_scopes = NULL;            //..added....
char   *cgi_scope_file = NULL;
int page_match_index = -1;

browser_obj *browser_tbl = NULL;
char *browser_temp_file = NULL;
int browser_tbl_len = 0;//..extern varaibles upto here




char *dir_file = NULL;


void initialize_output (void){
    printf("Content-type: text/html%c%c",10,10);
}

int initialize_get (void){
    register int x,m=-1;
    char *cl=NULL;

    cl = getenv("QUERY_STRING");
    if(cl == NULL) {
        printf("No query information to decode.\n");
        exit(1);
    }

    for(x=0;cl[0] != '\0';x++) {
        m=x;
	entries[x].val = (char*) malloc((strlen(cl)+1)*sizeof(char));
        getword(entries[x].val,cl,'&');
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
        entries[x].name = makeword(entries[x].val,'=');
    }
    return(m);
}

int initialize_post (void){
    register int x,m=-1;
    int cl;

    if(strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded")) {
        printf("This script can only be used to decode form results. \n");
        exit(1);
    }
    cl = atoi(getenv("CONTENT_LENGTH"));

    for(x=0;cl && (!feof(stdin));x++) {   // EOF may be set inside fmakeword
        m=x;
        entries[x].val = fmakeword(stdin,'&',&cl);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
        entries[x].name = makeword(entries[x].val,'=');
    }
    return(m);
}

int http_initialize (void){           // .. returns the value for "int m" in the "lookup.c" at first

  //initialize_output();

    /* putenv("REQUEST_METHOD"); */
    if(!strcmp(getenv("REQUEST_METHOD"),"POST"))
	return(initialize_post());
    else if(!strcmp(getenv("REQUEST_METHOD"),"GET"))
	return(initialize_get());
    else {
        printf("This script only works with a METHOD of POST or of GET.\n");
        printf("(called with '%s')\n",getenv("REQUEST_METHOD"));
        exit(1);
    }
}

char* converter_read_in(char *fname)
{
   FILE           *fin;
   char           *mem;
   struct stat     buf;

   if (stat(fname, &buf) != 0)
        cgi_error("read_in: Cannot access messages",fname);
   fin = fopen(fname, "r");
   if (fin == NULL)
        cgi_error("read_in: Cannot open the messages file",fname);
   mem = (char*) malloc((buf.st_size + 1)*sizeof(char));
   fread(mem, 1, buf.st_size, fin);
   mem[buf.st_size] = '\0';     /* Ensure it can be accessed as a string */
   return (mem);
}

char* converter_chop(char **s, char c)
{
    char* save;

    save = *s;
    while( ((*s)[0] != '\0') && ((*s)[0] != c) )
        (*s) += 1;

    if( (*s)[0] == c ) {
        (*s)[0] = '\0';
       	(*s) += 1;
    }
    return save;
}

void msgs_initialize ()
{
   char  *parse;
   char	 *msgs_file;

   /* initialize the message/dialog table */

   msgs_tbl = (converter*) malloc (MAX_MSGS*sizeof(entry));

   msgs_file = converter_read_in(MSGS_FILE);    // .. method defn above  .. MSGS_FILE -> directories_initialize()

   parse = msgs_file;	/* prepare to break into smaller substrings */
   while (parse[0] && (msgs_tbl_len < MAX_MSGS)) {
	msgs_tbl[msgs_tbl_len].internal_name = converter_chop(&parse,NEWLINE);
	msgs_tbl[msgs_tbl_len].display_text[EN] = converter_chop(&parse,NEWLINE);
	msgs_tbl[msgs_tbl_len++].display_text[FR] = converter_chop(&parse,NEWLINE);
	if (parse[0] == NEWLINE) parse++; /* skip the empty separator line */
	}
}

void texts_initialize ()
{
   char  *parse;
   char	 *texts_file;

   /* initialize the text files table */

   texts_tbl = (converter_2*) malloc (MAX_TEXTS*sizeof(converter_2));    //..texts_tbl, an array of converters, dcl in util.h

   texts_file = converter_read_in(TEXTS_FILE); //..the entire file, aka "textindex" saved as string in texts_file

   texts_max_len = 0;
   parse = texts_file;	/* prepare to break into smaller substrings */

    while (parse[0] && (texts_tbl_len < MAX_MSGS)) {

    texts_tbl[texts_tbl_len].internal_name = converter_chop(&parse,NEWLINE); //..returns upto newline, for every texts_table entries, from "parse" aka ..texts_file

    if (strlen(texts_tbl[texts_tbl_len].internal_name) > texts_max_len)
		texts_max_len = strlen(texts_tbl[texts_tbl_len].internal_name);

	texts_tbl[texts_tbl_len].display_text[EN] = converter_chop(&parse,NEWLINE);   //..the english line is stored, for displaying, from "textindex"
	texts_tbl[texts_tbl_len].display_text[FR] = converter_chop(&parse,NEWLINE); //..the french line is stored, for displaying, from "textindex"
	
	if(parse[0] != NEWLINE){  //only present in "textindex" file of "Albertus" project..not for "Margot Project", to print extra info beside the text name
		texts_tbl[texts_tbl_len].extraInfo = converter_chop(&parse,NEWLINE);
	}else{
		texts_tbl[texts_tbl_len].extraInfo = (char*)malloc(sizeof(char));
		(texts_tbl[texts_tbl_len].extraInfo)[0] = '\0';
	}
	texts_tbl_len++;

	
	if (parse[0] == NEWLINE) parse++; /* skip the empty separator line */
	}
}

void scopes_initialize()
{
   char           *parse;
   char           *temp;

   char            scope_filename[256];
   int             scopes_len = 0; //...tags_formats_len = 0;

   //* now initialize the scope table * / //.. ie, cgi_scope_val *cgi_scopes;

   cgi_scopes = (cgi_scope_val*) malloc (MAX_SCOPES*sizeof(cgi_scope_val));

   strcpy(scope_filename,SCOPES_FILE); //..SCOPES_FILE to be initialized in "directories_initialize()"

   cgi_scope_file = tags_read_in(scope_filename);
   parse = cgi_scope_file;	//* prepare to break into smaller substrings * /

   while (parse[0] && (scopes_len < MAX_SCOPES)) {
		cgi_scopes[scopes_len].name = tags_chop(&parse,NEWLINE);
		cgi_scopes[scopes_len].descr[EN] = tags_chop(&parse,NEWLINE);
		cgi_scopes[scopes_len].descr[FR] = tags_chop(&parse,NEWLINE);


		if( !strncmp(cgi_scopes[scopes_len].name,"PAGE_MATCH",10) )
			page_match_index = scopes_len;
		scopes_len++;

		while(parse[0] == NEWLINE)
			parse++;	//* skip over empty lines * /
	}

	/* //from original code, commenting out ...dunno wat it was doing in the actual code of :tags_initspecs(char* fname);
	for (temp=tags_scope_file; temp<parse; temp++) {
		if (temp[0] == NEED_NL) temp[0] = NEWLINE;
	} */

	if (scopes_len == MAX_SCOPES) scopes_len--;

	/* empty scope uses PAGE_MATCH name by default */
	cgi_scopes[scopes_len].name = cgi_scopes[page_match_index].name;
   	cgi_scopes[scopes_len].descr[EN] = "";
   	cgi_scopes[scopes_len].descr[FR] = "";
}

void directories_initialize()   //... using DIRECTORY_LOC   to look for : directoryList : to initialize all the file paths; must be invoked AFTER directoryList_initialize()
{
   char           *parse;
   char           *temp;

   char            dir_filename[256];
   int             dir_len = 0; //...tags_formats_len = 0;

   char           *currFile;
   char           *currLoc;

//    if (argc) DIRECTORY_FILE = argv[1];
//    else { // default location for control file is .searchrc
//   DIRECTORY_FILE = malloc((strlen(".searchrc")+1)*sizeof(char));
//   strcat(DIRECTORY_FILE,".searchrc");
//    }
   strcpy(dir_filename,DIRECTORY_FILE); //..DIRECTORY_FILE defined in cgi.h (by including .searchrc)

   dir_file = tags_read_in(dir_filename);
   parse = dir_file;	//* prepare to break into smaller substrings * /


   while(parse[0] && dir_len < MIN_DIR){
	   currFile = tags_chop(&parse,NEWLINE);
	   currLoc = tags_chop(&parse,NEWLINE);

	   if(strcmp(currFile,"LOG_FILE")==0)
	   		LOG_FILE = currLoc;
	   else if(strcmp(currFile,"MSGS_FILE")==0)
	   		MSGS_FILE = currLoc;
	   else if(strcmp(currFile,"TEXTS_FILE")==0)
			TEXTS_FILE = currLoc;
	   else if(strcmp(currFile,"SCOPES_FILE")==0)
			SCOPES_FILE = currLoc;
       	   else if(strcmp(currFile,"TEXTLOC")==0)
	   		TEXTLOC = currLoc;
	   else if(strcmp(currFile,"SEARCH_CMD")==0)
	   		SEARCH_CMD = currLoc;
	   else if(strcmp(currFile,"SEARCH_LOC")==0)
	   		SEARCH_LOC = currLoc;
	   else if(strcmp(currFile,"SEARCH_MATCHES")==0)
	   		SEARCH_MATCHES = currLoc;
	   else if(strcmp(currFile,"SEARCH_LOC_VAL")==0)
	   		SEARCH_LOC_VAL = currLoc;
   	   else if(strcmp(currFile,"COUNT_CMD")==0)
	   		COUNT_CMD = currLoc;
   	   else if(strcmp(currFile,"SPECS_DIR")==0)
   	   		SPECS_DIR = currLoc;
   	   else if(strcmp(currFile,"SPEC_NAMES")==0)
   	   		SPEC_NAMES = currLoc;
   	   else if(strcmp(currFile,"CHAR_CODES")==0)
	   		CHAR_CODES = currLoc;
	   else if(strcmp(currFile,"STRING_SUB_PROG")==0) //for "Albertus" project only - dependency 
	   		STRING_SUB_PROG = currLoc;
	   else if(strcmp(currFile,"INDEX_PREFIX")==0) //for "Albertus" project only - dependency 
	   		INDEX_PREFIX = currLoc;
	   else if(strcmp(currFile,"OUTLINE_FORMAT")==0) //for "Albertus" project only - dependency 
	   		OUTLINE_FORMAT = currLoc;
	   else if(strcmp(currFile,"USAGE_LOG")==0) //for "Albertus" project only - dependency 
	   		USAGE_LOG = currLoc;

           dir_len++;

           while(parse[0] == NEWLINE) parse++;	//* skip over empty lines * /
   }

}

//ONLY required for "Albertus" project
void browser_initialize()
{
   char           *parse;
   char           *temp;

   char            browser_filename[1000];
   //int             browser_len = 0; //...tags_formats_len = 0;

   //* now initialize the browser table * / //.. ie, cgi_scope_val *cgi_scopes;



   browser_tbl = (browser_obj*) malloc (MAX_TEXTS*sizeof(browser_obj));

   strcpy(browser_filename,BROWSER_TEXTS_FILE); //..BROWSER_TEXTS_FILE would be already initialized in "directories_initialize()"
   
   browser_temp_file = tags_read_in(browser_filename);

   parse = browser_temp_file;	//* prepare to break into smaller substrings * /

   while (parse[0] && (browser_tbl_len < MAX_TEXTS)) { //MAX_TEXTS defined in cgi.h
		browser_tbl[browser_tbl_len].brTextName = tags_chop(&parse,NEWLINE);
		browser_tbl[browser_tbl_len].brQuery = tags_chop(&parse,NEWLINE);
		browser_tbl[browser_tbl_len].brScope = tags_chop(&parse,NEWLINE);
		browser_tbl[browser_tbl_len].brFormat = tags_chop(&parse,NEWLINE);

		browser_tbl_len++;

		while(parse[0] == NEWLINE)
			parse++;	//* skip over empty lines * /
	}


	if (browser_tbl_len == MAX_TEXTS) browser_tbl_len--;

	browser_tbl[browser_tbl_len].brTextName = cgi_scopes[0].name;
	browser_tbl[browser_tbl_len].brQuery = "";
	browser_tbl[browser_tbl_len].brScope = "";
	browser_tbl[browser_tbl_len].brFormat = "";
}
