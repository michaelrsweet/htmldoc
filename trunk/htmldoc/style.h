//
// "$Id$"
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
//       Hollywood, Maryland 20636 USA
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

#  include "types.h"


/*
 * Markup constants...
 */

enum hdElement
{
  HD_ELEMENT_FILE = -3,			/* File Delimiter */
  HD_ELEMENT_UNKNOWN = -2,		/* Unknown element */
  HD_ELEMENT_ERROR = -1,	
  HD_ELEMENT_NONE = 0,
  HD_ELEMENT_COMMENT,
  HD_ELEMENT_DOCTYPE,
  HD_ELEMENT_A,
  HD_ELEMENT_ABBR,
  HD_ELEMENT_ACRONYM,
  HD_ELEMENT_ADDRESS,
  HD_ELEMENT_APPLET,
  HD_ELEMENT_AREA,
  HD_ELEMENT_B,
  HD_ELEMENT_BASE,
  HD_ELEMENT_BASEFONT,
  HD_ELEMENT_BDO,
  HD_ELEMENT_BIG,
  HD_ELEMENT_BLINK,
  HD_ELEMENT_BLOCKQUOTE,
  HD_ELEMENT_BODY,
  HD_ELEMENT_BR,
  HD_ELEMENT_BUTTON,
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
  HD_ELEMENT_FIELDSET,
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
  HD_ELEMENT_H7,
  HD_ELEMENT_H8,
  HD_ELEMENT_H9,
  HD_ELEMENT_H10,
  HD_ELEMENT_H11,
  HD_ELEMENT_H12,
  HD_ELEMENT_H13,
  HD_ELEMENT_H14,
  HD_ELEMENT_H15,
  HD_ELEMENT_HEAD,
  HD_ELEMENT_HR,
  HD_ELEMENT_HTML,
  HD_ELEMENT_I,
  HD_ELEMENT_IFRAME,
  HD_ELEMENT_IMG,
  HD_ELEMENT_INPUT,
  HD_ELEMENT_INS,
  HD_ELEMENT_ISINDEX,
  HD_ELEMENT_KBD,
  HD_ELEMENT_LABEL,
  HD_ELEMENT_LEGEND,
  HD_ELEMENT_LI,
  HD_ELEMENT_LINK,
  HD_ELEMENT_MAP,
  HD_ELEMENT_MENU,
  HD_ELEMENT_META,
  HD_ELEMENT_MULTICOL,
  HD_ELEMENT_NOBR,
  HD_ELEMENT_NOFRAMES,
  HD_ELEMENT_OBJECT,
  HD_ELEMENT_OL,
  HD_ELEMENT_OPTGROUP,
  HD_ELEMENT_OPTION,
  HD_ELEMENT_P,
  HD_ELEMENT_PARAM,
  HD_ELEMENT_PRE,
  HD_ELEMENT_Q,
  HD_ELEMENT_S,
  HD_ELEMENT_SAMP,
  HD_ELEMENT_SCRIPT,
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
// Stylesheet attribute values...
//

// "Auto" values
#define HD_HEIGHT_AUTO			-65536.0f
#define HD_MARGIN_AUTO			-65536.0f
#define HD_WIDTH_AUTO			-65536.0f

// "Inherit" values
#define HD_BORDER_WIDTH_INHERIT		65536.0f
#define HD_FONT_SIZE_INHERIT		65536.0f
#define HD_HEIGHT_INHERIT		65536.0f
#define HD_LETTER_SPACING_INHERIT	65536.0f
#define HD_LINE_HEIGHT_INHERIT		65536.0f
#define HD_MARGIN_INHERIT		65536.0f
#define HD_ORPHANS_INHERIT		65536
#define HD_PADDING_INHERIT		65536.0f
#define HD_TEXT_INDENT_INHERIT		65536.0f
#define HD_WIDOWS_INHERIT		65536
#define HD_WIDTH_INHERIT		65536.0f
#define HD_WORD_SPACING_INHERIT		65536.0f

// Indices for individual margins, padding, etc.
enum hdPos
{
  HD_POS_TOP = 0,
  HD_POS_RIGHT,
  HD_POS_BOTTOM,
  HD_POS_LEFT
};

// background-repeat values...
enum hdBackgroundRepeat
{
  HD_BACKGROUND_REPEAT_INHERIT,
  HD_BACKGROUND_REPEAT_REPEAT,
  HD_BACKGROUND_REPEAT_REPEAT_X,
  HD_BACKGROUND_REPEAT_REPEAT_Y,
  HD_BACKGROUND_REPEAT_NO_REPEAT
};

// border-style values...
enum hdBorderStyle
{
  HD_BORDER_STYLE_INHERIT = 0,
  HD_BORDER_STYLE_NONE,
  HD_BORDER_STYLE_DOTTED,
  HD_BORDER_STYLE_DASHED,
  HD_BORDER_STYLE_SOLID,
  HD_BORDER_STYLE_DOUBLE,
  HD_BORDER_STYLE_GROOVE,
  HD_BORDER_STYLE_RIDGE,
  HD_BORDER_STYLE_INSET,
  HD_BORDER_STYLE_OUTSET
};

// caption-side values...
enum hdCaptionSide
{
  HD_CAPTION_SIDE_TOP = 0,
  HD_CAPTION_SIDE_BOTTOM
};

// clear values...
enum hdClear
{
  HD_CLEAR_INHERIT = 0,
  HD_CLEAR_NONE,
  HD_CLEAR_LEFT,
  HD_CLEAR_RIGHT,
  HD_CLEAR_BOTH
};

// *color values...
enum hdColor
{
  HD_COLOR_INHERIT = 0,
  HD_COLOR_TRANSPARENT,
  HD_COLOR_SET,
  HD_COLOR_UNDEFINED
};

// direction values...
enum hdDirection
{
  HD_DIRECTION_INHERIT = 0,
  HD_DIRECTION_LTR,
  HD_DIRECTION_RTL
};

// display values...
enum hdDisplay
{
  HD_DISPLAY_INHERIT = 0,
  HD_DISPLAY_NONE,
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
  HD_FLOAT_INHERIT = 0,
  HD_FLOAT_NONE,
  HD_FLOAT_LEFT,
  HD_FLOAT_RIGHT
};

// font-style values...
enum hdFontStyle
{
  HD_FONT_STYLE_INHERIT = 0,
  HD_FONT_STYLE_NORMAL,
  HD_FONT_STYLE_ITALIC,
  HD_FONT_STYLE_OBLIQUE
};

// font-variant values...
enum hdFontVariant
{
  HD_FONT_VARIANT_INHERIT = 0,
  HD_FONT_VARIANT_NORMAL,
  HD_FONT_VARIANT_SMALL_CAPS
};

// font-weight values...
enum hdFontWeight
{
  HD_FONT_WEIGHT_INHERIT = 0,
  HD_FONT_WEIGHT_NORMAL,
  HD_FONT_WEIGHT_BOLD
};

// list-style-position values...
enum hdListStylePosition
{
  HD_LIST_STYLE_POSITION_INHERIT = 0,
  HD_LIST_STYLE_POSITION_INSIDE,
  HD_LIST_STYLE_POSITION_OUTSIDE
};

// list-style-type values...
enum hdListStyleType
{
  HD_LIST_STYLE_TYPE_INHERIT = 0,
  HD_LIST_STYLE_TYPE_NONE,
  HD_LIST_STYLE_TYPE_DISC,
  HD_LIST_STYLE_TYPE_CIRCLE,
  HD_LIST_STYLE_TYPE_SQUARE,
  HD_LIST_STYLE_TYPE_DECIMAL,
  HD_LIST_STYLE_TYPE_LOWER_ROMAN,
  HD_LIST_STYLE_TYPE_UPPER_ROMAN,
  HD_LIST_STYLE_TYPE_LOWER_ALPHA,
  HD_LIST_STYLE_TYPE_UPPER_ALPHA
};

// page-break values...
enum hdPageBreak
{
  HD_PAGE_BREAK_INHERIT = 0,
  HD_PAGE_BREAK_AUTO,
  HD_PAGE_BREAK_ALWAYS,
  HD_PAGE_BREAK_AVOID,
  HD_PAGE_BREAK_LEFT,
  HD_PAGE_BREAK_RIGHT
};

// text-align values...
enum hdTextAlign
{
  HD_TEXT_ALIGN_INHERIT = 0,
  HD_TEXT_ALIGN_LEFT,
  HD_TEXT_ALIGN_CENTER,
  HD_TEXT_ALIGN_RIGHT,
  HD_TEXT_ALIGN_JUSTIFY
};

// text-decoration values...
enum hdTextDecoration
{
  HD_TEXT_DECORATION_INHERIT = 0,
  HD_TEXT_DECORATION_NONE,
  HD_TEXT_DECORATION_UNDERLINE,
  HD_TEXT_DECORATION_OVERLINE,
  HD_TEXT_DECORATION_LINE_THROUGH
};

// text-transform values...
enum hdTextTransform
{
  HD_TEXT_TRANSFORM_INHERIT = 0,
  HD_TEXT_TRANSFORM_NONE,
  HD_TEXT_TRANSFORM_CAPITALIZE,
  HD_TEXT_TRANSFORM_UPPERCASE,
  HD_TEXT_TRANSFORM_LOWERCASE
};

// unicode-bidi values...
enum hdUnicodeBidi
{
  HD_UNICODE_BIDI_INHERIT = 0,
  HD_UNICODE_BIDI_NORMAL,
  HD_UNICODE_BIDI_EMBED,
  HD_UNICODE_BIDI_BIDI_OVERRIDE
};

// vertical-align values...
enum hdVerticalAlign
{
  HD_VERTICAL_ALIGN_INHERIT = 0,
  HD_VERTICAL_ALIGN_BASELINE,
  HD_VERTICAL_ALIGN_SUB,
  HD_VERTICAL_ALIGN_SUPER,
  HD_VERTICAL_ALIGN_TOP,
  HD_VERTICAL_ALIGN_TEXT_TOP,
  HD_VERTICAL_ALIGN_MIDDLE,
  HD_VERTICAL_ALIGN_BOTTOM,
  HD_VERTICAL_ALIGN_TEXT_BOTTOM
};

// white-space values...
enum hdWhiteSpace
{
  HD_WHITE_SPACE_INHERIT = 0,
  HD_WHITE_SPACE_NORMAL,
  HD_WHITE_SPACE_NOWRAP,
  HD_WHITE_SPACE_PRE,
  HD_WHITE_SPACE_PRE_WRAP,
  HD_WHITE_SPACE_PRE_LINE
};


//
// Font constants...
//

// Standard font face values...
enum hdFontFace
{
  HD_FONT_FACE_COURIER = 0,
  HD_FONT_FACE_TIMES,
  HD_FONT_FACE_HELVETICA,
  HD_FONT_FACE_MONOSPACE,
  HD_FONT_FACE_SERIF,
  HD_FONT_FACE_SANS_SERIF,
  HD_FONT_FACE_SYMBOL,
  HD_FONT_FACE_DINGBATS,
  HD_FONT_FACE_CURSIVE,
  HD_FONT_FACE_CUSTOM,
  HD_FONT_FACE_MAX = 16
};

// Standard font style values...
enum hdFontInternal
{
  HD_FONT_INTERNAL_NORMAL = 0,
  HD_FONT_INTERNAL_BOLD,
  HD_FONT_INTERNAL_ITALIC,
  HD_FONT_INTERNAL_BOLD_ITALIC,
  HD_FONT_INTERNAL_MAX
};

// Standard font encoding values...
enum hdFontEncoding
{
  HD_FONT_ENCODING_8BIT = 0,
  HD_FONT_ENCODING_UTF8
};


/**
 * The <tt>hdFontKernPair</tt> structure contains character kerning
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
 * The <tt>hdStyleFont</tt> structure describes a single font that is used
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
  //* Highest point in font.
  float		ascender;
  //* Bounding box
  float		bbox[4];
  //* Height of uppercase letters.
  float		cap_height;
  //* Lowest point in font.
  float		descender;
  //* Angle for italics.
  float		italic_angle;
  //* Offset for underline.
  float		ul_position;
  //* Thickness for underline.
  float		ul_thickness;
  //* Height of lowercase letters.
  float		x_height;
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
  * @param css hdStyleSheet* A pointer to the style sheet.
  * @param t hdFontFace The font typeface index.
  * @param s hdFontInternal The font style index.
  * @param n const&nbsp;char* The PostScript name of the font.
  */
  hdStyleFont(hdStyleSheet *css, hdFontFace t, hdFontInternal s, const char *n);

