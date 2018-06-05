Dragoman: converts simple tagged text into codes using a simple style table.
 
Simple XML-like tags and entity references(e.g., &epsilon.) are replaced 
by other text chosen to reflect the structure of the text through typography.
  
In the source text, an opening tag uses the pattern "TBEGIN tagname TEND", 
a closing tag uses "TBEGIN SLASH tagname TEND",and an entity reference 
uses "EBEGIN entref EEND".

An important characteristic of the formatter is that not only can tags
(and entity references) be replaced by formatting or other symbols, but tags
can also be used to suppress arbitrarily large portions of text.  Whether or not
to suppress text is controlled by a counter that is decremented with each
suppress command and incremented with each printon command, with printing
occurring only if the count is positive.  Replacement text is also suppressed
according to the counter, except if a command is to force tag replacement
regardless of text suppression.  Finally, a matched opening and closing tag pair
can be interpreted to indicate that the text between the tags be replaced
in a manner similar to an entity reference (effectively interpreting the
opening tag as EBEGIN and the closing tag as EEND).

Tag interpretation is read from a stylesheet represented by a table
with the following grammar:
	style = initial sep tags sep entrefs
	initial = (printon | suppress) newline
	printon = '+'
	suppress = '-'
	newline = ''
	sep = '%%' newline
	tags = tagstyle+
	tagstyle = (tagname | attrname) tab action tab action newline
	tagname = char+
	attrname = char+ '.' char+
	char = (any character except for a tab, period, or newline)
	tab = ''
	action = command char+
	command = replace | suppress | printon | force | interpret
	replace = '"'
	force = '*'
	interpret = '&'
	entrefs = erstyle+
	erstyle = entref tab replace char+ newline
	entref = EBEGIN char+ EEND
	EBEGIN = '&'
	EEND = ';'
The two actions in a tagstyle dictate behaviour for opening and closing
tags with the indicated tagname.  Actions can be to replace the tag with
the indicated text (subject to print suppression), suppress printing of text,
start printing text, replace the tag regardless of print suppression,
and interpret the entity content as if it were an entity reference.
Except for the last of these actions, there is no reliance on tags being
properly matched.  Tagnames not matched in the table are interpreted as
	name	"&lt;name>	"&lt;/name>

Attributes are handled almost the same way as tags, except that attributes are
identified by the format:
	tag.attribute
And this is stored in the tag table too. Attribute names not matched are ignored.

This code mimics the behaviour of LECTOR, written by Darrell Raymond at the Waterloo 
Centre for the New OED. 
 
   char* tags_convert(char* tagged);
         // Interpret tagged text according to the table

   void tags_fix_buffer(int needs);
         // make the text buffer bigger if needed
         // called only from tags_convert.

   void tags_get_tag(char* word, char* line, int* x);
         // strip off the tag at offset x within the input line.

   void tags_get_entref(char* word, char* line, int* x);
         // strip off the entity reference at offset x within the input line.

   void tags_get_data(char* lineout, int* y, char* line, int* x);
         // copy the non-tagged text at offset x within line
         // to offset y within lineout.

   char* tags_chop(char** s, char c);
         // return a copy of the start of text s up to character c
         // used only from tags_initialize.
 
   int tags_offset(char *s, int p, char c);
         // find the offset of character c after position p in text s.

   int tags_get_attr(char *attrnm, char *attrval, char *line, int *x);
         // get the attribute name and attribute value of each tag.

   int tags_lookup(char* tag);
         // find the stylesheet table entry matching tag.

   void tags_initialize(char* fname);
         // set up the internal tag-interpretation table from a specfile.

   void tags_initspecs(char* fname);
         // initialize the list of styles available.

   char* tags_read_in(char* fname);
         // read the style table from a file and return it as a string
         // called only from tags_initialize.

   void tags_error(char* msg);
         // signal an error and halt execution.
