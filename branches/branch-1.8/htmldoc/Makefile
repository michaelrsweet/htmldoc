#
#   "$Id$"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-2005 by Easy Software Products.
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
#       Hollywood, Maryland 20636-3142 USA
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

OBJS	=	gui.o file.o html.o htmldoc.o htmllib.o htmlsep.o http.o \
		http-addr.o http-support.o image.o iso8859.o license.o md5.o \
		progress.o ps-pdf.o rc4.o snprintf.o string.o toc.o util.o


#
# Make everything...
#

all:	htmldoc$(EXEEXT)

#
# Install everything...
#

install:	all
	if [ ! -d $(bindir) ]; then\
		$(MKDIR) $(bindir);\
	fi
	cp htmldoc$(EXEEXT) $(bindir)
	chmod ugo+rx $(bindir)/htmldoc$(EXEEXT)


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc$(EXEEXT)


#
# htmldoc
#

htmldoc$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) $(OBJS) $(LIBS)
	$(POST) htmldoc$(EXEEXT)

htmldoc.purify$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	purify $(CXX) $(LDFLAGS) -o htmldoc.purify$(EXEEXT) $(OBJS) $(LIBS)

file.o:		file.h http.h md5.h progress.h hdstring.h
gui.o:		gui.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h ../desktop/htmldoc.xbm ../desktop/htmldoc.xpm
html.o:		html.h htmldoc.h image.h iso8859.h progress.h types.h
htmldoc.o:	gui.h html.h htmldoc.h image.h iso8859.h progress.h types.h
htmllib.o:	debug.h htmldoc.h html.h image.h iso8859.h progress.h types.h
htmlsep.o:	html.h htmldoc.h image.h iso8859.h progress.h types.h
http.o:		http.h http-private.h hdstring.h
http-addr.o:	http.h http-private.h hdstring.h
http-support.o:	http.h http-private.h hdstring.h
image.o:	debug.h htmldoc.h image.h progress.h types.h
iso8859.o:	iso8859.h types.h
license.o:	gui.h
md5.o:		md5.h
ps-pdf.o:	debug.h html.h htmldoc.h image.h iso8859.h md5.h progress.h \
		rc4.h types.h
rc4.o:		rc4.h
string.o:	hdstring.h
toc.o:		html.h htmldoc.h image.h iso8859.h types.h
util.o:		htmldoc.h types.h


#
# testhtml
#

testhtml$(EXEEXT):	testhtml.o file.o gui.o htmllibd.o http.o \
		http-addr.o http-support.o image.o iso8859.o license.o \
		md5.o rc4.o progress.o ps-pdf.o toc.o util.o html.o \
		htmlsep.o string.o ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o testhtml$(EXEEXT) testhtml.o file.o gui.o htmllibd.o \
		http.o http-addr.o http-support.o image.o iso8859.o license.o \
		progress.o ps-pdf.o toc.o util.o string.o \
		md5.o rc4.o html.o htmlsep.o $(LIBS)

testhtml.o:	gui.h html.h htmldoc.h image.h iso8859.h progress.h types.h
htmllibd.o:	htmllib.cxx ../config.h ../Makedefs debug.h html.h htmldoc.h \
		image.h iso8859.h types.h
	echo Compiling $<...
	$(CXX) $(CXXFLAGS) -DDEBUG -o htmllibd.o -c htmllib.cxx

$(OBJS):	../Makedefs ../config.h


#
# End of "$Id$".
#