 /**
  * The destructor free all memory associated with the font.
  */
  ~hdStyleFont();

  static int	compare_kerns(hdFontKernPair *a, hdFontKernPair *b);

 /**
  * The <tt>get_char()</tt> method returns the next character from the specified
  * string and updates the string pointer to the next character position.
  * This method handles multi-byte and single-byte encodings transparently
  * to the caller.
  *
  * @param s const&nbsp;char* A pointer into a string.
  * @return The character value.
  */
  int		get_char(const hdChar *&s);

 /**
  * The <tt>get_kerning()</tt> method generates an array of kerning values
  * as well as a total kerning adjustment for the specified string.
  * 
  * @param s const&nbsp;char* The string to kern.
  * @param tk float Variable to hold total kerning value.
  * @param kl float* Pointer for kerning array for each character.
  * @return The number of kerning entries. Normally 1 less
  * then the total number of chracters in the input string.
  */
  int		get_kerning(const hdChar *s, float &tk, float *&kl);

 /**
  * The <tt>get_num_chars()</tt> method returns the actual number of
  * characters in the given string and handles multi-byte and
  * single-byte encodings transparently to the caller.
  *
  * @param s const&nbsp;char* A pointer to the string.
  * @return The number of characters in the string.
  */
  int		get_num_chars(const hdChar *s);

