//
// "$Id: style.cxx,v 1.2 2002/02/05 17:47:58 mike Exp $"
//
//   CSS routines for HTMLDOC, a HTML document processing program.
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
// Contents:
//
//

//
// Include necessary headers.
//

#include "tree.h"
#include "hdstring.h"
#include <stdlib.h>


//
// 'hdStyle::hdStyle()' - Create a new style record.
//

hdStyle::hdStyle(int        nsels,	// I - Number of selectors
		 hdSelector *sels,	// I - Selectors
                 hdStyle    *p)		// I - Parent style
{
  int	i;				// Looping var


  // Initialize everything to defaults (0, etc.)...
  //
  // Note: memset() is safe for structs like hdStyle...
  memset(this, 0, sizeof(hdStyle));

  background_position[0] = HD_MARGIN_AUTO;
  background_position[1] = HD_MARGIN_AUTO;

  for (i = 0; i < 4; i ++)
    border[i].width = HD_WIDTH_AUTO;

  display = HD_DISPLAY_INLINE;

  // Copy the selectors.  The selector strings are allocated by
  // the caller, but are freed by the destructor...
  num_selectors = nsels;
  selectors     = new hdSelector[nsels];

  memcpy(selectors, sels, nsels * sizeof(hdSelector));

  // Now copy the parent attributes as needed...
  inherit(p);
}


//
// 'hdStyle::~hdStyle()' - Destroy a style record.
//

hdStyle::~hdStyle()
{
  int	i;				// Looping var


  // Free the selectors as needed...
  if (num_selectors)
  {
    for (i = 0; i < num_selectors; i ++)
    {
      // Free selector strings as needed; these are allocated using
      // the strdup() function, so free() must be used...
      if (selectors[i].class_)
        free(selectors[i].class_);

      if (selectors[i].pseudo)
        free(selectors[i].pseudo);

      if (selectors[i].id)
        free(selectors[i].id);
    }

    delete[] selectors;
  }

  // Then free any string values that we have...
  if (background_image)
    free(background_image);

  for (i = 0; i < 2; i ++)
    if (background_position_rel[i])
      free(background_position_rel[i]);

  for (i = 0; i < 4; i ++)
  {
    if (margin_rel[i])
      free(margin_rel[i]);
    if (padding_rel[i])
      free(padding_rel[i]);
    if (position_rel[i])
      free(position_rel[i]);
  }

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

  if (text_indent_rel)
    free(text_indent_rel);

  if (width_rel)
    free(width_rel);
}


//
// 'hdStyle::get_border_style()' - Get a border style value.
//

hdBorderStyle					// O - Numeric value
hdStyle::get_border_style(const char *value)	// I - String value
{
  if (strcasecmp(value, "dotted") == 0)
    return (HD_BORDERSTYLE_DOTTED);
  else if (strcasecmp(value, "dashed") == 0)
    return (HD_BORDERSTYLE_DASHED);
  else if (strcasecmp(value, "solid") == 0)
    return (HD_BORDERSTYLE_SOLID);
  else if (strcasecmp(value, "double") == 0)
    return (HD_BORDERSTYLE_DOUBLE);
  else if (strcasecmp(value, "groove") == 0)
    return (HD_BORDERSTYLE_GROOVE);
  else if (strcasecmp(value, "ridge") == 0)
    return (HD_BORDERSTYLE_RIDGE);
  else if (strcasecmp(value, "inset") == 0)
    return (HD_BORDERSTYLE_INSET);
  else if (strcasecmp(value, "outset") == 0)
    return (HD_BORDERSTYLE_OUTSET);
  else
    return (HD_BORDERSTYLE_NONE);
}


//
// 'hdStyle::get_border_width()' - Get a border width value.
//

float						// O - Numeric value
hdStyle::get_border_width(const char *value)	// I - String value
{
  if (strcasecmp(value, "thin") == 0)
    return (1.0f);
  else if (strcasecmp(value, "medium") == 0)
    return (2.0f);
  else if (strcasecmp(value, "thick") == 0)
    return (3.0f);
  else
    return (get_length(value, 100.0f));
}


//
// 'hdStyle::get_color()' - Get a color value.
//

int					// O - 0 on success, -1 on error
get_color(const char    *color,		// I - Color string
          unsigned char *rgb)		// O - RGB color
{
  int		i, j;			// Temp/looping vars
  char		tempcolor[8];		// Temporary hex color
  static struct
  {
    const char		*name;		// Color name
    unsigned char	red,		// Red value
			green,		// Green value
			blue;		// Blue value
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


  // First, see if this is a hex color with a missing # in front...
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
    // No color specified, so set RGB to 0,0,0 (black) and return -1.
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    return (-1);
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

    return (0);
  }

  if (strncasecmp(color, "rgb(", 4) == 0)
  {
    // rgb(r,g,b)
    return (0);
  }

  for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
    if (strcasecmp(colors[i].name, color) == 0)
      break;

  if (i < (int)(sizeof(colors) / sizeof(colors[0])))
  {
    rgb[0] = colors[i].red;
    rgb[1] = colors[i].green;
    rgb[2] = colors[i].blue;

    return (0);
  }
  else
  {
    // Unknown color specified, so set RGB to 0,0,0 (black) and return -1.
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    return (-1);
  }
}


//
// 'hdStyle::get_length()' - Get a length/measurement value...
//

float						// O - Length value
hdStyle::get_length(const char *length,		// I - Length string
                    float      max_length,	// I - Maximum length
		    int        *relative)	// O - Relative value?
{
  float	val;					// Length value
  char	*units;					// Units after length


  // Unless otherwise set, all values are absolute...
  if (relative)
    *relative = 0;

  // Get the floating point value of "length" and skip to the units portion...
  val = (float)strtod(length, &units);

  // If the length doesn't have a numeric value, return 0.0...
  if (units == length)
    return (0.0f);

  // Check for a trailing units specifier...
  if (strcasecmp(units, "cm") == 0)
    val *= 72.0f / 2.54f;
  else if (strcasecmp(units, "em") == 0)
  {
    if (relative)
      *relative = 1;

    val *= font_size;
  }
  else if (strcasecmp(units, "ex") == 0)
  {
    if (relative)
      *relative = 1;

    // The "x" height should be taken from the current font,
    // however we may not know that info yet.  The constant
    // 0.45 corresponds to the Times-Roman font...

    if (font)
      val *= font->x_height * font_size;
    else
      val *= 0.45 * font_size;
  }
  else if (strcasecmp(units, "in") == 0)
    val *= 72.0f;
  else if (strcasecmp(units, "mm") == 0)
    val *= 72.0f / 25.4f;
  else if (strcasecmp(units, "pc") == 0)
    val *= 12.0f;
  else if (strcasecmp(units, "px") == 0 || !*units)
  {
    // Pixel resolutions use a global "pixels per inch" setting
    // that is used by all documents...

    val *= 72.0f / hdTree::ppi;
  }
  else if (strcasecmp(units, "%") == 0)
  {
    if (relative)
      *relative = 1;

    val = max_length * val / 100.0f;

    // Clamp N% to 0 <= N <= 100
    if (val < 0.0f)
      val = 0.0f;
    else if (val > max_length)
      val = max_length;
  }

  return (val);
}


//
// 'hdStyle::get_list_style_type()' - Get a list style type value.
//

