#
#   "$Id: Makefile,v 1.37 2004/03/31 07:28:13 mike Exp $"
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

LIBOBJS	=	\
		file.o \
		gui.o \
		html.o \
		htmldoc.o \
		htmllib.o \
		htmlsep.o \
		http.o \
		http-addr.o \
		http-support.o \
		image.o \
		iso8859.o \
		margin.o \
		md5.o \
		progress.o \
		ps-pdf.o \
		rc4.o \
		snprintf.o \
		string.o \
		toc.o \
		util.o

OBJS	=	\
		$(LIBOBJS) \
		htmldoc.o \
		htmllibd.o \
		testhtml.o

SRCS	=	\
		file.c \
		gui.cxx \
		html.cxx \
		htmldoc.cxx \
		htmllib.cxx \
		htmlsep.cxx \
		http-addr.c \
		http.c \
		http-support.c \
		image.cxx \
		iso8859.cxx \
		margin.cxx \
		md5.c \
		progress.cxx \
		ps-pdf.cxx \
		rc4.c \
		snprintf.c \
		string.c \
		testhtml.cxx \
		toc.cxx \
		util.cxx

TARGETS	=	\
		libhtmldoc.a \
		htmldoc$(EXEEXT) \
		testhtml$(EXEEXT)


#
# Make everything...
#

all:		$(TARGETS)


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
	$(RM) *.bak
	$(RM) *.bck
	$(RM) core
	$(RM) core.* 
	$(RM) $(OBJS)
	$(RM) $(TARGETS)


#
# Update dependencies...
#

depend:
	makedepend -Y -I.. -f Dependencies $(SRCS) testhtml.cxx


#
# libhtmldoc.a
#

libhtmldoc.a:	$(LIBOBJS) ../Makedefs
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@


#
# htmldoc
#

htmldoc$(EXEEXT):	htmldoc.o libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) htmldoc.o libhtmldoc.a $(LIBS)
	$(POST) htmldoc$(EXEEXT)


#
# testhtml
#

testhtml$(EXEEXT):	testhtml.o libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o testhtml$(EXEEXT) testhtml.o libhtmldoc.a $(LIBS)


#
# Dependencies...
#

$(OBJS):	../Makedefs ../config.h

include Dependencies


#
# End of "$Id: Makefile,v 1.37 2004/03/31 07:28:13 mike Exp $".
#