 /**
  * The <tt>get_width()</tt> method returns the width of a string, including
  * adjustments for kerning. The width value is scaled for a point
  * size of 1.0.
  *
  * @param s const&nbsp;char* A pointer to the string.
  * @return The width of the string.
  */
  float		get_width(const hdChar *s);

 /**
  * The <tt>read_afm()</tt> method loads font widths from an AFM file.
  *
  * @param fp FILE* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_afm(FILE *fp, hdStyleSheet *css);

 /**
  * The <tt>read_pfm()</tt> method loads font widths from a PFM file.
  *
  * @param fp FILE* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_pfm(FILE *fp, hdStyleSheet *css);

 /**
  * The <tt>read_ttf()</tt> method loads font widths from a TTF file.
  *
  * @param fp FILE* The file to read from.
  * @param css hdStyleSheet* The stylesheet.
  * @return 0 on success, -1 on error.
  */
  int		read_ttf(FILE *fp, hdStyleSheet *css);
};

/**
 * The <tt>hdBorder</tt> structure holds border attribute information.
 */
struct hdBorder
{
  //* Color of border
  unsigned char		color[3];
  //* Is the color set?
  hdColor		color_set;
  //* Rendering style of border
  hdBorderStyle		style;
  //* Width of border in points
  float			width;
  //* Relative width of border
  char			*width_rel;
};

