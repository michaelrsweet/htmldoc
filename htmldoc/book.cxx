//
// "$Id: book.cxx,v 1.1 2004/03/31 20:56:56 mike Exp $"
//
//   Book routines for HTMLDOC, a HTML document processing program.
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
//       Hollywood, Maryland 20636-3142 USA
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
#include <math.h>


const char		*hdBook::datadir = (const char *)0;
					// Data directory
const char * const	hdBook::PDFModes[] =
{					// Mode strings
  "document",
  "outline",
  "fullscreen"
};
const char * const	hdBook::PDFLayouts[] =
{					// Layout strings
  "single",
  "one",
  "twoleft",
  "tworight"
};
const char * const	hdBook::PDFPages[] =
{					// First page strings
  "p1",
  "toc",
  "c1"
};
const char * const	hdBook::PDFEffects[] =
{					// Effect strings
  "none",
  "bi",
  "bo",
  "d",
  "gd",
  "gdr",
  "gr",
  "hb",
  "hsi",
  "hso",
  "vb",
  "vsi",
  "vso",
  "wd",
  "wl",
  "wr",
  "wu"
};


//
// 'hdBook::hdBook()' - Initialize global data...
//

hdBook::hdBook()		// I - Data directory
{
  int	i;				// Looping var


  if (!datadir)
  {
    if ((datadir = getenv("HTMLDOC_DATA")) == NULL)
      datadir = HTML_DATA;
  }

  verbosity        = 0;
  error_count      = 0;
  strict_html      = false;
  progress_visible = false;
  num_sizes        = 0;
  sizes            = (hdPageSize *)0;
  num_entities     = 0;
  entities         = (hdEntity *)0;

  Compression       = 1;
  TitlePage         = true;
  TocLinks          = true;
  TocNumbers        = false;
  TocLevels         = 3;
  TocDocCount       = 0;
  OutputType        = HD_OUTPUT_BOOK;
  OutputPath[0]     = '\0';
  OutputFiles       = false;
  OutputColor       = true;
  OutputJPEG        = 0;
  PDFVersion        = 13;
  PDFPageMode       = HD_PDF_OUTLINE;
  PDFPageLayout     = HD_PDF_SINGLE;
  PDFFirstPage      = HD_PDF_CHAPTER_1;
  PDFEffect         = HD_PDF_NONE;
  PDFEffectDuration = 1.0f;
  PDFPageDuration   = 10.0f;
  Encryption        = false;
  Permissions       = -4;
  OwnerPassword[0]  = '\0';
  UserPassword[0]   = '\0';
  EmbedFonts        = false;
  PSLevel           = 2;
  PSCommands        = false;
  XRXComments       = false;
  PageWidth         = 595;
  PageLength        = 792;
  PageLeft          = 72;
  PageRight         = 36;
  PageTop           = 36;
  PageBottom        = 36;
  PageDuplex        = false;
  Landscape         = false;
  NumberUp          = 1;
  HeadFootType      = HD_FONTFACE_SANS_SERIF;
  HeadFootStyle     = HD_FONTINTERNAL_NORMAL;
  HeadFootSize      = 11.0f;
  Header[0]         = NULL;
  Header[1]         = NULL;
  Header[2]         = NULL;
  Footer[0]         = NULL;
  Footer[1]         = NULL;
  Footer[2]         = NULL;
  TocHeader[0]      = NULL;
  TocHeader[1]      = NULL;
  TocHeader[2]      = NULL;
  TocFooter[0]      = NULL;
  TocFooter[1]      = NULL;
  TocFooter[2]      = NULL;

  strcpy(TocTitle, "Table of Contents");

  TitleImage[0]     = '\0';
  LogoImage[0]      = '\0';
  BodyColor[0]      = '\0';
  BodyImage[0]      = '\0';
  LinkColor[0]      = '\0';

  for (i = 0; i < MAX_HF_IMAGES; i ++)
    HFImage[i][0] = '\0';

  LinkStyle         = true;
  Links             = true;
  Path[0]           = '\0';
  Proxy[0]          = '\0';
}


//
// 'hdBook::~hdBook()' - Free global data...
//