hdListStyleType					// O - Numeric value
hdStyle::get_list_style_type(const char *value)	// I - String value
{
  if (strcasecmp(value, "disc") == 0)
    return (HD_LISTSTYLETYPE_DISC);
  else if (strcasecmp(value, "circle") == 0)
    return (HD_LISTSTYLETYPE_CIRCLE);
  else if (strcasecmp(value, "square") == 0)
    return (HD_LISTSTYLETYPE_SQUARE);
  else if (strcasecmp(value, "decimal") == 0)
    return (HD_LISTSTYLETYPE_DECIMAL);
  else if (strcasecmp(value, "lower-roman") == 0)
    return (HD_LISTSTYLETYPE_LOWER_ROMAN);
  else if (strcasecmp(value, "upper-roman") == 0)
    return (HD_LISTSTYLETYPE_UPPER_ROMAN);
  else if (strcasecmp(value, "lower-alpha") == 0)
    return (HD_LISTSTYLETYPE_LOWER_ALPHA);
  else if (strcasecmp(value, "upper-alpha") == 0)
    return (HD_LISTSTYLETYPE_UPPER_ALPHA);
  else
    return (HD_LISTSTYLETYPE_NONE);
}


//
// 'hdStyle::get_page_break()' - Get a page break value.
//

hdPageBreak					// O - Numeric value
get_page_break(const char *value)		// I - String value
{
  if (strcasecmp(value, "always") == 0)
    return (HD_PAGEBREAK_ALWAYS);
  else if (strcasecmp(value, "avoid") == 0)
    return (HD_PAGEBREAK_AVOID);
  else if (strcasecmp(value, "left") == 0)
    return (HD_PAGEBREAK_LEFT);
  else if (strcasecmp(value, "right") == 0)
    return (HD_PAGEBREAK_RIGHT);
  else
    return (HD_PAGEBREAK_AUTO);
}


//
// 'hdStyle::get_pos()' - Get a margin/position/padding/border index.
//

int				// O - Index in property array
get_pos(const char *name)	// I - Name of property
{
  const char	*pos;		// Position name...


  if ((pos = strchr(name, '-')) != NULL)
    pos ++;
  else
    pos = name;

  if (strncasecmp(pos, "bottom", 6) == 0)
    return (HD_POS_BOTTOM);
  else if (strncasecmp(pos, "left", 4) == 0)
    return (HD_POS_LEFT);
  else if (strncasecmp(pos, "right", 5) == 0)
    return (HD_POS_RIGHT);
  else
    return (HD_POS_TOP);
}


//
// 'hdStyle::get_subvalue()' - Get a subvalue from a property value.
//

char *				// O - New value pointer
get_subvalue(char *valueptr)	// I - Value pointer
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
// 'hdStyle::inherit()' - Inherit style properties from a parent style.
//

void
hdStyle::inherit(hdStyle *p)		// I - Parent style
{
  int	i;				// Looping var


  if (!p)
    return;

  if ((hdElIsTable(selectors[0].element) &&
       hdElIsTable(p->selectors[0].element)) ||
      selectors[0].element == p->selectors[0].element ||
      p->selectors[0].element == HD_ELEMENT_NONE)
  {
    // Background and border attributes are only inherited by the
    // same and table elements...
    if (p->background_color_set)
    {
      memcpy(background_color, p->background_color, sizeof(background_color));
      background_color_set = 1;
    }

    if (p->background_image)
    {
      if (background_image)
        free(background_image);

      background_image = strdup(p->background_image);
    }

    for (i = 0; i < 2; i ++)
    {
      if (p->background_position_rel[i])
      {
	if (background_position_rel[i])
          free(background_position_rel[i]);

	background_position_rel[i] = strdup(p->background_position_rel[i]);
      }
      else if (p->background_position[i] != HD_MARGIN_AUTO)
      {
	background_position[i] = p->background_position[i];

        if (background_position_rel[i])
	{
	  free(background_position_rel[i]);
	  background_position_rel[i] = NULL;
	}
      }
    }

    if (p->background_repeat)
      background_repeat = p->background_repeat;

    for (i = 0; i < 4; i ++)
    {
      if (p->border[i].color_set)
      {
	memcpy(border[i].color, p->border[i].color, sizeof(border[i].color));
	border[i].color_set = 1;
      }

      if (p->border[i].style)
	border[i].style = p->border[i].style;

      if (p->border[i].width)
        border[i].width = p->border[i].width;
    }

    if (p->vertical_align)
      vertical_align = p->vertical_align;
  }

  if (selectors[0].element == p->selectors[0].element ||
      p->selectors[0].element == HD_ELEMENT_NONE)
  {
    // Some other attributes are only inherited by the same element...
    for (i = 0; i < 4; i ++)
    {
      if (p->position_rel[i])
      {
	if (position_rel[i])
          free(position_rel[i]);

	position_rel[i] = strdup(p->position_rel[i]);
      }
      else if (p->position[i])
      {
        if (position_rel[i])
	{
	  free(position_rel[i]);
	  position_rel[i] = NULL;
	}

	position[i] = p->position[i];
      }
    }

    if (p->clear)
      clear = p->clear;

    if (p->display)
      display = p->display;

    if (p->float_)
      float_ = p->float_;

    if (p->height_rel)
    {
      if (height_rel)
        free(height_rel);

      height_rel = strdup(height_rel);
    }
    else if (p->height != HD_HEIGHT_AUTO)
    {
      if (height_rel)
      {
        free(height_rel);
	height_rel = NULL;
      }

      height = p->height;
    }

    for (i = 0; i < 4; i ++)
    {
      if (p->margin[i] == HD_MARGIN_AUTO)
	margin[i] = p->margin[i];

      if (p->margin_rel[i])
      {
	if (margin_rel[i])
          free(margin_rel[i]);

	margin_rel[i] = strdup(p->margin_rel[i]);
      }
      else if (p->margin[i] != HD_MARGIN_AUTO)
      {
        if (margin_rel[i])
	{
	  free(margin_rel[i]);
	  margin_rel[i] = NULL;
	}

        margin[i] = p->margin[i];
      }
    }

    for (i = 0; i < 4; i ++)
    {
      if (p->padding_rel[i])
      {
	if (padding_rel[i])
          free(padding_rel[i]);

	padding_rel[i] = strdup(p->padding_rel[i]);
      }
      else if (p->padding[i] != HD_MARGIN_AUTO)
      {
        if (padding_rel[i])
	{
	  free(padding_rel[i]);
	  padding_rel[i] = NULL;
	}

	padding[i] = p->padding[i];
      }
    }

    if (p->page_break_after)
      page_break_after = p->page_break_after;

    if (p->page_break_before)
      page_break_before = p->page_break_before;

    if (p->page_break_inside)
      page_break_inside = p->page_break_inside;

    if (p->text_decoration)
      text_decoration = p->text_decoration;

    if (p->width_rel)
    {
      if (width_rel)
        free(width_rel);

      width_rel = strdup(width_rel);
    }
    else if (p->width != HD_WIDTH_AUTO)
    {
      if (width_rel)
      {
        free(width_rel);
	width_rel = NULL;
      }

      width = p->width;
    }
  }

  if (p->color_set)
  {
    memcpy(color, p->color, sizeof(color));
    color_set = 1;
  }

  if (p->font)
    font = p->font;

  if (p->font_family)
  {
    if (font_family)
      free(font_family);

    font_family = strdup(font_family);
  }

  if (p->font_size_rel)
  {
    if (font_size_rel)
      free(font_size_rel);

    font_size_rel = strdup(p->font_size_rel);
  }
  else if (p->font_size)
  {
    if (font_size_rel)
    {
      free(font_size_rel);
      font_size_rel = NULL;
    }

    font_size = p->font_size;
  }

  if (p->font_style)
    font_style = p->font_style;

  if (p->font_variant)
    font_variant = p->font_variant;

  if (p->font_weight)
    font_weight = p->font_weight;

  if (p->letter_spacing)
    letter_spacing = p->letter_spacing;

  if (p->line_height_rel)
  {
    if (line_height_rel)
      free(line_height_rel);

    line_height_rel = strdup(p->line_height_rel);
  }
  else if (p->line_height)
  {
    if (line_height_rel)
    {
      free(line_height_rel);
      line_height_rel = NULL;
    }

    line_height = p->line_height;
  }

  if ((hdElIsList(selectors[0].element) &&
       hdElIsList(p->selectors[0].element)) ||
      selectors[0].element == p->selectors[0].element ||
      p->selectors[0].element == HD_ELEMENT_NONE)
  {
    // List attributes only inherited by list elements...
    if (p->list_style_image)
      list_style_image = strdup(p->list_style_image);

    if (p->list_style_position)
      list_style_position = p->list_style_position;

    if (p->list_style_type)
      list_style_type = p->list_style_type;
  }

  if ((hdElIsBlock(selectors[0].element) &&
       hdElIsBlock(p->selectors[0].element)) ||
      selectors[0].element == p->selectors[0].element ||
      p->selectors[0].element == HD_ELEMENT_NONE)
  {
    // Some text attributes only inherited by block elements...
    if (p->text_align)
      text_align = p->text_align;

    if (p->text_indent_rel)
    {
      if (text_indent_rel)
        free(text_indent_rel);

      text_indent_rel = strdup(p->text_indent_rel);
    }
    else if (p->text_indent)
    {
      if (text_indent_rel)
      {
        free(text_indent_rel);
	text_indent_rel = NULL;
      }

      text_indent = p->text_indent;
    }
  }

  if (p->text_transform)
    text_transform = p->text_transform;

  if (p->word_spacing)
    word_spacing = p->word_spacing;
}


