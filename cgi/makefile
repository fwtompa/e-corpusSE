# For gcc
CC= gcc
# For ANSI compilers
#CC= cc

#For Optimization
#CFLAGS= -O2 -I.
#For debugging
CFLAGS= -g -I.

SUFFIX = .cgi

LOOKUP = $(BINDIR)/$(codeName)$(SUFFIX)

OBJS = \
	$(OBJDIR)/queryAssist.o \
        $(OBJDIR)/engine.o \
	$(OBJDIR)/error.o \
	$(OBJDIR)/initialize.o \
	$(OBJDIR)/util2.o \
	$(OBJDIR)/util.o

all:	$(LOOKUP) $(LIBDIR)/cgi.a

$(LOOKUP): $(OBJS) $(OBJDIR)/main.o $(LIBDIR)/tags.a
	$(CC) $(OBJS) $(OBJDIR)/main.o $(LIBDIR)/tags.a -o $(LOOKUP)
	chmod 755 $(LOOKUP)

$(OBJS): util.h tags.h cgi.h

$(LIBDIR)/cgi.a : $(OBJS)
	ar r $(LIBDIR)/cgi.a $(OBJS)

.SUFFIXES: .c .o

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	- rm $(OBJS)
	- rm $(LIBDIR)/cgi.a

