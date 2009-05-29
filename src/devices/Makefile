#
# Default rules for compiling code.
#
.c.o :
	$(CC) -c $(CFLAGS) -I.. $*.c
#
TTY_OFILES = 	ttyindex.o ttyget.o ttyopen.o ttyputs.o ttyio.o
OFILES =	stgsetup.o stgencode.o stgerase.o stgcursor.o stgclose.o\
		stgline.o stgreloc.o stgchar.o stglist.o stgfillpt.o \
		imagen.o raster.o sgi.o sunview.o sunwindows.o x10.o x11.o \
		unixpc.o metafile.o svgalib.o
UTIL_OFILES =	get1char.o gstrcpy.o
#
# Update the library in the directory above
#
../libdevices.a : $(OFILES) $(TTY_OFILES) $(UTIL_OFILES)
	ar r ../libdevices.a $?
	@- if test "$(RANLIB)" != "" -a -f "$(RANLIB)"; then \
		$(RANLIB) ../libdevices.a; fi
#
# sunwindow/win_cursor.h has an illegal #ifdef, so we have to use cc
#
sunview.o : sunview.c
	@- if test "`echo $(CC) | sed -e 's/ .*//'`" = "gcc"; then \
		if test `uname` = "SunOS"; then \
			echo \
	"You can safely ignore warnings from sunwindows header files" >&2;\
			echo ""; \
		fi; \
		$(CC) -c $(CFLAGS) -I.. sunview.c; \
	else \
		$(CC) -c $(CFLAGS) -I.. sunview.c; \
	fi
#
# run lint on this directory
#
../llib-ldevices.ln : $(OFILES) $(TTY_OFILES) $(UTIL_OFILES)
	lint -u $(LFLAGS) -Cdevices -I.. \
	`echo $(OFILES) $(TTY_OFILES) $(UTIL_OFILES) | sed -e 's/\.o/\.c/g'` |\
			grep -v "possible pointer alignment problem"
	@ mv llib-ldevices.ln ..
#
# Clean up directory
#
clean : tidy
	- rm -f *.o
tidy :
	- rm -f *~ .*~ *.orig *.rej
	@ sleep 1
#
# make Makefile (and descrip.mms for VMS)
#
make :
	../make_make -u -v -f $(MAKE_FILE) \
		$(OFILES) $(TTY_OFILES) $(UTIL_OFILES) \
		-D ../charset.h -D ../devices.h -D ../tree.h \
		-D ../sm.h -D ../options.h -D ../stdgraph.h -D ../tty.h \
		-O copyright.h -O sm_declare.h
#
# dependencies
#
#START_DEPEND
stgsetup.o :	../devices.h
stgsetup.o :	../options.h
stgsetup.o :	../sm.h
stgsetup.o :	../stdgraph.h
stgsetup.o :	../tty.h
stgencode.o :	../charset.h
stgencode.o :	../options.h
stgencode.o :	../stdgraph.h
stgencode.o :	../tty.h
stgerase.o :	../options.h
stgerase.o :	../sm.h
stgerase.o :	../stdgraph.h
stgerase.o :	../tty.h
stgcursor.o :	../charset.h
stgcursor.o :	../options.h
stgcursor.o :	../sm.h
stgcursor.o :	../stdgraph.h
stgcursor.o :	../tty.h
stgclose.o :	../options.h
stgclose.o :	../stdgraph.h
stgclose.o :	../tree.h
stgclose.o :	../tty.h
stgline.o :	../options.h
stgline.o :	../sm.h
stgline.o :	../stdgraph.h
stgline.o :	../tty.h
stgreloc.o :	../options.h
stgreloc.o :	../sm.h
stgreloc.o :	../stdgraph.h
stgreloc.o :	../tty.h
stgchar.o :	../options.h
stgchar.o :	../sm.h
stgchar.o :	../stdgraph.h
stgchar.o :	../tty.h
stglist.o :	../options.h
stgfillpt.o :	../options.h
stgfillpt.o :	../sm.h
stgfillpt.o :	../stdgraph.h
stgfillpt.o :	../tty.h
imagen.o :	../options.h
imagen.o :	../sm.h
imagen.o :	../stdgraph.h
imagen.o :	../tty.h
raster.o :	../devices.h
raster.o :	../options.h
raster.o :	../sm.h
raster.o :	../stdgraph.h
raster.o :	../tty.h
sgi.o :	../devices.h
sgi.o :	../options.h
sgi.o :	../sm.h
sunview.o :	SM_icon.h
sunview.o :	../options.h
sunview.o :	../sm.h
sunwindows.o :	../options.h
sunwindows.o :	../sm.h
x10.o :	cursor.h
x10.o :	../options.h
x10.o :	../sm.h
x11.o :	SM_icon.h
x11.o :	../devices.h
x11.o :	../options.h
x11.o :	../sm.h
unixpc.o :	../options.h
unixpc.o :	../sm.h
unixpc.o :	../stdgraph.h
unixpc.o :	../tty.h
unixpc.o :	upcrast.c
metafile.o :	../devices.h
metafile.o :	../options.h
metafile.o :	../sm.h
ttyindex.o :	../options.h
ttyindex.o :	../tty.h
ttyget.o :	../options.h
ttyget.o :	../tty.h
ttyopen.o :	cacheg.dat
ttyopen.o :	../options.h
ttyopen.o :	../tty.h
ttyputs.o :	../options.h
ttyputs.o :	../tty.h
ttyio.o :	../options.h
ttyio.o :	../sm.h
ttyio.o :	../stdgraph.h
ttyio.o :	../tty.h
get1char.o :	../charset.h
get1char.o :	../devices.h
get1char.o :	../options.h
gstrcpy.o :	../options.h
svgalib.o :	../sm.h
