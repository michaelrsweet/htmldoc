//
// "$Id: style.h,v 1.20 2004/02/03 02:55:29 mike Exp $"
//
//   Stylesheet definitions for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2004 by Easy Software Products.
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
#  include "types.h"


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

// "Auto" values
#define HD_MARGIN_AUTO	-65536.0
#define HD_WIDTH_AUTO	-65536.0
#define HD_HEIGHT_AUTO	-65536.0

// Indices for individual margins, padding, etc.
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
  * The <TT>constructor creates a new font record for the specified font
  * name and loads the necessary font width and kerning information.
  *
  * @param css hdStyleSheet* A pointer to the style sheet.
  * @param t hdFontFace The font typeface index.
  * @param s hdFontInternal The font style index.
  * @param n const&nbsp;char* The PostScript name of the font.
  */
  hdStyleFont(hdStyleSheet *css, hdFontFace t, hdFontInternal s, const char *n);

 /**
  * The <TT>destructor free all memory associated with the font.
  */
  ~hdStyleFont();

  static int	compare_kerns(hdFontKernPair *a, hdFontKernPair *b);

 /**
  * The <TT>get_char()</TT> method returns the next character from the specified
  * string and updates the string pointer to the next character position.
  * This method handles multi-byte and single-byte encodings transparently
  * to the caller.
  *
  * @param s const&nbsp;char* A pointer into a string.
  * @return The character value.
  */
  int		get_char(const char *&s);

 /**
  * The <TT>get_kerning()</TT> method generates an array of kerning values
  * as well as a total kerning adjustment for the specified string.
  * 
  * @param s const&nbsp;char* The string to kern.
  * @param tk float Variable to hold total kerning value.
  * @param kl float* Pointer for kerning array for each character.
  * @return The number of kerning entries. Normally 1 less
  * then the total number of chracters in the input string.
  */
  int		get_kerning(const char *s, float &tk, float *&kl);

 /**
  * The <TT>get_num_chars()</TT> method returns the actual number of
  * characters in the given string and handles multi-byte and
  * single-byte encodings transparently to the caller.
  *
  * @param s const&nbsp;char* A pointer to the string.
  * @return The number of characters in the string.
  */
  int		get_num_chars(const char *s);

 /**
  * The <TT>get_width()</TT> method returns the width of a string, including
  * adjustments for kerning. The width value is scaled for a point
  * size of 1.0.
  *
  * @param s const&nbsp;char* A pointer to the string.
  * @return The width of the string.
  */
  float		get_width(const char *s);

 /**
  * The <TT>read_afm()</TT> method loads font widths from an AFM file.
  *
  * @param fp hdFile* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_afm(hdFile *fp, hdStyleSheet *css);

 /**
  * The <TT>read_pfm()</TT> method loads font widths from a PFM file.
  *
  * @param fp hdFile* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_pfm(hdFile *fp, hdStyleSheet *css);

 /**
  * The <TT>read_ttf()</TT> method loads font widths from a TTF file.
  *
  * @param fp hdFile* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_ttf(hdFile *fp, hdStyleSheet *css);
};

/**
 * The <TT>hdBorder</TT> structure holds border attribute information.
 */
struct hdBorder
{
  //* Color of border
  unsigned char		color[3];
  //* Is the color set?
  char			color_set;
  //* Rendering style of border
  hdBorderStyle		style;
  //* Width of border in points
  float			width;
};

// Maximum number of selectors per style...
#define HD_SELECTOR_MAX	100

/**
 * The <TT>hdStyleSelector</TT> structure is used to select specific styles
 * in a stylesheet.
 */
struct hdStyleSelector
{
  //* Element for selection
  hdElement		element;
  //* Class name for selection
  char			*class_;
  //* Pseudo-class for selection
  char			*pseudo;
  //* ID for selection
  char			*id;

 /**
  * The <TT>constructor creates a new, blank selector.
  */
  hdStyleSelector();

 /**
  * The <TT>set()</TT> method sets the selector values.
  *
  * @param e hdElement The HTML element.
  * @param c const&nbsp;char* The HTML CLASS attribute.
  * @param p const&nbsp;char* The HTML pseudo-class attribute (link, visited, etc.)
  * @param i const&nbsp;char* The HTML ID attribute.
  */
  void	set(hdElement e, const char *c, const char *p, const char *i);

 /**
  * The <TT>clear()</TT> method resets the selector values, freeing memory as needed.
  */
  void	clear();
};

