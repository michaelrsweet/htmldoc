#
# "$Id$"
#
# ZIP library Makefile for the HTMLDOC software.
#
# ZIP library copyright 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h 
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

OBJS	=	adler32.o compress.o crc32.o gzio.o uncompr.o deflate.o \
		trees.o zutil.o inflate.o inftrees.o inffast.o

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
# libz.a
#

libz.a:	$(OBJS)
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $@

$(OBJS):	zlib.h zconf.h ../Makedefs
crc32.o:	zutil.h crc32.h
gzio.o:		zutil.h
deflate.o:	deflate.h zutil.h
trees.o:	deflate.h zutil.h trees.h
zutil.o:	zutil.h
inflate.o:	zutil.h inftrees.h inflate.h inffast.h inffixed.h
inftrees.o:	zutil.h inftrees.h
inffast.o:	zutil.h inftrees.h inflate.h inffast.h

#
# End of "$Id$".
#
