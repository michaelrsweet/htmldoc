/*
 * "$Id: htmldoc.cxx,v 1.2 1999/11/08 22:11:35 mike Exp $"
 *
 *   Main entry for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 *
 * Contents:
 *
 *   main()            - Main entry for HTMLDOC.
 *   progress_show()   - Show the current run status.
 *   progress_hide()   - Hide the current run status.
 *   progress_update() - Update the current run status.
 *   format_number()   - Format a number into arabic numerals, roman numerals,
 *                       or letters.
 *   get_measurement() - Get a size measurement in inches, points, centimeters,
 *                       or millimeters.
 *   set_page_size()   - Set the output page size.
 *   prefs_load()      - Load HTMLDOC preferences...
 *   prefs_save()      - Save HTMLDOC preferences...
 *   compare_strings() - Compare two command-line strings.
 *   usage()           - Show program version and command-line options.
 */

/*
 * Include necessary headers.
 */

#define _HTMLDOC_C_
#include "htmldoc.h"
#include <ctype.h>
#include <stdarg.h>


/*
 * Usage:
 *
 *    htmldoc [options] filename1.html [ ... filenameN.html ]
 *
 * Options:
 *
 *    --bodycolor color
 *    --bodyfont {courier,times,helvetica}
 *    --bodyimage filename.{gif,jpg,png}
 *    --bottom margin{in,cm,mm}
 *    --color
 *    --compression[=level]
 *    --duplex
 *    --fontsize {6.0..24.0)
 *    --fontspacing {1.0..3.0}
 *    --footer fff
 *    {--format, -t} {ps1,ps2,pdf,html}
 *    --gray
 *    --header fff
 *    --headfootfont {courier,times,helvetica}
 *    --headfootsize {6.0..24.0}
 *    --headingfont {courier,times,helvetica}
 *    --help
 *    --jpeg[=quality]
 *    --left margin{in,cm,mm}
 *    --logo filename.{gif,jpg,png}
 *    --no-compression
 *    --no-title
 *    --no-toc
 *    --numbered
 *    {--outdir, -d} dirname
 *    {--outfile, -f} filename.{ps,pdf,html}
 *    --right margin{in,cm,mm}
 *    --size {letter,a4,WxH{in,cm,mm},etc}
 *    --title filename.{gif,jpg,png}
 *    --tocfooter fff
 *    --tocheader fff
 *    --toclevels levels
 *    --top margin{in,cm,mm}
 *    {--verbose, -v}
 *    --webpage
 *
 * fff = Formatting for header/footer (left, middle, right); each "f" is:
 *
 *    . = blank
 *    t = title
 *    h = current heading
 *    c = current chapter heading
 *    l = logo image
 *    i = lowercase roman numerals
 *    I = uppercase roman numerals
 *    1 = arabic numbers (1, 2, 3, ...)
 *    a = lowercase letters
 *    A = uppercase letters
 */

/*
 * Local functions...
 */

static void	usage(void);
static int	compare_strings(char *s, char *t, int tmin);


/*
 * 'main()' - Main entry for HTMLDOC.
 */

