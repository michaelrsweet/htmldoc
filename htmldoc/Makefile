#
# "$Id: Makefile,v 1.22 2002/01/14 02:55:21 mike Exp $"
#
# Makefile for HTMLDOC, an HTML document processing program.
#
# Copyright 1997-2002 by Easy Software Products.
#
# These coded instructions, statements, and computer programs are the
# property of Easy Software Products and are protected by Federal
# copyright law.  Distribution and use rights are outlined in the file
# "COPYING.txt" which should have been included with this file.  If this
# file is missing or damaged please contact Easy Software Products
# at:
#
#     Attn: ESP Licensing Information
#     Easy Software Products
#     44141 Airport View Drive, Suite 204
#     Hollywood, Maryland 20636-3111 USA
#
#     Voice: (301) 373-9600
#     EMail: info@easysw.com
#       WWW: http://www.easysw.com
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

LIBOBJS	=	file.o file-common.o file-open.o file-std.o \
		http.o http-md5.o md5.o \
		image.o image-bmp.o image-gif.o image-jpeg.o image-png.o \
		margin.o \
		tree.o

OBJS	=	$(LIBOBJS) testsuite.o


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


#
# testsuite
#

testsuite:	testsuite.o libhtmldoc.a ../Makedefs
	echo Linking $@...
	$(RM) $@
	$(CXX) $(LDFLAGS) -o testsuite testsuite.o libhtmldoc.a $(LIBS)


$(OBJS):	../Makedefs


#
# End of "$Id: Makefile,v 1.22 2002/01/14 02:55:21 mike Exp $".
#
