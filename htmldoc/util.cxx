//
// Utility functions for HTMLDOC, a HTML document processing program.
//
// Copyright © 2011-2024 by Michael R Sweet.
// Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
//
// This program is free software.  Distribution and use rights are outlined in
// the file "COPYING".
//

#include "htmldoc.h"
#include <ctype.h>


//
// 'format_number()' - Format a number into arabic numerals, roman numerals,
//                     or letters.
//

char *					// O - String
format_number(int  n,			// I - Number
              char f)			// I - Format
{
  static const char *ones[10] =		// Roman numerals, 0-9
		{
		  "",	"i",	"ii",	"iii",	"iv",
		  "v",	"vi",	"vii",	"viii",	"ix"
		},
		*tens[10] =		// Roman numerals, 10-90
		{
		  "",	"x",	"xx",	"xxx",	"xl",
		  "l",	"lx",	"lxx",	"lxxx",	"xc"
		},
		*hundreds[30] =		// Roman numerals, 100-2900
		{
		  "",    "c",    "cc",    "ccc",    "cd",
		  "d",   "dc",   "dcc",   "dccc",   "cm",
		  "m",   "mc",   "cc",    "ccc",    "cd",
		  "m",   "mdc",  "mdcc",  "mdccc",  "mcm",
		  "mm",  "mmc",  "mmcc",  "mmccc",  "mmcd",
		  "mmd", "mmdc", "mmdcc", "mmdccc", "mmcm"
		};
  static const char *ONES[10] =		// Roman numerals, 0-9
		{
		  "",	"I",	"II",	"III",	"IV",
		  "V",	"VI",	"VII",	"VIII",	"IX"
		},
		*TENS[10] =		// Roman numerals, 10-90
		{
		  "",	"X",	"XX",	"XXX",	"XL",
		  "L",	"LX",	"LXX",	"LXXX",	"XC"
		},
		*HUNDREDS[30] =		// Roman numerals, 100-2900
		{
		  "",    "C",    "CC",    "CCC",    "CD",
		  "D",   "DC",   "DCC",   "DCCC",   "CM",
		  "M",   "MC",   "CC",    "CCC",    "CD",
		  "M",   "MDC",  "MDCC",  "MDCCC",  "MCM",
		  "MM",  "MMC",  "MMCC",  "MMCCC",  "MMCD",
		  "MMD", "MMDC", "MMDCC", "MMDCCC", "MMCM"
		};
  static char	buffer[1024];		// String buffer


  switch (f)
  {
    default :
        buffer[0] = '\0';
	break;

    case 'a' :
        if (n > (26 * 26))
          n = (n % (26 * 26)) + 1;

        if (n > 26)
          snprintf(buffer, sizeof(buffer), "%c%c", 'a' + (n / 26) - 1, 'a' + (n % 26) - 1);
        else
          snprintf(buffer, sizeof(buffer), "%c", 'a' + n - 1);
        break;

    case 'A' :
        if (n > (26 * 26))
          n = (n % (26 * 26)) + 1;

        if (n > 26)
          snprintf(buffer, sizeof(buffer), "%c%c", 'A' + (n / 26) - 1, 'A' + (n % 26) - 1);
        else
          snprintf(buffer, sizeof(buffer), "%c", 'A' + n - 1);
        break;

    case '1' :
        snprintf(buffer, sizeof(buffer), "%d", n);
        break;

    case 'i' :
        if (n >= 3000)
          n = ((n - 3000) % 2999) + 1;

	snprintf(buffer, sizeof(buffer), "%s%s%s", hundreds[n / 100], tens[(n / 10) % 10], ones[n % 10]);
        break;

    case 'I' :
        if (n >= 3000)
          n = ((n - 3000) % 2999) + 1;

	snprintf(buffer, sizeof(buffer), "%s%s%s", HUNDREDS[n / 100], TENS[(n / 10) % 10], ONES[n % 10]);
        break;
  }

  return (buffer);
}


//
// 'get_color()' - Get a standard color value...
//

