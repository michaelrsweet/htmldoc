//
// "$Id$"
//
// Main entry for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2009 Easy Software Products.
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
//   main()            - Main entry for HTMLDOC.
//   prefs_getrc()     - Get the rc file for preferences...
//   prefs_load()      - Load HTMLDOC preferences...
//   prefs_save()      - Save HTMLDOC preferences...
//   compare_strings() - Compare two command-line strings.
//   load_book()       - Load a book file...
//   parse_options()   - Parse options from a book file...
//   read_file()       - Read a file into the current document.
//   set_permissions() - Set the PDF permission bits...
//   term_handler()    - Handle CTRL-C or kill signals...
//   usage()           - Show program version and command-line options.
//

//
// Include necessary headers.
//

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
#  include <sys/time.h>
#endif // WIN32

/*
 * Local types...
 */

typedef int (*exportfunc_t)(hdTree *, hdTree *, hdTree *);


/*
 * Local functions...
 */

static int	compare_strings(const char *s, const char *t, int tmin);
static double	get_seconds(void);
static int	load_book(const char *filename, hdTree **document,
		          exportfunc_t *exportfunc, int set_nolocal = 0);
static void	parse_options(const char *line, exportfunc_t *exportfunc);
static int	read_file(const char *filename, hdTree **document,
		          const char *path);
static void	set_permissions(const char *p);
#ifndef WIN32
extern "C" {
static void	term_handler(int signum);
}
#endif // !WIN32
static void	usage(const char *arg = NULL);


/*
 * 'main()' - Main entry for HTMLDOC.
 */