/**
 * The <TT>hdStyle</TT> structure stores all of the supported style
 * information for a single style in a stylesheet.
 */
struct hdStyle
{
  //* True if relative attributes have been updated in the style record.
  int			updated;
  //* Number of selectors for the style record.
  int			num_selectors;
  //* Selectors for the style record.
  hdStyleSelector	*selectors;

  //* The <TT>background-color</TT> value.
  unsigned char		background_color[3];
  //* True if the <TT>background-color</TT> value is set for this style.
  char			background_color_set;
  //* The <TT>background-image</TT> value.
  char			*background_image;
  //* The <TT>background-position</TT> values.
  float			background_position[2];
  //* The relative <TT>background-position</TT> values, if any.
  char			*background_position_rel[2];
  //* The <TT>background-repeat</TT> value.
  hdBackgroundRepeat	background_repeat;
  //* The <TT>border-left</TT>, <TT>border-right</TT>, <TT>border-top</TT>,
  //* and <TT>border-bottom</TT> values.
  hdBorder		border[4];
  //* The <TT>clear</TT> value.
  hdClear		clear;
  //* The <TT>color</TT> value.
  unsigned char		color[3];
  //* True if the <TT>color</TT> value is set for this style.
  char			color_set;
  //* The <TT>display</TT> value.
  hdDisplay		display;
  //* The <TT>float</TT> value.
  hdFloat		float_;
  //* The <TT>font associated with this style.
  hdStyleFont		*font;
  //* The <TT>font-family</TT> value.
  char			*font_family;
  //* The <TT>font-size</TT> value.
  float			font_size;
  //* The relative <TT>font-size</TT> value, if any.
  char			*font_size_rel;
  //* The <TT>font-style</TT> value.
  hdFontStyle		font_style;
  //* The <TT>font-variant</TT> value.
  hdFontVariant		font_variant;
  //* The <TT>font-weight</TT> value.
  hdFontWeight		font_weight;
  //* The <TT>height</TT> value.
  float			height;
  //* The relative <TT>height</TT> value, if any.
  char			*height_rel;
  //* The <TT>letter-spacing</TT> value.
  float			letter_spacing;
  //* The <TT>line-height</TT> value.
  float			line_height;
  //* The relative <TT>line-height</TT> value, if any.
  char			*line_height_rel;
  //* The <TT>list-style-image</TT> value.
  char			*list_style_image;
  //* The <TT>list-style-position</TT> value.
  hdListStylePosition	list_style_position;
  //* The <TT>list-style-type</TT> value.
  hdListStyleType	list_style_type;
  //* The <TT>margin-left</TT>, <TT>margin-right</TT>, <TT>margin-top</TT>,
  //* and <TT>margin-bottom</TT> values.
  float			margin[4];
  //* The relative <TT>margin-left</TT>, <TT>margin-right</TT>,
  //* <TT>margin-top</TT>, and <TT>margin-bottom</TT> values, if any.
  char			*margin_rel[4];
  //* The <TT>padding-left</TT>, <TT>padding-right</TT>, <TT>padding-top</TT>,
  //* and <TT>padding-bottom</TT> values.
  float			padding[4];
  //* The relative <TT>padding-left</TT>, <TT>padding-right</TT>,
  //* <TT>padding-top</TT>, and <TT>padding-bottom values</TT>, if any.
  char			*padding_rel[4];
  //* The <TT>page-break-after</TT> value.
  hdPageBreak		page_break_after;
  //* The <TT>page-break-before</TT> value.
  hdPageBreak		page_break_before;
  //* The <TT>page-break-inside</TT> value.
  hdPageBreak		page_break_inside;
  //* The <TT>position-left</TT>, <TT>position-right</TT>,
  //* <TT>position-top</TT>, and <TT>position-bottom</TT> values.
  float			position[4];
  //* The relative <TT>position-left</TT>, <TT>position-right</TT>,
  //* <TT>position-top</TT>, and <TT>position-bottom</TT> values, if any.
  char			*position_rel[4];
  //* The <TT>text-align</TT> value.
  hdTextAlign		text_align;
  //* The <TT>text-decoration</TT> value.
  hdTextDecoration	text_decoration;
  //* The <TT>text-indent</TT> value.
  float			text_indent;
  //* The relative <TT>text-indent</TT> value, if any.
  char			*text_indent_rel;
  //* The <TT>text-transform</TT> value.
  hdTextTransform	text_transform;
  //* The <TT>vertical-align</TT> value.
  hdVerticalAlign	vertical_align;
  //* The <TT>white-space</TT> value.
  hdWhiteSpace		white_space;
  //* The <TT>width</TT> value.
  float			width;
  //* The relative <TT>width</TT> value, if any.
  char			*width_rel;
  //* The <TT>word-spacing</TT> value.
  float			word_spacing;

