/*
 * "$Id$"
 *
 * Xcode configuration file for HTMLDOC.
 *
 * Copyright 1997-2009 by Easy Software Products.
 *
 * These coded instructions, statements, and computer programs are the
 * property of Easy Software Products and are protected by Federal
 * copyright law.  Distribution and use rights are outlined in the file
 * "COPYING.txt" which should have been included with this file.  If this
 * file is missing or damaged please contact Easy Software Products
 * at:
 *
 *     Attn: HTMLDOC Licensing Information
 *     Easy Software Products
 *     516 Rio Grand Ct
 *     Morgan Hill, CA 95037 USA
 *
 *     http://www.htmldoc.org/
 */

/*
 * What is the version number for this software?
 */

#define SVERSION	"1.9svn"
#define COPYRIGHT	"Copyright 1997-2009 Easy Software Products. All rights reserved."


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

#define HTMLDOC_DOCDIR "/usr/share/doc/htmldoc"
#define HTMLDOC_DATA "/usr/share/htmldoc"


/*
 * Do we have OpenSSL?
 */

#define HAVE_SSL 1


/*
 * Do we need to use <strings.h>?
 */

#define HAVE_STRINGS_H 1


/*
 * Do we have the <locale.h> header file?
 */

#define HAVE_LOCALE_H 1


/*
 * Do we have some of the "standard" string functions?
 */

#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
/* #undef HAVE_STRDUPF */
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
#define HAVE_STRNCASECMP 1


/*
 * How about snprintf() and vsnprintf()?
 */

#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1


/*
 * Does the "tm" structure contain the "tm_gmtoff" member?
 */

#define HAVE_TM_GMTOFF 1


/*
 * Do we have hstrerror()?
 */

#define HAVE_HSTRERROR 1


/*
 * Do we have getaddrinfo()?
 */

#define HAVE_GETADDRINFO 1


/*
 * Do we have getnameinfo()?
 */

#define HAVE_GETNAMEINFO 1


/*
 * Do we have the long long type?
 */

#define HAVE_LONG_LONG 1

#ifdef HAVE_LONG_LONG
#  define HTMLDOC_LLFMT		"%lld"
#  define HTMLDOC_LLCAST	(long long)
#else
#  define HTMLDOC_LLFMT		"%ld"
#  define HTMLDOC_LLCAST	(long)
#endif /* HAVE_LONG_LONG */


/*
 * Do we have the strtoll() function?
 */

#define HAVE_STRTOLL 1

#ifndef HAVE_STRTOLL
#  define strtoll(nptr,endptr,base) strtol((nptr), (endptr), (base))
#endif /* !HAVE_STRTOLL */


/*
 * End of "$Id$".
 */

