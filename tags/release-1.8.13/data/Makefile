#
# "$Id: Makefile,v 1.4.2.3 2001/02/20 15:06:55 mike Exp $"
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
# Character set/glyph files...
#

FILES	=	iso-8859-1 iso-8859-14 iso-8859-15 iso-8859-2 iso-8859-3 \
		iso-8859-4 iso-8859-5 iso-8859-6 iso-8859-7 iso-8859-8 \
		iso-8859-9 koi8-r psglyphs


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
# End of "$Id: Makefile,v 1.4.2.3 2001/02/20 15:06:55 mike Exp $".
#
