#
# "$Id: Makefile,v 1.2 1999/11/08 18:35:14 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-1999 by Easy Software Products.
#
#   HTMLDOC is distributed under the terms of the GNU General Public License
#   which is described in the file "COPYING.txt".
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
# End of "$Id: Makefile,v 1.2 1999/11/08 18:35:14 mike Exp $".
#
