//
// "$Id: book.cxx,v 1.6 2004/10/22 05:43:14 mike Exp $"
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

hdBook::hdBook()
{
  int	i;				// Looping var


  if (!datadir)
  {
    if ((datadir = getenv("HTMLDOC_DATA")) == NULL)
      datadir = HTML_DATA;
  }

  verbosity           = 0;
  error_count         = 0;
  strict_html         = false;
  progress_visible    = false;
  num_sizes           = 0;
  sizes               = (hdPageSize *)0;
  num_entities        = 0;
  entities            = (hdEntity *)0;
  num_headings        = 0;
  alloc_headings      = 0;
  headings            = (uchar **)0;
  num_images          = 0;
  alloc_images        = 0;
  images              = (image_t **)0;
  num_links           = 0;
  alloc_links         = 0;
  links               = (hdLink *)0;
  num_pages           = 0;
  alloc_pages         = 0;
  pages               = (hdPage *)0;
  num_outpages        = 0;
  outpages            = (hdOutPage *)0;
  num_objects         = 0;
  alloc_objects       = 0;
  objects             = (int *)0;
  doc_title           = (uchar *)0;
  logo_image          = (image_t *)0;
  background_image    = (image_t *)0;
  background_color[0] = 1.0f;
  background_color[1] = 1.0f;
  background_color[2] = 1.0f;
  link_color[0]       = 0.0f;
  link_color[1]       = 0.0f;
  link_color[2]       = 1.0f;
  compressor_active   = 0;

  CGIMode             = false;
  Compression         = 1;
  TitlePage           = true;
  TocLinks            = true;
  TocNumbers          = false;
  TocLevels           = 3;
  TocDocCount         = 0;
  OutputFormat        = HD_OUTPUT_HTML;
  OutputType          = HD_OUTPUT_BOOK;
  OutputPath[0]       = '\0';
  OutputFiles         = false;
  OutputColor         = true;
  OutputJPEG          = 0;
  PDFVersion          = 13;
  PDFPageMode         = HD_PDF_OUTLINE;
  PDFPageLayout       = HD_PDF_SINGLE;
  PDFFirstPage        = HD_PDF_CHAPTER_1;
  PDFEffect           = HD_PDF_NONE;
  PDFEffectDuration   = 1.0f;
  PDFPageDuration     = 10.0f;
  Encryption          = false;
  Permissions         = -4;
  OwnerPassword[0]    = '\0';
  UserPassword[0]     = '\0';
  EmbedFonts          = false;
  PSLevel             = 2;
  PSCommands          = false;
  XRXComments         = false;
  PageWidth           = 595;
  PageLength          = 792;
  PageLeft            = 72;
  PageRight           = 36;
  PageTop             = 36;
  PageBottom          = 36;
  PageDuplex          = false;
  Landscape           = false;
  NumberUp            = 1;
  HeadFootType        = HD_FONTFACE_SANS_SERIF;
  HeadFootStyle       = HD_FONTINTERNAL_NORMAL;
  HeadFootSize        = 11.0f;
  Header[0]           = NULL;
  Header[1]           = NULL;
  Header[2]           = NULL;
  Footer[0]           = NULL;
  Footer[1]           = NULL;
  Footer[2]           = NULL;
  TocHeader[0]        = NULL;
  TocHeader[1]        = NULL;
  TocHeader[2]        = NULL;
  TocFooter[0]        = NULL;
  TocFooter[1]        = NULL;
  TocFooter[2]        = NULL;

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

  prefs_load();
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

  if (alloc_headings)
    free(headings);

  if (alloc_links)
    free(links);

  image_flush_cache();

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
// 'hdBook::prefs_getrc()' - Get the rc file for preferences...
//

const char *				// O - RC filename
hdBook::prefs_getrc(char *s,		// I - Buffer
                    int  slen)		// I - Size of buffer
{
#ifdef WIN32
  HKEY		key;			// Registry key
  DWORD		size;			// Size of string
  char		home[1024];		// Home (profile) directory
#else
  const char	*home;			// Home directory
#endif // WIN32


  // Find the home directory...
#ifdef WIN32
  // Open the registry...
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0,
		   KEY_READ, &key))
  {
    // Use the install directory...
    strlcpy(home, datadir, sizeof(home));
  }
  else
  {
    // Grab the current user's AppData directory...
    size = sizeof(home);
    if (RegQueryValueEx(key, "AppData", NULL, NULL, (unsigned char *)home, &size))
      strlcpy(home, datadir, sizeof(home));

    RegCloseKey(key);
  }