void
get_color(const uchar *color,	// I - Color attribute
          float       *rgb,	// O - RGB value
	  int         defblack)	// I - Default color is black?
{
  int		i;		// Looping vars
  static uchar	tempcolor[8];	// Temporary holding place for hex colors
  static struct
  {
    const char	*name;		// Color name
    uchar	red,		// Red value
		green,		// Green value
		blue;		// Blue value
  }		colors[] =	// Color "database"
  {
    { "aqua",		0,   255, 255 }, // AKA Cyan
    { "black",		0,   0,   0 },
    { "blue",		0,   0,   255 },
    { "cyan",		0,   255, 255 },
    { "fuchsia",	255, 0,   255 }, // AKA Magenta
    { "gray",		128, 128, 128 },
    { "green",		0,   128, 0 },
    { "grey",		128, 128, 128 },
    { "lime",		0,   255, 0 },
    { "magenta",	255, 0,   255 },
    { "maroon",		128, 0,   0 },
    { "navy",		0,   0,   128 },
    { "olive",		128, 128, 0 },
    { "purple",		128, 0,   128 },
    { "red",		255, 0,   0 },
    { "silver",		192, 192, 192 },
    { "teal",		0,   128, 128 },
    { "white",		255, 255, 255 },
    { "yellow",		255, 255, 0 }
  };


  // First, see if this is a hex color with a missing # in front...
  if (strlen((char *)color) == 6)
  {
    for (i = 0; i < 6; i ++)
      if (!isxdigit(color[i]))
        break;

    if (i == 3 || i == 6)
    {
      // Update the color name to be #RRGGBB instead of RRGGBB...
      tempcolor[0] = '#';
      strlcpy((char *)tempcolor + 1, (char *)color, sizeof(tempcolor) - 1);
      color = tempcolor;
    }
  }

  if (!color[0])
  {
    if (defblack)
    {
      rgb[0] = 0.0f;
      rgb[1] = 0.0f;
      rgb[2] = 0.0f;
    }
    else
    {
      rgb[0] = 1.0f;
      rgb[1] = 1.0f;
      rgb[2] = 1.0f;
    }
    return;
  }
  else if (color[0] == '#')
  {
    // RGB value in hex...
    i = (int)strlen((char *)color + 1);
    if (i == 3)
    {
      i      = strtol((char *)color + 1, NULL, 16);
      rgb[0] = (i >> 8) / 15.0f;
      rgb[1] = ((i >> 4) & 15) / 15.0f;
      rgb[2] = (i & 15) / 15.0f;
    }
    else if (i == 6)
    {
      i      = strtol((char *)color + 1, NULL, 16);
      rgb[0] = (i >> 16) / 255.0f;
      rgb[1] = ((i >> 8) & 255) / 255.0f;
      rgb[2] = (i & 255) / 255.0f;
    }
    else if (defblack)
    {
      rgb[0] = rgb[1] = rgb[2] = 0.0f;
    }
    else
    {
      rgb[0] = rgb[1] = rgb[2] = 1.0f;
    }
  }
  else
  {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
      if (strcasecmp(colors[i].name, (char *)color) == 0)
	break;

    if (i >= (int)(sizeof(colors) / sizeof(colors[0])))
    {
      if (defblack)
        i = 1; // Black
      else
        i = 17; // White
    }

    rgb[0] = colors[i].red / 255.0f;
    rgb[1] = colors[i].green / 255.0f;
    rgb[2] = colors[i].blue / 255.0f;
  }
}


//
// 'get_format()' - Convert an old "fff" format string to the new format.
//

void
get_format(const char *fmt,		// I - Old "fff" format
           char       **formats)	// O - New format strings
{
  int	i;				// Looping var


  for (i = 0; i < 3; i ++)
  {
    switch (fmt[i])
    {
      case '/' :
          formats[i] = hd_strdup("$PAGE(1)/$PAGES");
          break;

      case ':' :
          formats[i] = hd_strdup("$CHAPTERPAGE(1)/$CHAPTERPAGES");
          break;

      case '1' :
          formats[i] = hd_strdup("$PAGE(1)");
          break;

      case 'a' :
          formats[i] = hd_strdup("$PAGE(a)");
          break;

      case 'A' :
          formats[i] = hd_strdup("$PAGE(A)");
          break;

      case 'c' :
          formats[i] = hd_strdup("$CHAPTER");
          break;

      case 'C' :
          formats[i] = hd_strdup("$CHAPTERPAGE(1)");
          break;

      case 'd' :
          formats[i] = hd_strdup("$DATE");
          break;

      case 'D' :
          formats[i] = hd_strdup("$DATE $TIME");
          break;

      case 'h' :
          formats[i] = hd_strdup("$HEADING");
          break;

      case 'i' :
          formats[i] = hd_strdup("$PAGE(i)");
          break;

      case 'I' :
          formats[i] = hd_strdup("$PAGE(I)");
          break;

      case 'l' :
          formats[i] = hd_strdup("$LOGOIMAGE");
          break;

      case 'L' :
          formats[i] = hd_strdup("$LETTERHEAD");
          break;

      case 't' :
          formats[i] = hd_strdup("$TITLE");
          break;

      case 'T' :
          formats[i] = hd_strdup("$TIME");
          break;

      case 'u' :
          formats[i] = hd_strdup("$URL");
          break;

      default :
          formats[i] = NULL;
          break;
    }
  }
}


