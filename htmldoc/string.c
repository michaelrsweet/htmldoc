/*
 * "$Id: string.c,v 1.4.2.4 2003/01/06 22:09:44 mike Exp $"
 *
 *   String functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2003 by Easy Software Products.
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

#include "hdstring.h"
#include <ctype.h>


#ifndef HAVE_STRDUP
/*
 * 'strdup()' - Duplicate a string.
 */

char *			/* O - New string pointer... */
strdup(const char *s)	/* I - String to duplicate... */
{
  char	*t;		/* New string */


  if ((t = calloc(strlen(s) + 1, 1)) != NULL)
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
 * End of "$Id: string.c,v 1.4.2.4 2003/01/06 22:09:44 mike Exp $".
 */
