/*
 * "$Id: config.h,v 1.20.2.41 2004/10/23 17:45:23 mike Exp $"
 *
 *   Configuration file for HTMLDOC.
 *
 *   Copyright 1997-2004 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 */

/*
 * What is the version number for this software?
 */

#define SVERSION	"1.8.24 Open Source"


/*
 * Limits for the output "engines"...
 */

#define MAX_CHAPTERS	1000	/* Maximum number of chapters or files */
#define MAX_COLUMNS	200	/* Maximum number of columns in a table */
#define MAX_HF_IMAGES	10	/* Maximum number of header/footer images */


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
 * Locations of files (overridden by the registry...)
 */

#define DOCUMENTATION	"C:/Program Files/HTMLDOC/doc"
#define HTML_DATA	"C:/Program Files/HTMLDOC"


/*
 * Do we have the FLTK library?
 */

#ifndef _CONSOLE
#  define HAVE_LIBFLTK
#endif /* !_CONSOLE */


/*
 * Do we have the Xpm library?
 */

#undef HAVE_LIBXPM


/*
 * Which encryption libraries do we have?
 */

#undef HAVE_CDSASSL
#undef HAVE_GNUTLS
#define HAVE_LIBSSL
#define HAVE_SSL


/*
 * Do we need to use <strings.h>?
 */

#undef HAVE_STRINGS_H


/*
 * Do we have the <locale.h> header file?
 */

#define HAVE_LOCALE_H


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
 * Does the "tm" structure contain the "tm_gmtoff" member?
 */

#undef HAVE_TM_GMTOFF


/*
 * Do we have hstrerror()?
 */

#undef HAVE_HSTRERROR


/*
 * End of "$Id: config.h,v 1.20.2.41 2004/10/23 17:45:23 mike Exp $".
 */

