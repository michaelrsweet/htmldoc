#
# "$Id: Makefile,v 1.16 2001/12/04 22:07:42 mike Exp $"
#
# Makefile for HTMLDOC, an HTML document processing program.
#
# Copyright 1997-2001 by Easy Software Products.
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

TARGETS	=	libhtmldoc.a


#
# Object files...
#

LIBOBJS	=	image.o image-bmp.o image-gif.o image-jpeg.o image-png.o \
		margin.o

OBJS	=	$(LIBOBJS)


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
# End of "$Id: Makefile,v 1.16 2001/12/04 22:07:42 mike Exp $".
#
