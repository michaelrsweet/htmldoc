#
# "$Id: Makefile,v 1.3.2.1 2001/02/02 15:10:56 mike Exp $"
#
#   Makefile for HTMLDOC GUI library.
#
#   Copyright 1997-2001 by Easy Software Products.
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

include ../Makedefs


#
# Object files...
#

OBJS	=	CheckButton.o FileBrowser.o FileChooser.o \
		FileChooser2.o FileIcon.o FileInput.o HelpDialog.o HelpView.o \
		Progress.o

#
# Make everything...
#

all:	libgui.a

#
# Install everything...
#

install:	libgui.a

#
# Clean out object and library files...
#

clean:
	rm -f $(OBJS) libgui.a

#
# libgui.a
#

libgui.a:	$(OBJS) ../Makedefs
	echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $@

CheckButton.o:	CheckButton.h
FileBrowser.o:	FileBrowser.h FileIcon.h
FileChooser.o:	FileBrowser.h FileChooser.h FileIcon.h FileInput.h
FileChooser2.o:	FileBrowser.h FileChooser.h FileIcon.h FileInput.h
FileIcon.o:	FileIcon.h
FileInput.o:	FileInput.h
HelpDialog.o:	HelpDialog.h HelpView.h
HelpView.o:	HelpView.h
Progress.o:	Progress.h

$(OBJS):	../Makedefs

#
# End of "$Id: Makefile,v 1.3.2.1 2001/02/02 15:10:56 mike Exp $".
#
