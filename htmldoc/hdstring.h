/*
 * "$Id: hdstring.h,v 1.6 2004/02/03 02:55:28 mike Exp $"
 *
 *   String definitions for HTMLDOC, a HTML document processing program.
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
 *       Hollywood, Maryland 20636-3111 USA
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
#  elif defined(__EMX__)
#    define strcasecmp(s,t)	stricmp(s,t)
#    define strncasecmp(s,t,n)	strnicmp(s,t,n)
#  endif /* WIN32 || __EMX__ */


/*
 * Standard string functions that might not be available...
 */

#  ifndef HAVE_STRDUP
extern char	*strdup(const char *s);
#  endif /* !HAVE_STRDUP */

#  ifndef HAVE_STRCASECMP
extern int	strcasecmp(const char *s, const char *t);
#  endif /* !HAVE_STRCASECMP */

#  ifndef HAVE_STRNCASECMP
extern int	strncasecmp(const char *s, const char *t, size_t n);
#  endif /* !HAVE_STRNCASECMP */

#  ifndef HAVE_SNPRINTF
extern int	snprintf(char *, size_t, const char *, ...);
#  endif /* !HAVE_SNPRINTF */

#  ifndef HAVE_VSNPRINTF
extern int	vsnprintf(char *, size_t, const char *, va_list);
#  endif /* !HAVE_VSNPRINTF */


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HDSTRING_H_ */

/*
 * End of "$Id: hdstring.h,v 1.6 2004/02/03 02:55:28 mike Exp $".
 */
