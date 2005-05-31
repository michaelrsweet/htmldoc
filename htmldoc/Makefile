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
#       Hollywood, Maryland 20636 USA
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

OBJS	=	\
		gui.o \
		file.o \
		html.o \
		htmldoc.o \
		htmllib.o \
		htmlsep.o \
		http.o \
		http-addr.o \
		http-support.o \
		image.o \
		iso8859.o \
		license.o \
		margin.o \
		md5.o \
		progress.o \
		ps-pdf.o \
		rc4.o \
		snprintf.o \
		string.o \
		style.o \
		stylefont.o \
		stylemedia.o \
		stylesheet.o \
		styletree.o \
		toc.o \
		util.o

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
		license.cxx \
		margin.cxx \
		md5.c \
		progress.cxx \
		ps-pdf.cxx \
		rc4.c \
		snprintf.c \
		string.c \
		style.cxx \
		stylefont.cxx \
		stylemedia.cxx \
		stylesheet.cxx \
		styletree.cxx \
		testhtml.cxx \
		teststyle.cxx \
		toc.cxx \
		util.cxx


#
# Make everything...
#

all:	htmldoc$(EXEEXT) teststyle$(EXEEXT)

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
# Uninstall everything...
#

uninstall:


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc$(EXEEXT)


#
# Update dependencies...
#

depend:
	makedepend -Y -I.. -f Dependencies $(SRCS)


#
# htmldoc
#

htmldoc$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) $(OBJS) $(LIBS)
	$(POST) htmldoc$(EXEEXT)


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
		md5.o rc4.o html.o htmlsep.o ../license3/liblicense.a $(LIBS)

htmllibd.o:	htmllib.cxx ../config.h ../Makedefs debug.h html.h htmldoc.h \
		image.h iso8859.h types.h
	echo Compiling $<...
	$(CXX) $(CXXFLAGS) -DDEBUG -o htmllibd.o -c htmllib.cxx


#
# teststyle
#

teststyle$(EXEEXT):	teststyle.o \
	                file.o http.o http-addr.o http-support.o \
			snprintf.o string.o \
			style.o stylefont.o stylemedia.o stylesheet.o \
			../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o teststyle$(EXEEXT) teststyle.o \
	                file.o http.o http-addr.o http-support.o \
			snprintf.o string.o \
			style.o stylefont.o stylemedia.o stylesheet.o $(LIBS)

teststyle.o:	../Makedefs ../config.h


#
# Dependencies...
#

$(OBJS):	../Makedefs ../config.h

include Dependencies


#
# End of "$Id$".
#