// Maximum number of selectors per style...
#define HD_SELECTOR_MAX	100

/**
 * The <tt>hdStyleSelector</tt> structure is used to select specific styles
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
  * The constructor creates a new, blank selector.
  */
  hdStyleSelector();

 /**
  * The <tt>set()</tt> method sets the selector values.
  *
  * @param e hdElement The HTML element.
  * @param c const&nbsp;char* The HTML CLASS attribute.
  * @param p const&nbsp;char* The HTML pseudo-class attribute (link, visited, etc.)
  * @param i const&nbsp;char* The HTML ID attribute.
  */
  void	set(hdElement e, const char *c, const char *p, const char *i);

 /**
  * The <tt>clear()</tt> method resets the selector values, freeing memory
  * as needed.
  */
  void	clear();
};

/**
 * The <tt>hdStyle</tt> structure stores all of the supported style
 * information for a single style in a stylesheet.
 */
struct hdStyle
{
  //* True if relative attributes have been updated in the style record.
  bool			updated;
  //* Number of selectors for the style record.
  int			num_selectors;
  //* Selectors for the style record.
  hdStyleSelector	*selectors;

  //* The <tt>background-color</tt> value.
  hdByte		background_color[3];
  //* True if the <tt>background-color</tt> value is set for this style.
  hdColor		background_color_set;
  //* The <tt>background-image</tt> value.
  char			*background_image;
  //* The <tt>background-position</tt> values.
  float			background_position[2];
  //* The relative <tt>background-position</tt> values, if any.
  char			*background_position_rel[2];
  //* The <tt>background-repeat</tt> value.
  hdBackgroundRepeat	background_repeat;
  //* The <tt>border-left</tt>, <tt>border-right</tt>, <tt>border-top</tt>,
  //* and <tt>border-bottom</tt> values.
  hdBorder		border[4];
  //* The <tt>caption-side</tt> value.
  hdCaptionSide		caption_side;
  //* The <tt>clear</tt> value.
  hdClear		clear;
  //* The <tt>color</tt> value.
  unsigned char		color[3];
  //* True if the <tt>color</tt> value is set for this style.
  hdColor		color_set;
  //* The <tt>direction</tt> value.
  hdDirection		direction;
  //* The <tt>display</tt> value.
  hdDisplay		display;
  //* The <tt>float</tt> value.
  hdFloat		float_;
  //* The <tt>font associated with this style.
  hdStyleFont		*font;
  //* The <tt>font-family</tt> value.
  char			*font_family;
  //* The <tt>font-size</tt> value.
  float			font_size;
  //* The relative <tt>font-size</tt> value, if any.
  char			*font_size_rel;
  //* The <tt>font-style</tt> value.
  hdFontStyle		font_style;
  //* The <tt>font-variant</tt> value.
  hdFontVariant		font_variant;
  //* The <tt>font-weight</tt> value.
  hdFontWeight		font_weight;
  //* The <tt>height</tt> value.
  float			height;
  //* The relative <tt>height</tt> value, if any.
  char			*height_rel;
  //* The <tt>letter-spacing</tt> value.
  float			letter_spacing;
  //* The <tt>line-height</tt> value.
  float			line_height;
  //* The relative <tt>line-height</tt> value, if any.
  char			*line_height_rel;
  //* The <tt>list-style-image</tt> value.
  char			*list_style_image;
  //* The <tt>list-style-position</tt> value.
  hdListStylePosition	list_style_position;
  //* The <tt>list-style-type</tt> value.
  hdListStyleType	list_style_type;
  //* The <tt>margin-left</tt>, <tt>margin-right</tt>, <tt>margin-top</tt>,
  //* and <tt>margin-bottom</tt> values.
  float			margin[4];
  //* The relative <tt>margin-left</tt>, <tt>margin-right</tt>,
  //* <tt>margin-top</tt>, and <tt>margin-bottom</tt> values, if any.
  char			*margin_rel[4];
  //* The <tt>orphans</tt> value.
  int			orphans;
  //* The <tt>padding-left</tt>, <tt>padding-right</tt>, <tt>padding-top</tt>,
  //* and <tt>padding-bottom</tt> values.
  float			padding[4];
  //* The relative <tt>padding-left</tt>, <tt>padding-right</tt>,
  //* <tt>padding-top</tt>, and <tt>padding-bottom values</tt>, if any.
  char			*padding_rel[4];
  //* The <tt>page-break-after</tt> value.
  hdPageBreak		page_break_after;
  //* The <tt>page-break-before</tt> value.
  hdPageBreak		page_break_before;
  //* The <tt>page-break-inside</tt> value.
  hdPageBreak		page_break_inside;
  //* The <tt>text-align</tt> value.
  hdTextAlign		text_align;
  //* The <tt>text-decoration</tt> value.
  hdTextDecoration	text_decoration;
  //* The <tt>text-indent</tt> value.
  float			text_indent;
  //* The relative <tt>text-indent</tt> value, if any.
  char			*text_indent_rel;
  //* The <tt>text-transform</tt> value.
  hdTextTransform	text_transform;
  //* The <tt>unicode-bidi</tt> value.
  hdUnicodeBidi		unicode_bidi;
  //* The <tt>vertical-align</tt> value.
  hdVerticalAlign	vertical_align;
  //* The <tt>white-space</tt> value.
  hdWhiteSpace		white_space;
  //* The <tt>widows</tt> value.
  int			widows;
  //* The <tt>width</tt> value.
  float			width;
  //* The relative <tt>width</tt> value, if any.
  char			*width_rel;
  //* The <tt>word-spacing</tt> value.
  float			word_spacing;

