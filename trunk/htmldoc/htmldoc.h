//
// "$Id$"
//
//   Primary header file for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//

//
// Include necessary headers.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "html.h"
#include "image.h"
#include "debug.h"
#include "progress.h"

#ifdef WIN32	    /* Include all 8 million Windows header files... */
#  include <windows.h>
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


//
// Macro to get rid of "unreferenced variable xyz" warnings...
//

#define REF(x)		(void)x;


//
// Output type...
//

enum
{
  HD_OUTPUT_BOOK,
  HD_OUTPUT_CONTINUOUS,
  HD_OUTPUT_WEBPAGES
};


//
// PDF constants...
//

enum	/* PDF page mode */
{
  HD_PDF_DOCUMENT,
  HD_PDF_OUTLINE,
  HD_PDF_FULLSCREEN
};

enum	/* PDF page layout */
{
  HD_PDF_SINGLE,
  HD_PDF_ONE_COLUMN,
  HD_PDF_TWO_COLUMN_LEFT,
  HD_PDF_TWO_COLUMN_RIGHT
};

enum	/* PDF first page */
{
  HD_PDF_PAGE_1,
  HD_PDF_TOC,
  HD_PDF_CHAPTER_1
};

enum	/* PDF transition effect */
{
  HD_PDF_NONE,
  HD_PDF_BOX_INWARD,
  HD_PDF_BOX_OUTWARD,
  HD_PDF_DISSOLVE,
  HD_PDF_GLITTER_DOWN,
  HD_PDF_GLITTER_DOWN_RIGHT,
  HD_PDF_GLITTER_RIGHT,
  HD_PDF_HORIZONTAL_BLINDS,
  HD_PDF_HORIZONTAL_SWEEP_INWARD,
  HD_PDF_HORIZONTAL_SWEEP_OUTWARD,
  HD_PDF_VERTICAL_BLINDS,
  HD_PDF_VERTICAL_SWEEP_INWARD,
  HD_PDF_VERTICAL_SWEEP_OUTWARD,
  HD_PDF_WIPE_DOWN,
  HD_PDF_WIPE_LEFT,
  HD_PDF_WIPE_RIGHT,
  HD_PDF_WIPE_UP
};

enum	/* PDF document permissions */
{
  HD_PDF_PERM_PRINT = 4,
  HD_PDF_PERM_MODIFY = 8,
  HD_PDF_PERM_COPY = 16,
  HD_PDF_PERM_ANNOTATE = 32
};


//
// Globals...
//

#ifdef _HTMLDOC_CXX_
#  define VAR
#  define VALUE(x)	=x
#  define NULL3		={0,0,0}
#else
#  define VAR		extern
#  define VALUE(x)
#  define NULL3
#endif /* _HTML_DOC_CXX_ */

VAR int		Verbosity	VALUE(0);	/* Verbosity */
VAR int		OverflowErrors	VALUE(0);	/* Show errors on overflow */
VAR int		StrictHTML	VALUE(0);	/* Do strict HTML checking */
VAR int		CGIMode		VALUE(0);	/* Running as CGI? */
VAR int		Errors		VALUE(0);	/* Number of errors */
VAR int		Compression	VALUE(1);	/* Non-zero means compress PDFs */
VAR int		TitlePage	VALUE(1),	/* Need a title page */
		TocLevels	VALUE(3),	/* Number of table-of-contents levels */
		TocLinks	VALUE(1),	/* Generate links */
		TocNumbers	VALUE(0),	/* Generate heading numbers */
		TocDocCount	VALUE(0);	/* Number of chapters */
VAR int		OutputType	VALUE(HD_OUTPUT_BOOK);
						/* Output a "book", etc. */
VAR char	OutputPath[1024] VALUE("");	/* Output directory/name */
VAR int		OutputFiles	VALUE(0),	/* Generate multiple files? */
		OutputColor	VALUE(1);	/* Output color images */
VAR int		OutputJPEG	VALUE(0);	/* JPEG compress images? */
VAR int		PDFVersion	VALUE(13);	/* Version of PDF to support */
VAR int		PDFPageMode	VALUE(HD_PDF_OUTLINE),
						/* PageMode attribute */
		PDFPageLayout	VALUE(HD_PDF_SINGLE),
						/* PageLayout attribute */
		PDFFirstPage	VALUE(HD_PDF_CHAPTER_1),
						/* First page */
		PDFEffect	VALUE(HD_PDF_NONE);/* Page transition effect */
VAR float	PDFEffectDuration VALUE(1.0),	/* Page effect duration */
		PDFPageDuration	VALUE(10.0);	/* Page duration */
