//
// "$Id$"
//
//   Common file routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdFile::~hdFile() - Close a file...
//   hdFile::getline() - Read a logical line from a file.
//   hdFile::gets()    - Read a line from a file.
//   hdFile::printf()  - Write formatted text.
//   hdFile::putline() - Write a logical line to a file.
//   hdFile::puts()    - Write a string to a file.
//

//
// Include necessary headers.
//

#include "file.h"
#include "hdstring.h"
#include <ctype.h>


//
// 'hdFile::~hdFile()' - Close a file...
//

hdFile::~hdFile()
{
}


//
// 'hdFile::getline()' - Read a logical line from a file.
//

char *					// O - String buffer or NULL on EOF
hdFile::getline(char   *s,		// O - String buffer
                size_t slen)		// I - Size of string buffer
{
  int		ch;			// Character from file
  char		*ptr,			// Current position in line sfer
		*end;			// End of line sfer


 /*
  * Now loop until we have a valid line...
  */

  ptr = s;
  end = s + slen - 1;

  for (;;)
  {
    if ((ch = get()) == EOF)
      break;
    else if (ch == '\n')
      break;
    else if (ch == '\\')
    {
     /*
      * Handle \ escapes, to continue to multiple lines...
      */

      int nextch = get();

      if (nextch == EOF)
        break;
      else if (nextch != '\n' && ptr < end)
        *ptr++ = nextch;
    }
    else if (ptr < end)
      *ptr++ = ch;
  }

  *ptr = '\0';

  if (ch != EOF)
    return (s);
  else
    return (NULL);
}


//
// 'hdFile::gets()' - Read a line from a file.
//

char *					// O - String buffer or NULL on EOF
hdFile::gets(char   *s,			// O - String buffer
             size_t slen)		// I - Size of string buffer
{
  int		ch;			// Character from file
  char		*ptr,			// Current position in line sfer
		*end;			// End of line sfer


 /*
  * Now loop until we have a valid line...
  */

  ptr = s;
  end = s + slen - 1;

  for (;;)
  {
    if ((ch = get()) == EOF)
      break;
    else if (ptr < end)
      *ptr++ = ch;

    if (ch == '\n')
      break;
  }

  *ptr = '\0';

  if (ch != EOF)
    return (s);
  else
    return (NULL);
}


//
// 'hdFile::printf()' - Write formatted text.
//

ssize_t					// O - Number of bytes written or -1 on error
hdFile::printf(const char *f,		// I - Printf-style format string
               ...)			// I - Additional args as needed...
{
  ssize_t	bytes;			// Number of bytes written
  char		sign,			// Sign of format width
		sizec,			// Size character (h, l, L)
		type;			// Format type character
  const char	*fptr;			// Start of format
  int		width,			// Width of field
		prec;			// Number of characters of precision
  char		tf[100],		// Temporary format string for sprintf()
		buffer[256];		// Buffer for formatted numbers
  const char	*s;			// Pointer to string
  int		slen;			// Length of string
  va_list 	ap;			// Pointer to additional arguments


  // Loop through the format string, formatting as needed...
  va_start(ap, f);

  bytes = pos();

  while (*f)
  {
    if (*f == '%')
    {
      // Possibly a printf format; see if another % follows...
      fptr = f;
      f ++;

      if (*f == '%')
      {
        // Yes, insert a single %
        put(*f++);
	continue;
      }

      // Get any leading sign 
      if (strchr(" -+#\'", *f))
        sign = *f++;
      else
        sign = 0;

      // Get the field width...
      width = 0;
      while (isdigit(*f))
        width = width * 10 + *f++ - '0';

      if (*f == '.')
      {
        // Get the field precision...
        f ++;
	prec = 0;

	while (isdigit(*f))
          prec = prec * 10 + *f++ - '0';
      }
      else
        prec = -1;

      // Get the field size...
      if (*f == 'l' && f[1] == 'l')
      {
        sizec = 'L';
	f += 2;
      }
      else if (*f == 'h' || *f == 'l' || *f == 'L')
        sizec = *f++;

      if (!*f)
        break;

      type = *f++;

      switch (type)
      {
	case 'E' : // Floating point fs
	case 'G' :
	case 'e' :
	case 'f' :
	case 'g' :
	    if ((f - fptr + 1) > (int)sizeof(tf) ||
	        (width + 2) > (int)sizeof(buffer))
	      break;

	    memcpy(tf, fptr, f - fptr);
	    tf[f - fptr] = '\0';

	    sprintf(buffer, tf, va_arg(ap, double));

	    if (puts(buffer) < 0)
	    {
	      va_end(ap);
	      return (-1);
	    }
	    break;

        case 'B' : // Integer fs
	case 'X' :
	case 'b' :
        case 'd' :
	case 'i' :
	case 'o' :
	case 'u' :
	case 'x' :
	    if ((f - fptr + 1) > (int)sizeof(tf) ||
	        (width + 2) > (int)sizeof(buffer))
	      break;

	    memcpy(tf, fptr, f - fptr);
	    tf[f - fptr] = '\0';

	    sprintf(buffer, tf, va_arg(ap, int));

            puts(buffer);
	    break;

        case 'c' : // Character or character array
	    if (width <= 1)
	      put(va_arg(ap, int));
	    else
	    {
	      for (s = va_arg(ap, char *); width > 0; width --)
	        put(*s++);
	    }
	    break;

	case 's' : // String
	    if ((s = va_arg(ap, char *)) == NULL)
	      s = "(null)";

	    slen = strlen(s);
	    if (slen > width && prec != width)
	      width = slen;

            if (slen > width)
	      slen = width;

	    if (sign != '-')
	    {
	      for (; width > slen; width --)
	        put(' ');
	    }

	    write(s, slen);
	    width -= slen;

	    for (; width > 0; width --)
	      put(' ');
	    break;
      }
    }
    else
      put(*f++);
  }

  va_end(ap);

  // Return the number of bytes that were written...
  return (pos() - bytes);
}


//
// 'hdFile::putline()' - Write a logical line to a file.
//

ssize_t					// O - Number of bytes written
hdFile::putline(const char *s)		// I - Line to write
{
  ssize_t	bytes,			// Bytes written
		total;			// Total bytes written
  const char	*start,			// Start of unescaped line
		*current;		// Current position in line


  for (total = 0, start = s, current = s; *current; current ++)
  {
    if (*current == '\\' || *current == '\n')
    {
      if (current > start)
      {
        if ((bytes = write(start, current - start)) < 0)
	  return (-1);

	total += bytes;
      }

      if (put('\\') < 0)
        return (-1);

      total ++;
      start = current;
    }
  }

  if (current > start)
  {
    if ((bytes = write(start, current - start)) < 0)
      return (-1);

    total += bytes;
  }

  if (put('\n') < 0)
    return (-1);
  else
    return (total + 1);
}


//
// 'hdFile::puts()' - Write a string to a file.
//

ssize_t					// O - Number of bytes written
hdFile::puts(const char *s)		// I - String to write
{
  return (write(s, strlen(s)));
}


//
// End of "$Id$".
//
