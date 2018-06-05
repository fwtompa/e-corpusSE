//FILE: cgi.h
//LOCATION: "~/testSource/src/cgi/"

#include "tags.h"

#define MAX_SEARCH 1000
#define MATCH_BEGIN "-e '"  
#define SGREP_CLOSE ")' "
#define PREV_PAGE "PREV_PAGE"
#define NEXT_PAGE "NEXT_PAGE"

#define MATCH_TAG "match"

#define COMMAND2 SGREP_CLOSE

//#define timing // out put timing for runs
#undef timing

typedef struct {
    char* internal_name;
    char* display_text[LANGS];
} converter;

typedef struct {
    char* internal_name;
    char* display_text[LANGS];
    char* extraInfo;
#ifdef timing
    double CPU_time;
    double wall_time;
#endif
} converter_2;


typedef struct {
    char* name;
    char* descr [LANGS];
} cgi_scope_val;

typedef struct {
	char* brTextName;
	char* brQuery;
	char* brScope;
	char* brFormat;
} browser_obj;

typedef struct {
    FILE* file; 
    int begin;
    int end;
} cursor;
typedef struct {
    int begin;
    int end;
    cursor* part[2];
} merge_cursor;

#define MAX_MSGS 100
#define MAX_TEXTS 100


converter     *msgs_tbl;
int      msgs_tbl_len;
converter_2     *texts_tbl;
int      texts_tbl_len;
int	texts_max_len;

char   *cgi_scope_file;
cgi_scope_val *cgi_scopes;
int page_match_index;

char *browser_temp_file;
browser_obj *browser_tbl;
int browser_tbl_len;

#include ".searchloc"

#define MIN_DIR 20
#define MAX_SCOPES 20

char* LOG_FILE;
char* MSGS_FILE;
char* TEXTS_FILE;
char* SCOPES_FILE;
char* TEXTLOC;

char* COUNT_CMD;
char* SEARCH_CMD;
char* SEARCH_LOC;
char* SEARCH_LOC_VAL;
char* SEARCH_MATCHES;

char* SPECS_DIR;
char* SPEC_NAMES;
char* CHAR_CODES;
char* STRING_SUB_PROG;
char* LOC_INFO_PROG;
char* BROWSER_TEXTS_FILE;
char* INDEX_PREFIX;
char* OUTLINE_FORMAT;
char* USAGE_LOG;
// char* DIRECTORY_FILE;

void  cgi_error(char* msg, char* y);
char* msgs_lookup(char* msg_id, int lang);

// functions in  util2.c adn queryAssist.c
char* constructQuery(char* word, char* macro, int numWords, char* prog, char* userMacro);
char* adjustQuery(char* instances, char*command);
int   unfinishedXML(char* letters);
char* untag(char* text, int pos);
int   unfinishedTag(int begin, int end, cursor* datafile);
char* constructRegion(int begin, int end, cursor* hit, int fnum);
int   nestingCheck(char* word, int* numWords);
char* completeXML(char* tagged);
char* decodeUTF8(unsigned char* str);

// functions in engine.c
long int   engine_open(char* doc, cursor* datafile);
void   engine_close(cursor* datafile);
void   engine_idx_close(merge_cursor* datafile);
char* get_match(int begin, int end, FILE* data);
int   engine_count(char* wordQuery, char* doc, char* idx_query);
cursor* engine_findall(char* wordQuery, char* doc, int indexed);
merge_cursor* engine_idx_findall(char* wordQuery, char* doc, char* idx_query);
int   advanceN(cursor* matches, int count);
int   advanceMergedN(merge_cursor* matches, int count);
int   advanceToIncludeOrBeyond(cursor* matches, int target);
int   advanceMergedToIncludeOrBeyond(merge_cursor* matches, int target);
int   advanceToBeyond(cursor* matches, int target, int* prevBegin, int* prevEnd);
int   advanceMergedToBeyond(merge_cursor* matches, int target);
void  encompassWord(cursor* doc, int size);

// functions in initialize.c
int   http_initialize (void);
void  msgs_initialize ();
void  texts_initialize ();
void  scopes_initialize();
void  directories_initialize();
void  browser_initialize();
