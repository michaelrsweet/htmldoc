//
// "$Id: book.h,v 1.3 2004/04/03 03:18:32 mike Exp $"
//
//   Common definitions for HTMLDOC, a HTML document processing program.
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

#ifndef HTMLDOC_BOOK_H
#  define HTMLDOC_BOOK_H


//
// Include necessary headers...
//

#  include "html.h"


//
// Error codes (in addition to the HTTP status codes...)
//

enum hdError
{
  HD_ERROR_NONE = 0,
  HD_ERROR_NO_FILES,
  HD_ERROR_NO_PAGES,
  HD_ERROR_TOO_MANY_CHAPTERS,
  HD_ERROR_OUT_OF_MEMORY,
  HD_ERROR_FILE_NOT_FOUND,
  HD_ERROR_BAD_COMMENT,
  HD_ERROR_BAD_FORMAT,
  HD_ERROR_DELETE_ERROR,
  HD_ERROR_INTERNAL_ERROR,
  HD_ERROR_NETWORK_ERROR,
  HD_ERROR_READ_ERROR,
  HD_ERROR_WRITE_ERROR,
  HD_ERROR_HTML_ERROR,
  HD_ERROR_CONTENT_TOO_LARGE,
  HD_ERROR_UNRESOLVED_LINK,
  HD_ERROR_BAD_HF_STRING,
  HD_ERROR_CSS_ERROR,
  HD_ERROR_HTTPBASE = 100
};

/*
 * Output type...
 */

enum
{
  HD_OUTPUT_BOOK,
  HD_OUTPUT_CONTINUOUS,
  HD_OUTPUT_WEBPAGES
};

enum
{
  HD_OUTPUT_HTML,
  HD_OUTPUT_HTMLSEP,
  HD_OUTPUT_PDF,
  HD_OUTPUT_PS
};


/*
 * PDF constants...
 */

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
// Named link structure...
//

struct hdLink
{
  int		page,			// Page link appears on
		top;			// Y position of link
  uchar		*filename;		// File for link
  uchar		name[124];		// Reference name
};


//
// Named page size information...
//

struct hdPageSize
{
  const char	*name;			// Name of page size
  float		width,			// Width of page in points
		length;			// Length of page in points

  void	set(const char *n, float w, float l);
  void	clear();
};


//
// Entity information...
//

struct hdEntity
{
  const char	*html;			// HTML entity name
  const char	*glyph;			// PostScript glyph name

  void	set(const char *h, const char *g);
  void	clear();
};


//
// The hdBook structure contains common global data and structures
// for a book...
//

struct hdBook
{
  // Data for this book...
  int		error_count;		// Number of errors
  bool		strict_html;		// Strict HTML checking?
  bool		progress_visible;	// Progress visible?
  int		verbosity;		// Verbosity level

  int		num_sizes;		// Number of sizes in table
  hdPageSize	*sizes;			// Size table

  int		num_entities;		// Number of entities in table
  hdEntity	*entities;		// Entity table

  // Heading strings used for filenames...
  int		num_headings,		// Number of headings
		alloc_headings;		// Allocated headings
  uchar		**headings;		// Heading strings

  int		num_links,		// Number of links
		alloc_links;		// Allocated links
  hdLink	*links;			// Links

  int		Compression;		// Non-zero means compress PDFs
  bool		TitlePage,		// Need a title page
		TocLinks,		// Generate links
		TocNumbers;		// Generate heading numbers
  int		TocLevels,		// Number of table-of-contents levels
		TocDocCount;		// Number of chapters
  int		OutputFormat;		// HTML, PDF, etc.
  int		OutputType;		// Output a "book", etc.
  char		OutputPath[1024];	// Output directory/name
  bool		OutputFiles,		// Generate multiple files?
		OutputColor;		// Output color images
  int		OutputJPEG;		// JPEG compress images?
  int		PDFVersion;		// Version of PDF to support
  int		PDFPageMode,		// PageMode attribute
		PDFPageLayout,		// PageLayout attribute
		PDFFirstPage,		// First page
		PDFEffect;		// Page transition effect
  float		PDFEffectDuration,	// Page effect duration
		PDFPageDuration;	// Page duration
  bool		Encryption;		// Encrypt the PDF file?
  int		Permissions;		// File permissions?
  char		OwnerPassword[33],	// Owner password
		UserPassword[33];	// User password
  bool		EmbedFonts;		// Embed fonts?
  int		PSLevel;		// Language level (0 for PDF)
  bool		PSCommands,		// Output PostScript commands?
		XRXComments;		// Output Xerox comments?
  int		PageWidth,		// Page width in points
		PageLength,		// Page length in points
		PageLeft,		// Left margin
		PageRight,		// Right margin
		PageTop,		// Top margin
		PageBottom,		// Bottom margin
		PagePrintWidth,		// Printable width
		PagePrintLength;	// Printable length
  bool		PageDuplex,		// Adjust margins/pages for duplexing?
		Landscape;		// Landscape orientation?
  int		NumberUp;		// Number-up pages

