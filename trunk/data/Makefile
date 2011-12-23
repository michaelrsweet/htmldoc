#
# "$Id$"
#
# Makefile for HTMLDOC data files.
#
# Copyright 2011 by Michael R Sweet.
# Copyright 1997-2010 by Easy Software Products.
#
# This program is free software.  Distribution and use rights are outlined in
# the file "COPYING.txt".
#

#
# Include common definitions...
#

include ../Makedefs


#
# Character set/glyph files...
#

FILES	=	cp-1250 cp-1251 cp-1252 cp-1253 cp-1254 cp-1255 \
		cp-1256 cp-1257 cp-1258 cp-874 \
		iso-8859-1 iso-8859-14 iso-8859-15 iso-8859-2 iso-8859-3 \
		iso-8859-4 iso-8859-5 iso-8859-6 iso-8859-7 iso-8859-8 \
		iso-8859-9 \
		koi8-r prolog.ps psglyphs standard.css


#
# Make everything...
#

all:


#
# Install everything...
#

install:
	if [ ! -d $(datadir)/htmldoc/data ]; then\
		$(MKDIR) $(datadir)/htmldoc/data;\
	fi
	$(CP) $(FILES) $(datadir)/htmldoc/data
	$(CHMOD) ugo+r $(datadir)/htmldoc/data/*


#
# Clean out object and library files...
#

clean:

#
# End of "$Id$".
#
