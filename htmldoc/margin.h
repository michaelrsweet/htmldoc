//
// "$Id: margin.h,v 1.1 2001/12/04 22:07:42 mike Exp $"
//
// Margin class definitions for HTMLDOC, a HTML document processing
// program.
//
// Copyright 1997-2001 by Easy Software Products.
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


//
// Margin class...
//

class hdMargin
{
  private:
    float	_left[HD_MARGIN_MAX],	// Left margin
		_right[HD_MARGIN_MAX],	// Right margin
		_bottom[HD_MARGIN_MAX],	// Bottom margin
		_top;			// Top margin
    int		_level;			// Stack level

  public:

    hdMargin(float l, float r, float b, float t = 0.0);

    float	left() { return (_left[_level]); }
    float	right() { return (_right[_level]); }
    float	bottom() { return (_bottom[_level]); }
    float	bottom0() { return (_bottom[0]); }
    float	top() { return (_top); }

    void	push(float l, float r, float b);
    void	pop() { if (_level) _level --; }
    void	clear() { _level = 0; }
    int		level() { return (_level); }
};

#endif // !HTMLDOC_MARGIN_H


//
// End of "$Id: margin.h,v 1.1 2001/12/04 22:07:42 mike Exp $".
//
