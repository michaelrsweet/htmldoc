#
# "$Id: Makefile,v 1.3 1999/11/09 22:16:40 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-1999 by Easy Software Products.
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "COPYING.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#       Attn: ESP Licensing Information
#       Easy Software Products
#       44141 Airport View Drive, Suite 204
#       Hollywood, Maryland 20636-3111 USA
#
#       Voice: (301) 373-9600
#       EMail: info@easysw.com
#         WWW: http://www.easysw.com
#

#
# Include common definitions...
#

include Makedefs


#
# Subdirectories...
#

DIRS	=	gui jpeg png zlib htmldoc

#
# Make all targets...
#

all:
	for dir in $(DIRS); do\
		echo Making all in $$dir...;\
		(cd $$dir; make);\
	done

#
# Remove object and target files...
#

clean:
	for dir in $(DIRS); do\
		echo Cleaning in $$dir...;\
		(cd $$dir; $(MAKE) clean);\
	done
#
# Install object and target files...
#

install:
	for dir in $(DIRS); do\
		echo Installing in $$dir...;\
		(cd $$dir; $(MAKE) install);\
	done

#
# End of "$Id: Makefile,v 1.3 1999/11/09 22:16:40 mike Exp $".
#
