#
#   "$Id: Makefile,v 1.1 1999/11/12 21:39:37 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-1999 by Easy Software Products.
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
# Object files...
#

OBJS	=	gui.o file.o html.o htmldoc.o htmllib.o image.o iso8859.o \
		progress.o ps-pdf.o string.o toc.o

#
# Make everything...
#

all:	htmldoc

#
# Install everything...
#

install:	all
	if [ ! -d $(bindir) ]; then\
		mkdir -p $(bindir);\
	fi
	cp htmldoc $(bindir)
	chmod 555 $(bindir)/htmldoc


#
# clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc

#
# htmldoc
#

htmldoc:	$(OBJS) ../Makedefs ../gui/libgui.a ../jpeg/libjpeg.a \
		../png/libpng.a ../zlib/libz.a
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc $(OBJS) ../gui/libgui.a $(LIBS)

gui.o:		gui.h html.h htmldoc.h image.h iso8859.h types.h htmldoc.xbm
html.o:		html.h htmldoc.h image.h iso8859.h types.h
htmldoc.o:	gui.h html.h htmldoc.h image.h iso8859.h types.h
htmllib.o:	debug.h htmldoc.h html.h image.h iso8859.h types.h
image.o:	debug.h image.h types.h
iso8859.o:	iso8859.h types.h
ps-pdf.o:	debug.h html.h htmldoc.h image.h iso8859.h types.h
toc.o:		html.h htmldoc.h image.h iso8859.h types.h
string.o:	string.h
file.o:		file.h string.h

$(OBJS):	../Makedefs ../config.h


#
# End of "$Id: Makefile,v 1.1 1999/11/12 21:39:37 mike Exp $".
#
