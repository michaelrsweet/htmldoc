//
// "$Id$"
//
//   Margin class routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdMargin::hdMargin() - Initialize a margin class.
//   hdMargin::clear()    - Clear any old margins...
//   hdMargin::end()      - Find the end of the current margins.
//   hdMargin::push()     - Push a new set of margins on the stack.
//

#include "margin.h"


//
// 'hdMargin::hdMargin()' - Initialize a margin class.
//

hdMargin::hdMargin(float l,		// I - Initial left margin
        	   float r,		// I - Initial right margin
		   float b,		// I - Initial bottom margin
		   float t)		// I - Top margin
{
  level_     = 0;
  left_[0]   = l;
  right_[0]  = r;
  bottom_[0] = b;
  top_       = t;
  page_[0]   = 0;
}


//
// 'hdMargin::clear()' - Clear any old margins...
//

void
hdMargin::clear(float y,		// I - Current Y position
                int   p)		// I - Current page number
{
  while (level() > 0 && ((y <= bottom() && p == page()) || p > page()))
    pop();
}


//
// 'hdMargin::end()' - Find the end of the current margins.
//

void
hdMargin::end(float &y,			// IO - Y position
              int   &p)			// IO - Page number
{
  if (level_ > 0)
  {
    // We are only interested in the Y position and page of the second
    // element in the stack...
    if (page_[1])
      p = page_[1];

    y = bottom_[1];

    level_ = 0;;
  }
}


//
// 'hdMargin::push()' - Push a new set of margins on the stack.
//

void
hdMargin::push(float l,			// I - New left margin
               float r,			// I - New right margin
	       float b,			// I - New bottom margin
	       int   p)			// I - Page number for margin
{
  if (b > bottom() || p > page() || level_ == 0)
  {
    //
    // This new set of margins finishes before the current one;
    // just push the new margins on the stack...
    //

    if (level_ >= (HD_MARGIN_MAX - 1))
      return;

    level_ ++;
    left_[level_]   = l;
    right_[level_]  = r;
    bottom_[level_] = b;
    page_[level_]   = p;
  }
  else if (b == bottom() && p == page())
  {
    //
    // This new set of margins finishes at the same time as the
    // current one; replace the current one...
    //

    left_[level_]  = l;
    right_[level_] = r;
  }
  else
  {
    //
    // This new set of margins finishes after the current one;
    // push the new margins with the old bottom, and update the old
    // margins as needed...
    //

    if (level_ >= (HD_MARGIN_MAX - 1))
      return;

    left_[level_ + 1]   = l;
    right_[level_ + 1]  = r;
    bottom_[level_ + 1] = bottom_[level_];
    page_[level_ + 1]   = p;

    if (left_[level_] < l)
      left_[level_] = l;
    else
      left_[level_] = left_[level_ - 1];
    if (right_[level_] > r)
      right_[level_] = r;
    else
      right_[level_] = right_[level_ - 1];
    bottom_[level_] = b;

    level_ ++;
  }
}


//
// End of "$Id$".
//