#else
  if ((home = getenv("HOME")) == NULL)
    home = datadir;
#endif // WIN32

  // Format the rc filename and return...
  snprintf(s, slen, "%s/.htmldocrc", home);

  return (s);
}


//
// 'hdBook::prefs_load()' - Load HTMLDOC preferences...
//

void
hdBook::prefs_load(void)
{
  int	pos;				// Header/footer position
  char	line[2048];			// Line from RC file
  FILE	*fp;				// File pointer
#ifdef WIN32
  HKEY		key;			// Registry key
  DWORD		size;			// Size of string
  static char	data[1024];		// Data directory
  static char	doc[1024];		// Documentation directory
#endif // WIN32


  //
  // Get the installed directories...
  //

  if (!datadir)
  {
#ifdef WIN32
    // Open the registry...
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Easy Software Products\\HTMLDOC", 0,
                      KEY_READ, &key))
    {
      // Grab the installed directories...
      size = sizeof(data);
      if (!RegQueryValueEx(key, "data", NULL, NULL, (unsigned char *)data, &size))
	datadir = data;
      else
        datadir = HTML_DATA;

#  ifdef HAVE_LIBFLTK
      size = sizeof(doc);
      if (!RegQueryValueEx(key, "doc", NULL, NULL, (unsigned char *)doc, &size))
	GUI::help_dir = doc;
#  endif // HAVE_LIBFLTK

      RegCloseKey(key);
    }
#endif // WIN32

#if defined(__EMX__) && defined(HAVE_LIBFLTK)
    // If being installed within XFree86 OS/2 Environment
    // we can use those values which are overwritten by
    // the according environment variables.
    datadir = strdup(__XOS2RedirRoot("/XFree86/lib/X11/htmldoc"));
    GUI::help_dir = strdup(__XOS2RedirRoot("/XFree86/lib/X11/htmldoc/doc"));
#endif // __EMX__ && HAVE_LIBFLTK

    //
    // See if the installed directories have been overridden by
    // environment variables...
    //

    if (getenv("HTMLDOC_DATA") != NULL)
      datadir = getenv("HTMLDOC_DATA");
    else
      datadir = HTML_DATA;

#ifdef HAVE_LIBFLTK
    if (getenv("HTMLDOC_HELP") != NULL)
      GUI::help_dir = getenv("HTMLDOC_HELP");
