//
// "$Id$"
//
// CSS style routines for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2009 by Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//
// Contents:
//
//   hdStyleSelector::hdStyleSelector()  - Initialize a selector with the given
//                                         values.
//   hdStyleSelector::~hdStyleSelector() - Free a selector.
//   hdStyleSelector::clear()            - Free selector strings.
//   hdStyleSelector::set()              - Set selector values.
//   hdStyle::hdStyle()                  - Create a new style record.
//   hdStyle::~hdStyle()                 - Destroy a style record.
//   hdStyle::copy()                     - Copy style properties from an
//                                         original style.
//   hdStyle::get_border_style()         - Get a border style value.
//   hdStyle::get_border_width()         - Get a border width value.
//   hdStyle::get_color()                - Get a floating point color value.
//   hdStyle::get_length()               - Get a length/measurement value...
//   hdStyle::get_list_style_type()      - Get a list style type value.
//   hdStyle::get_page_break()           - Get a page break value.
//   hdStyle::get_pos()                  - Get a margin/position/padding/border
//                                         index.
//   hdStyle::get_subvalue()             - Get a subvalue from a property value.
//   hdStyle::get_width()                - Get width of string in points.
//   hdStyle::inherit()                  - Inherit style properties from a
//                                         parent style.
//   hdStyle::init()                     - Initialize the style data to
//                                         defaults.
//   hdStyle::load()                     - Load a style definition from a
//                                         string.
//   hdStyle::set_font_size()            - Set the font size.
//   hdStyle::set_line_height()          - Set the line height.
//   hdStyle::set_string()               - Copy and set a string value.
//   hdStyle::update()                   - Update relative style definitions.
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include "hdstring.h"
#include <stdlib.h>


//
// 'hdStyleSelector::hdStyleSelector()' - Initialize a blank selector.
//

hdStyleSelector::hdStyleSelector()
{
  // Clear the whole structure to 0...
  memset(this, 0, sizeof(hdStyleSelector));
}


//
// 'hdStyleSelector::hdStyleSelector()' - Initialize a selector with the given values.
//

hdStyleSelector::hdStyleSelector(
    hdElement  e,			// I - Element
    const char *c,			// I - Class string
    const char *p,			// I - Pseudo-selector string
    const char *i)			// I - ID string)
{
  // Clear the whole structure to 0...
  memset(this, 0, sizeof(hdStyleSelector));

  // Set the values...
  set(e, c, p, i);
}


//
// 'hdStyleSelector::~hdStyleSelector()' - Free a selector.
//

hdStyleSelector::~hdStyleSelector()
{
  clear();
}


//
// 'hdStyleSelector::clear()' - Free selector strings.
//

void
hdStyleSelector::clear()
{
  if (class_)
  {
    free(class_);
    class_ = NULL;
  }

  if (pseudo)
  {
    free(pseudo);
    pseudo = NULL;
  }

  if (id)
  {
    free(id);
    id = NULL;
  }
}


//
// 'hdStyleSelector::set()' - Set selector values.
//

void
hdStyleSelector::set(hdElement  e,	// I - Element
                     const char *c,	// I - Class string
                     const char *p,	// I - Pseudo-selector string
		     const char *i)	// I - ID string
{
  // Set/allocate the new values...
  element = e;

  if (c)
    class_ = strdup(c);
  else
    class_ = NULL;

  if (p)
    pseudo = strdup(p);
  else
    pseudo = NULL;

  if (i)
    id = strdup(i);
  else
    id = NULL;
}


//
// 'hdStyle::hdStyle()' - Create an empty style record.
//

hdStyle::hdStyle()
{
  // Initialize everything to defaults...
  init();

  // Setup a dummy selector...
  num_selectors = 1;
  selectors     = new hdStyleSelector[1];

  selectors[0].set(HD_ELEMENT_BODY, NULL, NULL, NULL);
}


//
// 'hdStyle::hdStyle()' - Create a new style record.
//

hdStyle::hdStyle(int             nsels,	// I - Number of selectors
		 hdStyleSelector *sels,	// I - Selectors
                 hdStyle         *p)	// I - Parent style
{
  int	i;				// Looping var


  // Initialize everything to defaults...
  init();

  // Copy the selectors.  The selector strings are allocated by
  // the caller, but are freed by the destructor...
  num_selectors = nsels;
  selectors     = new hdStyleSelector[nsels];

  for (i = 0; i < nsels; i ++, sels ++)
    selectors->set(sels->element, sels->class_, sels->pseudo, sels->id);

  // Now copy the parent attributes...
  copy(p);
}


//
// 'hdStyle::~hdStyle()' - Destroy a style record.
//

hdStyle::~hdStyle()
{
  int	i;				// Looping var


  // Free the selectors as needed...
  if (selectors)
    delete[] selectors;

  // Then free any string values that we have...
  if (background_image)
    free(background_image);

  for (i = 0; i < 2; i ++)
    if (background_position_rel[i])
      free(background_position_rel[i]);

  for (i = 0; i < 4; i ++)
    if (border[i].width_rel)
      free(border[i].width_rel);

  if (font_family)
    free(font_family);

  if (font_size_rel)
    free(font_size_rel);

  if (height_rel)
    free(height_rel);

  if (line_height_rel)
    free(line_height_rel);

  if (list_style_image)
    free(list_style_image);

  for (i = 0; i < 4; i ++)
  {
    if (margin_rel[i])
      free(margin_rel[i]);
    if (padding_rel[i])
      free(padding_rel[i]);
  }

  if (text_indent_rel)
    free(text_indent_rel);

  if (width_rel)
    free(width_rel);
}


//
// 'hdStyle::copy()' - Copy style properties from an original style.
//

void
hdStyle::copy(hdStyle *o)		// I - Original style
{
  int	i;				// Looping var


  if (!o)
    return;

  // Copy all of the properties from the original style to the current one.
  background_color[0]  = o->background_color[0];
  background_color[1]  = o->background_color[1];
  background_color[2]  = o->background_color[2];
  background_color_set = o->background_color_set;
  set_string(o->background_image, background_image);
  background_position[0] = o->background_position[0];
  background_position[1] = o->background_position[1];
  set_string(o->background_position_rel[0], background_position_rel[0]);
  set_string(o->background_position_rel[1], background_position_rel[1]);
  background_repeat = o->background_repeat;
  for (i = 0; i < 4; i ++)
  {
    border[i].color[0]  = o->border[i].color[0];
    border[i].color[1]  = o->border[i].color[1];
    border[i].color[2]  = o->border[i].color[2];
    border[i].color_set = o->border[i].color_set;
    border[i].style     = o->border[i].style;
    border[i].width     = o->border[i].width;
    set_string(o->border[i].width_rel, border[i].width_rel);
  }
  clear     = o->clear;
  color[0]  = o->color[0];
  color[1]  = o->color[1];
  color[2]  = o->color[2];
  color_set = o->color_set;
  direction = o->direction;
  display   = o->display;
  float_    = o->float_;
  font      = o->font;
  set_string(o->font_family, font_family);
  font_size = o->font_size;
  set_string(o->font_size_rel, font_size_rel);
  font_style   = o->font_style;
  font_variant = o->font_variant;
  font_weight  = o->font_weight;
  height       = o->height;
  set_string(o->height_rel, height_rel);
  letter_spacing = o->letter_spacing;
  line_height    = o->line_height;
  set_string(o->line_height_rel, line_height_rel);
  set_string(o->list_style_image, list_style_image);
  list_style_position = o->list_style_position;
  list_style_type     = o->list_style_type;
  for (i = 0; i < 4; i ++)
  {
    margin[i] = o->margin[i];
    set_string(o->margin_rel[i], margin_rel[i]);
  }
  orphans = o->orphans;
  for (i = 0; i < 4; i ++)
  {
    padding[i] = o->padding[i];
    set_string(o->padding_rel[i], padding_rel[i]);
  }
  page_break_after  = o->page_break_after;
  page_break_before = o->page_break_before;
  page_break_inside = o->page_break_inside;
  text_align        = o->text_align;
  text_decoration   = o->text_decoration;
  text_indent       = o->text_indent;
  set_string(o->text_indent_rel, text_indent_rel);
  text_transform = o->text_transform;
  unicode_bidi   = o->unicode_bidi;
  vertical_align = o->vertical_align;
  white_space    = o->white_space;
  widows         = o->widows;
  width          = o->width;
  set_string(o->width_rel, width_rel);
  word_spacing = o->word_spacing;
}


//
// 'hdStyle::get_border_style()' - Get a border style value.
//

hdBorderStyle				// O - Numeric value
hdStyle::get_border_style(
  const char *value)			// I - String value
{
  if (!strcasecmp(value, "dotted"))
    return (HD_BORDER_STYLE_DOTTED);
  else if (!strcasecmp(value, "dashed"))
    return (HD_BORDER_STYLE_DASHED);
  else if (!strcasecmp(value, "solid"))
    return (HD_BORDER_STYLE_SOLID);
  else if (!strcasecmp(value, "double"))
    return (HD_BORDER_STYLE_DOUBLE);
  else if (!strcasecmp(value, "groove"))
    return (HD_BORDER_STYLE_GROOVE);
  else if (!strcasecmp(value, "ridge"))
    return (HD_BORDER_STYLE_RIDGE);
  else if (!strcasecmp(value, "inset"))
    return (HD_BORDER_STYLE_INSET);
  else if (!strcasecmp(value, "outset"))
    return (HD_BORDER_STYLE_OUTSET);
  else if (!strcasecmp(value, "inherit"))
    return (HD_BORDER_STYLE_INHERIT);
  else
    return (HD_BORDER_STYLE_NONE);
}


