//
// "$Id: stylefont.cxx,v 1.2 2002/02/17 22:44:55 mike Exp $"
//
//   CSS font routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2002 by Easy Software Products.
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
//

//
// Include necessary headers.
//

#include "tree.h"
#include "hdstring.h"
#include <stdlib.h>


//
// 'hdStyleFont::hdStyleFont()' - Create a new font record.
//

hdStyleFont::hdStyleFont(hdStyleSheet   *css,	// I - Stylesheet
        		 hdFontFace     t,	// I - Typeface
			 hdFontInternal s,	// I - Style
			 const char     *n)	// I - Font name
{
}


//
// 'hdStyleFont::~hdStyleFont()' - Destroy a font record.
//

hdStyleFont::~hdStyleFont()
{
}


//
// 'hdStyleFont::get_kerning()' - Get the kerning list for a string.
//

int						// O - Number of kerning entries
hdStyleFont::get_kerning(const char     *s,	// I - String to kern
                         hdFontKernList **kl)	// O - Kerning list
{
}


//
// 'hdStyleFont::get_width()' - Compute the width of a string.
//

float					// O - Unscaled width
hdStyleFont::get_width(const char *s)	// I - String to measure
{
  return (0.0f);
}


//
// 'hdStyleFont::read_afm()' - Read a Type1 AFM file.
//

int					// O - 0 on success, -1 on error
read_afm(hdFile       *fp,		// I - File to read from
         hdStyleSheet *css)		// I - Stylesheet
{
  char		line[255],		// Line from file
		*lineptr,		// Pointer into line
		value[32],		// String value in line
		value2[32];		// Second string value in line
  int		number;			// Numeric value in line


  while (fp->gets(line, sizeof(line)) != NULL)
  {
    // Get the initial keyword...
    if ((lineptr = strchr(line, ' ')) != NULL)
    {
      // Nul-terminate the keyword, and then skip any remaining whitespace...
      while (isspace(*lineptr))
        *lineptr++ = '\0';
    }

    // 

  }

  return (0);
}


//
// 'hdStyleFont::read_pfm()' - Read a Type1 PFM file.
//

int					// O - 0 on success, -1 on error
read_pfm(hdFile       *fp,		// I - File to read from
         hdStyleSheet *css)		// I - Stylesheet
{
  return (0);
}


//
// 'hdStyleFont::read_ttf()' - Read a TrueType font file.
//

int					// O - 0 on success, -1 on error
read_ttf(hdFile       *fp,		// I - File to read from
         hdStyleSheet *css)		// I - Stylesheet
{
  return (0);
}


//
// End of "$Id: stylefont.cxx,v 1.2 2002/02/17 22:44:55 mike Exp $".
//
