#
# Makefile for HTMLDOC font files.
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
	$(INSTALL_DIR) $(BUILDROOT)$(datadir)/htmldoc/fonts
	for font in $(FONTS); do \
		$(INSTALL_DATA) $$font.afm $(BUILDROOT)$(datadir)/htmldoc/fonts; \
		$(INSTALL_DATA) $$font.pfa $(BUILDROOT)$(datadir)/htmldoc/fonts; \
	done


#
# Uninstall everything...
#

uninstall:
	echo "Uninstalling font files from $(datadir)/htmldoc/fonts..."
	for font in $(FONTS); do \
		$(RM) $(BUILDROOT)$(datadir)/htmldoc/fonts/$$font.afm; \
		$(RM) $(BUILDROOT)$(datadir)/htmldoc/fonts/$$font.pfa; \
	done
	-$(RMDIR) $(BUILDROOT)$(datadir)/htmldoc/fonts


#
# Clean out object and library files...
#

clean:
