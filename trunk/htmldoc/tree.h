//
// "$Id: tree.h,v 1.1 2002/01/14 02:55:21 mike Exp $"
//
//   HTML tree definitions for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2002 by Easy Software Products.
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

#ifndef _HTMLDOC_TREE_H_
#  define _HTMLDOC_TREE_H_

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <stdlib.h>

#  include "file.h"
#  include "hdstring.h"
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

enum hdElement
{
  HD_ELEMENT_FILE = -3,	// File Delimiter
  HD_ELEMENT_ERROR = -2,
  HD_ELEMENT_UNKNOWN = -1,
  HD_ELEMENT_NONE = 0,
  HD_ELEMENT_COMMENT,
  HD_ELEMENT_DOCTYPE,
  HD_ELEMENT_A,
  HD_ELEMENT_ACRONYM,
  HD_ELEMENT_ADDRESS,
  HD_ELEMENT_APPLET,
  HD_ELEMENT_AREA,
  HD_ELEMENT_B,
  HD_ELEMENT_BASE,
  HD_ELEMENT_BASEFONT,
  HD_ELEMENT_BIG,
  HD_ELEMENT_BLINK,
  HD_ELEMENT_BLOCKQUOTE,
  HD_ELEMENT_BODY,
  HD_ELEMENT_BR,
  HD_ELEMENT_CAPTION,
  HD_ELEMENT_CENTER,
  HD_ELEMENT_CITE,
  HD_ELEMENT_CODE,
  HD_ELEMENT_COL,
  HD_ELEMENT_COLGROUP,
  HD_ELEMENT_DD,
  HD_ELEMENT_DEL,
  HD_ELEMENT_DFN,
  HD_ELEMENT_DIR,
  HD_ELEMENT_DIV,
  HD_ELEMENT_DL,
  HD_ELEMENT_DT,
  HD_ELEMENT_EM,
  HD_ELEMENT_EMBED,
  HD_ELEMENT_FONT,
  HD_ELEMENT_FORM,
  HD_ELEMENT_FRAME,
  HD_ELEMENT_FRAMESET,
  HD_ELEMENT_H1,
  HD_ELEMENT_H2,
  HD_ELEMENT_H3,
  HD_ELEMENT_H4,
  HD_ELEMENT_H5,
  HD_ELEMENT_H6,
  HD_ELEMENT_HEAD,
  HD_ELEMENT_HR,
  HD_ELEMENT_HTML,
  HD_ELEMENT_I,
  HD_ELEMENT_IMG,
  HD_ELEMENT_INPUT,
  HD_ELEMENT_INS,
  HD_ELEMENT_ISINDEX,
  HD_ELEMENT_KBD,
  HD_ELEMENT_LI,
  HD_ELEMENT_LINK,
  HD_ELEMENT_MAP,
  HD_ELEMENT_MENU,
  HD_ELEMENT_META,
  HD_ELEMENT_MULTICOL,
  HD_ELEMENT_NOBR,
  HD_ELEMENT_NOFRAMES,
  HD_ELEMENT_OL,
  HD_ELEMENT_OPTION,
  HD_ELEMENT_P,
  HD_ELEMENT_PRE,
  HD_ELEMENT_S,
  HD_ELEMENT_SAMP,
  HD_ELEMENT_SCRIPT,
  HD_ELEMENT_SELECT,
  HD_ELEMENT_SMALL,
  HD_ELEMENT_SPACER,
  HD_ELEMENT_STRIKE,
  HD_ELEMENT_STRONG,
  HD_ELEMENT_STYLE,
  HD_ELEMENT_SUB,
  HD_ELEMENT_SUP,
  HD_ELEMENT_TABLE,
  HD_ELEMENT_TBODY,
  HD_ELEMENT_TD,
  HD_ELEMENT_TEXTAREA,
  HD_ELEMENT_TFOOT,
  HD_ELEMENT_TH,
  HD_ELEMENT_THEAD,
  HD_ELEMENT_TITLE,
  HD_ELEMENT_TR,
  HD_ELEMENT_TT,
  HD_ELEMENT_U,
  HD_ELEMENT_UL,
  HD_ELEMENT_VAR,
  HD_ELEMENT_WBR
};

//
// Horizontal alignment...
//

enum hdHAlignment
{
  HD_HALIGN_LEFT = 0,
  HD_HALIGN_CENTER,
  HD_HALIGN_RIGHT,
  HD_HALIGN_JUSTIFY
};

//
// Vertical alignment...
//