//
// 'hdStyle::get_border_width()' - Get a border width value.
//

float					// O - Numeric value
hdStyle::get_border_width(
  const char   *value,			// I - String value
  hdStyleSheet *css)			// I - Stylesheet
{
  if (!strcasecmp(value, "thin"))
    return (1.0f * 72.0f / css->ppi);
  else if (!strcasecmp(value, "medium"))
    return (2.0f * 72.0f / css->ppi);
  else if (!strcasecmp(value, "thick"))
    return (3.0f * 72.0f / css->ppi);
  else if (!strcasecmp(value, "inherit"))
    return (HD_BORDER_WIDTH_INHERIT);
  else
    return (get_length(value, css->ppi, 72.0f / css->ppi, css));
}


//
// 'hdStyle::get_color()' - Get a 24-bit color value.
//

bool					// O - True on success, false on error
hdStyle::get_color(const char *color,	// I - Color string
                   hdByte     *rgb,	// O - RGB color
		   hdColor    *set)	// O - Color setting
{
  int		i, j;			// Temp/looping vars
  char		tempcolor[8];		// Temporary hex color
  static struct
  {
    const char	*name;			// Color name
    hdByte	red,			// Red value
		green,			// Green value
		blue;			// Blue value
  }		colors[] =		// Color "database"
		{
		  { "aqua",	0,   255, 255 }, // AKA Cyan
		  { "black",	0,   0,   0 },
		  { "blue",	0,   0,   255 },
		  { "cyan",	0,   255, 255 },
		  { "fuchsia",	255, 0,   255 }, // AKA Magenta
		  { "gray",	128, 128, 128 },
		  { "green",	0,   128, 0 },
		  { "grey",	128, 128, 128 },
		  { "lime",	0,   255, 0 },
		  { "magenta",	255, 0,   255 },
		  { "maroon",	128, 0,   0 },
		  { "navy",	0,   0,   128 },
		  { "olive",	128, 128, 0 },
		  { "purple",	128, 0,   128 },
		  { "red",	255, 0,   0 },
		  { "silver",	192, 192, 192 },
		  { "teal",	0,   128, 128 },
		  { "white",	255, 255, 255 },
		  { "yellow",	255, 255, 0 }
		};


  if (!strcasecmp(color, "transparent"))
  {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    if (set)
      *set = HD_COLOR_TRANSPARENT;

    return (true);
  }
  else if (!strcasecmp(color, "inherit"))
  {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    if (set)
      *set = HD_COLOR_INHERIT;

    return (true);
  }
  else if (set)
    *set = HD_COLOR_SET;

  // See if this is a hex color with a missing # in front...
  if (strlen(color) == 6)
  {
    for (i = 0; i < 6; i ++)
      if (!isxdigit(color[i]))
        break;

    if (i == 6)
    {
      // Update the color name to be #RRGGBB instead of RRGGBB...
      tempcolor[0] = '#';
      strcpy(tempcolor + 1, color);
      color = tempcolor;
    }
  }

  // Lookup the color...
  if (!color[0])
  {
    // No color specified, so set RGB to 0,0,0 (black) and return false.
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    return (false);
  }

  if (color[0] == '#')
  {
    // RGB value in hex...
    i = strtol(color + 1, NULL, 16);
    if (color[4])
    {
      // #RRGGBB
      rgb[0] = i >> 16;
      rgb[1] = i >> 8;
      rgb[2] = i & 255;
    }
    else
    {
      // #RGB
      j = (i >> 8) & 15;
      rgb[0] = j | (j << 4);
      j = (i >> 4) & 15;
      rgb[1] = j | (j << 4);
      j = i & 15;
      rgb[2] = j | (j << 4);
    }

    return (true);
  }

  if (!strncasecmp(color, "rgb(", 4))
  {
    // rgb(r,g,b)
    int irgb[3];

    if (sscanf(color, "rgb(%d,%d,%d)", irgb + 0, irgb + 1, irgb + 2) != 3)
    {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
      return (false);
    }

    rgb[0] = irgb[0];
    rgb[1] = irgb[1];
    rgb[2] = irgb[2];

    return (true);
  }

  for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
    if (!strcasecmp(colors[i].name, color))
      break;

  if (i < (int)(sizeof(colors) / sizeof(colors[0])))
  {
    rgb[0] = colors[i].red;
    rgb[1] = colors[i].green;
    rgb[2] = colors[i].blue;

    return (true);
  }
  else
  {
    // Unknown color specified, so set RGB to 0,0,0 (black) and return -1.
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    return (false);
  }
}


//
// 'hdStyle::get_color()' - Get a floating point color value.
//

bool					// O - True on success, false on error
hdStyle::get_color(const char *color,	// I - Color string
                   float      *rgb)	// O - RGB color
{
  hdByte	temp[3];		// 24-bit RGB color
  bool		status;			// Conversion status


  status = get_color(color, temp);
  rgb[0] = (float)temp[0] / 255.0f;
  rgb[1] = (float)temp[1] / 255.0f;
  rgb[2] = (float)temp[2] / 255.0f;

  return (status);
}


//
// 'hdStyle::get_length()' - Get a length/measurement value...
//

float					// O - Length value
hdStyle::get_length(
  const char   *length,			// I - Length string
  float        max_length,		// I - Maximum length
  float        def_units,		// I - Default units
  hdStyleSheet *css,			// I - Stylesheet
  bool         *relative)		// O - Relative value?
{
  float	val;				// Length value
  char	*units;				// Units after length


  // Unless otherwise set, all values are absolute...
  if (relative)
    *relative = false;

  // Check early for "auto" and "inherit" values...
  if (!strcasecmp(length, "auto"))
    return (HD_WIDTH_AUTO);
  else if (!strcasecmp(length, "inherit"))
    return (HD_WIDTH_INHERIT);

  // Get the floating point value of "length" and skip to the units portion...
  val = (float)strtod(length, &units);

  // If the length doesn't have a numeric value, return 0.0...
  if (units == length)
    return (0.0f);

  // Check for a trailing units specifier...
  if (!strcasecmp(units, "cm"))
    val *= 72.0f / 2.54f;
  else if (!strcasecmp(units, "em"))
  {
    if (relative)
      *relative = true;

    val *= font_size;
  }
  else if (!strcasecmp(units, "ex"))
  {
    if (relative)
      *relative = true;

    // The "x" height should be taken from the current font,
    // however we may not know that info yet.  The constant
    // 0.45 corresponds to the Times-Roman font...

    if (font)
      val *= font->x_height * font_size;
    else
      val *= 0.45 * font_size;
  }
  else if (!strcasecmp(units, "in"))
    val *= 72.0f;
  else if (!strcasecmp(units, "mm"))
    val *= 72.0f / 25.4f;
  else if (!strcasecmp(units, "pc"))
    val *= 12.0f;
  else if (!strcasecmp(units, "pt") || (!*units && val >= 4.0))
  {
    // No conversion needed for points...
  }
  else if (!strcasecmp(units, "px"))
  {
    // Pixel resolutions use a global "pixels per inch" setting
    // from the stylesheet...
    if (relative)
      *relative = true;

    val *= 72.0f / css->ppi;
  }
  else if (!*units)
  {
    // No units == points or a multiplier...
    if (relative)
      *relative = true;

    val *= def_units;
  }
  else if (!strcasecmp(units, "%"))
  {
    if (relative)
      *relative = true;

    val = max_length * val / 100.0f;
  }

  return (val);
}


//
// 'hdStyle::get_list_style_type()' - Get a list style type value.
//

hdListStyleType				// O - Numeric value
hdStyle::get_list_style_type(
  const char *value)			// I - String value
{
  if (!strcasecmp(value, "disc"))
    return (HD_LIST_STYLE_TYPE_DISC);
  else if (!strcasecmp(value, "circle"))
    return (HD_LIST_STYLE_TYPE_CIRCLE);
  else if (!strcasecmp(value, "square"))
    return (HD_LIST_STYLE_TYPE_SQUARE);
  else if (!strcasecmp(value, "decimal"))
    return (HD_LIST_STYLE_TYPE_DECIMAL);
  else if (!strcasecmp(value, "lower-roman"))
    return (HD_LIST_STYLE_TYPE_LOWER_ROMAN);
  else if (!strcasecmp(value, "upper-roman"))
    return (HD_LIST_STYLE_TYPE_UPPER_ROMAN);
  else if (!strcasecmp(value, "lower-alpha"))
    return (HD_LIST_STYLE_TYPE_LOWER_ALPHA);
  else if (!strcasecmp(value, "upper-alpha"))
    return (HD_LIST_STYLE_TYPE_UPPER_ALPHA);
  else if (!strcasecmp(value, "inherit"))
    return (HD_LIST_STYLE_TYPE_INHERIT);
  else
    return (HD_LIST_STYLE_TYPE_NONE);
}


