CC = gcc

RANLIB = ranlib

CFLAGS = -I. -c -g -O

OBJS =  \
	$(OBJDIR)/tconvert.o \
	$(OBJDIR)/terror.o \
	$(OBJDIR)/tget_tag.o \
	$(OBJDIR)/tinitialize.o \
	$(OBJDIR)/tlookup.o

all : $(LIBDIR)/tags.a tags.h

$(OBJS) : tags.h

$(LIBDIR)/tags.a : $(OBJS)
	ar r $(LIBDIR)/tags.a $(OBJS)

.SUFFIXES: .c .o

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@  $*.c 

clean:
	- rm $(OBJS)
	- rm $(LIBDIR)/tags.a
