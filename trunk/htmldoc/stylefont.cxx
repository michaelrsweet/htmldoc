//
// "$Id: stylefont.cxx,v 1.3 2002/02/23 04:03:30 mike Exp $"
//
//   CSS font routines for HTMLDOC, a HTML document processing program.
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

#include "htmldoc.h"
#include "hdstring.h"
#include <stdlib.h>


//
// 'hdStyleFont::hdStyleFont()' - Create a new font record.
//

hdStyleFont::hdStyleFont(hdStyleSheet   *css,	// I - Stylesheet
        		 hdFontFace     t,	// I - Typeface
			 hdFontInternal s,	// I - Style
			 const char     *n)	// I - Font name
{
  int			i, j;			// Looping vars...
  char			filename[1024];		// Filename
  hdFile		*fp;			// File pointer
  static const char	*styles[][2] =		// PostScript style suffixes
			{
			  { "-Roman",		"-Regular" },
			  { "-Bold",		"-Medium" },
			  { "-Italic",		"-Oblique" },
			  { "-BoldItalic",	"-BoldOblique" }
			};


  // Clear the structure...
  memset(this, 0, sizeof(hdStyleFont));

  // Initialize variables...
  typeface = t;
  style    = s;
  encoding = css->encoding;
  widths   = new float[css->num_glyphs];

  memset(widths, 0, sizeof(float) * css->num_glyphs);

  // Map font family to base font...
  if (strcasecmp(n, "serif") == 0)
    n = "Times";
  else if (strcasecmp(n, "sans-serif") == 0)
    n = "Helvetica";
  else if (strcasecmp(n, "monospace") == 0)
    n = "Courier";
  else if (strcasecmp(n, "symbol") == 0)
    n = "Symbol";
  else if (strcasecmp(n, "cursive") == 0)
    n = "ZapfChancery";

  // Now try to find the font...
  for (fp = NULL, i = 0; i < 4; i ++)
  {
    for (j = 0; j < 2; j ++)
    {
      // Try a variation of the font name...
      snprintf(filename, sizeof(filename), "%s/fonts/%s%s.afm",
               hdGlobal.datadir, n, styles[i][j]);
      if ((fp = hdFile::open(filename, HD_FILE_READ)) != NULL)
        break;
    }

    if (fp != NULL)
      break;
  }

  if (fp == NULL)
  {
    // Use the font name without a suffix...
    snprintf(filename, sizeof(filename), "%s/fonts/%s.afm",
             hdGlobal.datadir, n);
    fp = hdFile::open(filename, HD_FILE_READ);
  }

  if (fp != NULL)
  {
    // Read the AFM file...
    read_afm(fp, css);
    delete fp;

    // Change the extension to ".pfa" and save the font filename for later
    // use as needed...
    strcpy(filename + strlen(filename) - 4, ".pfa");
    font_file = strdup(filename);
  }
}


//
// 'hdStyleFont::~hdStyleFont()' - Destroy a font record.
//

hdStyleFont::~hdStyleFont()
{
  if (num_kerns)
    delete[] kerns;

  delete[] widths;

  if (ps_name)
    free(ps_name);

  if (full_name)
    free(full_name);

  if (font_file)
    free(font_file);
}


//
// 'hdStyleFont::compare_kerns()' - Compare two kerning pairs...
//

int						// O - Result of comparison
hdStyleFont::compare_kerns(hdFontKernPair *a,	// I - First kerning pair
                           hdFontKernPair *b)	// I - Second kerning pair
{
  if (a->first != b->first)
    return (b->first - a->first);
  else
    return (b->second - a->second);
}


//
// 'hdStyleFont::get_kerning()' - Get the kerning list for a string.
//

int						// O - Number of kerning entries
hdStyleFont::get_kerning(const char *s,		// I - String to kern
                         float      *tk,	// O - Total kerning adjustment
                         float      **kl)	// O - Kerning adjustments
{
//  qsort(kerns, num_kerns, sizeof(hdFontKernPair),
//          (hdCompareFunc)compare_kerns);

  *tk = 0.0f;
  *kl = NULL;

  return (0);
}


//
// 'hdStyleFont::get_width()' - Compute the width of a string.
//

float					// O - Unscaled width
hdStyleFont::get_width(const char *s)	// I - String to measure
{
  int	ch;				// Character in string
  float	w;				// Current width


//  printf("get_width(\"%s\")\n", s);

  for (w = 0.0f; *s; s ++)
  {
    ch = *s & 255;

//    printf("    widths[%d] = %.3f\n", ch, widths[ch]);

    w += widths[ch];
  }

//  printf("    returning %.3f...\n", w);

  return (w);
}