//
// 'get_fmt()' - Convert a new format string to the old "fff" format.
//

const char *				// O - Old format string
get_fmt(char **formats)			// I - New format strings
{
  int		i, j;			// Looping vars
  static char	fmt[4];			// Old format string
  static struct				// Format string conversions...
  {
    char	f;			// Format character
    const char	*format;		// Format string
  }		table[] =
  {
    { '/', "$PAGE(1)/$PAGES" },
    { ':', "$CHAPTERPAGE(1)/$CHAPTERPAGES" },
    { '1', "$PAGE(1)" },
    { 'a', "$PAGE(a)" },
    { 'A', "$PAGE(A)" },
    { 'c', "$CHAPTER" },
    { 'C', "$CHAPTERPAGE(1)" },
    { 'd', "$DATE" },
    { 'D', "$DATE $TIME" },
    { 'h', "$HEADING" },
    { 'i', "$PAGE(i)" },
    { 'I', "$PAGE(I)" },
    { 'l', "$LOGOIMAGE" },
    { 't', "$TITLE" },
    { 'T', "$TIME" }
  };


  // Safe because fmt is 4 chars long
  strlcpy(fmt, "...", sizeof(fmt));

  for (i = 0; i < 3; i ++)
  {
    if (formats[i])
    {
      for (j = 0; j < (int)(sizeof(table) / sizeof(table[0])); j ++)
      {
        if (strcmp(formats[i], table[j].format) == 0)
	{
	  fmt[i] = table[j].f;
	  break;
	}
      }
    }
  }

  return (fmt);
}


//
// 'get_measurement()' - Get a size measurement in inches, points, centimeters,
//                       or millimeters.
//

int					// O - Measurement in points
get_measurement(const char *s,		// I - Measurement string
                float      mul)		// I - Multiplier
{
  float	val;				// Measurement value


  // Get the floating point value of "s" and skip all digits and decimal points.
  val = (float)atof(s);
  while (isdigit(*s) || *s == '.')
    s ++;

  // Check for a trailing unit specifier...
  if (strcasecmp(s, "mm") == 0)
    val *= 72.0f / 25.4f;
  else if (strcasecmp(s, "cm") == 0)
    val *= 72.0f / 2.54f;
  else if (strncasecmp(s, "in", 2) == 0)
    val *= 72.0f;
  else
    val *= mul;

  return ((int)val);
}


//
// 'set_page_size()' - Set the output page size.
//

void
set_page_size(const char *size)		// I - Page size string
{
  float	width,				// Width in points
	length;				// Length in points
  char	units[255];			// Units string


  // Check for common media sizes...
  if (!strcasecmp(size, "letter") || !strcasecmp(size, "a"))
  {
    // US Letter - 8.5x11 inches (216x279mm).
    PageWidth  = 612;
    PageLength = 792;
  }
  else if (!strcasecmp(size, "legal"))
  {
    // US Legal - 8.5x14 inches (216x356mm).
    PageWidth  = 612;
    PageLength = 1008;
  }
  else if (!strcasecmp(size, "tabloid") || !strcasecmp(size, "b"))
  {
    // US Tabloid - 11x17 inches (279x432mm).
    PageWidth  = 792;
    PageLength = 1224;
  }
  else if (!strcasecmp(size, "a4"))
  {
    // European standard A4 - 210x297mm (8.27x11.69 inches).
    PageWidth  = 595;
    PageLength = 842;
  }
  else if (!strcasecmp(size, "a3"))
  {
    // European standard A3 - 297x420mm (11.69x16.54 inches).
    PageWidth  = 842;
    PageLength = 1190;
  }
  else if (!strcasecmp(size, "universal"))
  {
    // "Universal" size - 8.27x11.00 inches (210x279mm).
    PageWidth  = 595;
    PageLength = 792;
  }
  else if (sscanf(size, "%fx%f%254s", &width, &length, units) >= 2)
  {
    // Custom size...
    if (!strcasecmp(units, "mm"))
    {
      PageWidth  = (int)(72.0 * width / 25.4);
      PageLength = (int)(72.0 * length / 25.4);
    }
    else if (!strcasecmp(units, "cm"))
    {
      PageWidth  = (int)(72.0 * width / 2.54);
      PageLength = (int)(72.0 * length / 2.54);
    }
    else if (!strncasecmp(units, "in", 2))
    {
      PageWidth  = (int)(72.0 * width);
      PageLength = (int)(72.0 * length);
    }
    else
    {
      PageWidth  = (int)width;
      PageLength = (int)length;
    }
  }
}