hdBook::~hdBook()
{
  int	i;			// Looping var


  // Free the size table, if loaded...
  if (num_sizes)
  {
    for (i = 0; i < num_sizes; i ++)
      sizes[i].clear();

    delete[] sizes;
  }

  // Free the entitie table, if loaded...
  if (num_entities)
  {
    for (i = 0; i < num_entities; i ++)
      entities[i].clear();

    delete[] entities;
  }

  // Cleanup the image and file caches...
//  hdImage::flush();
//  file_cleanup();
}


//
// 'hdBook::find_entity()' - Find the HTML entity associated with a glyph.
//

const char *				// O - HTML entity name
hdBook::find_entity(const char *g)	// I - PostScript glyph name
{
  int		i;			// Looping var
  hdEntity	*e;			// Current entity


  // Lookup the glyph name...
  load_entities();

  for (i = num_entities, e = entities; i > 0; i --, e ++)
    if (strcmp(e->glyph, g) == 0)
      return (e->html);

  // If not found, then use the glyph name for the entity name...
  return (g);
}


//
// 'hdBook::find_glyph()' - Find the glyph associated with an HTML entity.
//

const char *				// O - PostScript glyph name
hdBook::find_glyph(const char *h)	// I - HTML entity name
{
  int		i;			// Looping var
  hdEntity	*e;			// Current entity


  // Lookup the entity name...
  load_entities();

  for (i = num_entities, e = entities; i > 0; i --, e ++)
    if (strcmp(e->html, h) == 0)
      return (e->glyph);

  // If not found, then use the entity name for the glyph name...
  return (h);
}


//
// 'hdBook::find_size()' - Get the page size by numbers.
//

hdPageSize *				// O - Matching size
hdBook::find_size(float w,		// I - Width in points
                    float l)		// I - Length in points
{
  int		i;			// Looping var
  hdPageSize	*s;			// Current size record


  // Lookup the size in the size table...
  load_sizes();

  for (i = num_sizes, s = sizes; i > 0; i --, s ++)
    if (fabs(s->width - w) < 0.1f && fabs(s->length - l) < 0.1f)
      return (s);

  return (NULL);
}


//
// 'hdBook::find_size()' - Set the page size by name.
//

hdPageSize *				// O - Matching page size
hdBook::find_size(const char *name)	// I - Page size name
{
  int		i;			// Looping var
  hdPageSize	*s;			// Current size record


  // Lookup the name in the size table...
  load_sizes();

  for (i = num_sizes, s = sizes; i > 0; i --, s ++)
    if (strcasecmp(name, s->name) == 0)
      return (s);

  return (NULL);
}


//
// 'hdBook::format_number()' - Format a numeric value.
//

char *					// O - String
hdBook::format_number(char *s,	// O - String
                        int  slen,	// I - Size of string buffer
			char format,	// I - Format type
			int  number)	// I - Number to format
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


  switch (format)
  {
    default :
        s[0] = '\0';
	break;

    case 'a' :
        if (number >= (26 * 26))
	  s[0] = '\0';
        else if (number > 26)
          snprintf(s, slen, "%c%c", 'a' + (number / 26) - 1,
	           'a' + (number % 26) - 1);
        else
          snprintf(s, slen, "%c", 'a' + number - 1);
        break;

    case 'A' :
        if (number >= (26 * 26))
	  s[0] = '\0';
        else if (number > 26)
          snprintf(s, slen, "%c%c", 'A' + (number / 26) - 1,
	           'A' + (number % 26) - 1);
        else
          snprintf(s, slen, "%c", 'A' + number - 1);
        break;

    case '1' :
        snprintf(s, slen, "%d", number);
        break;

    case 'i' :
        if (number >= 1000)
	  s[0] = '\0';
	else
          snprintf(s, slen, "%s%s%s", hundreds[number / 100],
	           tens[(number / 10) % 10], ones[number % 10]);
        break;

    case 'I' :
        if (number >= 1000)
	  s[0] = '\0';
	else
          snprintf(s, slen, "%s%s%s", HUNDREDS[number / 100],
	           TENS[(number / 10) % 10], ONES[number % 10]);
        break;
  }

  return (s);
}


//
// 'hdBook::load_entities()' - Load the entity table.
//

