//
// "$Id: stylefont.cxx,v 1.1 2002/02/05 19:50:34 mike Exp $"
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
// 'hdStyleFont::width()' - Compute the width of a string.
//

float					// O - Unscaled width
hdStyleFont::width(const char *s)	// I - String to measure
{
  return (0.0f);
}


//
// End of "$Id: stylefont.cxx,v 1.1 2002/02/05 19:50:34 mike Exp $".
//
