/*
 * "$Id: config.h,v 1.13 2000/03/06 21:24:00 mike Exp $"
 *
 *   Configuration file for HTMLDOC.
 *
 *   Copyright 1997-2000 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 */

/*
 * Locations of files (overridden by the registry...)
 */

#define DOCUMENTATION	"C:/mike/htmldoc1/doc"
#define HTML_DATA	"C:/mike/htmldoc1"


/*
 * Do we have the FLTK library?
 */

#define HAVE_LIBFLTK


/*
 * Do we have some of the "standard" string functions?
 */

#define HAVE_STRDUP
#define HAVE_STRCASECMP
#define HAVE_STRNCASECMP

/*
 * What is the version number for this software?
 */

#define SVERSION	"1.8.5"


/*
 * Limits for the output "engines"...
 */

#define MAX_CHAPTERS	1000
#define MAX_COLUMNS	100
#define MAX_HEADINGS	10000
#define MAX_IMAGES	1000
#define MAX_LINKS	20000
#define MAX_OBJECTS	(10 * MAX_PAGES)
#define MAX_PAGES	5000
#define MAX_ROWS	1000

/*
 * End of "$Id: config.h,v 1.13 2000/03/06 21:24:00 mike Exp $".
 */

