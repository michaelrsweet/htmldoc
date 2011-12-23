#
# "$Id$"
#
# Makefile for HTMLDOC, an HTML document processing program.
#
# Copyright 2011 by Michael R Sweet.
# Copyright 1997-2010 by Easy Software Products.
#
# This program is free software.  Distribution and use rights are outlined in
# the file "COPYING.txt".
#

#
# Include common definitions...
#

include ../Makedefs


#
# Object files...
#

BASEOBJS =	\
		array.o \
		entity.o \
		file.o \
		file-ascii.o \
		file-common.o \
		file-flate.o \
		file-jpeg.o \
		file-mem.o \
		file-open.o \
		file-rc4.o \
		file-std.o \
		html.o \
		htmllib.o \
		http.o \
		http-md5.o \
		image.o \
		image-bmp.o \
		image-gif.o \
		image-jpeg.o \
		image-png.o \
		image-reduce.o \
		index.o \
		margin.o \
		md5.o \
		progress.o \
		ps-pdf.o \
		rc4.o \
		snprintf.o \
		string.o \
		style.o \
		stylefont.o \
		stylemedia.o \
		stylesheet.o \
		styletree.o \
		toc.o \
		util.o
OBJS	=	\
		$(BASEOBJS) \
		htmldoc.o \
		testhtml.o \
		teststyle.o
SRCS	=	\
		entity.cxx \
		file.cxx \
		file-ascii.cxx \
		file-common.cxx \
		file-flate.cxx \
		file-jpeg.cxx \
		file-mem.cxx \
		file-open.cxx \
		file-rc4.cxx \
		file-std.cxx \
		html.cxx \
		htmldoc.cxx \
		htmllib.cxx \
		htmlsep.cxx \
		http.cxx \
		http-md5.cxx \
		image.cxx \
		image-bmp.cxx \
		image-gif.cxx \
		image-jpeg.cxx \
		image-png.cxx \
		image-reduce.cxx \
		index.cxx \
		margin.cxx \
		md5.cxx \
		progress.cxx \
		ps-pdf.cxx \
		snprintf.c \
		string.c \
		style.cxx \
		stylefont.cxx \
		stylemedia.cxx \
		stylesheet.cxx \
		styletree.cxx \
		testhtml.cxx \
		teststyle.cxx \
		toc.cxx \
		util.cxx


#
# Make everything...
#

all:	htmldoc$(EXEEXT) testhtml$(EXEEXT) teststyle$(EXEEXT)

#
# Install everything...
#

install:	all
	if [ ! -d $(bindir) ]; then\
		$(MKDIR) $(bindir);\
	fi
	cp htmldoc$(EXEEXT) $(bindir)
	chmod ugo+rx $(bindir)/htmldoc$(EXEEXT)


#
# Uninstall everything...
#

uninstall:
	$(RM) $(bindir)/htmldoc$(EXEEXT)


#
# Clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc$(EXEEXT)


#
# Update dependencies...
#

depend:
	makedepend -Y -I.. -f Dependencies $(SRCS)


#
# htmldoc
#

htmldoc$(EXEEXT):	htmldoc.o $(BASEOBJS) ../Makedefs \
			$(PNGDEP) $(JPEGDEP) $(ZLIBDEP)
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o htmldoc$(EXEEXT) htmldoc.o $(BASEOBJS) $(LIBS)


#
# testhtml
#

testhtml$(EXEEXT):	testhtml.o $(BASEOBJS) ../Makedefs \
			$(PNGDEP) $(JPEGDEP) $(ZLIBDEP)
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o testhtml$(EXEEXT) testhtml.o $(BASEOBJS) $(LIBS)



#
# teststyle
#

teststyle$(EXEEXT):	teststyle.o $(BASEOBJS) ../Makedefs \
			$(PNGDEP) $(JPEGDEP) $(ZLIBDEP)
	echo Linking $@...
	$(CXX) $(LDFLAGS) -o teststyle$(EXEEXT) teststyle.o $(BASEOBJS) $(LIBS)


#
# Dependencies...
#

$(OBJS):	../Makedefs ../config.h

include Dependencies


#
# End of "$Id$".
#