 /**
  * The constructor creates a new empty style record.
  */
  hdStyle();

 /**
  * The constructor creates a new style record.
  *
  * @param nsels int The number of selectors for the style.
  * @param sels hdStyleSelector* The selectors for the style.
  * @param o hdStyle* The original style.
  */
  hdStyle(int nsels, hdStyleSelector *sels, hdStyle *o = (hdStyle *)0);

 /**
  * The destructor free all memory associated with the style record.
  */
  ~hdStyle();

 /**
  * The <tt>copy()</tt> method copies style data from the specified style.
  *
  * @param o hdStyle* Original style.
  */
  void		copy(hdStyle *o);

 /**
  * The <tt>get_border()</tt> method returns the width in points of the specified
  * border.
  *
  * @param p int The border position: <tt>HD_POS_LEFT</tt>,
  * <tt>HD_POS_RIGHT</tt>, <tt>HD_POS_TOP</tt>, or
  * <tt>HD_POS_BOTTOM</tt>.
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
  * The <tt>get_border_style()</tt> method returns the border style associated
  * with the specified string value.
  *
  * @param value const&nbsp;char* The string to be converted.
  * @return The border style associated with the string.
  */
  hdBorderStyle	get_border_style(const char *value);

 /**
  * The <tt>get_border_width()</tt> method returns the width in points for the
  * given string value.
  *
  * @param value const&nbsp;char* The border value string.
  * @param css hdStyleSheet* The stylesheet.
  * @return The width in points.
  */
  float		get_border_width(const char *value, hdStyleSheet *css);

