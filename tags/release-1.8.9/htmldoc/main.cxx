/*
 * "$Id: main.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $"
 *
 *   Main entry for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2000 by Easy Software Products.
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

#if defined(WIN32) || defined(__EMX__)
#  include <io.h>
#endif // WIN32 || __EMX__
#include <fcntl.h>


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

  int		i, j;		/* Looping vars */
  FILE		*docfile;	/* Document file */
  tree_t	*document,	/* Master HTML document */
		*file,		/* HTML document file */
		*toc;		/* Table of contents */
  int		(*exportfunc)(tree_t *, tree_t *);
				/* Export function */
  char		*extension,	/* Extension of output filename */
		*filename;	/* Current filename */
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
      OutputBook = 1;
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
        _htmlBrowserWidth = atof(argv[i]);
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
    else if (compare_strings(argv[i], "--datadir", 4) == 0)
    {
      i ++;
      if (i < argc)
        _htmlData = argv[i];
      else
        usage();
    }
    else if (compare_strings(argv[i], "--duplex", 4) == 0)
      PageDuplex = 1;
    else if (compare_strings(argv[i], "--effectduration", 4) == 0)
    {
      i ++;
      if (i < argc)
        PDFEffectDuration = atof(argv[i]);
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

      for (j = 0; j < (sizeof(PDFPages) / sizeof(PDFPages[0])); j ++)
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
        {
	  exportfunc = pspdf_export;
	  PSLevel    = 1;
	}
        else if (strcasecmp(argv[i], "ps2") == 0 ||
                 strcasecmp(argv[i], "ps") == 0)
        {
	  exportfunc = pspdf_export;
	  PSLevel    = 2;
	}
        else if (strcasecmp(argv[i], "ps3") == 0)
        {
	  exportfunc = pspdf_export;
	  PSLevel    = 3;
	}
        else if (strcasecmp(argv[i], "pdf13") == 0)
	{
          exportfunc = pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 1.3;
	}
        else if (strcasecmp(argv[i], "pdf12") == 0 ||
	         strcasecmp(argv[i], "pdf") == 0)
	{
          exportfunc = pspdf_export;
	  PSLevel    = 0;
	  PDFVersion = 1.2;
	}
        else if (strcasecmp(argv[i], "pdf11") == 0)
	{
          exportfunc  = pspdf_export;
	  PSLevel     = 0;
	  PDFVersion  = 1.1;
	  Compression = 0;
	}
        else if (strcasecmp(argv[i], "html") == 0)
          exportfunc = html_export;
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
    else if (compare_strings(argv[i], "--linkstyle", 7) == 0)
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
    else if (compare_strings(argv[i], "--no-numbered", 6) == 0)
      TocNumbers = 0;
    else if (compare_strings(argv[i], "--no-pscommands", 6) == 0)
      PSCommands = 0;
    else if (compare_strings(argv[i], "--no-toc", 7) == 0)
      TocLevels = 0;
    else if (compare_strings(argv[i], "--no-title", 7) == 0)
      TitlePage = 0;
    else if (compare_strings(argv[i], "--numbered", 4) == 0)
      TocNumbers = 1;
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
	    exportfunc = pspdf_export;

	    if (PSLevel == 0)
	      PSLevel = 2;
	  }
          else if (strcasecmp(extension, "pdf") == 0)
	  {
            exportfunc = pspdf_export;
	    PSLevel    = 0;
          }
	  else if (strcasecmp(extension, "html") == 0)
            exportfunc = html_export;
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
        PDFPageDuration = atof(argv[i]);
      else
        usage();
    }
    else if (compare_strings(argv[i], "--pageeffect", 7) == 0)
    {
      i ++;
      if (i >= argc)
        usage();

      for (j = 0; j < (sizeof(PDFEffects) / sizeof(PDFEffects[0])); j ++)
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

      for (j = 0; j < (sizeof(PDFLayouts) / sizeof(PDFLayouts[0])); j ++)
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

      for (j = 0; j < (sizeof(PDFModes) / sizeof(PDFModes[0])); j ++)
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
    else if (compare_strings(argv[i], "--pscommands", 3) == 0)
      PSCommands = 1;
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
    else if (compare_strings(argv[i], "--verbose", 3) == 0 ||
             strcmp(argv[i], "-v") == 0)
    {
      Verbosity ++;
    }
    else if (compare_strings(argv[i], "--webpage", 3) == 0)
    {
      TocLevels    = 0;
      TitlePage    = 0;
      OutputBook   = 0;
      PDFPageMode  = PDF_DOCUMENT;
      PDFFirstPage = PDF_PAGE_1;

      if (exportfunc == html_export)
      {
        exportfunc = pspdf_export;
	PSLevel    = 0;
      }
    }
    else if (strcmp(argv[i], "-") == 0)
    {
     /*
      * Read from stdin...
      */

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME", (uchar *)"");

#if defined(WIN32) || defined(__EMX__)
      // Make sure stdin is in binary mode.
      // (I hate Microsoft... I hate Microsoft... Everybody join in!)
      setmode(0, O_BINARY);
#endif // WIN32 || __EMX__

      htmlReadFile(file, stdin, "");

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
    else if ((filename = file_find(Path, argv[i])) != NULL)
    {
      if ((docfile = fopen(filename, "rb")) != NULL)
      {
       /*
	* Read from a file...
	*/

	if (Verbosity)
          fprintf(stderr, "htmldoc: Reading %s...\n", argv[i]);

	strcpy(base, file_directory(argv[i]));

	file = htmlAddTree(NULL, MARKUP_FILE, NULL);
	htmlSetVariable(file, (uchar *)"FILENAME",
                	(uchar *)file_basename(argv[i]));

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
        progress_error("Unable to read file \"%s\"...", filename);
    }
    else
      progress_error("Unable to find file \"%s\"...", argv[i]);

 /*
  * Display the GUI if necessary...
  */

#ifdef HAVE_LIBFLTK
  if (document == NULL && BookGUI == NULL)
    BookGUI = new GUI();

  FileIcon::load_system_icons();

  if (BookGUI != NULL)
  {
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
    usage();

 /*
  * Find the first one in the list...
  */

  while (document->prev != NULL)
    document = document->prev;

 /*
  * Build a table of contents for the documents if necessary...
  */

  if (OutputBook && TocLevels > 0)
    toc = toc_build(document);
  else
    toc = NULL;

 /*
  * Generate the output file(s).
  */

  Errors = 0;

  (*exportfunc)(document, toc);

  return (Errors);
}


/*
 * 'prefs_load()' - Load HTMLDOC preferences...
 */

void
prefs_load(void)
{
#ifdef WIN32			//// Do registry magic...
  HKEY		key;		// Registry key
  DWORD		size;		// Size of string
  char		value[2048];	// Attribute value
  static char	data[1024];	// Data directory
  static char	doc[1024];	// Documentation directory


  // Figure out what the HTML editor is...
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "SOFTWARE\\Easy Software Products\\HTMLDOC", 0,
		   KEY_READ, &key))
    return;

#  ifdef HAVE_LIBFLTK
  size = sizeof(HTMLEditor);
  RegQueryValueEx(key, "editor", NULL, NULL, (unsigned char *)HTMLEditor, &size);
#  endif // HAVE_LIBFLTK

  // Now grab the installed directories...
  size = sizeof(data);
  if (!RegQueryValueEx(key, "data", NULL, NULL, (unsigned char *)data, &size))
    _htmlData = data;

#  ifdef HAVE_LIBFLTK
  size = sizeof(doc);
  if (!RegQueryValueEx(key, "doc", NULL, NULL, (unsigned char *)doc, &size))
    GUI::help_dir = doc;
#  endif // HAVE_LIBFLTK

  // Then any other saved options...
  size = sizeof(value);
  if (!RegQueryValueEx(key, "textcolor", NULL, NULL, (unsigned char *)value, &size))
    htmlSetTextColor((uchar *)value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "bodycolor", NULL, NULL, (unsigned char *)value, &size))
    strcpy(BodyColor, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "bodyimage", NULL, NULL, (unsigned char *)value, &size))
    strcpy(BodyImage, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "linkcolor", NULL, NULL, (unsigned char *)value, &size))
    strcpy(LinkColor, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "linkstyle", NULL, NULL, (unsigned char *)value, &size))
    LinkStyle = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "browserwidth", NULL, NULL, (unsigned char *)value, &size))
    _htmlBrowserWidth = atof(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagewidth", NULL, NULL, (unsigned char *)value, &size))
    PageWidth = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagelength", NULL, NULL, (unsigned char *)value, &size))
    PageLength = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageleft", NULL, NULL, (unsigned char *)value, &size))
    PageLeft = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageright", NULL, NULL, (unsigned char *)value, &size))
    PageRight = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagetop", NULL, NULL, (unsigned char *)value, &size))
    PageTop = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagebottom", NULL, NULL, (unsigned char *)value, &size))
    PageBottom = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageduplex", NULL, NULL, (unsigned char *)value, &size))
    PageDuplex = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "landscape", NULL, NULL, (unsigned char *)value, &size))
    Landscape = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "compression", NULL, NULL, (unsigned char *)value, &size))
    Compression = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "outputcolor", NULL, NULL, (unsigned char *)value, &size))
    OutputColor = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "tocnumbers", NULL, NULL, (unsigned char *)value, &size))
    TocNumbers = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "toclevels", NULL, NULL, (unsigned char *)value, &size))
    TocLevels = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "jpeg", NULL, NULL, (unsigned char *)value, &size))
    OutputJPEG = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageheader", NULL, NULL, (unsigned char *)value, &size))
    strcpy(Header, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagefooter", NULL, NULL, (unsigned char *)value, &size))
    strcpy(Footer, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "tocheader", NULL, NULL, (unsigned char *)value, &size))
    strcpy(TocHeader, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "tocfooter", NULL, NULL, (unsigned char *)value, &size))
    strcpy(TocFooter, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "toctitle", NULL, NULL, (unsigned char *)value, &size))
    strcpy(TocTitle, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "bodyfont", NULL, NULL, (unsigned char *)value, &size))
    _htmlBodyFont = (typeface_t)atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "headingfont", NULL, NULL, (unsigned char *)value, &size))
    _htmlHeadingFont = (typeface_t)atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "fontsize", NULL, NULL, (unsigned char *)value, &size))
    htmlSetBaseSize(atof(value), _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "fontspacing", NULL, NULL, (unsigned char *)value, &size))
    htmlSetBaseSize(_htmlSizes[SIZE_P], atof(value));

  size = sizeof(value);
  if (!RegQueryValueEx(key, "headfoottype", NULL, NULL, (unsigned char *)value, &size))
    HeadFootType = (typeface_t)atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "headfootstyle", NULL, NULL, (unsigned char *)value, &size))
    HeadFootStyle = (style_t)atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "headfootsize", NULL, NULL, (unsigned char *)value, &size))
    HeadFootSize = atof(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pdfversion", NULL, NULL, (unsigned char *)value, &size))
    PDFVersion = atof(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pslevel", NULL, NULL, (unsigned char *)value, &size))
    PSLevel = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "charset", NULL, NULL, (unsigned char *)value, &size))
    htmlSetCharSet(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagemode", NULL, NULL, (unsigned char *)value, &size))
    PDFPageMode = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pagelayout", NULL, NULL, (unsigned char *)value, &size))
    PDFPageLayout = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "firstpage", NULL, NULL, (unsigned char *)value, &size))
    PDFFirstPage = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageeffect", NULL, NULL, (unsigned char *)value, &size))
    PDFEffect = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "pageduration", NULL, NULL, (unsigned char *)value, &size))
    PDFPageDuration = atof(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "effectduration", NULL, NULL, (unsigned char *)value, &size))
    PDFEffectDuration = atof(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "encryption", NULL, NULL, (unsigned char *)value, &size))
    Encryption = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "permissions", NULL, NULL, (unsigned char *)value, &size))
    Permissions = atoi(value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "ownerpassword", NULL, NULL, (unsigned char *)value, &size))
    strcpy(OwnerPassword, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "userpassword", NULL, NULL, (unsigned char *)value, &size))
    strcpy(UserPassword, value);

  size = sizeof(value);
  if (!RegQueryValueEx(key, "path", NULL, NULL, (unsigned char *)value, &size))
    strcpy(Path, value);

  RegCloseKey(key);