//
// 'hdStyle::get_page_break()' - Get a page break value.
//

hdPageBreak				// O - Numeric value
hdStyle::get_page_break(
  const char *value)			// I - String value
{
  if (!strcasecmp(value, "always"))
    return (HD_PAGE_BREAK_ALWAYS);
  else if (!strcasecmp(value, "avoid"))
    return (HD_PAGE_BREAK_AVOID);
  else if (!strcasecmp(value, "left"))
    return (HD_PAGE_BREAK_LEFT);
  else if (!strcasecmp(value, "right"))
    return (HD_PAGE_BREAK_RIGHT);
  else if (!strcasecmp(value, "inherit"))
    return (HD_PAGE_BREAK_INHERIT);
  else
    return (HD_PAGE_BREAK_AUTO);
}


//
// 'hdStyle::get_pos()' - Get a margin/position/padding/border index.
//

int					// O - Index in property array
hdStyle::get_pos(const char *name)	// I - Name of property
{
  const char	*pos;			// Position name...


  if ((pos = strchr(name, '-')) != NULL)
    pos ++;
  else
    pos = name;

  if (!strncasecmp(pos, "bottom", 6))
    return (HD_POS_BOTTOM);
  else if (!strncasecmp(pos, "left", 4))
    return (HD_POS_LEFT);
  else if (!strncasecmp(pos, "right", 5))
    return (HD_POS_RIGHT);
  else
    return (HD_POS_TOP);
}


//
// 'hdStyle::get_subvalue()' - Get a subvalue from a property value.
//

char *					// O - New value pointer
hdStyle::get_subvalue(char *valueptr)	// I - Value pointer
{
  while (*valueptr && !isspace(*valueptr))
    if (*valueptr == '(')
    {
      // Handle things like rgb(r, g, b), url(foo), etc...
      for (valueptr ++; *valueptr && *valueptr != ')'; valueptr ++);

      if (*valueptr)
        valueptr ++;
    }
    else if (*valueptr == ',')
    {
      // Skip whitespace after comma...
      for (valueptr ++; isspace(*valueptr); valueptr ++);
    }
    else
      valueptr ++;

  while (isspace(*valueptr))
    *valueptr++ = '\0';

  return (valueptr);
}


//
// 'hdStyle::get_width()' - Get width of string in points.
//

float					// O - Width
hdStyle::get_width(const hdChar *s)	// I - String
{
  if (!font)
    return (0.0f);
  else
    return (font->get_width(s) * font_size);
}

#define hdElIsBlock(x)	((x) == HD_ELEMENT_CENTER || (x) == HD_ELEMENT_DIV ||\
			 (x) == HD_ELEMENT_BLOCKQUOTE ||\
			 (x) == HD_ELEMENT_ADDRESS || \
			 (x) == HD_ELEMENT_P || (x) == HD_ELEMENT_PRE ||\
			 ((x) >= HD_ELEMENT_H1 && (x) <= HD_ELEMENT_H15) ||\
			 (x) == HD_ELEMENT_HR || (x) == HD_ELEMENT_TABLE)
#define hdElIsList(x)	((x) == HD_ELEMENT_DL || (x) == HD_ELEMENT_OL ||\
			 (x) == HD_ELEMENT_UL || (x) == HD_ELEMENT_DIR ||\
			 (x) == HD_ELEMENT_MENU || (x) == HD_ELEMENT_LI ||\
			 (x) == HD_ELEMENT_DD || (x) == HD_ELEMENT_DT)
#define hdElIsTable(x)	((x) == HD_ELEMENT_TBODY || (x) == HD_ELEMENT_THEAD ||\
			 (x) == HD_ELEMENT_TFOOT || (x) == HD_ELEMENT_TR ||\
			 (x) == HD_ELEMENT_TD || (x) == HD_ELEMENT_TH)

//
// 'hdStyle::inherit()' - Inherit style properties from a parent style.
//

void
hdStyle::inherit(hdStyle *p)		// I - Parent style
{
  int	i;				// Looping var


  if (!p)
    return;

  if (p->background_color_set != HD_COLOR_INHERIT)
  {
    background_color[0]  = p->background_color[0];
    background_color[1]  = p->background_color[1];
    background_color[2]  = p->background_color[2];
    background_color_set = p->background_color_set;
  }

  if (p->background_image && strcasecmp(p->background_image, "inherit"))
    set_string(p->background_image, background_image);

  for (i = 0; i < 2; i ++)
    if (p->background_position_rel[i] &&
        strcasecmp(p->background_position_rel[i], "inherit"))
    {
      set_string(NULL, background_position_rel[i]);
      background_position[i] = p->background_position[i];
    }

  if (p->background_repeat != HD_BACKGROUND_REPEAT_INHERIT)
    background_repeat = p->background_repeat;

  for (i = 0; i < 4; i ++)
  {
    if (p->border[i].color_set != HD_COLOR_INHERIT)
    {
      border[i].color[0]  = p->border[i].color[0];
      border[i].color[1]  = p->border[i].color[1];
      border[i].color[2]  = p->border[i].color[2];
      border[i].color_set = p->border[i].color_set;
    }

    if (p->border[i].style != HD_BORDER_STYLE_INHERIT)
      border[i].style = p->border[i].style;

    if (p->border[i].width != HD_BORDER_WIDTH_INHERIT)
    {
      set_string(NULL, border[i].width_rel);
      border[i].width = p->border[i].width;
    }
  }

  if (p->clear != HD_CLEAR_INHERIT)
    clear = p->clear;

  if (p->color_set != HD_COLOR_INHERIT)
  {
    color[0]  = p->color[0];
    color[1]  = p->color[1];
    color[2]  = p->color[2];
    color_set = p->color_set;
  }

  if (p->direction != HD_DIRECTION_INHERIT)
    direction = p->direction;

  if (p->display != HD_DISPLAY_INHERIT)
    display = p->display;

  if (p->float_ != HD_FLOAT_INHERIT)
    float_ = p->float_;

  if (p->font_family)
    set_string(p->font_family, font_family);

  if (p->font_size != HD_FONT_SIZE_INHERIT)
  {
    set_string(NULL, font_size_rel);
    font_size = p->font_size;
  }

  if (p->font_style != HD_FONT_STYLE_INHERIT)
    font_style = p->font_style;

  if (p->font_variant != HD_FONT_VARIANT_INHERIT)
    font_variant = p->font_variant;

  if (p->font_weight != HD_FONT_WEIGHT_INHERIT)
    font_weight = p->font_weight;

  if (p->height != HD_HEIGHT_INHERIT)
  {
    set_string(NULL, height_rel);
    height = p->height;
  }

  if (p->letter_spacing != HD_LETTER_SPACING_INHERIT)
    letter_spacing = p->letter_spacing;

  if (p->line_height != HD_LINE_HEIGHT_INHERIT)
  {
    set_string(NULL, line_height_rel);
    line_height = p->line_height;
  }

  if (p->list_style_image && strcasecmp(p->list_style_image, "inherit"))
    set_string(p->list_style_image, list_style_image);

  if (p->list_style_position != HD_LIST_STYLE_POSITION_INHERIT)
    list_style_position = p->list_style_position;

  if (p->list_style_type != HD_LIST_STYLE_TYPE_INHERIT)
    list_style_type = p->list_style_type;

  for (i = 0; i < 4; i ++)
    if (p->margin[i] != HD_MARGIN_INHERIT)
    {
      set_string(NULL, margin_rel[i]);
      margin[i] = p->margin[i];
    }

  if (p->orphans != HD_ORPHANS_INHERIT)
    orphans = p->orphans;

  for (i = 0; i < 4; i ++)
    if (p->padding[i] != HD_PADDING_INHERIT)
    {
      set_string(NULL, padding_rel[i]);
      padding[i] = p->padding[i];
    }

  if (p->page_break_after != HD_PAGE_BREAK_INHERIT)
    page_break_after = p->page_break_after;

  if (p->page_break_before != HD_PAGE_BREAK_INHERIT)
    page_break_before = p->page_break_before;

  if (p->page_break_inside != HD_PAGE_BREAK_INHERIT)
    page_break_inside = p->page_break_inside;

  if (p->text_align != HD_TEXT_ALIGN_INHERIT)
    text_align = p->text_align;

  if (p->text_decoration != HD_TEXT_DECORATION_INHERIT)
    text_decoration = p->text_decoration;

  if (p->text_indent != HD_TEXT_INDENT_INHERIT)
  {
    set_string(NULL, text_indent_rel);
    text_indent = p->text_indent;
  }

  if (p->text_transform != HD_TEXT_TRANSFORM_INHERIT)
    text_transform = p->text_transform;

  if (p->unicode_bidi != HD_UNICODE_BIDI_INHERIT)
    unicode_bidi = p->unicode_bidi;

  if (p->vertical_align != HD_VERTICAL_ALIGN_INHERIT)
    vertical_align = p->vertical_align;

  if (p->white_space != HD_WHITE_SPACE_INHERIT)
    white_space = p->white_space;

  if (p->widows != HD_WIDOWS_INHERIT)
    widows = p->widows;

  if (p->width != HD_WIDTH_INHERIT)
  {
    set_string(NULL, width_rel);
    width = p->width;
  }

  if (p->word_spacing != HD_WORD_SPACING_INHERIT)
    word_spacing = p->word_spacing;
}


