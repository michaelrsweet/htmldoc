#
#   "$Id: Makefile,v 1.13.2.15.2.4 2004/03/22 21:56:29 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-2004 by Easy Software Products.
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

TARGETS	=	libhtmldoc.a testsuite


#
# Object files...
#

HEADERS	=	\
		file.h \
		htmldoc.h \
		http.h \
		image.h \
		margin.h \
		md5.h \
		render.h \
		style.h \
		tree.h \
		types.h

LIBOBJS	=	\
		common.o \
		file.o file-common.o file-mem.o file-open.o file-std.o \
		file-ascii.o file-flate.o file-jpeg.o file-rc4.o \
		http.o http-md5.o md5.o \
		image.o image-bmp.o image-gif.o image-jpeg.o image-png.o \
		image-reduce.o index.o list.o margin.o \
		render.o \
		style.o stylefont.o stylemedia.o stylesheet.o toc.o \
		tree.o

OBJS	=	$(LIBOBJS) testsuite.o

#OBJS	=	gui.o htmldoc.o


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
	cp htmldoc$(EXEEXT) $(bindir)
	chmod ugo+rx $(bindir)/htmldoc$(EXEEXT)


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.bck
	rm -f *.bak


#
# htmldoc
#

htmldoc$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) $(OBJS) $(LIBS)
	$(POST) htmldoc$(EXEEXT)

file.o:		file.h http.h md5.h progress.h hdstring.h
gui.o:		gui.h html.h htmldoc.h image.h iso8859.h progress.h \
		types.h htmldoc.xbm htmldoc.xpm
html.o:		html.h htmldoc.h image.h iso8859.h progress.h types.h
htmldoc.o:	gui.h html.h htmldoc.h image.h iso8859.h progress.h types.h
htmllib.o:	debug.h htmldoc.h html.h image.h iso8859.h progress.h types.h
htmlsep.o:	html.h htmldoc.h image.h iso8859.h progress.h types.h
http.o:		http.h http-private.h hdstring.h
http-addr.o:	http.h http-private.h hdstring.h
http-support.o:	http.h http-private.h hdstring.h
image.o:	debug.h htmldoc.h image.h progress.h types.h
iso8859.o:	iso8859.h types.h
margin.o:	margin.h
md5.o:		md5.h
ps-pdf.o:	debug.h html.h htmldoc.h image.h iso8859.h md5.h progress.h \
		rc4.h types.h
rc4.o:		rc4.h
render.o:	htmldoc.h image.h margin.h md5.h rc4.h render.h
string.o:	hdstring.h
toc.o:		html.h htmldoc.h image.h iso8859.h types.h
util.o:		htmldoc.h types.h


#
# libhtmldoc.a
#

libhtmldoc.a:	$(LIBOBJS) ../Makedefs
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@


#
# testsuite
#

testsuite:	testsuite.o libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(RM) $@
	$(CXX) $(LDFLAGS) -o testsuite testsuite.o libhtmldoc.a $(LIBS)


$(OBJS):	../Makedefs


#
# reference.html/xml
#

reference.html:	$(LIBOBJS:.o=.cxx) $(HEADERS)
	$(RM) reference.xml
	mxmldoc reference.xml $(LIBOBJS:.o=.cxx) $(HEADERS) >reference.html


#
# depend
#

depend:
	makedepend -Y -I.. -f Dependencies $(LIBOBJS:.o=.cxx) testsuite.cxx

include Dependencies


#
# End of "$Id: Makefile,v 1.13.2.15.2.4 2004/03/22 21:56:29 mike Exp $".
#
