#
# "$Id: Makefile,v 1.12 2004/03/31 07:28:13 mike Exp $"
#
#   ZIP library Makefile for the HTMLDOC software.
#
#   Makefile copyright 1997-2004 by Easy Software Products, all rights
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
#       Attn: HTMLDOC Licensing Information
#       Easy Software Products
#       44141 Airport View Drive, Suite 204
#       Hollywood, Maryland 20636-3142 USA
#
#       Voice: (301) 373-9600
#       EMail: info@easysw.com
#         WWW: http://www.easysw.com
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
	$(RM) *.bak
	$(RM) *.bck
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


#
# Update dependencies...
#

depend:
	makedepend -Y -I.. -f Dependencies $(OBJS:.o=.c)


#
# Dependencies...
#
	
$(OBJS):	../Makedefs

include Dependencies


#
# End of "$Id: Makefile,v 1.12 2004/03/31 07:28:13 mike Exp $".
#
