//
// "$Id: style.h,v 1.8 2002/02/23 04:03:30 mike Exp $"
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
  HD_ELEMENT_NONE = 0,		// Text fragment
  HD_ELEMENT_FILE,		// File Delimiter
  HD_ELEMENT_ERROR,		// Bad element
  HD_ELEMENT_UNKNOWN,		// Unknown element
  HD_ELEMENT_COMMENT,
  HD_ELEMENT_DOCTYPE,
  HD_ELEMENT_A,
  HD_ELEMENT_ACRONYM,
  HD_ELEMENT_ADDRESS,
  HD_ELEMENT_AREA,
  HD_ELEMENT_B,
  HD_ELEMENT_BASE,
  HD_ELEMENT_BASEFONT,
  HD_ELEMENT_BIG,
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
  HD_ELEMENT_OL,
  HD_ELEMENT_OPTION,
  HD_ELEMENT_P,
  HD_ELEMENT_PRE,
  HD_ELEMENT_S,
  HD_ELEMENT_SAMP,
  HD_ELEMENT_SELECT,
  HD_ELEMENT_SMALL,
  HD_ELEMENT_SPACER,
  HD_ELEMENT_SPAN,
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
  HD_ELEMENT_WBR,
  HD_ELEMENT_MAX
};

//
// Macros to test elements for various things...
//

enum hdElGroup
{
  HD_ELGROUP_NONE = 0,
  HD_ELGROUP_GROUP = 1,
  HD_ELGROUP_BLOCK = 2,
  HD_ELGROUP_TABLE = 4,
  HD_ELGROUP_ROWCOL = 8,
  HD_ELGROUP_CELL = 16,
  HD_ELGROUP_LIST = 32,
  HD_ELGROUP_ITEM = 64,
  HD_ELGROUP_INLINE = 128
};

#define hdElIsGroup(x)	(hdTree::elgroup[x] & HD_ELGROUP_GROUP)
#define hdElIsBlock(x)	(hdTree::elgroup[x] & HD_ELGROUP_BLOCK)
#define hdElIsTable(x)	(hdTree::elgroup[x] & HD_ELGROUP_TABLE)
#define hdElIsRowCol(x)	(hdTree::elgroup[x] & HD_ELGROUP_ROWCOL)
#define hdElIsCell(x)	(hdTree::elgroup[x] & HD_ELGROUP_CELL)
#define hdElIsList(x)	(hdTree::elgroup[x] & HD_ELGROUP_LIST)
#define hdElIsItem(x)	(hdTree::elgroup[x] & HD_ELGROUP_ITEM)
#define hdElIsInline(x)	(hdTree::elgroup[x] & HD_ELGROUP_INLINE)
#define hdElIsNone(x)	(hdTree::elgroup[x] == HD_ELGROUP_NONE)


//
// Stylesheet attribute values...
//

#define HD_MARGIN_AUTO	-65536.0
#define HD_WIDTH_AUTO	-65536.0
#define HD_HEIGHT_AUTO	-65536.0

#define HD_POS_BOTTOM	0
#define HD_POS_LEFT	1
#define HD_POS_RIGHT	2
#define HD_POS_TOP	3

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
  HD_BORDERSTYLE_NONE = 0,
  HD_BORDERSTYLE_DOTTED,
  HD_BORDERSTYLE_DASHED,
  HD_BORDERSTYLE_SOLID,
  HD_BORDERSTYLE_DOUBLE,
  HD_BORDERSTYLE_GROOVE,
  HD_BORDERSTYLE_RIDGE,
  HD_BORDERSTYLE_INSET,
  HD_BORDERSTYLE_OUTSET
};