 /**
  * The <tt>get_color()</tt> method converts the string color to a 24-bit
  * RGB value.
  *
  * @param value const&nbsp;char* The color value string.
  * @param rgb hdByte* The RGB color array.
  * @param set hdColor* The type of color.
  * @return True if the color was converted, false otherwise.
  */
  static bool	get_color(const char *color, hdByte *rgb, hdColor *set = (hdColor *)0);

 /**
  * The <tt>get_color()</tt> method converts the string color to a floating
  * point RGB value.
  *
  * @param value const&nbsp;char* The color value string.
  * @param rgb float* The RGB color array.
  * @return True if the color was converted, false otherwise.
  */
  static bool	get_color(const char *color, float *rgb);

 /**
  * The <tt>get_format_spacing()</tt> method returns the
  * sum of the border, margin, and padding values.
  *
  * @param p int The spacing position: <tt>HD_POS_LEFT</tt>,
  * <tt>HD_POS_RIGHT</tt>, <tt>HD_POS_TOP</tt>, or
  * <tt>HD_POS_BOTTOM</tt>.
  * @return The spacing value in points.
  */
  float		get_format_spacing(int p)
		{
		  return (get_border(p) + get_margin(p) + get_padding(p));
		}

 /**
  * The <tt>get_length()</tt> method converts a "length" value to points.
  *
  * @param length const&nbsp;char* The length string.
  * @param max_length float The maximum value for the given axis.
  * @param def_units float The default units/multiplier.
  * @param css hdStyleSheet* The stylesheet.
  * @param relative int* Set to 1 if the length value is relative, 0 otherwise.
  * @return The length value in points.
  */
  float		get_length(const char *length, float max_length,
                           float def_units, hdStyleSheet *css,
			   bool *relative = (bool *)0);

 /**
  * The <tt>get_list_style_type()</tt> method returns the list style associated
  * with a string.
  *
  * @param value const&nbsp;char* The string value.
  * @return The list style integer value.
  */
  hdListStyleType get_list_style_type(const char *value);

 /**
  * The <tt>get_margin()</tt> method returns the margin value in points.
  *
  * @param p int The margin position: <tt>HD_POS_LEFT</tt>,
  * <tt>HD_POS_RIGHT</tt>, <tt>HD_POS_TOP</tt>, or
  * <tt>HD_POS_BOTTOM</tt>.
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
  * The <tt>get_padding()</tt> method returns the padding value in points.
  *
  * @param p int The padding position: <tt>HD_POS_LEFT</tt>,
  * <tt>HD_POS_RIGHT</tt>, <tt>HD_POS_TOP</tt>, or
  * <tt>HD_POS_BOTTOM</tt>.
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
  * The <tt>get_page_break()</tt> method returns the page break constant
  * associated with the string.
  *
  * @param value const&nbsp;char* The value string.
  * @return The page break constant.
  */
  hdPageBreak	get_page_break(const char *value);

 /**
  * The <tt>get_pos()</tt> method returns the position constant associated
  * with the string.
  *
  * @param name const&nbsp;char* The position string.
  * @return The position constant.
  */
  int		get_pos(const char *name);

 /**
  * The <tt>get_subvalue()</tt> method extracts a single value from a property string.
  *
  * @param valueptr char* Pointer to property string.
  * @return New string pointer after the property value.
  */
  char		*get_subvalue(char *valueptr);

 /**
  * The <tt>get_width()</tt> method computes the width of a string using
  * the current style.
  *
  * @param s const&nbsp;char* Pointer to string.
  * @return String width in points.
  */
  float		get_width(const hdChar *s);

