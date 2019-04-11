//File: intialize.c
//Location: ~/manuprobe/src/tags

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tags.h"
#include <sys/types.h>
#include <sys/stat.h>

/* make space for the external variables */ //not extern anymore

char    *tags_file = NULL;
entry   *tags_tbl = NULL;
int      tags_tbl_len = 0;
char    *tags_spec_file = NULL;
fentry  *tags_formats = NULL;


char    *tags_buffer = NULL;
int      tags_buffer_len = 0;
char    *tags_char_file = NULL;
centry  *char_tbl = NULL;
int      char_tbl_len = 0;
//char     strbuf[MAX_PARM_LEN];
int      printoff = 0;


char* tags_read_in(char *fname)  // ..making a file into a stream of characters
{
	FILE           *fin;
	char           *mem;
	struct stat     buf;
	
	if (stat(fname, &buf) != 0)
        	tags_error("read_in: Cannot access file",fname);
	fin = fopen(fname, "r");
	if (fin == NULL)
        	tags_error("read_in: Cannot open file",fname);
	mem = (char*) malloc((buf.st_size + 1)*sizeof(char));
	if (fread(mem, 1, buf.st_size, fin) < buf.st_size)
        	tags_error("read_in: Insufficient data read from file",fname);
	mem[buf.st_size] = '\0';	/* Ensure it can be accessed as a string */
	return (mem);
}

/*
 * this creates the tags_tbl of pointers into tags_file showing replacements for
 * tags in converting tagged text (tags_tbl_len set to count of tags)
 */

void tags_initialize(char* fname,char* tags_dir) //void tags_initialize(char* fname)
{
	char           *parse;
	char           *temp;
	char            tags_filename[256];
	
	/* initialize the output buffer for converted text */
	tags_buffer_len = 10000;
	tags_buffer = (char*) malloc (tags_buffer_len*sizeof(char));
	tags_buffer[0] = '\0'; 
	
	/* now initialize the tag translation table */
	
	tags_tbl = (entry*) malloc (MAX_TAGS*sizeof(entry)); 
	strcpy(tags_filename,tags_dir);
	strcat(tags_filename,fname);
	tags_file = tags_read_in(tags_filename);
	parse = tags_file;	/* prepare to break into smaller substrings */
	while (parse[0] && (tags_tbl_len < MAX_TAGS) && (strncmp(parse,"%%",2))) { 		
	    if (parse[0] == '-') printoff = 1;
		else printoff = 0;
		tags_chop(&parse,NEWLINE);
	}
	
	tags_chop(&parse,NEWLINE); /* skip over "%%" separator */
	while (parse[0] && (tags_tbl_len < MAX_TAGS) && (strncmp(parse,"%%",2))) {
		tags_tbl[tags_tbl_len].tagname = tags_chop(&parse,TAB);
		tags_tbl[tags_tbl_len].rbegin = tags_chop(&parse,TAB);
		tags_tbl[tags_tbl_len++].rend = tags_chop(&parse,NEWLINE);
	}
	tags_chop(&parse,NEWLINE); /* skip over "%%" separator */
	while (parse[0] && (tags_tbl_len < MAX_TAGS)) {
		tags_tbl[tags_tbl_len].tagname = tags_chop(&parse,TAB);
		tags_tbl[tags_tbl_len].rbegin = tags_chop(&parse,NEWLINE);
		tags_tbl[tags_tbl_len++].rend = NULL;	/* no end marker for entrefs */
	}
	for (temp=tags_file; temp<parse; temp++) {
		if (temp[0] == NEED_NL) temp[0] = NEWLINE;
	}

}

void tags_initspecs(char* fname,char* tags_dir)//void tags_initspecs(char* fname)
{
	char           *parse;
	char           *temp;
	char            tags_filename[256];
	int             tags_formats_len = 0;
	
	/* now initialize the spec translation table */ //.. ie, fentry *tags_formats;
	
	tags_formats = (fentry*) malloc (MAX_FORMATS*sizeof(fentry));
	strcpy(tags_filename,tags_dir);
	strcat(tags_filename,fname);
	tags_spec_file = tags_read_in(tags_filename);
	parse = tags_spec_file;	/* prepare to break into smaller substrings */
	
	while (parse[0] && (tags_formats_len < MAX_FORMATS)) {
		tags_formats[tags_formats_len].name = tags_chop(&parse,NEWLINE);
		tags_formats[tags_formats_len].descr[EN] = tags_chop(&parse,NEWLINE);
		tags_formats[tags_formats_len++].descr[FR] = tags_chop(&parse,NEWLINE);
		
		while(parse[0] == NEWLINE) 
			parse++;	/* skip over empty lines */
	}
	
	for (temp=tags_spec_file; temp<parse; temp++) {
		if (temp[0] == NEED_NL) temp[0] = NEWLINE;
	}
	
	if (tags_formats_len == MAX_FORMATS) tags_formats_len--;
	
	tags_formats[tags_formats_len].name = tags_formats[0].name;
   	tags_formats[tags_formats_len].descr[EN] = "";
   	tags_formats[tags_formats_len].descr[FR] = "";
}

void tags_initchars(char* chars_filename)
{
	char           *parse;
	unsigned int    c;
	
	/* now initialize the character translation table */
	
	char_tbl = (centry*) malloc (MAX_CHAR_REFS*sizeof(centry));
	tags_char_file = tags_read_in(chars_filename);
	parse = tags_char_file;	/* prepare to break into smaller substrings */
	for (char_tbl_len = 0; parse[0] && (char_tbl_len < MAX_CHAR_REFS); char_tbl_len++) {
		char_tbl[char_tbl_len].charref = tags_chop(&parse,TAB);
                char_tbl[char_tbl_len].norm = (tags_chop(&parse,TAB))[0]; // just one character in normaization
		tags_chop(&parse,NEWLINE);  // discard remainder of line
		while(parse[0] == NEWLINE) parse++;	/* skip over empty lines */
                if (char_tbl_len > 0 && strcmp(char_tbl[char_tbl_len].charref,char_tbl[char_tbl_len-1].charref) >= 0)
                   tags_error("initchars: Text normalization file violates strictly decreasing order",chars_filename);
	}
        char_tbl[char_tbl_len].charref = "";
        char_tbl[char_tbl_len++].norm = '\0';
}

