//
// "$Id: style.h,v 1.2 2002/01/16 22:10:49 mike Exp $"
//
//   Stylesheet definitions for HTMLDOC, a HTML document processing program.
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

#ifndef _HTMLDOC_STYLE_H_
#  define _HTMLDOC_STYLE_H_

//
// Include necessary headers...
//

#  include "file.h"


//
// HTML element constants...
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
// Stylesheet attribute values...
//

#define HD_MARGIN_AUTO	-65536.0
#define HD_WIDTH_AUTO	-65536.0
#define HD_HEIGHT_AUTO	-65536.0

enum hdBackgroundAttachment
{
  HD_BACKGROUNDATTACHMENT_SCROLL = 0,
  HD_BACKGROUNDATTACHMENT_FIXED
};

enum hdBackgroundRepeat
{
  HD_BACKGROUNDREPEAT_REPEAT = 0,
  HD_BACKGROUNDREPEAT_REPEAT_X,
  HD_BACKGROUNDREPEAT_REPEAT_Y,
  HD_BACKGROUNDREPEAT_NO_REPEAT
};

enum hdBorderStyle
{
  HD_BORDERSTYLE_NONE,
  HD_BORDERSTYLE_DOTTED,
  HD_BORDERSTYLE_DASHED,
  HD_BORDERSTYLE_SOLID,
  HD_BORDERSTYLE_DOUBLE,
  HD_BORDERSTYLE_GROOVE,
  HD_BORDERSTYLE_RIDGE,
  HD_BORDERSTYLE_INSET,
  HD_BORDERSTYLE_OUTSET
};

enum hdClear
{
  HD_CLEAR_NONE = 0,
  HD_CLEAR_LEFT,
  HD_CLEAR_RIGHT,
  HD_CLEAR_BOTH
};

enum hdDisplay
{
  HD_DISPLAY_NONE,
  HD_DISPLAY_BLOCK,
  HD_DISPLAY_INLINE,
  HD_DISPLAY_LIST_ITEM
};

enum hdFloat
{
  HD_FLOAT_NONE = 0,
  HD_FLOAT_LEFT,
  HD_FLOAT_RIGHT
};

enum hdFontStyle
{
  HD_FONTSTYLE_NORMAL = 0,
  HD_FONTSTYLE_ITALIC,
  HD_FONTSTYLE_OBLIQUE
};

enum hdFontVarient
{
  HD_FONTVARIENT_NORMAL = 0,
  HD_FONTVARIENT_SMALL_CAPS
};

enum hdFontWeight
{
  HD_FONTWEIGHT_NORMAL = 0,
  HD_FONTWEIGHT_BOLD,
  HD_FONTWEIGHT_BOLDER,
  HD_FONTWEIGHT_LIGHTER
};

enum hdListStylePosition
{
  HD_LISTSTYLEPOSITION_INSIDE,
  HD_LISTSTYLEPOSITION_OUTSIDE
};

enum hdListStyleType
{
  HD_LISTSTYLETYPE_NONE,
  HD_LISTSTYLETYPE_DISC,
  HD_LISTSTYLETYPE_CIRCLE,
  HD_LISTSTYLETYPE_SQUARE,
  HD_LISTSTYLETYPE_DECIMAL,
  HD_LISTSTYLETYPE_LOWER_ROMAN,
  HD_LISTSTYLETYPE_UPPER_ROMAN,
  HD_LISTSTYLETYPE_LOWER_ALPHA,
  HD_LISTSTYLETYPE_UPPER_ALPHA
};

enum hdPageBreak
{
  HD_PAGEBREAK_AUTO = 0,
  HD_PAGEBREAK_ALWAYS,
  HD_PAGEBREAK_AVOID,
  HD_PAGEBREAK_LEFT,
  HD_PAGEBREAK_RIGHT
};

enum hdTextAlign
{
  HD_TEXTALIGN_LEFT = 0,
  HD_TEXTALIGN_CENTER,
  HD_TEXTALIGN_RIGHT,
  HD_TEXTALIGN_JUSTIFY
};

enum hdTextDecoration
{
  HD_TEXTDECORATION_NONE = 0,
  HD_TEXTDECORATION_UNDERLINE,
  HD_TEXTDECORATION_OVERLINE,
  HD_TEXTDECORATION_LINE_THROUGH
};

enum hdTextTransform
{
  HD_TEXTTRANSFORM_NONE = 0,
  HD_TEXTTRANSFORM_CAPITALIZE,
  HD_TEXTTRANSFORM_UPPERCASE,
  HD_TEXTTRANSFORM_LOWERCASE
};

enum hdVerticalAlign
{
  HD_VERTICALALIGN_BASELINE = 0,
  HD_VERTICALALIGN_SUB,
  HD_VERTICALALIGN_SUPER,
  HD_VERTICALALIGN_TOP,
  HD_VERTICALALIGN_TEXT_TOP,
  HD_VERTICALALIGN_MIDDLE,
  HD_VERTICALALIGN_BOTTOM,
  HD_VERTICALALIGN_TEXT_BOTTOM
};

