//
// "$Id: margin.cxx,v 1.4 2004/02/03 02:55:28 mike Exp $"
//
// Margin class routines for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2004 by Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "LICENSE.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     44141 Airport View Drive, Suite 204
//     Hollywood, Maryland 20636-3111 USA
//
//     Voice: (301) 373-9600
//     EMail: info@easysw.com
//       WWW: http://www.easysw.com
//
// Contents:
//
//   hdMargin::hdMargin() - Initialize a margin class.
//   hdMargin::push()     - Push a new set of margins on the stack.
//

#include "margin.h"


//
// 'hdMargin::hdMargin()' - Initialize a margin class.
//

hdMargin::hdMargin(float l,	// I - Initial left margin
        	   float r,	// I - Initial right margin
		   float b,	// I - Initial bottom margin
		   float t)	// I - Top margin
{
  _level     = 0;
  _left[0]   = l;
  _right[0]  = r;
  _bottom[0] = b;
  _top       = t;
}


//
// 'hdMargin::clear()' - Clear any old margins...
//

void
hdMargin::clear(float y,	// I - Current Y position
                int   p)	// I - Current page number
{
  while (level() > 0 && (y >= bottom() || p >= page()))
    pop();
}


//
// 'hdMargin::push()' - Push a new set of margins on the stack.
//

void
hdMargin::push(float l,		// I - New left margin
               float r,		// I - New right margin
	       float b,		// I - New bottom margin
	       int   p)		// I - Page number for margin
{
  if (b > bottom() || p > page() || _level == 0)
  {
    //
    // This new set of margins finishes before the current one;
    // just push the new margins on the stack...
    //

    if (_level >= (HD_MARGIN_MAX - 1))
      return;

    _level ++;
    _left[_level]   = l;
    _right[_level]  = r;
    _bottom[_level] = b;
    _page[_level]   = p;
  }
  else if (b == bottom() && p == page())
  {
    //
    // This new set of margins finishes at the same time as the
    // current one; replace the current one...
    //

    _left[_level]  = l;
    _right[_level] = r;
  }
  else
  {
    //
    // This new set of margins finishes after the current one;
    // push the new margins with the old bottom, and update the old
    // margins as needed...
    //

    if (_level >= (HD_MARGIN_MAX - 1))
      return;

    _left[_level + 1]   = l;
    _right[_level + 1]  = r;
    _bottom[_level + 1] = _bottom[_level];
    _page[_level + 1]   = p;

    if (_left[_level] < l)
      _left[_level] = l;
    else
      _left[_level] = _left[_level - 1];
    if (_right[_level] > r)
      _right[_level] = r;
    else
      _right[_level] = _right[_level - 1];
    _bottom[_level] = b;

    _level ++;
  }
}


//
// End of "$Id: margin.cxx,v 1.4 2004/02/03 02:55:28 mike Exp $".
//
