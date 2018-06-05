This directory contains the source code for the Alberti Magni e-corpus project.
For an overview, see
   F. W. Tompa, "Fashioning a Search Engine to Support Humanities Research,"
   18th ACM Symposium on Document Engineering (DocEng 2018), 10 pp.
Example congtrol files are also included.

To create all software for accessing the corpus search engine from the Web,
type "make BINDIR=xxx codeName=yyy" at this level --
it recursively calls make to create all the software components in the
correct order, placing the resulting search code in the directory xxx with
the name yyy.cgi and the index code in obj/index.

To erase everything except for the running executables,
type "make clean" at this level -- it recursively calls
make to delete all object modules and libraries.


NOTE:

Before the software can be used, it requires files to be created for table-driven pieces:
  Sample files for table-driven operation are located in the "tables" directory.
  (Note: samples all use "/table" as location for files; change these to actual locations.)



cgi: directory for search engine code

   search engine outline:
      Search for entries containing a word or phrase.
      ---initialize http protocol.
      ---settings of search string, display format, manuscript to be searched in,
         number of entries to return.
      ---form Sgrep command according to the previous settings.
      ---read in the designated match (using get_match)
      ---convert it to HTML using tags_convert
      ---prepare for follow-on query
   .searchloc
      // file specifying where the table-driven data resides
      // contents must be changed to point to appropriate location

tags: directory for display code (Dragoman)

   Converts simple tagged text into codes using a simple style table.
    
index.c: program to create normalized versions of corpus
