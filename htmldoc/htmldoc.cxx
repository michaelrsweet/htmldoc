/*
 * "$Id: htmldoc.cxx,v 1.36.2.41 2002/05/07 20:40:56 mike Exp $"
 *
 *   Main entry for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2002 by Easy Software Products.
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
 *   main()            - Main entry for HTMLDOC.
 *   prefs_getrc()     - Get the rc file for preferences...
 *   prefs_load()      - Load HTMLDOC preferences...
 *   prefs_save()      - Save HTMLDOC preferences...
 *   compare_strings() - Compare two command-line strings.
 *   load_book()       - Load a book file...
 *   parse_options()   - Parse options from a book file...
 *   read_file()       - Read a file into the current document.
 *   term_handler()    - Handle CTRL-C or kill signals...
 *   usage()           - Show program version and command-line options.
 */

/*
 * Include necessary headers.
 */

#define _HTMLDOC_CXX_
#include "htmldoc.h"
#include <ctype.h>
#include <fcntl.h>

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif // HAVE_LOCALE_H

#ifdef WIN32
#  include <direct.h>
#  include <io.h>
#else
#  include <signal.h>
#  include <unistd.h>
#endif // WIN32

#ifdef __EMX__
extern "C" {
const char *__XOS2RedirRoot(const char *);
}
#endif
 

/*
 * Local types...
 */

typedef int (*exportfunc_t)(tree_t *, tree_t *);


/*
 * Local functions...
 */

static int	compare_strings(const char *s, const char *t, int tmin);
static int	load_book(const char *filename, tree_t **document,
		          exportfunc_t *exportfunc);
static void	parse_options(const char *line, exportfunc_t *exportfunc);
static int	read_file(const char *filename, tree_t **document);
static void	term_handler(int signum);
static void	usage(void);


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

  int		i, j;		/* Looping vars */
  tree_t	*document,	/* Master HTML document */
		*file,		/* HTML document file */
		*toc;		/* Table of contents */
  exportfunc_t	exportfunc;	/* Export function */
  const char	*extension;	/* Extension of output filename */
  float		fontsize,	/* Base font size */
		fontspacing;	/* Base font spacing */
#ifdef HAVE_LIBFLTK
  int		display_set;	/* True if display set */
#endif // HAVE_LIBFLTK


 /*
  * Localize as needed...
  */

#ifdef HAVE_LOCALE_H
  setlocale(LC_TIME, "");
#endif // HAVE_LOCALE_H

 /*
  * Catch CTRL-C and term signals...
  */

#ifdef WIN32
#else
  signal(SIGTERM, term_handler);
#endif // WIN32

 /*
  * Load preferences...
  */

  prefs_load();

  Errors = 0;

 /*
  * Default to producing HTML files.
  */

  document   = NULL;
  exportfunc = (exportfunc_t)html_export;

 /*
  * Parse command-line options...
  */

  fontsize    = 11.0f;
  fontspacing = 1.2f;
#ifdef HAVE_LIBFLTK
  display_set = 1;