//
// 'hdStyleFont::read_afm()' - Read a Type1 AFM file.
//

int					// O - 0 on success, -1 on error
hdStyleFont::read_afm(hdFile       *fp,	// I - File to read from
                      hdStyleSheet *css)// I - Stylesheet
{
  char		line[255],		// Line from file
		*lineptr,		// Pointer into line
		value[32],		// String value in line
		value2[32];		// Second string value in line
  int		number,			// Numeric value in line
		glyph,			// Glyph index
		glyph2;			// Second glyph index
  int		alloc_kerns;		// Allocated kerning pairs...
  hdFontKernPair *temp;			// Pointer to kerning pairs...


  // Loop through the AFM file...
  alloc_kerns = num_kerns;

  while (fp->gets(line, sizeof(line)) != NULL)
  {
    // Get the initial keyword...
    if ((lineptr = strchr(line, ' ')) != NULL)
    {
      // Nul-terminate the keyword, and then skip any remaining whitespace...
      while (isspace(*lineptr))
        *lineptr++ = '\0';
    }

    // See what we have...
    if (strcmp(line, "FontName") == 0)
    {
      // Get PostScript font name...
      ps_name = strdup(lineptr);
    }
    else if (strcmp(line, "FullName") == 0)
    {
      // Human-readable font name...
      full_name = strdup(lineptr);
    }
    else if (strcmp(line, "IsFixedPitch") == 0)
    {
      // Fixed-pitch font?
      fixed_width = strcmp(lineptr, "true") == 0;
    }
    else if (strcmp(line, "UnderlinePosition") == 0)
    {
      // Where to place the underline...
      ul_position = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "UnderlineThickness") == 0)
    {
      // How thick to make the underline...
      ul_thickness = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "CapHeight") == 0)
    {
      // How tall are uppercase letters?
      cap_height = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "XHeight") == 0)
    {
      // How tall are lowercase letters?
      x_height = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "Descender") == 0)
    {
      // How low do the descenders go?
      descender = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "Ascender") == 0)
    {
      // How high do the ascenders go?
      ascender = (float)atof(lineptr) * 0.001f;
    }
    else if (strcmp(line, "C") == 0)
    {
      // Get the character width and glyph name...
      if (sscanf(lineptr, "%*d%*s%*s%d%*s%*s%31s", &number, value) != 2)
        continue;

      if ((glyph = css->get_glyph(value)) < 0)
        continue;

      widths[glyph] = number * 0.001f;
    }
    else if (strcmp(line, "KPX") == 0)
    {
      // Get kerning data...
      if (sscanf(lineptr, "%31s%31s%d", value, value2, &number) != 3)
        continue;

      if ((glyph = css->get_glyph(value)) < 0)
        continue;

      if ((glyph2 = css->get_glyph(value2)) < 0)
        continue;

      if (num_kerns >= alloc_kerns)
      {
        // Allocate more kerning pairs...
	temp = new hdFontKernPair[alloc_kerns + 50];

	if (alloc_kerns)
	{
	  memcpy(temp, kerns, alloc_kerns * sizeof(hdFontKernPair));
	  delete[] kerns;
	}

        kerns       = temp;
        alloc_kerns += 50;
      }

      temp = kerns + num_kerns;
      num_kerns ++;

      temp->first  = glyph;
      temp->second = glyph2;
      temp->adjust = number * 0.001f;
    }
  }

  // Sort the kerning table as needed...
  if (num_kerns > 1)
    qsort(kerns, num_kerns, sizeof(hdFontKernPair),
          (hdCompareFunc)compare_kerns);

  return (0);
}


//
// 'hdStyleFont::read_pfm()' - Read a Type1 PFM file.
//

int					// O - 0 on success, -1 on error
hdStyleFont::read_pfm(hdFile       *fp,	// I - File to read from
                      hdStyleSheet *css)// I - Stylesheet
{
  return (0);
}


//
// 'hdStyleFont::read_ttf()' - Read a TrueType font file.
//

int					// O - 0 on success, -1 on error
hdStyleFont::read_ttf(hdFile       *fp,	// I - File to read from
                      hdStyleSheet *css)// I - Stylesheet
{
  return (0);
}


//
// End of "$Id: stylefont.cxx,v 1.3 2002/02/23 04:03:30 mike Exp $".
//
