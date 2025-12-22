/*
 * Xcode configuration file for HTMLDOC.
 *
 * Copyright © 2011-2025 by Michael R Sweet.
 * Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * What is the version number and year for this software?
 */

#define SVERSION "1.9.22"
#define SYEAR "2025"


/*
 * Limits for input processing...
 */

#define MAX_DEPTH	1000	/* Maximum depth of document tree */
#define MAX_INCLUDES	32	/* Maximum levels of included/embedded files */


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
 * Locations of files...
 */

#define DOCUMENTATION "/usr/local/share/doc/htmldoc"
#define HTML_DATA "/usr/local/share/htmldoc"


/*
 * Have the CUPS library?
 */

#define HAVE_LIBCUPS 1


/*
 * Have the FLTK library?
 */

#define HAVE_LIBFLTK 1


/*
 * Have the JPEG library?
 */

#define HAVE_LIBJPEG 1


/*
 * Have the PNG library?
 */

#define HAVE_LIBPNG 1


/*
 * Have the Xpm library?
 */

/* #undef HAVE_LIBXPM */


/*
 * Need to use <strings.h>?
 */

#define HAVE_STRINGS_H 1


/*
 * Have the <locale.h> header file?
 */

#define HAVE_LOCALE_H 1


/*
 * Have some of the "standard" string functions?
 */

#define HAVE_STRDUP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1


/*
 * Does the "tm" structure contain the "tm_gmtoff" member?
 */

#define HAVE_TM_GMTOFF 1
