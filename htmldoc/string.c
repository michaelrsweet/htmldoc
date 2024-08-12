//
// String functions for HTMLDOC.
//
// Copyright © 2011-2024 by Michael R Sweet.
// Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
//
// This program is free software.  Distribution and use rights are outlined in
// the file "COPYING".
//

#include "hdstring.h"


//
// Local globals...
//

size_t	num_strings = 0,		// Number of active strings
	alloc_strings = 0;		// Number of allocated pointers
char	**strings = NULL;		// String pool


//
// Local functions...
//

static size_t	find_string(const char *s, int *rdiff);


//
// 'hd_strcasecmp()' - Do a case-insensitive comparison.
//

#ifndef HAVE_STRCASECMP
int					// O - Result of comparison (-1, 0, or 1)
hd_strcasecmp(const char *s,		// I - First string
              const char *t)		// I - Second string
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
#endif // !HAVE_STRCASECMP


//
// 'hd_strncasecmp()' - Do a case-insensitive comparison on up to N chars.
//

#ifndef HAVE_STRNCASECMP
int					// O - Result of comparison (-1, 0, or 1)
hd_strncasecmp(const char *s,		// I - First string
               const char *t,		// I - Second string
	       size_t     n)		// I - Maximum number of characters to compare
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
#endif // !HAVE_STRNCASECMP


//
// 'hd_strcpy()' - Copy a string allowing for overlapping strings.
//

void
hd_strcpy(char       *dst,		// I - Destination string
          const char *src)		// I - Source string
{
  while (*src)
    *dst++ = *src++;

  *dst = '\0';
}


//
// 'hd_strdup()' - Duplicate a string to the string pool.
//

char *					// O - New string pointer
hd_strdup(const char *s)		// I - String to duplicate
{
  char		*news;			// New string
  size_t	idx;			// Index into strings
  int		diff;			// Different


  // Range check input...
  if (!s)
    return (NULL);

  // See if the string has already been added...
  if (num_strings > 0)
  {
    idx = find_string(s, &diff);
    if (diff == 0)
      return (strings[idx]);
  }
  else
  {
    idx  = 0;
    diff = -1;
  }

  // Not already added, so add it...
#ifdef HAVE_STRDUP
  if ((news = strdup(s)) == NULL)
    return (NULL);

#else
  size_t slen = strlen(s);		// Length of string

  if ((news = malloc(slen + 1)) == NULL)
    return (NULL);

  memcpy(news, s, slen + 1);
#endif // HAVE_STRDUP

  if (num_strings >= alloc_strings)
  {
    // Expand the string array...
    char **temp = (char **)realloc(strings, (alloc_strings + 128) * sizeof(char *));

    if (!temp)
    {
      free(news);
      return (NULL);
    }

    strings       = temp;
    alloc_strings += 128;
  }

  // Insert the string...
  if (diff > 0)
    idx ++;

  if (idx < num_strings)
    memmove(strings + idx + 1, strings + idx, (num_strings - idx) * sizeof(char *));

  strings[idx] = news;
  num_strings ++;

  return (news);
}


//
// 'hd_strfreeall()' - Free all strings duplicated with hd_strdup().
//

void
hd_strfreeall(void)
{
  size_t	i;			// Looping var


  // Free all strings in the string pool...
  for (i = 0; i < num_strings; i ++)
    free(strings[i]);

  // Free the string pool...
  free(strings);

  // Reinitialize the string pool...
  num_strings = alloc_strings = 0;
  strings     = NULL;
}


//
// 'hd_strgetsize()' - Get the size of the string pool...
//

size_t					// O - Bytes allocated
hd_strgetsize(void)
{
  size_t	i,			// Looping var
		bytes;			// Bytes allocated


  // Start with the size of the string pool pointer array...
  bytes = alloc_strings * sizeof(char *);

  // Then add the size of the strings themselves, rounded to the nearest 8 bytes
  for (i = 0; i < num_strings; i ++)
    bytes += ((strlen(strings[i]) + 8) & ~7);

  // Return the total...
  return (bytes);
}


#ifndef HAVE_STRLCAT
//
// 'hd_strlcat()' - Safely concatenate two strings.
//

size_t					// O - Length of string
hd_strlcat(char       *dst,		// O - Destination string
           const char *src,		// I - Source string
	   size_t     size)		// I - Size of destination string buffer
{
  size_t	srclen;			// Length of source string
  size_t	dstlen;			// Length of destination string


  // Figure out how much room is left...
  dstlen = strlen(dst);
  size   -= dstlen + 1;

  if (!size)
    return (dstlen);			// No room, return immediately...

  // Figure out how much room is needed...
  srclen = strlen(src);

  // Copy the appropriate amount...
  if (srclen > size)
    srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif // !HAVE_STRLCAT


#ifndef HAVE_STRLCPY
//
// 'hd_strlcpy()' - Safely copy two strings.
//

size_t					// O - Length of string
hd_strlcpy(char       *dst,		// O - Destination string
           const char *src,		// I - Source string
	   size_t      size)		// I - Size of destination string buffer
{
  size_t	srclen;			// Length of source string


  // Figure out how much room is needed...
  size --;

  srclen = strlen(src);

  // Copy the appropriate amount...
  if (srclen > size)
    srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif // !HAVE_STRLCPY


//
// 'find_string()' - Find an element in the array.
//

static size_t				// O - Index of match
find_string(const char   *s,		// I - String to find
	    int          *rdiff)	// O - Difference of match
{
  size_t	left,			// Left side of search
		right,			// Right side of search
		current;		// Current element
  int		diff;			// Comparison with current element


  // Do a binary search for the string...
  left  = 0;
  right = num_strings - 1;

  do
  {
    current = (left + right) / 2;
    diff    = strcmp(s, strings[current]);

    if (diff == 0)
      break;
    else if (diff < 0)
      right = current;
    else
      left = current;
  }
  while ((right - left) > 1);

  if (diff != 0)
  {
    // Check the last 1 or 2 elements...
    if ((diff = strcmp(s, strings[left])) <= 0)
    {
      current = left;
    }
    else
    {
      diff    = strcmp(s, strings[right]);
      current = right;
    }
  }

  // Return the closest string and the difference...
  *rdiff = diff;

  return (current);
}
