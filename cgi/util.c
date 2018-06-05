//File: util.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

#define LF 10
#define CR 13

void getword(char *word, char *line, char stop) {
    int x = 0,y;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while(line[y++] = line[x++]);
}

char *makeword(char *line, char stop) {
    int x = 0,y;
    char *word = (char *) malloc(sizeof(char) * (strlen(line) + 1));

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while(line[y++] = line[x++]);
    return word;
}

unsigned char *fmakeword(FILE *f, char stop, int *cl) {
    int wsize;
    char *word;
    int ll;

    wsize = 102400;
    ll=0;
    word = (char *) malloc(sizeof(char) * (wsize + 1));

    while(1) {
        word[ll] = (char)fgetc(f);
        if(ll==wsize) {
            word[ll+1] = '\0';
            wsize+=102400;
            word = (char *)realloc(word,sizeof(char)*(wsize+1));
        }
        --(*cl);
        if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
            if(word[ll] != stop) ll++;
            word[ll] = '\0';
	    word = (char *) realloc(word, ll+1);
            return word;
        }
        ++ll;
    }
}

unsigned char x2c(unsigned char *what) {
    register unsigned char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void unescape_url(unsigned char *url) {
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

void plustospace(unsigned char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

void spacetoplus(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == ' ') str[x] = '+';
}

int rind(char *s, char c) {
    register int x;
    for(x=strlen(s) - 1;x != -1; x--)
        if(s[x] == c) return x;
    return -1;
}

/*
int getline(char *s, int n, FILE *f) {
    register int i=0;

    while(1) {
        s[i] = (char)fgetc(f);

        if(s[i] == CR)
            s[i] = fgetc(f);

        if((s[i] == 0x4) || (s[i] == LF) || (i == (n-1))) {
            s[i] = '\0';
            return (feof(f) ? 1 : 0);
        }
        ++i;
    }
}
*/

void send_fd(FILE *f, FILE *fd)
{
    int num_chars=0;
    char c;

    while (1) {
        c = fgetc(f);
        if(feof(f))
            return;
        fputc(c,fd);
    }
}

int ind(char *s, char c) {
    register int x;

    for(x=0;s[x];x++)
        if(s[x] == c) return x;

    return -1;
}

void escape_shell_cmd(char *cmd) {
    register int x,y,l;

    l=strlen(cmd);
    for(x=0;cmd[x];x++) {
        if(ind("&;`'\"|*?~<>^()[]{}$\\\x0A",cmd[x]) != -1){
            for(y=l+1;y>x;y--)
                cmd[y] = cmd[y-1];
            l++; /* length has been increased */
            cmd[x] = '\\';
            x++; /* skip the character */
        }
    }
}

/* Source: http://www.w3.org/International/URLUTF8Encoder.java */

/*
 * Provides a method to encode any string into a URL-safe form.
 * Non-ASCII characters are first encoded as sequences of
 * two or three bytes, using the UTF-8 algorithm, before being
 * encoded as %HH escapes.
 */

    /**
   * Encode a string to the "x-www-form-urlencoded" form, enhanced
   * with the UTF-8-in-URL proposal. This is what happens:
   *
   * <ul>
   * <li><p>The ASCII characters 'a' through 'z', 'A' through 'Z',
   *        and '0' through '9' remain the same.
   *
   * <li><p>The unreserved characters - _ . ! ~ * ' ( ) remain the same.
   *
   * <li><p>The space character ' ' is converted into a plus sign '+'.
   *
   * <li><p>All other ASCII characters are converted into the
   *        3-character string "%xy", where xy is
   *        the two-digit hexadecimal representation of the character
   *        code
   *
   * <li><p>All non-ASCII characters are encoded in two steps: first
   *        to a sequence of 2 or 3 bytes, using the UTF-8 algorithm;
   *        secondly each of these bytes is encoded as "%xx".
   * </ul>
   *
   * @param s The string to be encoded
   * @return The encoded string
   */
char* url_encode (unsigned char* s) {
    int len;
    int i;
    char* sbuf;
    unsigned char ch;
    int pout = 0;
    int digit;

    if (s == NULL) return(s);
    len = strlen(s);
    sbuf = (char*) malloc (3*len+1); /* in the worst case, triple */
    for (i = 0; i < len; i++) {
      ch = s[i];
      if (('A' <= ch && ch <= 'Z') ||	// 'A'..'Z'
          ('a' <= ch && ch <= 'z') ||	// 'a'..'z'
          ('0' <= ch && ch <= '9') ||	// '0'..'9'
          ch == '-' || ch == '_' ||		// unreserved
          ch == '.' || ch == '!' ||
          ch == '~' || ch == '*' ||
          ch == '\'' || ch == '(' ||
          ch == ')')
		sbuf[pout++] = ch;
      else if (ch == ' ') 			// space
		sbuf[pout++] = '+';
      else if (ch <= 0x007f)  {			// other ASCII
		sbuf[pout++] = '%';
		digit = ch / 16;
		sbuf[pout++] = (digit<10)? ('0' + digit) : ('A' + digit - 10);
		digit = ch % 16;
		sbuf[pout++] = (digit<10)? ('0' + digit) : ('A' + digit - 10);
		}
      else /* if (ch <= 0x00FF) */ {	// non-ASCII <= 0x00FF
		sbuf[pout++] = '%';
		sbuf[pout++] = 'C';
		sbuf[pout++] = (ch >> 6);
		digit = 8 + ((ch / 16) % 4);
		sbuf[pout++] = (digit<10)? ('0' + digit) : ('A' + digit - 10);
		digit = ch % 16;
		sbuf[pout++] = (digit<10)? ('0' + digit) : ('A' + digit - 10);
		}

    /* 	
      else if (ch <= 0x07FF) {	// non-ASCII <= 0x7FF
	       sbuf.append(hex[0xc0 | (ch >> 6)]);
	       sbuf.append(hex[0x80 | (ch & 0x3F)]);

      } else {					// 0x7FF < ch <= 0xFFFF
	       sbuf.append(hex[0xe0 | (ch >> 12)]);
	       sbuf.append(hex[0x80 | ((ch >> 6) & 0x3F)]);
	       sbuf.append(hex[0x80 | (ch & 0x3F)]);
      }
    */

      } /* end for */
    sbuf[pout] = '\0';
    return sbuf;
}

