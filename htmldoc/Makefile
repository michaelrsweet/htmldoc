#
#   "$Id: Makefile,v 1.7 2000/05/08 14:27:41 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-2000 by Easy Software Products.
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
		$(MKDIR) $(bindir);\
	fi
	cp htmldoc $(bindir)
	chmod 555 $(bindir)/htmldoc


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc

#
# htmldoc
#

htmldoc:	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc $(OBJS) $(LIBS)

htmldoc.purify:	$(OBJS) ../Makedefs
	echo Linking $@...
	purify $(CXX) $(LDFLAGS) -o htmldoc.purify $(OBJS) $(LIBS)

file.o:		file.h string.h
gui.o:		gui.h html.h htmldoc.h image.h iso8859.h types.h htmldoc.xbm
html.o:		html.h htmldoc.h image.h iso8859.h types.h
htmldoc.o:	gui.h html.h htmldoc.h image.h iso8859.h types.h
htmllib.o:	debug.h htmldoc.h html.h image.h iso8859.h types.h
image.o:	debug.h htmldoc.h image.h types.h
iso8859.o:	iso8859.h types.h
ps-pdf.o:	debug.h html.h htmldoc.h image.h iso8859.h types.h
toc.o:		html.h htmldoc.h image.h iso8859.h types.h
string.o:	string.h

#
# testhtml
#

testhtml:	testhtml.o file.o htmllibd.o image.o iso8859.o Makefile
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o testhtml testhtml.o file.o htmllibd.o image.o \
		iso8859.o $(LIBS)

htmllibd.o:	htmllib.cxx ../config.h ../Makedefs html.h debug.h image.h \
		iso8859.h types.h
	echo Compiling $<...
	$(CXX) $(CXXFLAGS) -DDEBUG -o htmllibd.o -c htmllib.cxx

$(OBJS):	../Makedefs ../config.h


#
# End of "$Id: Makefile,v 1.7 2000/05/08 14:27:41 mike Exp $".
#
