All files in this directory must be changed to reflect the actual locations of texts, specifications, etc.
as well as locations of sgrep, perl, and m4 code. (/table, /texts/, /specs/ used for illustrative purposes only.)

directoryList
   master directory of file locations and command pieces used by the system
    MSGS_FILE
       location of messages file
    CHAR_CODES
       location of character conversion file
    TEXTS_FILE
       locations of text index file
    SCOPES_FILE
       location of scopes file
    TEXTLOC
       location of directory contains texts
    COUNT_CMD
       sgrep command to count matches
    SEARCH_CMD
       sgrep command to search for matches
    SEARCH_LOC
       sgrep parameter for finding location attributes within texts
    SEARCH_LOC_VAL
       sgrep parameter for finding location values with texts
    SEARCH_MATCHES
       sgrep nmacro for finding all individual words in a text
    SPECS_DIR
       location of directory containing Dragoman style sheets
    SPEC_NAMES
       list of style sheets available for Dragoman
    STRING_SUB_PROG
       perl command to create sgrep search
    INDEX_PREFIX
       prefix to prepend on file names for normalized files
    OUTLINE_FORMAT
       style sheet name to use for diplaying whole text at once
    USAGE_LOG
       location for keeping access log
charcode_list
   conversions for simple printing and symbolic printing of character entrefs
message_list
   list of messages used to headings, errors, etc. Two lines each, for English and French, used according
   to setting of parameter "lang", but now forced to 0 (English) in the code in cgi/main.c.
scope_list
   list of regions to search: first line is sgrep region name, second line is English, third is French
   default scope listed first
speclist
   list of Dragoman style sheets: filet line is file name, second is English, third is French
   default style listed first
stringSub.plx
   perl script to create sgrep search string
textindex
   list of texts to be searched: file name, text name for English, text name for French
   fourth line specifies whether the text is in the Cologne collection (excludable) or not (always)
