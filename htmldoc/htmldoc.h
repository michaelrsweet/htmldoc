/*
 * "$Id: htmldoc.h,v 1.2 1999/11/08 18:35:17 mike Exp $"
 *
 *   Header file for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 */

/*
 * Include necessary headers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "html.h"
#include "image.h"
#include "debug.h"

#ifdef HAVE_LIBFLTK
#  include "gui.h"
#endif /* HAVE_LIBFLTK */

#ifdef WIN32	    /* Include all 8 million Windows header files... */
#  include <windows.h>
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * Macro to get rid of "unreferenced variable xyz" warnings...
 */

#define REF(x)		(void)x;


/*
 * Globals...
 */

#ifdef _HTMLDOC_C_
#  define VAR
#  define VALUE(x)	=x
#else
#  define VAR		extern
#  define VALUE(x)
#endif /* _HTML_DOC_C */

VAR int		Verbosity	VALUE(0);	/* Verbosity */
VAR int		Compression	VALUE(1);	/* Non-zero means compress PDFs */
VAR int		TitlePage	VALUE(1),	/* Need a title page */
		TocLevels	VALUE(3),	/* Number of table-of-contents levels */
		TocLinks	VALUE(1),	/* Generate links */
		TocNumbers	VALUE(0),	/* Generate heading numbers */
		TocDocCount	VALUE(0);	/* Number of chapters */
VAR char	OutputPath[255]	VALUE("");	/* Output directory/name */
VAR int		OutputFiles	VALUE(0),	/* Generate multiple files? */
		OutputColor	VALUE(1);	/* Output color images */
VAR int		OutputJPEG	VALUE(0);	/* JPEG compress images? */
VAR float	PDFVersion	VALUE(1.2);	/* Version of PDF to support */

VAR int		PageWidth	VALUE(595),	/* Page width in points */
		PageLength	VALUE(792),	/* Page length in points */
		PageLeft	VALUE(72),	/* Left margin */
		PageRight	VALUE(36),	/* Right margin */
		PageTop		VALUE(36),	/* Top margin */
		PageBottom	VALUE(36),	/* Bottom margin */
		PagePrintWidth,			/* Printable width */
		PagePrintLength,		/* Printable length */
		PageDuplex	VALUE(0);	/* Adjust margins/pages for duplexing? */
VAR typeface_t	HeadFootFont	VALUE(TYPE_HELVETICA);
						/* Font for header & footer */
VAR float	HeadFootSize	VALUE(11.0f);	/* Size of header & footer */
VAR char	Header[4]	VALUE(".t."),	/* Header for regular pages */
		TocHeader[4]	VALUE(".t."),	/* Header for TOC pages */
		Footer[4] 	VALUE("h.1"),	/* Regular page footer */
		TocFooter[4]	VALUE("..i");	/* Footer for TOC pages */
VAR char	TitleImage[255]	VALUE("");	/* Title page image */
VAR char	LogoImage[255]	VALUE("");	/* Logo image */
VAR char	BodyColor[255]	VALUE(""),	/* Body color */
		BodyImage[255]	VALUE("");	/* Body image */
#ifdef HAVE_LIBFLTK
VAR GUI		*BookGUI	VALUE(NULL);	/* GUI for book files */
#  ifdef WIN32					/* Editor for HTML files */
VAR char	HTMLEditor[1024] VALUE("notepad.exe %s");
#  else
VAR char	HTMLEditor[1024] VALUE("nedit %s");
#  endif /* WIN32 */
#endif /* HAVE_LIBFLTK */

/*
 * Prototypes...
 */

extern int	pdf_export(tree_t *document, tree_t *toc);

extern int	ps_export_level1(tree_t *document, tree_t *toc);

extern int	ps_export_level2(tree_t *document, tree_t *toc);

extern int	html_export(tree_t *document, tree_t *toc);

extern tree_t	*toc_build(tree_t *tree);

extern int	get_measurement(char *s);
extern void	set_page_size(char *size);

extern void	progress_show(char *format, ...);
extern void	progress_hide(void);
extern void	progress_update(int percent);

extern void	prefs_load(void);
extern void	prefs_save(void);

extern char	*format_number(int n, char f);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

/*
 * End of "$Id: htmldoc.h,v 1.2 1999/11/08 18:35:17 mike Exp $".
 */