#else				//// Do .htmldocrc file in home dir...
  char	line[2048],		// Line from RC file
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
	  strcpy(Header, line + 11);
        else if (strncasecmp(line, "PAGEFOOTER=", 11) == 0)
	  strcpy(Footer, line + 11);
        else if (strncasecmp(line, "TOCHEADER=", 10) == 0)
	  strcpy(TocHeader, line + 10);
        else if (strncasecmp(line, "TOCFOOTER=", 10) == 0)
	  strcpy(TocFooter, line + 10);
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
	else if (strncasecmp(line, "PATH=", 5) == 0)
	{
	  strncpy(Path, line + 5, sizeof(Path) - 1);
	  Path[sizeof(Path) - 1] = '\0';
	}
#  ifdef HAVE_LIBFLTK
        else if (strncasecmp(line, "EDITOR=", 7) == 0)
	  strcpy(HTMLEditor, line + 7);
#  endif // HAVE_LIBFLTK
      }

      fclose(fp);
    }
  }

  if (getenv("HTMLDOC_DATA") != NULL)
    _htmlData = getenv("HTMLDOC_DATA");

#  ifdef HAVE_LIBFLTK
  if (getenv("HTMLDOC_HELP") != NULL)
    GUI::help_dir = getenv("HTMLDOC_HELP");
