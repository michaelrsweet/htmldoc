//
// "$Id: style.h,v 1.16 2002/09/02 23:04:12 mike Exp $"
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

//* @package HTMLDOC
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
  HD_ELEMENT_OBJECT,
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

// background-attachment values...
enum hdBackgroundAttachment
{
  HD_BACKGROUNDATTACHMENT_SCROLL = 0,
  HD_BACKGROUNDATTACHMENT_FIXED
};

// background-repeat values...
enum hdBackgroundRepeat
{
  HD_BACKGROUNDREPEAT_REPEAT = 0,
  HD_BACKGROUNDREPEAT_REPEAT_X,
  HD_BACKGROUNDREPEAT_REPEAT_Y,
  HD_BACKGROUNDREPEAT_NO_REPEAT
};

// border-style values...
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

// caption-side values...
enum hdCaptionSide
{
  HD_CAPTIONSIDE_TOP = 0,
  HD_CAPTIONSIDE_BOTTOM,
  HD_CAPTIONSIDE_LEFT,
  HD_CAPTIONSIDE_RIGHT
};

// clear values...
enum hdClear
{
  HD_CLEAR_NONE = 0,
  HD_CLEAR_LEFT,
  HD_CLEAR_RIGHT,
  HD_CLEAR_BOTH
};

// display values...
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

// float values...
enum hdFloat
{
  HD_FLOAT_NONE = 0,
  HD_FLOAT_LEFT,
  HD_FLOAT_RIGHT
};

// font-style values...
enum hdFontStyle
{
  HD_FONTSTYLE_NORMAL = 0,
  HD_FONTSTYLE_ITALIC,
  HD_FONTSTYLE_OBLIQUE
};

// font-variant values...
enum hdFontVariant
{
  HD_FONTVARIANT_NORMAL = 0,
  HD_FONTVARIANT_SMALL_CAPS
};

// font-weight values...
enum hdFontWeight
{
  HD_FONTWEIGHT_NORMAL = 0,
  HD_FONTWEIGHT_BOLD,
  HD_FONTWEIGHT_BOLDER,
  HD_FONTWEIGHT_LIGHTER
};

// list-style-position values...
enum hdListStylePosition
{
  HD_LISTSTYLEPOSITION_INSIDE = 0,
  HD_LISTSTYLEPOSITION_OUTSIDE
};

// list-style-type values...
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

// page-break values...
enum hdPageBreak
{
  HD_PAGEBREAK_AUTO = 0,
  HD_PAGEBREAK_ALWAYS,
  HD_PAGEBREAK_AVOID,
  HD_PAGEBREAK_LEFT,
  HD_PAGEBREAK_RIGHT
};

// text-align values...
enum hdTextAlign
{
  HD_TEXTALIGN_LEFT = 0,
  HD_TEXTALIGN_CENTER,
  HD_TEXTALIGN_RIGHT,
  HD_TEXTALIGN_JUSTIFY
};

// text-decoration values...
enum hdTextDecoration
{
  HD_TEXTDECORATION_NONE = 0,
  HD_TEXTDECORATION_UNDERLINE,
  HD_TEXTDECORATION_OVERLINE,
  HD_TEXTDECORATION_LINE_THROUGH
};

// text-transform values...
enum hdTextTransform
{
  HD_TEXTTRANSFORM_NONE = 0,
  HD_TEXTTRANSFORM_CAPITALIZE,
  HD_TEXTTRANSFORM_UPPERCASE,
  HD_TEXTTRANSFORM_LOWERCASE
};

// vertical-align values...
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

// white-space values...
enum hdWhiteSpace
{
  HD_WHITESPACE_NORMAL = 0,
  HD_WHITESPACE_PRE,
  HD_WHITESPACE_NOWRAP
};


//
// Font constants...
//

// Standard font face values...
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

// Standard font style values...
enum hdFontInternal
{
  HD_FONTINTERNAL_NORMAL = 0,
  HD_FONTINTERNAL_BOLD,
  HD_FONTINTERNAL_ITALIC,
  HD_FONTINTERNAL_BOLD_ITALIC,
  HD_FONTINTERNAL_MAX
};

// Standard font encoding values...
enum hdFontEncoding
{
  HD_FONTENCODING_8BIT = 0,
  HD_FONTENCODING_UTF8
};


/**
 * The <TT>hdFontKernPair</TT> structure contains character kerning
 * information for a pair of characters...
 */
struct hdFontKernPair
{
  //* The first character for kerning.
  int		first;
  //* The second character for kerning.
  int		second;
  //* The horizontal adjustment value between characters.
  float		adjust;
};


struct hdStyleSheet;

/**
 * The <TT>hdStyleFont</TT> structure describes a single font that is used
 * by a stylesheet.
 */

