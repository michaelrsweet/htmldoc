//
// "$Id: HelpView.h,v 1.3 1999/09/24 20:41:02 mike Exp $"
//
//   Help View definitions for the Common UNIX Printing System (CUPS).
//
//   Copyright 1997-1999 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outblockd in the file
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
#  include <FL/fl_draw.H>


//
// HelpBlock structure...
//

struct HelpBlock
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
  char		filename[192],	// Reference filename
		name[32];	// Link target (blank if none)
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
  uchar		textfont_,		// Default font for text
		textsize_;		// Default font size
  const char	*value_;		// HTML text value

  int		nblocks_,		// Number of blocks/paragraphs
		ablocks_;		// Allocated blocks
  HelpBlock	*blocks_;		// Blocks

  int		nfonts_;		// Number of fonts in stack
  uchar		fonts_[100][2];		// Font stack

  int		nlinks_,		// Number of links
		alinks_;		// Allocated links
  HelpLink	*links_;		// Links

  int		ntargets_,		// Number of targets
		atargets_;		// Allocated targets
  HelpTarget	*targets_;		// Targets

  char		filename_[256];		// Current filename
  int		topline_,		// Top line in document
		size_;			// Total document length
  Fl_Scrollbar	scrollbar_;		// Scrollbar for document

  HelpBlock	*add_block(const char *s, int xx, int yy, int ww, int hh, int a);
  void		add_link(const char *n, int xx, int yy, int ww, int hh);
  void		add_target(const char *n, int yy);
  static int	compare_targets(const HelpTarget *t0, const HelpTarget *t1);
  void		draw();
  void		format();
  const char	*get_attr(const char *p, const char *n, char *buf, int bufsize);
  int		handle(int);

  void		initfont(uchar &f, uchar &s) { nfonts_ = 0;
			fl_font(f = fonts_[0][0] = textfont_,
			        s = fonts_[0][1] = textsize_); }
  void		pushfont(uchar f, uchar s) { nfonts_ ++;
			fl_font(fonts_[nfonts_][0] = f,
			        fonts_[nfonts_][1] = s); }
  void		popfont(uchar &f, uchar &s) { if (nfonts_ > 0) nfonts_ --;
			fl_font(f = fonts_[nfonts_][0],
			        s = fonts_[nfonts_][1]); }

  public:

  HelpView(int xx, int yy, int ww, int hh, const char *l = 0);
  const char	*filename() const { if (filename_[0]) return (filename_);
  					else return ((const char *)0); }
  int		load(const char *f);
  void		resize(int,int,int,int);
  int		size() const { return (size_); }
  void		textfont(uchar f) { textfont_ = f; format(); }
  uchar		textfont() const { return (textfont_); }
  void		textsize(uchar s) { textsize_ = s; format(); }
  uchar		textsize() const { return (textsize_); }
  void		topline(const char *n);
  void		topline(int);
  int		topline() const { return (topline_); }
  void		value(const char *v);
  const char	*value() const { return (value_); }
};

#endif // !_GUI_HELPVIEW_H_

//
// End of "$Id: HelpView.h,v 1.3 1999/09/24 20:41:02 mike Exp $".
//