enum hdCaptionSide
{
  HD_CAPTIONSIDE_TOP = 0,
  HD_CAPTIONSIDE_BOTTOM,
  HD_CAPTIONSIDE_LEFT,
  HD_CAPTIONSIDE_RIGHT
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
  HD_DISPLAY_NONE = 0,
  HD_DISPLAY_BLOCK,
  HD_DISPLAY_COMPACT,
  HD_DISPLAY_INLINE,
  HD_DISPLAY_INLINE_TABLE,
  HD_DISPLAY_LIST_ITEM,
  HD_DISPLAY_MARKER,
  HD_DISPLAY_RUN_IN,
  HD_DISPLAY_TABLE,
  HD_DISPLAY_TABLE_CAPTION,
  HD_DISPLAY_TABLE_CELL,
  HD_DISPLAY_TABLE_COLUMN,
  HD_DISPLAY_TABLE_COLUMN_GROUP,
  HD_DISPLAY_TABLE_FOOTER_GROUP,
  HD_DISPLAY_TABLE_HEADER_GROUP,
  HD_DISPLAY_TABLE_ROW,
  HD_DISPLAY_TABLE_ROW_GROUP
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

enum hdFontVariant
{
  HD_FONTVARIANT_NORMAL = 0,
  HD_FONTVARIANT_SMALL_CAPS
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
  HD_LISTSTYLEPOSITION_INSIDE = 0,
  HD_LISTSTYLEPOSITION_OUTSIDE
};

enum hdListStyleType
{
  HD_LISTSTYLETYPE_NONE = 0,
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
  HD_WHITESPACE_NORMAL = 0,
  HD_WHITESPACE_PRE,
  HD_WHITESPACE_NOWRAP
};


//
// Font constants...
//

enum hdFontFace
{
  HD_FONTFACE_MONOSPACE = 0,
  HD_FONTFACE_SERIF,
  HD_FONTFACE_SANS_SERIF,
  HD_FONTFACE_SYMBOL,
  HD_FONTFACE_CURSIVE,
  HD_FONTFACE_CUSTOM,
  HD_FONTFACE_MAX = 16
};

enum hdFontInternal
{
  HD_FONTINTERNAL_NORMAL = 0,
  HD_FONTINTERNAL_BOLD,
  HD_FONTINTERNAL_ITALIC,
  HD_FONTINTERNAL_BOLD_ITALIC,
  HD_FONTINTERNAL_MAX
};

enum hdFontEncoding
{
  HD_FONTENCODING_8BIT = 0,
  HD_FONTENCODING_UTF8
};

//
// Kerning information...
//

struct hdFontKernPair		// Kerning pair data
{
  int		first,		// First character for kerning
		second;		// First character for kerning
  float		adjust;		// Horizontal adjustment
};


//
// Style data...
//


struct hdStyleSheet;

struct hdStyleFont
{
  hdFontFace	typeface;	// Typeface identifier
  hdFontInternal style;		// Internal font style
  hdFontEncoding encoding;	// Character encoding
  char		*ps_name,	// PostScript font name
		*full_name,	// Full font name
		*font_file;	// Font filename
  int		fixed_width;	// Fixed width font?
  float		ul_position,	// Offset for underline
		ul_thickness,	// Thickness for underline
		cap_height,	// Height of uppercase letters
		x_height,	// Height of lowercase letters
		ascender,	// Highest point in font
		descender;	// Lowest point in font
  int		num_widths;	// Number of widths in array
  float		*widths;	// Character widths for 1pt text
  int		num_kerns;	// Number of kerning pairs
  hdFontKernPair *kerns;	// Kerning pairs array

  hdStyleFont(hdStyleSheet *css, hdFontFace t, hdFontInternal s, const char *n);
  ~hdStyleFont();

  static int	compare_kerns(hdFontKernPair *a, hdFontKernPair *b);
  int		get_kerning(const char *s, float *tk, float **kl);
  float		get_width(const char *s);

  int		read_afm(hdFile *fp, hdStyleSheet *css);
  int		read_pfm(hdFile *fp, hdStyleSheet *css);
  int		read_ttf(hdFile *fp, hdStyleSheet *css);
};

struct hdBorder
{
  unsigned char		color[3];	// Color of border
  char			color_set;	// Is the color set?
  hdBorderStyle		style;		// Rendering style of border
  float			width;		// Width of border
};

#define HD_SELECTOR_MAX	100

struct hdSelector
{
  hdElement		element;	// Element for selection
  char			*class_,	// Class name for selection
			*pseudo,	// Pseudo-class for selection
			*id;		// ID for selection

  hdSelector();

  void	set(hdElement e, const char *c, const char *p, const char *i);
  void	clear();
};

struct hdStyle
{
  int			updated;	// True if relative attrs have been updated
  int			num_selectors;	// Number of selectors for style
  hdSelector		*selectors;	// Selectors for style