#  endif // HAVE_LIBFLTK
#endif // WIN32
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
  char		value[1024];	// Numeric value

  if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     "SOFTWARE\\Easy Software Products\\HTMLDOC", 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL))
    return;

  // Save what the HTML editor is...
  size = strlen(HTMLEditor) + 1;
  RegSetValueEx(key, "editor", 0, REG_SZ, (unsigned char *)HTMLEditor, size);

  // Now the rest of the options...
  size = strlen((char *)_htmlTextColor) + 1;
  RegSetValueEx(key, "textcolor", 0, REG_SZ, (unsigned char *)_htmlTextColor, size);

  size = strlen(BodyColor) + 1;
  RegSetValueEx(key, "bodycolor", 0, REG_SZ, (unsigned char *)BodyColor, size);

  size = strlen(BodyImage) + 1;
  RegSetValueEx(key, "bodyimage", 0, REG_SZ, (unsigned char *)BodyImage, size);

  size = strlen(LinkColor) + 1;
  RegSetValueEx(key, "linkcolor", 0, REG_SZ, (unsigned char *)LinkColor, size);

  sprintf(value, "%d", LinkStyle);
  size = strlen(value) + 1;
  RegSetValueEx(key, "linkstyle", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.0f", _htmlBrowserWidth);
  size = strlen(value) + 1;
  RegSetValueEx(key, "browserwidth", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageWidth);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagewidth", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageLength);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagelength", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageLeft);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pageleft", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageRight);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pageright", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageTop);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagetop", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageBottom);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagebottom", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PageDuplex);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pageduplex", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", Landscape);
  size = strlen(value) + 1;
  RegSetValueEx(key, "landscape", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", Compression);
  size = strlen(value) + 1;
  RegSetValueEx(key, "compression", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", OutputColor);
  size = strlen(value) + 1;
  RegSetValueEx(key, "outputcolor", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", TocNumbers);
  size = strlen(value) + 1;
  RegSetValueEx(key, "tocnumbers", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", TocLevels);
  size = strlen(value) + 1;
  RegSetValueEx(key, "toclevels", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", OutputJPEG);
  size = strlen(value) + 1;
  RegSetValueEx(key, "jpeg", 0, REG_SZ, (unsigned char *)value, size);

  size = strlen(Header) + 1;
  RegSetValueEx(key, "pageheader", 0, REG_SZ, (unsigned char *)Header, size);

  size = strlen(Footer) + 1;
  RegSetValueEx(key, "pagefooter", 0, REG_SZ, (unsigned char *)Footer, size);

  size = strlen(TocHeader) + 1;
  RegSetValueEx(key, "tocheader", 0, REG_SZ, (unsigned char *)TocHeader, size);

  size = strlen(TocFooter) + 1;
  RegSetValueEx(key, "tocfooter", 0, REG_SZ, (unsigned char *)TocFooter, size);

  size = strlen(TocTitle) + 1;
  RegSetValueEx(key, "toctitle", 0, REG_SZ, (unsigned char *)TocTitle, size);

  sprintf(value, "%d", _htmlBodyFont);
  size = strlen(value) + 1;
  RegSetValueEx(key, "bodyfont", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", _htmlHeadingFont);
  size = strlen(value) + 1;
  RegSetValueEx(key, "headingfont", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", _htmlSizes[SIZE_P]);
  size = strlen(value) + 1;
  RegSetValueEx(key, "fontsize", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
  size = strlen(value) + 1;
  RegSetValueEx(key, "fontspacing", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", HeadFootType);
  size = strlen(value) + 1;
  RegSetValueEx(key, "headfoottype", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", HeadFootStyle);
  size = strlen(value) + 1;
  RegSetValueEx(key, "headfootstyle", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", HeadFootSize);
  size = strlen(value) + 1;
  RegSetValueEx(key, "headfootsize", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", PDFVersion);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pdfversion", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PSLevel);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pslevel", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PSCommands);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pscommands", 0, REG_SZ, (unsigned char *)value, size);

  size = strlen(_htmlCharSet) + 1;
  RegSetValueEx(key, "charset", 0, REG_SZ, (unsigned char *)_htmlCharSet, size);

  sprintf(value, "%d", PDFPageMode);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagemode", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PDFPageLayout);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pagelayout", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PDFFirstPage);
  size = strlen(value) + 1;
  RegSetValueEx(key, "firstpage", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", PDFEffect);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pageeffect", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", PDFPageDuration);
  size = strlen(value) + 1;
  RegSetValueEx(key, "pageduration", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%.1f", PDFEffectDuration);
  size = strlen(value) + 1;
  RegSetValueEx(key, "effectduration", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", Encryption);
  size = strlen(value) + 1;
  RegSetValueEx(key, "encryption", 0, REG_SZ, (unsigned char *)value, size);

  sprintf(value, "%d", Permissions);
  size = strlen(value) + 1;
  RegSetValueEx(key, "permissions", 0, REG_SZ, (unsigned char *)value, size);

  size = strlen(OwnerPassword) + 1;
  RegSetValueEx(key, "ownerpassword", 0, REG_SZ,
                (unsigned char *)OwnerPassword, size);

  size = strlen(UserPassword) + 1;
  RegSetValueEx(key, "userpassword", 0, REG_SZ,
                (unsigned char *)UserPassword, size);

  size = strlen(Path) + 1;
  RegSetValueEx(key, "path", 0, REG_SZ, (unsigned char *)Path, size);

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
      fprintf(fp, "PAGEHEADER=%s\n", Header);
      fprintf(fp, "PAGEFOOTER=%s\n", Footer);
      fprintf(fp, "TOCHEADER=%s\n", TocHeader);
      fprintf(fp, "TOCFOOTER=%s\n", TocFooter);
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
      fprintf(fp, "PATH=%s\n", Path);

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
  puts("HTMLDOC Version " SVERSION " Copyright 1997-2000 Easy Software Products, All Rights Reserved.");
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
  puts("  {--format, -t} {ps1,ps2,ps3,pdf11,pdf12,pdf13,html}");
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
  puts("  --linkstyle {plain,underline}");
  puts("  --logoimage filename.{gif,jpg,png}");
  puts("  --owner-password password");
  puts("  --no-compression");
  puts("  --no-duplex");
  puts("  --no-encryption");
  puts("  --no-numbered");
  puts("  --no-pscommands");
  puts("  --no-title");
  puts("  --no-toc");
  puts("  --numbered");
  puts("  {--outdir, -d} dirname");
  puts("  {--outfile, -f} filename.{ps,pdf,html}");
  puts("  --pageduration {1.0..60.0}");
  puts("  --pageeffect {none,bi,bo,d,gd,gdr,gr,hb,hsi,hso,vb,vsi,vso,wd,wl,wr,wu}");
  puts("  --pagelayout {single,one,twoleft,tworight}");
  puts("  --pagemode {document,outline,fullscreen}");
  puts("  --path \"dir1;dir2;dir3;...;dirN\"");
  puts("  --permissions {all,annotate,copy,modify,print,no-annotate,no-copy,no-modify,no-print,none}");
  puts("  --portrait");
  puts("  --pscommands");
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
  puts("  --user-password password");
  puts("  {--verbose, -v}");
  puts("  --webpage");
  puts("");
  puts("  fff = heading format string; each \'f\' can be one of:");
  puts("");
  puts("        . = blank");
  puts("        t = title text");
  puts("        h = current heading");
  puts("        c = current chapter heading");
  puts("        C = current chapter page number (arabic)");
  puts("        l = logo image");
  puts("        i = lowercase roman numerals");
  puts("        I = uppercase roman numerals");
  puts("        1 = arabic numbers (1, 2, 3, ...)");
  puts("        a = lowercase letters");
  puts("        A = uppercase letters");

  exit(1);
}


/*
 * End of "$Id: main.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $".
 */
