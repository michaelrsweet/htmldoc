#
#   "$Id: Makefile,v 1.14 2000/10/16 03:25:06 mike Exp $"
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
# Targets...
#

TARGETS	=	libhtmldoc.a htmldoc topdf


#
# Object files...
#

LIBOBJS	=	file.o flate.o format.o html.o htmldoc.o htmllib.o http.o \
		image.o iso8859.o jpeg.o md5.o progress.o pdf.o rc4.o \
		render.o ps.o ps-pdf.o string.o toc.o util.o
APPOBJS	=	gui.o main.o
CGIOBJS	=	topdf.o
OBJS	=	$(LIBOBJS) $(APPOBJS) $(LIBOBJS)


#
# Make everything...
#

all:	$(TARGETS)


#
# Install everything...
#

install:	all
	if [ ! -d $(bindir) ]; then\
		$(MKDIR) $(bindir);\
	fi
	cp htmldoc $(bindir)
	chmod ugo+rx $(bindir)/htmldoc


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) $(TARGETS)


#
# libhtmldoc.a
#

libhtmldoc.a:	$(LIBOBJS) ../Makedefs
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@

file.o:		file.h http.h md5.h progress.h string.h
format.o:	debug.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h
html.o:		html.h htmldoc.h image.h iso8859.h progress.h types.h
htmldoc.o:	html.h htmldoc.h image.h iso8859.h progress.h types.h
htmllib.o:	debug.h htmldoc.h html.h image.h iso8859.h progress.h types.h
http.o:		http.h string.h
image.o:	debug.h htmldoc.h image.h progress.h types.h
iso8859.o:	iso8859.h types.h
md5.o:		md5.h
pdf.o:		debug.h html.h htmldoc.h image.h iso8859.h md5.h progress.h \
		rc4.h types.h
ps.o:		debug.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h
rc4.o:		rc4.h
render.o:	debug.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h
string.o:	string.h
toc.o:		html.h htmldoc.h image.h iso8859.h types.h
util.o:		htmldoc.h types.h


#
# htmldoc
#

htmldoc:	$(APPOBJS) libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc $(APPOBJS) libhtmldoc.a $(LIBS)

main.o:		gui.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h
gui.o:		gui.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h htmldoc.xbm htmldoc.xpm


#
# topdf
#

topdf:	$(CGIOBJS) libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o topdf $(CGIOBJS) libhtmldoc.a $(LIBS)

topdf.o:	html.h htmldoc.h image.h iso8859.h progress.h \
		types.h


$(OBJS):	../Makedefs ../config.h


#
# End of "$Id: Makefile,v 1.14 2000/10/16 03:25:06 mike Exp $".
#
