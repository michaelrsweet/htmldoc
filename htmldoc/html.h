//
// "$Id: html.h,v 1.10 2000/10/16 03:25:07 mike Exp $"
//
//   HTML parsing definitions for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
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

#ifndef _HTML_H_
#  define _HTML_H_

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <stdlib.h>

#  include "file.h"
#  include "string.h"
#  include "iso8859.h"


//
// Define some compatibility macros for Microsoft Windows...
//

#  if defined(WIN32) || defined(__EMX__)
#    define strcasecmp(s,t)	stricmp(s,t)
#    define strncasecmp(s,t,n)	strnicmp(s,t,n)
#  endif // WIN32 || __EMX__


//
// Markup constants...
//

enum HDmarkup
{
  MARKUP_FILE = -2,	// File Delimiter
  MARKUP_ERROR = -1,
  MARKUP_NONE = 0,
  MARKUP_COMMENT,
  MARKUP_DOCTYPE,
  MARKUP_A,
  MARKUP_ACRONYM,
  MARKUP_ADDRESS,
  MARKUP_APPLET,
  MARKUP_AREA,
  MARKUP_B,
  MARKUP_BASE,
  MARKUP_BASEFONT,
  MARKUP_BIG,
  MARKUP_BLINK,
  MARKUP_BLOCKQUOTE,
  MARKUP_BODY,
  MARKUP_BR,
  MARKUP_CAPTION,
  MARKUP_CENTER,
  MARKUP_CITE,
  MARKUP_CODE,
  MARKUP_COL,
  MARKUP_COLGROUP,
  MARKUP_DD,
  MARKUP_DEL,
  MARKUP_DFN,
  MARKUP_DIR,
  MARKUP_DIV,
  MARKUP_DL,
  MARKUP_DT,
  MARKUP_EM,
  MARKUP_EMBED,
  MARKUP_FONT,
  MARKUP_FORM,
  MARKUP_FRAME,
  MARKUP_FRAMESET,
  MARKUP_H1,
  MARKUP_H2,
  MARKUP_H3,
  MARKUP_H4,
  MARKUP_H5,
  MARKUP_H6,
  MARKUP_HEAD,
  MARKUP_HR,
  MARKUP_HTML,
  MARKUP_I,
  MARKUP_IMG,
  MARKUP_INPUT,
  MARKUP_INS,
  MARKUP_ISINDEX,
  MARKUP_KBD,
  MARKUP_LI,
  MARKUP_LINK,
  MARKUP_MAP,
  MARKUP_MENU,
  MARKUP_META,
  MARKUP_MULTICOL,
  MARKUP_NOBR,
  MARKUP_NOFRAMES,
  MARKUP_OL,
  MARKUP_OPTION,
  MARKUP_P,
  MARKUP_PRE,
  MARKUP_S,
  MARKUP_SAMP,
  MARKUP_SCRIPT,
  MARKUP_SELECT,
  MARKUP_SMALL,
  MARKUP_SPACER,
  MARKUP_STRIKE,
  MARKUP_STRONG,
  MARKUP_STYLE,
  MARKUP_SUB,
  MARKUP_SUP,
  MARKUP_TABLE,
  MARKUP_TBODY,
  MARKUP_TD,
  MARKUP_TEXTAREA,
  MARKUP_TFOOT,
  MARKUP_TH,
  MARKUP_THEAD,
  MARKUP_TITLE,
  MARKUP_TR,
  MARKUP_TT,
  MARKUP_U,
  MARKUP_UL,
  MARKUP_VAR,
  MARKUP_WBR
};

//
// Horizontal alignment...
//

enum
{
  ALIGN_LEFT = 0,
  ALIGN_CENTER,
  ALIGN_RIGHT
};

//
// Vertical alignment...
//

enum
{
  ALIGN_TOP = 0,
  ALIGN_MIDDLE,
  ALIGN_BOTTOM
};

