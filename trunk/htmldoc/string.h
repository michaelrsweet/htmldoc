/*
 * "$Id: string.h,v 1.4 1999/11/19 17:38:10 mike Exp $"
 *
 *   HTML string definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Easy Software Products.
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

#ifndef _HTMLDOC_STRING_H_
#  define _HTMLDOC_STRING_H_

/*
 * Include necessary headers...
 */

#  include "config.h"

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Define some compatibility macros for Microsoft Windows...
 */

#  if defined(WIN32) || defined(__EMX__)
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

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HTMLDOC_STRING_H_ */

/*
 * End of "$Id: string.h,v 1.4 1999/11/19 17:38:10 mike Exp $".
 */
