# For gcc
CC= gcc
# For ANSI compilers
#CC= cc

#For Optimization
#CFLAGS= -O2
#For debugging
CFLAGS= -g -I.

OBJDIR = ../obj
LIBDIR = ../lib

all:
	(cd tags; \
	 make LIBDIR=$(LIBDIR) OBJDIR=$(OBJDIR) all)
	(cd cgi; \
	 make BINDIR=$(BINDIR) LIBDIR=$(LIBDIR) \
		OBJDIR=$(OBJDIR) codeName=$(codeName) all)
	make index

index: cgi/util2.c tags/tinitialize.c cgi/initialize.c index.c
	$(CC) $(CFLAGS) -o index index.c $(LIBDIR)/cgi.a $(LIBDIR)/tags.a
	chmod 755 index

clean:
	(cd tags; make LIBDIR=$(LIBDIR) OBJDIR=$(OBJDIR) clean)
	(cd cgi; make OBJDIR=$(OBJDIR) clean)
	rm index
