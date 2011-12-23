#
# "$Id$"
#
#   Makefile for HTMLDOC documentation files.
#
#   Copyright 2011 by Michael R Sweet.
#   Copyright 1997-2010 by Easy Software Products.
#
#   This program is free software.  Distribution and use rights are outlined in
#   the file "COPYING.txt".
#

#
# Include common definitions...
#

include ../Makedefs


#
# Documentation files...
#

SOURCES	=	intro.html 1-install.html 2-starting.html \
		3-books.html 4-cmdline.html 5-cgi.html \
		6-htmlref.html 7-guiref.html 8-cmdref.html \
		a-license.html b-book.html c-relnotes.html \
		d-compile.html

DOCUMENTS =	htmldoc.html htmldoc.pdf htmldoc.ps
DOCFILES =	help.html htmldoc.pdf

HTMLDOC	=	../htmldoc/htmldoc$(EXEEXT) --datadir .. --strict


#
# Make everything...
#

all: $(DOCUMENTS)


#
# Install everything...
#

install: $(DOCUMENTS)
	if [ ! -d $(datadir)/doc/htmldoc ]; then\
		$(MKDIR) $(datadir)/doc/htmldoc;\
	fi
	$(CP) $(DOCFILES) $(datadir)/doc/htmldoc
	$(CHMOD) ugo+r $(datadir)/doc/htmldoc/*
	if [ ! -d $(mandir)/man1 ]; then\
		$(MKDIR) $(mandir)/man1;\
	fi
	$(CP) htmldoc.man $(mandir)/man1/htmldoc.1
	$(CHMOD) ugo+r $(mandir)/man1/htmldoc.1


#
# Clean out document files...
#

clean:
	$(RM) $(DOCUMENTS)


#
# htmldoc program (dummy rule)
#

../htmldoc/htmldoc$(EXEEXT):
	echo Rebuilding documentation...


#
# htmldoc.html
#

htmldoc.html:	$(SOURCES) ../htmldoc/htmldoc$(EXEEXT)
	echo Formatting htmldoc.html...
	$(VALGRIND) $(HTMLDOC) --batch htmldoc.book -f htmldoc.html


#
# htmldoc.pdf
#

htmldoc.pdf:	$(SOURCES) ../htmldoc/htmldoc$(EXEEXT)
	echo Formatting htmldoc.pdf...
	$(VALGRIND) $(HTMLDOC) --batch htmldoc.book -f htmldoc.pdf


#
# htmldoc.ps
#

htmldoc.ps:	$(SOURCES) ../htmldoc/htmldoc$(EXEEXT)
	echo Formatting htmldoc.ps...
	$(VALGRIND) $(HTMLDOC) --batch htmldoc.book -f htmldoc.ps


#
# htmldoc.1
#

htmldoc.1:	htmldoc.man
	echo Formatting htmldoc.1...
	groff -T ascii -man htmldoc.man >htmldoc.1


#
# End of "$Id$".
#