 /**
  * The <tt>inherit()</tt> method inherits style data from the specified style.
  *
  * @param p hdStyle* Parent style.
  */
  void		inherit(hdStyle *p);

 /**
  * The <tt>init()</tt> method initializes the style data to the defaults.
  */
  void		init();

 /**
  * The <tt>load()</tt> method loads style data from a string.
  *
  * @param css hdStyleSheet* The stylesheet.
  * @param s const&nbsp;char* The style string.
  * @return True on success, false on error.
  */
  bool		load(hdStyleSheet *css, const char *s);

 /**
  * The <tt>set_font_size()</tt> method sets the font size.
  *
  * @param s const&nbsp;char* The font size.
  * @param css hdStyleSheet* The stylesheet.
  */
  void		set_font_size(const char *s, hdStyleSheet *css);

 /**
  * The <tt>set_line_height()</tt> method sets the line height.
  *
  * @param lh const&nbsp;char* The line height.
  * @param css hdStyleSheet* The stylesheet.
  */
  void		set_line_height(const char *lh, hdStyleSheet *css);

 /**
  * The <tt>set_string()</tt> method copies and sets a string value,
  * freeing any old value as needed.
  */
  void		set_string(const char *s, char *&var);

 /**
  * The <tt>update()</tt> method updates all relative values in the style.
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
 * The <tt>hdStyleMedia</tt> structure describes the output media
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
  * The constructor creates a new hdStyleMedia structure.
  */
  hdStyleMedia();

 /**
  * The <tt>set_margins()</tt> method sets the page margins.
  *
  * @param l float The left margin in points.
  * @param b float The bottom margin in points.
  * @param r float The right margin in points.
  * @param t float The top margin in points.
  */
  void		set_margins(float l, float b, float r, float t);

 /**
  * The <tt>set_orientation()</tt> method sets the orientation of the page.
  *
  * @param o hdOrientation The orientation of the page:
  * <tt>HD_ORIENTATION_PORTRAIT</tt>,
  * <tt>HD_ORIENTATION_LANDSCAPE</tt>,
  * <tt>HD_ORIENTATION_REVERSE_PORTRAIT</tt>, or
  * <tt>HD_ORIENTATION_REVERSE_LANDSCAPE</tt>.
  */
  void		set_orientation(hdOrientation o);

 /**
  * The <tt>set_size()</tt> method sets the page dimensions by number.
  *
  * @param w float The width in points.
  * @param l float The length in points.
  */
  void		set_size(float w, float l);

 /**
  * The <tt>set_size()</tt> method sets the page dimensions by name.
  *
  * @param name const&nbsp;char* The size name.
  */
  void		set_size(const char *name); 

 /**
  * The <tt>update_printable()</tt> method updates the printable width and
  * length for the current dimensions and margins.
  */
  void		update_printable();
};

