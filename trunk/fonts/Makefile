#
# "$Id: Makefile,v 1.1 2002/02/17 14:18:58 mike Exp $"
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
		Times-Roman \
		ZapfChancery \
		ZapfDingbats


#
# Make everything...
#

all:


#
# Install everything...
#

install:
	echo "Installing font files in $(datadir)/htmldoc/fonts..."
	$(INSTALL_DIR) $(datadir)/htmldoc/fonts;
	for font in $(FONTS); do \
		$(INSTALL_DATA) $$font.afm $(datadir)/htmldoc/fonts; \
		$(INSTALL_DATA) $$font.pfa $(datadir)/htmldoc/fonts; \
	done


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
# End of "$Id: Makefile,v 1.1 2002/02/17 14:18:58 mike Exp $".
#
