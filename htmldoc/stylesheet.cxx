//
// "$Id: stylesheet.cxx,v 1.2 2002/02/06 20:24:08 mike Exp $"
//
//   CSS sheet routines for HTMLDOC, a HTML document processing program.
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
// 'hdStyleSheet::hdStyleSheet()' - Create a new stylesheet.
//

hdStyleSheet::hdStyleSheet()
{
  // Initialize the stylesheet structure.  Using memset() is safe
  // on structures...

  memset(this, 0, sizeof(hdStyleSheet));

  // Set the default page to "Universal" with 
  width = 595.0f;
  length = 792.0f;
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
  return ((hdStyleFont *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                tree node.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdTree *t)	// I - Tree node
{
  return ((hdStyle *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                selectors.
//

hdStyle *					// O - Style record
hdStyleSheet::find_style(int        nsels,	// I - Number of selectors
                         hdSelector *sels)	// I - Selectors
{
  return ((hdStyle *)0);
}


//
// 'hdStyleSheet::load()' - Load a stylesheet from the given file.
//

int					// O - 0 on success, -1 on failure
hdStyleSheet::load(hdFile     *f,	// I - File to read from
                   const char *path)	// I - Search path for included files
{
  return (0);
}


//
// 'hdStyleSheet::pattern()' - Initialize a regex pattern buffer...
//

void
hdStyleSheet::pattern(const char *r,	// I - Regular expression pattern
                      char       p[256])// O - Character lookup table
{
  int	s;				// Set state
  int	ch,				// Char for range
	end;				// Last char in range


  // The regex pattern string "r" can be any regex character pattern,
  // e.g.:
  //
  //    a-zA-Z      Matches all letters
  //    \-+.0-9      Matches all numbers, +, -, and .
  //    ~ \t\n      Matches anything except whitespace.
  //
  // A leading '~' inverts the logic, e.g. all characters *except*
  // those listed.  If you want to match the dash (-) then it must
  // appear be quoted (\-)...

  // Set the logic mode...
  if (*r == '~')
  {
    // Invert logic
    s = 0;
    r ++;
  }
  else
    s = 1;

  // Initialize the pattern buffer...
  memset(p, !s, 256);

  // Loop through the pattern string, updating the pattern buffer as needed.
  for (; *r; r ++)
  {
    if (*r == '\\')
    {
      // Handle quoted char...
      r ++;

      switch (*r)
      {
        case 'n' :
            ch = '\n';
	    break;

        case 'r' :
            ch = '\r';
	    break;

        case 't' :
            ch = '\t';
	    break;

        default :
            ch = *r;
	    break;
      }
    }
    else
      ch = *r;

    // Set this character...
    p[ch] = s;

    // Look ahead to see if we have a range...
    if (r[1] == '-')
    {
      // Yes, grab end character...
      r += 2;

      if (*r == '\\')
      {
	r ++;

	switch (*r)
	{
          case 'n' :
              end =  '\n';
	      break;

          case 'r' :
              end =  '\r';
	      break;

          case 't' :
              end =  '\t';
	      break;

          default :
              end =  *r;
	      break;
	}
      }
      else if (*r)
	end =  *r;
      else
        end =  255;

      // Loop through all chars until we are done...
      for (ch ++; ch <= end; ch ++)
        p[ch] = s;
    }
  }
}


//
// 'hdStyleSheet::read()' - Read a string from the given file.
//

char *					// O - String or NULL on EOF
hdStyleSheet::read(hdFile     *f,	// I - File to read from
                   const char *p,	// I - Allowed chars pattern buffer
		   char       *s,	// O - String buffer
		   int        slen)	// I - Number of bytes in string buffer
{
  int	ch;
  char	*ptr,
	*end;


  // Setup pointers for the start and end of the buffer...
  ptr = s;
  end = s + slen - 1;

  // Loop until we hit EOF or a character that is not allowed...
  while (ptr < end && (ch = f->get()) != EOF)
    if (p[ch])
      *ptr++ = ch;
    else
    {
      f->unget(ch);
      break;
    }

  // Nul-terminate the string...
  *ptr = '\0';

  // Return the string if it is not empty...
  if (ptr > s)
    return (s);
  else
    return (NULL);
}


//
// 'hdStyleSheet::set_charset()' - Set the document character set.
//

void
hdStyleSheet::set_charset(const char *cs)// I - Character set name
{
}


//
// 'hdStyleSheet::set_margins()' - Set the page margins.
//

void
hdStyleSheet::set_margins(float l,	// I - Left margin in points
                          float b,	// I - Bottom margin in points
			  float r,	// I - Right margin in points
			  float t)	// I - Top margin in points
{
  left   = l;
  bottom = b;
  right  = r;
  top    = t;

  update_printable();
}


//
// 'hdStyleSheet::set_orientation()' - Set the page orientation.
//

void
hdStyleSheet::set_orientation(hdOrientation o)	// I - Orientation
{
  orientation = o;

  update_printable();
}


//
// 'hdStyleSheet::set_size()' - Set the page size.
//

void
hdStyleSheet::set_size(float w,		// I - Width in points
                       float l)		// I - Length in points
{
  width  = w;
  length = l;

  update_printable();
}


//
// 'hdStyleSheet::update()' - Update all relative stylesheet data.
//

void
hdStyleSheet::update()
{
}


//
// 'hdStyleSheet::update_printable()' - Update the printable page area.
//

void
hdStyleSheet::update_printable()
{
  switch (orientation)
  {
    case HD_ORIENTATION_PORTRAIT :
    case HD_ORIENTATION_REVERSE_PORTRAIT :
        print_width  = width - left - right;
	print_length = length - top - bottom;
	break;

    case HD_ORIENTATION_LANDSCAPE :
    case HD_ORIENTATION_REVERSE_LANDSCAPE :
	print_width  = length - left - right;
        print_length = width - top - bottom;
	break;
  }
}


//
// 'hdStyleSheet::update_styles()' - Update all relative style data.
//

void
hdStyleSheet::update_styles()
{
  int		i;		// Looping var
  hdStyle	**style;	// Current style


  // First clear the "updated" state of all styles...
  for (i = num_styles, style = styles; i > 0; i --, style ++)
    (*style)->updated = 0;

  // Then update all the styles...
  for (i = num_styles, style = styles; i > 0; i --, style ++)
    (*style)->update(this);
}


//
// End of "$Id: stylesheet.cxx,v 1.2 2002/02/06 20:24:08 mike Exp $".
//
