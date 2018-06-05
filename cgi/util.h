//FILE: util.h
//Location: ~/manuprobe/src/cgi

#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

#define MAX_ENTRIES 10000

typedef struct {
    char *name;
    unsigned char *val;
} name_val;

name_val entries[MAX_ENTRIES];   //previously extern

void	 getword(char *word, char *line, char stop);
char	*makeword(char *line, char stop);
unsigned char	*fmakeword(FILE *f, char stop, int *cl);
unsigned char	 x2c(unsigned char *what);
void	 unescape_url(unsigned char *url);
void	 plustospace(unsigned char *str);
int	     rind(char *s, char c);
/* int	     getline(char *s, int n, FILE *f); */
void	 send_fd(FILE *f, FILE *fd);
int	     ind(char *s, char c);
void	 escape_shell_cmd(char *cmd);
char*	 url_encode(unsigned char *s);

int	     http_initialize(void);
unsigned char	*http_val(char* word, int m);
void http_val_dump (int m, FILE* out);
void	 http_printf (char* text);
char* 	 normalize(char* word, int indexed);
void	 semi2comma(unsigned char *str);
void     writeLog(int m);

char*	orthonormal(char* text, int align);
#endif
