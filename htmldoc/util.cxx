/*
 * "$Id: util.cxx,v 1.1.2.17 2004/02/06 03:51:09 mike Exp $"
 *
 *   Utility functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2004 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   format_number()   - Format a number into arabic numerals, roman numerals,
 *                       or letters.
 *   get_color()       - Get a standard color value...
 *   get_format()      - Convert an old "fff" format string to the new format.
 *   get_fmt()         - Convert a new format string to the old "fff" format.
 *   get_measurement() - Get a size measurement in inches, points, centimeters,
 *                       or millimeters.
 *   set_page_size()   - Set the output page size.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <ctype.h>


/*
 * 'format_number()' - Format a number into arabic numerals, roman numerals,
 *                     or letters.
 */

char *				/* O - String */
format_number(int  n,		/* I - Number */
              char f)		/* I - Format */
{
  static const char *ones[10] =	/* Roman numerals, 0-9 */
		{
		  "",	"i",	"ii",	"iii",	"iv",
		  "v",	"vi",	"vii",	"viii",	"ix"
		},
		*tens[10] =	/* Roman numerals, 10-90 */
		{
		  "",	"x",	"xx",	"xxx",	"xl",
		  "l",	"lx",	"lxx",	"lxxx",	"xc"
		},
		*hundreds[10] =	/* Roman numerals, 100-900 */
		{
		  "",	"c",	"cc",	"ccc",	"cd",
		  "d",	"dc",	"dcc",	"dccc",	"cm"
		};
  static const char *ONES[10] =	/* Roman numerals, 0-9 */
		{
		  "",	"I",	"II",	"III",	"IV",
		  "V",	"VI",	"VII",	"VIII",	"IX"
		},
		*TENS[10] =	/* Roman numerals, 10-90 */
		{
		  "",	"X",	"XX",	"XXX",	"XL",
		  "L",	"LX",	"LXX",	"LXXX",	"XC"
		},
		*HUNDREDS[10] =	/* Roman numerals, 100-900 */
		{
		  "",	"C",	"CC",	"CCC",	"CD",
		  "D",	"DC",	"DCC",	"DCCC",	"CM"
		};
  static char	buffer[1024];	/* String buffer */


  switch (f)
  {
    default :
        buffer[0] = '\0';
	break;

    case 'a' :
        if (n >= (26 * 26))
	  buffer[0] = '\0';
        else if (n > 26)
          sprintf(buffer, "%c%c", 'a' + (n / 26) - 1, 'a' + (n % 26) - 1);
        else
          sprintf(buffer, "%c", 'a' + n - 1);
        break;

    case 'A' :
        if (n >= (26 * 26))
	  buffer[0] = '\0';
        else if (n > 26)
          sprintf(buffer, "%c%c", 'A' + (n / 26) - 1, 'A' + (n % 26) - 1);
        else
          sprintf(buffer, "%c", 'A' + n - 1);
        break;

    case '1' :
        sprintf(buffer, "%d", n);
        break;

    case 'i' :
        if (n >= 1000)
	  buffer[0] = '\0';
	else
          sprintf(buffer, "%s%s%s", hundreds[n / 100], tens[(n / 10) % 10],
                  ones[n % 10]);
        break;

    case 'I' :
        if (n >= 1000)
	  buffer[0] = '\0';
	else
          sprintf(buffer, "%s%s%s", HUNDREDS[n / 100], TENS[(n / 10) % 10],
                  ONES[n % 10]);
        break;
  }

  return (buffer);
}


/*
 * 'get_color()' - Get a standard color value...
 */

