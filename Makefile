#
#   "$Id: Makefile,v 1.1 1999/11/08 17:12:39 mike Exp $"
#
#   Makefile for HTMLDOC, an HTML document processing program.
#
#   Copyright 1997-1999 by Michael Sweet.
#
#   HTMLDOC is distributed under the terms of the GNU General Public License
#   which is described in the file "COPYING-2.0".
#

#
# Include common definitions...
#

include Makedefs


#
# Object files...
#

GUI_OBJS	=	CheckButton.o FileBrowser.o FileChooser.o \
			FileChooser2.o FileIcon.o HelpDialog.o HelpView.o gui.o
CMD_OBJS	=	file.o html.o htmldoc.o htmllib.o image.o iso8859.o \
			ps-pdf.o string.o toc.o
OBJS		=	$(GUI_OBJS) $(CMD_OBJS)

#
# make htmldoc...
#

all:	htmldoc

#
# install htmldoc...
#

install:	all
	if [ ! -d $(bindir) ]; then\
		mkdir -p $(bindir);\
	fi
	cp htmldoc $(bindir)
	chmod 555 $(bindir)/htmldoc

	if [ ! -d $(mandir)/man1 ]; then\
		mkdir -p $(mandir)/man1;\
	fi
	cp doc/htmldoc.1 $(mandir)/man1
	chmod 444 $(mandir)/man1/htmldoc.1

	if [ ! -d $(mandir)/cat1 ]; then\
		mkdir -p $(mandir)/cat1;\
	fi
	cp doc/htmldoc.z $(mandir)/cat1
	chmod 444 $(mandir)/cat1/htmldoc.z

#
# clean out object and executable files...
#

clean:
	rm -f $(OBJS) htmldoc

#
# htmldoc
#

htmldoc:	$(OBJS) Makefile
	$(CXX) $(LDFLAGS) -o htmldoc $(OBJS) $(LIBS)

gui.o:		config.h gui.h html.h htmldoc.h image.h iso8859.h types.h htmldoc.xbm Makefile
html.o:		config.h html.h htmldoc.h image.h iso8859.h types.h Makefile
htmldoc.o:	config.h gui.h html.h htmldoc.h image.h iso8859.h types.h Makefile
htmllib.o:	config.h debug.h htmldoc.h html.h image.h iso8859.h types.h widths.h Makefile
image.o:	config.h debug.h image.h types.h Makefile
iso8859.o:	config.h iso8859.h types.h Makefile
ps-pdf.o:	config.h debug.h html.h htmldoc.h image.h iso8859.h types.h widths.h Makefile
toc.o:		config.h html.h htmldoc.h image.h iso8859.h types.h Makefile
string.o:	config.h string.h Makefile
file.o:		config.h file.h string.h Makefile

#
# End of "$Id: Makefile,v 1.1 1999/11/08 17:12:39 mike Exp $".
#