struct hdStyleFont
{
  //* Object number for PDF files.
  int		object;
  //* Typeface identifier.
  hdFontFace	typeface;
  //* Internal font style.
  hdFontInternal style;
  // Character encoding.
  hdFontEncoding encoding;
  //* PostScript font name.
  char		*ps_name;
  //* Full font name.
  char		*full_name;
  //* Font filename.
  char		*font_file;
  //* True if this font is fixed width.
  int		fixed_width;
  //* Offset for underline.
  float		ul_position;
  //* Thickness for underline.
  float		ul_thickness;
  //* Height of uppercase letters.
  float		cap_height;
  //* Height of lowercase letters.
  float		x_height;
  //* Highest point in font.
  float		ascender;
  //* Lowest point in font.
  float		descender;
  //* Number of widths in array.
  int		num_widths;
  //* Character widths for 1pt text.
  float		*widths;
  //* Number of kerning pairs.
  int		num_kerns;
  //* Kerning pairs array.
  hdFontKernPair *kerns;

 /**
  * The constructor creates a new font record for the specified font
  * name and loads the necessary font width and kerning information.
  *
  * @param css A pointer to the style sheet.
  * @param t The font typeface index.
  * @param s The font style index.
  * @param n The PostScript name of the font.
  */
  hdStyleFont(hdStyleSheet *css, hdFontFace t, hdFontInternal s, const char *n);

 /**
  * The destructor free all memory associated with the font.
  */
  ~hdStyleFont();

  static int	compare_kerns(hdFontKernPair *a, hdFontKernPair *b);

 /**
  * The get_char() method returns the next character from the specified
  * string and updates the string pointer to the next character position.
  * This method handles multi-byte and single-byte encodings transparently
  * to the caller.
  *
  * @param s A pointer into a string.
  * @return The character value.
  */
  int		get_char(const char *&s);

 /**
  * The get_kerning() method generates an array of kerning values
  * as well as a total kerning adjustment for the specified string.
  * 
  * @param s The string to kern.
  * @param tk Variable to hold total kerning value.
  * @param kl Pointer for kerning array for each character.
  * @return The number of kerning entries. Normally 1 less
  * then the total number of chracters in the input string.
  */
  int		get_kerning(const char *s, float &tk, float *&kl);

 /**
  * The get_num_chars() method returns the actual number of
  * characters in the given string and handles multi-byte and
  * single-byte encodings transparently to the caller.
  *
  * @param s A pointer to the string.
  * @return The number of characters in the string.
  */
  int		get_num_chars(const char *s);

 /**
  * The get_width() method returns the width of a string, including
  * adjustments for kerning. The width value is scaled for a point
  * size of 1.0.
  *
  * @param s A pointer to the string.
  * @return The width of the string.
  */
  float		get_width(const char *s);

  int		read_afm(hdFile *fp, hdStyleSheet *css);
  int		read_pfm(hdFile *fp, hdStyleSheet *css);
  int		read_ttf(hdFile *fp, hdStyleSheet *css);
};

// border information...
struct hdBorder
{
  unsigned char		color[3];	// Color of border
  char			color_set;	// Is the color set?
  hdBorderStyle		style;		// Rendering style of border
  float			width;		// Width of border
};

#define HD_SELECTOR_MAX	100

/**
 * The hdStyleSelector structure is used to select specific styles
 * in a stylesheet.
 */
struct hdStyleSelector
{
  hdElement		element;	// Element for selection
  char			*class_,	// Class name for selection
			*pseudo,	// Pseudo-class for selection
			*id;		// ID for selection

 /**
  * The constructor creates a new, blank selector.
  */
  hdStyleSelector();

 /**
  * The set() method sets the selector values.
  *
  * @param e The HTML element.
  * @param c The HTML CLASS attribute.
  * @param p The HTML pseudo-class attribute (link, visited, etc.)
  * @param i The HTML ID attribute.
  */
  void	set(hdElement e, const char *c, const char *p, const char *i);

 /**
  * The clear() method resets the selector values, freeing memory as needed.
  */
  void	clear();
};

/**
 * The hdStyle structure stores all of the supported style information
 * for a single style in a stylesheet.
 */
struct hdStyle
{
 /**
  * True if relative attributes have been updated in the style record.
  */
  int			updated;
 /**
  * Number of selectors for the style record.
  */
  int			num_selectors;
 /**
  * Selectors for the style record.
  */
  hdStyleSelector	*selectors;

