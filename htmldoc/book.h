//
// "$Id$"
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
#  include "http.h"
#  include "image.h"
#  include "margin.h"
#  include "md5.h"
#  include "rc4.h"
#  include <zlib.h>

extern "C" {		/* Workaround for JPEG header problems... */
#  include <jpeglib.h>	/* JPEG/JFIF image definitions */
}



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


enum					// Render types
{
  HD_RENDER_TEXT = 1,			// Text fragment
  HD_RENDER_IMAGE = 2,			// Image
  HD_RENDER_BOX = 4,			// Box
  HD_RENDER_LINK = 8,			// Hyperlink
  HD_RENDER_BG = 16			// Background image
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


struct hdRender				/**** Render entity structure ****/
{
  struct hdRender *prev;		/* Previous rendering entity */
  struct hdRender *next;		/* Next rendering entity */
  int		type;			/* Type of entity */
  float		x,			/* Position in points */
		y,			/* ... */
		width,			/* Size in points */
		height;			/* ... */
  union
  {
    struct
    {
      int	typeface,		/* Typeface for text */
		style;			/* Style of text */
      float	size;			/* Size of text in points */
      float	spacing;		/* Inter-character spacing */
      float	rgb[3];			/* Color of text */
      uchar	buffer[1];		/* String buffer */
    }   	text;
    image_t	*image;			/* Image pointer */
    float	box[3];			/* Box color */
    uchar	link[1];		/* Link URL */
  }		data;
};


struct hdPage				//// Page information
{
  int		width,			// Width of page in points
		length,			// Length of page in points
		left,			// Left margin in points
		right,			// Right margin in points
		top,			// Top margin in points
		bottom,			// Bottom margin in points
		duplex,			// Duplex this page?
		landscape;		// Landscape orientation?
  hdRender	*start,			// First render element
		*end;			// Last render element
  uchar		*chapter,		// Chapter text
		*heading;		// Heading text
  hdTree	*headnode;		// Heading node
  uchar		*header[3],		// Headers
		*footer[3];		// Footers
  char		media_color[64],	// Media color
		media_type[64];		// Media type
  int		media_position;		// Media position
  char		page_text[64];		// Page number for TOC
  image_t	*background_image;	// Background image
  float		background_color[3];	// Background color

  // Number-up support
  int		nup;			// Number up pages
  int		outpage;		// Output page #
  float		outmatrix[2][3];	// Transform matrix
};


struct hdOutPage			//// Output page info
{
  int		nup;			// Number up pages
  int		pages[16];		// Pages on this output page
  int		annot_object;		// Annotation object
};


struct hdFileCache			//// Cache for all temporary files
{
  char	*name;				// Temporary filename
  char	*url;				// URL
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
  bool		overflow_errors;	// Show errors on overflow?
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
  int		*heading_pages,		// Page for headings
		*heading_tops;		// Top position for headings

  int		num_images,		// Number of images in cache
		alloc_images;		// Allocated images
  image_t	**images;		// Images in cache

  int		num_links,		// Number of links
		alloc_links;		// Allocated links
  hdLink	*links;			// Links

  time_t	doc_time;		// Current time
  struct tm	*doc_date;		// Current date

  int		title_page;
  int		chapter,
		chapter_outstarts[MAX_CHAPTERS],
		chapter_outends[MAX_CHAPTERS],
		chapter_starts[MAX_CHAPTERS],
		chapter_ends[MAX_CHAPTERS];

  int		num_pages,
		alloc_pages;
  hdPage	*pages;
  hdTree	*current_heading;

  int		num_outpages;
  hdOutPage	*outpages;

  uchar		list_types[16];
  int		list_values[16];

  char		stdout_filename[256];
  int		num_objects,
		alloc_objects,
		*objects,
		root_object,
		info_object,
		outline_object,
		pages_object,
		names_object,
		encrypt_object,
		font_objects[16];

  uchar		*doc_title;
  image_t	*logo_image;
  float		logo_width,
		logo_height;

