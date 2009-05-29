#
# Default rules for compiling code.
#
.c.o :
	$(CC) -c $(CFLAGS) -I.. -DDEFAULT_PATH="\"$(DEFAULT_PATH)\"" $*.c
#
OBJECT = \
	ftruncate.o get_val.o regexp.o sort.o str_scanf.o system.o \
	tree.o utils.o vectors.o
#
#
# The target to update the library
#
../libutils.a : $(OBJECT) 
	ar r ../libutils.a $?
	@- if test "$(RANLIB)" != "" -a -f "$(RANLIB)"; then \
		$(RANLIB) ../libutils.a; fi
#
# run lint on this directory
#
../llib-lutils.ln : $(OBJECT)
	lint -u $(LFLAGS) -I.. -Cutils \
	`echo $(OBJECT) | sed -e 's/\.o/\.c/g' -e 's/interface\.c//'` |\
		grep -v "possible pointer alignment problem"
	@ mv llib-lutils.ln ..
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
		-D ../charset.h -D ../options.h -D ../sm.h \
		-D ../tree.h -D ../vectors.h \
		-O copyright.h -O sm_declare.h
#
# dependencies
#
#START_DEPEND
ftruncate.o :	../options.h
get_val.o :	../options.h
regexp.o :	../options.h
sort.o :	../options.h
str_scanf.o :	../options.h
tree.o :	../options.h
tree.o :	../tree.h
utils.o :	../sm.h
utils.o :	../options.h
vectors.o :	../options.h
vectors.o :	../tree.h
vectors.o :	../vectors.h
