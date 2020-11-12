#
# Makefile for HTMLDOC desktop files.
#
# Copyright 2017 by Michael R Sweet.
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
# Make everything...
#

all:


#
# Install everything...
#

install:
	if test `uname` = Linux; then \
		$(INSTALL_DIR) $(BUILDROOT)$(datadir)/applications; \
		$(INSTALL_DATA) htmldoc.desktop $(BUILDROOT)$(datadir)/applications; \
		$(INSTALL_DIR) $(BUILDROOT)$(datadir)/mime/packages; \
		$(INSTALL_DATA) htmldoc.xml $(BUILDROOT)$(datadir)/mime/packages; \
		$(INSTALL_DIR) $(BUILDROOT)$(datadir)/pixmaps; \
		$(INSTALL_DATA) htmldoc.xpm $(BUILDROOT)$(datadir)/pixmaps; \
		$(INSTALL_DIR) $(BUILDROOT)$(datadir)/icons/hicolor/128x128/apps; \
		$(INSTALL_DATA) htmldoc-128.png $(BUILDROOT)$(datadir)/icons/hicolor/128x128/apps/htmldoc.png; \
		$(INSTALL_DIR) $(BUILDROOT)$(datadir)/icons/hicolor/256x256/apps; \
		$(INSTALL_DATA) htmldoc-256.png $(BUILDROOT)$(datadir)/icons/hicolor/256x256/apps/htmldoc.png; \
	fi


#
# Clean out object and library files...
#

clean:
