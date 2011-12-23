/*
 * "$Id$"
 *
 * Visual C++ configuration file for HTMLDOC.
 *
 * Copyright 2011 by Michael R Sweet.
 * Copyright 1997-2010 by Easy Software Products.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING.txt".
 */

/*
 * Beginning with VC2005, Microsoft breaks ISO C and POSIX conformance
 * by deprecating a number of functions in the name of security, even
 * when many of the affected functions are otherwise completely secure.
 * The _CRT_SECURE_NO_DEPRECATE definition ensures that we won't get
 * warnings from their use...
 *
 * Then Microsoft decided that they should ignore this in VC2008 and use
 * yet another define (_CRT_SECURE_NO_WARNINGS) instead.  Bastards.
 */

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS


/*
 * Include necessary headers...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <io.h>


/*
 * Microsoft also renames the POSIX functions to _name, and introduces
 * a broken compatibility layer using the original names.  As a result,
 * random crashes can occur when, for example, strdup() allocates memory
 * from a different heap than used by malloc() and free().
 *
 * To avoid moronic problems like this, we #define the POSIX function
 * names to the corresponding non-standard Microsoft names.
 */

#define access		_access
//#define close		_close
#define fdopen		_fdopen
//#define getcwd		_getcwd
//#define open		_open
//#define read	        _read
#define setmode		_setmode
#define snprintf 	_snprintf
#define strdup		_strdup
#define stricmp		_stricmp
#define strnicmp	_strnicmp
#define timezone	_timezone
#define unlink		_unlink
#define vsnprintf 	_vsnprintf
//#define write		_write


/*
 * What is the version number for this software?
 */

#define SVERSION	"1.9svn"
#define COPYRIGHT	"Copyright 2011 by Michael R Sweet. All rights reserved."


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

#define HTMLDOC_DOCDIR	"C:/Program Files/HTMLDOC/doc"
#define HTMLDOC_DATA	"C:/Program Files/HTMLDOC"


/*
 * Do we have OpenSSL?
 */

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

#define HAVE_STRCASECMP
#define HAVE_STRDUP
#undef HAVE_STRDUPF
#undef HAVE_STRLCAT
#undef HAVE_STRLCPY
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
 * Do we have getaddrinfo()?
 */

#define HAVE_GETADDRINFO


/*
 * Do we have getnameinfo()?
 */

#define HAVE_GETNAMEINFO


/*
 * Do we have the long long type?
 */

#undef HAVE_LONG_LONG

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

#undef HAVE_STRTOLL

#ifndef HAVE_STRTOLL
#  define strtoll(nptr,endptr,base) strtol((nptr), (endptr), (base))
#endif /* !HAVE_STRTOLL */


/*
 * End of "$Id$".
 */

