.c.o :
	$(CC) -c $(CFLAGS) $*.c 
.f.o :
	f77 -c $(FFLAGS) $*.f
#
SHELL = /bin/sh
#
# Where the SM source is
#
SMSRC = ../src
#
# Compilers:
#
CC = gcc -O3 -Wall -Dlinux
# (Note, if you compiled SM with gcc you'll have to use it to link, too.
# use F77 = gcc. In this case you also need to add $(GCC_FORT_LIBS) to
# LDFLAGS when linking fortran)
GCC_FORT_LIBS = -L/usr/lang/SC1.0 -lF77 -lc -lm
#
#F77 = gcc
F77 = f77
#
# cc flags
#
CFLAGS = -g -I/home/jon/research/utils/sm/sm2_4_1/src
#
# f77 flags
#
FFLAGS = -g -c
#
# linker flags
#
SGILIB = -lgl
SUNLIB = -lcore -lsunwindow -lpixrect
SUNVLIB = -lsuntool -lsunwindow -lpixrect
XLIB = -lX
XLIB11 = -lX11 
#
# Add the libraries that you need for the #define's you chose in options.h:
# SGI			$(SGILIB)
# SUN_VIEW		$(SUNVLIB)
# SUNWINDOWS		$(SUNLIB)
# XWINDOWS, (i.e. X10)	$(XLIB)
# X11, (i.e. X11)	$(XLIB11)
#
LDFLAGS = -L$(SMSRC) -lplotsub -ldevices -lutils  $(XLIB11) -lm
#
test : test.o
	$(F77) -o test test.o $(LDFLAGS) # $(GCC_FORT_LIBS)
testd : testd.o
	$(F77) -o testd testd.o $(LDFLAGS) # $(GCC_FORT_LIBS)
tstc : tstc.o
	$(CC) -o tstc tstc.o $(LDFLAGS)
contc : contc.o
	$(CC) -o contc contc.o $(LDFLAGS)
cont : cont.o
	$(F77) -o cont cont.o $(LDFLAGS) # $(GCC_FORT_LIBS)
colors : colors.o
	$(F77) -o colors colors.o $(LDFLAGS) # $(GCC_FORT_LIBS)
#
interp : interp.o
	$(CC) -o interp interp.o -L$(SMSRC) -lparser $(LDFLAGS)
finterp : finterp.o
	$(F77) -o finterp finterp.o -L$(SMSRC) -lparser \
			$(LDFLAGS) $(GCC_FORT_LIBS)
#
clean :
	- rm -f core cont contc finterp interp test testd tstc *.o *.trace *~
