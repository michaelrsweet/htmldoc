/*
 * "$Id$"
 *
 *   ISO-8859-1 conversion routines for HTMLDOC, an HTML document
 *   processing program.
 *
 *   Copyright 1997-2005 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   iso8859()     - Return the 8-bit character value of a glyph name.
 *   iso8859()     - Return the glyph name of an 8-bit character value.
 *   compare_lut() - Compare two glyphs.
 */

/*
 * Include necessary headers.
 */

#include <stdio.h>
#include <stdlib.h>

#include "hdstring.h"
#include "iso8859.h"
#include "types.h"


/*
 * Lookup table structure...
 */

typedef struct
{
  hdChar	name[7],
	value;
} lut_t;

static lut_t	iso8859_numbers[] =
		{
		  { "AElig",	198 },
		  { "Aacute",	193 },
		  { "Acirc",	194 },
		  { "Agrave",	192 },
		  { "Aring",	197 },
		  { "Atilde",	195 },
		  { "Auml",	196 },
		  { "Ccedil",	199 },
		  { "Dstrok",	208 },
		  { "ETH",	208 },
		  { "Eacute",	201 },
		  { "Ecirc",	202 },
		  { "Egrave",	200 },
		  { "Euml",	203 },
		  { "Iacute",	205 },
		  { "Icirc",	206 },
		  { "Igrave",	204 },
		  { "Iuml",	207 },
		  { "Ntilde",	209 },
		  { "Oacute",	211 },
		  { "Ocirc",	212 },
		  { "Ograve",	210 },
		  { "Oslash",	216 },
		  { "Otilde",	213 },
		  { "Ouml",	214 },
		  { "THORN",	222 },
		  { "Uacute",	218 },
		  { "Ucirc",	219 },
		  { "Ugrave",	217 },
		  { "Uuml",	220 },
		  { "Yacute",	221 },
		  { "aacute",	225 },
		  { "acirc",	226 },
		  { "acute",	180 },
		  { "aelig",	230 },
		  { "agrave",	224 },
		  { "amp",	'&' },
		  { "aring",	229 },
		  { "atilde",	227 },
		  { "auml",	228 },
		  { "brkbar",	166 },
		  { "brvbar",	166 },
		  { "ccedil",	231 },
		  { "cedil",	184 },
		  { "cent",	162 },
		  { "copy",	169 },
		  { "curren",	164 },
		  { "deg",	176 },
		  { "die",	168 },
		  { "divide",	247 },
		  { "eacute",	233 },
		  { "ecirc",	234 },
		  { "egrave",	232 },
		  { "eth",	240 },
		  { "euml",	235 },
		  { "euro",	128 },
		  { "frac12",	189 },
		  { "frac14",	188 },
		  { "frac34",	190 },
		  { "gt",	'>' },
		  { "hibar",	175 },
		  { "iacute",	237 },
		  { "icirc",	238 },
		  { "iexcl",	161 },
		  { "igrave",	236 },
		  { "iquest",	191 },
		  { "iuml",	239 },
		  { "laquo",	171 },
		  { "lt",	'<' },
		  { "macr",	175 },
		  { "micro",	181 },
		  { "middot",	183 },
		  { "nbsp",	160 },
		  { "not",	172 },
		  { "ntilde",	241 },
		  { "oacute",	243 },
		  { "ocirc",	244 },
		  { "ograve",	242 },
		  { "ordf",	170 },
		  { "ordm",	186 },
		  { "oslash",	248 },
		  { "otilde",	245 },
		  { "ouml",	246 },
		  { "para",	182 },
		  { "plusmn",	177 },
		  { "pound",	163 },
		  { "quot",	'\"' },
		  { "raquo",	187 },
		  { "reg",	174 },
		  { "sect",	167 },
		  { "shy",	173 },
		  { "sup1",	185 },
		  { "sup2",	178 },
		  { "sup3",	179 },
		  { "szlig",	223 },
		  { "thorn",	254 },
		  { "times",	215 },
		  { "uacute",	250 },
		  { "ucirc",	251 },
		  { "ugrave",	249 },
		  { "uml",	168 },
		  { "uuml",	252 },
		  { "yacute",	253 },
		  { "yen",	165 },
		  { "yuml",	255 }
		};
static lut_t	*iso8859_names[256];

static int	compare_lut(lut_t *, lut_t *);


/*
 * 'iso8859()' - Return the 8-bit character value of a glyph name.
 */

hdChar				/* O - ISO-8859-1 equivalent */
iso8859(hdChar *name)		/* I - Glyph name */
{
  lut_t		key,		/* Lookup table key */
		*match;		/* Matching entry pointer */


  if (strlen((char *)name) == 1)
    return (name[0]);
  else if (strlen((char *)name) > 6)
    return (0);
  else if (name[0] == '#')
  {
    // Return a decimal or hex character...
    int ch;			/* Character */

    if (name[1] == 'x')
      ch = strtol((char *)name + 2, NULL, 16);
    else
      ch = strtol((char *)name + 1, NULL, 10);

    if (ch == 0x20ac)
      ch = 0x80; /* Remap Euro character */

    return (ch);
  }

  strlcpy((char *)key.name, (char *)name, sizeof(key.name));
  match = (lut_t *)bsearch(&key, iso8859_numbers,
                           sizeof(iso8859_numbers) / sizeof(iso8859_numbers[0]),
                           sizeof(iso8859_numbers[0]),
                           (int (*)(const void *, const void *))compare_lut);

  if (match == NULL)
    return (0);
  else
    return (match->value);
}


/*
 * 'iso8859()' - Return the glyph name of an 8-bit character value.
 */

hdChar *			/* O - Glyph name */
iso8859(hdChar value)	/* I - ISO-8859-1 equivalent */
{
  int		i;		/* Looping var */
  static int	first_time = 1;	/* First time called? */
  static hdChar	buf[255];	/* Character buffer */


  if (first_time)
  {
    memset(iso8859_names, 0, sizeof(iso8859_names));
    for (i = 0; i < (int)(sizeof(iso8859_numbers) / sizeof(iso8859_numbers[0])); i ++)
      iso8859_names[iso8859_numbers[i].value] = iso8859_numbers + i;

    first_time = 0;
  }

  if (iso8859_names[value] == NULL)
  {
    buf[0] = value;
    buf[1] = '\0';
  }
  else
    sprintf((char *)buf, "&%s;", iso8859_names[value]->name);

  return (buf);
}


/*
 * 'compare_lut()' - Compare two glyphs.
 */

static int		/* O - 0 if equal, -1 if a<b, 1 if a>b */
compare_lut(lut_t *a,	/* I - First glyph */
            lut_t *b)	/* I - Second glyph */
{
  return (strcmp((char *)a->name, (char *)b->name));
}


/*
 * End of "$Id$".
 */
