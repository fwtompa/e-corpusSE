//File: get_tag.c
//Location: ~/manuprobe/src/tags

#include <string.h>
#include <stdio.h>
#include "tags.h"

int tags_get_tag(char *word, char *line, int *x)
{
   /* pre: line[*x] == TBEGIN
      post: *x is two chars past tagname, which is copied to *word
            returns 0 iff TEND found
   */
    register int y = 0;

    if(line[*x] == TBEGIN) (*x)++;
    while((line[*x]) && (line[*x] != TEND) && (line[*x] != ' '))
        word[y++] = line[(*x)++];
    word[y] = '\0';
    if(line[*x]) ++(*x);
    return ((line[*x-1] == ' ')?1:0);
}

int tags_get_attr(char *attrnm, char *attrval, char *line, int *x)
{
   /* 
      post: *x is one char past attribute/value pair, which is copied to *attrnm, *attrval
            returns 0 iff TEND found
   */
    register int y = 0;
    int quote;

    if(line[*x] == TEND)
    {
      (*x)++;
      attrnm = NULL;
      attrval = NULL;
      return 0;
    }
    if( !line[*x])
    {
      attrnm = NULL;
      attrval = NULL;
      return 0;
    }
    /*get the attribute name*/
    while((line[*x]) && (line[*x] != TBEGIN) && (line[*x] != TEND) && (line[*x] != ' ') && (line[*x] != '='))
        attrnm[y++] = line[(*x)++];
    attrnm[y] = '\0';
    y = 0;
    
    /*skip over blanks and equal signs*/
    while(line[*x] == ' ' || line[*x] == '=')  (*x)++;
    if(line[*x] == TEND || !line[*x] || line[*x] == TBEGIN) // invalid attr/val pair
    {
      attrval = NULL;
      return 0;
    }
    
    /*get the attribute value*/
    if (line[*x] == '"') 
    {
      (*x)++;
      quote = 1;
    }
    /* handle single quotes as well  */
    else if (line[*x] == '\'') 
      /*use \ to escape single quotes? */
    {
      (*x)++;
      quote = 0;
    }
    else // no valid attrval provided
    {
      attrval = NULL;
      return 0;
    }
	
    if (quote == 1)
    {
      while((line[*x]) && (line[*x] != '"'))
      {
        attrval[y++] = line[(*x)++];
      }
      if (line[*x] == '"') (*x)++;
    }
    else if (quote == 0)
    {
      while((line[*x]) && (line[*x] != '\''))
      {
        attrval[y++] = line[(*x)++];
      }
      if (line[*x] == '\'') (*x)++;
    }

    attrval[y] = '\0';
    return ((line[*x] == TEND) || (line[*x] == TCLOSE)?0:1);
    
}

void tags_get_entref(char *word, char *line, int *x)
{
    register int y = 0;

    if(line[*x] == EBEGIN) {
       while((line[*x]) && (line[*x] != EEND))
          word[y++] = line[(*x)++];
       }
    if(line[*x]) {
       (*x) += 1;
       word[y++] = EEND;
       }
    word[y] = '\0';
}

void tags_get_data(char *word, int *y, char *line, int *x)
{

    while((line[*x]) && (line[*x] != TBEGIN) && (line[*x] != EBEGIN))
        word[(*y)++] = line[(*x)++];
    word[*y] = '\0';
}

char* tags_chop(char **s, char c)
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

int tags_offset(char *s, int p, char c)
{
    register int x;

    /* Does not check for addressing out of range */
    /* if (p > strlen(x)) return -1; */
    /* Thus useful for breaking string into small substrings */
    for(x=p;s[x];x++)
        if(s[x] == c) return x;
    return -1;
}

int tags_count(char *s, char c)
{
    register int x = 0, y = -1;

    for (y=-1 ; (y = tags_offset(s,y+1,c)) >= 0 ; x++) ; 
    return x;
}