  unsigned char		background_color[3];
  char			background_color_set;
  char			*background_image;
  float			background_position[2];
  char			*background_position_rel[2];
  hdBackgroundRepeat	background_repeat;
  hdBorder		border[4];
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
  hdFontVariant		font_variant;
  hdFontWeight		font_weight;
  float			height;
  char			*height_rel;
  float			letter_spacing;
  float			line_height;
  char			*line_height_rel;
  char			*list_style_image;
  hdListStylePosition	list_style_position;
  hdListStyleType	list_style_type;
  float			margin[4];
  char			*margin_rel[4];
  float			padding[4];
  char			*padding_rel[4];
  hdPageBreak		page_break_after;
  hdPageBreak		page_break_before;
  hdPageBreak		page_break_inside;
  float			position[4];
  char			*position_rel[4];
  hdTextAlign		text_align;
  hdTextDecoration	text_decoration;
  float			text_indent;
  char			*text_indent_rel;
  hdTextTransform	text_transform;
  hdVerticalAlign	vertical_align;
  hdWhiteSpace		white_space;
  float			width;
  char			*width_rel;
  float			word_spacing;

  hdStyle(int nsels, hdSelector *sels, hdStyle *p = (hdStyle *)0);
  ~hdStyle();

  hdBorderStyle	get_border_style(const char *value);
  float		get_border_width(const char *value, hdStyleSheet *css);
  int		get_color(const char *color, unsigned char *rgb);
  float		get_length(const char *length, float max_length,
		           hdStyleSheet *css, int *relative = (int *)0);
  hdListStyleType get_list_style_type(const char *value);
  hdPageBreak	get_page_break(const char *value);
  int		get_pos(const char *name);
  char		*get_subvalue(char *valueptr);
  void		inherit(hdStyle *p);
  int		load(hdStyleSheet *css, const char *s);
  void		update(hdStyleSheet *css);
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
};

enum hdSides
{
  HD_SIDES_ONE_SIDED = 0,
  HD_SIDES_TWO_SIDED_LONG_EDGE,
  HD_SIDES_TWO_SIDED_SHORT_EDGE
};

struct hdTree;

struct hdStyleSheet
{
  int		num_styles,	// Number of styles
		alloc_styles;	// Allocate style slots
  hdStyle	**styles;	// Array of styles
  int		max_selectors[HD_ELEMENT_MAX];
				// Maximum number of selectors in styles
  int		elements[HD_ELEMENT_MAX];
				// First style for each element
  int		num_fonts;	// Number of fonts defined
  hdStyleFont	*fonts[HD_FONTFACE_MAX][HD_FONTINTERNAL_MAX];
				// Array of fonts
  char		*font_names[HD_FONTFACE_MAX];
				// Names of base fonts...

  char		*charset;	// Character set
  hdFontEncoding encoding;	// Character encoding
  int		num_glyphs;	// Number of glyphs in charset
  char		**glyphs;	// Glyphs in charset

  char		size_name[64];	// Page size name
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
  int		grayscale;	// Grayscale output?
  float		ppi;		// Pixel resolution
  unsigned	private_id;	// Private style ID


  hdStyleSheet();
  ~hdStyleSheet();

  void		add_style(hdStyle *s);
  hdStyleFont	*find_font(hdStyle *s);
  hdStyle	*find_style(hdTree *t);
  hdStyle	*find_style(int nsels, hdSelector *sels, int exact = 0);
  int		get_glyph(const char *s);
  hdStyle	*get_private_style(hdTree *t);
  int		load(hdFile *f, const char *path = (const char *)0);
  void		pattern(const char *r, char p[256]);
  char		*read(hdFile *f, const char *p, char *s, int slen);
  void		set_charset(const char *cs);
  void		set_margins(float l, float b, float r, float t);
  void		set_orientation(hdOrientation o);
  void		set_size(float w, float l);
  void		set_size(const char *name); 
 
  void		update_printable();
  void		update_styles();
};


#endif // !_HTMLDOC_STYLE_H_

//
// End of "$Id: style.h,v 1.8 2002/02/23 04:03:30 mike Exp $".
//
