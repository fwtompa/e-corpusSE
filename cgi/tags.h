 //FILE: tags.h
//Location: ~/manuprobe/src/tags (soft link in ...../cgi )

#ifndef TAGS_H

#define EN 0
#define FR 1
#define LANGS 2

#define TBEGIN '<'
#define TCLOSE '/'
#define TEND '>'
#define EBEGIN '&'
#define NUMCHARREF '#'
#define EEND ';'

#define NEWLINE '\n'
#define TAB '\t'
#define NEED_NL '\\'
#define MAX_TAGS 1000
#define MAX_FORMATS 100



#define MAX_CHAR_REFS 256
#define MAX_PARM_LEN 1000

typedef struct {
    char* tagname;
    char* rbegin;
    char* rend;
} entry;

typedef struct {
    char* charref; // unnormalized string (character  reference)
    char norm; // orthonormalized version
} centry;

typedef struct {
    char* name;
    char* descr [LANGS];
} fentry;

char    *tags_file;		//prviously all extern
entry   *tags_tbl;
int      tags_tbl_len;
char    *tags_buffer;
int      tags_buffer_len;
char    *tags_spec_file;
fentry  *tags_formats;
char    *tags_char_file;
centry   *char_tbl;
int      char_tbl_len;
char	strbuf[MAX_PARM_LEN];//char	strbuf[];
int      printoff;

#define TAGS_H

char*		tags_convert(char* tagged);
void        tags_handle_start(int which, int *pin, int *pout, char *tagged, char *tag);
void        tags_handle_end(int which, int *pout);
void		tags_fix_buffer(int needs);
int			tags_get_tag(char* word, char* line, int* x);
int         tags_get_attr(char *attrnm, char *attrval, char *line, int *x);
void		tags_get_entref(char* word, char* line, int* x);
void		tags_get_data(char* word, int* y, char* line, int* x);
char*		tags_chop(char** s, char c);
int			tags_offset(char *s, int p, char c);
int		    tags_lookup(char* tag);
void		tags_convert_char(unsigned char c1, unsigned char c2, char* out);
char*		tags_char2ent(unsigned char* in, int quotesOK);


char*		tags_read_in(char* fname);
void		tags_initialize(char* fname,char* tags_dir);
void		tags_initspecs(char* fname,char* tags_dir);
void		tags_initchars(char* fname);

void		tags_error(char* msg, char* fname);

#endif