#endif // HAVE_LIBFLTK
  }

  //
  // Read the preferences file...
  //

  if ((fp = fopen(prefs_getrc(line, sizeof(line)), "r")) != NULL)
  {
    while (fgets(line, sizeof(line), fp) != NULL)
    {
      if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';

      if (strncasecmp(line, "TEXTCOLOR=", 10) == 0)
	htmlSetTextColor((uchar *)(line + 10));
      else if (strncasecmp(line, "BODYCOLOR=", 10) == 0)
	strlcpy(BodyColor, line + 10, sizeof(BodyColor));
      else if (strncasecmp(line, "BODYIMAGE=", 10) == 0)
	strlcpy(BodyImage, line + 10, sizeof(BodyImage));
      else if (strncasecmp(line, "LINKCOLOR=", 10) == 0)
        strlcpy(LinkColor, line + 10, sizeof(LinkColor));
      else if (strncasecmp(line, "LINKSTYLE=", 10) == 0)
	LinkStyle = atoi(line + 10);
      else if (strncasecmp(line, "BROWSERWIDTH=", 13) == 0)
	_htmlBrowserWidth = atof(line + 13);
      else if (strncasecmp(line, "PAGEWIDTH=", 10) == 0)
	PageWidth = atoi(line + 10);
      else if (strncasecmp(line, "PAGELENGTH=", 11) == 0)
	PageLength = atoi(line + 11);
      else if (strncasecmp(line, "PAGELEFT=", 9) == 0)
	PageLeft = atoi(line + 9);
      else if (strncasecmp(line, "PAGERIGHT=", 10) == 0)
	PageRight = atoi(line + 10);
      else if (strncasecmp(line, "PAGETOP=", 8) == 0)
	PageTop = atoi(line + 8);
      else if (strncasecmp(line, "PAGEBOTTOM=", 11) == 0)
	PageBottom = atoi(line + 11);
      else if (strncasecmp(line, "PAGEDUPLEX=", 11) == 0)
	PageDuplex = atoi(line + 11);
      else if (strncasecmp(line, "LANDSCAPE=", 10) == 0)
	Landscape = atoi(line + 10);
      else if (strncasecmp(line, "COMPRESSION=", 12) == 0)
	Compression = atoi(line + 12);
      else if (strncasecmp(line, "OUTPUTCOLOR=", 12) == 0)
      {
	OutputColor    = atoi(line + 12);
	_htmlGrayscale = !OutputColor;
      }
      else if (strncasecmp(line, "TOCNUMBERS=", 11) == 0)
	TocNumbers = atoi(line + 11);
      else if (strncasecmp(line, "TOCLEVELS=", 10) == 0)
	TocLevels = atoi(line + 10);
      else if (strncasecmp(line, "JPEG=", 5) == 0)
	OutputJPEG = atoi(line + 1);
      else if (strncasecmp(line, "PAGEHEADER=", 11) == 0)
	get_format(line + 11, Header);
      else if (strncasecmp(line, "PAGEFOOTER=", 11) == 0)
	get_format(line + 11, Footer);
      else if (strncasecmp(line, "NUMBERUP=", 9) == 0)
        NumberUp = atoi(line + 9);
      else if (strncasecmp(line, "TOCHEADER=", 10) == 0)
	get_format(line + 10, TocHeader);
      else if (strncasecmp(line, "TOCFOOTER=", 10) == 0)
	get_format(line + 10, TocFooter);
      else if (strncasecmp(line, "TOCTITLE=", 9) == 0)
	strlcpy(TocTitle, line + 9, sizeof(TocTitle));
      else if (strncasecmp(line, "BODYFONT=", 9) == 0)
	_htmlBodyFont = (hdFontFace)atoi(line + 9);
      else if (strncasecmp(line, "HEADINGFONT=", 12) == 0)
	_htmlHeadingFont = (hdFontFace)atoi(line + 12);
      else if (strncasecmp(line, "FONTSIZE=", 9) == 0)
	htmlSetBaseSize(atof(line + 9),
	                _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
      else if (strncasecmp(line, "FONTSPACING=", 12) == 0)
	htmlSetBaseSize(_htmlSizes[SIZE_P], atof(line + 12));
      else if (strncasecmp(line, "HEADFOOTTYPE=", 13) == 0)
	HeadFootType = (hdFontFace)atoi(line + 13);
      else if (strncasecmp(line, "HEADFOOTSTYLE=", 14) == 0)
        HeadFootStyle = (hdFontInternal)atoi(line + 14);
      else if (strncasecmp(line, "HEADFOOTSIZE=", 13) == 0)
	HeadFootSize = atof(line + 13);
      else if (strncasecmp(line, "PDFVERSION=", 11) == 0)
      {
        if (strchr(line + 11, '.') != NULL)
	  PDFVersion = (int)(atof(line + 11) * 10.0 + 0.5);
	else
	  PDFVersion = atoi(line + 11);
      }
      else if (strncasecmp(line, "PSLEVEL=", 8) == 0)
	PSLevel = atoi(line + 8);
      else if (strncasecmp(line, "PSCOMMANDS=", 11) == 0)
	PSCommands = atoi(line + 11);
      else if (strncasecmp(line, "XRXCOMMENTS=", 12) == 0)
	XRXComments = atoi(line + 12);
      else if (strncasecmp(line, "CHARSET=", 8) == 0)
	htmlSetCharSet(line + 8);
      else if (strncasecmp(line, "PAGEMODE=", 9) == 0)
	PDFPageMode = atoi(line + 9);
      else if (strncasecmp(line, "PAGELAYOUT=", 11) == 0)
	PDFPageLayout = atoi(line + 11);
      else if (strncasecmp(line, "FIRSTPAGE=", 10) == 0)
	PDFFirstPage = atoi(line + 10);
      else if (strncasecmp(line, "PAGEEFFECT=", 11) == 0)
	PDFEffect = atoi(line + 11);
      else if (strncasecmp(line, "PAGEDURATION=", 14) == 0)
	PDFPageDuration = atof(line + 14);
      else if (strncasecmp(line, "EFFECTDURATION=", 16) == 0)
	PDFEffectDuration = atof(line + 16);
      else if (strncasecmp(line, "ENCRYPTION=", 11) == 0)
	Encryption = atoi(line + 11);
      else if (strncasecmp(line, "PERMISSIONS=", 12) == 0)
	Permissions = atoi(line + 12);
      else if (strncasecmp(line, "OWNERPASSWORD=", 14) == 0)
      {
	strncpy(OwnerPassword, line + 14, sizeof(OwnerPassword) - 1);
	OwnerPassword[sizeof(OwnerPassword) - 1] = '\0';
      }
      else if (strncasecmp(line, "USERPASSWORD=", 13) == 0)
      {
        strncpy(UserPassword, line + 13, sizeof(UserPassword) - 1);
        UserPassword[sizeof(UserPassword) - 1] = '\0';
      }
      else if (strncasecmp(line, "LINKS=", 6) == 0)
        Links = atoi(line + 6);
      else if (strncasecmp(line, "TRUETYPE=", 9) == 0)
        EmbedFonts = atoi(line + 9);
      else if (strncasecmp(line, "EMBEDFONTS=", 11) == 0)
        EmbedFonts = atoi(line + 11);
      else if (strncasecmp(line, "PATH=", 5) == 0)
      {
	strncpy(Path, line + 5, sizeof(Path) - 1);
	Path[sizeof(Path) - 1] = '\0';
      }
      else if (strncasecmp(line, "PROXY=", 6) == 0)
      {
	strncpy(Proxy, line + 6, sizeof(Proxy) - 1);
	Proxy[sizeof(Proxy) - 1] = '\0';
      }
      else if (strncasecmp(line, "STRICTHTML=", 11) == 0)
        strict_html = atoi(line + 11);

#  ifdef HAVE_LIBFLTK
      else if (strncasecmp(line, "EDITOR=", 7) == 0)
        strlcpy(HTMLEditor, line + 7, sizeof(HTMLEditor));
      else if (strncasecmp(line, "TOOLTIPS=", 9) == 0)
        Tooltips = atoi(line + 9);
      else if (strncasecmp(line, "MODERN=", 7) == 0)
        ModernSkin = atoi(line + 7);
#  endif // HAVE_LIBFLTK
    }

    fclose(fp);
  }

  // Check header/footer formats...
  for (pos = 0; pos < 3; pos ++)
    if (Header[pos])
      break;

  if (pos == 3)
    get_format(".t.", Header);

  for (pos = 0; pos < 3; pos ++)
    if (Footer[pos])
      break;

  if (pos == 3)
    get_format("h.1", Footer);

  for (pos = 0; pos < 3; pos ++)
    if (TocHeader[pos])
      break;

  if (pos == 3)
    get_format(".t.", TocHeader);

  for (pos = 0; pos < 3; pos ++)
    if (TocFooter[pos])
      break;

  if (pos == 3)
    get_format("..i", TocFooter);
}


