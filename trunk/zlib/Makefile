#
# "$Id: Makefile,v 1.9 2002/12/04 20:21:50 swdev Exp $"
#
#   ZIP library Makefile for the Common UNIX Printing System (CUPS).
#
#   Makefile copyright 1997-2002 by Easy Software Products, all rights
#   reserved.
#   ZIP library copyright 1995-1998 Jean-loup Gailly.
#   For conditions of distribution and use, see copyright notice in zlib.h 
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "LICENSE.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#       Attn: CUPS Licensing Information
#       Easy Software Products
#       44141 Airport View Drive, Suite 204
#       Hollywood, Maryland 20636-3111 USA
#
#       Voice: (301) 373-9603
#       EMail: cups-info@cups.org
#         WWW: http://www.cups.org
#

include ../Makedefs

#
# Object files...
#

OBJS	=	adler32.o compress.o crc32.o gzio.o uncompr.o deflate.o \
		trees.o zutil.o inflate.o infblock.o inftrees.o infcodes.o \
		infutil.o inffast.o


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
# Update dependencies (without system header dependencies...)
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


#
# Dependencies...
#

include Dependencies


#
# End of "$Id: Makefile,v 1.9 2002/12/04 20:21:50 swdev Exp $".
#
