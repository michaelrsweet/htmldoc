//
// "$Id: margin.h,v 1.7 2003/01/02 04:36:07 mike Exp $"
//
// Margin class definitions for HTMLDOC, a HTML document processing
// program.
//
// Copyright 1997-2002 by Easy Software Products.
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

#ifndef HTMLDOC_MARGIN_H
#  define HTMLDOC_MARGIN_H


//
// Margin stack size...
//

#  define HD_MARGIN_MAX	10


/**
 * The <TT>hdMargin</TT> class tracks changes to the margins for a document
 * in a stack. This is used to manage margins around floating elements such
 * as images and tables.
 */
class hdMargin
{
  private:
    //* Left margin stack
    float	_left[HD_MARGIN_MAX];
    //* Right margin stack
    float	_right[HD_MARGIN_MAX];
    //* Bottom margin stack
    float	_bottom[HD_MARGIN_MAX];
    //* Top margin
    float	_top;
    //* End page stack
    int		_page[HD_MARGIN_MAX];
    //* Stack level
    int		_level;

  public:

   /**
    * The constructor creates a new margin stack object.
    *
    * @param l float Initial left margin in points.
    * @param b float Initial bottom margin in points.
    * @param r float Initial right margin in points.
    * @param t float Top margin in points.
    */
    hdMargin(float l, float r, float b, float t = 0.0);

   /**
    * The <TT>left()</TT> method returns the current left margin.
    *
    * @return The left margin in points.
    */
    float	left() { return (_left[_level]); }

   /**
    * The <TT>right()</TT> method returns the current right margin.
    *
    * @return The right margin in points.
    */
    float	right() { return (_right[_level]); }

   /**
    * The <TT>bottom()</TT> method returns the current bottom margin.
    *
    * @return The bottom margin in points.
    */
    float	bottom() { return (_bottom[_level]); }

   /**
    * The <TT>bottom0()</TT> method returns the initial bottom margin.
    *
    * @return The bottom margin in points.
    */
    float	bottom0() { return (_bottom[0]); }

   /**
    * The <TT>page()</TT> method returns the ending page for the current margins.
    *
    * @return The ending page number.
    */
    int		page() { return (_page[_level]); }

   /**
    * The <TT>top()</TT> method returns the top margin.
    *
    * @return The top margin in points.
    */
    float	top() { return (_top); }

   /**
    * The <TT>width()</TT> method returns the current width.
    *
    * @return The width in points.
    */
    float	width() { return (_right[_level] - _left[_level]); }

   /**
    * The <TT>length()</TT> method returns the current length.
    *
    * @return The length in points.
    */
    float	length() { return (_bottom[0] - _top); }

   /**
    * The <TT>push()</TT> method pushes a new set of margins on the stack.
    *
    * @param l float The new left margin in points.
    * @param r float The new right margin in points.
    * @param b float The new bottom margin in points.
    * @param p int The new ending page.
    */
    void	push(float l, float r, float b, int p = 0);

   /**
    * The <TT>pop()</TT> method pops the current margins off the stack. If the
    * current margins are at the top of the stack, nothing is popped.
    */
    void	pop() { if (_level) _level --; }

   /**
    * The <TT>clear()</TT> method pops all margins off the stack, leaving the
    * initial margins only.
    */
    void	clear() { _level = 0; }

   /**
    * The <TT>clear()</TT> method pops all margins earlier than the specified
    * vertical position and page.
    *
    * @param y float The vertical position on the page in points.
    * @param p int The page number.
    */
    void	clear(float y, int p);

   /**
    * The <TT>level()</TT> method returns the current stack depth.
    *
    * @return The stack depth.
    */
    int		level() { return (_level); }
};

#endif // !HTMLDOC_MARGIN_H


//
// End of "$Id: margin.h,v 1.7 2003/01/02 04:36:07 mike Exp $".
//