  image_t	*hfimage[MAX_HF_IMAGES];
  float		hfimage_width[MAX_HF_IMAGES],
		hfimage_height[MAX_HF_IMAGES];
  float		maxhfheight;

  image_t	*background_image;
  float		background_color[3],
		link_color[3];

  int		render_typeface,
		render_style;
  float		render_size,
		render_rgb[3],
		render_x,
		render_y,
		render_startx,
		render_spacing;

  int		compressor_active;
  z_stream	compressor;
  uchar		comp_buffer[8192];
  uchar		encrypt_key[16];
  int		encrypt_len;
  rc4_context_t	encrypt_state;
  md5_byte_t	file_id[16];

  int		pdf_stream_length;
  int		pdf_stream_start;
  int		pdf_object_type;

  FILE		*jpg_file;	/* JPEG file */
  uchar		jpg_buf[8192];	/* JPEG buffer */
  jpeg_destination_mgr	jpg_dest;	/* JPEG destination manager */
  struct jpeg_error_mgr	jerr;		/* JPEG error handler */

  int		heading_numbers[15];
  uchar		heading_types[15];
  int		last_level;
  hdTree	*heading_parents[15];

  bool		CGIMode;		// Running as CGI?
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

  char		proxy_host[HTTP_MAX_URI];
					// Proxy hostname
  int		proxy_port;		// Proxy port
  http_t	*http;			// Connection to remote server
  int		web_files,		// Number of temporary files
		web_alloc;		// Number of allocated files
  hdFileCache	*web_cache;		// Cache array
  int		no_local;		// Non-zero to disable local files
  char		cookies[1024];		// HTTP cookies, if any

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

  void		toc_add_heading(hdTree *toc, hdTree *heading);
  hdTree	*toc_build(hdTree *tree);
  void		toc_parse_tree(hdTree *t);

  void		get_color(const uchar *c, float *rgb, int defblack = 1);
  const char	*get_fmt(char **formats);
  void		get_format(const char *fmt, char **formats);
  int		get_measurement(const char *s, float mul = 1.0f);
  void		set_page_size(const char *size);
  void		set_permissions(const char *p);

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

  void		pspdf_debug_stats();

  void		pspdf_transform_coords(hdPage *p, float &x, float  &y);
  void		pspdf_transform_page(int outpage, int pos, int page);

  void		pspdf_prepare_outpages();
  void		pspdf_prepare_page(int page);
  void		pspdf_prepare_heading(int page, int print_page, uchar **format,
		                      int y, char *hdPageext, int page_len,
				      int render_heading = 1);
  void		ps_write_document(uchar *author, uchar *creator,
		                  uchar *copyright, uchar *keywords,
				  uchar *subject);
  void		ps_write_outpage(FILE *out, int outpage);
  void		ps_write_page(FILE *out, int page);
  void		ps_write_background(FILE *out);
  void		pdf_write_document(uchar *author, uchar *creator,
		                   uchar *copyright, uchar *keywords,
				   uchar *subject, hdTree *doc, hdTree *toc);
  void		pdf_write_outpage(FILE *out, int outpage);
  void		pdf_write_page(FILE *out, int page);
  void		pdf_write_resources(FILE *out, int page);
  void		pdf_write_contents(FILE *out, hdTree *toc, int parent,
		                   int prev, int next, int *heading);
  void		pdf_write_files(FILE *out, hdTree *doc);
  void		pdf_write_links(FILE *out);
  void		pdf_write_names(FILE *out);
  int		pdf_count_headings(hdTree *toc);

  int		pdf_start_object(FILE *out, int array = 0);
  void		pdf_start_stream(FILE *out);
  void		pdf_end_object(FILE *out);

  void		encrypt_init(void);
  void		flate_open_stream(FILE *out);
  void		flate_close_stream(FILE *out);
  void		flate_puts(const char *s, FILE *out);
  void		flate_printf(FILE *out, const char *format, ...);
  void		flate_write(FILE *out, uchar *inbuf, int length, int flush=0);	

