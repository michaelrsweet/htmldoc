/*
 * "$Id: config.h,v 1.20.2.11 2001/06/20 18:04:53 mike Exp $"
 *
 *   Configuration file for HTMLDOC.
 *
 *   Copyright 1997-2001 by Easy Software Products.
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

#ifndef _CONSOLE
#  define HAVE_LIBFLTK
#endif /* !_CONSOLE */


/*
 * Do we have the OpenSSL library?
 */

#define HAVE_LIBSSL


/*
 * Do we have the image libraries?
 */

#define HAVE_LIBJPEG
#define HAVE_LIBPNG
#define HAVE_LIBZ


/*
 * Do we have some of the "standard" string functions?
 */

#define HAVE_STRDUP
#define HAVE_STRCASECMP
#define HAVE_STRNCASECMP


/*
 * How about snprintf() and vsnprintf()?
 */

#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF


/*
 * What is the version number for this software?
 */

#define SVERSION	"1.8.14"


/*
 * Limits for the output "engines"...
 */

#define MAX_CHAPTERS	1000
#define MAX_COLUMNS	200


/*
 * Memory allocation units for other stuff...
 */

#define ALLOC_FILES	10	/* Temporary/image files */
#define ALLOC_HEADINGS	50	/* Headings */
#define ALLOC_LINKS	100	/* Web links */
#define ALLOC_OBJECTS	100	/* PDF objects */
#define ALLOC_PAGES	10	/* PS/PDF pages */
#define ALLOC_ROWS	20	/* Table rows */


/*
 * End of "$Id: config.h,v 1.20.2.11 2001/06/20 18:04:53 mike Exp $".
 */