enum hdWhiteSpace
{
  HD_WHITESPACE_NORMAL,
  HD_WHITESPACE_PRE,
  HD_WHITESPACE_NOWRAP
};


//
// Font data...
//

enum hdFontFace
{
  HD_FONTFACE_COURIER = 0,
  HD_FONTFACE_TIMES,
  HD_FONTFACE_HELVETICA,
  HD_FONTFACE_SYMBOL,
  HD_FONTFACE_CUSTOM,
  HD_FONTFACE_MAX = 16
};

enum hdFontInternal
{
  HD_FONTINTERNAL_NORMAL = 0,
  HD_FONTINTERNAL_BOLD,
  HD_FONTINTERNAL_ITALIC,
  HD_FONTINTERNAL_BOLD_ITALIC,
  HD_FONTINTERNAL_MAX,
};

struct hdStyleFont
{
  hdFontFace	typeface;	// Typeface identifier
  char		*ps_name;	// PostScript font name
  float		ul_position,	// Offset for underline
		ul_thickness,	// Thickness for underline
		cap_height,	// Height of uppercase letters
		x_height,	// Height of lowercase letters
		ascender,	// Highest point in font
		descender;	// Lowest point in font
  float		*widths;	// Character widths for 1pt text
};


//
// Style data...
//

struct hdStyle
{
  int			updated;	// True if relative attrs have been updated
  int			num_selectors;	// Number of selectors for style
  hdElement		*elements;	// Elements for selection
  char			*classes,	// Classes for selection
			*ids,		// IDs for selection
			*targets;	// Targets for selection

  unsigned char		background_color[3];
  char			background_color_set;
  char			*background_image;
  float			background_position[2];
  char			*background_position_rel;
  hdBackgroundRepeat	background_repeat;
  float			border_bottom;
  hdBorderStyle		border_bottom_style;
  float			border_bottom_width;
  unsigned char		border_color[3];
  char			border_color_set;
  float			border_left;
  hdBorderStyle		border_left_style;
  float			border_left_width;
  float			border_right;
  hdBorderStyle		border_right_style;
  float			border_right_width;
  float			border_top;
  hdBorderStyle		border_top_style;
  float			border_top_width;
  hdClear		clear;
  unsigned char		color[3];
  char			color_set;
  hdDisplay		display;
  hdFloat		float_;
  hdStyleFont		*font;
  char			*font_family;
  float			font_size;
  char			*font_size_rel;
  hdFontStyle		font_style;
  hdFontVarient		font_variant;
  hdFontWeight		font_weight;
  float			height;
  char			*height_rel;
  float			left;
  char			*left_rel;
  float			letter_spacing;
  float			line_height;
  char			*line_height_rel;
  char			*list_style_image;
  hdListStylePosition	list_style_position;
  hdListStyleType	list_style_type;
  float			margin_bottom;
  char			*margin_bottom_set;
  float			margin_left;
  char			*margin_left_set;
  float			margin_right;
  char			*margin_right_set;
  float			margin_top;
  char			*margin_top_set;
  float			padding_bottom;
  char			*padding_bottom_rel;
  float			padding_left;
  char			*padding_left_rel;
  float			padding_right;
  char			*padding_right_rel;
  float			padding_top;
  char			*padding_top_rel;
  hdPageBreak		page_break_after;
  hdPageBreak		page_break_before;
  hdPageBreak		page_break_inside;
  float			right;
  char			*right_rel;
  hdTextAlign		text_align;
  hdTextDecoration	text_decoration;
  float			text_indent;
  char			*text_indent_rel;
  hdTextTransform	text_transform;
  float			top;
  char			*top_rel;
  hdVerticalAlign	vertical_align;
  hdWhiteSpace		white_space;
  float			width;
  char			*width_rel;
  float			word_spacing;
};


//
// Stylesheet...
//

enum hdOrientation
{
  HD_ORIENTATION_PORTRAIT = 0,
  HD_ORIENTATION_LANDSCAPE,
  HD_ORIENTATION_REVERSE_PORTRAIT,
  HD_ORIENTATION_REVERSE_LANDSCAPE
}

enum hdSides
{
  HD_SIDES_ONE_SIDED = 0,
  HD_SIDES_TWO_SIDED_LONG_EDGE,
  HD_SIDES_TWO_SIDED_SHORT_EDGE
}

struct hdStyleSheet
{
  int		num_styles;	// Number of styles
  hdStyle	**styles;	// Array of styles

  hdStyleFont	*fonts[HD_FONTFACE_MAX][HD_FONTINTERNAL_MAX];
				// Array of fonts

  float		width,		// Page width, points
		length,		// Page length, points
		left,		// Left position, points
		bottom,		// Bottom position, points
		right,		// Right position, points
		top,		// Top position, points
		print_width,	// Printable width, points
		print_length;	// Printable length, points

  hdOrientation	orientation;	// Orientation of the page
  hdSides	sides;		// Format single or double-sided?
};


#endif // !_HTMLDOC_STYLE_H_

//
// End of "$Id: style.h,v 1.2 2002/01/16 22:10:49 mike Exp $".
//
