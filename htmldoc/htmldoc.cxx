/*
 * "$Id: htmldoc.cxx,v 1.44 2004/04/05 01:39:34 mike Exp $"
 *
 *   Main entry for HTMLDOC, a HTML document processing program.
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   main()            - Main entry for HTMLDOC.
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
 * Local functions...
 */

static int	compare_strings(const char *s, const char *t, int tmin);
static int	load_book(hdBook *book, const char *filename, hdTree **document);
static void	parse_options(hdBook *book, const char *line);
static int	read_file(hdBook *book, const char *filename,
		          hdTree **document, const char *path);
static void	term_handler(int signum);
static void	usage(const char *arg = NULL);


/*
 * 'main()' - Main entry for HTMLDOC.
 */

int					// O - Exit status
main(int  argc,				// I - Number of command-line arguments
     char *argv[])			// I - Command-line arguments
{
  int		i, j;			// Looping vars
  hdBook	*book;			// Current book
  hdTree	*document,		// Master HTML document
		*file,			// HTML document file
		*toc;			// Table of contents
  const char	*extension;		// Extension of output filename
  float		fontsize,		// Base font size
		fontspacing;		// Base font spacing
  int		num_files;		// Number of files provided


#ifdef __APPLE__
  // OSX passes an extra command-line option when run from the Finder.
  // If the first command-line argument is "-psn..." then skip it...
  if (argc > 1 && strncmp(argv[1], "-psn", 4) == 0)
  {
    argv ++;
    argc --;
  }
#endif // __APPLE__

  // Localize as needed...
#ifdef HAVE_LOCALE_H
  setlocale(LC_TIME, "");
#endif // HAVE_LOCALE_H

  // Catch CTRL-C and term signals...
#ifdef WIN32
#else
  signal(SIGTERM, term_handler);
#endif // WIN32

  // Create book and load preferences...
  book = new hdBook();

  // Parse command-line options...
  document    = NULL;
  fontsize    = 11.0f;
  fontspacing = 1.2f;
  num_files   = 0;

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
        load_book(book, argv[i], &document);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodycolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy((char *)book->BodyColor, argv[i], sizeof(book->BodyColor));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodyfont", 7) == 0 ||
             compare_strings(argv[i], "--textfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0 ||
	    strcasecmp(argv[i], "monospace") == 0)
	  _htmlBodyFont = HD_FONTFACE_MONOSPACE;
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "serif") == 0)
	  _htmlBodyFont = HD_FONTFACE_SERIF;
        else if (strcasecmp(argv[i], "helvetica") == 0 ||
	         strcasecmp(argv[i], "arial") == 0 ||
		 strcasecmp(argv[i], "sans-serif") == 0)
	  _htmlBodyFont = HD_FONTFACE_SANS_SERIF;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--bodyimage", 7) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy((char *)book->BodyImage, argv[i], sizeof(book->BodyImage));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--book", 5) == 0)
      book->OutputType = HD_OUTPUT_BOOK;
    else if (compare_strings(argv[i], "--bottom", 5) == 0)
    {
      i ++;
      if (i < argc)
        book->PageBottom = book->get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--browserwidth", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        _htmlBrowserWidth = atof(argv[i]);

	if (_htmlBrowserWidth < 1.0f)
	{
	  book->progress_error(HD_ERROR_INTERNAL_ERROR, "Bad browser width \"%s\"!",
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
        htmlSetCharSet(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--color", 5) == 0)
    {
      book->OutputColor = 1;
      _htmlGrayscale    = 0;
    }
    else if (compare_strings(argv[i], "--compression", 5) == 0 ||
             strncmp(argv[i], "--compression=", 14) == 0)
    {
      if (strlen(argv[i]) > 14 && book->PDFVersion >= 12)
        book->Compression = atoi(argv[i] + 14);
      else if (book->PDFVersion >= 12)
        book->Compression = 1;
    }
    else if (compare_strings(argv[i], "--continuous", 5) == 0)
    {
      book->TocLevels    = 0;
      book->TitlePage    = 0;
      book->OutputType   = HD_OUTPUT_CONTINUOUS;
      book->PDFPageMode  = HD_PDF_DOCUMENT;
      book->PDFFirstPage = HD_PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--datadir", 4) == 0)
    {
      i ++;
      if (i < argc)
        hdBook::datadir = argv[i];
      else
        usage(argv[i - 1]);
    }
#if defined(HAVE_LIBFLTK) && !WIN32
    else if (compare_strings(argv[i], "-display", 3) == 0 ||
             compare_strings(argv[i], "--display", 4) == 0)
    {
      // The X standard requires support for the -display option, but
      // we also support the GNU standard --display...
      i ++;
      if (i < argc)
        Fl::display(argv[i]);
      else
        usage(argv[i - 1]);
    }
#endif // HAVE_LIBFLTK && !WIN32
    else if (compare_strings(argv[i], "--duplex", 4) == 0)
      book->PageDuplex = 1;
    else if (compare_strings(argv[i], "--effectduration", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        book->PDFEffectDuration = atof(argv[i]);

	if (book->PDFEffectDuration < 0.0f)
	{
	  book->progress_error(HD_ERROR_INTERNAL_ERROR, "Bad effect duration \"%s\"!",
	                       argv[i]);
	  usage();
	}
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--embedfonts", 4) == 0)
      book->EmbedFonts = true;
    else if (compare_strings(argv[i], "--encryption", 4) == 0)
      book->Encryption = true;
    else if (compare_strings(argv[i], "--firstpage", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      for (j = 0; j < (int)(sizeof(hdBook::PDFPages) /
                            sizeof(hdBook::PDFPages[0])); j ++)
        if (strcasecmp(argv[i], hdBook::PDFPages[j]) == 0)
	{
	  book->PDFFirstPage = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--fontsize", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        fontsize = atof(argv[i]);

	if (fontsize < 4.0f)
	  fontsize = 4.0f;
	else if (fontsize > 24.0f)
	  fontsize = 24.0f;

        htmlSetBaseSize(fontsize, fontspacing);
      }
      else
        usage(argv[i - 1]);
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
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--footer", 5) == 0)
    {
      i ++;
      if (i < argc)
        book->get_format(argv[i], book->Footer);
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
	  book->OutputFormat = HD_OUTPUT_PS;
	  book->PSLevel      = 1;
	}
        else if (strcasecmp(argv[i], "ps2") == 0 ||
                 strcasecmp(argv[i], "ps") == 0)
        {
	  book->OutputFormat = HD_OUTPUT_PS;
	  book->PSLevel      = 2;
	}
        else if (strcasecmp(argv[i], "ps3") == 0)
        {
	  book->OutputFormat = HD_OUTPUT_PS;
	  book->PSLevel      = 3;
	}
        else if (strcasecmp(argv[i], "pdf14") == 0)
	{
	  book->OutputFormat = HD_OUTPUT_PDF;
	  book->PSLevel      = 0;
	  book->PDFVersion   = 14;
	}
        else if (strcasecmp(argv[i], "pdf13") == 0 ||
	         strcasecmp(argv[i], "pdf") == 0)
	{
	  book->OutputFormat = HD_OUTPUT_PDF;
	  book->PSLevel      = 0;
	  book->PDFVersion   = 13;
	}
        else if (strcasecmp(argv[i], "pdf12") == 0)
	{
	  book->OutputFormat = HD_OUTPUT_PDF;
	  book->PSLevel      = 0;
	  book->PDFVersion   = 12;
	}
        else if (strcasecmp(argv[i], "pdf11") == 0)
	{
	  book->OutputFormat = HD_OUTPUT_PDF;
	  book->PSLevel      = 0;
	  book->PDFVersion   = 11;
	  book->Compression  = 0;
	}
        else if (strcasecmp(argv[i], "html") == 0)
	  book->OutputFormat = HD_OUTPUT_HTML;
        else if (strcasecmp(argv[i], "htmlsep") == 0)
	  book->OutputFormat = HD_OUTPUT_HTMLSEP;
	else
	  usage(argv[i - 1]);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--grayscale", 3) == 0)
    {
      book->OutputColor = false;
      _htmlGrayscale    = 1;
    }
    else if (compare_strings(argv[i], "--header", 7) == 0)
    {
      i ++;
      if (i < argc)
        book->get_format(argv[i], book->Header);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headfootfont", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	  book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
	}
        else if (strcasecmp(argv[i], "courier-bold") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
	}
        else if (strcasecmp(argv[i], "courier-oblique") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	  book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
	}
        else if (strcasecmp(argv[i], "courier-boldoblique") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
	}
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "times-roman") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
	}
        else if (strcasecmp(argv[i], "times-bold") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
	}
        else if (strcasecmp(argv[i], "times-italic") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
	}
        else if (strcasecmp(argv[i], "times-bolditalic") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
	}
        else if (strcasecmp(argv[i], "helvetica") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
	}
        else if (strcasecmp(argv[i], "helvetica-bold") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
	}
        else if (strcasecmp(argv[i], "helvetica-oblique") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
	}
        else if (strcasecmp(argv[i], "helvetica-boldoblique") == 0)
	{
	  book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	  book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
	}
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headfootsize", 11) == 0)
    {
      i ++;
      if (i < argc)
      {
        book->HeadFootSize = atof(argv[i]);

	if (book->HeadFootSize < 6.0f)
	  book->HeadFootSize = 6.0f;
	else if (book->HeadFootSize > 24.0f)
	  book->HeadFootSize = 24.0f;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--headingfont", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcasecmp(argv[i], "courier") == 0 ||
	    strcasecmp(argv[i], "monospace") == 0)
	  _htmlHeadingFont = HD_FONTFACE_MONOSPACE;
        else if (strcasecmp(argv[i], "times") == 0 ||
	         strcasecmp(argv[i], "serif") == 0)
	  _htmlHeadingFont = HD_FONTFACE_SERIF;
        else if (strcasecmp(argv[i], "helvetica") == 0 ||
	         strcasecmp(argv[i], "arial") == 0 ||
	         strcasecmp(argv[i], "sans-serif") == 0)
	  _htmlHeadingFont = HD_FONTFACE_SANS_SERIF;
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--help", 6) == 0)
      usage(argv[i - 1]);
#ifdef HAVE_LIBFLTK
    else if (compare_strings(argv[i], "--helpdir", 7) == 0)
    {
      i ++;
      if (i < argc)
        GUI::help_dir = argv[i];
      else
        usage(argv[i - 1]);
    }
#endif // HAVE_LIBFLTK
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

      strlcpy(book->HFImage[hfimgnum], argv[i], sizeof(book->HFImage[0]));
    }
    else if (compare_strings(argv[i], "--jpeg", 3) == 0 ||
             strncmp(argv[i], "--jpeg=", 7) == 0)
    {
      if (strlen(argv[i]) > 7)
        book->OutputJPEG = atoi(argv[i] + 7);
      else
        book->OutputJPEG = 90;
    }
    else if (compare_strings(argv[i], "--landscape", 4) == 0)
      book->Landscape = true;
    else if (compare_strings(argv[i], "--left", 4) == 0)
    {
      i ++;
      if (i < argc)
        book->PageLeft = book->get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--linkcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->LinkColor, argv[i], sizeof(book->LinkColor));
      else
        usage(argv[i - 1]);
    }
    else if (strcmp(argv[i], "--links") == 0)
      book->Links = true;
    else if (compare_strings(argv[i], "--linkstyle", 8) == 0)
    {
      i ++;
      if (i < argc)
      {
        if (strcmp(argv[i], "plain") == 0)
	  book->LinkStyle = false;
        else if (strcmp(argv[i], "underline") == 0)
	  book->LinkStyle = true;
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
        strlcpy(book->LogoImage, argv[i], sizeof(book->LogoImage));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--no-compression", 6) == 0)
      book->Compression = 0;
    else if (compare_strings(argv[i], "--no-duplex", 4) == 0)
      book->PageDuplex = false;
    else if (compare_strings(argv[i], "--no-embedfonts", 7) == 0)
      book->EmbedFonts = false;
    else if (compare_strings(argv[i], "--no-encryption", 7) == 0)
      book->Encryption = false;
    else if (compare_strings(argv[i], "--no-jpeg", 6) == 0)
      book->OutputJPEG = 0;
    else if (compare_strings(argv[i], "--no-links", 7) == 0)
      book->Links = false;
    else if (compare_strings(argv[i], "--no-localfiles", 7) == 0)
      file_nolocal();
    else if (compare_strings(argv[i], "--no-numbered", 6) == 0)
      book->TocNumbers = false;
    else if (compare_strings(argv[i], "--no-pscommands", 6) == 0)
      book->PSCommands = false;
    else if (compare_strings(argv[i], "--no-strict", 6) == 0)
      book->strict_html = false;
    else if (compare_strings(argv[i], "--no-title", 7) == 0)
      book->TitlePage = false;
    else if (compare_strings(argv[i], "--no-toc", 7) == 0)
      book->TocLevels = 0;
    else if (compare_strings(argv[i], "--no-truetype", 7) == 0)
    {
      fputs("htmldoc: Warning, --no-truetype option superceded by --no-embedfonts!\n", stderr);
      book->EmbedFonts = false;
    }
    else if (compare_strings(argv[i], "--no-xrxcomments", 6) == 0)
      book->XRXComments = false;
    else if (compare_strings(argv[i], "--numbered", 5) == 0)
      book->TocNumbers = true;
    else if (compare_strings(argv[i], "--nup", 5) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      book->NumberUp = atoi(argv[i]);

      if (book->NumberUp != 1 && book->NumberUp != 2 && book->NumberUp != 4 &&
          book->NumberUp != 6 && book->NumberUp != 9 && book->NumberUp != 16)
	usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--outdir", 6) == 0 ||
             strcmp(argv[i], "-d") == 0)
    {
      i ++;
      if (i < argc)
      {
        strlcpy(book->OutputPath, argv[i], sizeof(book->OutputPath));
        book->OutputFiles = true;
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
        strlcpy(book->OutputPath, argv[i], sizeof(book->OutputPath));
        book->OutputFiles = false;

        if ((extension = file_extension(argv[i])) != NULL)
        {
          if (strcasecmp(extension, "ps") == 0)
          {
	    book->OutputFormat = HD_OUTPUT_PS;

	    if (book->PSLevel == 0)
	      book->PSLevel = 2;
	  }
          else if (strcasecmp(extension, "pdf") == 0)
	  {
            book->OutputFormat = HD_OUTPUT_PDF;
	    book->PSLevel      = 0;
          }
	  else if (strcasecmp(extension, "html") == 0)
            book->OutputFormat = HD_OUTPUT_HTML;
        }
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--owner-password", 4) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->OwnerPassword, argv[i], sizeof(book->OwnerPassword));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--pageduration", 7) == 0)
    {
      i ++;
      if (i < argc)
      {
        book->PDFPageDuration = atof(argv[i]);

	if (book->PDFPageDuration < 1.0f)
	{
	  book->progress_error(HD_ERROR_INTERNAL_ERROR, "Bad page duration \"%s\"!",
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

      for (j = 0; j < (int)(sizeof(hdBook::PDFEffects) /
                            sizeof(hdBook::PDFEffects[0])); j ++)
        if (strcasecmp(argv[i], hdBook::PDFEffects[j]) == 0)
	{
	  book->PDFEffect = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--pagelayout", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      for (j = 0; j < (int)(sizeof(hdBook::PDFLayouts) /
                            sizeof(hdBook::PDFLayouts[0])); j ++)
        if (strcasecmp(argv[i], hdBook::PDFLayouts[j]) == 0)
	{
	  book->PDFPageLayout = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--pagemode", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      for (j = 0; j < (int)(sizeof(hdBook::PDFModes) /
                            sizeof(hdBook::PDFModes[0])); j ++)
        if (strcasecmp(argv[i], hdBook::PDFModes[j]) == 0)
	{
	  book->PDFPageMode = j;
	  break;
	}
    }
    else if (compare_strings(argv[i], "--path", 5) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->Path, argv[i], sizeof(book->Path));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--permissions", 4) == 0)
    {
      i ++;
      if (i >= argc)
        usage(argv[i - 1]);

      if (strcasecmp(argv[i], "all") == 0)
        book->Permissions = -4;
      else if (strcasecmp(argv[i], "none") == 0)
        book->Permissions = -64;
      else if (strcasecmp(argv[i], "print") == 0)
        book->Permissions |= HD_PDF_PERM_PRINT;
      else if (strcasecmp(argv[i], "no-print") == 0)
        book->Permissions &= ~HD_PDF_PERM_PRINT;
      else if (strcasecmp(argv[i], "modify") == 0)
        book->Permissions |= HD_PDF_PERM_MODIFY;
      else if (strcasecmp(argv[i], "no-modify") == 0)
        book->Permissions &= ~HD_PDF_PERM_MODIFY;
      else if (strcasecmp(argv[i], "copy") == 0)
        book->Permissions |= HD_PDF_PERM_COPY;
      else if (strcasecmp(argv[i], "no-copy") == 0)
        book->Permissions &= ~HD_PDF_PERM_COPY;
      else if (strcasecmp(argv[i], "annotate") == 0)
        book->Permissions |= HD_PDF_PERM_ANNOTATE;
      else if (strcasecmp(argv[i], "no-annotate") == 0)
        book->Permissions &= ~HD_PDF_PERM_ANNOTATE;

      if (book->Permissions != -4)
        book->Encryption = true;
    }
    else if (compare_strings(argv[i], "--portrait", 4) == 0)
      book->Landscape = false;
    else if (compare_strings(argv[i], "--proxy", 4) == 0)
    {
      i ++;
      if (i < argc)
      {
        strlcpy(book->Proxy, argv[i], sizeof(book->Proxy) - 1);
	file_proxy(book->Proxy);
      }
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--pscommands", 3) == 0)
      book->PSCommands = true;
    else if (compare_strings(argv[i], "--quiet", 3) == 0)
      book->verbosity = -1;
    else if (compare_strings(argv[i], "--right", 3) == 0)
    {
      i ++;
      if (i < argc)
        book->PageRight = book->get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--size", 4) == 0)
    {
      i ++;
      if (i < argc)
        book->set_page_size(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--strict", 4) == 0)
      book->strict_html = true;
    else if (compare_strings(argv[i], "--textcolor", 7) == 0)
    {
      i ++;
      if (i < argc)
        htmlSetTextColor((uchar *)argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--title", 7) == 0)
      book->TitlePage = true;
    else if (compare_strings(argv[i], "--titlefile", 8) == 0 ||
             compare_strings(argv[i], "--titleimage", 8) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->TitleImage, argv[i], sizeof(book->TitleImage));
      else
        usage(argv[i - 1]);

      book->TitlePage = true;
    }
    else if (compare_strings(argv[i], "--tocfooter", 6) == 0)
    {
      i ++;
      if (i < argc)
        book->get_format(argv[i], book->TocFooter);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--tocheader", 6) == 0)
    {
      i ++;
      if (i < argc)
        book->get_format(argv[i], book->TocHeader);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--toclevels", 6) == 0)
    {
      i ++;
      if (i < argc)
        book->TocLevels = atoi(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--toctitle", 6) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->TocTitle, argv[i], sizeof(book->TocTitle));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--top", 5) == 0)
    {
      i ++;
      if (i < argc)
        book->PageTop = book->get_measurement(argv[i]);
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--user-password", 4) == 0)
    {
      i ++;
      if (i < argc)
        strlcpy(book->UserPassword, argv[i], sizeof(book->UserPassword));
      else
        usage(argv[i - 1]);
    }
    else if (compare_strings(argv[i], "--truetype", 4) == 0)
    {
      fputs("htmldoc: Warning, --truetype option superceded by --embedfonts!\n", stderr);

      book->EmbedFonts = true;
    }
    else if (compare_strings(argv[i], "--verbose", 6) == 0 ||
             strcmp(argv[i], "-v") == 0)
    {
      book->verbosity ++;
    }
    else if (compare_strings(argv[i], "--version", 6) == 0)
    {
      puts(SVERSION);
      return (0);
    }
    else if (compare_strings(argv[i], "--webpage", 3) == 0)
    {
      book->TocLevels    = 0;
      book->TitlePage    = false;
      book->OutputType   = HD_OUTPUT_WEBPAGES;
      book->PDFPageMode  = HD_PDF_DOCUMENT;
      book->PDFFirstPage = HD_PDF_PAGE_1;
    }
    else if (compare_strings(argv[i], "--xrxcomments", 3) == 0)
      book->XRXComments = true;
    else if (strcmp(argv[i], "-") == 0)
    {
     /*
      * Read from stdin...
      */

      num_files ++;

      _htmlPPI = 72.0f * _htmlBrowserWidth /
                 (book->PageWidth - book->PageLeft - book->PageRight);

      file = htmlAddTree(NULL, HD_ELEMENT_FILE, NULL);
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
#ifdef HAVE_LIBFLTK
    else if (strlen(argv[i]) > 5 &&
             strcmp(argv[i] + strlen(argv[i]) - 5, ".book") == 0)
    {
      // GUI mode...
      if (BookGUI == NULL)
        BookGUI = new GUI(argv[i]);
      else
        BookGUI->loadBook(argv[i]);
    }
#endif /* HAVE_LIBFLTK */
    else
    {
      num_files ++;

      read_file(book, argv[i], &document, book->Path);
    }
  }

  // Display the GUI if necessary...
#ifdef HAVE_LIBFLTK
  if (num_files == 0 && BookGUI == NULL)
    BookGUI = new GUI();

  if (BookGUI != NULL)
  {
    Fl_File_Icon::load_system_icons();

    BookGUI->show();

    i = Fl::run();

    delete BookGUI;

    return (i);
  }
#endif /* HAVE_LIBFLTK */
    
  // We *must* have a document to process...
  if (num_files == 0 || document == NULL)
  {
    puts("ERROR: No HTML files!");
    usage();
  }

  // Find the first one in the list...
  while (document->prev != NULL)
    document = document->prev;

  htmlDebugStats("Document Tree", document);

  // Build a table of contents for the documents if necessary...
  if (book->OutputType == HD_OUTPUT_BOOK && book->TocLevels > 0)
    toc = book->toc_build(document);
  else
    toc = NULL;

  htmlDebugStats("Table of Contents Tree", toc);

  // Generate the output file(s).
  switch (book->OutputFormat)
  {
    case HD_OUTPUT_HTML :
        book->html_export(document, toc);
        break;
    case HD_OUTPUT_HTMLSEP :
        book->htmlsep_export(document, toc);
        break;
    case HD_OUTPUT_PDF :
    case HD_OUTPUT_PS :
        book->pspdf_export(document, toc);
        break;
  }

  htmlDeleteTree(document);
  htmlDeleteTree(toc);

  file_cleanup();
  book->image_flush_cache();

  return (book->error_count);
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
load_book(hdBook       *book,		// I  - Book
          const char   *filename,	// I  - Book file
          hdTree       **document)	// IO - Document tree
{
  FILE		*fp;			// File to read from
  char		line[10240];		// Line from file
  const char 	*dir;			// Directory
  const char	*local;			// Local filename
  char		path[2048];		// Current path


  // See if the filename contains a path...
  dir = file_directory(filename);

  if (dir != NULL)
    snprintf(path, sizeof(path), "%s;%s", dir, book->Path);
  else
    strlcpy(path, book->Path, sizeof(path));

  // Open the file...
  if ((local = file_find(book->Path, filename)) == NULL)
    return (0);

  if ((fp = fopen(local, "rb")) == NULL)
  {
    fprintf(stderr, "htmldoc: Unable to open \"%s\": %s\n", local,
            strerror(errno));
    return (0);
  }

  // Get the header...
  file_gets(line, sizeof(line), fp);
  if (strncmp(line, "#HTMLDOC", 8) != 0)
  {
    fclose(fp);
    book->progress_error(HD_ERROR_BAD_FORMAT,
                   "htmldoc: Bad or missing #HTMLDOC header in %s.", filename);
    return (0);
  }

  // Read the second line from the book file; for older book files, this will
  // be the file count; for new files this will be the options...
  do
  {
    file_gets(line, sizeof(line), fp);

    if (line[0] == '-')
    {
      parse_options(book, line);

      if (dir != NULL)
	snprintf(path, sizeof(path), "%s;%s", dir, book->Path);
      else
	strlcpy(path, book->Path, sizeof(path));
    }
  }
  while (!line[0]);			// Skip blank lines

  // Get input files/options...
  while (file_gets(line, sizeof(line), fp) != NULL)
  {
    if (!line[0])
      continue;				// Skip blank lines
    else if (line[0] == '-')
    {
      parse_options(book, line);

      if (dir != NULL)
	snprintf(path, sizeof(path), "%s;%s", dir, book->Path);
      else
	strlcpy(path, book->Path, sizeof(path));
    }
    else if (line[0] == '\\')
      read_file(book, line + 1, document, path);
    else
      read_file(book, line, document, path);
  }

  // Close the book file and return...
  fclose(fp);

  return (1);
}


//
// 'parse_options()' - Parse options from a book file...
//

static void
parse_options(hdBook       *book,	// I  - Book
              const char   *line)	// I - Options from book file
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
      book->PageDuplex = true;
      continue;
    }
    else if (strcmp(temp, "--landscape") == 0)
    {
      book->Landscape = true;
      continue;
    }
    else if (strcmp(temp, "--portrait") == 0)
    {
      book->Landscape = false;
      continue;
    }
    else if (strncmp(temp, "--jpeg", 6) == 0)
    {
      if (strlen(temp) > 7)
        book->OutputJPEG = atoi(temp + 7);
      else
        book->OutputJPEG = 90;
      continue;
    }
    else if (strcmp(temp, "--grayscale") == 0)
    {
      book->OutputColor = false;
      continue;
    }
    else if (strcmp(temp, "--color") == 0)
    {
      book->OutputColor = true;
      continue;
    }
    else if (strcmp(temp, "--links") == 0)
    {
      book->Links = true;
      continue;
    }
    else if (strcmp(temp, "--no-links") == 0)
    {
      book->Links = false;
      continue;
    }
    else if (strcmp(temp, "--embedfonts") == 0 ||
             strcmp(temp, "--truetype") == 0)
    {
      book->EmbedFonts = true;
      continue;
    }
    else if (strcmp(temp, "--no-embedfonts") == 0 ||
             strcmp(temp, "--no-truetype") == 0)
    {
      book->EmbedFonts = false;
      continue;
    }
    else if (strcmp(temp, "--pscommands") == 0)
    {
      book->PSCommands = true;
      continue;
    }
    else if (strcmp(temp, "--no-pscommands") == 0)
    {
      book->PSCommands = false;
      continue;
    }
    else if (strcmp(temp, "--xrxcomments") == 0)
    {
      book->XRXComments = true;
      continue;
    }
    else if (strcmp(temp, "--no-xrxcomments") == 0)
    {
      book->XRXComments = false;
      continue;
    }
    else if (strncmp(temp, "--compression", 13) == 0)
    {
      if (strlen(temp) > 14)
        book->Compression = atoi(temp + 14);
      else
        book->Compression = 1;
      continue;
    }
    else if (strcmp(temp, "--no-compression") == 0)
    {
      book->Compression = 0;
      continue;
    }
    else if (strcmp(temp, "--no-jpeg") == 0)
    {
      book->OutputJPEG = 0;
      continue;
    }
    else if (strcmp(temp, "--numbered") == 0)
    {
      book->TocNumbers = true;
      continue;
    }
    else if (strcmp(temp, "--no-numbered") == 0)
    {
      book->TocNumbers = false;
      continue;
    }
    else if (strcmp(temp, "--no-toc") == 0)
    {
      book->TocLevels = 0;
      continue;
    }
    else if (strcmp(temp, "--title") == 0 &&
             (*lineptr == '-' || !*lineptr))
    {
      book->TitlePage = true;
      continue;
    }
    else if (strcmp(temp, "--no-title") == 0)
    {
      book->TitlePage = false;
      continue;
    }
    else if (strcmp(temp, "--book") == 0)
    {
      book->OutputType = HD_OUTPUT_BOOK;
      continue;
    }
    else if (strcmp(temp, "--continuous") == 0)
    {
      book->OutputType = HD_OUTPUT_CONTINUOUS;
      continue;
    }
    else if (strcmp(temp, "--webpage") == 0)
    {
      book->OutputType = HD_OUTPUT_WEBPAGES;
      continue;
    }
    else if (strcmp(temp, "--encryption") == 0)
    {
      book->Encryption = true;
      continue;
    }
    else if (strcmp(temp, "--no-encryption") == 0)
    {
      book->Encryption = false;
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
        book->OutputFormat = HD_OUTPUT_HTML;
      else if (strcmp(temp2, "htmlsep") == 0)
        book->OutputFormat = HD_OUTPUT_HTMLSEP;
      else if (strcmp(temp2, "ps1") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PS;
	book->PSLevel      = 1;
      }
      else if (strcmp(temp2, "ps") == 0 ||
               strcmp(temp2, "ps2") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PS;
	book->PSLevel      = 2;
      }
      else if (strcmp(temp2, "ps3") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PS;
	book->PSLevel      = 3;
      }
      else if (strcmp(temp2, "pdf11") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PDF;
	book->PSLevel      = 0;
	book->PDFVersion   = 11;
      }
      else if (strcmp(temp2, "pdf12") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PDF;
	book->PSLevel      = 0;
	book->PDFVersion   = 12;
      }
      else if (strcmp(temp2, "pdf") == 0 ||
               strcmp(temp2, "pdf13") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PDF;
	book->PSLevel      = 0;
	book->PDFVersion   = 13;
      }
      else if (strcmp(temp2, "pdf14") == 0)
      {
        book->OutputFormat = HD_OUTPUT_PDF;
	book->PSLevel      = 0;
	book->PDFVersion   = 14;
      }
    }
    else if (strcmp(temp, "--logo") == 0 ||
             strcmp(temp, "--logoimage") == 0)
      strlcpy(book->LogoImage, temp2, sizeof(book->LogoImage));
    else if (strcmp(temp, "--titleimage") == 0)
    {
      book->TitlePage = false;
      strlcpy(book->TitleImage, temp2, sizeof(book->TitleImage));
    }
    else if (strcmp(temp, "-f") == 0)
    {
      book->OutputFiles = false;
      strlcpy(book->OutputPath, temp2, sizeof(book->OutputPath));
    }
    else if (strcmp(temp, "-d") == 0)
    {
      book->OutputFiles = true;
      strlcpy(book->OutputPath, temp2, sizeof(book->OutputPath));
    }
    else if (strcmp(temp, "--browserwidth") == 0)
      _htmlBrowserWidth = atof(temp2);
    else if (strcmp(temp, "--nup") == 0)
      book->NumberUp = atoi(temp2);
    else if (strcmp(temp, "--size") == 0)
      book->set_page_size(temp2);
    else if (strcmp(temp, "--left") == 0)
      book->PageLeft = book->get_measurement(temp2);
    else if (strcmp(temp, "--right") == 0)
      book->PageRight = book->get_measurement(temp2);
    else if (strcmp(temp, "--top") == 0)
      book->PageTop = book->get_measurement(temp2);
    else if (strcmp(temp, "--bottom") == 0)
      book->PageBottom = book->get_measurement(temp2);
    else if (strcmp(temp, "--header") == 0)
      book->get_format(temp2, book->Header);
    else if (strcmp(temp, "--footer") == 0)
      book->get_format(temp2, book->Footer);
    else if (strcmp(temp, "--bodycolor") == 0)
      strlcpy(book->BodyColor, temp2, sizeof(book->BodyColor));
    else if (strcmp(temp, "--bodyimage") == 0)
      strlcpy(book->BodyImage, temp2, sizeof(book->BodyImage));
    else if (strcmp(temp, "--textcolor") == 0)
      htmlSetTextColor((uchar *)temp2);
    else if (strcmp(temp, "--linkcolor") == 0)
      strlcpy(book->LinkColor, temp2, sizeof(book->LinkColor));
    else if (strcmp(temp, "--linkstyle") == 0)
    {
      if (strcmp(temp2, "plain") == 0)
        book->LinkStyle = false;
      else
        book->LinkStyle = true;
    }
    else if (strcmp(temp, "--toclevels") == 0)
      book->TocLevels = atoi(temp2);
    else if (strcmp(temp, "--tocheader") == 0)
      book->get_format(temp2, book->TocHeader);
    else if (strcmp(temp, "--tocfooter") == 0)
      book->get_format(temp2, book->TocFooter);
    else if (strcmp(temp, "--toctitle") == 0)
      strlcpy(book->TocTitle, temp2, sizeof(book->TocTitle));
    else if (strcmp(temp, "--fontsize") == 0)
    {
      fontsize    = atof(temp2);
      fontspacing = _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P];

      if (fontsize < 4.0f)
        fontsize = 4.0f;
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
        _htmlHeadingFont = HD_FONTFACE_MONOSPACE;
      else if (strcasecmp(temp2, "times") == 0 ||
               strcasecmp(temp2, "serif") == 0)
        _htmlHeadingFont = HD_FONTFACE_SERIF;
      else if (strcasecmp(temp2, "helvetica") == 0 ||
               strcasecmp(temp2, "arial") == 0 ||
               strcasecmp(temp2, "sans-serif") == 0)
        _htmlHeadingFont = HD_FONTFACE_SANS_SERIF;
    }
    else if (strcmp(temp, "--bodyfont") == 0)
    {
      if (strcasecmp(temp2, "courier") == 0 ||
          strcasecmp(temp2, "monospace") == 0)
        _htmlBodyFont = HD_FONTFACE_MONOSPACE;
      else if (strcasecmp(temp2, "times") == 0 ||
               strcasecmp(temp2, "serif") == 0)
        _htmlBodyFont = HD_FONTFACE_SERIF;
      else if (strcasecmp(temp2, "helvetica") == 0 ||
               strcasecmp(temp2, "arial") == 0 ||
               strcasecmp(temp2, "sans-serif") == 0)
        _htmlBodyFont = HD_FONTFACE_SANS_SERIF;
    }
    else if (strcmp(temp, "--headfootsize") == 0)
      book->HeadFootSize = atof(temp2);
    else if (strcmp(temp, "--headfootfont") == 0)
    {
      if (strcasecmp(temp2, "courier") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
      }
      else if (strcasecmp(temp2, "courier-bold") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
      }
      else if (strcasecmp(temp2, "courier-oblique") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
      }
      else if (strcasecmp(temp2, "courier-boldoblique") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_MONOSPACE;
	book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
      }
      else if (strcasecmp(temp2, "times") == 0 ||
	         strcasecmp(temp2, "times-roman") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
      }
      else if (strcasecmp(temp2, "times-bold") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
      }
      else if (strcasecmp(temp2, "times-italic") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
      }
      else if (strcasecmp(temp2, "times-bolditalic") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
      }
      else if (strcasecmp(temp2, "helvetica") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_NORMAL;
      }
      else if (strcasecmp(temp2, "helvetica-bold") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_BOLD;
      }
      else if (strcasecmp(temp2, "helvetica-oblique") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
	book->HeadFootStyle = HD_FONTINTERNAL_ITALIC;
      }
      else if (strcasecmp(temp2, "helvetica-boldoblique") == 0)
      {
	book->HeadFootType  = HD_FONTFACE_SANS_SERIF;
        book->HeadFootStyle = HD_FONTINTERNAL_BOLD_ITALIC;
      }
    }
    else if (strcmp(temp, "--charset") == 0)
      htmlSetCharSet(temp2);
    else if (strcmp(temp, "--pagemode") == 0)
    {
      for (i = 0; i < (int)(sizeof(hdBook::PDFModes) /
                            sizeof(hdBook::PDFModes[0])); i ++)
        if (strcasecmp(temp2, hdBook::PDFModes[i]) == 0)
	{
	  book->PDFPageMode = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pagelayout") == 0)
    {
      for (i = 0; i < (int)(sizeof(hdBook::PDFLayouts) /
                            sizeof(hdBook::PDFLayouts[0])); i ++)
        if (strcasecmp(temp2, hdBook::PDFLayouts[i]) == 0)
	{
	  book->PDFPageLayout = i;
	  break;
	}
    }
    else if (strcmp(temp, "--firstpage") == 0)
    {
      for (i = 0; i < (int)(sizeof(hdBook::PDFPages) /
                            sizeof(hdBook::PDFPages[0])); i ++)
        if (strcasecmp(temp2, hdBook::PDFPages[i]) == 0)
	{
	  book->PDFFirstPage = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pageeffect") == 0)
    {
      for (i = 0; i < (int)(sizeof(hdBook::PDFEffects) /
                            sizeof(hdBook::PDFEffects[0])); i ++)
        if (strcasecmp(temp2, hdBook::PDFEffects[i]) == 0)
	{
	  book->PDFEffect = i;
	  break;
	}
    }
    else if (strcmp(temp, "--pageduration") == 0)
      book->PDFPageDuration = atof(temp2);
    else if (strcmp(temp, "--effectduration") == 0)
      book->PDFEffectDuration = atof(temp2);
    else if (strcmp(temp, "--permissions") == 0)
    {
      if (strcasecmp(temp2, "all") == 0)
        book->Permissions = -4;
      else if (strcasecmp(temp2, "none") == 0)
        book->Permissions = -64;
      else if (strcasecmp(temp2, "print") == 0)
        book->Permissions |= HD_PDF_PERM_PRINT;
      else if (strcasecmp(temp2, "no-print") == 0)
        book->Permissions &= ~HD_PDF_PERM_PRINT;
      else if (strcasecmp(temp2, "modify") == 0)
        book->Permissions |= HD_PDF_PERM_MODIFY;
      else if (strcasecmp(temp2, "no-modify") == 0)
        book->Permissions &= ~HD_PDF_PERM_MODIFY;
      else if (strcasecmp(temp2, "copy") == 0)
        book->Permissions |= HD_PDF_PERM_COPY;
      else if (strcasecmp(temp2, "no-copy") == 0)
        book->Permissions &= ~HD_PDF_PERM_COPY;
      else if (strcasecmp(temp2, "annotate") == 0)
        book->Permissions |= HD_PDF_PERM_ANNOTATE;
      else if (strcasecmp(temp2, "no-annotate") == 0)
        book->Permissions &= ~HD_PDF_PERM_ANNOTATE;
    }
    else if (strcmp(temp, "--user-password") == 0)
      strlcpy(book->UserPassword, temp2, sizeof(book->UserPassword));
    else if (strcmp(temp, "--owner-password") == 0)
      strlcpy(book->OwnerPassword, temp2, sizeof(book->OwnerPassword));
    else if (strcmp(temp, "--path") == 0)
      strlcpy(book->Path, temp2, sizeof(book->Path));
    else if (strcmp(temp, "--proxy") == 0)
    {
      strlcpy(book->Proxy, temp2, sizeof(book->Proxy));
      file_proxy(book->Proxy);
    }
  }
}


//
// 'read_file()' - Read a file into the current document.
//

static int				// O  - 1 on success, 0 on failure
read_file(hdBook     *book,		// I  - Book
          const char *filename,		// I  - File/URL to read
          hdTree     **document,	// IO - Current document
	  const char *path)		// I  - Search path
{
  FILE		*docfile;		// Document file
  hdTree	*file;			// HTML document file
  const char	*realname;		// Real name of file
  char		base[1024];		// Base directory name of file


  DEBUG_printf(("read_file(filename=\"%s\", document=%p, path=\"%s\")\n",
                filename, document, path));

  if ((realname = file_find(path, filename)) != NULL)
  {
    if ((docfile = fopen(realname, "rb")) != NULL)
    {
     /*
      * Read from a file...
      */

      if (book->verbosity > 0)
        fprintf(stderr, "htmldoc: Reading %s...\n", filename);

      _htmlPPI = 72.0f * _htmlBrowserWidth /
                 (book->PageWidth - book->PageLeft - book->PageRight);

      strlcpy(base, file_directory(filename), sizeof(base));

      file = htmlAddTree(NULL, HD_ELEMENT_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME",
                      (uchar *)file_basename(filename));

      htmlReadFile(file, docfile, base);

      fclose(docfile);

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
      book->progress_error(HD_ERROR_FILE_NOT_FOUND,
                           "Unable to open \"%s\" for reading...", filename);
    }
  }
  else
  {
    file = NULL;
    book->progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to find \"%s\"...",
                         filename);
  }

  return (file != NULL);
}


//
// 'term_handler()' - Handle CTRL-C or kill signals...
//

static void
term_handler(int signum)		// I - Signal number
{
  REF(signum);

  file_cleanup();
//  image_flush_cache();
  exit(1);
}


//
// 'usage()' - Show program version and command-line options.
//

static void
usage(const char *arg)			// I - Bad argument string
{
  if (arg)
    printf("Bad option argument \"%s\"!\n\n", arg);

  puts("HTMLDOC Version " SVERSION " Copyright 1997-2004 Easy Software Products, All Rights Reserved.");
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
  puts("  --bodyimage filename.{bmp,gif,jpg,png}");
  puts("  --book");
  puts("  --bottom margin{in,cm,mm}");
  puts("  --browserwidth pixels");
  puts("  --charset {cp-874...1258,iso-8859-1...8859-15,koi8-r}");
  puts("  --color");
  puts("  --compression[=level]");
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
  puts("  --headfootfont {courier{-bold,-oblique,-boldoblique},\n"
       "                  times{-roman,-bold,-italic,-bolditalic},\n"
       "                  helvetica{-bold,-oblique,-boldoblique}}");
  puts("  --headfootsize {6.0..24.0}");
  puts("  --headingfont {courier,times,helvetica}");
  puts("  --help");
#ifdef HAVE_LIBFLTK
  puts("  --helpdir directory");
#endif // HAVE_LIBFLTK
  for (int i = 0; i < MAX_HF_IMAGES; i ++)
    printf("  --hfimage%d filename.{bmp,gif,jpg,png}\n", i);
  puts("  --jpeg[=quality]");
  puts("  --landscape");
  puts("  --left margin{in,cm,mm}");
  puts("  --linkcolor color");
  puts("  --links");
  puts("  --linkstyle {plain,underline}");
  puts("  --logoimage filename.{bmp,gif,jpg,png}");
  puts("  --owner-password password");
  puts("  --no-compression");
  puts("  --no-duplex");
  puts("  --no-embedfonts");
  puts("  --no-encryption");
  puts("  --no-links");
  puts("  --no-localfiles");
  puts("  --no-numbered");
  puts("  --no-pscommands");
  puts("  --no-strict");
  puts("  --no-title");
  puts("  --no-toc");
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

  exit(1);
}


//
// End of "$Id: htmldoc.cxx,v 1.44 2004/04/05 01:39:34 mike Exp $".
//
