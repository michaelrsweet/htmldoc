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
#       Attn: HTMLDOC Licensing Information
#       Easy Software Products
#       516 Rio Grand Ct
#       Morgan Hill, CA 95037 USA
#
#       http://www.htmldoc.org/
#

#
# Include common definitions...
#

include ../Makedefs


#
# Object files...
#

BASEOBJS =	\
		entity.o \
		file.o \
		html.o \
		htmllib.o \
		htmlsep.o \
		http.o \
		http-addr.o \
		http-addrlist.o \
		http-support.o \
		image.o \
		index.o \
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
OBJS	=	\
		$(BASEOBJS) \
		htmldoc.o \
		testhtml.o \
		teststyle.o
SRCS	=	\
		entity.cxx \
		file.c \
		html.cxx \
		htmldoc.cxx \
		htmllib.cxx \
		htmlsep.cxx \
		http.c \
		http-addr.c \
		http-addrlist.c \
		http-support.c \
		image.cxx \
		index.cxx \
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

all:	htmldoc$(EXEEXT) testhtml$(EXEEXT) teststyle$(EXEEXT)

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
	$(RM) $(bindir)/htmldoc$(EXEEXT)


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

htmldoc$(EXEEXT):	htmldoc.o $(BASEOBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) htmldoc.o $(BASEOBJS) $(LIBS)


#
# testhtml
#

testhtml$(EXEEXT):	testhtml.o $(BASEOBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o testhtml$(EXEEXT) testhtml.o $(BASEOBJS) $(LIBS)



#
# teststyle
#

teststyle$(EXEEXT):	teststyle.o $(BASEOBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o teststyle$(EXEEXT) teststyle.o $(BASEOBJS) $(LIBS)


#
# Dependencies...
#

$(OBJS):	../Makedefs ../config.h

include Dependencies


#
# End of "$Id$".
#
