//
// "$Id: HelpView.h,v 1.1 1999/09/22 16:51:24 mike Exp $"
//
//   Help View definitions for the Common UNIX Printing System (CUPS).
//
//   Copyright 1997-1999 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "LICENSE.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: CUPS Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9603
//       EMail: cups-info@cups.org
//         WWW: http://www.cups.org
//

#ifndef _GUI_HELPVIEW_H_
#  define _GUI_HELPVIEW_H_

//
// Include necessary header files...
//

#  include <FL/Fl.H>
#  include <FL/Fl_Group.H>
#  include <FL/Fl_Scrollbar.H>


//
// HelpLine structure...
//

struct HelpLine
{
  const char	*start,	// Start of text
		*end;	// End of text
  uchar		font,	// Text font
		size,	// Text size
		align;	// Alignment: -1 = right, 0 = center, 1 = left
  int		x,	// Indentation/starting X coordinate
		y,	// Starting Y coordinate
		w,	// Width
		h;	// Height
};

//
// HelpLink structure...
//

struct HelpLink
{
  char		href[224];	// Reference
  int		x,		// X offset of link text
		y,		// Y offset of link text
		w,		// Width of link text
		h;		// Height of link text
};

//
// HelpTarget structure...
//

struct HelpTarget
{
  char		name[32];	// Target name
  int		y;		// Y offset of target
};

//
// HelpView class...
//

class HelpView : public Fl_Group	//// Help viewer widget
{
  uchar		textfont_,
		textsize_;
  const char	*value_;

  int		nlines_,
		alines_;
  HelpLine	*lines_;

  int		nlinks_,
		alinks_;
  HelpLink	*links_;

  int		ntargets_,
		atargets_;
  HelpTarget	*targets_;

  int		top_,
		size_;
  Fl_Scrollbar	scrollbar_;

  HelpLine	*add_line(const char *s, int x, int y, int w, int h, int a);
  void		add_link(const char *n, int x, int y, int w, int h);
  void		add_target(const char *n, int y);
  void		draw();
  void		format();
  const char	*get_attr(const char *p, const char *n, char *buf, int bufsize);
  int		handle(int);

  public:

  HelpView(int x, int y, int w, int h, const char *l = 0);
  int		load(const char *f);
  void		resize(int,int,int,int);
  int		size() const { return (size_); }
  void		textfont(uchar f) { textfont_ = f; format(); redraw(); }
  uchar		textfont() const { return (textfont_); }
  void		textsize(uchar s) { textsize_ = s; format(); redraw(); }
  uchar		textsize() const { return (textsize_); }
  void		topline(const char *n);
  void		topline(int);
  void		value(const char *v);
  const char	*value() { return (value_); }
};

#endif // !_GUI_HELPVIEW_H_

//
// End of "$Id: HelpView.h,v 1.1 1999/09/22 16:51:24 mike Exp $".
//
