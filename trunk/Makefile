#
# "$Id$"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-2008 by Easy Software Products.
#
#   These coded instructions, statements, and computer programs are the
#   property of Easy Software Products and are protected by Federal
#   copyright law.  Distribution and use rights are outlined in the file
#   "COPYING.txt" which should have been included with this file.  If this
#   file is missing or damaged please contact Easy Software Products
#   at:
#
#       Attn: HTMLDOC Licensing Information
#       Easy Software Products
#       516 Rio Grand Ct
#       Morgan Hill, CA 95037 USA
#
#       http://www.htmldoc.org/
#

#
# Include common definitions...
#

include Makedefs


#
# Software packaging...
#

EPM	=	epm -v --output-dir dist


#
# Subdirectories...
#

DIRS	=	$(ZLIBDIR) $(JPEGDIR) $(PNGDIR) htmldoc doc
INSTALLDIRS =	fonts data doc htmldoc


#
# Make all targets...
#

all:	Makedefs config.h htmldoc.list
	for dir in $(DIRS); do\
		echo Making all in $$dir...;\
		(cd $$dir; $(MAKE) -$(MAKEFLAGS)) || break;\
	done


#
# Remove object and target files...
#

clean:
	for dir in $(DIRS); do\
		echo Cleaning in $$dir...;\
		(cd $$dir; $(MAKE) -$(MAKEFLAGS) clean) || break;\
	done


#
# Remove everything that isn't included with a distribution...
#

distclean:	clean
	$(RM) -r dist
	$(RM) *.bak
	$(RM) *.bck
	$(RM) core
	$(RM) core.* 
	$(RM) -r autom4te*.cache
	$(RM) config.h config.log config.status
	$(RM) Makedefs


#
# Install object and target files...
#

install:
	$(MAKE) all
	for dir in $(INSTALLDIRS); do\
		echo Installing in $$dir...;\
		(cd $$dir; $(MAKE) -$(MAKEFLAGS) install) || break;\
	done


#
# Re-run configure script as needed...
#

config.h:	config.h.in configure
	$(MAKE) -$(MAKEFLAGS) reconfigure

Makedefs:	Makedefs.in configure
	$(MAKE) -$(MAKEFLAGS) reconfigure

htmldoc.list:	htmldoc.list.in configure
	$(MAKE) -$(MAKEFLAGS) reconfigure

reconfigure:
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h


#
# Make binary distributions using EPM.
#
# EPM = ESP Package Manager, available at "http://www.epmhome.org/".
#

epm:
	$(RM) -r dist
	$(EPM) htmldoc
	case `uname` in \
		Linux*) $(EPM) -f rpm htmldoc ;; \
		Darwin*) $(EPM) -f osx htmldoc ;; \
		SunOS*) $(RPM) -f pkg htmdoc ;; \
	esac


#
# Scan code using clang (http://clang.llvm.org/StaticAnalysisUsage.html)
#

.PHONY: clang
clang:
	$(RM) -r clang
	cd htmldoc; scan-build -V -k $(MAKE) $(MFLAGS) \
		CC=ccc-analyzer CXX=ccc-analyzer clean all


#
# End of "$Id$".
#