  void		render_contents(hdTree *t, hdMargin *margins, float *y,
  		                int *page, int heading,	hdTree *chap);
  int		count_headings(hdTree *t);
  void		parse_contents(hdTree *t, hdMargin *margins, float *y,
		               int *page, int *heading, hdTree *chap);
  void		parse_doc(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			  hdTree *cpara, int *needspace);
  void		parse_heading(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			      int needspace);
  void		parse_paragraph(hdTree *t, hdMargin *margins, float *x,
		                float *y, int *page, int needspace);
  void		parse_pre(hdTree *t, hdMargin *margins, float *x, float *y,
		          int *page, int needspace);
  void		parse_table(hdTree *t, hdMargin *margins, float *x, float *y,
		            int *page, int needspace);
  void		parse_list(hdTree *t, hdMargin *margins, float *x, float *y,
		           int *page, int needspace);
  void		init_list(hdTree *t);
  void		parse_comment(hdTree *t, hdMargin *margins, float *x, float *y,
		              int *page, hdTree *para, int needspace);

  hdTree	*real_prev(hdTree *t);
  hdTree	*real_next(hdTree *t);

  void		check_pages(int page);

  void		find_background(hdTree *t);
  void		write_background(int page, FILE *out);

  hdRender	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data,
			    hdRender *insert = 0);
  void		copy_tree(hdTree *parent, hdTree *t);
  float		get_cell_size(hdTree *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
  float		get_table_size(hdTree *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
  hdTree	*flatten_tree(hdTree *t);
  float		get_width(uchar *s, int typeface, int style, float size);
  void		update_image_size(hdTree *t);
  FILE		*pspdf_open_file(void);
  void		set_color(FILE *out, float *rgb);
  void		set_font(FILE *out, int typeface, int style, float size);
  void		set_pos(FILE *out, float x, float y);
  void		write_prolog(FILE *out, int pages, uchar *author,
		             uchar *creator, uchar *copyright,
			     uchar *keywords, uchar *subject);
  void		ps_hex(FILE *out, uchar *data, int length);
  void		ps_ascii85(FILE *out, uchar *data, int length, int eod = 0);
  static void	jpg_init(j_compress_ptr cinfo);
  static boolean jpg_empty(j_compress_ptr cinfo);
  static void	jpg_term(j_compress_ptr cinfo);
  void		jpg_setup(FILE *out, image_t *img, j_compress_ptr cinfo);
  static int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
  void		write_image(FILE *out, hdRender *r, int write_obj = 0);
  void		write_imagemask(FILE *out, hdRender *r);
  void		write_string(FILE *out, uchar *s, int compress);
  void		write_text(FILE *out, hdRender *r);
  void		write_trailer(FILE *out, int pages);
  int		write_type1(FILE *out, hdFontFace typeface,
			    hdFontInternal style);

  static const char	*file_basename(const char *s);
  void			file_cleanup(void);
  void			file_cookies(const char *s);
  static const char	*file_directory(const char *s);
  static const char	*file_extension(const char *s);
  const char		*file_find(const char *path, const char *s);
  const char		*file_find_check(const char *filename);
  static char		*file_gets(char *buf, int buflen, FILE *fp);
  static const char	*file_localize(const char *filename, const char *newcwd);
  static const char	*file_method(const char *s);
  void			file_nolocal();
  void			file_proxy(const char *url);
  static const char	*file_target(const char *s);
  FILE			*file_temp(char *name, size_t len);

  static int	image_compare(image_t **img1, image_t **img2);
  void		image_copy(const char *filename, const char *destpath);
  image_t	*image_find(const char *filename, int load_data = 0);
  void		image_flush_cache(void);
  int		image_getlist(image_t ***ptrs);
  image_t	*image_load(const char *filename, int gray, int load_data = 0);
  void		image_unload(image_t *img);
};

#endif // !HTMLDOC_BOOK_H

//
// End of "$Id$".
//