 /**
  * The <TT>background-color</TT> value.
  */
  unsigned char		background_color[3];
 /**
  * True if the <TT>background-color</TT> value is set for this style.
  */
  char			background_color_set;
 /**
  * The <TT>background-image</TT> value.
  */
  char			*background_image;
 /**
  * The <TT>background-position</TT> values.
  */
  float			background_position[2];
 /**
  * The relative <TT>background-position</TT> values, if any.
  */
  char			*background_position_rel[2];
 /**
  * The <TT>background-repeat</TT> value.
  */
  hdBackgroundRepeat	background_repeat;
 /**
  * The <TT>border-left</TT>, <TT>border-right</TT>, <TT>border-top</TT>,
  * and <TT>border-bottom</TT> values.
  */
  hdBorder		border[4];
 /**
  * The <TT>clear</TT> value.
  */
  hdClear		clear;
 /**
  * The <TT>color</TT> value.
  */
  unsigned char		color[3];
 /**
  * True if the <TT>color</TT> value is set for this style.
  */
  char			color_set;
 /**
  * The <TT>display</TT> value.
  */
  hdDisplay		display;
 /**
  * The <TT>float</TT> value.
  */
  hdFloat		float_;
 /**
  * The <TT>font associated with this style.
  */
  hdStyleFont		*font;
 /**
  * The <TT>font-family</TT> value.
  */
  char			*font_family;
 /**
  * The <TT>font-size</TT> value.
  */
  float			font_size;
 /**
  * The relative <TT>font-size</TT> value, if any.
  */
  char			*font_size_rel;
 /**
  * The <TT>font-style</TT> value.
  */
  hdFontStyle		font_style;
 /**
  * The <TT>font-variant</TT> value.
  */
  hdFontVariant		font_variant;
 /**
  * The <TT>font-weight</TT> value.
  */
  hdFontWeight		font_weight;
 /**
  * The <TT>height</TT> value.
  */
  float			height;
 /**
  * The relative <TT>height</TT> value, if any.
  */
  char			*height_rel;
 /**
  * The <TT>letter-spacing</TT> value.
  */
  float			letter_spacing;
 /**
  * The <TT>line-height</TT> value.
  */
  float			line_height;
 /**
  * The relative <TT>line-height</TT> value, if any.
  */
  char			*line_height_rel;
 /**
  * The <TT>list-style-image</TT> value.
  */
  char			*list_style_image;
 /**
  * The <TT>list-style-position</TT> value.
  */
  hdListStylePosition	list_style_position;
 /**
  * The <TT>list-style-type</TT> value.
  */
  hdListStyleType	list_style_type;
 /**
  * The <TT>margin-left</TT>, <TT>margin-right</TT>, <TT>margin-top</TT>,
  * and <TT>margin-bottom</TT> values.
  */
  float			margin[4];
 /**
  * The relative <TT>margin-left</TT>, <TT>margin-right</TT>,
  * <TT>margin-top</TT>, and <TT>margin-bottom</TT> values, if any.
  */
  char			*margin_rel[4];
 /**
  * The <TT>padding-left</TT>, <TT>padding-right</TT>, <TT>padding-top</TT>,
  * and <TT>padding-bottom</TT> values.
  */
  float			padding[4];
 /**
  * The relative <TT>padding-left</TT>, <TT>padding-right</TT>,
  * <TT>padding-top</TT>, and <TT>padding-bottom values</TT>, if any.
  */
  char			*padding_rel[4];
 /**
  * The <TT>page-break-after</TT> value.
  */
  hdPageBreak		page_break_after;
 /**
  * The <TT>page-break-before</TT> value.
  */
  hdPageBreak		page_break_before;
 /**
  * The <TT>page-break-inside</TT> value.
  */
  hdPageBreak		page_break_inside;
 /**
  * The <TT>position-left</TT>, <TT>position-right</TT>,
  * <TT>position-top</TT>, and <TT>position-bottom</TT> values.
  */
  float			position[4];
 /**
  * The relative <TT>position-left</TT>, <TT>position-right</TT>,
  * <TT>position-top</TT>, and <TT>position-bottom</TT> values, if any.
  */
  char			*position_rel[4];
 /**
  * The <TT>text-align</TT> value.
  */
  hdTextAlign		text_align;
 /**
  * The <TT>text-decoration</TT> value.
  */
  hdTextDecoration	text_decoration;
 /**
  * The <TT>text-indent</TT> value.
  */
  float			text_indent;
 /**
  * The relative <TT>text-indent</TT> value, if any.
  */
  char			*text_indent_rel;
 /**
  * The <TT>text-transform</TT> value.
  */
  hdTextTransform	text_transform;
 /**
  * The <TT>vertical-align</TT> value.
  */
  hdVerticalAlign	vertical_align;
 /**
  * The <TT>white-space</TT> value.
  */
  hdWhiteSpace		white_space;
 /**
  * The <TT>width</TT> value.
  */
  float			width;
 /**
  * The relative <TT>width</TT> value, if any.
  */
  char			*width_rel;
 /**
  * The <TT>word-spacing</TT> value.
  */
  float			word_spacing;

