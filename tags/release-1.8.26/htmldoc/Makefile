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

OBJS	=	gui.o file.o html.o htmldoc.o htmllib.o htmlsep.o \
		http.o http-addr.o http-addrlist.o http-support.o image.o \
		iso8859.o license.o md5.o progress.o ps-pdf.o rc4.o \
		snprintf.o string.o toc.o util.o


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
# Update dependencies...
#

depend:
	makedepend -Y -I.. -fDependencies $(OBJS:.o=.c) $(OBJS:.o=.cxx) >/dev/null 2>&1


#
# htmldoc
#

htmldoc$(EXEEXT):	$(OBJS) ../Makedefs
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) $(OBJS) $(LIBS)
	$(POST) htmldoc$(EXEEXT)

$(OBJS):	../Makedefs
include Dependencies


#
# End of "$Id$".
#
