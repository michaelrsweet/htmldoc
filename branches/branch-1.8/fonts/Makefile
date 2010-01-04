#
# "$Id$"
#
#   Makefile for HTMLDOC font files.
#
#   Copyright 1997-2010 by Easy Software Products.
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "COPYING.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#     Attn: HTMLDOC Licensing Information
#     Easy Software Products
#     516 Rio Grand Ct
#     Morgan Hill, CA 95037 USA
#
#     http://www.htmldoc.org/
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
		Dingbats \
		Helvetica \
		Helvetica-Bold \
		Helvetica-BoldOblique \
		Helvetica-Oblique \
		Monospace \
		Monospace-Bold \
		Monospace-BoldOblique \
		Monospace-Oblique \
		Sans \
		Sans-Bold \
		Sans-BoldOblique \
		Sans-Oblique \
		Serif-Roman \
		Serif-Bold \
		Serif-BoldOblique \
		Serif-Oblique \
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
# End of "$Id$".
#
