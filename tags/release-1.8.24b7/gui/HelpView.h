//
// "$Id: HelpView.h,v 1.13.2.2 2001/04/18 23:24:11 mike Exp $"
//
//   Help Viewer widget definitions.
//
//   Copyright 1997-2001 by Easy Software Products.
//   Image support donated by Matthias Melcher, Copyright 2000.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outblockd in the file
//   "COPYING" which should have been included with this file.  If this
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

#ifndef _GUI_HELPVIEW_H_
#  define _GUI_HELPVIEW_H_

//
// Include necessary header files...
//

#  include <stdio.h>
#  include <FL/Fl.H>
#  include <FL/Fl_Group.H>
#  include <FL/Fl_Scrollbar.H>
#  include <FL/fl_draw.H>


//
// HelpFunc type - link callback function for files...
//


typedef const char *(HelpFunc)(const char *);


//
// HelpBlock structure...
//

struct HelpBlock
{
  const char	*start,		// Start of text
		*end;		// End of text
  uchar		font,		// Text font
		size,		// Text size
		border;		// Draw border?
  int		x,		// Indentation/starting X coordinate
		y,		// Starting Y coordinate
		w,		// Width
		h;		// Height
  int		line[32];	// Left starting position for each line
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
// HelpImage structure...
//

struct Fl_Pixmap;
struct Fl_Image;

struct HelpImage 
{
  char		*name,		// Path and name of the image
		wattr[8],	// Width attribute
		hattr[8];	// Height attribute
  Fl_Image	*image;		// FLTK image representation
  unsigned char	*data;		// Raw image data
  int		w, h, d;	// Image size & depth
};

//
// HelpView class...
//

class HelpView : public Fl_Group	//// Help viewer widget
{
  enum { RIGHT = -1, CENTER, LEFT };	// Alignments

  char		title_[1024];		// Title string
  Fl_Color	defcolor_,		// Default text color
		bgcolor_,		// Background color
		textcolor_,		// Text color
		linkcolor_;		// Link color
  uchar		textfont_,		// Default font for text
		textsize_;		// Default font size
  const char	*value_;		// HTML text value

  int		nblocks_,		// Number of blocks/paragraphs
		ablocks_;		// Allocated blocks
  HelpBlock	*blocks_;		// Blocks

  int		nfonts_;		// Number of fonts in stack
  uchar		fonts_[100][2];		// Font stack

  HelpFunc	*link_;			// Link transform function

  int		nlinks_,		// Number of links
		alinks_;		// Allocated links
  HelpLink	*links_;		// Links

  int		ntargets_,		// Number of targets
		atargets_;		// Allocated targets
  HelpTarget	*targets_;		// Targets

  char		directory_[1024];	// Directory for current file
  char		filename_[1024];	// Current filename
  int		topline_,		// Top line in document
		size_;			// Total document length
  Fl_Scrollbar	scrollbar_;		// Vertical scrollbar for document

  int		nimage_,		// Number of images in a page
		aimage_;		// Allocated blocks
  HelpImage	*image_;		// list of image descriptors

  HelpImage	*add_image(const char *name, const char *wattr,
		           const char *hattr, int make = 1);
  HelpImage	*find_image(const char *name, const char *wattr,
		           const char *hattr);
  int		load_gif(HelpImage *img, FILE *fp);
  int		load_jpeg(HelpImage *img, FILE *fp);
  int		load_png(HelpImage *img, FILE *fp);

  HelpBlock	*add_block(const char *s, int xx, int yy, int ww, int hh, uchar border = 0);
  void		add_link(const char *n, int xx, int yy, int ww, int hh);
  void		add_target(const char *n, int yy);
  static int	compare_targets(const HelpTarget *t0, const HelpTarget *t1);
  int		do_align(HelpBlock *block, int line, int xx, int a, int &l);
  void		draw();
  void		format();
  int		get_align(const char *p, int a);
  const char	*get_attr(const char *p, const char *n, char *buf, int bufsize);
  Fl_Color	get_color(const char *n, Fl_Color c);
  int		handle(int);

  void		initfont(uchar &f, uchar &s) { nfonts_ = 0;
			fl_font(f = fonts_[0][0] = textfont_,
			        s = fonts_[0][1] = textsize_); }
  void		pushfont(uchar f, uchar s) { if (nfonts_ < 99) nfonts_ ++;
			fl_font(fonts_[nfonts_][0] = f,
			        fonts_[nfonts_][1] = s); }
  void		popfont(uchar &f, uchar &s) { if (nfonts_ > 0) nfonts_ --;
			fl_font(f = fonts_[nfonts_][0],
			        s = fonts_[nfonts_][1]); }

  public:

  HelpView(int xx, int yy, int ww, int hh, const char *l = 0);
  ~HelpView();
  const char	*directory() const { if (directory_[0]) return (directory_);
  					else return ((const char *)0); }
  const char	*filename() const { if (filename_[0]) return (filename_);
  					else return ((const char *)0); }
  void		link(HelpFunc *fn) { link_ = fn; }
  int		load(const char *f);
  void		resize(int,int,int,int);
  int		size() const { return (size_); }
  void		textcolor(Fl_Color c) { if (textcolor_ == defcolor_) textcolor_ = c; defcolor_ = c; }
  Fl_Color	textcolor() const { return (defcolor_); }
  void		textfont(uchar f) { textfont_ = f; format(); }
  uchar		textfont() const { return (textfont_); }
  void		textsize(uchar s) { textsize_ = s; format(); }
  uchar		textsize() const { return (textsize_); }
  const char	*title() { return (title_); }
  void		topline(const char *n);
  void		topline(int);
  int		topline() const { return (topline_); }
  void		value(const char *v);
  const char	*value() const { return (value_); }
};

#endif // !_GUI_HELPVIEW_H_

//
// End of "$Id: HelpView.h,v 1.13.2.2 2001/04/18 23:24:11 mike Exp $".
//