 /**
  * The constructor creates a new style record.
  *
  * @param nsels The number of selectors for the style.
  * @param sels The selectors for the style.
  * @param p The parent style.
  */
  hdStyle(int nsels, hdStyleSelector *sels, hdStyle *p = (hdStyle *)0);

 /**
  * The destructor free all memory associated with the style record.
  */
  ~hdStyle();

 /**
  * The get_border() method returns the width in points of the specified
  * border.
  *
  * @param p The border position: HD_POS_LEFT, HD_POS_RIGHT, HD_POS_TOP,
  * or HD_POS_BOTTOM.
  * @return The border width in points.
  */
  float		get_border(int p)
		{
		  if (border[p].width == HD_WIDTH_AUTO)
		    return (0.0f);
		  else
		    return (border[p].width);
		}

 /**
  * The get_border_style() method returns the border style associated
  * with the specified string value.
  *
  * @param value The string to be converted.
  * @return The border style associated with the string.
  */
  hdBorderStyle	get_border_style(const char *value);

 /**
  * The get_border_width() method returns the width in points for the
  * given string value.
  *
  * @param value The border value string.
  * @param css The stylesheet.
  * @return The width in points.
  */
  float		get_border_width(const char *value, hdStyleSheet *css);

 /**
  * The get_color() method converts the string color to a 24-bit
  * RGB value.
  *
  * @param value The color value string.
  * @param rgb The RGB color array.
  * @return 1 if the color was converted, 0 otherwise.
  */
  static int	get_color(const char *color, unsigned char *rgb);

  float		get_length(const char *length, float max_length,
		           hdStyleSheet *css, int *relative = (int *)0);
  hdListStyleType get_list_style_type(const char *value);
  float		get_margin(int p)
		{
		  if (margin[p] == HD_WIDTH_AUTO)
		    return (0.0f);
		  else
		    return (margin[p]);
		}
  float		get_padding(int p)
		{
		  if (padding[p] == HD_WIDTH_AUTO)
		    return (0.0f);
		  else
		    return (padding[p]);
		}
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

//* orientation values...
enum hdOrientation
{
  HD_ORIENTATION_PORTRAIT = 0,
  HD_ORIENTATION_LANDSCAPE,
  HD_ORIENTATION_REVERSE_PORTRAIT,
  HD_ORIENTATION_REVERSE_LANDSCAPE
};

//* sides values...
enum hdSides
{
  HD_SIDES_ONE_SIDED = 0,
  HD_SIDES_TWO_SIDED_LONG_EDGE,
  HD_SIDES_TWO_SIDED_SHORT_EDGE
};

struct hdTree;

//* media data...
struct hdStyleMedia
{
  char		size_name[64];	// Page size name
  float		page_width,	// Page width, points
		page_length,	// Page length, points
		page_left,	// Left position, points
		page_bottom,	// Bottom position, points
		page_right,	// Right position, points
		page_top,	// Top position, points
		page_print_width,
				// Printable width, points
		page_print_length;
				// Printable length, points
  char		media_color[64],// Current media color
		media_type[64];	// Current media type
  int		media_position;	// Current media position
  hdOrientation	orientation;	// Orientation of the page
  hdSides	sides;		// Format single or double-sided?

  hdStyleMedia();

  void		set_margins(float l, float b, float r, float t);
  void		set_orientation(hdOrientation o);
  void		set_size(float w, float l);
  void		set_size(const char *name); 
  void		update_printable();
};

//* style sheet data...
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

  hdStyleMedia	default_media,	// Default media attributes from stylesheet
		media;		// Current media attributes

  int		grayscale;	// Grayscale output?
  float		ppi;		// Pixel resolution
  unsigned	private_id;	// Private style ID


  hdStyleSheet();
  ~hdStyleSheet();

  void		add_style(hdStyle *s);
  hdStyleFont	*find_font(hdStyle *s);
  hdStyle	*find_style(hdTree *t);
  hdStyle	*find_style(int nsels, hdStyleSelector *sels, int exact = 0);
  int		get_glyph(const char *s);
  hdStyle	*get_private_style(hdTree *t);
  int		load(hdFile *f, const char *path = (const char *)0);
  void		pattern(const char *r, char p[256]);
  char		*read(hdFile *f, const char *p, char *s, int slen);
  void		set_charset(const char *cs);
 
  void		update_styles();
};


#endif // !_HTMLDOC_STYLE_H_

//
// End of "$Id: style.h,v 1.16 2002/09/02 23:04:12 mike Exp $".
//