 /**
  * The <TT>constructor creates a new style record.
  *
  * @param nsels int The number of selectors for the style.
  * @param sels hdStyleSelector* The selectors for the style.
  * @param p hdStyle* The parent style.
  */
  hdStyle(int nsels, hdStyleSelector *sels, hdStyle *p = (hdStyle *)0);

 /**
  * The <TT>destructor free all memory associated with the style record.
  */
  ~hdStyle();

 /**
  * The <TT>get_border()</TT> method returns the width in points of the specified
  * border.
  *
  * @param p int The border position: <TT>HD_POS_LEFT</TT>,
  * <TT>HD_POS_RIGHT</TT>, <TT>HD_POS_TOP</TT>, or
  * <TT>HD_POS_BOTTOM</TT>.
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
  * The <TT>get_border_style()</TT> method returns the border style associated
  * with the specified string value.
  *
  * @param value const&nbsp;char* The string to be converted.
  * @return The border style associated with the string.
  */
  hdBorderStyle	get_border_style(const char *value);

 /**
  * The <TT>get_border_width()</TT> method returns the width in points for the
  * given string value.
  *
  * @param value const&nbsp;char* The border value string.
  * @param css hdStyleSheet* The stylesheet.
  * @return The width in points.
  */
  float		get_border_width(const char *value, hdStyleSheet *css);

 /**
  * The <TT>get_color()</TT> method converts the string color to a 24-bit
  * RGB value.
  *
  * @param value const&nbsp;char* The color value string.
  * @param rgb hdByte* The RGB color array.
  * @return 1 if the color was converted, 0 otherwise.
  */
  static int	get_color(const char *color, hdByte *rgb);

 /**
  * The <TT>get_format_spacing()</TT> method returns the
  * sum of the border, margin, and padding values.
  *
  * @param p int The spacing position: <TT>HD_POS_LEFT</TT>,
  * <TT>HD_POS_RIGHT</TT>, <TT>HD_POS_TOP</TT>, or
  * <TT>HD_POS_BOTTOM</TT>.
  * @return The spacing value in points.
  */
  float		get_format_spacing(int p)
		{
		  return (get_border(p) + get_margin(p) + get_padding(p));
		}

 /**
  * The <TT>get_length()</TT> method converts a "length" value to points.
  *
  * @param length const&nbsp;char* The length string.
  * @param max_length float The maximum value for the given axis.
  * @param css hdStyleSheet* The stylesheet.
  * @param relative int* Set to 1 if the length value is relative, 0 otherwise.
  * @return The length value in points.
  */
  float		get_length(const char *length, float max_length,
		           hdStyleSheet *css, int *relative = (int *)0);

 /**
  * The <TT>get_list_style_type()</TT> method returns the list style associated
  * with a string.
  *
  * @param value const&nbsp;char* The string value.
  * @return The list style integer value.
  */
  hdListStyleType get_list_style_type(const char *value);

 /**
  * The <TT>get_margin()</TT> method returns the margin value in points.
  *
  * @param p int The margin position: <TT>HD_POS_LEFT</TT>,
  * <TT>HD_POS_RIGHT</TT>, <TT>HD_POS_TOP</TT>, or
  * <TT>HD_POS_BOTTOM</TT>.
  * @return The margin value in points.
  */
  float		get_margin(int p)
		{
		  if (margin[p] == HD_WIDTH_AUTO)
		    return (0.0f);
		  else
		    return (margin[p]);
		}

 /**
  * The <TT>get_padding()</TT> method returns the padding value in points.
  *
  * @param p int The padding position: <TT>HD_POS_LEFT</TT>,
  * <TT>HD_POS_RIGHT</TT>, <TT>HD_POS_TOP</TT>, or
  * <TT>HD_POS_BOTTOM</TT>.
  * @return The padding value in points.
  */
  float		get_padding(int p)
		{
		  if (padding[p] == HD_WIDTH_AUTO)
		    return (0.0f);
		  else
		    return (padding[p]);
		}

