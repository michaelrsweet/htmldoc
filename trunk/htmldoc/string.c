/*
 * "$Id: string.c,v 1.1 1999/11/08 17:12:42 mike Exp $"
 *
 *   String functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Easy Software Products.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING.txt".
 *
 * Contents:
 *
 *   strdup()      - Duplicate a string.
 *   strcasecmp()  - Compare two strings, ignoring differences in case.
 *   strncasecmp() - Compare two strings, ignoring differences in case.
 */

/*
 * Include necessary headers.
 */

#include "string.h"
#include <ctype.h>


#ifndef HAVE_STRDUP
/*
 * 'strdup()' - Duplicate a string.
 */

char *			/* O - New string pointer... */
strdup(const char *s)	/* I - String to duplicate... */
{
  char	*t;		/* New string */


  if ((t = calloc(strlen(s) + 1)) != NULL)
    strcpy(t, s);

  return (t);
}
#endif /* !HAVE_STRDUP */


#ifndef HAVE_STRCASECMP
/*
 * 'strcasecmp()' - Compare two strings, ignoring differences in case.
 */

int				/* O - Result of comparison (-1, 0, or 1) */
strcasecmp(const char *s,	/* I - First string */
           const char *t)	/* I - Second string */
{
  while (*s != '\0' && *t != '\0')
  {
    if (tolower(*s) < tolower(*t))
      return (-1);
    else if (tolower(*s) > tolower(*t))
      return (1);

    s ++;
    t ++;
  }

  if (*s == '\0' && *t == '\0')
    return (0);
  else if (*s != '\0')
    return (1);
  else
    return (-1);
}
#endif /* !HAVE_STRCASECMP */


#ifndef HAVE_STRNCASECMP
/*
 * 'strncasecmp()' - Compare two strings, ignoring differences in case.
 */

int				/* O - Result of comparison (-1, 0, or 1) */
strncasecmp(const char *s,	/* I - First string */
            const char *t,	/* I - Second string */
	    size_t     n)	/* I - Maximum number of characters to compare */
{
  while (*s != '\0' && *t != '\0' && n > 0)
  {
    if (tolower(*s) < tolower(*t))
      return (-1);
    else if (tolower(*s) > tolower(*t))
      return (1);

    s ++;
    t ++;
    n --;
  }

  if (n == 0)
    return (0);
  else if (*s == '\0' && *t == '\0')
    return (0);
  else if (*s != '\0')
    return (1);
  else
    return (-1);
}
#endif /* !HAVE_STRNCASECMP */


/*
 * End of "$Id: string.c,v 1.1 1999/11/08 17:12:42 mike Exp $".
 */
