# Makefile for bison
#
# If you don't have an alloca() call define NO_ALLOCA in ../options.h
# Where temp files go
TEMPDIR = /tmp/
# where the installed binary goes
#BINDIR = /usr/local
#
# where the parsers go
#PARSERDIR = /usr/local/lib
#
# These are for the use of SM - if you install bison for general use
# you'd probably want to change them back. You'll have to change the SM
# Makefile to find bison, of course.
#
PARSERDIR = ./bison
BINDIR = $(PARSERDIR)
# names of parser files
PFILE = bison.simple
PFILE1 = bison.hairy
#
# Trim -Wall and -fullwarn from the list of options as bison does
# _not_ compile cleanly
#
.c.o :
	@echo `echo $(CC) | \
		sed -e 's/-fullwarn//' -e 's/-Wall//'` \
						-c $(CFLAGS) '$$(PFILES)' $*.c
	@`echo $(CC) | \
		sed -e 's/-fullwarn//' -e 's/-Wall//'` \
						-c $(CFLAGS) $(PFILES) $*.c
	
CC = cc
# It is unwise ever to compile a program without symbols (so says the FSF)
CFLAGS = -c -g 
PFILES = -DXPFILE=\"$(PARSERDIR)/$(PFILE)\" \
	 -DXPFILE1=\"$(PARSERDIR)/$(PFILE1)\" \
	 -DTDIR=\"$(TEMPDIR)\"
OBJECTS = lr0.o allocate.o closure.o conflicts.o derives.o files.o	\
	  getargs.o gram.o lalr.o					\
	  lex.o main.o nullable.o output.o print.o reader.o symtab.o	\
	  warshall.o utils.o
start: bison
clean:
	rm -f *.o core *~
install: bison
	install bison $(BINDIR)
	install -c $(PFILE) $(PFILE1) $(PARSERDIR)
bison: $(OBJECTS)
	$(CC) -o bison $(LDFLAGS) $(OBJECTS)
# This file is different to pass the parser file names
# to the compiler.
#files.o: files.c files.h new.h gram.h
#	$(CC) -c $(CFLAGS) $(PFILES) files.c
lr0.o: machine.h new.h gram.h state.h
closure.o: machine.h new.h gram.h
conflicts.o: machine.h new.h files.h gram.h state.h ../options.h
derives.o: new.h types.h gram.h
getargs.o: files.h
lalr.o: machine.h types.h state.h new.h gram.h
lex.o: files.h symtab.h lex.h
main.o: machine.h
nullable.o: types.h gram.h new.h
output.o: machine.h new.h files.h gram.h state.h
print.o: machine.h new.h files.h gram.h state.h
reader.o: files.h new.h symtab.h lex.h gram.h
symtab.o: new.h symtab.h gram.h ../options.h
utils.o : ../options.h
warshall.o: machine.h