 /**
  * The <TT>get_page_break()</TT> method returns the page break constant
  * associated with the string.
  *
  * @param value const&nbsp;char* The value string.
  * @return The page break constant.
  */
  hdPageBreak	get_page_break(const char *value);

 /**
  * The <TT>get_pos()</TT> method returns the position constant associated
  * with the string.
  *
  * @param name const&nbsp;char* The position string.
  * @return The position constant.
  */
  int		get_pos(const char *name);

 /**
  * The <TT>get_subvalue()</TT> method extracts a single value from a property string.
  *
  * @param valueptr char* Pointer to property string.
  * @return New string pointer after the property value.
  */
  char		*get_subvalue(char *valueptr);

 /**
  * The <TT>inherit()</TT> method inherits style data from the specified style.
  *
  * @param p hdStyle* Parent style.
  */
  void		inherit(hdStyle *p);

 /**
  * The <TT>load()</TT> method loads style data from a string.
  *
  * @param css hdStyleSheet* The stylesheet.
  * @param s const&nbsp;char* The style string.
  * @return 0 on success, -1 on error.
  */
  int		load(hdStyleSheet *css, const char *s);

 /**
  * The <TT>update()</TT> method updates all relative values in the style.
  *
  * @param css hdStyleSheet* The stylesheet.
  */
  void		update(hdStyleSheet *css);
};


//
// Stylesheet...
//

// orientation values...
enum hdOrientation
{
  HD_ORIENTATION_PORTRAIT = 0,
  HD_ORIENTATION_LANDSCAPE,
  HD_ORIENTATION_REVERSE_PORTRAIT,
  HD_ORIENTATION_REVERSE_LANDSCAPE
};

// sides values...
enum hdSides
{
  HD_SIDES_ONE_SIDED = 0,
  HD_SIDES_TWO_SIDED_LONG_EDGE,
  HD_SIDES_TWO_SIDED_SHORT_EDGE
};

struct hdTree;

/**
 * The <TT>hdStyleMedia</TT> structure describes the output media
 * attributes.
 */
struct hdStyleMedia
{
  //* Page size name
  char		size_name[64];
  //* Page width in points
  float		page_width;
  //* Page length in points
  float		page_length;
  //* Left position in points
  float		page_left;
  //* Bottom position in points
  float		page_bottom;
  //* Right position in points
  float		page_right;
  //* Top position in points
  float		page_top;
  //* Printable width in points
  float		page_print_width;
  //* Printable length in points
  float		page_print_length;
  //* Current media color
  char		media_color[64];
  //* Current media type
  char		media_type[64];
  //* Current media position
  int		media_position;
  //* Orientation of the page
  hdOrientation	orientation;
  //* Format single or double-sided?
  hdSides	sides;

 /**
  * The <TT>constructor creates a new hdStyleMedia structure.
  */
  hdStyleMedia();

 /**
  * The <TT>set_margins()</TT> method sets the page margins.
  *
  * @param l float The left margin in points.
  * @param b float The bottom margin in points.
  * @param r float The right margin in points.
  * @param t float The top margin in points.
  */
  void		set_margins(float l, float b, float r, float t);

 /**
  * The <TT>set_orientation()</TT> method sets the orientation of the page.
  *
  * @param o hdOrientation The orientation of the page:
  * <TT>HD_ORIENTATION_PORTRAIT</TT>,
  * <TT>HD_ORIENTATION_LANDSCAPE</TT>,
  * <TT>HD_ORIENTATION_REVERSE_PORTRAIT</TT>, or
  * <TT>HD_ORIENTATION_REVERSE_LANDSCAPE</TT>.
  */
  void		set_orientation(hdOrientation o);

 /**
  * The <TT>set_size()</TT> method sets the page dimensions by number.
  *
  * @param w float The width in points.
  * @param l float The length in points.
  */
  void		set_size(float w, float l);

 /**
  * The <TT>set_size()</TT> method sets the page dimensions by name.
  *
  * @param name const&nbsp;char* The size name.
  */
  void		set_size(const char *name); 

 /**
  * The <TT>update_printable()</TT> method updates the printable width and
  * length for the current dimensions and margins.
  */
  void		update_printable();
};

/**
 * The <TT>hdStyleSheet</TT> structure holds a style sheet for a
 * document.
 */
