//
// "$Id: file-common.cxx,v 1.3 2004/02/03 02:55:28 mike Exp $"
//
//   Common file routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2004 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdFile::gets()   - Read a line from a file.
//   hdFile::printf() - Write formatted text.
//   hdFile::puts()   - Write a line from a file.
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
// 'hdFile::gets()' - Read a line from a file.
//

char *				// O - String buffer or NULL on EOF
hdFile::gets(char *s,		// O - String buffer
             int  slen)		// I - Size of string buffer
{
  int		ch;		/* Character from file */
  char		*ptr,		/* Current position in line sfer */
		*end;		/* End of line sfer */


 /*
  * Range check everything...
  */

  if (s == NULL || slen < 2)
    return (NULL);

 /*
  * Now loop until we have a valid line...
  */

  ptr = s;
  end = s + slen - 1;

  for (;;)
  {
    if ((ch = get()) == EOF)
      break;
    else if (ch == '\r')
    {
     /*
      * See if we have CR or CR LF...
      */

      int nextch = get();

      if (nextch == EOF || nextch == '\n')
        break;

     /*
      * No LF, so save the next char for later...
      */

      unget(nextch);

      break;
    }
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
      else if (nextch == '\r')
      {
        nextch = get();

	if (nextch == EOF)
	  break;
	else if (nextch != '\n')
	  unget(nextch);
      }
      else if (nextch != '\n' && ptr < end)
        *ptr++ = nextch;
    }
    else if (ptr < end)
      *ptr++ = ch;
  }

  *ptr = '\0';

  if (ch != EOF || ptr > s)
    return (s);
  else
    return (NULL);
}


//
// 'hdFile::printf()' - Write formatted text.
//

int				// O - Number of bytes written or -1 on error
hdFile::printf(const char *f,	// I - Printf-style format string
               ...)		// I - Additional args as needed...
{
  int		bytes;		// Number of bytes written
  char		sign,		// Sign of format width
		size,		// Size character (h, l, L)
		type;		// Format type character
  const char	*fptr;	// Start of format
  int		width,		// Width of field
		prec;		// Number of characters of precision
  char		tf[100],	// Temporary format string for sprintf()
		temp[1024],	// Buffer for formatted numbers
		*tempptr;	// Pointer into number buffer
  const char	*s;		// Pointer to string
  int		slen;		// Length of string
  va_list 	ap;		// Pointer to additional arguments


  // Return immediately if format is NULL...
  if (!f)
    return (0);

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
        size = 'L';
	f += 2;
      }
      else if (*f == 'h' || *f == 'l' || *f == 'L')
        size = *f++;

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
	        (width + 2) > (int)sizeof(temp))
	      break;

	    strncpy(tf, fptr, f - fptr);
	    tf[f - fptr] = '\0';

	    sprintf(temp, tf, va_arg(ap, double));

	    // Strip trailing zeros...
            for (tempptr = temp + strlen(temp) - 1; tempptr > temp; tempptr --)
	      if (*tempptr != '0')
	        break;

            if (tempptr > temp && *tempptr == '.')
	      tempptr --;

	    if (write(temp, tempptr - temp) < 0)
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
	        (width + 2) > (int)sizeof(temp))
	      break;

	    strncpy(tf, fptr, f - fptr);
	    tf[f - fptr] = '\0';

	    sprintf(temp, tf, va_arg(ap, int));

            puts(temp);
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
// 'hdFile::puts()' - Write a string to a file.
//

int				// O - Number of bytes written
hdFile::puts(const char *s)	// I - String to write
{
  if (s)
    return (write(s, strlen(s)));
  else
    return (0);
}


//
// End of "$Id: file-common.cxx,v 1.3 2004/02/03 02:55:28 mike Exp $".
//