#ifdef MAC		// MacOS subverts ANSI C...
int
main(void)
{
  int		argc;	// Number of command-line arguments
  char		**argv;	// Command-line arguments


  argc = ccommand(&argv);
#else			// All other operating systems...
int
main(int  argc,		/* I - Number of command-line arguments */
     char *argv[])	/* I - Command-line arguments */
{
#endif // MAC

  int		i;		/* Looping var */
  FILE		*docfile;	/* Document file */
  tree_t	*document,	/* Master HTML document */
		*file,		/* HTML document file */
		*toc;		/* Table of contents */
  int		(*exportfunc)(tree_t *, tree_t *);
				/* Export function */
  char		*extension;	/* Extension of output filename */
  char		base[1024];	/* Base directory name of file */
  float		fontsize,	/* Base font size */
		fontspacing;	/* Base font spacing */


 /*
  * Load preferences...
  */

  prefs_load();

 /*
  * Default to producing HTML files.
  */

  document   = NULL;
  exportfunc = html_export;

 /*
  * Parse command-line options...
  */

  fontsize    = 11.0f;
  fontspacing = 1.2f;

  for (i = 1; i < argc; i ++)
    if (compare_strings(argv[i], "--bodycolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strcpy((char *)BodyColor, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--bodyfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0 ||
	    strcasecmp(argv[i], "monospace") == 0)
	  _htmlBodyFont = TYPE_COURIER;
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "serif") == 0)
	  _htmlBodyFont = TYPE_TIMES;
        else if (strcasecmp(argv[i], "helvetica") == 0 ||
	         strcasecmp(argv[i], "arial") == 0 ||
		 strcasecmp(argv[i], "sans-serif") == 0)
	  _htmlBodyFont = TYPE_HELVETICA;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--bodyimage", 7) == 0)
    {
      i ++;
      if (i < argc)
        strcpy((char *)BodyImage, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--bottom", 5) == 0)
    {
      i ++;
      if (i < argc)
        PageBottom = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--color", 5) == 0)
    {
      OutputColor = 1;
    }
    else if (compare_strings(argv[i], "--compression", 5) == 0 ||
             strncmp(argv[i], "--compression=", 14) == 0)
    {
      if (strlen(argv[i]) > 14 && PDFVersion >= 1.2)
        Compression = atoi(argv[i] + 14);
      else if (PDFVersion >= 1.2)
        Compression = 1;
    }
    else if (compare_strings(argv[i], "--duplex", 3) == 0)
    {
      PageDuplex = 1;
    }
    else if (compare_strings(argv[i], "--footer", 5) == 0)
    {
      i ++;
      if (i < argc)
        strncpy(Footer, argv[i], 3);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--format", 5) == 0 ||
             strcmp(argv[i], "-t") == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "ps1") == 0)
          exportfunc = ps_export_level1;
        else if (strcasecmp(argv[i], "ps2") == 0 ||
                 strcasecmp(argv[i], "ps") == 0)
          exportfunc = ps_export_level2;
        else if (strcasecmp(argv[i], "pdf") == 0)
	{
          exportfunc = pdf_export;
	  PDFVersion = 1.2;
	}
        else if (strcasecmp(argv[i], "pdf11") == 0)
	{
          exportfunc  = pdf_export;
	  PDFVersion  = 1.1;
	  Compression = 0;
	}
        else if (strcasecmp(argv[i], "html") == 0)
          exportfunc = html_export;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--fontsize", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        fontsize = atof(argv[i]);

	if (fontsize < 6.0f)
	  fontsize = 6.0f;
	else if (fontsize > 24.0f)
	  fontsize = 24.0f;

        htmlSetBaseSize(fontsize, fontspacing);
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--fontspacing", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        fontspacing = atof(argv[i]);

	if (fontspacing < 1.0f)
	  fontspacing = 1.0f;
	else if (fontspacing > 3.0f)
	  fontspacing = 3.0f;

        htmlSetBaseSize(fontsize, fontspacing);
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--gray", 3) == 0)
    {
      OutputColor    = 0;
      _htmlGrayscale = 1;
    }
    else if (compare_strings(argv[i], "--header", 7) == 0)
    {
      i ++;
      if (i < argc)
        strncpy(Header, argv[i], 3);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--headfootfont", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0)
	  HeadFootFont = TYPE_COURIER;
        else if (strcasecmp(argv[i], "times") == 0)
	  HeadFootFont = TYPE_TIMES;
        else if (strcasecmp(argv[i], "helvetica") == 0)
	  HeadFootFont = TYPE_HELVETICA;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--headfootsize", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        HeadFootSize = atof(argv[i]);

	if (HeadFootSize < 6.0f)
	  HeadFootSize = 6.0f;
	else if (HeadFootSize > 24.0f)
	  HeadFootSize = 24.0f;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--headingfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0 ||
	    strcasecmp(argv[i], "monospace") == 0)
	  _htmlHeadingFont = TYPE_COURIER;
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "serif") == 0)
	  _htmlHeadingFont = TYPE_TIMES;
        else if (strcasecmp(argv[i], "helvetica") == 0 ||
	         strcasecmp(argv[i], "arial") == 0 ||
	         strcasecmp(argv[i], "sans-serif") == 0)
	  _htmlHeadingFont = TYPE_HELVETICA;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--jpeg", 3) == 0 ||
             strncmp(argv[i], "--jpeg=", 7) == 0)
    {
      if (strlen(argv[i]) > 7)
        OutputJPEG = atoi(argv[i] + 7);
      else
        OutputJPEG = 90;
    }
    else if (compare_strings(argv[i], "--left", 4) == 0)
    {
      i ++;
      if (i < argc)
        PageLeft = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--logo", 5) == 0)
    {
      i ++;
      if (i < argc)
        strcpy(LogoImage, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--no-compression", 5) == 0)
    {
      Compression = 0;
    }
    else if (compare_strings(argv[i], "--no-toc", 6) == 0)
    {
      TocLevels = 0;
    }
    else if (compare_strings(argv[i], "--no-title", 6) == 0)
    {
      TitlePage = 0;
    }
    else if (compare_strings(argv[i], "--numbered", 4) == 0)
    {
      TocNumbers = 1;
    }
    else if (compare_strings(argv[i], "--outdir", 6) == 0 ||
             strcmp(argv[i], "-d") == 0)
    {
      i ++;
      if (i < argc)
      {
        strcpy(OutputPath, argv[i]);
        OutputFiles = 1;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--outfile", 6) == 0 ||
             strcmp(argv[i], "-f") == 0)
    {
      i ++;
      if (i < argc)
      {
        strcpy(OutputPath, argv[i]);
        OutputFiles = 0;

        if ((extension = file_extension(argv[i])) != NULL)
        {
          if (strcasecmp(extension, "ps") == 0 &&
              exportfunc != ps_export_level1)
            exportfunc = ps_export_level2;
          else if (strcasecmp(extension, "pdf") == 0)
            exportfunc = pdf_export;
          else if (strcasecmp(extension, "html") == 0)
            exportfunc = html_export;
        }
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--right", 3) == 0)
    {
      i ++;
      if (i < argc)
        PageRight = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--size", 3) == 0)
    {
      i ++;
      if (i < argc)
        set_page_size(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--title", 4) == 0)
    {
      i ++;
      if (i < argc)
        strcpy(TitleImage, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--tocfooter", 6) == 0)
    {
      i ++;
      if (i < argc)
        strncpy(TocFooter, argv[i], 3);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--tocheader", 6) == 0)
    {
      i ++;
      if (i < argc)
        strncpy(TocHeader, argv[i], 3);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--toclevels", 6) == 0)
    {
      i ++;
      if (i < argc)
        TocLevels = atoi(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--top", 5) == 0)
    {
      i ++;
      if (i < argc)
        PageTop = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--verbose", 3) == 0 ||
             strcmp(argv[i], "-v") == 0)
    {
      Verbosity ++;
    }
    else if (compare_strings(argv[i], "--webpage", 3) == 0)
    {
      TocLevels = 0;
      TitlePage = 0;
    }
    else if (strcmp(argv[i], "-") == 0)
    {
     /*
      * Read from stdin...
      */

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME", (uchar *)"");

      htmlReadFile(file, stdin, NULL);

      if (file->child != NULL)
      {
        if (document == NULL)
          document = file;
        else
        {
          while (document->next != NULL)
            document = document->next;

          document->next = file;
          file->prev     = document;
        }
      }
      else
        htmlDeleteTree(file);
    }
#ifdef HAVE_LIBFLTK
    else if (strlen(argv[i]) > 5 &&
             strcmp(argv[i] + strlen(argv[i]) - 5, ".book") == 0)
    {
     /*
      * GUI mode...
      */

      if (BookGUI == NULL)
        BookGUI = new GUI(argv[i]);
      else
        BookGUI->loadBook(argv[i]);
    }
#endif /* HAVE_LIBFLTK */
    else if ((docfile = fopen(argv[i], "r")) != NULL)
    {
     /*
      * Read from a file...
      */

      if (Verbosity)
        fprintf(stderr, "htmldoc: Reading %s...\n", argv[i]);

      strcpy(base, file_directory(argv[i]));

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME", (uchar *)argv[i]);

      htmlReadFile(file, docfile, base);

      fclose(docfile);

      if (file->child != NULL)
      {
        if (document == NULL)
          document = file;
        else
        {
          while (document->next != NULL)
            document = document->next;

          document->next = file;
          file->prev     = document;
        }
      }
      else
        htmlDeleteTree(file);
    }
    else
      usage();

 /*
  * Display the GUI if necessary...
  */

#ifdef HAVE_LIBFLTK
  if (document == NULL && BookGUI == NULL)
    BookGUI = new GUI();

  FileIcon::load_system_icons();

  if (BookGUI != NULL)
  {
    i = BookGUI->doGUI();
    delete BookGUI;
    return (i);
  }
#endif /* HAVE_LIBFLTK */
    
 /*
  * We *must* have a document to process...
  */

  if (document == NULL)
    usage();

 /*
  * Find the first one in the list...
  */

  while (document->prev != NULL)
    document = document->prev;

 /*
  * Build a table of contents for the documents if necessary...
  */

  if (TocLevels > 0)
    toc = toc_build(document);
  else
    toc = NULL;

 /*
  * Figure out the printable area of the output page...
  */

  PagePrintWidth  = PageWidth - PageLeft - PageRight;
  PagePrintLength = PageLength - PageTop - PageBottom;

 /*
  * Generate the output file(s).
  */

  (*exportfunc)(document, toc);

  return (0);
}


/*
 * 'progress_show()' - Show the current run status.
 */

void
progress_show(char *format,	/* I - Printf-style format string */
              ...)		/* I - Additional args as needed */
{
  va_list	ap;		/* Argument pointer */
  char		text[2048];	/* Formatted text string */


  va_start(ap, format);
  vsprintf(text, format, ap);
  va_end(ap);

#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(0, text);
    return;
  }
#endif /* HAVE_LIBFLTK */

  printf("\r%-79s", text);
  fflush(stdout);
}


/*
 * 'progress_hide()' - Hide the current run status.
 */

void
progress_hide(void)
{
#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(0, "HTMLDOC " SVERSION " Ready.");
    return;
  }
#endif /* HAVE_LIBFLTK */

  printf("\r%-79s\r", "");
  fflush(stdout);
}


/*
 * 'progress_update()' - Update the current run status.
 */

void
progress_update(int percent)	/* I - Percent complete */
{
#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(percent);
    return;
  }
#endif /* HAVE_LIBFLTK */
}


/*
 * 'format_number()' - Format a number into arabic numerals, roman numerals,
 *                     or letters.
 */

char *				/* O - String */
format_number(int  n,		/* I - Number */
              char f)		/* I - Format */
{
  static char	*ones[10] =	/* Roman numerals, 0-9 */
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
  static char	*ONES[10] =	/* Roman numerals, 0-9 */
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
 * 'get_measurement()' - Get a size measurement in inches, points, centimeters,
 *                       or millimeters.
 */

int				/* O - Measurement in points */
get_measurement(char *s)	/* I - Measurement string */
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

  return ((int)val);
}


/*
 * 'set_page_size()' - Set the output page size.
 */

void
set_page_size(char *size)	/* I - Page size string */
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
  else if (strcasecmp(size, "a4") == 0)
  {
   /*
    * European standard A4 - 210x297mm (8.27x11.69 inches).
    */

    PageWidth  = 595;
    PageLength = 842;
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
 * 'prefs_load()' - Load HTMLDOC preferences...
 */

void
prefs_load(void)
{
#ifdef HAVE_LIBFLTK
#  ifdef WIN32			//// Do registry magic...
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string


  // Figure out what the HTML editor is...
  size = sizeof(HTMLEditor);

  if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, "htmldoc", 0, KEY_READ, &key))
    RegQueryValueEx(key, "editor", NULL, NULL, (unsigned char *)HTMLEditor, &size);
#  else				//// Do .htmldocrc file in home dir...
  char	line[1024],		// Line from RC file
	htmldocrc[1024];	// HTMLDOC RC file
  FILE	*fp;			// File pointer


  if (getenv("HOME") != NULL)
  {
    sprintf(htmldocrc, "%s/.htmldocrc", getenv("HOME"));

    if ((fp = fopen(htmldocrc, "r")) != NULL)
    {
      while (fgets(line, sizeof(line), fp) != NULL)
      {
        if (line[strlen(line) - 1] == '\n')
	  line[strlen(line) - 1] = '\0';

        if (strncasecmp(line, "EDITOR=", 7) == 0)
	  strcpy(HTMLEditor, line + 7);
      }

      fclose(fp);
    }
  }
#  endif // WIN32
#endif // HAVE_LIBFLTK
}


/*
 * 'prefs_save()' - Save HTMLDOC preferences...
 */

void
prefs_save(void)
{
#ifdef HAVE_LIBFLTK
#  ifdef WIN32			//// Do registry magic...
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string


  // Figure out what the HTML editor is...
  size = sizeof(HTMLEditor);

  if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "htmldoc", 0, "htmldoc",
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL))
    return;

  RegSetValueEx(key, "editor", 0, REG_SZ, (unsigned char *)HTMLEditor, size);
  RegCloseKey(key);
#  else				//// Do .htmldocrc file in home dir...
  char	htmldocrc[1024];	// HTMLDOC RC file
  FILE	*fp;			// File pointer


  if (getenv("HOME") != NULL)
  {
    sprintf(htmldocrc, "%s/.htmldocrc", getenv("HOME"));

    if ((fp = fopen(htmldocrc, "w")) != NULL)
    {
      fputs("#HTMLDOCRC " SVERSION "\n", fp);

      fprintf(fp, "EDITOR=%s\n", HTMLEditor);

      fclose(fp);
    }
  }
#  endif // WIN32
#endif // HAVE_LIBFLTK
}


/*
 * 'compare_strings()' - Compare two command-line strings.
 */

static int			/* O - -1 or 1 = no match, 0 = match */
compare_strings(char *s,	/* I - Command-line string */
                char *t,	/* I - Option string */
                int  tmin)	/* I - Minimum number of unique chars in option */
{
  int	slen;			/* Length of command-line string */


  slen = strlen(s);
  if (slen < tmin)
    return (-1);
  else
    return (strncmp(s, t, slen));
}


/*
 * 'usage()' - Show program version and command-line options.
 */

static void
usage(void)
{
  fputs("HTMLDOC Version " SVERSION " Copyright 1997-1999 Michael Sweet, All Rights Reserved.\n", stderr);
  fputs("This software is governed by the GNU General Public License, Version 2, and\n", stderr);
  fputs("is based in part on the work of the Independent JPEG Group.\n", stderr);
  fputs("\n", stderr);
  fputs("Usage:\n", stderr);
  fputs("  htmldoc [options] filename1.html [ ... filenameN.html ]\n", stderr);
  fputs("  htmldoc filename.book\n", stderr);
  fputs("\n", stderr);
  fputs("Options:\n", stderr);
  fputs("\n", stderr);
  fputs("  --bodycolor color\n", stderr);
  fputs("  --bodyfont {courier,times,helvetica}\n", stderr);
  fputs("  --bodyimage filename.{gif,jpg,png}\n", stderr);
  fputs("  --bottom margin{in,cm,mm}\n", stderr);
  fputs("  --color\n", stderr);
  fputs("  --compression[=level]\n", stderr);
  fputs("  --duplex\n", stderr);
  fputs("  --fontsize {6.0..24.0}\n", stderr);
  fputs("  --fontspacing {1.0..3.0}\n", stderr);
  fputs("  --footer fff\n", stderr);
  fputs("  {--format, -t} {ps1,ps2,pdf,html}\n", stderr);
  fputs("  --gray\n", stderr);
  fputs("  --header fff\n", stderr);
  fputs("  --headfootfont {courier,times,helvetica}\n", stderr);
  fputs("  --headfootsize {6.0..24.0}\n", stderr);
  fputs("  --headingfont {courier,times,helvetica}\n", stderr);
  fputs("  --help\n", stderr);
  fputs("  --jpeg[=quality]\n", stderr);
  fputs("  --left margin{in,cm,mm}\n", stderr);
  fputs("  --logo filename.{gif,jpg,png}\n", stderr);
  fputs("  --no-compression\n", stderr);
  fputs("  --no-title\n", stderr);
  fputs("  --no-toc\n", stderr);
  fputs("  --numbered\n", stderr);
  fputs("  {--outdir, -d} dirname\n", stderr);
  fputs("  {--outfile, -f} filename.{ps,pdf,html}\n", stderr);
  fputs("  --right margin{in,cm,mm}\n", stderr);
  fputs("  --size {letter,a4,WxH{in,cm,mm},etc}\n", stderr);
  fputs("  --title filename.{gif,jpg,png}\n", stderr);
  fputs("  --tocfooter fff\n", stderr);
  fputs("  --tocheader fff\n", stderr);
  fputs("  --toclevels levels\n", stderr);
  fputs("  --top margin{in,cm,mm}\n", stderr);
  fputs("  {--verbose, -v}\n", stderr);
  fputs("  --webpage\n", stderr);
  fputs("\n", stderr);
  fputs("  fff = heading format string; each \'f\' can be one of:\n", stderr);
  fputs("\n", stderr);
  fputs("        . = blank\n", stderr);
  fputs("        t = title text\n", stderr);
  fputs("        h = current heading\n", stderr);
  fputs("        c = current chapter heading\n", stderr);
  fputs("        l = logo image\n", stderr);
  fputs("        i = lowercase roman numerals\n", stderr);
  fputs("        I = uppercase roman numerals\n", stderr);
  fputs("        1 = arabic numbers (1, 2, 3, ...)\n", stderr);
  fputs("        a = lowercase letters\n", stderr);
  fputs("        A = uppercase letters\n", stderr);

  exit(1);
}


/*
 * End of "$Id: htmldoc.cxx,v 1.2 1999/11/08 22:11:35 mike Exp $".
 */