enum hdVAlignment
{
  HD_VALIGN_TOP = 0,
  HD_VALIGN_MIDDLE,
  HD_VALIGN_BOTTOM
};

//
// Typeface...
//

enum hdFontFace
{
  HD_FONTFACE_COURIER = 0,
  HD_FONTFACE_TIMES,
  HD_FONTFACE_HELVETICA,
  HD_FONTFACE_SYMBOL,
  HD_FONTFACE_CUSTOM,
  HD_FONTFACE_MAX = 15
};

//
// Style...
//

enum hdFontStyle
{
  HD_FONTSTYLE_NORMAL = 0,
  HD_FONTSTYLE_BOLD,
  HD_FONTSTYLE_ITALIC,
  HD_FONTSTYLE_BOLD_ITALIC
};

//
// Sizes...
//

#  define HD_FONTSIZE_H1	6
#  define HD_FONTSIZE_H2	5
#  define HD_FONTSIZE_H3	4
#  define HD_FONTSIZE_H4	3
#  define HD_FONTSIZE_H5	2
#  define HD_FONTSIZE_H6	1
#  define HD_FONTSIZE_H7	0
#  define HD_FONTSIZE_P		3
#  define HD_FONTSIZE_PRE	2
#  define HD_FONTSIZE_SUB	-2
#  define HD_FONTSIZE_SUP	-2


//
// Style attributes...
//

enum hdStyleAttr
{
  HD_STYLE_BACKGROUND,
  HD_STYLE_BACKGROUND_COLOR,
  HD_STYLE_BACKGROUND_POSITION,
  HD_STYLE_BACKGROUND_REPEAT,
  HD_STYLE_BORDER,
  HD_STYLE_BORDER_BOTTOM,
  HD_STYLE_BORDER_BOTTOM_STYLE,
  HD_STYLE_BORDER_BOTTOM_WIDTH,
  HD_STYLE_BORDER_COLLAPSE,
  HD_STYLE_BORDER_COLOR,
  HD_STYLE_BORDER_LEFT,
  HD_STYLE_BORDER_LEFT_STYLE,
  HD_STYLE_BORDER_LEFT_WIDTH,
  HD_STYLE_BORDER_RIGHT,
  HD_STYLE_BORDER_RIGHT_STYLE,
  HD_STYLE_BORDER_RIGHT_WIDTH,
  HD_STYLE_BORDER_SPACING,
  HD_STYLE_BORDER_STYLE,
  HD_STYLE_BORDER_TOP,
  HD_STYLE_BORDER_TOP_STYLE,
  HD_STYLE_BORDER_TOP_WIDTH,
  HD_STYLE_BORDER_WIDTH,
  HD_STYLE_BOTTOM,
  HD_STYLE_CAPTION_SIDE,
  HD_STYLE_CLEAR,
  HD_STYLE_COLOR,
  HD_STYLE_CONTENT,
  HD_STYLE_COUNTER_INCREMENT,
  HD_STYLE_COUNTER_RESET,
  HD_STYLE_DIRECTION,
  HD_STYLE_DISPLAY,
  HD_STYLE_EMPTY_CELLS,
  HD_STYLE_FLOAT,
  HD_STYLE_FONT,
  HD_STYLE_FONT_FAMILY,
  HD_STYLE_FONT_SIZE,
  HD_STYLE_FONT_SIZE_ADJUST,
  HD_STYLE_FONT_STRETCH,
  HD_STYLE_FONT_STYLE,
  HD_STYLE_FONT_VARIANT,
  HD_STYLE_FONT_WEIGHT,
  HD_STYLE_HEIGHT,
  HD_STYLE_LEFT,
  HD_STYLE_LETTER_SPACING,
  HD_STYLE_LINE_HEIGHT,
  HD_STYLE_LIST_STYLE,
  HD_STYLE_LIST_STYLE_IMAGE,
  HD_STYLE_LIST_STYLE_POSITION,
  HD_STYLE_LIST_STYLE_TYPE,
  HD_STYLE_MARGIN,
  HD_STYLE_MARGIN_BOTTOM,
  HD_STYLE_MARGIN_LEFT,
  HD_STYLE_MARGIN_RIGHT,
  HD_STYLE_MARGIN_TOP,
  HD_STYLE_MARKER_OFFSET,
  HD_STYLE_MAX_HEIGHT,
  HD_STYLE_MAX_WIDTH,
  HD_STYLE_MIN_HEIGHT,
  HD_STYLE_MIN_WIDTH,
  HD_STYLE_ORPHANS,
  HD_STYLE_OUTLINE,
  HD_STYLE_OUTLINE_COLOR,
  HD_STYLE_OUTLINE_STYLE,
  HD_STYLE_OUTLINE_WIDTH,
  HD_STYLE_PADDING,
  HD_STYLE_PADDING_BOTTOM,
  HD_STYLE_PADDING_LEFT,
  HD_STYLE_PADDING_RIGHT,
  HD_STYLE_PADDING_TOP,
  HD_STYLE_PAGE_BREAK_AFTER,
  HD_STYLE_PAGE_BREAK_BEFORE,
  HD_STYLE_PAGE_BREAK_INSIDE,
  HD_STYLE_POSITION,
  HD_STYLE_QUOTES,
  HD_STYLE_RIGHT,
  HD_STYLE_SIZE,
  HD_STYLE_TEXT_ALIGN,
  HD_STYLE_TEXT_DECORATION,
  HD_STYLE_TEXT_INDENT,
  HD_STYLE_TEXT_TRANSFORM,
  HD_STYLE_TOP,
  HD_STYLE_UNICODE_BIDI,
  HD_STYLE_VERTICAL_ALIGN,
  HD_STYLE_WHITE_SPACE,
  HD_STYLE_WIDOWS,
  HD_STYLE_WIDTH,
  HD_STYLE_WORD_SPACING,
  HD_STYLE_Z_INDEX,
  HD_STYLE_MAX
}

