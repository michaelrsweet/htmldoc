/*
 * "$Id: string.h,v 1.1 1999/11/08 17:12:42 mike Exp $"
 *
 *   HTML string definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 */

#ifndef _STRING_H_
#  define _STRING_H_

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

#ifndef HAVE_STRDUP
extern char	*strdup(const char *s);
#endif /* !HAVE_STRDUP */

#ifndef HAVE_STRCASECMP
extern int	strcasecmp(const char *s, const char *t);
#endif /* !HAVE_STRCASECMP */

#ifndef HAVE_STRNCASECMP
extern int	strncasecmp(const char *s, const char *t, size_t n);
#endif /* !HAVE_STRNCASECMP */

#endif /* !_STRING_H_ */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* _STRING_H_ */

/*
 * End of "$Id: string.h,v 1.1 1999/11/08 17:12:42 mike Exp $".
 */