struct hdStyleSheet
{
  //* Number of styles
  int		num_styles;
  //* Allocate style slots
  int		alloc_styles;
  //* Array of styles
  hdStyle	**styles;
  //* Maximum number of selectors in styles
  int		max_selectors[HD_ELEMENT_MAX];
  //* First style for each element
  int		elements[HD_ELEMENT_MAX];
  //* Number of fonts defined
  int		num_fonts;
  //* Array of fonts
  hdStyleFont	*fonts[HD_FONTFACE_MAX][HD_FONTINTERNAL_MAX];
  //* Names of base fonts...
  char		*font_names[HD_FONTFACE_MAX];

  //* Character set
  char		*charset;
  //* Character encoding
  hdFontEncoding encoding;
  //* Number of glyphs in charset
  int		num_glyphs;
  //* Glyphs in charset
  char		**glyphs;

  //* Default media attributes from stylesheet
  hdStyleMedia	default_media;
  //* Current media attributes
  hdStyleMedia	media;

  //* Grayscale output?
  int		grayscale;
  //* Pixel resolution
  float		ppi;
  //* Private style ID
  unsigned	private_id;

 /**
  * The constructor creates a new, empty stylesheet.
  */
  hdStyleSheet();

 /**
  * The destructor frees the stylesheet including all styles and
  * fonts in it.
  */
  ~hdStyleSheet();

 /**
  * The <TT>add_style()</TT> method adds a style to the stylesheet.
  *
  * @param s hdStyle* The style to add.
  */
  void		add_style(hdStyle *s);

 /**
  * The <TT>find_font()</TT> method finds the font used by a style.
  *
  * @param s hdStyle* The style.
  * @return A pointer to the font for the style.
  */
  hdStyleFont	*find_font(hdStyle *s);

 /**
  * The <TT>find_style()</TT> method finds the matching style for the given document
  * tree node.
  *
  * @param t hdTree* The document tree node.
  * @return A pointer to the matching style, or NULL if no matching style
  * is available.
  */
  hdStyle	*find_style(hdTree *t);

 /**
  * The <TT>find_style()</TT> method finds the matching style for the given selectors.
  *
  * @param nsels int The number of selectors.
  * @param sels hdStyleSelector* The selector array.
  * @param exact int 1 if an exact match is needed, 0 for a close match.
  * @return A pointer to the matching style, or NULL if no matching style
  * is available.
  */
  hdStyle	*find_style(int nsels, hdStyleSelector *sels, int exact = 0);

 /**
  * The <TT>get_glyph()</TT> method returns the character code for the given character
  * name. Both HTML and PostScript glyph names are recognized and supported.
  *
  * @param s const&nbsp;char* The character name string.
  * @return The character code or -1 if the name is unknown.
  */
  int		get_glyph(const char *s);

 /**
  * The <TT>get_private_style()</TT> method generates a private style for the given
  * document tree node.
  *
  * @param t hdTree* The document tree node.
  * @return A pointer to a new, private style record.
  */
  hdStyle	*get_private_style(hdTree *t);

 /**
  * The <TT>load()</TT> method loads a stylesheet from a file stream.
  *
  * @param f hdFile* A pointer to the file stream.
  * @param path const&nbsp;char* A search path to be used by any included files.
  * @return 0 on success, -1 on failure.
  */
  int		load(hdFile *f, const char *path = (const char *)0);

 /**
  * The <TT>pattern()</TT> method initializes a regex character pattern that is used
  * when reading stylesheets.
  *
  * @param r const&nbsp;char* The regex character pattern.
  * @param p char[256] Initialized with true/false values for each character in
  * the pattern.
  */
  void		pattern(const char *r, char p[256]);

 /**
  * The <TT>read()</TT> method reads a single string from a file stream using the
  * specified pattern initialized by the pattern()</TT> method.
  *
  * @param f hdFile* The file stream to read from.
  * @param p const&nbsp;char* The pattern array initialized by the pattern()
  * method.
  * @param s char* The string buffer.
  * @param slen int The size of the string buffer.
  * @return A pointer to the string that was read or NULL if no string
  * could be read that matched the input pattern.
  */
  char		*read(hdFile *f, const char *p, char *s, int slen);

 /**
  * The <TT>set_charset()</TT> method sets the current character encoding to the named
  * IANA-defined character set.
  *
  * @param cs const&nbsp;char* The character set name.
  */
  void		set_charset(const char *cs);
 
 /**
  * The <TT>update_styles()</TT> method updates all of the relative style information
  * in the stylesheet.
  */
  void		update_styles();
};


#endif // !_HTMLDOC_STYLE_H_

//
// End of "$Id: style.h,v 1.20 2004/02/03 02:55:29 mike Exp $".
//