//
// 'hdStyle::init()' - Initialize the style data to defaults.
//

void
hdStyle::init()
{
  int	i;				// Looping var


  // Initialize properties to defaults...
  background_color[0]        = 0;
  background_color[1]        = 0;
  background_color[2]        = 0;
  background_color_set       = HD_COLOR_TRANSPARENT;
  background_image           = NULL;
  background_position[0]     = 0.0f;
  background_position[1]     = 0.0f;
  background_position_rel[0] = strdup("0%");
  background_position_rel[1] = strdup("0%");
  background_repeat          = HD_BACKGROUND_REPEAT_NO_REPEAT;

  for (i = 0; i < 4; i ++)
  {
    border[i].style     = HD_BORDER_STYLE_NONE;
    border[i].color[0]  = 0;
    border[i].color[1]  = 0;
    border[i].color[2]  = 0;
    border[i].color_set = HD_COLOR_UNDEFINED;
    border[i].width     = 0.0f;
    border[i].width_rel = strdup("2px");
  }

  caption_side        = HD_CAPTION_SIDE_TOP;
  clear               = HD_CLEAR_NONE;
  color[0]            = 0;
  color[1]            = 0;
  color[2]            = 0;
  color_set           = HD_COLOR_INHERIT;
  direction           = HD_DIRECTION_INHERIT;
  display             = HD_DISPLAY_INLINE;
  float_              = HD_FLOAT_NONE;
  font                = NULL;
  font_family         = NULL;
  font_size           = HD_FONT_SIZE_INHERIT;
  font_size_rel       = NULL;
  font_style          = HD_FONT_STYLE_INHERIT;
  font_variant        = HD_FONT_VARIANT_INHERIT;
  font_weight         = HD_FONT_WEIGHT_INHERIT;
  height              = HD_WIDTH_AUTO;
  height_rel          = NULL;
  letter_spacing      = HD_LETTER_SPACING_INHERIT;
  line_height         = HD_LINE_HEIGHT_INHERIT;
  line_height_rel     = NULL;
  list_style_image    = NULL;
  list_style_position = HD_LIST_STYLE_POSITION_INHERIT;
  list_style_type     = HD_LIST_STYLE_TYPE_INHERIT;

  for (i = 0; i < 4; i ++)
  {
    margin[i]      = 0.0;
    margin_rel[i]  = NULL;
    padding[i]     = 0.0;
    padding_rel[i] = NULL;
  }

  orphans           = HD_ORPHANS_INHERIT;
  page_break_after  = HD_PAGE_BREAK_AUTO;
  page_break_before = HD_PAGE_BREAK_AUTO;
  page_break_inside = HD_PAGE_BREAK_AUTO;

  text_align      = HD_TEXT_ALIGN_INHERIT;
  text_decoration = HD_TEXT_DECORATION_NONE;
  text_indent     = HD_TEXT_INDENT_INHERIT;
  text_indent_rel = NULL;
  text_transform  = HD_TEXT_TRANSFORM_INHERIT;
  unicode_bidi    = HD_UNICODE_BIDI_NORMAL;
  vertical_align  = HD_VERTICAL_ALIGN_BASELINE;
  white_space     = HD_WHITE_SPACE_NORMAL;
  widows          = HD_WIDOWS_INHERIT;
  width           = HD_WIDTH_AUTO;
  width_rel       = NULL;
  word_spacing    = 0.0f;
}


//
// 'hdStyle::load()' - Load a style definition from a string.
//

