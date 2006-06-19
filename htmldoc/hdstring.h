/*
 * "$Id$"
 *
 *   String definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2005 by Easy Software Products.
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

#ifndef _HDSTRING_H_
#  define _HDSTRING_H_

/*
 * Include necessary headers...
 */

#  include <config.h>

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <ctype.h>

#  ifdef HAVE_STRINGS_H
#    include <strings.h>
#  endif /* HAVE_STRINGS_H */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Define some compatibility macros for Microsoft Windows...
 */

#  ifdef WIN32
#    define strcasecmp(s,t)	stricmp(s,t)
#    define strncasecmp(s,t,n)	strnicmp(s,t,n)
#    define snprintf		_snprintf
#    define vsnprintf		_vsnprintf
#  endif /* WIN32 */

/*
 * Implementation of strcpy() that allows for overlapping buffers.
 */

extern void	hd_strcpy(char *dst, const char *src);


/*
 * Standard string functions that might not be available...
 */

#  ifndef HAVE_STRDUP
extern char	*hd_strdup(const char *);
#    define strdup hd_strdup
#  endif /* !HAVE_STRDUP */

#  ifndef HAVE_STRCASECMP
extern int	hd_strcasecmp(const char *, const char *);
#    define strcasecmp hd_strcasecmp
#  endif /* !HAVE_STRCASECMP */

#  ifndef HAVE_STRNCASECMP
extern int	hd_strncasecmp(const char *, const char *, size_t n);
#    define strncasecmp hd_strncasecmp
#  endif /* !HAVE_STRNCASECMP */

#  ifndef HAVE_STRLCAT
extern size_t hd_strlcat(char *, const char *, size_t);
#    define strlcat hd_strlcat
#  endif /* !HAVE_STRLCAT */

#  ifndef HAVE_STRLCPY
extern size_t hd_strlcpy(char *, const char *, size_t);
#    define strlcpy hd_strlcpy
#  endif /* !HAVE_STRLCPY */

#  ifndef HAVE_SNPRINTF
extern int	hd_snprintf(char *, size_t, const char *, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif /* __GNUC__ */
;
#    define snprintf hd_snprintf
#  endif /* !HAVE_SNPRINTF */

#  ifndef HAVE_VSNPRINTF
extern int	hd_vsnprintf(char *, size_t, const char *, va_list);
#    define vsnprintf hd_vsnprintf
#  endif /* !HAVE_VSNPRINTF */


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HDSTRING_H_ */

/*
 * End of "$Id$".
 */
