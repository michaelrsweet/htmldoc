/*
 * "$Id: string.c,v 1.9 2004/03/31 08:39:12 mike Exp $"
 *
 *   String functions for HTMLDOC.
 *
 *   Copyright 1997-2004 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "LICENSE.txt" which should have been included with this file.  If this
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
 *
 * Contents:
 *
 *   hd_strcasecmp()  - Do a case-insensitive comparison.
 *   hd_strcpy()      - Copy a string allowing for overlapping strings.
 *   hd_strdup()      - Duplicate a string.
 *   hd_strdupf()     - Format and duplicate a string.
 *   hd_strncasecmp() - Do a case-insensitive comparison on up to N chars.
 *   hd_strlcat()     - Safely concatenate two strings.
 *   hd_strlcpy()     - Safely copy two strings.
 */

/*
 * Include necessary headers...
 */

#include "hdstring.h"


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
 * 'hd_strdupf()' - Format and duplicate a string.
 */

char *					/* O - New string pointer */
hd_strdupf(const char *format,		/* I - Printf-style format string */
           ...)				/* I - Additional arguments */
{
  va_list	ap;			/* Argument pointer */
  int		bytes;			/* Number of bytes required */
  char		*buffer,		/* String buffer */
		temp[256];		/* Small buffer for first vsnprintf */


 /*
  * First format with a tiny buffer; this will tell us how many bytes are
  * needed...
  */

  va_start(ap, format);
  bytes = vsnprintf(temp, sizeof(temp), format, ap);
  va_end(ap);

  if (bytes < sizeof(temp))
  {
   /*
    * Hey, the formatted string fits in the tiny buffer, so just dup that...
    */

    return (strdup(temp));
  }

 /*
  * Allocate memory for the whole thing and reformat to the new, larger
  * buffer...
  */

  if ((buffer = calloc(1, (size_t)(bytes + 1))) != NULL)
  {
    va_start(ap, format);
    vsnprintf(buffer, (size_t)(bytes + 1), format, ap);
    va_end(ap);
  }

 /*
  * Return the new string...
  */

  return (buffer);
}


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
 * End of "$Id: string.c,v 1.9 2004/03/31 08:39:12 mike Exp $".
 */
