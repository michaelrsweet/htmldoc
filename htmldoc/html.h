//
// "$Id: html.h,v 1.11 2001/09/27 22:33:22 mike Exp $"
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

#ifndef _HD_HTML_H_
#  define _HD_HTML_H_

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

enum HDelement
{
  HD_MARKUP_FILE = -2,	// File Delimiter
  HD_MARKUP_ERROR = -1,
  HD_MARKUP_NONE = 0,
  HD_MARKUP_COMMENT,
  HD_MARKUP_DOCTYPE,
  HD_MARKUP_A,
  HD_MARKUP_ACRONYM,
  HD_MARKUP_ADDRESS,
  HD_MARKUP_APPLET,
  HD_MARKUP_AREA,
  HD_MARKUP_B,
  HD_MARKUP_BASE,
  HD_MARKUP_BASEFONT,
  HD_MARKUP_BIG,
  HD_MARKUP_BLINK,
  HD_MARKUP_BLOCKQUOTE,
  HD_MARKUP_BODY,
  HD_MARKUP_BR,
  HD_MARKUP_CAPTION,
  HD_MARKUP_CENTER,
  HD_MARKUP_CITE,
  HD_MARKUP_CODE,
  HD_MARKUP_COL,
  HD_MARKUP_COLGROUP,
  HD_MARKUP_DD,
  HD_MARKUP_DEL,
  HD_MARKUP_DFN,
  HD_MARKUP_DIR,
  HD_MARKUP_DIV,
  HD_MARKUP_DL,
  HD_MARKUP_DT,
  HD_MARKUP_EM,
  HD_MARKUP_EMBED,
  HD_MARKUP_FONT,
  HD_MARKUP_FORM,
  HD_MARKUP_FRAME,
  HD_MARKUP_FRAMESET,
  HD_MARKUP_H1,
  HD_MARKUP_H2,
  HD_MARKUP_H3,
  HD_MARKUP_H4,
  HD_MARKUP_H5,
  HD_MARKUP_H6,
  HD_MARKUP_HEAD,
  HD_MARKUP_HR,
  HD_MARKUP_HTML,
  HD_MARKUP_I,
  HD_MARKUP_IMG,
  HD_MARKUP_INPUT,
  HD_MARKUP_INS,
  HD_MARKUP_ISINDEX,
  HD_MARKUP_KBD,
  HD_MARKUP_LI,
  HD_MARKUP_LINK,
  HD_MARKUP_MAP,
  HD_MARKUP_MENU,
  HD_MARKUP_META,
  HD_MARKUP_MULTICOL,
  HD_MARKUP_NOBR,
  HD_MARKUP_NOFRAMES,
  HD_MARKUP_OL,
  HD_MARKUP_OPTION,
  HD_MARKUP_P,
  HD_MARKUP_PRE,
  HD_MARKUP_S,
  HD_MARKUP_SAMP,
  HD_MARKUP_SCRIPT,
  HD_MARKUP_SELECT,
  HD_MARKUP_SMALL,
  HD_MARKUP_SPACER,
  HD_MARKUP_STRIKE,
  HD_MARKUP_STRONG,
  HD_MARKUP_STYLE,
  HD_MARKUP_SUB,
  HD_MARKUP_SUP,
  HD_MARKUP_TABLE,
  HD_MARKUP_TBODY,
  HD_MARKUP_TD,
  HD_MARKUP_TEXTAREA,
  HD_MARKUP_TFOOT,
  HD_MARKUP_TH,
  HD_MARKUP_THEAD,
  HD_MARKUP_TITLE,
  HD_MARKUP_TR,
  HD_MARKUP_TT,
  HD_MARKUP_U,
  HD_MARKUP_UL,
  HD_MARKUP_VAR,
  HD_MARKUP_WBR
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
  HDelement	markup;		// Markup code
  uchar		*data;		// Text (HD_MARKUP_NONE or HD_MARKUP_COMMENT)
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
  HDtree(HDtree *p, HDelement m, uchar *d, int ins = 0);
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

#endif // !_HD_HTML_H_

//
// End of "$Id: html.h,v 1.11 2001/09/27 22:33:22 mike Exp $".
//
