#
# "$Id: Makefile,v 1.28 2002/03/10 03:17:24 mike Exp $"
#
# Makefile for HTMLDOC library.
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

LIBOBJS	=	common.o \
		file.o file-common.o file-mem.o file-open.o file-std.o \
		file-ascii.o file-flate.o file-jpeg.o file-rc4.o \
		http.o http-md5.o md5.o \
		image.o image-bmp.o image-gif.o image-jpeg.o image-png.o \
		image-reduce.o margin.o \
		style.o stylefont.o stylesheet.o tree.o

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
# depend
#

depend:
	makedepend -Y -I.. -f Dependencies $(LIBOBJS:.o=.cxx) testsuite.cxx

include Dependencies


#
# End of "$Id: Makefile,v 1.28 2002/03/10 03:17:24 mike Exp $".
#