void
hdBook::load_entities()
{
  FILE	*fp;			// Page size file pointer
  char		filename[1024],		// Page size filename
		line[255],		// Line from file
		html[64],		// HTML entity name
		glyph[64];		// PostScript glyph name
  hdEntity	*temp;			// Temporary array pointer
  int		alloc_entities;		// Number of entities allocated


  // See if we need to load the entity table...
  if (num_entities)
    return;

  // Open the entities.dat file...
  snprintf(filename, sizeof(filename), "%s/data/entities.dat", datadir);
  if ((fp = fopen(filename, "r")) == NULL)
  {
    fprintf(stderr, "Unable to open entity file \"%s\"!\n", filename);
    return;
  }

  // Read all of the entities from the file...
  alloc_entities = 32;
  entities       = new hdEntity[32];
  num_entities   = 0;

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    // Skip blank and comment lines...
    if (line[0] == '#' || !line[0])
      continue;

    // Scan the line for the entitie info...
    if (sscanf(line, "%63s%63s", html, glyph) != 2)
    {
      fprintf(stderr, "Bad entity line \"%s\" in \"%s\"!\n", line,
              filename);
      continue;
    }

    // Reallocate memory as needed...
    if (num_entities >= alloc_entities)
    {
      temp = new hdEntity[alloc_entities + 32];

      memcpy(temp, entities, alloc_entities * sizeof(hdEntity));

      delete[] entities;

      entities       = temp;
      alloc_entities += 32;
    }

    // Set the next entitie in the array...
    entities[num_entities].set(html, glyph);
    num_entities ++;
  }

  fclose(fp);
}


//
// 'hdBook::load_sizes()' - Load the size table.
//

void
hdBook::load_sizes()
{
  FILE		*fp;			// Page size file pointer
  char		filename[1024],		// Page size filename
		line[255],		// Line from file
		name[64];		// Size name
  float		width,			// Width in points
		length;			// Height in points
  hdPageSize	*temp;			// Temporary array pointer
  int		alloc_sizes;		// Number of sizes allocated


  // See if we need to load the size table...
  if (num_sizes)
    return;

  // Open the sizes.dat file...
  snprintf(filename, sizeof(filename), "%s/data/sizes.dat", datadir);
  if ((fp = fopen(filename, "r")) == NULL)
  {
    fprintf(stderr, "Unable to open page size file \"%s\"!\n", filename);
    return;
  }

  // Read all of the sizes from the file...
  alloc_sizes = 32;
  sizes       = new hdPageSize[32];
  num_sizes   = 0;

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    // Skip blank and comment lines...
    if (line[0] == '#' || !line[0])
      continue;

    // Scan the line for the size info...
    if (sscanf(line, "%63s%f%f", name, &width, &length) != 3)
    {
      fprintf(stderr, "Bad page size line \"%s\" in \"%s\"!\n", line,
              filename);
      continue;
    }

    // Reallocate memory as needed...
    if (num_sizes >= alloc_sizes)
    {
      temp = new hdPageSize[alloc_sizes + 32];

      memcpy(temp, sizes, alloc_sizes * sizeof(hdPageSize));

      delete[] sizes;

      sizes       = temp;
      alloc_sizes += 32;
    }

    // Set the next size in the array...
    sizes[num_sizes].set(name, width, length);
    num_sizes ++;
  }

  fclose(fp);
}


//
// 'hdEntity::set()' - Initialize a page size.
//

void
hdEntity::set(const char *h,	// I - HTML entity name
              const char *g)	// I - PostScript glyph name
{
  html  = strdup(h);
  glyph = strdup(g);
}


//
// 'hdEntity::clear()' - Clear a page size.
//

void
hdEntity::clear()
{
  if (html)
  {
    free((char *)html);
    html = NULL;
  }

  if (glyph)
  {
    free((char *)glyph);
    glyph = NULL;
  }
}


//
// 'hdPageSize::set()' - Initialize a page size.
//

void
hdPageSize::set(const char *n,	// I - Name of size
                float      w,	// I - Width in points
		float      l)	// I - Length in points
{
  name   = strdup(n);
  width  = w;
  length = l;
}


//
// 'hdPageSize::clear()' - Clear a page size.
//

void
hdPageSize::clear()
{
  if (name)
  {
    free((char *)name);
    name = NULL;
  }
}


//
// End of "$Id: book.cxx,v 1.1 2004/03/31 20:56:56 mike Exp $".
//
