#
# "$Id: Makefile,v 1.3.2.1 2001/02/02 15:10:53 mike Exp $"
#
#   Makefile for HTMLDOC AFM files.
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
# Adobe Font Metrics files...
#

AFMS	=	Courier Courier-Bold Courier-BoldOblique \
		Courier-Oblique Helvetica Helvetica-Bold \
		Helvetica-BoldOblique Helvetica-Oblique Symbol \
		Times-Bold Times-BoldItalic Times-Italic \
		Times-Roman


#
# Make everything...
#

all:


#
# Install everything...
#

install:
	if [ ! -d $(datadir)/htmldoc/afm ]; then\
		$(MKDIR) $(datadir)/htmldoc/afm;\
	fi
	$(CP) $(AFMS) $(datadir)/htmldoc/afm
	$(CHMOD) ugo+r $(datadir)/htmldoc/afm/*


#
# Clean out object and library files...
#

clean:

#
# End of "$Id: Makefile,v 1.3.2.1 2001/02/02 15:10:53 mike Exp $".
#
