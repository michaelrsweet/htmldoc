/*
 * Visual Studio configuration file for HTMLDOC.
 *
 * Copyright © 2011-2025 by Michael R Sweet.
 * Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>


/*
 * Microsoft renames the POSIX functions to _name, and introduces
 * a broken compatibility layer using the original names.  As a result,
 * random crashes can occur when, for example, strdup() allocates memory
 * from a different heap than used by malloc() and free().
 *
 * To avoid moronic problems like this, we #define the POSIX function
 * names to the corresponding non-standard Microsoft names.
 */

#define access		_access
#define close		_close
#define fileno		_fileno
#define lseek		_lseek
#define mkdir(d,p)	_mkdir(d)
#define open		_open
#define read	        _read
#define rmdir		_rmdir
#define snprintf 	_snprintf
#define strdup		_strdup
#define unlink		_unlink
#define vsnprintf 	_vsnprintf
#define write		_write


/*
 * Microsoft "safe" functions use a different argument order than POSIX...
 */

#define gmtime_r(t,tm)	gmtime_s(tm,t)
#define localtime_r(t,tm) localtime_s(tm,t)


/*
 * Map the POSIX sleep() and usleep() functions to the Win32 Sleep() function...
 */

typedef unsigned long useconds_t;
#define sleep(X)	Sleep(1000 * (X))
#define usleep(X)	Sleep((X)/1000)


/*
 * Map various parameters to Posix style system calls
 */

#  define F_OK		00
#  define W_OK		02
#  define R_OK		04
#  define O_RDONLY	_O_RDONLY
#  define O_WRONLY	_O_WRONLY
#  define O_CREAT	_O_CREAT
#  define O_TRUNC	_O_TRUNC


/*
 * Compiler stuff...
 */

#undef const
#undef __CHAR_UNSIGNED__
#define __attribute__(x)
#  if defined(_WIN32) && !defined(__CUPS_SSIZE_T_DEFINED)
#    define __CUPS_SSIZE_T_DEFINED
typedef __int64 ssize_t;
#  endif /* _WIN32 && !__CUPS_SSIZE_T_DEFINED */


/*
 * What is the version number and year for this software?
 */

#define SVERSION "1.9.22"
#define SYEAR "2025"


/*
 * Limits for input processing...
 */

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
 * Locations of files (overridden by the registry...)
 */

#define DOCUMENTATION	"C:/Program Files/HTMLDOC/doc"
#define HTML_DATA	"C:/Program Files/HTMLDOC"


/*
 * Have the CUPS library?
 */

#define HAVE_LIBCUPS 1


/*
 * Have the FLTK library?
 */

#ifndef _CONSOLE
#  define HAVE_LIBFLTK 1
#endif /* !_CONSOLE */


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

/* #undef HAVE_STRINGS_H */


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
/* #undef HAVE_STRLCAT */
/* #undef HAVE_STRLCPY */
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1


/*
 * Does the "tm" structure contain the "tm_gmtoff" member?
 */

/* #undef HAVE_TM_GMTOFF */
