#
# "$Id: Makefile,v 1.11.2.9 2002/06/11 19:36:15 mike Exp $"
#
#   Makefile for HTMLDOC documentation files.
#
#   Copyright 1997-2001 by Easy Software Products.
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "COPYING.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#       Attn: ESP Licensing Information
#       Easy Software Products
#       44141 Airport View Drive, Suite 204
#       Hollywood, Maryland 20636-3111 USA
#
#       Voice: (301) 373-9600
#       EMail: info@easysw.com
#         WWW: http://www.easysw.com
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
		a-license.html b-book.html c-relnotes.html

DOCUMENTS =	htmldoc.html htmldoc.pdf htmldoc.ps
DOCFILES =	help.html htmldoc.pdf


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
# htmldoc.html
#

htmldoc.html:	$(SOURCES) ../htmldoc/htmldoc
	echo Formatting htmldoc.html...
	../htmldoc/htmldoc --datadir .. --batch htmldoc.book -f htmldoc.html --datadir ..


#
# htmldoc.pdf
#

htmldoc.pdf:	$(SOURCES) ../htmldoc/htmldoc
	echo Formatting htmldoc.pdf...
	../htmldoc/htmldoc --datadir .. --batch htmldoc.book -f htmldoc.pdf --datadir ..


#
# htmldoc.ps
#

htmldoc.ps:	$(SOURCES) ../htmldoc/htmldoc
	echo Formatting htmldoc.ps...
	../htmldoc/htmldoc --datadir .. --batch htmldoc.book -f htmldoc.ps --datadir ..


#
# htmldoc.1
#

htmldoc.1:	htmldoc.man
	echo Formatting htmldoc.1...
	groff -T ascii -man htmldoc.man >htmldoc.1


#
# End of "$Id: Makefile,v 1.11.2.9 2002/06/11 19:36:15 mike Exp $".
#
