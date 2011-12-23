#
# "$Id$"
#
# PNG library makefile for the HTMLDOC software.
#
# Copyright 2011 by Michael R Sweet.
# Copyright 1997-2010 by Easy Software Products.
#
# This program is free software.  Distribution and use rights are outlined in
# the file "COPYING.txt".
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