int
main(int  argc,				/* I - Number of command-line arguments */
     char *argv[])			/* I - Command-line arguments */
{
  int		i, j;			/* Looping vars */
  hdTree	*document,		/* Master HTML document */
		*file,			/* HTML document file */
		*toc,			/* Table of contents */
		*ind;			/* Index */
  exportfunc_t	exportfunc;		/* Export function */
  const char	*extension;		/* Extension of output filename */
  float		fontsize,		/* Base font size */
		fontspacing;		/* Base font spacing */
  int		num_files;		/* Number of files provided on the command-line */
  double	start_time,		/* Start time */
		load_time,		/* Load time */
		end_time;		/* End time */
  const char	*debug;			/* HTMLDOC_DEBUG environment variable */
  hdStyle	*style;			// Current style


  start_time = get_seconds();

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
  signal(SIGHUP, term_handler);
  signal(SIGINT, term_handler);
  signal(SIGTERM, term_handler);
#endif // WIN32

 /*
  * Set the location of data and help files...
  */

  prefs_set_paths();

 /*
  * Register image handlers...
  */

  hdImage::register_standard();

 /*
  * Load preferences...
  */

  prefs_load();

 /*
  * Check if we are being executed as a CGI program...
  */

  if (!getenv("HTMLDOC_NOCGI") &&
      getenv("GATEWAY_INTERFACE") && getenv("SERVER_NAME") &&
      getenv("SERVER_SOFTWARE") && getenv("PATH_TRANSLATED"))
  {
    const char	*path_translated;	// PATH_TRANSLATED env var
    char	bookfile[1024];		// Book filename

	
    // CGI mode implies the following options:
    //
    // --no-localfiles
    // --webpage
    // -t pdf
    // -f -
    //
    // Additional args cannot be provided on the command-line, however
    // we load directory-specific options from the ".book" file in the
    // current web server directory...

    CGIMode       = 1;
    TocLevels     = 0;
    TitlePage     = 0;
    OutputPath[0] = '\0';
    OutputType    = HD_OUTPUT_WEBPAGES;
    document      = NULL;
    exportfunc    = (exportfunc_t)pspdf_export;
    PSLevel       = 0;
    PDFVersion    = 13;
    PDFPageMode   = HD_PDF_DOCUMENT;
    PDFFirstPage  = HD_PDF_PAGE_1;

    hdFile::cookies(getenv("HTTP_COOKIE"));
    hdFile::referer(getenv("HTTP_REFERER"));

    progress_error(HD_ERROR_NONE, "INFO: HTMLDOC " SVERSION " starting in CGI mode.");
#ifdef WIN32
    progress_error(HD_ERROR_NONE, "INFO: TEMP is \"%s\"\n", getenv("TEMP"));
#else
    progress_error(HD_ERROR_NONE, "INFO: TMPDIR is \"%s\"\n", getenv("TMPDIR"));
#endif // WIN32

    argc = 1;

    // Look for a book file in the following order:
    //
    // $PATH_TRANSLATED.book
    // `dirname $PATH_TRANSLATED`/.book
    // .book
    //
    // If we find one, use it...
    if ((path_translated = getenv("PATH_TRANSLATED")) != NULL)
    {
      // Try $PATH_TRANSLATED.book...
      snprintf(bookfile, sizeof(bookfile), "%s.book", path_translated);
      if (access(bookfile, 0))
      {
        // Not found, try `dirname $PATH_TRANSLATED`/.book
	char path_dirname[1024];

        snprintf(bookfile, sizeof(bookfile), "%s/.book",
	         hdFile::dirname(path_translated, path_dirname,
		                 sizeof(path_dirname)));
        if (access(bookfile, 0))
	  strlcpy(bookfile, ".book", sizeof(bookfile));
      }
    }
    else
      strlcpy(bookfile, ".book", sizeof(bookfile));

    if (!access(bookfile, 0))
      load_book(bookfile, &document, &exportfunc, 1);
    else
      hdFile::no_local(true);
  }
  else
  {
   /*
    * Default to producing HTML files.
    */

    document   = NULL;
    exportfunc = (exportfunc_t)html_export;
  }

 /*
  * Parse command-line options...
  */

  fontsize    = 11.0f;
  fontspacing = 1.2f;
  num_files   = 0;
  Errors      = 0;

  for (i = 1; i < argc; i ++)
  {
#ifdef DEBUG
    printf("argv[%d] = \"%s\"\n", i, argv[i]);
#endif // DEBUG

    if (compare_strings(argv[i], "--batch", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        num_files ++;
        load_book(argv[i], &document, &exportfunc);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodycolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy((char *)BodyColor, argv[i], sizeof(BodyColor));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodyfont", 7) == 0 ||
             compare_strings(argv[i], "--textfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (!strcasecmp(argv[i], "monospace"))
	  _htmlBodyFont = HD_FONT_FACE_MONOSPACE;
        else if (!strcasecmp(argv[i], "serif"))
	  _htmlBodyFont = HD_FONT_FACE_SERIF;
        else if (!strcasecmp(argv[i], "sans-serif"))
	  _htmlBodyFont = HD_FONT_FACE_SANS_SERIF;
        else if (!strcasecmp(argv[i], "courier"))
	  _htmlBodyFont = HD_FONT_FACE_COURIER;
        else if (!strcasecmp(argv[i], "times"))
	  _htmlBodyFont = HD_FONT_FACE_TIMES;
        else if (!strcasecmp(argv[i], "helvetica") ||
	         !strcasecmp(argv[i], "arial"))
	  _htmlBodyFont = HD_FONT_FACE_HELVETICA;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodyimage", 7) == 0)
    {
      i ++;
      if (i < argc)
        BodyImage = hdImage::find(argv[i], !OutputColor, Path);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--book", 5) == 0)
      OutputType = HD_OUTPUT_BOOK;
    else if (compare_strings(argv[i], "--bottom", 5) == 0)
    {
      i ++;
      if (i < argc)
        PageBottom = get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--browserwidth", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        _htmlStyleSheet->browser_width = atof(argv[i]);

	if (_htmlStyleSheet->browser_width < 1.0f)
	{
	  progress_error(HD_ERROR_INTERNAL_ERROR, "Bad browser width \"%s\"!",
	                 argv[i]);
	  usage();
	}
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--charset", 4) == 0)
    {
      i ++;
      if (i < argc)
        _htmlStyleSheet->set_charset(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--color", 5) == 0)
    {
      OutputColor                = 1;
      _htmlStyleSheet->grayscale = false;
    }
    else if (compare_strings(argv[i], "--compression", 5) == 0 ||
             strncmp(argv[i], "--compression=", 14) == 0)
    {
      if (strlen(argv[i]) > 14 && PDFVersion >= 12)
        Compression = atoi(argv[i] + 14);
      else if (PDFVersion >= 12)
        Compression = 1;
    }
    else if (compare_strings(argv[i], "--continuous", 5) == 0)
    {
      TocLevels    = 0;
      TitlePage    = 0;
      OutputType   = HD_OUTPUT_CONTINUOUS;
      PDFPageMode  = HD_PDF_DOCUMENT;
      PDFFirstPage = HD_PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--cookies", 5) == 0)
    {
      i ++;
      if (i < argc)
        hdFile::cookies(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--datadir", 4) == 0)
    {
      i ++;
      if (i < argc)
        _htmlData = argv[i];
      else
        usage(argv[i - 1]);

      htmlDeleteStyleSheet();
      htmlInitStyleSheet();
    }
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
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--embedfonts", 4) == 0)
      EmbedFonts = 1;
    else if (compare_strings(argv[i], "--encryption", 4) == 0)
      Encryption = 1;
    else if (compare_strings(argv[i], "--firstpage", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

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
        _htmlStyleSheet->set_font_size(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--fontspacing", 8) == 0)
    {
      i ++;
      if (i < argc)
        _htmlStyleSheet->set_line_height(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--footer", 5) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], Footer);
      else
        usage(argv[i - 1]);
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
	  PDFVersion = 14;
	}
        else if (strcasecmp(argv[i], "pdf13") == 0 ||
	         strcasecmp(argv[i], "pdf") == 0)
	{
          exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 13;
	}
        else if (strcasecmp(argv[i], "pdf12") == 0)
	{
          exportfunc = (exportfunc_t)pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 12;
	}
        else if (strcasecmp(argv[i], "pdf11") == 0)
	{
          exportfunc  = (exportfunc_t)pspdf_export;
	  PSLevel     = 0;
	  PDFVersion  = 11;
	  Compression = 0;
	}
        else if (strcasecmp(argv[i], "html") == 0)
          exportfunc = (exportfunc_t)html_export;
        else if (strcasecmp(argv[i], "htmlsep") == 0)
          exportfunc = (exportfunc_t)htmlsep_export;
	else
	  usage(argv[i - 1]);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--grayscale", 3) == 0)
    {
      OutputColor                = 0;
      _htmlStyleSheet->grayscale = true;
    }
    else if (!strcmp(argv[i], "--header"))
    {
      i ++;
      if (i < argc)
        get_format(argv[i], Header);
      else
        usage(argv[i - 1]);
    }
    else if (!strcmp(argv[i], "--header1"))
    {
      i ++;
      if (i < argc)
        get_format(argv[i], Header1);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headfootfont", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
	const char	*font_family;	// Font family
	hdFontStyle	font_style;	// Font style
	hdFontWeight	font_weight;	// Font weight


	if (!strcasecmp(argv[i], "courier"))
	{
	  font_family = "courier";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "courier-bold"))
	{
	  font_family = "courier";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "courier-oblique"))
	{
	  font_family = "courier";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "courier-boldoblique"))
	{
	  font_family = "courier";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "times") ||
		 !strcasecmp(argv[i], "times-roman"))
	{
	  font_family = "times";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "times-bold"))
	{
	  font_family = "times";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "times-italic"))
	{
	  font_family = "times";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "times-bolditalic"))
	{
	  font_family = "times";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "helvetica"))
	{
	  font_family = "helvetica";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "helvetica-bold"))
	{
	  font_family = "helvetica";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "helvetica-oblique"))
	{
	  font_family = "helvetica";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "helvetica-boldoblique"))
	{
	  font_family = "helvetica";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "monospace"))
	{
	  font_family = "monospace";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "monospace-bold"))
	{
	  font_family = "monospace";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "monospace-oblique"))
	{
	  font_family = "monospace";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "monospace-boldoblique"))
	{
	  font_family = "monospace";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "serif") ||
		 !strcasecmp(argv[i], "serif-roman"))
	{
	  font_family = "serif";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "serif-bold"))
	{
	  font_family = "serif";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "serif-italic"))
	{
	  font_family = "serif";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "serif-bolditalic"))
	{
	  font_family = "serif";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "sans-serif") ||
		 !strcasecmp(argv[i], "sans"))
	{
	  font_family = "sans-serif";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "sans-serif-bold") ||
		 !strcasecmp(argv[i], "sans-bold"))
	{
	  font_family = "sans-serif";
	  font_style  = HD_FONT_STYLE_NORMAL;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}
	else if (!strcasecmp(argv[i], "sans-serif-oblique") ||
		 !strcasecmp(argv[i], "sans-oblique"))
	{
	  font_family = "sans-serif";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_NORMAL;
	}
	else if (!strcasecmp(argv[i], "sans-serif-boldoblique") ||
		 !strcasecmp(argv[i], "sans-boldoblique"))
	{
	  font_family = "sans-serif";
	  font_style  = HD_FONT_STYLE_ITALIC;
	  font_weight = HD_FONT_WEIGHT_BOLD;
	}

	style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
	style->set_string(font_family, style->font_family);
	style->font_style  = font_style;
	style->font_weight = font_weight;
	style->updated     = false;

	style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
	style->set_string(font_family, style->font_family);
	style->font_style  = font_style;
	style->font_weight = font_weight;
	style->updated     = false;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headfootsize", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        float font_size = atof(argv[i]);

	if (font_size < 6.0f)
	  font_size = 6.0f;
	else if (font_size > 24.0f)
	  font_size = 24.0f;

        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
	style->font_size = font_size;
	style->set_string(NULL, style->font_size_rel);
	style->updated = false;

        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
	style->font_size = font_size;
	style->set_string(NULL, style->font_size_rel);
	style->updated = false;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headingfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (!strcasecmp(argv[i], "monospace"))
	  _htmlHeadingFont = HD_FONT_FACE_MONOSPACE;
        else if (!strcasecmp(argv[i], "serif"))
	  _htmlHeadingFont = HD_FONT_FACE_SERIF;
        else if (!strcasecmp(argv[i], "sans-serif"))
	  _htmlHeadingFont = HD_FONT_FACE_SANS_SERIF;
        else if (!strcasecmp(argv[i], "courier"))
	  _htmlHeadingFont = HD_FONT_FACE_COURIER;
        else if (!strcasecmp(argv[i], "times"))
	  _htmlHeadingFont = HD_FONT_FACE_TIMES;
        else if (!strcasecmp(argv[i], "helvetica") ||
	         !strcasecmp(argv[i], "arial"))
	  _htmlHeadingFont = HD_FONT_FACE_HELVETICA;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--help", 6) == 0)
      usage(argv[i - 1]);
    else if (strncmp(argv[i], "--hfimage", 9) == 0)
    {
      int	hfimgnum;		// Image number
      char	*hfptr;			// Pointer into option


      if (strlen(argv[i]) > 9)
      {
        hfimgnum = strtol(argv[i] + 9, &hfptr, 10);

	if (hfimgnum < 0 || hfimgnum >= MAX_HF_IMAGES || *hfptr)
	  usage(argv[i]);
      }
      else
        hfimgnum = 0;

      i ++;

      if (i >= argc)
        usage(argv[i - 1]);

      HFImage[hfimgnum] = hdImage::find(argv[i], !OutputColor, Path);
    }
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
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--linkcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(LinkColor, argv[i], sizeof(LinkColor));
      else
        usage(argv[i - 1]);
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
	  usage(argv[i - 1]);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--logoimage", 5) == 0)
    {
      i ++;
      if (i < argc)
        LogoImage = hdImage::find(argv[i], !OutputColor, Path);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--no-compression", 6) == 0)
      Compression = 0;
    else if (compare_strings(argv[i], "--no-duplex", 4) == 0)
      PageDuplex = 0;
    else if (compare_strings(argv[i], "--no-embedfonts", 7) == 0)
      EmbedFonts = 0;
    else if (compare_strings(argv[i], "--no-encryption", 7) == 0)
      Encryption = 0;
    else if (compare_strings(argv[i], "--no-jpeg", 6) == 0)
      OutputJPEG = 0;
    else if (compare_strings(argv[i], "--no-links", 7) == 0)
      Links = 0;
    else if (compare_strings(argv[i], "--no-localfiles", 7) == 0)
      hdFile::no_local(true);
    else if (compare_strings(argv[i], "--no-numbered", 6) == 0)
      TocNumbers = 0;
    else if (compare_strings(argv[i], "--no-overflow", 6) == 0)
      OverflowErrors = 0;
    else if (compare_strings(argv[i], "--no-pscommands", 6) == 0)
      PSCommands = 0;
    else if (compare_strings(argv[i], "--no-strict", 6) == 0)
      StrictHTML = 0;
    else if (compare_strings(argv[i], "--no-title", 7) == 0)
      TitlePage = 0;
    else if (compare_strings(argv[i], "--no-toc", 7) == 0)
      TocLevels = 0;
    else if (compare_strings(argv[i], "--no-truetype", 7) == 0)
    {
      fputs("htmldoc: Warning, --no-truetype option superceded by --no-embedfonts!\n", stderr);
      EmbedFonts = 0;
    }
    else if (compare_strings(argv[i], "--no-xrxcomments", 6) == 0)
      XRXComments = 0;
    else if (compare_strings(argv[i], "--numbered", 5) == 0)
      TocNumbers = 1;
    else if (compare_strings(argv[i], "--nup", 5) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      NumberUp = atoi(argv[i]);

      if (NumberUp != 1 && NumberUp != 2 && NumberUp != 4 &&
          NumberUp != 6 && NumberUp != 9 && NumberUp != 16)
	usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--outdir", 6) == 0 ||
             strcmp(argv[i], "-d") == 0)
    {
      i ++;
      if (i < argc)
      {
        strlcpy(OutputPath, argv[i], sizeof(OutputPath));
        OutputFiles = 1;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--outfile", 6) == 0 ||
             strcmp(argv[i], "-f") == 0)
    {
      i ++;
      if (i < argc)
      {
        char	ext[256];		// Extension

        strlcpy(OutputPath, argv[i], sizeof(OutputPath));
        OutputFiles = 0;

        if ((extension = hdFile::extension(argv[i], ext, sizeof(ext))) != NULL)
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
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--overflow", 4) == 0)
      OverflowErrors = 1;
    else if (compare_strings(argv[i], "--owner-password", 4) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(OwnerPassword, argv[i], sizeof(OwnerPassword));
      else
        usage(argv[i - 1]);
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
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--pageeffect", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

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
        usage(argv[i - 1]);

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
        usage(argv[i - 1]);

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
        strlcpy(Path, argv[i], sizeof(Path));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--permissions", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      set_permissions(argv[i]);
    }
    else if (compare_strings(argv[i], "--portrait", 4) == 0)
      Landscape = 0;
    else if (compare_strings(argv[i], "--proxy", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        strlcpy(Proxy, argv[i], sizeof(Proxy));
	hdFile::proxy(Proxy);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--pscommands", 3) == 0)
      PSCommands = 1;
    else if (compare_strings(argv[i], "--quiet", 3) == 0)
      Verbosity = -1;
    else if (!compare_strings(argv[i], "--referer", 4))
    {
      i ++;
      if (i < argc)
        hdFile::referer(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--right", 4) == 0)
    {
      i ++;
      if (i < argc)
        PageRight = get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--size", 4) == 0)
    {
      i ++;
      if (i < argc)
        set_page_size(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--strict", 4) == 0)
      StrictHTML = 1;
    else if (compare_strings(argv[i], "--textcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        _htmlStyleSheet->set_color(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--title", 7) == 0)
      TitlePage = 1;
    else if (compare_strings(argv[i], "--titlefile", 8) == 0 ||
             compare_strings(argv[i], "--titleimage", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        strlcpy(TitleFile, argv[i], sizeof(TitleFile));
        TitleImage = hdImage::find(argv[i], !OutputColor, Path);
      }
      else
        usage(argv[i - 1]);

      TitlePage = 1;
    }
    else if (compare_strings(argv[i], "--tocfooter", 6) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], TocFooter);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--tocheader", 6) == 0)
    {
      i ++;
      if (i < argc)
        get_format(argv[i], TocHeader);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--toclevels", 6) == 0)
    {
      i ++;
      if (i < argc)
        TocLevels = atoi(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--toctitle", 6) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(TocTitle, argv[i], sizeof(TocTitle));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--top", 5) == 0)
    {
      i ++;
      if (i < argc)
        PageTop = get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--user-password", 4) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(UserPassword, argv[i], sizeof(UserPassword));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--truetype", 4) == 0)
    {
      fputs("htmldoc: Warning, --truetype option superceded by --embedfonts!\n", stderr);

      EmbedFonts = 1;
    }
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
    else if (compare_strings(argv[i], "--webpage", 4) == 0)
    {
      TocLevels    = 0;
      TitlePage    = 0;
      OutputType   = HD_OUTPUT_WEBPAGES;
      PDFPageMode  = HD_PDF_DOCUMENT;
      PDFFirstPage = HD_PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--words", 4) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(Words, argv[i], sizeof(Words));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--xrxcomments", 3) == 0)
      XRXComments = 1;
    else if (strcmp(argv[i], "-") == 0)
    {
     /*
      * Read from stdin...
      */

      num_files ++;

      _htmlStyleSheet->ppi = 72.0f * _htmlStyleSheet->browser_width / (PageWidth - PageLeft - PageRight);

      file = htmlAddTree(NULL, HD_ELEMENT_FILE, NULL);
      htmlSetAttr(file, "_HD_FILENAME", (hdChar *)"");
      htmlSetAttr(file, "_HD_BASE", (hdChar *)".");

#ifdef WIN32
      // Make sure stdin is in binary mode.
      // (I hate Microsoft... I hate Microsoft... Everybody join in!)
      setmode(0, O_BINARY);
#endif // WIN32

      hdStdFile *hdstdin = new hdStdFile(stdin, HD_FILE_READ);

      htmlReadFile(file, hdstdin, ".");

      delete hdstdin;

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
    else if (argv[i][0] == '-')
      usage(argv[i]);
    else
    {
      num_files ++;

      read_file(argv[i], &document, Path);
    }
  }

  if (CGIMode)
  {
    char	url[1024];		// URL
    const char	*path_info,		// Path info, if any
		*query,			// Query string, if any
		*https;			// HTTPS env var, if any


    path_info = getenv("PATH_INFO");
    query     = getenv("QUERY_STRING");
    https     = getenv("HTTPS");

    if (getenv("SERVER_PORT") && path_info && *path_info)
    {
      // Read the referenced file from the local server...
      if (https && strcmp(https, "off"))
	snprintf(url, sizeof(url), "https://%s:%s%s", getenv("SERVER_NAME"),
        	 getenv("SERVER_PORT"), getenv("PATH_INFO"));
      else
	snprintf(url, sizeof(url), "http://%s:%s%s", getenv("SERVER_NAME"),
        	 getenv("SERVER_PORT"), getenv("PATH_INFO"));

      if (query && *query && *query != '-')
      {
	// Include query string on end of URL...
        strlcat(url, "?", sizeof(url));
	strlcat(url, query, sizeof(url));
      }

      progress_error(HD_ERROR_NONE, "INFO: HTMLDOC converting \"%s\".", url);

      num_files ++;

      read_file(url, &document, Path);
    }
  }
    
 /*
  * We *must* have a document to process...
  */

  if (num_files == 0 || document == NULL)
    usage("No HTML files!");

  if (!_htmlStyleSheet || _htmlStyleSheet->num_styles == 0)
    usage("Missing standard.css!");

 /*
  * Find the first one in the list...
  */

  while (document->prev != NULL)
    document = document->prev;

  // Fix links...
  htmlFixLinks(document, document);

  load_time = get_seconds();

  // Show debug info...
  htmlDebugStats("Document Tree", document);

 /*
  * Build a table of contents for the documents if necessary...
  */

  if (Words[0])
    ind = index_build(document, Words);
  else
    ind = NULL;

  if (OutputType == HD_OUTPUT_BOOK && TocLevels > 0)
    toc = toc_build(document, ind);
  else
    toc = NULL;

  htmlDebugStats("Table of Contents Tree", toc);

  htmlDebugStyleStats();

 /*
  * Generate the output file(s).
  */

  (*exportfunc)(document, toc, ind);

  end_time = get_seconds();

 /*
  * Report running time statistics as needed...
  */

  if ((debug = getenv("HTMLDOC_DEBUG")) != NULL &&
      (strstr(debug, "all") != NULL || strstr(debug, "timing") != NULL))
    progress_error(HD_ERROR_NONE, "TIMING: %.3f %.3f %.3f",
                   load_time - start_time, end_time - load_time,
		   end_time - start_time);

 /*
  * Cleanup...
  */

  htmlDeleteTree(document);
  htmlDeleteTree(toc);

  hdFile::cleanup();
  hdImage::flush();

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
    strlcpy(home, _htmlData, sizeof(home));
  }
  else
  {
    // Grab the current user's AppData directory...
    size = sizeof(home);
    if (RegQueryValueEx(key, "AppData", NULL, NULL, (unsigned char *)home, &size))
      strlcpy(home, _htmlData, sizeof(home));

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
  int		pos;			// Header/footer position
  char		line[2048];		// Line from RC file
  hdFile	*fp;			// File pointer
  hdStyle	*style;			// Stylesheet data
  static const char * const families[] =// Font families
  {
    "courier",
    "times",
    "helvetica",
    "monospace",
    "serif",
    "sans-serif"
  };


  //
  // Initialize the stylesheet...
  //

  htmlDeleteStyleSheet();
  htmlInitStyleSheet();

  //
  // Read the preferences file...
  //

  if ((fp = hdFile::open(prefs_getrc(), HD_FILE_READ, NULL)) != NULL)
  {
    while (fp->getline(line, sizeof(line)) != NULL)
    {
      if (strncasecmp(line, "TEXTCOLOR=", 10) == 0)
	_htmlStyleSheet->set_color(line + 10);
      else if (strncasecmp(line, "BODYCOLOR=", 10) == 0)
	strlcpy(BodyColor, line + 10, sizeof(BodyColor));
      else if (strncasecmp(line, "BODYIMAGE=", 10) == 0)
        BodyImage = hdImage::find(line + 10, !OutputColor, Path);
      else if (strncasecmp(line, "LINKCOLOR=", 10) == 0)
        strlcpy(LinkColor, line + 10, sizeof(LinkColor));
      else if (strncasecmp(line, "LINKSTYLE=", 10) == 0)
	LinkStyle = atoi(line + 10);
      else if (strncasecmp(line, "BROWSERWIDTH=", 13) == 0)
	_htmlStyleSheet->browser_width = atof(line + 13);
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
	OutputColor                = atoi(line + 12);
	_htmlStyleSheet->grayscale = !OutputColor;
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
	_htmlStyleSheet->set_font_size(line + 9);
      else if (strncasecmp(line, "FONTSPACING=", 12) == 0)
	_htmlStyleSheet->set_line_height(line + 12);
      else if (strncasecmp(line, "HEADFOOTTYPE=", 13) == 0)
      {
        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
	style->set_string(families[atoi(line + 13)], style->font_family);
	style->updated = false;

        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
	style->set_string(families[atoi(line + 13)], style->font_family);
	style->updated = false;
      }
      else if (strncasecmp(line, "HEADFOOTSTYLE=", 14) == 0)
      {
        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
	style->font_style = (hdFontStyle)atoi(line + 14);
	style->updated = false;

        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
	style->font_style = (hdFontStyle)atoi(line + 14);
	style->updated = false;
      }
      else if (strncasecmp(line, "HEADFOOTSIZE=", 13) == 0)
      {
        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
	style->font_size = atof(line + 13);
	style->set_string(NULL, style->font_size_rel);
	style->updated = false;

        style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
	style->font_size = atof(line + 13);
	style->set_string(NULL, style->font_size_rel);
	style->updated = false;
      }
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
	_htmlStyleSheet->set_charset(line + 8);
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
	strlcpy(OwnerPassword, line + 14, sizeof(OwnerPassword));
      else if (strncasecmp(line, "USERPASSWORD=", 13) == 0)
        strlcpy(UserPassword, line + 13, sizeof(UserPassword));
      else if (strncasecmp(line, "LINKS=", 6) == 0)
        Links = atoi(line + 6);
      else if (strncasecmp(line, "TRUETYPE=", 9) == 0)
        EmbedFonts = atoi(line + 9);
      else if (strncasecmp(line, "EMBEDFONTS=", 11) == 0)
        EmbedFonts = atoi(line + 11);
      else if (strncasecmp(line, "PATH=", 5) == 0)
	strlcpy(Path, line + 5, sizeof(Path));
      else if (strncasecmp(line, "PROXY=", 6) == 0)
	strlcpy(Proxy, line + 6, sizeof(Proxy));
      else if (strncasecmp(line, "STRICTHTML=", 11) == 0)
        StrictHTML = atoi(line + 11);
    }

    delete fp;
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
  hdFile	*fp;			// File pointer


  if ((fp = hdFile::open(prefs_getrc(), HD_FILE_WRITE, NULL)) != NULL)
  {
    fp->puts("#HTMLDOCRC " SVERSION "\n");

    fp->printf("TEXTCOLOR=#%02X%02X%02X\n",
	       _htmlStyleSheet->def_style.color[0],
	       _htmlStyleSheet->def_style.color[1],
	       _htmlStyleSheet->def_style.color[2]);
    fp->printf("BODYCOLOR=%s\n", BodyColor);
    fp->printf("BODYIMAGE=%s\n", BodyImage);
    fp->printf("LINKCOLOR=%s\n", LinkColor);
    fp->printf("LINKSTYLE=%d\n", LinkStyle);
    fp->printf("BROWSERWIDTH=%.0f\n", _htmlStyleSheet->browser_width);
    fp->printf("PAGEWIDTH=%d\n", PageWidth);
    fp->printf("PAGELENGTH=%d\n", PageLength);
    fp->printf("PAGELEFT=%d\n", PageLeft);
    fp->printf("PAGERIGHT=%d\n", PageRight);
    fp->printf("PAGETOP=%d\n", PageTop);
    fp->printf("PAGEBOTTOM=%d\n", PageBottom);
    fp->printf("PAGEDUPLEX=%d\n", PageDuplex);
    fp->printf("LANDSCAPE=%d\n", Landscape);
    fp->printf("COMPRESSION=%d\n", Compression);
    fp->printf("OUTPUTCOLOR=%d\n", OutputColor);
    fp->printf("TOCNUMBERS=%d\n", TocNumbers);
    fp->printf("TOCLEVELS=%d\n", TocLevels);
    fp->printf("JPEG=%d\n", OutputJPEG);
    fp->printf("PAGEHEADER=%s\n", get_fmt(Header));
    fp->printf("PAGEFOOTER=%s\n", get_fmt(Footer));
    fp->printf("NUMBERUP=%d\n", NumberUp);
    fp->printf("TOCHEADER=%s\n", get_fmt(TocHeader));
    fp->printf("TOCFOOTER=%s\n", get_fmt(TocFooter));
    fp->printf("TOCTITLE=%s\n", TocTitle);
    fp->printf("BODYFONT=%d\n", _htmlBodyFont);
    fp->printf("HEADINGFONT=%d\n", _htmlHeadingFont);
    fp->printf("FONTSIZE=%.2f\n", _htmlStyleSheet->def_style.font_size);
    fp->printf("FONTSPACING=%.2f\n", _htmlStyleSheet->def_style.line_height /
                                     _htmlStyleSheet->def_style.font_size);
//    fp->printf("HEADFOOTTYPE=%d\n", HeadFootType);
//    fp->printf("HEADFOOTSTYLE=%d\n", HeadFootStyle);
//    fp->printf("HEADFOOTSIZE=%.2f\n", HeadFootSize);
    fp->printf("PDFVERSION=%d\n", PDFVersion);
    fp->printf("PSLEVEL=%d\n", PSLevel);
    fp->printf("PSCOMMANDS=%d\n", PSCommands);
    fp->printf("XRXCOMMENTS=%d\n", XRXComments);
    fp->printf("CHARSET=%s\n", _htmlStyleSheet->charset);
    fp->printf("PAGEMODE=%d\n", PDFPageMode);
    fp->printf("PAGELAYOUT=%d\n", PDFPageLayout);
    fp->printf("FIRSTPAGE=%d\n", PDFFirstPage);
    fp->printf("PAGEEFFECT=%d\n", PDFEffect);
    fp->printf("PAGEDURATION=%.0f\n", PDFPageDuration);
    fp->printf("EFFECTDURATION=%.1f\n", PDFEffectDuration);
    fp->printf("ENCRYPTION=%d\n", Encryption);
    fp->printf("PERMISSIONS=%d\n", Permissions);
    fp->printf("OWNERPASSWORD=%s\n", OwnerPassword);
    fp->printf("USERPASSWORD=%s\n", UserPassword);
    fp->printf("LINKS=%d\n", Links);
    fp->printf("EMBEDFONTS=%d\n", EmbedFonts);
    fp->printf("PATH=%s\n", Path);
    fp->printf("PROXY=%s\n", Proxy);
    fp->printf("STRICTHTML=%d\n", StrictHTML);

    delete fp;
  }
}


/*
 * 'prefs_set_paths()' - Set HTMLDOC data path...
 */

void
prefs_set_paths(void)
{
#ifdef WIN32			//// Do registry magic...
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string
  static char	data[1024];	// Data directory
  static char	doc[1024];	// Documentation directory
  static char	path[4096];	// PATH environment variable
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
    else
      progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to read \"data\" value from registry!");

    RegCloseKey(key);
  }
  else
    progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to read HTMLDOC installation from registry!");

  // See if the HTMLDOC program folder is in the system execution path...
  if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                    0, KEY_READ | KEY_WRITE, &key))
  {
    // Grab the current path...
    size = sizeof(path);
    if (!RegQueryValueEx(key, "Path", NULL, NULL, (unsigned char *)path, &size))
      if (strstr(path, _htmlData) == NULL)
      {
        // The data directory is not in the path, so add it...
        strlcat(path, ";", sizeof(path));
        strlcat(path, _htmlData, sizeof(path));
        RegSetValueEx(key, "Path", 0, REG_EXPAND_SZ, (unsigned char *)path, strlen(path) + 1);
      }
  }
#endif // WIN32

  //
  // See if the installed directories have been overridden by
  // environment variables...
  //

  if (getenv("HTMLDOC_DATA") != NULL)
    _htmlData = getenv("HTMLDOC_DATA");
}


/*
 * 'compare_strings()' - Compare two command-line strings.
 */

static int				/* O - -1 or 1 = no match, 0 = match */
compare_strings(const char *s,		/* I - Command-line string */
                const char *t,		/* I - Option string */
                int        tmin)	/* I - Minimum number of unique chars in option */
{
  int	slen;				/* Length of command-line string */


  slen = strlen(s);
  if (slen < tmin)
    return (-1);
  else
    return (strncmp(s, t, slen));
}


/*
 * 'get_seconds()' - Get the current fractional time in seconds.
 */

static double				/* O - Number of seconds */
get_seconds(void)
{
#ifdef WIN32
  return (GetTickCount() * 0.001);
#else
  struct timeval	curtime;	/* Current time */

  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + curtime.tv_usec * 0.000001);
#endif /* WIN32 */
}


//
// 'load_book()' - Load a book file...
//

static int				// O  - 1 = success, 0 = failure
load_book(const char   *filename,	// I  - Book file
          hdTree       **document,	// IO - Document tree
          exportfunc_t *exportfunc,	// O  - Export function
          int          set_nolocal)	// I  - Set no_local(true) after lookup?
{
  hdFile	*fp;			// File to read from
  char		line[10240];		// Line from file
  char		dir[1024];		// Directory path
  char		newpath[2048];		// Updated/new path
  const char	*path;			// Path to use


  // Open the file...
  if ((fp = hdFile::open(filename, HD_FILE_READ, Path)) == NULL)
  {
    progress_error(HD_ERROR_READ_ERROR, "Unable to open book file \"%s\": %s",
                   filename, strerror(errno));
    return (0);
  }

  if (set_nolocal)
    hdFile::no_local(true);

  // See if the filename contains a path...
  if (hdFile::dirname(filename, dir, sizeof(dir)))
  {
    snprintf(newpath, sizeof(newpath), "%s;%s", dir, Path);
    path = newpath;
  }
  else
  {
    dir[0] = '\0';
    path   = Path;
  }

  // Get the header...
  if (!fp->getline(line, sizeof(line)) || strncmp(line, "#HTMLDOC", 8))
  {
    delete fp;
    progress_error(HD_ERROR_BAD_FORMAT,
                   "Bad or missing #HTMLDOC header in \"%s\".", filename);
    return (0);
  }

  // Read the second line from the book file; for older book files, this will
  // be the file count; for new files this will be the options...
  do
  {
    if (!fp->getline(line, sizeof(line)))
      break;

    if (line[0] == '-')
    {
      parse_options(line, exportfunc);

      if (dir[0])
	snprintf(newpath, sizeof(newpath), "%s;%s", dir, Path);
    }
  }
  while (!line[0]);			// Skip blank lines

  // Get input files/options...
  while (fp->getline(line, sizeof(line)))
  {
    if (!line[0])
      continue;				// Skip blank lines
    else if (line[0] == '-')
    {
      parse_options(line, exportfunc);

      if (dir[0])
	snprintf(newpath, sizeof(newpath), "%s;%s", dir, Path);
    }
    else if (line[0] == '\\')
      read_file(line + 1, document, path);
    else
      read_file(line, document, path);
  }

  // Close the book file and return...
  delete fp;

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
  hdStyle	*style;			// Current style


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
    else if (strcmp(temp, "--embedfonts") == 0 ||
             strcmp(temp, "--truetype") == 0)
    {
      EmbedFonts = 1;
      continue;
    }
    else if (strcmp(temp, "--no-embedfonts") == 0 ||
             strcmp(temp, "--no-truetype") == 0)
    {
      EmbedFonts = 0;
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
    else if (strcmp(temp, "--title") == 0)
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
      OutputType = HD_OUTPUT_BOOK;
      continue;
    }
    else if (strcmp(temp, "--continuous") == 0)
    {
      OutputType = HD_OUTPUT_CONTINUOUS;
      continue;
    }
    else if (strcmp(temp, "--webpage") == 0)
    {
      OutputType = HD_OUTPUT_WEBPAGES;
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
    else if (strcmp(temp, "--strict") == 0)
      StrictHTML = 1;
    else if (strcmp(temp, "--no-strict") == 0)
      StrictHTML = 0;
    else if (strcmp(temp, "--overflow") == 0)
      OverflowErrors = 1;
    else if (strcmp(temp, "--no-overflow") == 0)
      OverflowErrors = 0;

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

    if (strcmp(temp, "-t") == 0 && !CGIMode)
    {
      if (strcmp(temp2, "html") == 0)
        *exportfunc = (exportfunc_t)html_export;
      else if (strcmp(temp2, "htmlsep") == 0)
        *exportfunc = (exportfunc_t)htmlsep_export;
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
	PDFVersion  = 11;
      }
      else if (strcmp(temp2, "pdf12") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 12;
      }
      else if (strcmp(temp2, "pdf") == 0 ||
               strcmp(temp2, "pdf13") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 13;
      }
      else if (strcmp(temp2, "pdf14") == 0)
      {
        *exportfunc = (exportfunc_t)pspdf_export;
	PSLevel     = 0;
	PDFVersion  = 14;
      }
    }
    else if (strcmp(temp, "--logo") == 0 ||
             strcmp(temp, "--logoimage") == 0)
      LogoImage = hdImage::find(temp2, !OutputColor, Path);
    else if (strcmp(temp, "--titlefile") == 0 ||
             strcmp(temp, "--titleimage") == 0)
    {
      TitlePage = 1;
      strlcpy(TitleFile, temp2, sizeof(TitleFile));
      TitleImage = hdImage::find(temp2, !OutputColor, Path);
    }
    else if (strcmp(temp, "-f") == 0 && !CGIMode)
    {
      OutputFiles = 0;
      strlcpy(OutputPath, temp2, sizeof(OutputPath));
    }
    else if (strcmp(temp, "-d") == 0 && !CGIMode)
    {
      OutputFiles = 1;
      strlcpy(OutputPath, temp2, sizeof(OutputPath));
    }
    else if (strcmp(temp, "--browserwidth") == 0)
      _htmlStyleSheet->browser_width = atof(temp2);
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
    else if (strcmp(temp, "--header1") == 0)
      get_format(temp2, Header1);
    else if (strcmp(temp, "--footer") == 0)
      get_format(temp2, Footer);
    else if (strcmp(temp, "--bodycolor") == 0)
      strlcpy(BodyColor, temp2, sizeof(BodyColor));
    else if (strcmp(temp, "--bodyimage") == 0)
      BodyImage = hdImage::find(temp2, !OutputColor, Path);
    else if (strcmp(temp, "--textcolor") == 0)
      _htmlStyleSheet->set_color(temp2);
    else if (strcmp(temp, "--linkcolor") == 0)
      strlcpy(LinkColor, temp2, sizeof(LinkColor));
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
      strlcpy(TocTitle, temp2, sizeof(TocTitle));
    else if (strcmp(temp, "--fontsize") == 0)
      _htmlStyleSheet->set_font_size(temp2);
    else if (strcmp(temp, "--fontspacing") == 0)
      _htmlStyleSheet->set_line_height(temp2);
    else if (strcmp(temp, "--headingfont") == 0)
    {
      if (!strcasecmp(temp2, "monospace"))
        _htmlHeadingFont = HD_FONT_FACE_MONOSPACE;
      else if (!strcasecmp(temp2, "serif"))
        _htmlHeadingFont = HD_FONT_FACE_SERIF;
      else if (!strcasecmp(temp2, "sans"))
        _htmlHeadingFont = HD_FONT_FACE_SANS_SERIF;
      else if (!strcasecmp(temp2, "courier"))
        _htmlHeadingFont = HD_FONT_FACE_COURIER;
      else if (!strcasecmp(temp2, "times"))
        _htmlHeadingFont = HD_FONT_FACE_TIMES;
      else if (!strcasecmp(temp2, "helvetica") ||
               !strcasecmp(temp2, "arial"))
        _htmlHeadingFont = HD_FONT_FACE_HELVETICA;
    }
    else if (strcmp(temp, "--bodyfont") == 0)
    {
      if (!strcasecmp(temp2, "monospace"))
        _htmlBodyFont = HD_FONT_FACE_MONOSPACE;
      else if (!strcasecmp(temp2, "serif"))
        _htmlBodyFont = HD_FONT_FACE_SERIF;
      else if (!strcasecmp(temp2, "sans"))
        _htmlBodyFont = HD_FONT_FACE_SANS_SERIF;
      else if (!strcasecmp(temp2, "courier"))
        _htmlBodyFont = HD_FONT_FACE_COURIER;
      else if (!strcasecmp(temp2, "times"))
        _htmlBodyFont = HD_FONT_FACE_TIMES;
      else if (!strcasecmp(temp2, "helvetica") ||
               !strcasecmp(temp2, "arial"))
        _htmlBodyFont = HD_FONT_FACE_HELVETICA;
    }
    else if (strcmp(temp, "--headfootsize") == 0)
    {
      float font_size = atof(temp2);

      if (font_size < 6.0f)
	font_size = 6.0f;
      else if (font_size > 24.0f)
	font_size = 24.0f;

      style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
      style->font_size = font_size;
      style->set_string(NULL, style->font_size_rel);
      style->updated = false;

      style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
      style->font_size = font_size;
      style->set_string(NULL, style->font_size_rel);
      style->updated = false;
    }
    else if (strcmp(temp, "--headfootfont") == 0)
    {
      const char	*font_family;	// Font family
      hdFontStyle	font_style;	// Font style
      hdFontWeight	font_weight;	// Font weight


      if (!strcasecmp(temp2, "courier"))
      {
	font_family = "courier";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "courier-bold"))
      {
	font_family = "courier";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "courier-oblique"))
      {
	font_family = "courier";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "courier-boldoblique"))
      {
	font_family = "courier";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "times") ||
	       !strcasecmp(temp2, "times-roman"))
      {
	font_family = "times";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "times-bold"))
      {
	font_family = "times";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "times-italic"))
      {
	font_family = "times";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "times-bolditalic"))
      {
	font_family = "times";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "helvetica"))
      {
	font_family = "helvetica";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "helvetica-bold"))
      {
	font_family = "helvetica";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "helvetica-oblique"))
      {
	font_family = "helvetica";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "helvetica-boldoblique"))
      {
	font_family = "helvetica";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "monospace"))
      {
	font_family = "monospace";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "monospace-bold"))
      {
	font_family = "monospace";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "monospace-oblique"))
      {
	font_family = "monospace";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "monospace-boldoblique"))
      {
	font_family = "monospace";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "serif") ||
	       !strcasecmp(temp2, "serif-roman"))
      {
	font_family = "serif";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "serif-bold"))
      {
	font_family = "serif";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "serif-italic"))
      {
	font_family = "serif";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "serif-bolditalic"))
      {
	font_family = "serif";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "sans-serif") ||
	       !strcasecmp(temp2, "sans"))
      {
	font_family = "sans-serif";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "sans-serif-bold") ||
	       !strcasecmp(temp2, "sans-bold"))
      {
	font_family = "sans-serif";
	font_style  = HD_FONT_STYLE_NORMAL;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }
      else if (!strcasecmp(temp2, "sans-serif-oblique") ||
	       !strcasecmp(temp2, "sans-oblique"))
      {
	font_family = "sans-serif";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_NORMAL;
      }
      else if (!strcasecmp(temp2, "sans-serif-boldoblique") ||
	       !strcasecmp(temp2, "sans-boldoblique"))
      {
	font_family = "sans-serif";
	font_style  = HD_FONT_STYLE_ITALIC;
	font_weight = HD_FONT_WEIGHT_BOLD;
      }

      style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
      style->set_string(font_family, style->font_family);
      style->font_style  = font_style;
      style->font_weight = font_weight;
      style->updated     = false;

      style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
      style->set_string(font_family, style->font_family);
      style->font_style  = font_style;
      style->font_weight = font_weight;
      style->updated     = false;
    }
    else if (strcmp(temp, "--charset") == 0)
      _htmlStyleSheet->set_charset(temp2);
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
      set_permissions(temp2);
    else if (strcmp(temp, "--user-password") == 0)
      strlcpy(UserPassword, temp2, sizeof(UserPassword));
    else if (strcmp(temp, "--owner-password") == 0)
      strlcpy(OwnerPassword, temp2, sizeof(OwnerPassword));
    else if (strcmp(temp, "--path") == 0)
      strlcpy(Path, temp2, sizeof(Path) - 1);
    else if (strcmp(temp, "--words") == 0)
      strlcpy(Words, temp2, sizeof(Words));
    else if (strcmp(temp, "--proxy") == 0)
    {
      strlcpy(Proxy, temp2, sizeof(Proxy));
      hdFile::proxy(Proxy);
    }
    else if (strcmp(temp, "--cookies") == 0)
      hdFile::cookies(temp2);
  }
}