  hdFontFace	HeadFootType;		// Typeface for header & footer
  hdFontInternal HeadFootStyle;		// Type style
  float		HeadFootSize;		// Size of header & footer

  char		*Header[3],		// Header for regular pages
		*TocHeader[3],		// Header for TOC pages
		*Footer[3],		// Regular page footer
		*TocFooter[3],		// Footer for TOC pages
		TocTitle[1024];		// TOC title string

  char		TitleImage[1024],	// Title page image
		LogoImage[1024],	// Logo image
		BodyColor[255],		// Body color
		BodyImage[1024],	// Body image
		LinkColor[255];		// Link color

  char		HFImage[MAX_HF_IMAGES][1024];
					// Header/footer images
  bool		LinkStyle,		// true = underline, false = plain
		Links;			// true = generate links, false = no links
  char		Path[2048],		// Search path
		Proxy[1024];		// Proxy URL

  static const char *datadir;		// Directory for data files
  static const char * const PDFModes[3],// Mode strings
	* const	PDFLayouts[4],		// Layout strings
	* const	PDFPages[3],		// First page strings
	* const	PDFEffects[17];		// Effect strings

  // Functions...
  hdBook();
  ~hdBook();

  const char	*find_entity(const char *g);
  const char	*find_glyph(const char *h);
  hdPageSize	*find_size(const char *n);
  hdPageSize	*find_size(float w, float l);
  char		*format_number(char *s, int slen, char format, int number);
  void		load_entities();
  void		load_sizes();

  void		progress_debug(const char *format, ...);
  void		progress_error(hdError error, const char *format, ...);
  void		progress_hide(void);
  void		progress_show(const char *format, ...);
  void		progress_update(int percent);

  int		pspdf_export(hdTree *document, hdTree *toc);

  int		html_export(hdTree *document, hdTree *toc);

  int		htmlsep_export(hdTree *document, hdTree *toc);

  hdTree	*toc_build(hdTree *tree);

  void		get_color(const uchar *c, float *rgb, int defblack = 1);
  const char	*get_fmt(char **formats);
  void		get_format(const char *fmt, char **formats);
  int		get_measurement(const char *s, float mul = 1.0f);
  void		set_page_size(const char *size);

  const char	*prefs_getrc(char *s, int slen);
  void		prefs_load(void);
  void		prefs_save(void);

  char		*format_number(int n, char f);

  void		html_scan_links(hdTree *t, uchar *filename);
  void		html_update_links(hdTree *t, uchar *filename);

  void		html_header(FILE **out, uchar *filename, uchar *title,
		     	    uchar *author, uchar *copyright,
			    uchar *docnumber, hdTree *t);
  void		html_footer(FILE **out, hdTree *t);
  void		html_title(FILE *out, uchar *title, uchar *author,
		           uchar *copyright, uchar *docnumber);
  int		html_write(FILE *out, hdTree *t, int col);

  void		htmlsep_header(FILE **out, uchar *filename, uchar *title,
		               uchar *author, uchar *copyright,
			       uchar *docnumber, int heading);
  void		htmlsep_footer(FILE **out, int heading);
  void		htmlsep_title(FILE *out, uchar *title, uchar *author,
		              uchar *copyright, uchar *docnumber);
  int		htmlsep_write(FILE *out, hdTree *t, int col);
  int		htmlsep_doc(FILE **out, hdTree *t, int col, int *heading,
		            uchar *title, uchar *author, uchar *copyright,
			    uchar *docnumber);
  int		htmlsep_node(FILE *out, hdTree *t, int col);
  int		htmlsep_nodeclose(FILE *out, hdTree *t, int col);

  void		htmlsep_scan_links(hdTree *t);
  void		htmlsep_update_links(hdTree *t, int *heading);

  uchar		*get_title(hdTree *doc);

  void		add_heading(hdTree *t);

  void		add_link(uchar *name, uchar *filename, int page = 0, int top = 0);
  static int	compare_links(hdLink *n1, hdLink *n2);
  hdLink	*find_link(uchar *name, uchar *filename = 0);

};

#endif // !HTMLDOC_BOOK_H

//
// End of "$Id: book.h,v 1.3 2004/04/03 03:18:32 mike Exp $".
//