bool					// O - True on success, false on errors
hdStyle::load(hdStyleSheet *css,	// I - Stylesheet
              const char   *s)		// I - Style data
{
  int		pos;			// Position in multi-valued props
  bool		relative;		// Value is relative?
  float		length;			// Length value
  hdByte	rgb[3];			// RGB value
  char	 	name[255],		// Property name
		*nameptr,		// Pointer into property name
		value[1024],		// Property value
		*subvalue,		// Pointer to current sub-value
		*valueptr;		// Pointer into property value
  bool		status;			// Did we find any errors?
  hdColor	set;			// Color set?


  // Range check...
  if (!css || !s)
    return (false);

  // Loop until we have nothing more...
  status = true;

  while (*s)
  {
    // Skip leading whitespace...
    while (isspace(*s))
      s ++;

    if (!*s)
      break;

    // Get the attribute name...
    for (nameptr = name; isalpha(*s) || *s == '-'; s ++)
      if (nameptr < (name + sizeof(name) - 1))
        *nameptr++ = *s;

    *nameptr = '\0';

    if (*s != ':')
    {
      // No colon after name... 
      status = false;
      break;
    }

    s ++;

    // Get the attribute value...
    while (isspace(*s))
      s ++;

    for (valueptr = value; *s && *s != ';'; s ++)
      if (valueptr < (value + sizeof(value) - 1))
        *valueptr++ = *s;

    *valueptr = '\0';

    if (*s == ';')
      s ++;

    // Strip trailing whitespace...
    for (valueptr --; valueptr > value && isspace(*valueptr); *valueptr-- = '\0');

    // See if we know the name...
    if (!strcasecmp(name, "background"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = -1;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (get_color(subvalue, rgb, &set))
	{
	  memcpy(background_color, rgb, sizeof(background_color));
          background_color_set = set;
	}
        else if (!strcasecmp(subvalue, "transparent"))
	  background_color_set = HD_COLOR_TRANSPARENT;
	else if (!strcasecmp(subvalue, "none"))
	  set_string(NULL, background_image);
	else if (!strcasecmp(subvalue, "bottom"))
	{
	  set_string("100%", background_position_rel[1]);

	  pos = 0;
	}
	else if (!strcasecmp(subvalue, "center"))
	{
	  if (pos < 0)
	  {
	    set_string("50%", background_position_rel[0]);
	    set_string("50%", background_position_rel[1]);
          }
	  else
	  {
	    set_string("50%", background_position_rel[pos]);

	    pos = 1 - pos;
	  }
	}
	else if (!strcasecmp(subvalue, "left"))
	{
	  set_string("0%", background_position_rel[0]);

	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "right"))
	{
	  set_string("100%", background_position_rel[0]);

	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "top"))
	{
	  set_string("0%", background_position_rel[1]);

	  pos = 0;
	}
	else if (!strcasecmp(subvalue, "repeat"))
	  background_repeat = HD_BACKGROUND_REPEAT_REPEAT;
	else if (!strcasecmp(subvalue, "repeat-x"))
	  background_repeat = HD_BACKGROUND_REPEAT_REPEAT_X;
	else if (!strcasecmp(subvalue, "repeat-y"))
	  background_repeat = HD_BACKGROUND_REPEAT_REPEAT_Y;
	else if (!strcasecmp(subvalue, "no-repeat"))
	  background_repeat = HD_BACKGROUND_REPEAT_NO_REPEAT;
	else if (!strcasecmp(subvalue, "inherit"))
	{
	  set_string(NULL, background_image);

	  background_color_set = HD_COLOR_INHERIT;
	  background_repeat    = HD_BACKGROUND_REPEAT_INHERIT;
	}
	else if (isdigit(subvalue[0]))
	{
	  // Get a numeric position for the background...
          length = get_length(subvalue, 100.0, 72.0f / css->ppi, css, &relative);

	  if (pos < 0)
	  {
	    set_string(relative ? subvalue : NULL, background_position_rel[0]);
	    set_string(relative ? subvalue : NULL, background_position_rel[1]);

	    background_position[0] = length;
	    background_position[1] = length;

	    pos = 0;
	  }
	  else
	  {
	    set_string(relative ? subvalue : NULL, background_position_rel[pos]);
	    background_position[pos] = length;

            pos = 1 - pos;
	  }
	}
	else if (strncasecmp(subvalue, "url(", 4) == 0)
	{
          char	*paren;		// Closing parenthesis


	  if ((paren = strrchr(subvalue, ')')) != NULL)
	    *paren = '\0';

	  set_string(subvalue + 4, background_image);
	}
	else
	{
	  // Unknown background value...
	  status = false;
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown background property \"%s\"!\n", subvalue);
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "background-attachment"))
    {
      // Ignore...
    }
    else if (!strcasecmp(name, "background-color"))
    {
      if (get_color(value, rgb, &set))
      {
	memcpy(background_color, rgb, sizeof(background_color));
        background_color_set = set;
      }
      else
      {
	// Unknown value...
	status = false;
      }
    }
    else if (!strcasecmp(name, "background-image"))
    {
      if (strncasecmp(value, "url(", 4) == 0)
      {
        char	*paren;		// Closing parenthesis


	if ((paren = strrchr(value, ')')) != NULL)
	  *paren = '\0';

	set_string(value + 4, background_image);
      }
    }
    else if (!strcasecmp(name, "background-position"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = -1;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (!strcasecmp(subvalue, "bottom"))
	{
	  set_string("100%", background_position_rel[1]);

	  pos = 0;
	}
	else if (!strcasecmp(subvalue, "center"))
	{
	  if (pos < 0)
	  {
	    set_string("50%", background_position_rel[0]);
	    set_string("50%", background_position_rel[1]);
          }
	  else
	  {
	    set_string("50%", background_position_rel[pos]);

	    pos = 1 - pos;
	  }
	}
	else if (!strcasecmp(subvalue, "left"))
	{
	  set_string("0%", background_position_rel[0]);

	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "right"))
	{
	  set_string("100%", background_position_rel[0]);

	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "top"))
	{
	  set_string("0%", background_position_rel[1]);

	  pos = 0;
	}
	else if (isdigit(subvalue[0]))
	{
	  // Get a numeric position for the background...
          length = get_length(subvalue, 100.0, 72.0f / css->ppi, css, &relative);

	  if (pos < 0)
	  {
	    set_string(relative ? subvalue : NULL, background_position_rel[0]);
	    set_string(relative ? subvalue : NULL, background_position_rel[1]);

	    background_position[0] = length;
	    background_position[1] = length;

	    pos = 0;
	  }
	  else
	  {
	    set_string(relative ? subvalue : NULL, background_position_rel[pos]);

            background_position[pos] = length;

            pos = 1 - pos;
	  }
	}
	else
	{
	  // Unknown background value...
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "background-repeat"))
    {
      if (!strcasecmp(value, "repeat"))
	background_repeat = HD_BACKGROUND_REPEAT_REPEAT;
      else if (!strcasecmp(value, "repeat-x"))
	background_repeat = HD_BACKGROUND_REPEAT_REPEAT_X;
      else if (!strcasecmp(value, "repeat-y"))
	background_repeat = HD_BACKGROUND_REPEAT_REPEAT_Y;
      else if (!strcasecmp(value, "no-repeat"))
	background_repeat = HD_BACKGROUND_REPEAT_NO_REPEAT;
      else if (!strcasecmp(value, "inherit"))
	background_repeat = HD_BACKGROUND_REPEAT_INHERIT;
      else
      {
	// Unknown value...
	status = false;
      }
    }
    else if (!strcasecmp(name, "border"))
    {
      // Loop until we have exhausted the value string...
      char last = '\0';
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (get_color(subvalue, rgb, &set))
	{
	  if (last != 'c')
	  {
	    last = 'c';
	    pos  = 0;
	  }

          switch (pos)
	  {
	    case 0 :
	        memcpy(border[HD_POS_TOP].color, rgb,
		       sizeof(border[HD_POS_TOP].color));
		border[HD_POS_TOP].color_set = set;

	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = set;

	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = set;

	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;

	    case 1 :
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = set;

	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;

	    case 2 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = set;
		break;

	    case 3 :
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;
          }

	  pos ++;
	}
        else if (!strcasecmp(subvalue, "thin") ||
		 !strcasecmp(subvalue, "medium") ||
		 !strcasecmp(subvalue, "thick") ||
		 isdigit(subvalue[0]))
	{
	  float bw = get_border_width(subvalue, css);


	  if (last != 'w')
	  {
	    last = 'w';
	    pos  = 0;
	  }

          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].width    = bw;
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_BOTTOM].width = bw;
		border[HD_POS_LEFT].width   = bw;
		break;

	    case 1 :
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_LEFT].width   = bw;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 3 :
		border[HD_POS_LEFT].width   = bw;
		break;
          }

	  pos ++;
	}
        else if (!strcasecmp(subvalue, "none") ||
	         !strcasecmp(subvalue, "dotted") ||
	         !strcasecmp(subvalue, "dashed") ||
	         !strcasecmp(subvalue, "solid") ||
	         !strcasecmp(subvalue, "double") ||
	         !strcasecmp(subvalue, "groove") ||
	         !strcasecmp(subvalue, "ridge") ||
	         !strcasecmp(subvalue, "inset") ||
	         !strcasecmp(subvalue, "outset"))
	{
	  hdBorderStyle bs = get_border_style(subvalue);


	  if (last != 's')
	  {
	    last = 's';
	    pos  = 0;
	  }

          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].style    = bs;
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_BOTTOM].style = bs;
		border[HD_POS_LEFT].style   = bs;
		break;

	    case 1 :
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_LEFT].style   = bs;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 3 :
		border[HD_POS_LEFT].style   = bs;
		break;
          }

	  pos ++;
	}
	else if (!strcasecmp(subvalue, "inherit"))
	{
	  // TODO
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown border value \"%s\"!\n",
	                 subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "border-bottom") ||
             !strcasecmp(name, "border-left") ||
             !strcasecmp(name, "border-right") ||
             !strcasecmp(name, "border-top"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = get_pos(name);

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (get_color(subvalue, rgb, &set))
	{
	  memcpy(border[pos].color, rgb, sizeof(border[pos].color));
	  border[pos].color_set = set;
	}
        else if (!strcasecmp(subvalue, "thin") ||
		 !strcasecmp(subvalue, "medium") ||
		 !strcasecmp(subvalue, "thick") ||
		 isdigit(subvalue[0]))
	{
	  border[pos].width = get_border_width(subvalue, css);
	}
        else if (!strcasecmp(subvalue, "none") ||
	         !strcasecmp(subvalue, "dotted") ||
	         !strcasecmp(subvalue, "dashed") ||
	         !strcasecmp(subvalue, "solid") ||
	         !strcasecmp(subvalue, "double") ||
	         !strcasecmp(subvalue, "groove") ||
	         !strcasecmp(subvalue, "ridge") ||
	         !strcasecmp(subvalue, "inset") ||
	         !strcasecmp(subvalue, "outset"))
	{
	  border[pos].style = get_border_style(subvalue);
	}
	else if (!strcasecmp(subvalue, "inherit"))
	{
	  border[pos].color_set = HD_COLOR_INHERIT;
	  border[pos].style     = HD_BORDER_STYLE_INHERIT;
	  border[pos].width     = HD_BORDER_WIDTH_INHERIT;
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown %s value \"%s\"!\n", name, subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "border-bottom-color") ||
             !strcasecmp(name, "border-left-color") ||
             !strcasecmp(name, "border-right-color") ||
             !strcasecmp(name, "border-top-color"))
    {
      pos = get_pos(name);

      if (get_color(value, rgb, &set))
      {
	memcpy(border[pos].color, rgb, sizeof(border[pos].color));
        border[pos].color_set = set;
      }
    }
    else if (!strcasecmp(name, "border-bottom-style") ||
             !strcasecmp(name, "border-left-style") ||
             !strcasecmp(name, "border-right-style") ||
             !strcasecmp(name, "border-top-style"))
    {
      pos = get_pos(name);

      if (!strcasecmp(value, "none") ||
	  !strcasecmp(value, "dotted") ||
	  !strcasecmp(value, "dashed") ||
	  !strcasecmp(value, "solid") ||
	  !strcasecmp(value, "double") ||
	  !strcasecmp(value, "groove") ||
	  !strcasecmp(value, "ridge") ||
	  !strcasecmp(value, "inset") ||
	  !strcasecmp(value, "outset"))
	border[pos].style = get_border_style(value);
      else if (!strcasecmp(value, "inherit"))
        border[pos].style = HD_BORDER_STYLE_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown %s value \"%s\"!\n", name,
	               value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "border-bottom-width") ||
             !strcasecmp(name, "border-left-width") ||
             !strcasecmp(name, "border-right-width") ||
             !strcasecmp(name, "border-top-width"))
    {
      border[get_pos(name)].width = get_border_width(value, css);
    }
    else if (!strcasecmp(name, "border-collapse"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "border-color"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (get_color(subvalue, rgb, &set))
	{
          switch (pos)
	  {
	    case 0 :
	        memcpy(border[HD_POS_TOP].color, rgb,
		       sizeof(border[HD_POS_TOP].color));
		border[HD_POS_TOP].color_set = set;

	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = set;

	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = set;

	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;

	    case 1 :
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = set;

	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;

	    case 2 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = set;
		break;

	    case 3 :
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = set;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown border-color value \"%s\"!\n", subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "border-bottom") ||
             !strcasecmp(name, "border-left") ||
             !strcasecmp(name, "border-right") ||
             !strcasecmp(name, "border-top"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = get_pos(name);

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (get_color(subvalue, rgb, &set))
	{
	  memcpy(border[pos].color, rgb, sizeof(border[pos].color));
	  border[pos].color_set = set;
	}
        else if (!strcasecmp(subvalue, "thin") ||
		 !strcasecmp(subvalue, "medium") ||
		 !strcasecmp(subvalue, "thick") ||
		 isdigit(subvalue[0]))
	{
	  border[pos].width = get_border_width(subvalue, css);
	}
        else if (!strcasecmp(subvalue, "none") ||
	         !strcasecmp(subvalue, "dotted") ||
	         !strcasecmp(subvalue, "dashed") ||
	         !strcasecmp(subvalue, "solid") ||
	         !strcasecmp(subvalue, "double") ||
	         !strcasecmp(subvalue, "groove") ||
	         !strcasecmp(subvalue, "ridge") ||
	         !strcasecmp(subvalue, "inset") ||
	         !strcasecmp(subvalue, "outset"))
	{
	  border[pos].style = get_border_style(subvalue);
	}
	else if (!strcasecmp(subvalue, "inherit"))
	{
	  border[pos].color_set = HD_COLOR_INHERIT;
	  border[pos].style     = HD_BORDER_STYLE_INHERIT;
	  border[pos].width     = HD_BORDER_WIDTH_INHERIT;
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown %s value \"%s\"!\n", name, subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "border-spacing"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "border-style"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!strcasecmp(subvalue, "none") ||
	    !strcasecmp(subvalue, "dotted") ||
	    !strcasecmp(subvalue, "dashed") ||
	    !strcasecmp(subvalue, "solid") ||
	    !strcasecmp(subvalue, "double") ||
	    !strcasecmp(subvalue, "groove") ||
	    !strcasecmp(subvalue, "ridge") ||
	    !strcasecmp(subvalue, "inset") ||
	    !strcasecmp(subvalue, "outset") ||
	    !strcasecmp(subvalue, "inherit"))
	{
	  hdBorderStyle bs = get_border_style(subvalue);


          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].style    = bs;
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_BOTTOM].style = bs;
		border[HD_POS_LEFT].style   = bs;
		break;

	    case 1 :
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_LEFT].style   = bs;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 3 :
		border[HD_POS_LEFT].style   = bs;
		break;
          }

	  pos ++;
	}
	else if (!strcasecmp(subvalue, "inherit"))
	{
          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].style    = HD_BORDER_STYLE_INHERIT;
		border[HD_POS_RIGHT].style  = HD_BORDER_STYLE_INHERIT;
		border[HD_POS_BOTTOM].style = HD_BORDER_STYLE_INHERIT;
		border[HD_POS_LEFT].style   = HD_BORDER_STYLE_INHERIT;
		break;

	    case 1 :
		border[HD_POS_RIGHT].style  = HD_BORDER_STYLE_INHERIT;
		border[HD_POS_LEFT].style   = HD_BORDER_STYLE_INHERIT;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].style = HD_BORDER_STYLE_INHERIT;
		break;

	    case 3 :
		border[HD_POS_LEFT].style   = HD_BORDER_STYLE_INHERIT;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown border-style value \"%s\"!\n", subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "border-width"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!strcasecmp(subvalue, "thin") ||
	    !strcasecmp(subvalue, "medium") ||
	    !strcasecmp(subvalue, "thick") ||
	    !strcasecmp(subvalue, "inherit") ||
	    isdigit(subvalue[0]))
	{
	  float bw = get_border_width(subvalue, css);


          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].width    = bw;
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_BOTTOM].width = bw;
		border[HD_POS_LEFT].width   = bw;
		break;

	    case 1 :
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_LEFT].width   = bw;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 3 :
		border[HD_POS_LEFT].width   = bw;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown border value \"%s\"!\n", subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "bottom") ||
             !strcasecmp(name, "left") ||
             !strcasecmp(name, "right") ||
             !strcasecmp(name, "top"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "caption-side"))
    {
      if (!strcasecmp(value, "top"))
        caption_side = HD_CAPTION_SIDE_TOP;
      else if (!strcasecmp(value, "bottom"))
        caption_side = HD_CAPTION_SIDE_BOTTOM;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown caption-side value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "clear"))
    {
      if (!strcasecmp(value, "none"))
        clear = HD_CLEAR_NONE;
      else if (!strcasecmp(value, "left"))
        clear = HD_CLEAR_LEFT;
      else if (!strcasecmp(value, "right"))
        clear = HD_CLEAR_RIGHT;
      else if (!strcasecmp(value, "both"))
        clear = HD_CLEAR_BOTH;
      else if (!strcasecmp(value, "inherit"))
        clear = HD_CLEAR_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown clear value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "color"))
    {
      if (get_color(value, rgb, &set))
      {
	memcpy(color, rgb, sizeof(color));
        color_set = set;
      }
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown color value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "content"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "counter-increment"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "counter-reset"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "direction"))
    {
      if (!strcasecmp(value, "ltr"))
        direction = HD_DIRECTION_LTR;
      else if (!strcasecmp(value, "rtl"))
        direction = HD_DIRECTION_RTL;
      else if (!strcasecmp(value, "inherit"))
        direction = HD_DIRECTION_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown direction value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "display"))
    {
      if (!strcasecmp(value, "none"))
        display = HD_DISPLAY_NONE;
      else if (!strcasecmp(value, "block"))
        display = HD_DISPLAY_BLOCK;
      else if (!strcasecmp(value, "compact"))
        display = HD_DISPLAY_COMPACT;
      else if (!strcasecmp(value, "inline"))
        display = HD_DISPLAY_INLINE;
      else if (!strcasecmp(value, "inline-table"))
        display = HD_DISPLAY_INLINE_TABLE;
      else if (!strcasecmp(value, "list-item"))
        display = HD_DISPLAY_LIST_ITEM;
      else if (!strcasecmp(value, "marker"))
        display = HD_DISPLAY_MARKER;
      else if (!strcasecmp(value, "run-in"))
        display = HD_DISPLAY_RUN_IN;
      else if (!strcasecmp(value, "table"))
        display = HD_DISPLAY_TABLE;
      else if (!strcasecmp(value, "table-caption"))
        display = HD_DISPLAY_TABLE_CAPTION;
      else if (!strcasecmp(value, "table-cell"))
        display = HD_DISPLAY_TABLE_CELL;
      else if (!strcasecmp(value, "table-column"))
        display = HD_DISPLAY_TABLE_COLUMN;
      else if (!strcasecmp(value, "table-column-group"))
        display = HD_DISPLAY_TABLE_COLUMN_GROUP;
      else if (!strcasecmp(value, "table-footer-group"))
        display = HD_DISPLAY_TABLE_FOOTER_GROUP;
      else if (!strcasecmp(value, "table-header-group"))
        display = HD_DISPLAY_TABLE_HEADER_GROUP;
      else if (!strcasecmp(value, "table-row"))
        display = HD_DISPLAY_TABLE_ROW;
      else if (!strcasecmp(value, "table-row-group"))
        display = HD_DISPLAY_TABLE_ROW_GROUP;
      else if (!strcasecmp(value, "inherit"))
        display = HD_DISPLAY_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown display value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "empty-cells"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "float"))
    {
      if (!strcasecmp(value, "none"))
        float_ = HD_FLOAT_NONE;
      else if (!strcasecmp(value, "left"))
        float_ = HD_FLOAT_LEFT;
      else if (!strcasecmp(value, "right"))
        float_ = HD_FLOAT_RIGHT;
      else if (!strcasecmp(value, "inherit"))
        float_ = HD_FLOAT_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown float value \"%s\"!", value);
      }
    }
    else if (!strcasecmp(name, "font"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (!strcasecmp(subvalue, "normal"))
	{
	  switch (pos)
	  {
	    case 0 : // Style
	        font_style = HD_FONT_STYLE_NORMAL;
		break;

	    case 1 : // Variant
	        font_variant = HD_FONT_VARIANT_NORMAL;
		break;

	    case 2 : // Weight
	        font_weight = HD_FONT_WEIGHT_NORMAL;
		break;

            default :
	        status = false;
	        progress_error(HD_ERROR_CSS_ERROR, "Unknown font value \"normal\"!");
		break;
	  }

	  pos ++;
	}
	else if (!strcasecmp(subvalue, "italic"))
	{
	  font_style = HD_FONT_STYLE_ITALIC;
	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "oblique"))
	{
	  font_style = HD_FONT_STYLE_OBLIQUE;
	  pos = 1;
	}
	else if (!strcasecmp(subvalue, "small-caps"))
	{
	  font_variant = HD_FONT_VARIANT_SMALL_CAPS;
	  pos = 2;
	}
	else if (!strcasecmp(subvalue, "bold") ||
	         !strcasecmp(subvalue, "bolder") ||
	         !strcasecmp(subvalue, "600") ||
	         !strcasecmp(subvalue, "700") ||
	         !strcasecmp(subvalue, "800") ||
	         !strcasecmp(subvalue, "900"))
	  font_weight = HD_FONT_WEIGHT_BOLD;
	else if (!strcasecmp(subvalue, "100") ||
	         !strcasecmp(subvalue, "200") ||
	         !strcasecmp(subvalue, "300") ||
	         !strcasecmp(subvalue, "400") ||
	         !strcasecmp(subvalue, "500") ||
		 !strcasecmp(subvalue, "lighter"))
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	else if (!strcasecmp(subvalue, "inherit"))
	  font_weight = HD_FONT_WEIGHT_INHERIT;
	else if (isdigit(subvalue[0]))
	{
	  char	*lh;			// Line height, if available


          if ((lh = strchr(subvalue, '/')) != NULL)
	    *lh++ = '\0';

          set_font_size(subvalue, css);

          if (lh)
	    set_line_height(lh, css);
	}
	else if (!strcasecmp(subvalue, "inherit"))
	{
	  // TODO
	}
	else
	  set_string(subvalue, font_family);

	// Advance to the next sub-value...
	subvalue = valueptr;
	pos ++;
      }
    }
    else if (!strcasecmp(name, "font-family"))
      set_string(value, font_family);
    else if (!strcasecmp(name, "font-size"))
    {
      char	*lh;			// Line height, if available


      if ((lh = strchr(value, '/')) != NULL)
	*lh++ = '\0';

      set_font_size(value, css);

      if (lh)
        set_line_height(lh, css);
    }
    else if (!strcasecmp(name, "font-size-adjust"))
    {
      // Not implemented
    }
    else if (!strcasecmp(name, "font-stretch"))
    {
      // Not implemented
    }
    else if (!strcasecmp(name, "font-style"))
    {
      if (!strcasecmp(value, "normal"))
        font_style = HD_FONT_STYLE_NORMAL;
      else if (!strcasecmp(value, "italic"))
	font_style = HD_FONT_STYLE_ITALIC;
      else if (!strcasecmp(value, "oblique"))
	font_style = HD_FONT_STYLE_OBLIQUE;
      else if (!strcasecmp(value, "inherit"))
	font_style = HD_FONT_STYLE_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown font-style value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "font-variant"))
    {
      if (!strcasecmp(value, "normal"))
        font_variant = HD_FONT_VARIANT_NORMAL;
      else if (!strcasecmp(value, "small-caps"))
	font_variant = HD_FONT_VARIANT_SMALL_CAPS;
      else if (!strcasecmp(value, "inherit"))
	font_variant = HD_FONT_VARIANT_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown font-variant value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "font-weight"))
    {
      if (!strcasecmp(value, "bold") ||
	  !strcasecmp(value, "bolder") ||
	  !strcasecmp(value, "600") ||
	  !strcasecmp(value, "700") ||
	  !strcasecmp(value, "800") ||
	  !strcasecmp(value, "900"))
	font_weight = HD_FONT_WEIGHT_BOLD;
      else if (!strcasecmp(value, "normal") ||
               !strcasecmp(value, "lighter") ||
               !strcasecmp(value, "100") ||
	       !strcasecmp(value, "200") ||
	       !strcasecmp(value, "300") ||
	       !strcasecmp(value, "400") ||
	       !strcasecmp(value, "500"))
	font_weight = HD_FONT_WEIGHT_NORMAL;
      else if (!strcasecmp(value, "inherit"))
	font_weight = HD_FONT_WEIGHT_INHERIT;
      else
      {
        status = false;
	progress_error(HD_ERROR_CSS_ERROR, "Unknown font-weight value \"%s\"!\n", value);
      }
    }
    else if (!strcasecmp(name, "height"))
    {
      height = get_length(value, css->media.page_print_length,
                          72.0f / css->ppi, css, &relative);

      set_string(relative ? value : NULL, height_rel);
    }
    else if (!strcasecmp(name, "letter-spacing"))
      letter_spacing = get_length(value, 0.0f, 72.0f / css->ppi, css);
    else if (!strcasecmp(name, "line-height"))
      set_line_height(value, css);
    else if (!strcasecmp(name, "list-style"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!strcasecmp(subvalue, "none"))
	{
	  switch (pos)
	  {
	    case 0 : // list-style-type
	        list_style_type = HD_LIST_STYLE_TYPE_NONE;
		break;

	    case 1 : // list-style-image
	        set_string(NULL, list_style_image);
		break;
	  }

	  pos ++;
	}
        else if (!strcasecmp(subvalue, "disc") ||
	         !strcasecmp(subvalue, "circle") ||
	         !strcasecmp(subvalue, "square") ||
	         !strcasecmp(subvalue, "decimal") ||
	         !strcasecmp(subvalue, "lower-roman") ||
	         !strcasecmp(subvalue, "upper-roman") ||
	         !strcasecmp(subvalue, "lower-alpha") ||
	         !strcasecmp(subvalue, "upper-alpha"))
	{
	  list_style_type = get_list_style_type(subvalue);
	  pos = 1;
	}
        else if (!strcasecmp(subvalue, "inside"))
	  list_style_position = HD_LIST_STYLE_POSITION_INSIDE;
        else if (!strcasecmp(subvalue, "outside"))
	  list_style_position = HD_LIST_STYLE_POSITION_OUTSIDE;
        else if (!strcasecmp(subvalue, "inherit"))
	{
	  // TODO
	}
        else if (!strncasecmp(subvalue, "url(", 4))
	{
	  char	*paren;		// Closing parenthesis


          if ((paren = strrchr(subvalue, ')')) != NULL)
	    *paren = '\0';

          set_string(subvalue + 4, list_style_image);
	}
	else
	{
	  // Unknown value...
	  progress_error(HD_ERROR_CSS_ERROR, "Unknown list-style value \"%s\"!\n", subvalue);
	  status = false;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "list-style-image"))
    {
      if (!strncasecmp(value, "url(", 4))
      {
	char	*paren;		// Closing parenthesis


        if ((paren = strrchr(value, ')')) != NULL)
	  *paren = '\0';

        set_string(value + 4, list_style_image);
      }
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown list-style-image value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "list-style-position"))
    {
      if (!strcasecmp(value, "inside"))
	list_style_position = HD_LIST_STYLE_POSITION_INSIDE;
      else if (!strcasecmp(value, "outside"))
	list_style_position = HD_LIST_STYLE_POSITION_OUTSIDE;
      else if (!strcasecmp(value, "inherit"))
	list_style_position = HD_LIST_STYLE_POSITION_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown list-style-position value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "list-style-type"))
    {
      if (!strcasecmp(value, "inherit") ||
	  !strcasecmp(value, "disc") ||
	  !strcasecmp(value, "circle") ||
	  !strcasecmp(value, "square") ||
	  !strcasecmp(value, "decimal") ||
	  !strcasecmp(value, "lower-roman") ||
	  !strcasecmp(value, "upper-roman") ||
	  !strcasecmp(value, "lower-alpha") ||
	  !strcasecmp(value, "upper-alpha") ||
	  !strcasecmp(value, "inherit"))
	list_style_type = get_list_style_type(value);
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown list-style-type value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "margin"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        length = get_length(subvalue, 100.0f, 72.0f / css->ppi, css, &relative);

        switch (pos)
	{
	  case 0 :
	      margin[HD_POS_TOP] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_TOP]);

	      margin[HD_POS_RIGHT] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_RIGHT]);

	      margin[HD_POS_BOTTOM] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_BOTTOM]);

	      margin[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_LEFT]);
	      break;

	  case 1 :
	      margin[HD_POS_RIGHT] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_RIGHT]);

	      margin[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_LEFT]);
	      break;

	  case 2 :
	      margin[HD_POS_BOTTOM] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_BOTTOM]);
	      break;

	  case 3 :
	      margin[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, margin_rel[HD_POS_LEFT]);
	      break;
        }

	pos ++;

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "margin-bottom") ||
             !strcasecmp(name, "margin-left") ||
	     !strcasecmp(name, "margin-right") ||
	     !strcasecmp(name, "margin-top"))
    {
      pos = get_pos(name);

      margin[pos] = get_length(value, 100.0f, 72.0f / css->ppi, css, &relative);
      set_string(relative ? value : NULL, margin_rel[pos]);
    }
    else if (!strcasecmp(name, "marker-offset"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "marks"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "max-height"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "max-width"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "min-height"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "min-width"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "orphans"))
    {
      if (!strcasecmp(value, "inherit"))
        orphans = HD_ORPHANS_INHERIT;
      else if (isdigit(value[0]))
        orphans = atoi(value);
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown orphans value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "outline"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "outline-color"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "outline-style"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "outline-width"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "padding"))
    {
      // Loop until we have exhausted the value string...
      subvalue = valueptr = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        length = get_length(subvalue, 100.0f, 72.0f / css->ppi, css, &relative);

        switch (pos)
	{
	  case 0 :
	      padding[HD_POS_TOP] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_TOP]);

	      padding[HD_POS_RIGHT] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_RIGHT]);

	      padding[HD_POS_BOTTOM] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_BOTTOM]);

	      padding[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_LEFT]);
	      break;

	  case 1 :
	      padding[HD_POS_RIGHT] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_RIGHT]);

	      padding[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_LEFT]);
	      break;

	  case 2 :
	      padding[HD_POS_BOTTOM] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_BOTTOM]);
	      break;

	  case 3 :
	      padding[HD_POS_LEFT] = length;
	      set_string(relative ? subvalue : NULL, padding_rel[HD_POS_LEFT]);
	      break;
        }

	pos ++;

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (!strcasecmp(name, "padding-bottom") ||
             !strcasecmp(name, "padding-left") ||
	     !strcasecmp(name, "padding-right") ||
	     !strcasecmp(name, "padding-top"))
    {
      pos = get_pos(name);

      padding[pos] = get_length(value, 100.0f, 72.0f / css->ppi, css, &relative);
      set_string(relative ? value : NULL, padding_rel[pos]);
    }
    else if (!strcasecmp(name, "page"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "page-break-after"))
    {
      page_break_after = get_page_break(value);
    }
    else if (!strcasecmp(name, "page-break-before"))
    {
      page_break_before = get_page_break(value);
    }
    else if (!strcasecmp(name, "page-break-inside"))
    {
      page_break_inside = get_page_break(value);
    }
    else if (!strcasecmp(name, "position"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "quotes"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "size"))
    {
      // NOT IMPLEMENTED
    }
    else if (!strcasecmp(name, "text-align"))
    {
      if (!strcasecmp(value, "left"))
        text_align = HD_TEXT_ALIGN_LEFT;
      else if (!strcasecmp(value, "center"))
        text_align = HD_TEXT_ALIGN_CENTER;
      else if (!strcasecmp(value, "right"))
        text_align = HD_TEXT_ALIGN_RIGHT;
      else if (!strcasecmp(value, "justify"))
        text_align = HD_TEXT_ALIGN_JUSTIFY;
      else if (!strcasecmp(value, "inherit"))
        text_align = HD_TEXT_ALIGN_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown text-align value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "text-decoration"))
    {
      if (!strcasecmp(value, "none"))
        text_decoration = HD_TEXT_DECORATION_NONE;
      else if (!strcasecmp(value, "underline"))
        text_decoration = HD_TEXT_DECORATION_UNDERLINE;
      else if (!strcasecmp(value, "overline"))
        text_decoration = HD_TEXT_DECORATION_OVERLINE;
      else if (!strcasecmp(value, "line-through"))
        text_decoration = HD_TEXT_DECORATION_LINE_THROUGH;
      else if (!strcasecmp(value, "inherit"))
        text_decoration = HD_TEXT_DECORATION_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown text-decoration value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "text-indent"))
    {
      text_indent = get_length(value, css->media.page_print_width,
                               72.0f / css->ppi, css, &relative);
      set_string(relative ? value : NULL, text_indent_rel);
    }
    else if (!strcasecmp(name, "text-transform"))
    {
      if (!strcasecmp(value, "none"))
        text_transform = HD_TEXT_TRANSFORM_NONE;
      else if (!strcasecmp(value, "capitalize"))
        text_transform = HD_TEXT_TRANSFORM_CAPITALIZE;
      else if (!strcasecmp(value, "uppercase"))
        text_transform = HD_TEXT_TRANSFORM_UPPERCASE;
      else if (!strcasecmp(value, "lowercase"))
        text_transform = HD_TEXT_TRANSFORM_LOWERCASE;
      else if (!strcasecmp(value, "inherit"))
        text_transform = HD_TEXT_TRANSFORM_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown text-transform value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "unicode-bidi"))
    {
      if (!strcasecmp(value, "inherit"))
        unicode_bidi = HD_UNICODE_BIDI_INHERIT;
      else if (!strcasecmp(value, "normal"))
        unicode_bidi = HD_UNICODE_BIDI_NORMAL;
      else if (!strcasecmp(value, "embed"))
        unicode_bidi = HD_UNICODE_BIDI_EMBED;
      else if (!strcasecmp(value, "bidi-override"))
        unicode_bidi = HD_UNICODE_BIDI_BIDI_OVERRIDE;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown unicode-bidi value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "vertical-align"))
    {
      if (!strcasecmp(value, "baseline"))
        vertical_align = HD_VERTICAL_ALIGN_BASELINE;
      else if (!strcasecmp(value, "sub"))
        vertical_align = HD_VERTICAL_ALIGN_SUB;
      else if (!strcasecmp(value, "super"))
        vertical_align = HD_VERTICAL_ALIGN_SUPER;
      else if (!strcasecmp(value, "top"))
        vertical_align = HD_VERTICAL_ALIGN_TOP;
      else if (!strcasecmp(value, "text-top"))
        vertical_align = HD_VERTICAL_ALIGN_TEXT_TOP;
      else if (!strcasecmp(value, "middle"))
        vertical_align = HD_VERTICAL_ALIGN_MIDDLE;
      else if (!strcasecmp(value, "bottom"))
        vertical_align = HD_VERTICAL_ALIGN_BOTTOM;
      else if (!strcasecmp(value, "text-bottom"))
        vertical_align = HD_VERTICAL_ALIGN_TEXT_BOTTOM;
      else if (!strcasecmp(value, "inherit"))
        vertical_align = HD_VERTICAL_ALIGN_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown vertical-align value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "white-space"))
    {
      if (!strcasecmp(value, "normal"))
        white_space = HD_WHITE_SPACE_NORMAL;
      else if (!strcasecmp(value, "pre"))
        white_space = HD_WHITE_SPACE_PRE;
      else if (!strcasecmp(value, "pre-wrap"))
        white_space = HD_WHITE_SPACE_PRE_WRAP;
      else if (!strcasecmp(value, "pre-line"))
        white_space = HD_WHITE_SPACE_PRE_LINE;
      else if (!strcasecmp(value, "nowrap"))
        white_space = HD_WHITE_SPACE_NOWRAP;
      else if (!strcasecmp(value, "inherit"))
        white_space = HD_WHITE_SPACE_INHERIT;
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown white-space value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "widows"))
    {
      if (!strcasecmp(value, "inherit"))
        widows = HD_ORPHANS_INHERIT;
      else if (isdigit(value[0]))
        widows = atoi(value);
      else
      {
	// Unknown value...
	progress_error(HD_ERROR_CSS_ERROR, "Unknown widows value \"%s\"!\n", value);
	status = false;
      }
    }
    else if (!strcasecmp(name, "width"))
    {
      width = get_length(value, css->media.page_print_width,
                         72.0f / css->ppi, css, &relative);
      set_string(relative ? value : NULL, width_rel);
    }
    else if (!strcasecmp(name, "word-spacing"))
      word_spacing = get_length(value, 0.0f, 72.0f / css->ppi, css);
    else
    {
      progress_error(HD_ERROR_CSS_ERROR, "Unknown style property \"%s\"!\n", name);
      status = false;
    }
  }

  updated = false;

  return (status);
}


