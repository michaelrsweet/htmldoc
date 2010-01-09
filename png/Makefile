#
# "$Id$"
#
#   PNG library makefile for the HTMLDOC software.
#
#   Copyright 1997-2010 by Easy Software Products.
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "COPYING.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#     Attn: HTMLDOC Licensing Information
#     Easy Software Products
#     516 Rio Grand Ct
#     Morgan Hill, CA 95037 USA
#
#     http://www.htmldoc.org/
#

include ../Makedefs

#
# Object files...
#

OBJS	=	png.o pngset.o pngget.o pngrutil.o pngtrans.o pngwutil.o \
		pngread.o pngrio.o pngwio.o pngwrite.o pngrtran.o \
		pngwtran.o pngmem.o pngerror.o pngpread.o

#
# Make all of the targets...
#

all:	libpng.a

#
# Clean all of the targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) libpng.a

#
# libpng.a
#

libpng.a:	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@  $(OBJS)
	$(RANLIB) $@

$(OBJS):	png.h pngpriv.h pngconf.h ../Makedefs

#
# End of "$Id$".
#
