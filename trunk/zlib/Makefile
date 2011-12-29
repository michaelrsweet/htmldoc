#
# "$Id$"
#
#   ZIP library Makefile for the HTMLDOC software.
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
		adler32.o \
		compress.o \
		crc32.o \
		deflate.o \
		gzclose.o \
		gzlib.o \
		gzread.o \
		gzwrite.o \
		infback.o \
		inffast.o \
		inflate.o \
		inftrees.o \
		trees.o \
		uncompr.o \
		zutil.o


#
# Make all targets...
#

all:	libz.a


#
# Clean all targets and object files...
#

clean:
	$(RM) $(OBJS)
	$(RM) libz.a


#
# Update dependencies...
#

depend:
	makedepend -Y -I.. -fDependencies $(OBJS:.o=.c) >/dev/null 2>&1


#
# libz.a
#

libz.a:	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $@

$(OBJS):	../Makedefs
include Dependencies


#
# End of "$Id$".
#
