#
# Makefile for HTMLDOC documentation files.
#
# Copyright 2011-2017 by Michael R Sweet.
# Copyright 1997-2010 by Easy Software Products.
#
# This program is free software.  Distribution and use rights are outlined in
# the file "COPYING".
#

#
# Include common definitions...
#

include ../Makedefs


#
# Documentation files...
#

SOURCES	=	1-intro.html \
		2-using.html \
		3-cmdref.html \
		4-htmlref.html \
		a-license.html \
		b-book.html

DOCUMENTS =	htmldoc.html htmldoc.pdf htmldoc.ps
DOCFILES =	help.html htmldoc.pdf

HTMLDOC	=	../htmldoc/htmldoc$(EXEEXT) --datadir .. --strict


#
# Make everything...
#

all: $(DOCUMENTS) htmldoc.1


#
# Install everything...
#

install: $(DOCUMENTS)
	if test ! -d $(datadir)/doc/htmldoc; then\
		$(MKDIR) $(datadir)/doc/htmldoc;\
	fi
	$(CP) $(DOCFILES) $(datadir)/doc/htmldoc
	$(CHMOD) ugo+r $(datadir)/doc/htmldoc/*
	if test ! -d $(mandir)/man1; then\
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
# htmldoc.d (directory)
#

.PHONY: htmldoc.d
htmldoc.d:	$(SOURCES) ../htmldoc/htmldoc$(EXEEXT)
	echo Formatting htmldoc.d...
	if test -d htmldoc.d; then \
		$(RM) -r htmldoc.d; \
	fi
	$(MKDIR) htmldoc.d
	$(VALGRIND) $(HTMLDOC) --batch htmldoc.book -t htmlsep -d htmldoc.d


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
	nroff -man htmldoc.man >htmldoc.1