//
// Typeface...
//

enum HDtypeface
{
  TYPE_COURIER = 0,
  TYPE_TIMES,
  TYPE_HELVETICA,
  TYPE_SYMBOL
};

//
// Style...
//

enum HDstyle
{
  STYLE_NORMAL = 0,
  STYLE_BOLD,
  STYLE_ITALIC,
  STYLE_BOLD_ITALIC
};

//
// Sizes...
//

#  define SIZE_H1	6
#  define SIZE_H2	5
#  define SIZE_H3	4
#  define SIZE_H4	3
#  define SIZE_H5	2
#  define SIZE_H6	1
#  define SIZE_H7	0
#  define SIZE_P	3
#  define SIZE_PRE	2
#  define SIZE_SUB	1
#  define SIZE_SUP	1


//
// Markup variables...
//

struct HDvar
{
  uchar			*name,		// Variable name
			*value;		// Variable value
};

//
// Parsing tree...
//

class HDtree
{
  private:

  static int	compare_markups(uchar **m0, uchar **m1);
  static int	compare_variables(HDvar *v0, HDvar *v1);
  static char	*fix_filename(char *path, const char *base);


  int	set_color(uchar *c);
  int	get_size();
  void	get_text(uchar *buf, int buflen);
  int	get_alignment();
  int	parse_markup(FILE *fp);
  int	parse_variable(FILE *fp);

  public:

  HDtree	*parent,	// Parent tree entry
		*child,		// First child entry
		*last_child,	// Last child entry
		*prev,		// Previous entry on this level
		*next,		// Next entry on this level
		*link;		// Linked-to
  HDmarkup	markup;		// Markup code
  uchar		*data;		// Text (MARKUP_NONE or MARKUP_COMMENT)
  unsigned	halignment:2,	// Horizontal alignment
		valignment:2,	// Vertical alignment
		typeface:2,	// Typeface code
		style:2,	// Style of text
		size:3,		// Size of text
		underline:1,	// Text is underlined?
		strikethrough:1,// Text is struck-through?
		subscript:1,	// Text is subscripted?
		superscript:1,	// Text is superscripted?
		preformatted:1,	// Preformatted text?
		indent:4,	// Indentation level 0-15
		is_copy:1;	// Is this a copy?
  uchar		red,		// Color of this fragment
		green,
		blue;
  float		width,		// Width of this fragment in points
		height;		// Height of this fragment in points
  int		nvars;		// Number of variables...
  HDvar		*vars;		// Variables...

  HDtree(HDtree *p = (HDtree *)0) { init(parent); }
  HDtree(HDtree *p, HDmarkup m, uchar *d, int ins = 0);
  ~HDtree();

  void		add(HDtree *p);
  void		copy(HDtree *p);
  HDtree	*dup();
  HDtree	*flatten(float padding = 0.0f);
  uchar		*get_text();
  uchar		*get_meta(uchar *name);
  void		init(HDtree *p);
  void		insert(HDtree *p);
  void		read(FILE *fp, const char *base);
  HDtree	*real_prev();
  HDtree	*real_next();

  uchar		*var(uchar *name);
  int		var(uchar *name, uchar *value);

  static void	set_base_size(float p, float s);
  static void	set_char_set(const char *cs);
  static void	set_text_color(uchar *color);

  static const char	*markups[];
  static const char	*datadir;
  static float		ppi;
  static int		grayscale;
  static int		initialized;
  static uchar		body_color[];
  static uchar		text_color[];
  static float		browser_width;
  static float		sizes[],
			spacings[];
  static HDtypeface	body_font,
			heading_font;
  static char		char_set[];
  static float		widths[4][4][256];
  static const char	*glyphs_all[];
  static const char	*glyphs[];
  static const char	*fonts[4][4];
};

#endif // !_HTML_H_

//
// End of "$Id: html.h,v 1.10 2000/10/16 03:25:07 mike Exp $".
//
