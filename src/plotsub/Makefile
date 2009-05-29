#
# Default rules for compiling code.
#
.c.o :
	$(CC) -c $(CFLAGS) -I.. $*.c
#
OBJECT = \
	compress.o dummies.o \
        axis.o chopper.o colour.o connect.o contour.o cursor.o declare.o \
	devices.o dither.o interface.o label.o line.o point.o \
	images.o relocate.o set.o shade.o string.o surfaces.o vec_image.o
#
#
# The target to update the library
#
../libplotsub.a : $(OBJECT)
	ar r ../libplotsub.a $?
	@- if test "$(RANLIB)" != "" -a -f "$(RANLIB)"; then \
		$(RANLIB) ../libplotsub.a; fi
../libparser.a : interface.o
	ar r ../libparser.a $?
	@- if test "$(RANLIB)" != "" -a -f "$(RANLIB)"; then \
		$(RANLIB) ../libparser.a; fi
#
# run lint on this directory
#
../llib-lplotsub.ln : $(OBJECT)
	lint -u $(LFLAGS) -I.. -Cplotsub \
	`echo $(OBJECT) | sed -e 's/\.o/\.c/g' -e 's/interface\.c//'` |\
		grep -v "possible pointer alignment problem"
	@ mv llib-lplotsub.ln ..
clean : tidy
	- rm -f *.o
tidy :
	- rm -f *~ .*~ *.orig *.rej
	@ sleep 1
#
# make Makefile (and descrip.mms for VMS)
#
make :
	../make_make -u -v -f $(MAKE_FILE) $(OBJECT) $(DEVICES) \
		-D ../charset.h \
		-D ../defs.h -D ../devices.h -D ../fonts.h \
		-D ../sm.h -D ../options.h -D ../stdgraph.h -D ../tree.h \
		-D ../tty.h -D ../vectors.h -D ../yaccun.h \
		-O copyright.h -O sm_declare.h
#
# dependencies
#
#START_DEPEND
compress.o :	../defs.h
compress.o :	../options.h
compress.o :	../sm.h
dummies.o :	../defs.h
dummies.o :	../devices.h
dummies.o :	../options.h
dummies.o :	../sm.h
dummies.o :	../tree.h
axis.o :	../defs.h
axis.o :	../options.h
axis.o :	../sm.h
axis.o :	../vectors.h
chopper.o :	../defs.h
chopper.o :	../devices.h
chopper.o :	../options.h
chopper.o :	../sm.h
chopper.o :	../stdgraph.h
chopper.o :	../tty.h
colour.o :	../devices.h
colour.o :	../options.h
colour.o :	../sm.h
colour.o :	../vectors.h
connect.o :	../defs.h
connect.o :	../options.h
connect.o :	../sm.h
contour.o :	image.h
contour.o :	../options.h
contour.o :	../sm.h
cursor.o :	../charset.h
cursor.o :	../devices.h
cursor.o :	../options.h
cursor.o :	../sm.h
cursor.o :	../vectors.h
declare.o :	../options.h
declare.o :	../sm.h
devices.o :	../devices.h
devices.o :	../options.h
devices.o :	../sm.h
interface.o :	../defs.h
interface.o :	../devices.h
interface.o :	../options.h
interface.o :	../sm.h
interface.o :	../tree.h
label.o :	../defs.h
label.o :	../options.h
label.o :	../sm.h
line.o :	../defs.h
line.o :	../devices.h
line.o :	../options.h
line.o :	../sm.h
point.o :	../defs.h
point.o :	../devices.h
point.o :	../options.h
point.o :	../sm.h
point.o :	../vectors.h
point.o :	../yaccun.h
images.o :	image.h
images.o :	../options.h
images.o :	../sm.h
images.o :	../tree.h
images.o :	../tty.h
images.o :	../vectors.h
relocate.o :	../defs.h
relocate.o :	../devices.h
relocate.o :	../options.h
relocate.o :	../sm.h
set.o :	../defs.h
set.o :	../devices.h
set.o :	../options.h
set.o :	../sm.h
set.o :	../vectors.h
shade.o :	../options.h
shade.o :	../sm.h
string.o :	../defs.h
string.o :	../devices.h
string.o :	../fonts.h
string.o :	../options.h
string.o :	../sm.h
string.o :	../tree.h
surfaces.o :	image.h
surfaces.o :	../options.h
surfaces.o :	../sm.h