#endif // HAVE_LIBFLTK

  for (i = 1; i < argc; i ++)
    if (compare_strings(argv[i], "--batch", 4) == 0)
    {
      i ++;
      if (i < argc)
        load_book(argv[i], &document, &exportfunc);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--bodycolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strcpy((char *)BodyColor, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--bodyfont", 7) == 0 ||
             compare_strings(argv[i], "--textfont", 7) == 0)
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
    else if (compare_strings(argv[i], "--book", 5) == 0)
      OutputType = OUTPUT_BOOK;
    else if (compare_strings(argv[i], "--bottom", 5) == 0)
    {
      i ++;
      if (i < argc)
        PageBottom = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--browserwidth", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        _htmlBrowserWidth = atof(argv[i]);

	if (_htmlBrowserWidth < 1.0f)
	{
	  progress_error(HD_ERROR_INTERNAL_ERROR, "Bad browser width \"%s\"!",
	                 argv[i]);
	  usage();
	}
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--charset", 4) == 0)
    {
      i ++;
      if (i < argc)
        htmlSetCharSet(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--color", 5) == 0)
    {
      OutputColor    = 1;
      _htmlGrayscale = 0;
    }
    else if (compare_strings(argv[i], "--compression", 5) == 0 ||
             strncmp(argv[i], "--compression=", 14) == 0)
    {
      if (strlen(argv[i]) > 14 && PDFVersion >= 1.2)
        Compression = atoi(argv[i] + 14);
      else if (PDFVersion >= 1.2)
        Compression = 1;
    }
    else if (compare_strings(argv[i], "--continuous", 5) == 0)
    {
      TocLevels    = 0;
      TitlePage    = 0;
      OutputType   = OUTPUT_CONTINUOUS;
      PDFPageMode  = PDF_DOCUMENT;
      PDFFirstPage = PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--datadir", 4) == 0)
    {
      i ++;
      if (i < argc)
        _htmlData = argv[i];
      else
        usage();
    }
#if defined(HAVE_LIBFLTK) && !WIN32
    else if (compare_strings(argv[i], "-display", 3) == 0 ||
             compare_strings(argv[i], "--display", 4) == 0)
    {
      // The X standard requires support for the -display option, but
      // we also support the GNU standard --display...
      i ++;
      display_set = 3;
      if (i < argc)
        Fl::display(argv[i]);
      else
        usage();
    }
#endif // HAVE_LIBFLTK && !WIN32
    else if (compare_strings(argv[i], "--duplex", 4) == 0)
      PageDuplex = 1;
    else if (compare_strings(argv[i], "--effectduration", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        PDFEffectDuration = atof(argv[i]);

	if (PDFEffectDuration < 0.0f)
	{
	  progress_error(HD_ERROR_INTERNAL_ERROR, "Bad effect duration \"%s\"!",
	                 argv[i]);
	  usage();
	}
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--encryption", 4) == 0)
      Encryption = 1;
    else if (compare_strings(argv[i], "--firstpage", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      for (j = 0; j < (int)(sizeof(PDFPages) / sizeof(PDFPages[0])); j ++)
        if (strcasecmp(argv[i], PDFPages[j]) == 0)
	{
	  PDFFirstPage = j;
	  break;
	}
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
    else if (compare_strings(argv[i], "--footer", 5) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], Footer);
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
        {
	  exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 1;
	}
        else if (strcasecmp(argv[i], "ps2") == 0 ||
                 strcasecmp(argv[i], "ps") == 0)
        {
	  exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 2;
	}
        else if (strcasecmp(argv[i], "ps3") == 0)
        {
	  exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 3;
	}
        else if (strcasecmp(argv[i], "pdf14") == 0)
	{
          exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 1.4;
	}
        else if (strcasecmp(argv[i], "pdf13") == 0 ||
	         strcasecmp(argv[i], "pdf") == 0)
	{
          exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 1.3;
	}
        else if (strcasecmp(argv[i], "pdf12") == 0)
	{
          exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 1.2;
	}
        else if (strcasecmp(argv[i], "pdf11") == 0)
	{
          exportfunc  = (exportfunc_t)pspdf_export;
	  PSLevel     = 0;
	  PDFVersion  = 1.1;
	  Compression = 0;
	}
        else if (strcasecmp(argv[i], "html") == 0)
          exportfunc = (exportfunc_t)html_export;
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--grayscale", 3) == 0)
    {
      OutputColor    = 0;
      _htmlGrayscale = 1;
    }
    else if (compare_strings(argv[i], "--header", 7) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], Header);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--headfootfont", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0)
	{
	  HeadFootType  = TYPE_COURIER;
	  HeadFootStyle = STYLE_NORMAL;
	}
        else if (strcasecmp(argv[i], "courier-bold") == 0)
	{
	  HeadFootType  = TYPE_COURIER;
	  HeadFootStyle = STYLE_BOLD;
	}
        else if (strcasecmp(argv[i], "courier-oblique") == 0)
	{
	  HeadFootType  = TYPE_COURIER;
	  HeadFootStyle = STYLE_ITALIC;
	}
        else if (strcasecmp(argv[i], "courier-boldoblique") == 0)
	{
	  HeadFootType  = TYPE_COURIER;
	  HeadFootStyle = STYLE_BOLD_ITALIC;
	}
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "times-roman") == 0)
	{
	  HeadFootType  = TYPE_TIMES;
	  HeadFootStyle = STYLE_NORMAL;
	}
        else if (strcasecmp(argv[i], "times-bold") == 0)
	{
	  HeadFootType  = TYPE_TIMES;
	  HeadFootStyle = STYLE_BOLD;
	}
        else if (strcasecmp(argv[i], "times-italic") == 0)
	{
	  HeadFootType  = TYPE_TIMES;
	  HeadFootStyle = STYLE_ITALIC;
	}
        else if (strcasecmp(argv[i], "times-bolditalic") == 0)
	{
	  HeadFootType  = TYPE_TIMES;
	  HeadFootStyle = STYLE_BOLD_ITALIC;
	}
        else if (strcasecmp(argv[i], "helvetica") == 0)
	{
	  HeadFootType  = TYPE_HELVETICA;
	  HeadFootStyle = STYLE_NORMAL;
	}
        else if (strcasecmp(argv[i], "helvetica-bold") == 0)
	{
	  HeadFootType  = TYPE_HELVETICA;
	  HeadFootStyle = STYLE_BOLD;
	}
        else if (strcasecmp(argv[i], "helvetica-oblique") == 0)
	{
	  HeadFootType  = TYPE_HELVETICA;
	  HeadFootStyle = STYLE_ITALIC;
	}
        else if (strcasecmp(argv[i], "helvetica-boldoblique") == 0)
	{
	  HeadFootType  = TYPE_HELVETICA;
	  HeadFootStyle = STYLE_BOLD_ITALIC;
	}
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
    else if (compare_strings(argv[i], "--help", 6) == 0)
      usage();
#ifdef HAVE_LIBFLTK
    else if (compare_strings(argv[i], "--helpdir", 7) == 0)
    {
      i ++;
      if (i < argc)
        GUI::help_dir = argv[i];
      else
        usage();
    }
#endif // HAVE_LIBFLTK
    else if (compare_strings(argv[i], "--jpeg", 3) == 0 ||
             strncmp(argv[i], "--jpeg=", 7) == 0)
    {
      if (strlen(argv[i]) > 7)
        OutputJPEG = atoi(argv[i] + 7);
      else
        OutputJPEG = 90;
    }
    else if (compare_strings(argv[i], "--landscape", 4) == 0)
      Landscape = 1;
    else if (compare_strings(argv[i], "--left", 4) == 0)
    {
      i ++;
      if (i < argc)
        PageLeft = get_measurement(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--linkcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strcpy(LinkColor, argv[i]);
      else
        usage();
    }
    else if (strcmp(argv[i], "--links") == 0)
      Links = 1;
    else if (compare_strings(argv[i], "--linkstyle", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcmp(argv[i], "plain") == 0)
	  LinkStyle = 0;
        else if (strcmp(argv[i], "underline") == 0)
	  LinkStyle = 1;
	else
	  usage();
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--logoimage", 5) == 0)
    {
      i ++;
      if (i < argc)
        strcpy(LogoImage, argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--no-compression", 6) == 0)
      Compression = 0;
    else if (compare_strings(argv[i], "--no-duplex", 4) == 0)
      PageDuplex = 0;
    else if (compare_strings(argv[i], "--no-encryption", 6) == 0)
      Encryption = 0;
    else if (compare_strings(argv[i], "--no-jpeg", 6) == 0)
      OutputJPEG = 0;
    else if (compare_strings(argv[i], "--no-links", 7) == 0)
      Links = 0;
    else if (compare_strings(argv[i], "--no-localfiles", 7) == 0)
      file_nolocal();
    else if (compare_strings(argv[i], "--no-numbered", 6) == 0)
      TocNumbers = 0;
    else if (compare_strings(argv[i], "--no-pscommands", 6) == 0)
      PSCommands = 0;
    else if (compare_strings(argv[i], "--no-title", 7) == 0)
      TitlePage = 0;
    else if (compare_strings(argv[i], "--no-toc", 7) == 0)
      TocLevels = 0;
    else if (compare_strings(argv[i], "--no-truetype", 7) == 0)
      TrueType = 0;
    else if (compare_strings(argv[i], "--no-xrxcomments", 6) == 0)
      XRXComments = 0;
    else if (compare_strings(argv[i], "--numbered", 5) == 0)
      TocNumbers = 1;
    else if (compare_strings(argv[i], "--nup", 5) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      NumberUp = atoi(argv[i]);

      if (NumberUp != 1 && NumberUp != 2 && NumberUp != 4 &&
          NumberUp != 6 && NumberUp != 9 && NumberUp != 16)
	usage();
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
          if (strcasecmp(extension, "ps") == 0)
          {
	    exportfunc = (exportfunc_t)pspdf_export;

	    if (PSLevel == 0)
	      PSLevel = 2;
	  }
          else if (strcasecmp(extension, "pdf") == 0)
	  {
            exportfunc = (exportfunc_t)pspdf_export;
	    PSLevel    = 0;
          }
	  else if (strcasecmp(extension, "html") == 0)
            exportfunc = (exportfunc_t)html_export;
        }
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--owner-password", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        strncpy(OwnerPassword, argv[i], sizeof(OwnerPassword) - 1);
	OwnerPassword[sizeof(OwnerPassword) - 1] = '\0';
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--pageduration", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        PDFPageDuration = atof(argv[i]);

	if (PDFPageDuration < 1.0f)
	{
	  progress_error(HD_ERROR_INTERNAL_ERROR, "Bad page duration \"%s\"!",
	                 argv[i]);
	  usage();
	}
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--pageeffect", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      for (j = 0; j < (int)(sizeof(PDFEffects) / sizeof(PDFEffects[0])); j ++)
        if (strcasecmp(argv[i], PDFEffects[j]) == 0)
	{
	  PDFEffect = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--pagelayout", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      for (j = 0; j < (int)(sizeof(PDFLayouts) / sizeof(PDFLayouts[0])); j ++)
        if (strcasecmp(argv[i], PDFLayouts[j]) == 0)
	{
	  PDFPageLayout = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--pagemode", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      for (j = 0; j < (int)(sizeof(PDFModes) / sizeof(PDFModes[0])); j ++)
        if (strcasecmp(argv[i], PDFModes[j]) == 0)
	{
	  PDFPageMode = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--path", 5) == 0)
    {
      i ++;
      if (i < argc)
      {
        strncpy(Path, argv[i], sizeof(Path) - 1);
	Path[sizeof(Path) - 1] = '\0';
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--permissions", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      if (strcasecmp(argv[i], "all") == 0)
        Permissions = -4;
      else if (strcasecmp(argv[i], "none") == 0)
        Permissions = -64;
      else if (strcasecmp(argv[i], "print") == 0)
        Permissions |= PDF_PERM_PRINT;
      else if (strcasecmp(argv[i], "no-print") == 0)
        Permissions &= ~PDF_PERM_PRINT;
      else if (strcasecmp(argv[i], "modify") == 0)
        Permissions |= PDF_PERM_MODIFY;
      else if (strcasecmp(argv[i], "no-modify") == 0)
        Permissions &= ~PDF_PERM_MODIFY;
      else if (strcasecmp(argv[i], "copy") == 0)
        Permissions |= PDF_PERM_COPY;
      else if (strcasecmp(argv[i], "no-copy") == 0)
        Permissions &= ~PDF_PERM_COPY;
      else if (strcasecmp(argv[i], "annotate") == 0)
        Permissions |= PDF_PERM_ANNOTATE;
      else if (strcasecmp(argv[i], "no-annotate") == 0)
        Permissions &= ~PDF_PERM_ANNOTATE;
    }
    else if (compare_strings(argv[i], "--portrait", 4) == 0)
      Landscape = 0;
    else if (compare_strings(argv[i], "--proxy", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        strncpy(Proxy, argv[i], sizeof(Proxy) - 1);
	Proxy[sizeof(Proxy) - 1] = '\0';
	file_proxy(Proxy);
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--pscommands", 3) == 0)
      PSCommands = 1;
    else if (compare_strings(argv[i], "--quiet", 3) == 0)
      Verbosity = -1;
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
    else if (compare_strings(argv[i], "--textcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        htmlSetTextColor((uchar *)argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--title", 7) == 0)
      TitlePage = 1;
    else if (compare_strings(argv[i], "--titlefile", 8) == 0 ||
             compare_strings(argv[i], "--titleimage", 8) == 0)
    {
      i ++;
      if (i < argc)
        strcpy(TitleImage, argv[i]);
      else
        usage();

      TitlePage = 1;
    }
    else if (compare_strings(argv[i], "--tocfooter", 6) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], TocFooter);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--tocheader", 6) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], TocHeader);
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
    else if (compare_strings(argv[i], "--toctitle", 6) == 0)
    {
      i ++;
      if (i < argc)
      {
        strncpy(TocTitle, argv[i], sizeof(TocTitle) - 1);
	TocTitle[sizeof(TocTitle) - 1] = '\0';
      }
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
    else if (compare_strings(argv[i], "--user-password", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        strncpy(UserPassword, argv[i], sizeof(UserPassword) - 1);
	UserPassword[sizeof(UserPassword) - 1] = '\0';
      }
      else
        usage();
    }
    else if (compare_strings(argv[i], "--truetype", 4) == 0)
      TrueType = 1;
    else if (compare_strings(argv[i], "--verbose", 6) == 0 ||
             strcmp(argv[i], "-v") == 0)
    {
      Verbosity ++;
    }
    else if (compare_strings(argv[i], "--version", 6) == 0)
    {
      puts(SVERSION);
      return (0);
    }
    else if (compare_strings(argv[i], "--webpage", 3) == 0)
    {
      TocLevels    = 0;
      TitlePage    = 0;
      OutputType   = OUTPUT_WEBPAGES;
      PDFPageMode  = PDF_DOCUMENT;
      PDFFirstPage = PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--xrxcomments", 3) == 0)
      XRXComments = 1;
    else if (strcmp(argv[i], "-") == 0)
    {
     /*
      * Read from stdin...
      */

      _htmlPPI = _htmlBrowserWidth / PageWidth * 72.0f;

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME", (uchar *)"");

#ifdef WIN32
      // Make sure stdin is in binary mode.
      // (I hate Microsoft... I hate Microsoft... Everybody join in!)
      setmode(0, O_BINARY);
#elif defined(__EMX__)
      // OS/2 has a setmode for FILE's...
      fflush(stdin);
      _fsetmode(stdin, "b");
#endif // WIN32 || __EMX__

      htmlReadFile(file, stdin, ".");

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
    else if (argv[i][0] == '-')
      usage();
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
    else
      read_file(argv[i], &document);

 /*
  * Display the GUI if necessary...
  */

#ifdef HAVE_LIBFLTK
  if (document == NULL && BookGUI == NULL)
  {
    if (argc == display_set)
      BookGUI = new GUI();
    else
      usage();
  }

  if (BookGUI != NULL)
  {
    FileIcon::load_system_icons();

    BookGUI->show();

    i = Fl::run();

    delete BookGUI;

    return (i);
  }
#endif /* HAVE_LIBFLTK */
    
 /*
  * We *must* have a document to process...
  */

  if (document == NULL)
  {
    puts("ERROR: No HTML files!");
    usage();
  }

 /*
  * Find the first one in the list...
  */

  while (document->prev != NULL)
    document = document->prev;

 /*
  * Build a table of contents for the documents if necessary...
  */

  if (OutputType == OUTPUT_BOOK && TocLevels > 0)
    toc = toc_build(document);
  else
    toc = NULL;

 /*
  * Generate the output file(s).
  */

  (*exportfunc)(document, toc);

  htmlDeleteTree(document);
  htmlDeleteTree(toc);

  file_cleanup();
  image_flush_cache();

  return (Errors);
}


/*
 * 'prefs_getrc()' - Get the rc file for preferences...
 */

const char *
prefs_getrc(void)
{
#ifdef WIN32
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string
  char		home[1024];	// Home (profile) directory
#else
  const char	*home;		// Home directory
#endif // WIN32
  static char	htmldocrc[1024];// HTMLDOC RC file


  // Find the home directory...
#ifdef WIN32
  // Open the registry...
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0,
		   KEY_READ, &key))
  {
    // Use the install directory...
    strcpy(home, _htmlData);
  }
  else
  {
    // Grab the current user's AppData directory...
    size = sizeof(home);
    if (RegQueryValueEx(key, "AppData", NULL, NULL, (unsigned char *)home, &size))
      strcpy(home, _htmlData);

    RegCloseKey(key);
  }
#else
  if ((home = getenv("HOME")) == NULL)
    home = _htmlData;
#endif // WIN32

  // Format the rc filename and return...
  snprintf(htmldocrc, sizeof(htmldocrc), "%s/.htmldocrc", home);

  return (htmldocrc);
}


/*
 * 'prefs_load()' - Load HTMLDOC preferences...
 */

void
prefs_load(void)
{
  int	pos;			// Header/footer position
  char	line[2048];		// Line from RC file
  FILE	*fp;			// File pointer
#ifdef WIN32			//// Do registry magic...
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string
  static char	data[1024];	// Data directory
  static char	doc[1024];	// Documentation directory
#endif // WIN32


  //
  // Get the installed directories...
  //

#ifdef WIN32
  // Open the registry...
  if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Easy Software Products\\HTMLDOC", 0,
                    KEY_READ, &key))
  {
    // Grab the installed directories...
    size = sizeof(data);
    if (!RegQueryValueEx(key, "data", NULL, NULL, (unsigned char *)data, &size))
      _htmlData = data;

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
  _htmlData = strdup(__XOS2RedirRoot("/XFree86/lib/X11/htmldoc"));
  GUI::help_dir = strdup(__XOS2RedirRoot("/XFree86/lib/X11/htmldoc/doc"));
#endif // __EMX__ && HAVE_LIBFLTK

  //
  // See if the installed directories have been overridden by
  // environment variables...
  //

  if (getenv("HTMLDOC_DATA") != NULL)
    _htmlData = getenv("HTMLDOC_DATA");

#ifdef HAVE_LIBFLTK
  if (getenv("HTMLDOC_HELP") != NULL)
    GUI::help_dir = getenv("HTMLDOC_HELP");
#endif // HAVE_LIBFLTK

  //
  // Read the preferences file...
  //

  if ((fp = fopen(prefs_getrc(), "r")) != NULL)
  {
    while (fgets(line, sizeof(line), fp) != NULL)
    {
      if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';

      if (strncasecmp(line, "TEXTCOLOR=", 10) == 0)
	htmlSetTextColor((uchar *)(line + 10));
      else if (strncasecmp(line, "BODYCOLOR=", 10) == 0)
	strcpy(BodyColor, line + 10);
      else if (strncasecmp(line, "BODYIMAGE=", 10) == 0)
	strcpy(BodyImage, line + 10);
      else if (strncasecmp(line, "LINKCOLOR=", 10) == 0)
        strcpy(LinkColor, line + 10);
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
      else if (strncasecmp(line, "TOCHEADER=", 10) == 0)
	get_format(line + 10, TocHeader);
      else if (strncasecmp(line, "TOCFOOTER=", 10) == 0)
	get_format(line + 10, TocFooter);
      else if (strncasecmp(line, "TOCTITLE=", 9) == 0)
	strcpy(TocTitle, line + 9);
      else if (strncasecmp(line, "BODYFONT=", 9) == 0)
	_htmlBodyFont = (typeface_t)atoi(line + 9);
      else if (strncasecmp(line, "HEADINGFONT=", 12) == 0)
	_htmlHeadingFont = (typeface_t)atoi(line + 12);
      else if (strncasecmp(line, "FONTSIZE=", 9) == 0)
	htmlSetBaseSize(atof(line + 9),
	                _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
      else if (strncasecmp(line, "FONTSPACING=", 12) == 0)
	htmlSetBaseSize(_htmlSizes[SIZE_P], atof(line + 12));
      else if (strncasecmp(line, "HEADFOOTTYPE=", 13) == 0)
	HeadFootType = (typeface_t)atoi(line + 13);
      else if (strncasecmp(line, "HEADFOOTSTYLE=", 14) == 0)
        HeadFootStyle = (style_t)atoi(line + 14);
      else if (strncasecmp(line, "HEADFOOTSIZE=", 13) == 0)
	HeadFootSize = atof(line + 13);
      else if (strncasecmp(line, "PDFVERSION=", 11) == 0)
	PDFVersion = atof(line + 11);
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
        TrueType = atoi(line + 9);
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
#  ifdef HAVE_LIBFLTK
      else if (strncasecmp(line, "EDITOR=", 7) == 0)
        strcpy(HTMLEditor, line + 7);
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


/*
 * 'prefs_save()' - Save HTMLDOC preferences...
 */

void
prefs_save(void)
{
  FILE	*fp;			// File pointer


  if ((fp = fopen(prefs_getrc(), "w")) != NULL)
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
    fprintf(fp, "PDFVERSION=%.1f\n", PDFVersion);
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
    fprintf(fp, "TRUETYPE=%d\n", TrueType);
    fprintf(fp, "PATH=%s\n", Path);
    fprintf(fp, "PROXY=%s\n", Proxy);

#ifdef HAVE_LIBFLTK
    fprintf(fp, "EDITOR=%s\n", HTMLEditor);
    fprintf(fp, "TOOLTIPS=%d\n", Tooltips);
    fprintf(fp, "MODERN=%d\n", ModernSkin);
#endif // HAVE_LIBFLTK

    fclose(fp);
  }
}


/*
 * 'compare_strings()' - Compare two command-line strings.
 */

static int			/* O - -1 or 1 = no match, 0 = match */
compare_strings(const char *s,	/* I - Command-line string */
                const char *t,	/* I - Option string */
                int        tmin)/* I - Minimum number of unique chars in option */
{
  int	slen;			/* Length of command-line string */


  slen = strlen(s);
  if (slen < tmin)
    return (-1);
  else
    return (strncmp(s, t, slen));
}


//
// 'load_book()' - Load a book file...
//

static int				// O  - 1 = success, 0 = failure
load_book(const char   *filename,	// I  - Book file
          tree_t       **document,	// IO - Document tree
          exportfunc_t *exportfunc)	// O  - Export function
{
  FILE		*fp;			// File to read from
  char		line[10240];		// Line from file
  const char 	*dir;			// Directory


  // If the filename contains a path, chdir to it first...
  if ((dir = file_directory(filename)) != NULL)
  {
   /*
    * Filename contains a complete path - get the directory portion and do
    * a chdir()...
    */

    chdir(dir);

    filename = file_basename(filename);
  }

  // Open the file...
  if ((fp = fopen(filename, "rb")) == NULL)
  {
    fprintf(stderr, "htmldoc: Unable to open \"%s\": %s\n", filename,
            strerror(errno));
    return (0);
  }

  // Get the header...
  file_gets(line, sizeof(line), fp);
  if (strncmp(line, "#HTMLDOC", 8) != 0)
  {
    fclose(fp);
    fprintf(stderr, "htmldoc: Bad or missing #HTMLDOC header in %s.", filename);
    return (0);
  }

  // Read the second line from the book file; for older book files, this will
  // be the file count; for new files this will be the options...
  do
  {
    file_gets(line, sizeof(line), fp);

    if (line[0] == '-')
      parse_options(line, exportfunc);
  }
  while (!line[0]);			// Skip blank lines

  // Get input files/options...
  while (file_gets(line, sizeof(line), fp) != NULL)
  {
    if (!line[0])
      continue;				// Skip blank lines
    else if (line[0] == '-')
      parse_options(line, exportfunc);
    else if (line[0] == '\\')
      read_file(line + 1, document);
    else
      read_file(line, document);
  }

  // Close the book file and return...
  fclose(fp);

  return (1);
}


//
// 'parse_options()' - Parse options from a book file...
//

static void
parse_options(const char   *line,	// I - Options from book file
              exportfunc_t *exportfunc)	// O - Export function
{
  int		i;			// Looping var
  const char	*lineptr;		// Pointer into line
  char		temp[1024],		// Option name
		temp2[1024],		// Option value
		*tempptr;		// Pointer into option
  float		fontsize,		// Size of body text
		fontspacing;		// Spacing between lines


  // Parse the input line...
  for (lineptr = line; *lineptr != '\0';)
  {
    while (*lineptr == ' ')
      lineptr ++;

    for (tempptr = temp; *lineptr != '\0' && *lineptr != ' '; lineptr ++)
      if (tempptr < (temp + sizeof(temp) - 1))
        *tempptr++ = *lineptr;

    *tempptr = '\0';

    while (*lineptr == ' ')
      lineptr ++;

    if (strcmp(temp, "--duplex") == 0)
    {
      PageDuplex = 1;
      continue;
    }
    else if (strcmp(temp, "--landscape") == 0)
    {
      Landscape = 1;
      continue;
    }
    else if (strcmp(temp, "--portrait") == 0)
    {
      Landscape = 0;
      continue;
    }
    else if (strncmp(temp, "--jpeg", 6) == 0)
    {
      if (strlen(temp) > 7)
        OutputJPEG = atoi(temp + 7);
      else
        OutputJPEG = 90;
      continue;
    }
    else if (strcmp(temp, "--grayscale") == 0)
    {
      OutputColor = 0;
      continue;
    }
    else if (strcmp(temp, "--color") == 0)
    {
      OutputColor = 1;
      continue;
    }
    else if (strcmp(temp, "--links") == 0)
    {
      Links = 1;
      continue;
    }
    else if (strcmp(temp, "--no-links") == 0)
    {
      Links = 0;
      continue;
    }
    else if (strcmp(temp, "--truetype") == 0)
    {
      TrueType = 1;
      continue;
    }
    else if (strcmp(temp, "--no-truetype") == 0)
    {
      TrueType = 0;
      continue;
    }
    else if (strcmp(temp, "--pscommands") == 0)
    {
      PSCommands = 1;
      continue;
    }
    else if (strcmp(temp, "--no-pscommands") == 0)
    {
      PSCommands = 0;
      continue;
    }
    else if (strcmp(temp, "--xrxcomments") == 0)
    {
      XRXComments = 1;
      continue;
    }
    else if (strcmp(temp, "--no-xrxcomments") == 0)
    {
      XRXComments = 0;
      continue;
    }
    else if (strncmp(temp, "--compression", 13) == 0)
    {
      if (strlen(temp) > 14)
        Compression = atoi(temp + 14);
      else
        Compression = 1;
      continue;
    }
    else if (strcmp(temp, "--no-compression") == 0)
    {
      Compression = 0;
      continue;
    }
    else if (strcmp(temp, "--no-jpeg") == 0)
    {
      OutputJPEG = 0;
      continue;
    }
    else if (strcmp(temp, "--numbered") == 0)
    {
      TocNumbers = 1;
      continue;
    }
    else if (strcmp(temp, "--no-numbered") == 0)
    {
      TocNumbers = 0;
      continue;
    }
    else if (strcmp(temp, "--no-toc") == 0)
    {
      TocLevels = 0;
      continue;
    }
    else if (strcmp(temp, "--title") == 0 &&
             (*lineptr == '-' || !*lineptr))
    {
      TitlePage = 1;
      continue;
    }
    else if (strcmp(temp, "--no-title") == 0)
    {
      TitlePage = 0;
      continue;
    }
    else if (strcmp(temp, "--book") == 0)
    {
      OutputType = OUTPUT_BOOK;
      continue;
    }
    else if (strcmp(temp, "--continuous") == 0)
    {
      OutputType = OUTPUT_CONTINUOUS;
      continue;
    }
    else if (strcmp(temp, "--webpage") == 0)
    {
      OutputType = OUTPUT_WEBPAGES;
      continue;
    }
    else if (strcmp(temp, "--encryption") == 0)
    {
      Encryption = 1;
      continue;
    }
    else if (strcmp(temp, "--no-encryption") == 0)
    {
      Encryption = 0;
      continue;
    }

    if (*lineptr == '\"')
    {
      lineptr ++;

      for (tempptr = temp2; *lineptr != '\0' && *lineptr != '\"'; lineptr ++)
        if (tempptr < (temp2 + sizeof(temp2) - 1))
	  *tempptr++ = *lineptr;

      if (*lineptr == '\"')
        lineptr ++;
    }
    else
    {
      for (tempptr = temp2; *lineptr != '\0' && *lineptr != ' '; lineptr ++)
        if (tempptr < (temp2 + sizeof(temp2) - 1))
	  *tempptr++ = *lineptr;
    }

    *tempptr = '\0';

    if (strcmp(temp, "-t") == 0)
    {
      if (strcmp(temp2, "html") == 0)
        *exportfunc = (exportfunc_t)html_export;
      else if (strcmp(temp2, "ps1") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 1;
      }
      else if (strcmp(temp2, "ps") == 0 ||
               strcmp(temp2, "ps2") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 2;
      }
      else if (strcmp(temp2, "ps3") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 3;
      }
      else if (strcmp(temp2, "pdf11") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 1.1f;
      }
      else if (strcmp(temp2, "pdf12") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 1.2f;
      }
      else if (strcmp(temp2, "pdf") == 0 ||
               strcmp(temp2, "pdf13") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 1.3f;
      }
      else if (strcmp(temp2, "pdf14") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 1.4f;
      }
    }
    else if (strcmp(temp, "--logo") == 0 ||
             strcmp(temp, "--logoimage") == 0)
    {
      strncpy(LogoImage, temp2, sizeof(LogoImage) - 1);
      LogoImage[sizeof(LogoImage) - 1] = '\0';
    }
    else if (strcmp(temp, "--titleimage") == 0)
    {
      TitlePage = 1;
      strncpy(TitleImage, temp2, sizeof(TitleImage) - 1);
      TitleImage[sizeof(TitleImage) - 1] = '\0';
    }
    else if (strcmp(temp, "-f") == 0)
    {
      OutputFiles = 0;
      strncpy(OutputPath, temp2, sizeof(OutputPath) - 1);
      OutputPath[sizeof(OutputPath) - 1] = '\0';
    }
    else if (strcmp(temp, "-d") == 0)
    {
      OutputFiles = 1;
      strncpy(OutputPath, temp2, sizeof(OutputPath) - 1);
      OutputPath[sizeof(OutputPath) - 1] = '\0';
    }
    else if (strcmp(temp, "--browserwidth") == 0)
      _htmlBrowserWidth = atof(temp2);
    else if (strcmp(temp, "--nup") == 0)
      NumberUp = atoi(temp2);
    else if (strcmp(temp, "--size") == 0)
      set_page_size(temp2);
    else if (strcmp(temp, "--left") == 0)
      PageLeft = get_measurement(temp2);
    else if (strcmp(temp, "--right") == 0)
      PageRight = get_measurement(temp2);
    else if (strcmp(temp, "--top") == 0)
      PageTop = get_measurement(temp2);
    else if (strcmp(temp, "--bottom") == 0)
      PageBottom = get_measurement(temp2);
    else if (strcmp(temp, "--header") == 0)
      get_format(temp2, Header);
    else if (strcmp(temp, "--footer") == 0)
      get_format(temp2, Footer);
    else if (strcmp(temp, "--bodycolor") == 0)
    {
      strncpy(BodyColor, temp2, sizeof(BodyColor) - 1);
      BodyColor[sizeof(BodyColor) - 1] = '\0';
    }
    else if (strcmp(temp, "--bodyimage") == 0)
    {
      strncpy(BodyImage, temp2, sizeof(BodyImage) - 1);
      BodyImage[sizeof(BodyImage) - 1] = '\0';
    }
    else if (strcmp(temp, "--textcolor") == 0)
      htmlSetTextColor((uchar *)temp2);
    else if (strcmp(temp, "--linkcolor") == 0)
    {
      strncpy(LinkColor, temp2, sizeof(LinkColor) - 1);
      LinkColor[sizeof(LinkColor) - 1] = '\0';
    }
    else if (strcmp(temp, "--linkstyle") == 0)
    {
      if (strcmp(temp2, "plain") == 0)
        LinkStyle = 0;
      else
        LinkStyle = 1;
    }
    else if (strcmp(temp, "--toclevels") == 0)
      TocLevels = atoi(temp2);
    else if (strcmp(temp, "--tocheader") == 0)
      get_format(temp2, TocHeader);
    else if (strcmp(temp, "--tocfooter") == 0)
      get_format(temp2, TocFooter);
    else if (strcmp(temp, "--toctitle") == 0)
    {
      strncpy(TocTitle, temp2, sizeof(TocTitle) - 1);
      TocTitle[sizeof(TocTitle) - 1] = '\0';
    }
    else if (strcmp(temp, "--fontsize") == 0)
    {
      fontsize    = atof(temp2);
      fontspacing = _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P];

      if (fontsize < 6.0f)
        fontsize = 6.0f;
      else if (fontsize > 24.0f)
        fontsize = 24.0f;

      htmlSetBaseSize(fontsize, fontspacing);
    }
    else if (strcmp(temp, "--fontspacing") == 0)
    {
      fontsize    = _htmlSizes[SIZE_P];
      fontspacing = atof(temp2);

      if (fontspacing < 1.0f)
        fontspacing = 1.0f;
      else if (fontspacing > 3.0f)
        fontspacing = 3.0f;

      htmlSetBaseSize(fontsize, fontspacing);
    }
    else if (strcmp(temp, "--headingfont") == 0)
    {
      if (strcasecmp(temp2, "courier") == 0 ||
          strcasecmp(temp2, "monospace") == 0)
        _htmlHeadingFont = TYPE_COURIER;
      else if (strcasecmp(temp2, "times") == 0 ||
               strcasecmp(temp2, "serif") == 0)
        _htmlHeadingFont = TYPE_TIMES;
      else if (strcasecmp(temp2, "helvetica") == 0 ||
               strcasecmp(temp2, "arial") == 0 ||
               strcasecmp(temp2, "sans-serif") == 0)
        _htmlHeadingFont = TYPE_HELVETICA;
    }
    else if (strcmp(temp, "--bodyfont") == 0)
    {
      if (strcasecmp(temp2, "courier") == 0 ||
          strcasecmp(temp2, "monospace") == 0)
        _htmlBodyFont = TYPE_COURIER;
      else if (strcasecmp(temp2, "times") == 0 ||
               strcasecmp(temp2, "serif") == 0)
        _htmlBodyFont = TYPE_TIMES;
      else if (strcasecmp(temp2, "helvetica") == 0 ||
               strcasecmp(temp2, "arial") == 0 ||
               strcasecmp(temp2, "sans-serif") == 0)
        _htmlBodyFont = TYPE_HELVETICA;
    }
    else if (strcmp(temp, "--headfootsize") == 0)
      HeadFootSize = atof(temp2);
    else if (strcmp(temp, "--headfootfont") == 0)
    {
      if (strcasecmp(temp2, "courier") == 0)
      {
	HeadFootType  = TYPE_COURIER;
	HeadFootStyle = STYLE_NORMAL;
      }
      else if (strcasecmp(temp2, "courier-bold") == 0)
      {
	HeadFootType  = TYPE_COURIER;
	HeadFootStyle = STYLE_BOLD;
      }
      else if (strcasecmp(temp2, "courier-oblique") == 0)
      {
	HeadFootType  = TYPE_COURIER;
	HeadFootStyle = STYLE_ITALIC;
      }
      else if (strcasecmp(temp2, "courier-boldoblique") == 0)
      {
	HeadFootType  = TYPE_COURIER;
	HeadFootStyle = STYLE_BOLD_ITALIC;
      }
      else if (strcasecmp(temp2, "times") == 0 ||
	         strcasecmp(temp2, "times-roman") == 0)
      {
	HeadFootType  = TYPE_TIMES;
	HeadFootStyle = STYLE_NORMAL;
      }
      else if (strcasecmp(temp2, "times-bold") == 0)
      {
	HeadFootType  = TYPE_TIMES;
	HeadFootStyle = STYLE_BOLD;
      }
      else if (strcasecmp(temp2, "times-italic") == 0)
      {
	HeadFootType  = TYPE_TIMES;
	HeadFootStyle = STYLE_ITALIC;
      }
      else if (strcasecmp(temp2, "times-bolditalic") == 0)
      {
	HeadFootType  = TYPE_TIMES;
	HeadFootStyle = STYLE_BOLD_ITALIC;
      }
      else if (strcasecmp(temp2, "helvetica") == 0)
      {
	HeadFootType  = TYPE_HELVETICA;
	HeadFootStyle = STYLE_NORMAL;
      }
      else if (strcasecmp(temp2, "helvetica-bold") == 0)
      {
	HeadFootType  = TYPE_HELVETICA;
	HeadFootStyle = STYLE_BOLD;
      }
      else if (strcasecmp(temp2, "helvetica-oblique") == 0)
      {
	HeadFootType  = TYPE_HELVETICA;
	HeadFootStyle = STYLE_ITALIC;
      }
      else if (strcasecmp(temp2, "helvetica-boldoblique") == 0)
      {
	HeadFootType  = TYPE_HELVETICA;
        HeadFootStyle = STYLE_BOLD_ITALIC;
      }
    }
    else if (strcmp(temp, "--charset") == 0)
      htmlSetCharSet(temp2);
    else if (strcmp(temp, "--pagemode") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFModes) / sizeof(PDFModes[0])); i ++)
        if (strcasecmp(temp2, PDFModes[i]) == 0)
	{
	  PDFPageMode = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pagelayout") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFLayouts) / sizeof(PDFLayouts[0])); i ++)
        if (strcasecmp(temp2, PDFLayouts[i]) == 0)
	{
	  PDFPageLayout = i;
	  break;
	}
    }
    else if (strcmp(temp, "--firstpage") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFPages) / sizeof(PDFPages[0])); i ++)
        if (strcasecmp(temp2, PDFPages[i]) == 0)
	{
	  PDFFirstPage = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pageeffect") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFEffects) / sizeof(PDFEffects[0])); i ++)
        if (strcasecmp(temp2, PDFEffects[i]) == 0)
	{
	  PDFEffect = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pageduration") == 0)
      PDFPageDuration = atof(temp2);
    else if (strcmp(temp, "--effectduration") == 0)
      PDFEffectDuration = atof(temp2);
    else if (strcmp(temp, "--permissions") == 0)
    {
      if (strcasecmp(temp2, "all") == 0)
        Permissions = -4;
      else if (strcasecmp(temp2, "none") == 0)
        Permissions = -64;
      else if (strcasecmp(temp2, "print") == 0)
        Permissions |= PDF_PERM_PRINT;
      else if (strcasecmp(temp2, "no-print") == 0)
        Permissions &= ~PDF_PERM_PRINT;
      else if (strcasecmp(temp2, "modify") == 0)
        Permissions |= PDF_PERM_MODIFY;
      else if (strcasecmp(temp2, "no-modify") == 0)
        Permissions &= ~PDF_PERM_MODIFY;
      else if (strcasecmp(temp2, "copy") == 0)
        Permissions |= PDF_PERM_COPY;
      else if (strcasecmp(temp2, "no-copy") == 0)
        Permissions &= ~PDF_PERM_COPY;
      else if (strcasecmp(temp2, "annotate") == 0)
        Permissions |= PDF_PERM_ANNOTATE;
      else if (strcasecmp(temp2, "no-annotate") == 0)
        Permissions &= ~PDF_PERM_ANNOTATE;
    }
    else if (strcmp(temp, "--user-password") == 0)
    {
      strncpy(UserPassword, temp2, sizeof(UserPassword) - 1);
      UserPassword[sizeof(UserPassword) - 1] = '\0';
    }
    else if (strcmp(temp, "--owner-password") == 0)
    {
      strncpy(OwnerPassword, temp2, sizeof(OwnerPassword) - 1);
      OwnerPassword[sizeof(OwnerPassword) - 1] = '\0';
    }
    else if (strcmp(temp, "--path") == 0)
    {
      strncpy(Path, temp2, sizeof(Path) - 1);
      Path[sizeof(Path) - 1] = '\0';
    }
    else if (strcmp(temp, "--proxy") == 0)
    {
      strncpy(Proxy, temp2, sizeof(Proxy) - 1);
      Proxy[sizeof(Proxy) - 1] = '\0';
      file_proxy(Proxy);
    }
  }
}


//
// 'read_file()' - Read a file into the current document.
//

static int				// O  - 1 on success, 0 on failure
read_file(const char *filename,		// I  - File/URL to read
          tree_t     **document)	// IO - Current document
{
  FILE		*docfile;		// Document file
  tree_t	*file;			// HTML document file
  const char	*realname;		// Real name of file
  char		base[1024];		// Base directory name of file


  if ((realname = file_find(Path, filename)) != NULL)
  {
    if ((docfile = fopen(realname, "rb")) != NULL)
    {
     /*
      * Read from a file...
      */

      if (Verbosity > 0)
        fprintf(stderr, "htmldoc: Reading %s...\n", filename);

      _htmlPPI = _htmlBrowserWidth / PageWidth * 72.0f;

      strcpy(base, file_directory(filename));

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME",
                      (uchar *)file_basename(filename));

      htmlReadFile(file, docfile, base);

      fclose(docfile);

      if (file->child != NULL)
      {
        if (*document == NULL)
          *document = file;
        else
        {
          while ((*document)->next != NULL)
            *document = (*document)->next;

          (*document)->next = file;
          file->prev        = *document;
        }
      }
      else
        htmlDeleteTree(file);
    }
    else
    {
      file = NULL;
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open \"%s\" for reading...", filename);
    }
  }
  else
  {
    file = NULL;
    progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to find \"%s\"...",
                   filename);
  }

  return (file != NULL);
}


//
// 'term_handler()' - Handle CTRL-C or kill signals...
//

static void
term_handler(int signum)	// I - Signal number
{
  REF(signum);

  file_cleanup();
  image_flush_cache();
  exit(1);
}


/*
 * 'usage()' - Show program version and command-line options.
 */

static void
usage(void)
{
  puts("HTMLDOC Version " SVERSION " Copyright 1997-2002 Easy Software Products, All Rights Reserved.");
  puts("This software is governed by the GNU General Public License, Version 2, and");
  puts("is based in part on the work of the Independent JPEG Group.");
  puts("");
  puts("Usage:");
  puts("  htmldoc [options] filename1.html [ ... filenameN.html ]");
#ifdef HAVE_LIBFLTK
  puts("  htmldoc filename.book");
#endif // HAVE_LIBFLTK
  puts("");
  puts("Options:");
  puts("");
  puts("  --batch filename.book");
  puts("  --bodycolor color");
  puts("  --bodyfont {courier,times,helvetica}");
  puts("  --bodyimage filename.{gif,jpg,png}");
  puts("  --book");
  puts("  --bottom margin{in,cm,mm}");
  puts("  --browserwidth pixels");
  puts("  --charset {8859-1...8859-15}");
  puts("  --color");
  puts("  --compression[=level]");
  puts("  --datadir directory");
  puts("  --duplex");
  puts("  --effectduration {0.1..10.0}");
  puts("  --encryption");
  puts("  --firstpage {p1,toc,c1}");
  puts("  --fontsize {6.0..24.0}");
  puts("  --fontspacing {1.0..3.0}");
  puts("  --footer fff");
  puts("  {--format, -t} {ps1,ps2,ps3,pdf11,pdf12,pdf13,pdf14,html}");
  puts("  --gray");
  puts("  --header fff");
  puts("  --headfootfont {courier{-bold,-oblique,-boldoblique},\n"
       "                  times{-roman,-bold,-italic,-bolditalic},\n"
       "                  helvetica{-bold,-oblique,-boldoblique}}");
  puts("  --headfootsize {6.0..24.0}");
  puts("  --headingfont {courier,times,helvetica}");
  puts("  --help");
#ifdef HAVE_LIBFLTK
  puts("  --helpdir directory");
#endif // HAVE_LIBFLTK
  puts("  --jpeg[=quality]");
  puts("  --landscape");
  puts("  --left margin{in,cm,mm}");
  puts("  --linkcolor color");
  puts("  --links");
  puts("  --linkstyle {plain,underline}");
  puts("  --logoimage filename.{gif,jpg,png}");
  puts("  --owner-password password");
  puts("  --no-compression");
  puts("  --no-duplex");
  puts("  --no-encryption");
  puts("  --no-links");
  puts("  --no-localfiles");
  puts("  --no-numbered");
  puts("  --no-pscommands");
  puts("  --no-title");
  puts("  --no-toc");
  puts("  --no-truetype");
  puts("  --numbered");
  puts("  --nup {1,2,4,6,9,16}");
  puts("  {--outdir, -d} dirname");
  puts("  {--outfile, -f} filename.{ps,pdf,html}");
  puts("  --pageduration {1.0..60.0}");
  puts("  --pageeffect {none,bi,bo,d,gd,gdr,gr,hb,hsi,hso,vb,vsi,vso,wd,wl,wr,wu}");
  puts("  --pagelayout {single,one,twoleft,tworight}");
  puts("  --pagemode {document,outline,fullscreen}");
  puts("  --path \"dir1;dir2;dir3;...;dirN\"");
  puts("  --permissions {all,annotate,copy,modify,print,no-annotate,no-copy,no-modify,no-print,none}");
  puts("  --portrait");
  puts("  --proxy http://host:port");
  puts("  --pscommands");
  puts("  --quiet");
  puts("  --right margin{in,cm,mm}");
  puts("  --size {letter,a4,WxH{in,cm,mm},etc}");
  puts("  --textcolor color");
  puts("  --textfont {courier,times,helvetica}");
  puts("  --title");
  puts("  --titlefile filename.{htm,html,shtml}");
  puts("  --titleimage filename.{gif,jpg,png}");
  puts("  --tocfooter fff");
  puts("  --tocheader fff");
  puts("  --toclevels levels");
  puts("  --toctitle string");
  puts("  --top margin{in,cm,mm}");
  puts("  --truetype");
  puts("  --user-password password");
  puts("  {--verbose, -v}");
  puts("  --version");
  puts("  --webpage");
  puts("");
  puts("  fff = heading format string; each \'f\' can be one of:");
  puts("");
  puts("        . = blank");
  puts("        / = n/N arabic page numbers (1/3, 2/3, 3/3)");
  puts("        : = c/C arabic chapter page numbers (1/2, 2/2, 1/4, 2/4, ...)");
  puts("        1 = arabic numbers (1, 2, 3, ...)");
  puts("        a = lowercase letters");
  puts("        A = uppercase letters");
  puts("        c = current chapter heading");
  puts("        C = current chapter page number (arabic)");
  puts("        d = current date");
  puts("        D = current date and time");
  puts("        h = current heading");
  puts("        i = lowercase roman numerals");
  puts("        I = uppercase roman numerals");
  puts("        l = logo image");
  puts("        t = title text");
  puts("        T = current time");

  exit(1);
}


/*
 * End of "$Id: htmldoc.cxx,v 1.36.2.41 2002/05/07 20:40:56 mike Exp $".
 */
