#
# "$Id$"
#
#   Makefile for HTMLDOC data files.
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
#       Attn: HTMLDOC Licensing Information
#       Easy Software Products
#       516 Rio Grand Ct
#       Morgan Hill, CA 95037 USA
#
#       http://www.htmldoc.org/
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
