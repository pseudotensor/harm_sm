#
# Make the info files
#
IFLAGS = --footnote-style "end"
MAKEINFO = makeinfo
#
first : sm
all : sm tutorial
#
FAQ : _FAQ
_FAQ :
	@ $(MAKEINFO) $(IFLAGS) FAQ.txi
install : _install
_install :
	@ $(MAKEINFO) $(IFLAGS) install.txi
release : notes
notes : _notes
_notes :
	@ $(MAKEINFO) $(IFLAGS) notes.txi
sm : _sm;
_sm :
	@ $(MAKEINFO) $(IFLAGS) sm.txi
tutorial : _tutorial
_tutorial :
	@ $(MAKEINFO) $(IFLAGS) tutorial.txi
#
# Release notes in text form
#
release.txt : notes
	info -o release.txt --subnodes -f notes
#
# And now the .dvi files
#
texindex : texindex.c
	$(CC) -o texindex texindex.c
#
# Build the SM manual from scratch
#
sm.dvi : _sm.dvi
_sm.dvi : sm.tps
	tex sm.txi
sm.tps : sm.tp texindex
	texindex sm.??
sm.tp :
	tex sm.txi
#
# The others are easy:
#
install.dvi : install.txi
	tex install.txi
notes.dvi : notes.txi
	tex notes.txi
tutorial.dvi : tutorial.aux
	tex tutorial.txi
tutorial.aux : tutorial.txi
	tex tutorial.txi
#
# You may have to add
#   -invisible " "
# to HTMLFLAGS if you have a broken browser (e.g. some versions of mosaic)
#
HTMLFLAGS = -menu -monolithic 

html : sm.html tutorial.html notes.html
sm.html :
	./texi2html $(HTMLFLAGS) sm.txi
tutorial.html : tutorial.txi
	./texi2html $(HTMLFLAGS) tutorial.txi
notes.html : notes.txi
	./texi2html $(HTMLFLAGS) notes.txi
FAQ.html : FAQ.txi
	./texi2html $(HTMLFLAGS) FAQ.txi
#
../sm.install : install
	@ if test ! -f $@; then touch $@; fi; chmod 644 $@
	@ echo "This File Is Machine-Generated. Do Not Edit It!" > $@
	@ echo "" >> $@
	@ echo "The installation instructions are now in info/install.txi" >>$@
	@ echo "which is a tex file. This file may be out of date, as it" >> $@
	@ echo "occasionally prepared from install.txi." >> $@
	@ echo "" >> $@
	@ echo "" >> $@
	@ info -o temp~ --subnodes  -f install> /dev/null 2>&1; cat temp~ >> $@
	@- rm temp~
	@ chmod 444 $@
#
installinfo : sm tutorial
	@ if test "$(DESTINFO)" = ""; then \
		echo No info directory is specified; \
	else \
		if test ! -d $(DESTINFO) ; then \
			echo creating $(DESTINFO); mkdir $(DESTINFO); \
		fi; \
		cp sm sm-[0-9]* tutorial $(DESTINFO); \
	fi
#
tidy :
	- rm -f *.dvi *.log *~ core texindex
	- rm -f *.toc *.cp *.fn *.ky *.pg *.tp *.vr
clean : tidy
	- rm -f install notes FAQ release.txt sm sm-[0-9]* tutorial
	- rm -f notes.html tutorial.html sm.html 
	- rm -f *.aux *.cps *.fns *.kys *.pgs *.tps *.vrs
empty :;