//
// 'hdStyle::load()' - Load a style definition from a string.
//

int					// O - 0 on success, -1 on errors
hdStyle::load(hdStyleSheet *css,	// I - Stylesheet
              const char   *s)		// I - Style data
{
  int		i,			// Looping var
		pos,			// Position in multi-valued props
		relative;		// Value is relative?
  float		length;			// Length value
  unsigned char	rgb[3];			// RGB value
  char	 	name[255],		// Property name
		*nameptr,		// Pointer into property name
		value[1024],		// Property value
		*subvalue,		// Pointer to current sub-value
		*valueptr;		// Pointer into property value
  int		status;			// Did we find any errors?


  // Range check...
  if (!css || !s)
    return (-1);

  // Loop until we have nothing more...
  status = 0;

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
      status = -1;
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
    if (strcasecmp(name, "background") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = -1;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (!get_color(subvalue, rgb))
	{
	  memcpy(background_color, rgb, sizeof(background_color));
          background_color_set = 1;
	}
        else if (strcasecmp(subvalue, "transparent") == 0)
	{
	  background_color_set = 0;
	}
	else if (strcasecmp(subvalue, "none") == 0)
	{
	  if (background_image)
	    free(background_image);

	  background_image = NULL;
	}
	else if (strcasecmp(subvalue, "bottom") == 0)
	{
	  if (background_position_rel[1])
	    free(background_position_rel[1]);

          background_position_rel[1] = strdup(subvalue);

	  pos = 0;
	}
	else if (strcasecmp(subvalue, "center") == 0)
	{
	  if (pos < 0)
	  {
	    if (background_position_rel[0])
	      free(background_position_rel[0]);

            background_position_rel[0] = strdup(subvalue);

	    if (background_position_rel[1])
	      free(background_position_rel[1]);

            background_position_rel[1] = strdup(subvalue);
          }
	  else
	  {
	    if (background_position_rel[pos])
	      free(background_position_rel[pos]);

            background_position_rel[pos] = strdup(subvalue);

	    pos = 1 - pos;
	  }
	}
	else if (strcasecmp(subvalue, "left") == 0)
	{
	  if (background_position_rel[0])
	    free(background_position_rel[0]);

          background_position_rel[0] = strdup(subvalue);

	  pos = 1;
	}
	else if (strcasecmp(subvalue, "right") == 0)
	{
	  if (background_position_rel[0])
	    free(background_position_rel[0]);

          background_position_rel[0] = strdup(subvalue);

	  pos = 1;
	}
	else if (strcasecmp(subvalue, "top") == 0)
	{
	  if (background_position_rel[1])
	    free(background_position_rel[1]);

          background_position_rel[1] = strdup(subvalue);

	  pos = 0;
	}
	else if (strcasecmp(subvalue, "repeat") == 0)
	{
	  background_repeat = HD_BACKGROUNDREPEAT_REPEAT;
	}
	else if (strcasecmp(subvalue, "repeat-x") == 0)
	{
	  background_repeat = HD_BACKGROUNDREPEAT_REPEAT_X;
	}
	else if (strcasecmp(subvalue, "repeat-y") == 0)
	{
	  background_repeat = HD_BACKGROUNDREPEAT_REPEAT_Y;
	}
	else if (strcasecmp(subvalue, "no-repeat") == 0)
	{
	  background_repeat = HD_BACKGROUNDREPEAT_NO_REPEAT;
	}
	else if (isdigit(subvalue[0]))
	{
	  // Get a numeric position for the background...
          length = get_length(subvalue, 100.0, &relative);

	  if (pos < 0)
	  {
	    if (relative)
	    {
	      if (background_position_rel[0])
		free(background_position_rel[0]);

              background_position_rel[0] = strdup(subvalue);

	      if (background_position_rel[1])
		free(background_position_rel[1]);

              background_position_rel[1] = strdup(subvalue);
	    }
	    else
	    {
	      background_position[0] = length;
	      background_position[1] = length;
	    }

	    pos = 0;
	  }
	  else
	  {
	    if (relative)
	    {
	      if (background_position_rel[pos])
		free(background_position_rel[pos]);

              background_position_rel[pos] = strdup(subvalue);
	    }
	    else
	    {
	      background_position[pos] = length;
	    }

            pos = 1 - pos;
	  }
	}
	else if (strncasecmp(subvalue, "url(", 4) == 0)
	{
          char	*paren;		// Closing parenthesis


	  if (background_image)
	    free(background_image);

	  if ((paren = strrchr(subvalue, ')')) != NULL)
	    *paren = '\0';

	  background_image = strdup(subvalue + 4);
	}
	else
	{
	  // Unknown background value...
	  status = -1;
	  fprintf(stderr, "Unknown background property \"%s\"!\n", subvalue);
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "background-color") == 0)
    {
      if (!get_color(value, rgb))
      {
	memcpy(background_color, rgb, sizeof(background_color));
        background_color_set = 1;
      }
      else
      {
	// Unknown value...
	status = -1;
      }
    }
    else if (strcasecmp(name, "background-image") == 0)
    {
      if (strncasecmp(value, "url(", 4) == 0)
      {
        char	*paren;		// Closing parenthesis


	if (background_image)
	  free(background_image);

	if ((paren = strrchr(value, ')')) != NULL)
	  *paren = '\0';

	background_image = strdup(value + 4);
      }
    }
    else if (strcasecmp(name, "background-position") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = -1;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (strcasecmp(subvalue, "bottom") == 0)
	{
	  if (background_position_rel[1])
	    free(background_position_rel[1]);

          background_position_rel[1] = strdup(subvalue);

	  pos = 0;
	}
	else if (strcasecmp(subvalue, "center") == 0)
	{
	  if (pos < 0)
	  {
	    if (background_position_rel[0])
	      free(background_position_rel[0]);

            background_position_rel[0] = strdup(subvalue);

	    if (background_position_rel[1])
	      free(background_position_rel[1]);

            background_position_rel[1] = strdup(subvalue);
          }
	  else
	  {
	    if (background_position_rel[pos])
	      free(background_position_rel[pos]);

            background_position_rel[pos] = strdup(subvalue);

	    pos = 1 - pos;
	  }
	}
	else if (strcasecmp(subvalue, "left") == 0)
	{
	  if (background_position_rel[0])
	    free(background_position_rel[0]);

          background_position_rel[0] = strdup(subvalue);

	  pos = 1;
	}
	else if (strcasecmp(subvalue, "right") == 0)
	{
	  if (background_position_rel[0])
	    free(background_position_rel[0]);

          background_position_rel[0] = strdup(subvalue);

	  pos = 1;
	}
	else if (strcasecmp(subvalue, "top") == 0)
	{
	  if (background_position_rel[1])
	    free(background_position_rel[1]);

          background_position_rel[1] = strdup(subvalue);

	  pos = 0;
	}
	else if (isdigit(subvalue[0]))
	{
	  // Get a numeric position for the background...
          length = get_length(subvalue, 100.0, &relative);

	  if (pos < 0)
	  {
	    if (relative)
	    {
	      if (background_position_rel[0])
		free(background_position_rel[0]);

              background_position_rel[0] = strdup(subvalue);

	      if (background_position_rel[1])
		free(background_position_rel[1]);

              background_position_rel[1] = strdup(subvalue);
	    }
	    else
	    {
	      background_position[0] = length;
	      background_position[1] = length;
	    }

	    pos = 0;
	  }
	  else
	  {
	    if (relative)
	    {
	      if (background_position_rel[pos])
		free(background_position_rel[pos]);

              background_position_rel[pos] = strdup(subvalue);
	    }
	    else
	    {
	      background_position[pos] = length;
	    }

            pos = 1 - pos;
	  }
	}
	else
	{
	  // Unknown background value...
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "background-repeat") == 0)
    {
      if (strcasecmp(subvalue, "repeat") == 0)
      {
	background_repeat = HD_BACKGROUNDREPEAT_REPEAT;
      }
      else if (strcasecmp(subvalue, "repeat-x") == 0)
      {
	background_repeat = HD_BACKGROUNDREPEAT_REPEAT_X;
      }
      else if (strcasecmp(subvalue, "repeat-y") == 0)
      {
	background_repeat = HD_BACKGROUNDREPEAT_REPEAT_Y;
      }
      else if (strcasecmp(subvalue, "no-repeat") == 0)
      {
	background_repeat = HD_BACKGROUNDREPEAT_NO_REPEAT;
      }
      else
      {
	// Unknown value...
	status = -1;
      }
    }
    else if (strcasecmp(name, "border") == 0)
    {
      // Loop until we have exhausted the value string...
      char last = '\0';
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!get_color(subvalue, rgb))
	{
	  if (last != 'c')
	  {
	    last = 'c';
	    pos  = 0;
	  }

          switch (pos)
	  {
	    case 0 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = 1;
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = 1;
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
	        memcpy(border[HD_POS_TOP].color, rgb,
		       sizeof(border[HD_POS_TOP].color));
		border[HD_POS_TOP].color_set = 1;
		break;

	    case 1 :
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = 1;
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
		break;

	    case 2 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = 1;
		break;

	    case 3 :
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
		break;
          }

	  pos ++;
	}
        else if (strcasecmp(subvalue, "thin") == 0 ||
		 strcasecmp(subvalue, "medium") == 0 ||
		 strcasecmp(subvalue, "thick") == 0 ||
		 isdigit(subvalue[0]))
	{
	  float bw = get_border_width(subvalue);


	  if (last != 'w')
	  {
	    last = 'w';
	    pos  = 0;
	  }

          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].width    = bw;
		border[HD_POS_LEFT].width   = bw;
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 1 :
		border[HD_POS_LEFT].width   = bw;
		border[HD_POS_RIGHT].width  = bw;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 3 :
		border[HD_POS_RIGHT].width  = bw;
		break;
          }

	  pos ++;
	}
        else if (strcasecmp(subvalue, "none") == 0 ||
	         strcasecmp(subvalue, "dotted") == 0 ||
	         strcasecmp(subvalue, "dashed") == 0 ||
	         strcasecmp(subvalue, "solid") == 0 ||
	         strcasecmp(subvalue, "double") == 0 ||
	         strcasecmp(subvalue, "groove") == 0 ||
	         strcasecmp(subvalue, "ridge") == 0 ||
	         strcasecmp(subvalue, "inset") == 0 ||
	         strcasecmp(subvalue, "outset") == 0)
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
		border[HD_POS_LEFT].style   = bs;
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 1 :
		border[HD_POS_LEFT].style   = bs;
		border[HD_POS_RIGHT].style  = bs;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 3 :
		border[HD_POS_RIGHT].style  = bs;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown border value \"%s\"!\n", subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "border-bottom") == 0 ||
             strcasecmp(name, "border-left") == 0 ||
             strcasecmp(name, "border-right") == 0 ||
             strcasecmp(name, "border-top") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = get_pos(name);

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!get_color(subvalue, rgb))
	{
	  memcpy(border[pos].color, rgb, sizeof(border[pos].color));
	  border[pos].color_set = 1;
	}
        else if (strcasecmp(subvalue, "thin") == 0 ||
		 strcasecmp(subvalue, "medium") == 0 ||
		 strcasecmp(subvalue, "thick") == 0 ||
		 isdigit(subvalue[0]))
	{
	  border[pos].width = get_border_width(subvalue);
	}
        else if (strcasecmp(subvalue, "none") == 0 ||
	         strcasecmp(subvalue, "dotted") == 0 ||
	         strcasecmp(subvalue, "dashed") == 0 ||
	         strcasecmp(subvalue, "solid") == 0 ||
	         strcasecmp(subvalue, "double") == 0 ||
	         strcasecmp(subvalue, "groove") == 0 ||
	         strcasecmp(subvalue, "ridge") == 0 ||
	         strcasecmp(subvalue, "inset") == 0 ||
	         strcasecmp(subvalue, "outset") == 0)
	{
	  border[pos].style = get_border_style(subvalue);
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown %s value \"%s\"!\n", name, subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "border-bottom-color") == 0 ||
             strcasecmp(name, "border-left-color") == 0 ||
             strcasecmp(name, "border-right-color") == 0 ||
             strcasecmp(name, "border-top-color") == 0)
    {
      pos = get_pos(name);

      if (!get_color(value, rgb))
      {
	memcpy(border[pos].color, rgb, sizeof(border[pos].color));
        border[pos].color_set = 1;
      }
    }
    else if (strcasecmp(name, "border-bottom-width") == 0 ||
             strcasecmp(name, "border-left-width") == 0 ||
             strcasecmp(name, "border-right-width") == 0 ||
             strcasecmp(name, "border-top-width") == 0)
    {
      border[get_pos(name)].width = get_border_width(value);
    }
    else if (strcasecmp(name, "border-collapse") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "border-color") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!get_color(subvalue, rgb))
	{
          switch (pos)
	  {
	    case 0 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = 1;
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = 1;
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
	        memcpy(border[HD_POS_TOP].color, rgb,
		       sizeof(border[HD_POS_TOP].color));
		border[HD_POS_TOP].color_set = 1;
		break;

	    case 1 :
	        memcpy(border[HD_POS_LEFT].color, rgb,
		       sizeof(border[HD_POS_LEFT].color));
		border[HD_POS_LEFT].color_set = 1;
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
		break;

	    case 2 :
	        memcpy(border[HD_POS_BOTTOM].color, rgb,
		       sizeof(border[HD_POS_BOTTOM].color));
		border[HD_POS_BOTTOM].color_set = 1;
		break;

	    case 3 :
	        memcpy(border[HD_POS_RIGHT].color, rgb,
		       sizeof(border[HD_POS_RIGHT].color));
		border[HD_POS_RIGHT].color_set = 1;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown border-color value \"%s\"!\n", subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "border-bottom") == 0 ||
             strcasecmp(name, "border-left") == 0 ||
             strcasecmp(name, "border-right") == 0 ||
             strcasecmp(name, "border-top") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = get_pos(name);

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (!get_color(subvalue, rgb))
	{
	  memcpy(border[pos].color, rgb, sizeof(border[pos].color));
	  border[pos].color_set = 1;
	}
        else if (strcasecmp(subvalue, "thin") == 0 ||
		 strcasecmp(subvalue, "medium") == 0 ||
		 strcasecmp(subvalue, "thick") == 0 ||
		 isdigit(subvalue[0]))
	{
	  border[pos].width = get_border_width(subvalue);
	}
        else if (strcasecmp(subvalue, "none") == 0 ||
	         strcasecmp(subvalue, "dotted") == 0 ||
	         strcasecmp(subvalue, "dashed") == 0 ||
	         strcasecmp(subvalue, "solid") == 0 ||
	         strcasecmp(subvalue, "double") == 0 ||
	         strcasecmp(subvalue, "groove") == 0 ||
	         strcasecmp(subvalue, "ridge") == 0 ||
	         strcasecmp(subvalue, "inset") == 0 ||
	         strcasecmp(subvalue, "outset") == 0)
	{
	  border[pos].style = get_border_style(subvalue);
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown %s value \"%s\"!\n", name, subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "border-spacing") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "border-style") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (strcasecmp(subvalue, "none") == 0 ||
	         strcasecmp(subvalue, "dotted") == 0 ||
	         strcasecmp(subvalue, "dashed") == 0 ||
	         strcasecmp(subvalue, "solid") == 0 ||
	         strcasecmp(subvalue, "double") == 0 ||
	         strcasecmp(subvalue, "groove") == 0 ||
	         strcasecmp(subvalue, "ridge") == 0 ||
	         strcasecmp(subvalue, "inset") == 0 ||
	         strcasecmp(subvalue, "outset") == 0)
	{
	  hdBorderStyle bs = get_border_style(subvalue);


          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].style    = bs;
		border[HD_POS_LEFT].style   = bs;
		border[HD_POS_RIGHT].style  = bs;
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 1 :
		border[HD_POS_LEFT].style   = bs;
		border[HD_POS_RIGHT].style  = bs;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].style = bs;
		break;

	    case 3 :
		border[HD_POS_RIGHT].style  = bs;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown border-style value \"%s\"!\n", subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "border-width") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (strcasecmp(subvalue, "thin") == 0 ||
	    strcasecmp(subvalue, "medium") == 0 ||
	    strcasecmp(subvalue, "thick") == 0 ||
	    isdigit(subvalue[0]))
	{
	  float bw = get_border_width(subvalue);


          switch (pos)
	  {
	    case 0 :
		border[HD_POS_TOP].width    = bw;
		border[HD_POS_LEFT].width   = bw;
		border[HD_POS_RIGHT].width  = bw;
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 1 :
		border[HD_POS_LEFT].width   = bw;
		border[HD_POS_RIGHT].width  = bw;
		break;

	    case 2 :
		border[HD_POS_BOTTOM].width = bw;
		break;

	    case 3 :
		border[HD_POS_RIGHT].width  = bw;
		break;
          }

	  pos ++;
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown border value \"%s\"!\n", subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "bottom") == 0 ||
             strcasecmp(name, "left") == 0 ||
             strcasecmp(name, "right") == 0 ||
             strcasecmp(name, "top") == 0)
    {
      pos           = get_pos(name);
      position[pos] = get_length(value, 100.0, &relative);

      if (relative)
        position_rel[pos] = strdup(value);
    }
    else if (strcasecmp(name, "caption-side") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "clear") == 0)
    {
      if (strcasecmp(value, "none") == 0)
        clear = HD_CLEAR_NONE;
      else if (strcasecmp(value, "left") == 0)
        clear = HD_CLEAR_LEFT;
      else if (strcasecmp(value, "right") == 0)
        clear = HD_CLEAR_RIGHT;
      else if (strcasecmp(value, "both") == 0)
        clear = HD_CLEAR_BOTH;
      else
      {
        status = -1;
	fprintf(stderr, "Unknown clear value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "color") == 0)
    {
      if (!get_color(value, rgb))
      {
	memcpy(color, rgb, sizeof(color));
        color_set = 1;
      }
      else
      {
        status = -1;
	fprintf(stderr, "Unknown color value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "content") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "counter-increment") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "counter-reset") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "direction") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "display") == 0)
    {
      if (strcasecmp(value, "none") == 0)
        display = HD_DISPLAY_NONE;
      else if (strcasecmp(value, "block") == 0)
        display = HD_DISPLAY_BLOCK;
      else if (strcasecmp(value, "compact") == 0)
        display = HD_DISPLAY_COMPACT;
      else if (strcasecmp(value, "inline") == 0)
        display = HD_DISPLAY_INLINE;
      else if (strcasecmp(value, "inline-table") == 0)
        display = HD_DISPLAY_INLINE_TABLE;
      else if (strcasecmp(value, "list-item") == 0)
        display = HD_DISPLAY_LIST_ITEM;
      else if (strcasecmp(value, "marker") == 0)
        display = HD_DISPLAY_MARKER;
      else if (strcasecmp(value, "run-in") == 0)
        display = HD_DISPLAY_RUN_IN;
      else if (strcasecmp(value, "table") == 0)
        display = HD_DISPLAY_TABLE;
      else if (strcasecmp(value, "table-caption") == 0)
        display = HD_DISPLAY_TABLE_CAPTION;
      else if (strcasecmp(value, "table-cell") == 0)
        display = HD_DISPLAY_TABLE_CELL;
      else if (strcasecmp(value, "table-column") == 0)
        display = HD_DISPLAY_TABLE_COLUMN;
      else if (strcasecmp(value, "table-column-group") == 0)
        display = HD_DISPLAY_TABLE_COLUMN_GROUP;
      else if (strcasecmp(value, "table-footer-group") == 0)
        display = HD_DISPLAY_TABLE_FOOTER_GROUP;
      else if (strcasecmp(value, "table-header-group") == 0)
        display = HD_DISPLAY_TABLE_HEADER_GROUP;
      else if (strcasecmp(value, "table-row") == 0)
        display = HD_DISPLAY_TABLE_ROW;
      else if (strcasecmp(value, "table-row-group") == 0)
        display = HD_DISPLAY_TABLE_ROW_GROUP;
      else
      {
        status = -1;
	fprintf(stderr, "Unknown display value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "empty-cells") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "float") == 0)
    {
      if (strcasecmp(value, "none") == 0)
        float_ = HD_FLOAT_NONE;
      else if (strcasecmp(value, "left") == 0)
        float_ = HD_FLOAT_LEFT;
      else if (strcasecmp(value, "right") == 0)
        float_ = HD_FLOAT_RIGHT;
      else
      {
        status = -1;
	fprintf(stderr, "Unknown float value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "font") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
	if (strcasecmp(subvalue, "normal") == 0)
	{
	  switch (pos)
	  {
	    case 0 : // Style
	        font_style = HD_FONTSTYLE_NORMAL;
		break;

	    case 1 : // Variant
	        font_variant = HD_FONTVARIANT_NORMAL;
		break;

	    case 2 : // Weight
	        font_weight = HD_FONTWEIGHT_NORMAL;
		break;

            default :
	        status = -1;
	        fputs("Unknown font value \"normal\"!\n", stderr);
		break;
	  }

	  pos ++;
	}
	else if (strcasecmp(subvalue, "italic") == 0)
	{
	  font_style = HD_FONTSTYLE_ITALIC;
	  pos = 1;
	}
	else if (strcasecmp(subvalue, "oblique") == 0)
	{
	  font_style = HD_FONTSTYLE_OBLIQUE;
	  pos = 1;
	}
	else if (strcasecmp(subvalue, "small-caps") == 0)
	{
	  font_variant = HD_FONTVARIANT_SMALL_CAPS;
	  pos = 2;
	}
	else if (strcasecmp(subvalue, "bold") == 0 ||
	         strcasecmp(subvalue, "600") == 0 ||
	         strcasecmp(subvalue, "700") == 0 ||
	         strcasecmp(subvalue, "800") == 0 ||
	         strcasecmp(subvalue, "900") == 0)
	{
	  font_weight = HD_FONTWEIGHT_BOLD;
	}
	else if (strcasecmp(subvalue, "bolder") == 0)
	{
	  font_weight = HD_FONTWEIGHT_BOLDER;
	}
	else if (strcasecmp(subvalue, "lighter") == 0)
	{
	  font_weight = HD_FONTWEIGHT_LIGHTER;
	}
	else if (strcasecmp(subvalue, "100") == 0 ||
	         strcasecmp(subvalue, "200") == 0 ||
	         strcasecmp(subvalue, "300") == 0 ||
	         strcasecmp(subvalue, "400") == 0 ||
	         strcasecmp(subvalue, "500") == 0)
	{
	  font_weight = HD_FONTWEIGHT_NORMAL;
	}
	else if (isdigit(subvalue[0]))
	{
	  char	*lh;			// Line height, if available


          if ((lh = strchr(subvalue, '/')) != NULL)
	    *lh++ = '\0';

          font_size = get_length(subvalue, font_size, &relative);

	  if (relative)
	  {
	    if (font_size_rel)
	      free(font_size_rel);

	    font_size_rel = strdup(subvalue);
	  }

          if (lh)
	    line_height = get_length(lh, 1.0f);
	}
	else
	{
	  if (font_family)
	    free(font_family);

	  font_family = strdup(subvalue);
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
	pos ++;
      }
    }
    else if (strcasecmp(name, "font-family") == 0)
    {
      if (font_family)
	free(font_family);

      font_family = strdup(value);
    }
    else if (strcasecmp(name, "font-size") == 0)
    {
       char	*lh;			// Line height, if available


       if ((lh = strchr(value, '/')) != NULL)
	 *lh++ = '\0';

       font_size = get_length(value, font_size, &relative);

       if (relative)
       {
	 if (font_size_rel)
	   free(font_size_rel);

	 font_size_rel = strdup(subvalue);
       }

       if (lh)
	 line_height = get_length(lh, 1.0f);
    }
    else if (strcasecmp(name, "font-size-adjust") == 0)
    {
      // Not implemented
    }
    else if (strcasecmp(name, "font-stretch") == 0)
    {
      // Not implemented
    }
    else if (strcasecmp(name, "font-style") == 0)
    {
      if (strcasecmp(value, "normal") == 0)
      {
        font_style = HD_FONTSTYLE_NORMAL;
      }
      else if (strcasecmp(value, "italic") == 0)
      {
	font_style = HD_FONTSTYLE_ITALIC;
      }
      else if (strcasecmp(value, "oblique") == 0)
      {
	font_style = HD_FONTSTYLE_OBLIQUE;
      }
      else
      {
        status = -1;
	fprintf(stderr, "Unknown font-style value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "font-variant") == 0)
    {
      if (strcasecmp(value, "normal") == 0)
      {
        font_variant = HD_FONTVARIANT_NORMAL;
      }
      else if (strcasecmp(value, "small-caps") == 0)
      {
	font_variant = HD_FONTVARIANT_SMALL_CAPS;
      }
      else
      {
        status = -1;
	fprintf(stderr, "Unknown font-variant value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "font-weight") == 0)
    {
      if (strcasecmp(value, "bold") == 0 ||
	  strcasecmp(value, "600") == 0 ||
	  strcasecmp(value, "700") == 0 ||
	  strcasecmp(value, "800") == 0 ||
	  strcasecmp(value, "900") == 0)
      {
	font_weight = HD_FONTWEIGHT_BOLD;
      }
      else if (strcasecmp(value, "bolder") == 0)
      {
	font_weight = HD_FONTWEIGHT_BOLDER;
      }
      else if (strcasecmp(value, "lighter") == 0)
      {
	font_weight = HD_FONTWEIGHT_LIGHTER;
      }
      else if (strcasecmp(value, "normal") == 0 ||
               strcasecmp(value, "100") == 0 ||
	       strcasecmp(value, "200") == 0 ||
	       strcasecmp(value, "300") == 0 ||
	       strcasecmp(value, "400") == 0 ||
	       strcasecmp(value, "500") == 0)
      {
	font_weight = HD_FONTWEIGHT_NORMAL;
      }
      else
      {
        status = -1;
	fprintf(stderr, "Unknown font-weight value \"%s\"!\n", value);
      }
    }
    else if (strcasecmp(name, "height") == 0)
    {
      height = get_length(value, 100.0f, &relative);

      if (relative)
      {
        if (height_rel)
	  free(height_rel);

	height_rel = strdup(value);
      }
      else if (height_rel)
      {
        free(height_rel);
	height_rel = NULL;
      }
    }
    else if (strcasecmp(name, "letter-spacing") == 0)
    {
      letter_spacing = get_length(value, 0.0f);
    }
    else if (strcasecmp(name, "line-height") == 0)
    {
      if (strcasecmp(value, "normal") == 0)
        line_height = 1.2f;
      else
        line_height = get_length(value, 1.0f);
    }
    else if (strcasecmp(name, "list-style") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        if (strcasecmp(subvalue, "none") == 0)
	{
	  switch (pos)
	  {
	    case 0 : // list-style-type
	        list_style_type = HD_LISTSTYLETYPE_NONE;
		break;

	    case 1 : // list-style-image
	        if (list_style_image)
		  free(list_style_image);

	        list_style_image = NULL;
		break;
	  }

	  pos ++;
	}
        else if (strcasecmp(subvalue, "disc") == 0 ||
	         strcasecmp(subvalue, "circle") == 0 ||
	         strcasecmp(subvalue, "square") == 0 ||
	         strcasecmp(subvalue, "decimal") == 0 ||
	         strcasecmp(subvalue, "lower-roman") == 0 ||
	         strcasecmp(subvalue, "upper-roman") == 0 ||
	         strcasecmp(subvalue, "lower-alpha") == 0 ||
	         strcasecmp(subvalue, "upper-alpha") == 0)
	{
	  list_style_type = get_list_style_type(subvalue);
	  pos = 1;
	}
        else if (strcasecmp(subvalue, "inside") == 0)
	{
	  list_style_position = HD_LISTSTYLEPOSITION_INSIDE;
	}
        else if (strcasecmp(subvalue, "outside") == 0)
	{
	  list_style_position = HD_LISTSTYLEPOSITION_OUTSIDE;
	}
        else if (strncasecmp(subvalue, "url(", 4) == 0)
	{
	  char	*paren;		// Closing parenthesis


	  if (list_style_image)
	    free(list_style_image);

          if ((paren = strrchr(subvalue, ')')) != NULL)
	    *paren = '\0';

          list_style_image = strdup(subvalue + 4);
	}
	else
	{
	  // Unknown value...
	  fprintf(stderr, "Unknown list-style value \"%s\"!\n", subvalue);
	  status = -1;
	}

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "list-style-image") == 0)
    {
      if (strncasecmp(value, "url(", 4) == 0)
      {
	char	*paren;		// Closing parenthesis


	if (list_style_image)
	  free(list_style_image);

        if ((paren = strrchr(value, ')')) != NULL)
	  *paren = '\0';

        list_style_image = strdup(value + 4);
      }
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown list-style-image value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "list-style-position") == 0)
    {
      if (strcasecmp(value, "inside") == 0)
      {
	list_style_position = HD_LISTSTYLEPOSITION_INSIDE;
      }
      else if (strcasecmp(value, "outside") == 0)
      {
	list_style_position = HD_LISTSTYLEPOSITION_OUTSIDE;
      }
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown list-style-position value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "list-style-type") == 0)
    {
      if (strcasecmp(value, "disc") == 0 ||
	  strcasecmp(value, "circle") == 0 ||
	  strcasecmp(value, "square") == 0 ||
	  strcasecmp(value, "decimal") == 0 ||
	  strcasecmp(value, "lower-roman") == 0 ||
	  strcasecmp(value, "upper-roman") == 0 ||
	  strcasecmp(value, "lower-alpha") == 0 ||
	  strcasecmp(value, "upper-alpha") == 0)
      {
	list_style_type = get_list_style_type(value);
      }
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown list-style-type value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "margin") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        length = get_length(subvalue, 100.0f, &relative);

        switch (pos)
	{
	  case 0 :
	      for (i = HD_POS_BOTTOM; i <= HD_POS_TOP; i ++)
	      {
	        margin[i] = length;

		if (relative)
		{
		  if (margin_rel[i])
		    free(margin_rel[i]);

		  margin_rel[i] = strdup(subvalue);
		}
		else if (margin_rel[i])
		{
		  free(margin_rel[i]);
		  margin_rel[i] = NULL;
		}
	      }
	      break;

	  case 1 :
	      for (i = HD_POS_LEFT; i <= HD_POS_RIGHT; i ++)
	      {
	        margin[i] = length;

		if (relative)
		{
		  if (margin_rel[i])
		    free(margin_rel[i]);

		  margin_rel[i] = strdup(subvalue);
		}
		else if (margin_rel[i])
		{
		  free(margin_rel[i]);
		  margin_rel[i] = NULL;
		}
	      }
	      break;

	  case 2 :
	      margin[HD_POS_BOTTOM] = length;

	      if (relative)
	      {
		if (margin_rel[HD_POS_BOTTOM])
		  free(margin_rel[HD_POS_BOTTOM]);

		margin_rel[HD_POS_BOTTOM] = strdup(subvalue);
	      }
	      else if (margin_rel[HD_POS_BOTTOM])
	      {
		free(margin_rel[HD_POS_BOTTOM]);
		margin_rel[HD_POS_BOTTOM] = NULL;
	      }
	      break;

	  case 3 :
	      margin[HD_POS_RIGHT] = length;

	      if (relative)
	      {
		if (margin_rel[HD_POS_RIGHT])
		  free(margin_rel[HD_POS_RIGHT]);

		margin_rel[HD_POS_RIGHT] = strdup(subvalue);
	      }
	      else if (margin_rel[HD_POS_RIGHT])
	      {
		free(margin_rel[HD_POS_RIGHT]);
		margin_rel[HD_POS_RIGHT] = NULL;
	      }
	      break;
        }

	pos ++;

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "margin-bottom") == 0 ||
             strcasecmp(name, "margin-left") == 0 ||
	     strcasecmp(name, "margin-right") == 0 ||
	     strcasecmp(name, "margin-top") == 0)
    {
      pos = get_pos(name);

      margin[pos] = get_length(value, 100.0f, &relative);

      if (relative)
      {
        if (margin_rel[pos])
	  free(margin_rel[pos]);

	margin_rel[pos] = strdup(value);
      }
      else if (margin_rel[pos])
      {
        free(margin_rel[pos]);
	margin_rel[pos] = NULL;
      }
    }
    else if (strcasecmp(name, "marker-offset") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "marks") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "max-height") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "max-width") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "min-height") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "min-width") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "orphans") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "outline") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...

	// Advance to the next sub-value...
	subvalue = valueptr;
	pos ++;
      }
    }
    else if (strcasecmp(name, "outline-color") == 0)
    {
    }
    else if (strcasecmp(name, "outline-style") == 0)
    {
    }
    else if (strcasecmp(name, "outline-width") == 0)
    {
    }
    else if (strcasecmp(name, "padding") == 0)
    {
      // Loop until we have exhausted the value string...
      subvalue = value;
      pos      = 0;

      while (*subvalue)
      {
        // Get the next sub-value...
	valueptr = get_subvalue(valueptr);

        // Process it...
        length = get_length(subvalue, 100.0f, &relative);

        switch (pos)
	{
	  case 0 :
	      for (i = HD_POS_BOTTOM; i <= HD_POS_TOP; i ++)
	      {
	        padding[i] = length;

		if (relative)
		{
		  if (padding_rel[i])
		    free(padding_rel[i]);

		  padding_rel[i] = strdup(subvalue);
		}
		else if (padding_rel[i])
		{
		  free(padding_rel[i]);
		  padding_rel[i] = NULL;
		}
	      }
	      break;

	  case 1 :
	      for (i = HD_POS_LEFT; i <= HD_POS_RIGHT; i ++)
	      {
	        padding[i] = length;

		if (relative)
		{
		  if (padding_rel[i])
		    free(padding_rel[i]);

		  padding_rel[i] = strdup(subvalue);
		}
		else if (padding_rel[i])
		{
		  free(padding_rel[i]);
		  padding_rel[i] = NULL;
		}
	      }
	      break;

	  case 2 :
	      padding[HD_POS_BOTTOM] = length;

	      if (relative)
	      {
		if (padding_rel[HD_POS_BOTTOM])
		  free(padding_rel[HD_POS_BOTTOM]);

		padding_rel[HD_POS_BOTTOM] = strdup(subvalue);
	      }
	      else if (padding_rel[HD_POS_BOTTOM])
	      {
		free(padding_rel[HD_POS_BOTTOM]);
		padding_rel[HD_POS_BOTTOM] = NULL;
	      }
	      break;

	  case 3 :
	      padding[HD_POS_RIGHT] = length;

	      if (relative)
	      {
		if (padding_rel[HD_POS_RIGHT])
		  free(padding_rel[HD_POS_RIGHT]);

		padding_rel[HD_POS_RIGHT] = strdup(subvalue);
	      }
	      else if (padding_rel[HD_POS_RIGHT])
	      {
		free(padding_rel[HD_POS_RIGHT]);
		padding_rel[HD_POS_RIGHT] = NULL;
	      }
	      break;
        }

	pos ++;

	// Advance to the next sub-value...
	subvalue = valueptr;
      }
    }
    else if (strcasecmp(name, "padding-bottom") == 0 ||
             strcasecmp(name, "padding-left") == 0 ||
	     strcasecmp(name, "padding-right") == 0 ||
	     strcasecmp(name, "padding-top") == 0)
    {
      pos = get_pos(name);

      padding[pos] = get_length(value, 100.0f, &relative);

      if (relative)
      {
        if (padding_rel[pos])
	  free(padding_rel[pos]);

	padding_rel[pos] = strdup(value);
      }
      else if (padding_rel[pos])
      {
        free(padding_rel[pos]);
	padding_rel[pos] = NULL;
      }
    }
    else if (strcasecmp(name, "page") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "page-break-after") == 0)
    {
      page_break_after = get_page_break(value);
    }
    else if (strcasecmp(name, "page-break-before") == 0)
    {
      page_break_before = get_page_break(value);
    }
    else if (strcasecmp(name, "page-break-inside") == 0)
    {
      page_break_inside = get_page_break(value);
    }
    else if (strcasecmp(name, "position") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "quotes") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "size") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "text-align") == 0)
    {
      if (strcasecmp(value, "left") == 0)
        text_align = HD_TEXTALIGN_LEFT;
      else if (strcasecmp(value, "center") == 0)
        text_align = HD_TEXTALIGN_CENTER;
      else if (strcasecmp(value, "right") == 0)
        text_align = HD_TEXTALIGN_RIGHT;
      else if (strcasecmp(value, "justify") == 0)
        text_align = HD_TEXTALIGN_JUSTIFY;
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown text-align value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "text-decoration") == 0)
    {
      if (strcasecmp(value, "none") == 0)
        text_decoration = HD_TEXTDECORATION_NONE;
      else if (strcasecmp(value, "underline") == 0)
        text_decoration = HD_TEXTDECORATION_UNDERLINE;
      else if (strcasecmp(value, "overline") == 0)
        text_decoration = HD_TEXTDECORATION_OVERLINE;
      else if (strcasecmp(value, "line-through") == 0)
        text_decoration = HD_TEXTDECORATION_LINE_THROUGH;
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown text-decoration value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "text-indent") == 0)
    {
      text_indent = get_length(value, 100.0f, &relative);

      if (relative)
      {
        if (text_indent_rel)
	  free(text_indent_rel);

	text_indent_rel = strdup(value);
      }
      else if (text_indent_rel)
      {
        free(text_indent_rel);
	text_indent_rel = NULL;
      }
    }
    else if (strcasecmp(name, "text-shadow") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "text-transform") == 0)
    {
      if (strcasecmp(value, "none") == 0)
        text_transform = HD_TEXTTRANSFORM_NONE;
      else if (strcasecmp(value, "capitalize") == 0)
        text_transform = HD_TEXTTRANSFORM_CAPITALIZE;
      else if (strcasecmp(value, "uppercase") == 0)
        text_transform = HD_TEXTTRANSFORM_UPPERCASE;
      else if (strcasecmp(value, "lowercase") == 0)
        text_transform = HD_TEXTTRANSFORM_LOWERCASE;
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown text-transform value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "unicode-bidi") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "vertical-align") == 0)
    {
      if (strcasecmp(value, "baseline") == 0)
        vertical_align = HD_VERTICALALIGN_BASELINE;
      else if (strcasecmp(value, "sub") == 0)
        vertical_align = HD_VERTICALALIGN_SUB;
      else if (strcasecmp(value, "super") == 0)
        vertical_align = HD_VERTICALALIGN_SUPER;
      else if (strcasecmp(value, "top") == 0)
        vertical_align = HD_VERTICALALIGN_TOP;
      else if (strcasecmp(value, "text-top") == 0)
        vertical_align = HD_VERTICALALIGN_TEXT_TOP;
      else if (strcasecmp(value, "middle") == 0)
        vertical_align = HD_VERTICALALIGN_MIDDLE;
      else if (strcasecmp(value, "bottom") == 0)
        vertical_align = HD_VERTICALALIGN_BOTTOM;
      else if (strcasecmp(value, "text-bottom") == 0)
        vertical_align = HD_VERTICALALIGN_TEXT_BOTTOM;
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown vertical-align value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "white-space") == 0)
    {
      if (strcasecmp(value, "normal") == 0)
        white_space = HD_WHITESPACE_NORMAL;
      else if (strcasecmp(value, "pre") == 0)
        white_space = HD_WHITESPACE_PRE;
      else if (strcasecmp(value, "nowrap") == 0)
        white_space = HD_WHITESPACE_NOWRAP;
      else
      {
	// Unknown value...
	fprintf(stderr, "Unknown white-space value \"%s\"!\n", value);
	status = -1;
      }
    }
    else if (strcasecmp(name, "widows") == 0)
    {
      // NOT IMPLEMENTED
    }
    else if (strcasecmp(name, "width") == 0)
    {
      width = get_length(value, 100.0f, &relative);

      if (relative)
      {
        if (width_rel)
	  free(width_rel);

	width_rel = strdup(value);
      }
      else if (width_rel)
      {
        free(width_rel);
	width_rel = NULL;
      }
    }
    else if (strcasecmp(name, "word-spacing") == 0)
    {
      word_spacing = get_length(value, 0.0f);
    }
    else if (strcasecmp(name, "z-index") == 0)
    {
      // NOT IMPLEMENTED
    }
    else
    {
      fprintf(stderr, "Unknown style property \"%s\"!\n", name);
      status = -1;
    }
  }

  return (status);
}


