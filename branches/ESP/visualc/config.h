/*
 * "$Id: config.h,v 1.1 1999/11/07 13:37:05 mike Exp $"
 *
 *   Configuration file for HTMLDOC.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 */

/*
 * Do we have various libraries?
 */

#define HAVE_LIBFLTK
#define HAVE_LIBJPEG
#define HAVE_LIBPNG
#define HAVE_LIBZ

/*
 * Do we have some of the "standard" string functions?
 */

#ifndef MAC
#  define HAVE_STRDUP
#  define HAVE_STRCASECMP
#  define HAVE_STRNCASECMP
#endif /* MAC */

/*
 * What is the version number for this software?
 */

#define SVERSION	"1.8b4"


/*
 * Limits for the output "engines"...
 */

#define MAX_CHAPTERS	100
#define MAX_COLUMNS	20
#define MAX_HEADINGS	10000
#define MAX_IMAGES	1000
#define MAX_LINKS	20000
#define MAX_OBJECTS	(10 * MAX_PAGES)
#define MAX_PAGES	5000
#define MAX_ROWS	1000

/*
 * End of "$Id: config.h,v 1.1 1999/11/07 13:37:05 mike Exp $".
 */

