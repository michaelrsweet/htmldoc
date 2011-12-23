/*
 * "$Id$"
 *
 *   String functions for HTMLDOC.
 *
 *   Copyright 2011 by Michael R Sweet.
 *   Copyright 1997-2010 by Easy Software Products.
 *
 *   This program is free software.  Distribution and use rights are outlined in
 *   the file "COPYING.txt".
 *
 * Contents:
 *
 *   hd_strcpy()      - Copy a string allowing for overlapping strings.
 *   hd_strdup()      - Duplicate a string.
 *   hd_strcasecmp()  - Do a case-insensitive comparison.
 *   hd_strncasecmp() - Do a case-insensitive comparison on up to N chars.
 *   hd_strlcat()     - Safely concatenate two strings.
 *   hd_strlcpy()     - Safely copy two strings.
 */

/*
 * Include necessary headers...
 */

#include "hdstring.h"


/*
 * 'hd_strcpy()' - Copy a string allowing for overlapping strings.
 */

void
hd_strcpy(char       *dst,		/* I - Destination string */
          const char *src)		/* I - Source string */
{
  while (*src)
    *dst++ = *src++;

  *dst = '\0';
}


/*
 * 'hd_strdup()' - Duplicate a string.
 */

#ifndef HAVE_STRDUP
char *				/* O - New string pointer */
hd_strdup(const char *s)	/* I - String to duplicate */
{
  char	*t;			/* New string pointer */


  if (s == NULL)
    return (NULL);

  if ((t = malloc(strlen(s) + 1)) == NULL)
    return (NULL);

  return (strcpy(t, s));
}
#endif /* !HAVE_STRDUP */


/*
 * 'hd_strcasecmp()' - Do a case-insensitive comparison.
 */

#ifndef HAVE_STRCASECMP
int				/* O - Result of comparison (-1, 0, or 1) */
hd_strcasecmp(const char *s,	/* I - First string */
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

/*
 * 'hd_strncasecmp()' - Do a case-insensitive comparison on up to N chars.
 */

#ifndef HAVE_STRNCASECMP
int				/* O - Result of comparison (-1, 0, or 1) */
hd_strncasecmp(const char *s,	/* I - First string */
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


#ifndef HAVE_STRLCAT
/*
 * 'hd_strlcat()' - Safely concatenate two strings.
 */

size_t				/* O - Length of string */
hd_strlcat(char       *dst,	/* O - Destination string */
           const char *src,	/* I - Source string */
	   size_t     size)	/* I - Size of destination string buffer */
{
  size_t	srclen;		/* Length of source string */
  size_t	dstlen;		/* Length of destination string */


 /*
  * Figure out how much room is left...
  */

  dstlen = strlen(dst);
  size   -= dstlen + 1;

  if (!size)
    return (dstlen);		/* No room, return immediately... */

 /*
  * Figure out how much room is needed...
  */

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif /* !HAVE_STRLCAT */


#ifndef HAVE_STRLCPY
/*
 * 'hd_strlcpy()' - Safely copy two strings.
 */

size_t				/* O - Length of string */
hd_strlcpy(char       *dst,	/* O - Destination string */
           const char *src,	/* I - Source string */
	   size_t      size)	/* I - Size of destination string buffer */
{
  size_t	srclen;		/* Length of source string */


 /*
  * Figure out how much room is needed...
  */

  size --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif /* !HAVE_STRLCPY */


/*
 * End of "$Id$".
 */