//
// 'hdStyle::update()' - Update relative style definitions.
//

void
hdStyle::update(hdStyleSheet *css)	// I - Stylesheet
{
}


//
// 'hdStyleFont::hdStyleFont()' - Create a new font record.
//

hdStyleFont::hdStyleFont(hdStyleSheet   *css,	// I - Stylesheet
        		 hdFontFace     t,	// I - Typeface
			 hdFontInternal s,	// I - Style
			 const char     *n)	// I - Font name
{
}


//
// 'hdStyleFont::~hdStyleFont()' - Destroy a font record.
//

hdStyleFont::~hdStyleFont()
{
}


//
// 'hdStyleFont::width()' - Compute the width of a string.
//

float					// O - Unscaled width
hdStyleFont::width(const char *s)	// I - String to measure
{
  return (0.0f);
}


//
// 'hdStyleSheet::hdStyleSheet()' - Create a new stylesheet.
//

hdStyleSheet::hdStyleSheet()
{
}


//
// 'hdStyleSheet::~hdStyleSheet()' - Destroy a stylesheet.
//

hdStyleSheet::~hdStyleSheet()
{
}


//
// 'hdStyleSheet::add_style()' - Add a style to a stylesheet.
//

void
hdStyleSheet::add_style(hdStyle *s)	// I - New style
{
}


