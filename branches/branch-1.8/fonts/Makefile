#
# "$Id: Makefile,v 1.1.2.3 2002/06/06 19:43:37 mike Exp $"
#
#   Makefile for HTMLDOC font files.
#
#   Copyright 1997-2002 by Easy Software Products.
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
# Standard fonts...
#

FONTS	=	\
		Courier \
		Courier-Bold \
		Courier-BoldOblique \
		Courier-Oblique \
		Helvetica \
		Helvetica-Bold \
		Helvetica-BoldOblique \
		Helvetica-Oblique \
		Symbol \
		Times-Bold \
		Times-BoldItalic \
		Times-Italic \
		Times-Roman


#
# Make everything...
#

all:


#
# Install everything...
#

install:
	echo "Installing font files in $(datadir)/htmldoc/fonts..."
	if [ ! -d $(datadir)/htmldoc/fonts ]; then\
		$(MKDIR) $(datadir)/htmldoc/fonts;\
	fi
	for font in $(FONTS); do \
		$(CP) $$font.afm $(datadir)/htmldoc/fonts; \
		$(CP) $$font.pfa $(datadir)/htmldoc/fonts; \
	done
	$(CHMOD) ugo+r $(datadir)/htmldoc/fonts/*


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling font files from $(datadir)/htmldoc/fonts..."
	for font in $(FONTS); do \
		$(RM) $(datadir)/htmldoc/fonts/$$font.afm; \
		$(RM) $(datadir)/htmldoc/fonts/$$font.pfa; \
	done
	-$(RMDIR) $(datadir)/htmldoc/fonts


#
# Clean out object and library files...
#

clean:


#
# End of "$Id: Makefile,v 1.1.2.3 2002/06/06 19:43:37 mike Exp $".
#