//
// 'hdStyle::set_font_size()' - Set the font size.
//

void
hdStyle::set_font_size(
  const char   *s,			// I - Font size
  hdStyleSheet *css)			// I - Stylesheet
{
  bool	relative;			// Relative height?


  font_size = get_length(s, css->def_style.font_size,
                         css->def_style.font_size, css, &relative);

//  printf("set_font_size(s=\"%s\", css=%p) = %.1f, font_size=%.1f\n",
//         s, css, font_size, css->def_style.font_size);

  set_string(relative ? s : NULL, font_size_rel);
}


//
// 'hdStyle::set_line_height()' - Set the line height.
//

void
hdStyle::set_line_height(
  const char   *lh,			// I - Line height
  hdStyleSheet *css)			// I - Stylesheet
{
  bool	relative;			// Relative height?


  line_height = get_length(lh, css->def_style.font_size,
                           css->def_style.font_size, css, &relative);

//  printf("set_line_height(lh=\"%s\", css=%p) = %.1f, font_size=%.1f\n",
//         lh, css, line_height, css->def_style.font_size);
  set_string(relative ? lh : NULL, line_height_rel);
}


//
// 'hdStyle::set_string()' - Copy and set a string value.
//

void
hdStyle::set_string(const char *s,	// I - New string value
                    char       *&var)	// O - String variable
{
  if (var)
    free(var);

  var = s ? strdup(s) : NULL;
}