//
// Style data...
//

struct hdStyle
{
  hdElement	elements[4];	// Elements for selection
  char		*classes[4],	// Classes for selection
		*ids[4],	// IDs for selection
		*targets[4],	// Targets for selection
		*attrs[HD_STYLE_MAX];
				// Style attributes
};

//
// Stylesheet...
//

struct hdStyleSheet
{
  int		num_styles;	// Number of styles
  hdStyle	**styles;	// Array of styles
};


//
// Tree attribute...
//

struct hdTreeAttr
{
  char		*name,		// Variable name
		*value;		// Variable value
};

//
// Parsing tree...
//

struct hdTree
{
  // Node data...
  hdTree	*parent,	// Parent tree entry
		*child,		// First child entry
		*last_child,	// Last child entry
		*prev,		// Previous entry on this level
		*next,		// Next entry on this level
		*link;		// Linked-to
  hdElement	element;	// Markup element
  char		*data;		// Text (HD_ELEMENT_NONE or HD_ELEMENT_COMMENT)
  unsigned	halignment:2,	// Horizontal alignment
		valignment:2,	// Vertical alignment
		typeface:4,	// Typeface code
		size:3,		// Size of text
		style:2,	// Style of text
		underline:1,	// Text is underlined?
		strikethrough:1,// Text is struck-through?
		subscript:1,	// Text is subscripted?
		superscript:1,	// Text is superscripted?
		preformatted:1,	// Preformatted text?
		indent:4;	// Indentation level 0-15
  char		red,		// Color of this fragment
		green,
		blue;
  float		width,		// Width of this fragment in points
		height;		// Height of this fragment in points
  int		nattrs;		// Number of attributes...
  hdTreeAttr	*attrs;		// Attributes...

  // Global data...
  static const char	*elements[];
  static const char	*datadir;
  static float		ppi;
  static int		grayscale;
  static char		text_color[];
  static float		browser_width;
  static float		font_sizes[],
			line_spacings[];
  static hdFontFace	body_font,
			heading_font;
  static char		text_charset[32];
  static float		font_widths[4][4][256];
  static const char	font_glyphs[];
  static const char	*fonts[4][4];

  // Methods...
  hdTree(hdElement e = 0, const char *d = (const char *)0);
  ~hdTree();

  void		add(hdTree *p);	// Add to end of parent
  void		insert(hdTree *p);// Insert at beginning of parent
  void		remove();	// Remove from parent

  char		*get_text();	// Create a copy of the text under this node
  const char	*get_meta(const char *name);
				// Find META data
  static hdTree	*read(hdTree *p, hdFile *fp, const char *base,
		      const char *path);

  const char	*get_attr(const char *name);
  void		set_attr(const char *name, const char *value);

  const char	*get_style(const char *name);

  static void	set_base_size(float p, float s);
  static void	set_charset(const char *cs);
  static void	set_text_color(const char *color);
} hdTree;

#endif // !_HTMLDOC_TREE_H_

//
// End of "$Id: tree.h,v 1.1 2002/01/14 02:55:21 mike Exp $".
//
