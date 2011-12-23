//
// "$Id$"
//
//   Margin class definitions for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//

#ifndef HTMLDOC_MARGIN_H
#  define HTMLDOC_MARGIN_H


//
// Margin stack size...
//

#  define HD_MARGIN_MAX	128


/**
 * The <TT>hdMargin</TT> class tracks changes to the margins for a document
 * in a stack. This is used to manage margins around floating elements such
 * as images and tables.
 */
class hdMargin
{
  private:
    //* Left margin stack
    float	left_[HD_MARGIN_MAX];
    //* Right margin stack
    float	right_[HD_MARGIN_MAX];
    //* Bottom margin stack
    float	bottom_[HD_MARGIN_MAX];
    //* Top margin
    float	top_;
    //* End page stack
    int		page_[HD_MARGIN_MAX];
    //* Stack level
    int		level_;

  public:

   /**
    * The constructor creates a new margin stack object.
    *
    * @param l float Initial left margin in points.
    * @param b float Initial bottom margin in points.
    * @param r float Initial right margin in points.
    * @param t float Top margin in points.
    */
    hdMargin(float l, float r, float b, float t);

   /**
    * The <TT>left()</TT> method returns the current left margin.
    *
    * @return The left margin in points.
    */
    float	left() { return (left_[level_]); }

   /**
    * The <TT>left0()</TT> method returns the initial left margin.
    *
    * @return The left margin in points.
    */
    float	left0() { return (left_[0]); }

   /**
    * The <TT>right()</TT> method returns the current right margin.
    *
    * @return The right margin in points.
    */
    float	right() { return (right_[level_]); }

   /**
    * The <TT>bottom()</TT> method returns the current bottom margin.
    *
    * @return The bottom margin in points.
    */
    float	bottom() { return (bottom_[level_]); }

   /**
    * The <TT>bottom0()</TT> method returns the initial bottom margin.
    *
    * @return The bottom margin in points.
    */
    float	bottom0() { return (bottom_[0]); }

   /**
    * The <TT>page()</TT> method returns the ending page for the current margins.
    *
    * @return The ending page number.
    */
    int		page() { return (page_[level_]); }

   /**
    * The <TT>top()</TT> method returns the top margin.
    *
    * @return The top margin in points.
    */
    float	top() { return (top_); }

   /**
    * The <TT>width()</TT> method returns the current width.
    *
    * @return The width in points.
    */
    float	width() { return (right_[level_] - left_[level_]); }

   /**
    * The <TT>width0()</TT> method returns the initial width.
    *
    * @return The width in points.
    */
    float	width0() { return (right_[0] - left_[0]); }

   /**
    * The <TT>length()</TT> method returns the current length.
    *
    * @return The length in points.
    */
    float	length() { return (top_ - bottom_[0]); }

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
    void	pop() { if (level_) level_ --; }

   /**
    * The <TT>clear()</TT> method pops all margins off the stack, leaving the
    * initial margins only.
    */
    void	clear() { level_ = 0; }

   /**
    * The <TT>clear()</TT> method pops all margins earlier than the specified
    * vertical position and page.
    *
    * @param y float The vertical position on the page in points.
    * @param p int The page number.
    */
    void	clear(float y, int p);

   /**
    * The <TT>end()</TT> method pops all margins and updates the
    * vertical position and page.
    *
    * @param y float The vertical position on the page in points.
    * @param p int The page number.
    */
    void	end(float &y, int &p);

   /**
    * The <TT>level()</TT> method returns the current stack depth.
    *
    * @return The stack depth.
    */
    int		level() { return (level_); }

   /**
    * The <tt>adjust_bottom()</tt> method adds the specified value
    * to the bottom margins in the stack.
    */
    void	adjust_bottom(float b)
		{ for (int i = 0; i <= level_; i ++) bottom_[i] += b; }

   /**
    * The <tt>adjust_left()</tt> method adds the specified value
    * to the left margins in the stack.
    */
    void	adjust_left(float l)
		{ for (int i = 0; i <= level_; i ++) left_[i] += l; }

   /**
    * The <tt>adjust_right()</tt> method adds the specified value
    * to the right margins in the stack.
    */
    void	adjust_right(float r)
		{ for (int i = 0; i <= level_; i ++) right_[i] += r; }

   /**
    * The <tt>adjust_top()</tt> method adds the specified value
    * to the top margin.
    */
    void	adjust_top(float t)
		{ top_ += t; }
};

#endif // !HTMLDOC_MARGIN_H


//
// End of "$Id$".
//