//
// 'hdStyle::update()' - Update relative style definitions.
//

void
hdStyle::update(hdStyleSheet *css)	// I - Stylesheet
{
  int	i;				// Looping var


  // Stop immediately if we are already updated...
  if (updated)
    return;

  updated = true;

  // Start by updating the font info for this style, since many things
  // depend on the current font size...
  if (font_size_rel)
  {
    hdStyle	*body_style;		// BODY element style


    if (selectors[0].element == HD_ELEMENT_BODY ||
        (body_style = css->find_style(HD_ELEMENT_BODY)) == NULL)
      font_size = get_length(font_size_rel, css->def_style.font_size,
                             css->def_style.font_size, css);
    else
    {
      body_style->update(css);

      font_size = body_style->font_size;
      font_size = get_length(font_size_rel, body_style->font_size,
                             body_style->font_size, css);
    }
  }
  else if (font_size == HD_FONT_SIZE_INHERIT)
    font_size = css->def_style.font_size;

  font = css->find_font(this);

  // Then do all of the other relative properties...
  if (background_position_rel[0])
    background_position[0] = get_length(background_position_rel[0],
                                        css->media.page_print_width,
				        72.0f / css->ppi, css);
  if (background_position_rel[1])
    background_position[1] = get_length(background_position_rel[1],
                                        css->media.page_print_length,
					72.0f / css->ppi, css);

  if (height_rel)
    height = get_length(height_rel, css->media.page_print_length,
                        72.0f / css->ppi, css);

  if (line_height_rel)
  {
    if (!strcasecmp(line_height_rel, "normal"))
      line_height = 1.2f * font_size;
    else
      line_height = get_length(line_height_rel, font_size, font_size, css);
  }

  for (i = 0; i < 4; i ++)
  {
    if (border[i].width_rel)
      border[i].width = get_length(border[i].width_rel,
                                   css->media.page_print_width,
                                   72.0f / css->ppi, css);

    if (margin_rel[i])
      margin[i] = get_length(margin_rel[i], css->media.page_print_width,
                             72.0f / css->ppi, css);

    if (padding_rel[i])
      padding[i] = get_length(padding_rel[i], css->media.page_print_width,
                              72.0f / css->ppi, css);
  }

  if (text_indent_rel)
    text_indent = get_length(text_indent_rel, css->media.page_print_width,
                             72.0f / css->ppi, css);

  if (width_rel)
    width = get_length(width_rel, css->media.page_print_width,
                       72.0f / css->ppi, css);
}


//
// End of "$Id$".
//
