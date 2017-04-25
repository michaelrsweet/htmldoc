#
# Makefile for HTMLDOC, an HTML document processing program.
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
# Object files...
#

OBJS	=	array.o \
		gui.o \
		epub.o \
		file.o \
		html.o \
		htmldoc.o \
		htmllib.o \
		htmlsep.o \
		http.o \
		http-addr.o \
		http-addrlist.o \
		http-support.o \
		image.o \
		iso8859.o \
		license.o \
		md5.o \
		markdown.o \
		mmd.o \
		progress.o \
		ps-pdf.o \
		rc4.o \
		snprintf.o \
		string.o \
		tls.o \
		toc.o \
		util.o \
		zipc.o


#
# Make everything...
#

all:	htmldoc$(EXEEXT)

#
# Install everything...
#

install:	all
	$(INSTALL_DIR) $(BUILDROOT)$(bindir)
	$(INSTALL_BIN) htmldoc$(EXEEXT) $(BUILDROOT)$(bindir)


#
# Clean out object and executable files...
#

clean:
	$(RM) $(OBJS) htmldoc$(EXEEXT)


#
# Update dependencies...
#

depend:
	$(CC) -MM $(CFLAGS) $(OBJS:.o=.c) >Dependencies
	$(CXX) -MM $(CXXFLAGS) $(OBJS:.o=.cxx) >>Dependencies


#
# htmldoc
#

htmldoc$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) $(OBJS) $(LIBS)
	if test `uname` = Darwin; then \
		$(MAKE) -$(MAKEFLAGS) htmldoc.app || exit 1; \
	fi

htmldoc.app:	htmldoc
	echo Creating application bundle...
	$(RM) -r htmldoc.app
	$(MKDIR) htmldoc.app
	$(MKDIR) htmldoc.app/Contents
	$(CP) ../desktop/htmldoc.plist htmldoc.app/Contents/Info.plist
	$(MKDIR) htmldoc.app/Contents/MacOS
	$(CP) htmldoc htmldoc.app/Contents/MacOS
	$(MKDIR) htmldoc.app/Contents/Resources
	$(CP) ../desktop/htmldoc.icns htmldoc.app/Contents/Resources
	$(CP) ../doc/help.html htmldoc.app/Contents/Resources
	$(MKDIR) htmldoc.app/Contents/Resources/data
	$(CP) ../data/cp-* htmldoc.app/Contents/Resources/data
	$(CP) ../data/iso-* htmldoc.app/Contents/Resources/data
	$(CP) ../data/koi8-r htmldoc.app/Contents/Resources/data
	$(CP) ../data/prolog.ps htmldoc.app/Contents/Resources/data
	$(CP) ../data/psglyphs htmldoc.app/Contents/Resources/data
	$(MKDIR) htmldoc.app/Contents/Resources/fonts
	$(CP) ../fonts/*.afm htmldoc.app/Contents/Resources/fonts
	$(CP) ../fonts/*.pfa htmldoc.app/Contents/Resources/fonts


#
# Dependencies...
#

$(OBJS):	../Makedefs
tls.o:		tls-darwin.c tls-gnutls.c tls-sspi.c
include Dependencies