VAR int		Encryption	VALUE(0),	/* Encrypt the PDF file? */
		Permissions	VALUE(-4);	/* File permissions? */
VAR char	OwnerPassword[33] VALUE(""),	/* Owner password */
		UserPassword[33] VALUE("");	/* User password */
VAR int		EmbedFonts	VALUE(0);	/* Embed fonts? */
VAR int		PSLevel		VALUE(2),	/* Language level (0 for PDF) */
		PSCommands	VALUE(0),	/* Output PostScript commands? */
		XRXComments	VALUE(0);	/* Output Xerox comments? */
VAR int		PageWidth	VALUE(595),	/* Page width in points */
		PageLength	VALUE(792),	/* Page length in points */
		PageLeft	VALUE(72),	/* Left margin */
		PageRight	VALUE(36),	/* Right margin */
		PageTop		VALUE(36),	/* Top margin */
		PageBottom	VALUE(36),	/* Bottom margin */
		PagePrintWidth,			/* Printable width */
		PagePrintLength,		/* Printable length */
		PageDuplex	VALUE(0),	/* Adjust margins/pages for duplexing? */
		Landscape	VALUE(0),	/* Landscape orientation? */
		NumberUp	VALUE(1);	/* Number-up pages */

VAR char	*Header[3]	NULL3,		/* Header for regular pages */
		*Header1[3]	NULL3,		/* Header for first pages */
		*TocHeader[3]	NULL3,		/* Header for TOC pages */
		*Footer[3]	NULL3,		/* Regular page footer */
		*TocFooter[3]	NULL3,		/* Footer for TOC pages */
		TocTitle[1024]	VALUE("Table of Contents"),
						/* TOC title string */
		IndexTitle[1024] VALUE("Index"),
						/* Index title string */
		TitleFile[1024]	VALUE("");	/* Title page file */

VAR hdImage	*TitleImage	VALUE(NULL),	/* Title page image */
		*LogoImage	VALUE(NULL),	/* Logo image */
		*BodyImage	VALUE(NULL),	/* Body image */
		*HFImage[MAX_HF_IMAGES]		/* Header/footer images */
#  ifdef _HTMLDOC_CXX_
= { NULL }
#  endif /* _HTMLDOC_CXX_ */
;

VAR char	BodyColor[255]	VALUE(""),	/* Body color */
		LinkColor[255]	VALUE("");	/* Link color */

VAR int		LinkStyle	VALUE(1);	/* 1 = underline, 0 = plain */
VAR int		Links		VALUE(1);	/* 1 = generate links, 0 = no links */
VAR char	Path[2048]	VALUE(""),	/* Search path */
		Proxy[1024]	VALUE(""),	/* Proxy URL */
		Words[1024]	VALUE("");	/* Word/phrase list file */

VAR const char	*PDFModes[3]			/* Mode strings */
#  ifdef _HTMLDOC_CXX_
= { "document", "outline", "fullscreen" }
#  endif /* _HTMLDOC_CXX_ */
;
VAR const char	*PDFLayouts[4]			/* Layout strings */
#  ifdef _HTMLDOC_CXX_
= { "single", "one", "twoleft", "tworight" }
#  endif /* _HTMLDOC_CXX_ */
;
VAR const char	*PDFPages[3]			/* First page strings */
#  ifdef _HTMLDOC_CXX_
= { "p1", "toc", "c1" }
#  endif /* _HTMLDOC_CXX_ */
;
VAR const char	*PDFEffects[17]			/* Effect strings */
#  ifdef _HTMLDOC_CXX_
= { "none", "bi", "bo", "d", "gd", "gdr", "gr", "hb", "hsi", "hso",
    "vb", "vsi", "vso", "wd", "wl", "wr", "wu" }
#  endif /* _HTMLDOC_CXX_ */
;


//
// Prototypes...
//

extern int	pspdf_export(hdTree *document, hdTree *toc, hdTree *idx);

extern int	html_export(hdTree *document, hdTree *toc, hdTree *idx);

extern int	htmlsep_export(hdTree *document, hdTree *toc, hdTree *idx);

extern hdTree	*toc_build(hdTree *tree, hdTree *ind);

extern hdTree	*index_build(hdTree *doc, const char *words);

extern void	get_color(const hdChar *c, float *rgb, int defblack = 1);
extern const char *get_fmt(char **formats);
extern void	get_format(const char *fmt, char **formats);
extern int	get_measurement(const char *s, float mul = 1.0f);
extern void	set_page_size(const char *size);

extern void	prefs_load(void);
extern void	prefs_save(void);
extern void	prefs_set_paths(void);

extern char	*format_number(int n, char f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

//
// End of "$Id$".
//
