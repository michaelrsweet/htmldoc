//
// "$Id: style.cxx,v 1.1 2002/01/20 15:10:14 mike Exp $"
//
//   CSS routines for HTMLDOC, a HTML document processing program.
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
#include <ctype.h>


//
// 'hdStyle::hdStyle()' - Create a new style record.
//

hdStyle::hdStyle(hdStyle    *p,
                 int        nsels,
		 hdSelector *sels)
{
}


//
// 'hdStyle::~hdStyle()' - Destroy a style record.
//

hdStyle::~hdStyle()
{
}


//
// 'hdStyle::load()' - Load a style definition from a string.
//

int					// O - 0 on success, -1 on failure
hdStyle::load(hdStyleSheet *css,	// I - Stylesheet
              const char   *s)		// I - Style data
{
}


//
// 'hdStyle::update()' - Update relative style definitions.
//

void
hdStyle::update(hdStyleSheet *css)	// I - Stylesheet
{
}


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
}


//
// 'hdStyleSheet::hdStyleSheet()' - Create a new stylesheet.
//

hdStyleSheet::hdStyleSheet()
{
}


//
// 'hdStyleSheet::~hdStyleSheet()' - Destroy a stylesheet.
//

hdStyleSheet::~hdStyleSheet()
{
}


//
// 'hdStyleSheet::add_style()' - Add a style to a stylesheet.
//

void
hdStyleSheet::add_style(hdStyle *s)	// I - New style
{
}


//
// 'hdStyleSheet::find_font()' - Find a font for the given style.
//

hdStyleFont *				// O - Font record
hdStyleSheet::find_font(hdStyle *s)	// I - Style record
{
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                tree node.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdTree *t)	// I - Tree node
{
}


//
// 'hdStyleSheet::load()' - Load a stylesheet from the given file.
//

int					// O - 0 on success, -1 on failure
hdStyleSheet::load(hdFile     *f,	// I - File to read from
                   const char *path)	// I - Search path for included files
{
}


//
// 'hdStyleSheet::set_charset()' - Set the character set to use for the
//                                 document.
//

void
hdStyleSheet::set_charset(const char *cs)// I - Character set name
{
}


//
// 'hdStyleSheet::update()' - Update all relative stylesheet data.
//

void
hdStyleSheet::update()
{
}


//
// 'hdStyleSheet::update_styles()' - Update all relative style data.
//

void
hdStyleSheet::update_styles()
{
}


//
// End of "$Id: style.cxx,v 1.1 2002/01/20 15:10:14 mike Exp $".
//