void
get_color(const uchar *color,	/* I - Color attribute */
          float       *rgb,	/* O - RGB value */
	  int         defblack)	/* I - Default color is black? */
{
  int		i;		/* Looping vars */
  static uchar	tempcolor[8];	/* Temporary holding place for hex colors */
  static struct
  {
    const char	*name;		/* Color name */
    uchar	red,		/* Red value */
		green,		/* Green value */
		blue;		/* Blue value */
  }		colors[] =	/* Color "database" */
  {
    { "aqua",		0,   255, 255 }, /* AKA Cyan */
    { "black",		0,   0,   0 },
    { "blue",		0,   0,   255 },
    { "cyan",		0,   255, 255 },
    { "fuchsia",	255, 0,   255 }, /* AKA Magenta */
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

    if (i == 6)
    {
      // Update the color name to be #RRGGBB instead of RRGGBB...
      tempcolor[0] = '#';
      strcpy((char *)tempcolor + 1, (char *)color);
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
   /*
    * RGB value in hex...
    */

    i = strtol((char *)color + 1, NULL, 16);
    rgb[0] = (i >> 16) / 255.0f;
    rgb[1] = ((i >> 8) & 255) / 255.0f;
    rgb[2] = (i & 255) / 255.0f;
  }
  else
  {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
      if (strcasecmp(colors[i].name, (char *)color) == 0)
	break;

    if (i >= (int)(sizeof(colors) / sizeof(colors[0])))
    {
      if (defblack)
        i = 1; /* Black */
      else
        i = 17; /* White */
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
    if (formats[i])
    {
      free(formats[i]);
      formats[i] = NULL;
    }

    switch (fmt[i])
    {
      case '/' :
          formats[i] = strdup("$PAGE(1)/$PAGES");
          break;

      case ':' :
          formats[i] = strdup("$CHAPTERPAGE(1)/$CHAPTERPAGES");
          break;

      case '1' :
          formats[i] = strdup("$PAGE(1)");
          break;

      case 'a' :
          formats[i] = strdup("$PAGE(a)");
          break;

      case 'A' :
          formats[i] = strdup("$PAGE(A)");
          break;

      case 'c' :
          formats[i] = strdup("$CHAPTER");
          break;

      case 'C' :
          formats[i] = strdup("$CHAPTERPAGE(1)");
          break;

      case 'd' :
          formats[i] = strdup("$DATE");
          break;

      case 'D' :
          formats[i] = strdup("$DATE $TIME");
          break;

      case 'h' :
          formats[i] = strdup("$HEADING");
          break;

      case 'i' :
          formats[i] = strdup("$PAGE(i)");
          break;

      case 'I' :
          formats[i] = strdup("$PAGE(I)");
          break;

      case 'l' :
          formats[i] = strdup("$LOGOIMAGE");
          break;

      case 't' :
          formats[i] = strdup("$TITLE");
          break;

      case 'T' :
          formats[i] = strdup("$TIME");
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


  strcpy(fmt, "...");

  for (i = 0; i < 3; i ++)
    if (formats[i])
      for (j = 0; j < (int)(sizeof(table) / sizeof(table[0])); j ++)
        if (strcmp(formats[i], table[j].format) == 0)
	{
	  fmt[i] = table[j].f;
	  break;
	}

  return (fmt);
}


/*
 * 'get_measurement()' - Get a size measurement in inches, points, centimeters,
 *                       or millimeters.
 */

int				/* O - Measurement in points */
get_measurement(const char *s,	/* I - Measurement string */
                float      mul)	/* I - Multiplier */
{
  float	val;			/* Measurement value */


 /*
  * Get the floating point value of "s" and skip all digits and decimal points.
  */

  val = (float)atof(s);
  while (isdigit(*s) || *s == '.')
    s ++;

 /*
  * Check for a trailing unit specifier...
  */

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


/*
 * 'set_page_size()' - Set the output page size.
 */

void
set_page_size(const char *size)	/* I - Page size string */
{
  float	width,			/* Width in points */
	length;			/* Length in points */
  char	units[255];		/* Units string */


 /*
  * Check for common media sizes...
  */

  if (strcasecmp(size, "letter") == 0 ||
      strcasecmp(size, "a") == 0)
  {
   /*
    * US Letter - 8.5x11 inches (216x279mm).
    */

    PageWidth  = 612;
    PageLength = 792;
  }
  else if (strcasecmp(size, "legal") == 0)
  {
   /*
    * US Legal - 8.5x14 inches (216x356mm).
    */

    PageWidth  = 612;
    PageLength = 1008;
  }
  else if (strcasecmp(size, "tabloid") == 0 ||
           strcasecmp(size, "b") == 0)
  {
   /*
    * US Tabloid - 11x17 inches (279x432mm).
    */

    PageWidth  = 792;
    PageLength = 1224;
  }
  else if (strcasecmp(size, "a4") == 0)
  {
   /*
    * European standard A4 - 210x297mm (8.27x11.69 inches).
    */

    PageWidth  = 595;
    PageLength = 842;
  }
  else if (strcasecmp(size, "a3") == 0)
  {
   /*
    * European standard A3 - 297x420mm (11.69x16.54 inches).
    */

    PageWidth  = 842;
    PageLength = 1190;
  }
  else if (strcasecmp(size, "universal") == 0)
  {
   /*
    * "Universal" size - 8.27x11.00 inches (210x279mm).
    */

    PageWidth  = 595;
    PageLength = 792;
  }
  else if (sscanf(size, "%fx%f%s", &width, &length, units) >= 2)
  {
   /*
    * Custom size...
    */

    if (strcasecmp(units, "mm") == 0)
    {
      PageWidth  = (int)(72.0 * width / 25.4);
      PageLength = (int)(72.0 * length / 25.4);
    }
    else if (strcasecmp(units, "cm") == 0)
    {
      PageWidth  = (int)(72.0 * width / 2.54);
      PageLength = (int)(72.0 * length / 2.54);
    }
    else if (strncasecmp(units, "in", 2) == 0)
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


/*
 * End of "$Id: util.cxx,v 1.1.2.17 2004/02/06 03:51:09 mike Exp $".
 */