/**
 * The <tt>hdStyleSheet</tt> structure holds a style sheet for a
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
  //* Default style info
  hdStyle	def_style;
  //* Maximum number of selectors in styles
  int		max_selectors[HD_ELEMENT_MAX];
  //* First style for each element
  int		elements[HD_ELEMENT_MAX];
  //* Number of fonts defined
  int		num_fonts;
  //* Array of fonts
  hdStyleFont	*fonts[HD_FONT_FACE_MAX][HD_FONT_INTERNAL_MAX];
  //* Names of base fonts...
  char		*font_names[HD_FONT_FACE_MAX];

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
  bool		grayscale;
  //* Pixel resolution
  float		ppi;
  //* Browser width
  float		browser_width;
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
  * The <tt>add_style()</tt> method adds a style to the stylesheet.
  *
  * @param s hdStyle* The style to add.
  */
  void		add_style(hdStyle *s);

 /**
  * The <tt>find_font()</tt> method finds the font used by a style.
  *
  * @param s hdStyle* The style.
  * @return A pointer to the font for the style.
  */
  hdStyleFont	*find_font(hdStyle *s);

 /**
  * The <tt>find_font()</tt> method finds a specific font.
  *
  * @param face const&nbsp;char* The font face.
  * @param fs hdFontInternal The internal font style
  * @return A pointer to the font.
  */
  hdStyleFont	*find_font(const char *family, hdFontInternal fs);

 /**
  * The <tt>find_style()</tt> method finds the matching style for the given document
  * tree node.
  *
  * @param t hdTree* The document tree node.
  * @return A pointer to the matching style, or NULL if no matching style
  * is available.
  */
  hdStyle	*find_style(hdTree *t);

 /**
  * The <tt>find_style()</tt> method finds the matching style for the given selectors.
  *
  * @param nsels int The number of selectors.
  * @param sels hdStyleSelector* The selector array.
  * @param exact int 1 if an exact match is needed, 0 for a close match.
  * @return A pointer to the matching style, or NULL if no matching style
  * is available.
  */
  hdStyle	*find_style(int nsels, hdStyleSelector *sels, int exact = 0);

 /**
  * The <tt>find_style()</tt> method finds the matching style for the given element.
  *
  * @param e hdElement The element to find.
  * @return A pointer to the matching style, or NULL if no matching style
  * is available.
  */
  hdStyle	*find_style(hdElement e, const char *c = 0,
		            const char *i = 0, const char *p = 0);

 /**
  * The <tt>get_element()</tt> method returns the enumeration for the
  * given element.
  *
  * @param s const&nbsp;char* The element name string.
  * @return The enumeration value.
  */
  hdElement	get_element(const char *s);

 /**
  * The <tt>get_element()</tt> method returns the enumeration for the
  * given element.
  *
  * @param e The enumeration value.
  * @return const&nbsp;char* The element name string.
  */
  const char	*get_element(hdElement e);

 /**
  * The <tt>get_glyph()</tt> method returns the character code for the given character
  * name. Both HTML and PostScript glyph names are recognized and supported.
  *
  * @param s const&nbsp;char* The character name string.
  * @return The character code or -1 if the name is unknown.
  */
  int		get_glyph(const char *s);

 /**
  * The <tt>get_private_style()</tt> method generates a private style for the given
  * document tree node.
  *
  * @param t hdTree* The document tree node.
  * @return A pointer to a new, private style record.
  */
  hdStyle	*get_private_style(hdTree *t, bool force = false);

 /**
  * The <tt>load()</tt> method loads a stylesheet from a file stream.
  *
  * @param f FILE* A pointer to the file stream.
  * @param path const&nbsp;char* A search path to be used by any included files.
  * @return True on success, false on failure.
  */
  bool		load(FILE *f, const char *path = (const char *)0);

 /**
  * The <tt>pattern()</tt> method initializes a regex character pattern that is used
  * when reading stylesheets.
  *
  * @param r const&nbsp;char* The regex character pattern.
  * @param p char[256] Initialized with true/false values for each character in
  * the pattern.
  */
  void		pattern(const char *r, char p[256]);

 /**
  * The <tt>read()</tt> method reads a single string from a file stream using the
  * specified pattern initialized by the pattern()</tt> method.
  *
  * @param f FILE* The file stream to read from.
  * @param p const&nbsp;char* The pattern array initialized by the pattern()
  * method.
  * @param s char* The string buffer.
  * @param slen int The size of the string buffer.
  * @return A pointer to the string that was read or NULL if no string
  * could be read that matched the input pattern.
  */
  char		*read(FILE *f, const char *p, char *s, int slen);

 /**
  * The <tt>set_charset()</tt> method sets the current character encoding to the named
  * IANA-defined character set.
  *
  * @param cs const&nbsp;char* The character set name.
  */
  void		set_charset(const char *cs);

 /**
  * The <tt>set_color()</tt> method sets the default color.
  *
  * @param color const&nbsp;char* The default color.
  */
  void		set_color(const char *color) { def_style.get_color(color, def_style.color, &(def_style.color_set)); }

 /**
  * The <tt>set_font_family()</tt> method sets the default font family.
  *
  * @param f const&nbsp;char* The default font damily.
  */
  void		set_font_family(const char *f) { def_style.set_string(f, def_style.font_family); }

 /**
  * The <tt>set_font_size()</tt> method sets the default font size.
  *
  * @param s const&nbsp;char* The default font size.
  */
  void		set_font_size(const char *s) { def_style.set_font_size(s, this); }

 /**
  * The <tt>set_line_height()</tt> method sets the default line height.
  *
  * @param lh const&nbsp;char* The default line height.
  */
  void		set_line_height(const char *lh) { def_style.set_line_height(lh, this); }

 /**
  * The <tt>update_styles()</tt> method updates all of the relative style
  * information in the stylesheet.
  *
  * @param force bool Force updating of all styles?
  */
  void		update_styles(bool force = false);
};


#endif // !_HTMLDOC_STYLE_H_

//
// End of "$Id$".
//
