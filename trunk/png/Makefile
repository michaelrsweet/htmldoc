#
# "$Id$"
#
#   PNG library makefile for the HTMLDOC software.
#
#   Copyright 2011 by Michael R Sweet.
#   Copyright 1997-2010 by Easy Software Products.
#
#   This program is free software.  Distribution and use rights are outlined in
#   the file "COPYING.txt".
#

include ../Makedefs

#
# Object files...
#

OBJS	=	\
		png.o \
		pngerror.o \
		pngget.o \
		pngmem.o \
		pngpread.o \
		pngread.o \
		pngrio.o \
		pngrtran.o \
		pngrutil.o \
		pngset.o \
		pngtrans.o \
		pngwio.o \
		pngwrite.o \
		pngwtran.o \
		pngwutil.o


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
# Update dependencies...
#

depend:
	makedepend -Y -I.. -fDependencies $(OBJS:.o=.c) >/dev/null 2>&1


#
# libpng.a
#

libpng.a:	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@  $(OBJS)
	$(RANLIB) $@

$(OBJS):	../Makedefs
include Dependencies


#
# End of "$Id$".
#