//
// 'read_file()' - Read a file into the current document.
//

static int				// O  - 1 on success, 0 on failure
read_file(const char *filename,		// I  - File/URL to read
          hdTree     **document,	// IO - Current document
	  const char *path)		// I  - Search path
{
  hdFile	*docfile;		// Document file
  hdTree	*file;			// HTML document file
  char		base[1024];		// Base directory name of file


  DEBUG_printf(("read_file(filename=\"%s\", document=%p, path=\"%s\")\n",
                filename, document, path));

  if ((docfile = hdFile::open(filename, HD_FILE_READ, path)) != NULL)
  {
   /*
    * Read from a file...
    */

    if (Verbosity > 0)
      progress_error(HD_ERROR_NONE, "INFO: Reading %s...", filename);

    _htmlStyleSheet->ppi = 72.0f * _htmlStyleSheet->browser_width /
			   (PageWidth - PageLeft - PageRight);

    file = htmlAddTree(NULL, HD_ELEMENT_FILE, NULL);
    htmlSetAttr(file, "_HD_FILENAME",
                (hdChar *)docfile->basename(base, sizeof(base)));
    htmlSetAttr(file, "_HD_BASE",
                (hdChar *)docfile->dirname(base, sizeof(base)));

    htmlReadFile(file, docfile, base);

    delete docfile;

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
  {
    file = NULL;
    progress_error(HD_ERROR_FILE_NOT_FOUND,
		   "Unable to open \"%s\" for reading...", filename);
  }

  return (file != NULL);
}


//
// 'set_permissions()' - Set the PDF permission bits.
//

void
set_permissions(const char *p)		// I - Permission string
{
  char	*copyp,				// Copy of string
	*start,				// Start of current keyword
	*ptr;				// Pointer into string


  // Range check input...
  if (!p || !*p)
    return;

  // Make a copy of the string and parse it...
  copyp = strdup(p);
  if (!copyp)
    return;

  for (start = copyp; *start; start = ptr)
  {
    for (ptr = start; *ptr; ptr ++)
      if (*ptr == ',')
      {
	*ptr++ = '\0';
	break;
      }

    if (!strcasecmp(start, "all"))
      Permissions = -4;
    else if (!strcasecmp(start, "none"))
      Permissions = -64;
    else if (!strcasecmp(start, "print"))
      Permissions |= HD_PDF_PERM_PRINT;
    else if (!strcasecmp(start, "no-print"))
      Permissions &= ~HD_PDF_PERM_PRINT;
    else if (!strcasecmp(start, "modify"))
      Permissions |= HD_PDF_PERM_MODIFY;
    else if (!strcasecmp(start, "no-modify"))
      Permissions &= ~HD_PDF_PERM_MODIFY;
    else if (!strcasecmp(start, "copy"))
      Permissions |= HD_PDF_PERM_COPY;
    else if (!strcasecmp(start, "no-copy"))
      Permissions &= ~HD_PDF_PERM_COPY;
    else if (!strcasecmp(start, "annotate"))
      Permissions |= HD_PDF_PERM_ANNOTATE;
    else if (!strcasecmp(start, "no-annotate"))
      Permissions &= ~HD_PDF_PERM_ANNOTATE;
  }

  if (Permissions != -4)
    Encryption = 1;
}


#ifndef WIN32
//
// 'term_handler()' - Handle CTRL-C or kill signals...
//

static void
term_handler(int signum)	// I - Signal number
{
  REF(signum);

  hdFile::cleanup();
  hdImage::flush();
  exit(1);
}
#endif // !WIN32


/*
 * 'usage()' - Show program version and command-line options.
 */

static void
usage(const char *arg)			// I - Bad argument string
{
  if (CGIMode)
    puts("Content-Type: text/plain\r\n\r");

  puts("HTMLDOC Version " SVERSION " Copyright 1997-2005 Easy Software Products, All Rights Reserved.");
  puts("This software is governed by the GNU General Public License, Version 2, and");
  puts("is based in part on the work of the Independent JPEG Group.");
  puts("");

  if (!CGIMode)
  {
    if (arg && arg[0] == '-')
      printf("ERROR: Bad option argument \"%s\"!\n\n", arg);
    else
      printf("ERROR: %s\n", arg);

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
    puts("  --bodyfont {courier,helvetica,monospace,sans,serif,times}");
    puts("  --bodyimage filename.{bmp,gif,jpg,png}");
    puts("  --book");
    puts("  --bottom margin{in,cm,mm}");
    puts("  --browserwidth pixels");
    puts("  --charset {cp-874...1258,iso-8859-1...8859-15,koi8-r}");
    puts("  --color");
    puts("  --compression[=level]");
    puts("  --continuous");
    puts("  --cookies 'name=\"value with space\"; name=value'");
    puts("  --datadir directory");
    puts("  --duplex");
    puts("  --effectduration {0.1..10.0}");
    puts("  --embedfonts");
    puts("  --encryption");
    puts("  --firstpage {p1,toc,c1}");
    puts("  --fontsize {4.0..24.0}");
    puts("  --fontspacing {1.0..3.0}");
    puts("  --footer fff");
    puts("  {--format, -t} {ps1,ps2,ps3,pdf11,pdf12,pdf13,pdf14,html,htmlsep}");
    puts("  --gray");
    puts("  --header fff");
    puts("  --header1 fff");
    puts("  --headfootfont {courier{-bold,-oblique,-boldoblique},\n"
	 "                  helvetica{-bold,-oblique,-boldoblique},\n"
	 "                  monospace{-bold,-oblique,-boldoblique},\n"
	 "                  sans{-bold,-oblique,-boldoblique},\n"
	 "                  serif{-bold,-italic,-bolditalic},\n"
	 "                  times{-roman,-bold,-italic,-bolditalic}}\n");
    puts("  --headfootsize {6.0..24.0}");
    puts("  --headingfont {courier,helvetica,monospace,sans,serif,times}");
    puts("  --help");
    for (int i = 0; i < MAX_HF_IMAGES; i ++)
      printf("  --hfimage%d filename.{bmp,gif,jpg,png}\n", i);
    puts("  --jpeg[=quality]");
    puts("  --landscape");
    puts("  --left margin{in,cm,mm}");
    puts("  --linkcolor color");
    puts("  --links");
    puts("  --linkstyle {plain,underline}");
    puts("  --logoimage filename.{bmp,gif,jpg,png}");
    puts("  --no-compression");
    puts("  --no-duplex");
    puts("  --no-embedfonts");
    puts("  --no-encryption");
    puts("  --no-links");
    puts("  --no-localfiles");
    puts("  --no-numbered");
    puts("  --no-overflow");
    puts("  --no-pscommands");
    puts("  --no-strict");
    puts("  --no-title");
    puts("  --no-toc");
    puts("  --numbered");
    puts("  --nup {1,2,4,6,9,16}");
    puts("  {--outdir, -d} dirname");
    puts("  {--outfile, -f} filename.{ps,pdf,html}");
    puts("  --overflow");
    puts("  --owner-password password");
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
    puts("  --referer url");
    puts("  --right margin{in,cm,mm}");
    puts("  --size {letter,a4,WxH{in,cm,mm},etc}");
    puts("  --strict");
    puts("  --textcolor color");
    puts("  --textfont {courier,times,helvetica}");
    puts("  --title");
    puts("  --titlefile filename.{htm,html,shtml}");
    puts("  --titleimage filename.{bmp,gif,jpg,png}");
    puts("  --tocfooter fff");
    puts("  --tocheader fff");
    puts("  --toclevels levels");
    puts("  --toctitle string");
    puts("  --top margin{in,cm,mm}");
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
  }

  exit(1);
}


//
// End of "$Id$".
//