//
// 'hdStyleSheet::find_font()' - Find a font for the given style.
//

hdStyleFont *				// O - Font record
hdStyleSheet::find_font(hdStyle *s)	// I - Style record
{
  return ((hdStyleFont *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                tree node.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdTree *t)	// I - Tree node
{
  return ((hdStyle *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                selectors.
//

hdStyle *				// O - Style record
find_style(int        nsels,		// I - Number of selectors
           hdSelector *sels)		// I - Selectors
{
  return ((hdStyle *)0);
}


//
// 'hdStyleSheet::load()' - Load a stylesheet from the given file.
//

int					// O - 0 on success, -1 on failure
hdStyleSheet::load(hdFile     *f,	// I - File to read from
                   const char *path)	// I - Search path for included files
{
  return (0);
}


//
// 'hdStyleSheet::set_charset()' - Set the character set to use for the
//                                 document.
//

void
hdStyleSheet::set_charset(const char *cs)// I - Character set name
{
}


//
// 'hdStyleSheet::update()' - Update all relative stylesheet data.
//

void
hdStyleSheet::update()
{
}


//
// 'hdStyleSheet::update_styles()' - Update all relative style data.
//

void
hdStyleSheet::update_styles()
{
}


//
// End of "$Id: style.cxx,v 1.2 2002/02/05 17:47:58 mike Exp $".
//
