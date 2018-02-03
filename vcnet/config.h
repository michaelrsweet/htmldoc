/*
 * Configuration file for HTMLDOC.
 *
 * Copyright © 2011-2017 by Michael R Sweet.
 * Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * Include necessary headers...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>
#include <direct.h>


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
typedef long ssize_t;


/*
 * What is the version number for this software?
 */

#define SVERSION	"1.9.2"


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
#  define HAVE_LIBFLTK 1
#endif /* !_CONSOLE */


/*
 * Do we have the Xpm library?
 */

/* #undef HAVE_LIBXPM */


/*
 * Which encryption libraries do we have?
 */

/* #undef HAVE_CDSASSL */
/* #undef HAVE_GNUTLS */
#define HAVE_SSPISSL 1
#define HAVE_SSL 1


/*
 * Do we have the gnutls_transport_set_pull_timeout_function function?
 */

/* #undef HAVE_GNUTLS_TRANSPORT_SET_PULL_TIMEOUT_FUNCTION */


/*
 * Do we have the gnutls_priority_set_direct function?
 */

/* #undef HAVE_GNUTLS_PRIORITY_SET_DIRECT */


/*
 * What Security framework headers do we have?
 */

/* #undef HAVE_AUTHORIZATION_H */
/* #undef HAVE_SECBASEPRIV_H */
/* #undef HAVE_SECCERTIFICATE_H */
/* #undef HAVE_SECIDENTITYSEARCHPRIV_H */
/* #undef HAVE_SECITEM_H */
/* #undef HAVE_SECITEMPRIV_H */
/* #undef HAVE_SECPOLICY_H */
/* #undef HAVE_SECPOLICYPRIV_H */
/* #undef HAVE_SECURETRANSPORTPRIV_H */


/*
 * Do we have the cssmErrorString function?
 */

/* #undef HAVE_CSSMERRORSTRING */


/*
 * Do we have the SecGenerateSelfSignedCertificate function?
 */

/* #undef HAVE_SECGENERATESELFSIGNEDCERTIFICATE */


/*
 * Do we have the SecKeychainOpen function?
 */

/* #undef HAVE_SECKEYCHAINOPEN */


/*
 * Do we have (a working) SSLSetEnabledCiphers function?
 */

/* #undef HAVE_SSLSETENABLEDCIPHERS */


/*
 * Do we need to use <strings.h>?
 */

/* #undef HAVE_STRINGS_H */


/*
 * Do we have the <locale.h> header file?
 */

#define HAVE_LOCALE_H 1


/*
 * Do we have some of the "standard" string functions?
 */

#define HAVE_STRDUP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
/* #undef HAVE_STRLCAT */
/* #undef HAVE_STRLCPY */


/*
 * How about snprintf() and vsnprintf()?
 */

#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1


/*
 * Does the "tm" structure contain the "tm_gmtoff" member?
 */

/* #undef HAVE_TM_GMTOFF */


/*
 * Which random number generator function to use...
 */

/* #undef HAVE_ARC4RANDOM */
/* #undef HAVE_RANDOM */
/* #undef HAVE_LRAND48 */

#ifdef HAVE_ARC4RANDOM
#  define HTMLDOC_RAND() arc4random()
#  define HTMLDOC_SRAND(v)
#elif defined(HAVE_RANDOM)
#  define HTMLDOC_RAND() random()
#  define HTMLDOC_SRAND(v) srandom(v)
#elif defined(HAVE_LRAND48)
#  define HTMLDOC_RAND() lrand48()
#  define HTMLDOC_SRAND(v) srand48(v)
#else
#  define HTMLDOC_RAND() rand()
#  define HTMLDOC_SRAND(v) srand(v)
#endif /* HAVE_ARC4RANDOM */


/*
 * Do we have hstrerror()?
 */

/* #undef HAVE_HSTRERROR */


/*
 * Do we have getaddrinfo()?
 */

#define HAVE_GETADDRINFO 1


/*
 * Do we have getnameinfo()?
 */

#define HAVE_GETNAMEINFO 1


/*
 * Do we have the <resolv.h> header file and/or res_init()?
 */

/* #undef HAVE_RESOLV_H */
/* #undef HAVE_RES_INIT */


/*
 * Do we have poll()?
 */

/* #undef HAVE_POLL */


/*
 * Do we have the long long type?
 */

/* #undef HAVE_LONG_LONG */

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

/* #undef HAVE_STRTOLL */

#ifndef HAVE_STRTOLL
#  define strtoll(nptr,endptr,base) strtol((nptr), (endptr), (base))
#endif /* !HAVE_STRTOLL */
