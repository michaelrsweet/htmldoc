#
# "$Id$"
#
# Makefile for HTMLDOC font files.
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
	if test ! -d $(datadir)/htmldoc/fonts; then\
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
	-$(RMDIR) $(datadir)/htmldoc/fonts


#
# Clean out object and library files...
#

clean:


#
# End of "$Id$".
#