//
// 'hdBook::prefs_save()' - Save HTMLDOC preferences...
//

void
hdBook::prefs_save(void)
{
  FILE	*fp;				// File pointer
  char	rcfile[1024];			// RC filename


  if ((fp = fopen(prefs_getrc(rcfile, sizeof(rcfile)), "w")) != NULL)
  {
    fputs("#HTMLDOCRC " SVERSION "\n", fp);

    fprintf(fp, "TEXTCOLOR=%s\n", _htmlTextColor);
    fprintf(fp, "BODYCOLOR=%s\n", BodyColor);
    fprintf(fp, "BODYIMAGE=%s\n", BodyImage);
    fprintf(fp, "LINKCOLOR=%s\n", LinkColor);
    fprintf(fp, "LINKSTYLE=%d\n", LinkStyle);
    fprintf(fp, "BROWSERWIDTH=%.0f\n", _htmlBrowserWidth);
    fprintf(fp, "PAGEWIDTH=%d\n", PageWidth);
    fprintf(fp, "PAGELENGTH=%d\n", PageLength);
    fprintf(fp, "PAGELEFT=%d\n", PageLeft);
    fprintf(fp, "PAGERIGHT=%d\n", PageRight);
    fprintf(fp, "PAGETOP=%d\n", PageTop);
    fprintf(fp, "PAGEBOTTOM=%d\n", PageBottom);
    fprintf(fp, "PAGEDUPLEX=%d\n", PageDuplex);
    fprintf(fp, "LANDSCAPE=%d\n", Landscape);
    fprintf(fp, "COMPRESSION=%d\n", Compression);
    fprintf(fp, "OUTPUTCOLOR=%d\n", OutputColor);
    fprintf(fp, "TOCNUMBERS=%d\n", TocNumbers);
    fprintf(fp, "TOCLEVELS=%d\n", TocLevels);
    fprintf(fp, "JPEG=%d\n", OutputJPEG);
    fprintf(fp, "PAGEHEADER=%s\n", get_fmt(Header));
    fprintf(fp, "PAGEFOOTER=%s\n", get_fmt(Footer));
    fprintf(fp, "NUMBERUP=%d\n", NumberUp);
    fprintf(fp, "TOCHEADER=%s\n", get_fmt(TocHeader));
    fprintf(fp, "TOCFOOTER=%s\n", get_fmt(TocFooter));
    fprintf(fp, "TOCTITLE=%s\n", TocTitle);
    fprintf(fp, "BODYFONT=%d\n", _htmlBodyFont);
    fprintf(fp, "HEADINGFONT=%d\n", _htmlHeadingFont);
    fprintf(fp, "FONTSIZE=%.2f\n", _htmlSizes[SIZE_P]);
    fprintf(fp, "FONTSPACING=%.2f\n",
            _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
    fprintf(fp, "HEADFOOTTYPE=%d\n", HeadFootType);
    fprintf(fp, "HEADFOOTSTYLE=%d\n", HeadFootStyle);
    fprintf(fp, "HEADFOOTSIZE=%.2f\n", HeadFootSize);
    fprintf(fp, "PDFVERSION=%d\n", PDFVersion);
    fprintf(fp, "PSLEVEL=%d\n", PSLevel);
    fprintf(fp, "PSCOMMANDS=%d\n", PSCommands);
    fprintf(fp, "XRXCOMMENTS=%d\n", XRXComments);
    fprintf(fp, "CHARSET=%s\n", _htmlCharSet);
    fprintf(fp, "PAGEMODE=%d\n", PDFPageMode);
    fprintf(fp, "PAGELAYOUT=%d\n", PDFPageLayout);
    fprintf(fp, "FIRSTPAGE=%d\n", PDFFirstPage);
    fprintf(fp, "PAGEEFFECT=%d\n", PDFEffect);
    fprintf(fp, "PAGEDURATION=%.0f\n", PDFPageDuration);
    fprintf(fp, "EFFECTDURATION=%.1f\n", PDFEffectDuration);
    fprintf(fp, "ENCRYPTION=%d\n", Encryption);
    fprintf(fp, "PERMISSIONS=%d\n", Permissions);
    fprintf(fp, "OWNERPASSWORD=%s\n", OwnerPassword);
    fprintf(fp, "USERPASSWORD=%s\n", UserPassword);
    fprintf(fp, "LINKS=%d\n", Links);
    fprintf(fp, "EMBEDFONTS=%d\n", EmbedFonts);
    fprintf(fp, "PATH=%s\n", Path);
    fprintf(fp, "PROXY=%s\n", Proxy);
    fprintf(fp, "STRICTHTML=%d\n", strict_html);

#ifdef HAVE_LIBFLTK
    fprintf(fp, "EDITOR=%s\n", HTMLEditor);
    fprintf(fp, "TOOLTIPS=%d\n", Tooltips);
    fprintf(fp, "MODERN=%d\n", ModernSkin);
#endif // HAVE_LIBFLTK

    fclose(fp);
  }
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
// End of "$Id: book.cxx,v 1.6 2004/10/22 05:43:14 mike Exp $".
//
