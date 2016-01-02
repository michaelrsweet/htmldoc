/*
 * "$Id$"
 *
 * PostScript + PDF output routines for HTMLDOC, a HTML document processing
 * program.
 *
 * Just in case you didn't notice it, this file is too big; it will be
 * broken into more manageable pieces once we make all of the output
 * "drivers" into classes...
 *
 * Copyright 2011-2016 by Michael R Sweet.
 * Copyright 1997-2010 by Easy Software Products.  All rights reserved.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING.txt".
 */

/*
 * Include necessary headers.
 */

/*
 * The GCC compiler on HP-UX has a nasty habit of incorrectly "fixing"
 * the vmtypes.h header file provided with HP-UX.  The following
 * conditional magic makes sure that "page_t" (which we use in our
 * code) is not defined...
 */

#ifdef __hpux
#  define page_t	hpux_page_t
#endif // __hpux

/*#define DEBUG*/
#include "htmldoc.h"
#include "md5-private.h"
#define md5_append _cupsMD5Append
#define md5_finish _cupsMD5Finish
#define md5_init _cupsMD5Init
typedef unsigned char md5_byte_t;
#define md5_state_t _cups_md5_state_t
#include "rc4.h"
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#ifdef WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif // WIN32

#include <fcntl.h>

#include <zlib.h>

extern "C" {		/* Workaround for JPEG header problems... */
#include <jpeglib.h>	/* JPEG/JFIF image definitions */
}

#ifdef __hpux
#  undef page_t
#endif // __hpux


/*
 * Output options...
 */

#define HTMLDOC_ASCII85
//#define HTMLDOC_INTERPOLATION


/*
 * Constants...
 */

#define RENDER_TEXT	0		/* Text fragment */
#define RENDER_IMAGE	1		/* Image */
#define RENDER_BOX	2		/* Box */
#define RENDER_LINK	3		/* Hyperlink */
#define RENDER_BG	4		/* Background image */


/*
 * Structures...
 */

typedef struct render_str		/**** Render entity structure ****/
{
  struct render_str	*prev;		/* Previous rendering entity */
  struct render_str	*next;		/* Next rendering entity */
  int	type;				/* Type of entity */
  float	x,				/* Position in points */
	y,				/* ... */
	width,				/* Size in points */
	height;				/* ... */
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
  }	data;
} render_t;

typedef struct				/**** Named link position structure */
{
  short		page,			/* Page # */
		top;			/* Top position */
  uchar		name[124];		/* Reference name */
} link_t;

typedef struct				//// Page information
{
  int		width,			// Width of page in points
		length,			// Length of page in points
		left,			// Left margin in points
		right,			// Right margin in points
		top,			// Top margin in points
		bottom,			// Bottom margin in points
		duplex,			// Duplex this page?
		landscape;		// Landscape orientation?
  render_t	*start,			// First render element
		*end;			// Last render element
  uchar		*chapter,		// Chapter text
		*heading;		// Heading text
  tree_t	*headnode;		// Heading node
  uchar		*header[3],		// Headers for regular pages
		*header1[3],		// Headers for first pages
		*footer[3];		// Footers for all pages
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
} page_t;

typedef struct				//// Output page info
{
  int		nup;			// Number up pages
  int		pages[16];		// Pages on this output page
  int		annot_object;		// Annotation object
} outpage_t;


/*
 * Timezone offset for dates, below...
 */

#ifdef HAVE_TM_GMTOFF
#  define timezone (doc_date->tm_gmtoff)
#elif defined(__CYGWIN__)
#  define timezone _timezone
#endif /* HAVE_TM_GMTOFF */


/*
 * Local globals...
 */

static time_t	doc_time;		// Current time
static struct tm *doc_date;		// Current date

static int	title_page;
static int	chapter,
		chapter_outstarts[MAX_CHAPTERS],
		chapter_outends[MAX_CHAPTERS],
		chapter_starts[MAX_CHAPTERS],
		chapter_ends[MAX_CHAPTERS];

static int	num_headings = 0,
		alloc_headings = 0,
		*heading_pages = NULL,
		*heading_tops = NULL;

static int	num_pages = 0,
		alloc_pages = 0;
static page_t	*pages = NULL;
static tree_t	*current_heading;

static int	num_outpages = 0;
static outpage_t *outpages = NULL;

static int	num_links = 0,
		alloc_links = 0;
static link_t	*links = NULL;

static uchar	list_types[16];
static int	list_values[16];

static char	stdout_filename[256];
static int	num_objects = 0,
		alloc_objects = 0,
		*objects = NULL,
		root_object,
		info_object,
		outline_object,
		pages_object,
		names_object,
		encrypt_object,
		font_objects[TYPE_MAX * STYLE_MAX];

static uchar	*doc_title = NULL;
static image_t	*logo_image = NULL;
static float	logo_width,
		logo_height;

static image_t	*hfimage[MAX_HF_IMAGES];
static float	hfimage_width[MAX_HF_IMAGES],
		hfimage_height[MAX_HF_IMAGES];
static float    maxhfheight;

static image_t	*background_image = NULL;
static float	background_color[3] = { 1.0, 1.0, 1.0 },
		link_color[3] = { 0.0, 0.0, 1.0 };

static int	render_typeface,
		render_style;
static float	render_size,
		render_rgb[3],
		render_x,
		render_y,
		render_startx,
		render_spacing;

static int		compressor_active = 0;
static z_stream		compressor;
static uchar		comp_buffer[8192];
static uchar		encrypt_key[16];
static int		encrypt_len;
static rc4_context_t	encrypt_state;
static md5_byte_t	file_id[16];


/*
 * Local functions...
 */

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static void	pspdf_debug_stats();

static void	pspdf_transform_coords(page_t *p, float &x, float  &y);
static void	pspdf_transform_page(int outpage, int pos, int page);

static void	pspdf_prepare_outpages();
static void	pspdf_prepare_page(int page);
static void	pspdf_prepare_heading(int page, int print_page, uchar **format,
		                      int y, char *page_text, int page_len);
static void	ps_write_document(uchar *author, uchar *creator,
		                  uchar *copyright, uchar *keywords,
				  uchar *subject);
static void	ps_write_outpage(FILE *out, int outpage);
static void	ps_write_page(FILE *out, int page);
static void	ps_write_background(FILE *out);
static void	pdf_write_document(uchar *author, uchar *creator,
		                   uchar *copyright, uchar *keywords,
				   uchar *subject, tree_t *doc, tree_t *toc);
static void	pdf_write_outpage(FILE *out, int outpage);
static void	pdf_write_page(FILE *out, int page);
static void	pdf_write_resources(FILE *out, int page);
#ifdef DEBUG_TOC
static void	pdf_text_contents(FILE *out, tree_t *toc, int indent = 0);
#endif // DEBUG_TOC
static void	pdf_write_contents(FILE *out, tree_t *toc, int parent,
		                   int prev, int next, int *heading);
static void	pdf_write_files(FILE *out, tree_t *doc);
static void	pdf_write_links(FILE *out);
static void	pdf_write_names(FILE *out);
static int	pdf_count_headings(tree_t *toc);

static int	pdf_start_object(FILE *out, int array = 0);
static void	pdf_start_stream(FILE *out);
static void	pdf_end_object(FILE *out);

static void	encrypt_init(void);
static void	flate_open_stream(FILE *out);
static void	flate_close_stream(FILE *out);
static void	flate_puts(const char *s, FILE *out);
static void	flate_printf(FILE *out, const char *format, ...);
static void	flate_write(FILE *out, uchar *inbuf, int length, int flush=0);

static void	parse_contents(tree_t *t, float left, float width, float bottom,
		               float length, float *y, int *page, int *heading,
			       tree_t *chap);
static void	parse_doc(tree_t *t, float *left, float *right, float *bottom,
		          float *top, float *x, float *y, int *page,
			  tree_t *cpara, int *needspace);
static void	parse_heading(tree_t *t, float left, float width, float bottom,
		              float length, float *x, float *y, int *page,
			      int needspace);
static void	parse_paragraph(tree_t *t, float left, float width, float bottom,
		                float length, float *x, float *y, int *page,
			        int needspace);
static void	parse_pre(tree_t *t, float left, float width, float bottom,
		          float length, float *x, float *y, int *page,
			  int needspace);
static void	parse_table(tree_t *t, float left, float width, float bottom,
		            float length, float *x, float *y, int *page,
			    int needspace);
static void	parse_list(tree_t *t, float *left, float *width, float *bottom,
		           float *length, float *x, float *y, int *page,
			   int needspace);
static void	init_list(tree_t *t);
static void	parse_comment(tree_t *t, float *left, float *width, float *bottom,
		              float *length, float *x, float *y, int *page,
			      tree_t *para, int needspace);

static void	check_pages(int page);

static void	add_link(uchar *name, int page, int top);
static link_t	*find_link(uchar *name);
static int	compare_links(link_t *n1, link_t *n2);

static void	find_background(tree_t *t);
static void	write_background(int page, FILE *out);

static render_t	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data,
			    render_t *insert = 0);
static float	get_cell_size(tree_t *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
static float	get_table_size(tree_t *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
static tree_t	*flatten_tree(tree_t *t);
static float	get_width(uchar *s, int typeface, int style, int size);
static void	update_image_size(tree_t *t);
static uchar	*get_title(tree_t *doc);
static FILE	*open_file(void);
static void	set_color(FILE *out, float *rgb);
static void	set_font(FILE *out, int typeface, int style, float size);
static void	set_pos(FILE *out, float x, float y);
static void	write_prolog(FILE *out, int pages, uchar *author,
		             uchar *creator, uchar *copyright,
			     uchar *keywords, uchar *subject);
static void	ps_hex(FILE *out, uchar *data, int length);
#ifdef HTMLDOC_ASCII85
static void	ps_ascii85(FILE *out, uchar *data, int length, int eod = 0);
#endif // HTMLDOC_ASCII85
static void	jpg_init(j_compress_ptr cinfo);
static boolean	jpg_empty(j_compress_ptr cinfo);
static void	jpg_term(j_compress_ptr cinfo);
static void	jpg_setup(FILE *out, image_t *img, j_compress_ptr cinfo);
static int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
static void	write_image(FILE *out, render_t *r, int write_obj = 0);
static void	write_imagemask(FILE *out, render_t *r);
static void	write_string(FILE *out, uchar *s, int compress);
static void	write_text(FILE *out, render_t *r);
static void	write_trailer(FILE *out, int pages);
static int	write_type1(FILE *out, typeface_t typeface,
			    style_t style);
static void	write_utf16(FILE *out, uchar *s);


/*
 * 'pspdf_export()' - Export PostScript/PDF file(s)...
 */

int
pspdf_export(tree_t *document,	/* I - Document to export */
             tree_t *toc)	/* I - Table of contents for document */
{
  int		i, j;		/* Looping vars */
  const char	*title_file;	/* Location of title image/file */
  uchar		*author,	/* Author of document */
		*creator,	/* HTML file creator (Netscape, etc) */
		*copyright,	/* File copyright */
		*docnumber,	/* Document number */
		*keywords,	/* Search keywords */
		*subject;	/* Subject */
  tree_t	*t;		/* Title page document tree */
  FILE		*fp;		/* Title page file */
  float		x, y,		/* Current page position */
		left, right,	/* Left and right margins */
		bottom, top,	/* Bottom and top margins */
		width,		/* Width of , author, etc */
		height;		/* Height of  area */
  int		pos,		/* Current header/footer position */
		page,		/* Current page # */
		heading,	/* Current heading # */
		toc_duplex,	/* Duplex TOC pages? */
		toc_landscape,	/* Do TOC in landscape? */
		toc_width,	/* Width of TOC pages */
		toc_length,	/* Length of TOC pages */
		toc_left,	/* TOC page margins */
		toc_right,
		toc_bottom,
		toc_top;
  image_t	*timage;	/* Title image */
  float		timage_width,	/* Title image width */
		timage_height;	/* Title image height */
  render_t	*r;		/* Rendering structure... */
  float		rgb[3];		/* Text color */
  int		needspace;	/* Need whitespace */


 /*
  * Figure out the printable area of the output page...
  */

  if (Landscape)
  {
    PagePrintWidth  = PageLength - PageLeft - PageRight;
    PagePrintLength = PageWidth - PageTop - PageBottom;
  }
  else
  {
    PagePrintWidth  = PageWidth - PageLeft - PageRight;
    PagePrintLength = PageLength - PageTop - PageBottom;
  }

  toc_width     = PageWidth;
  toc_length    = PageLength;
  toc_left      = PageLeft;
  toc_right     = PageRight;
  toc_bottom    = PageBottom;
  toc_top       = PageTop;
  toc_landscape = Landscape;
  toc_duplex    = PageDuplex;

 /*
  * Get the document title, author, etc...
  */

  doc_title   = get_title(document);
  author      = htmlGetMeta(document, (uchar *)"author");
  creator     = htmlGetMeta(document, (uchar *)"generator");
  copyright   = htmlGetMeta(document, (uchar *)"copyright");
  docnumber   = htmlGetMeta(document, (uchar *)"docnumber");
  keywords    = htmlGetMeta(document, (uchar *)"keywords");
  subject     = htmlGetMeta(document, (uchar *)"subject");
  logo_image  = image_load(LogoImage, !OutputColor);
  maxhfheight = 0.0f;

  if (logo_image != NULL)
  {
    logo_width  = logo_image->width * PagePrintWidth / _htmlBrowserWidth;
    logo_height = logo_width * logo_image->height / logo_image->width;

    if (logo_height > maxhfheight)
      maxhfheight = logo_height;
  }
  else
    logo_width = logo_height = 0.0f;

  for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
  {
    hfimage[hfi] = image_load(HFImage[hfi], !OutputColor);

    if (hfimage[hfi])
    {
      hfimage_width[hfi]  = hfimage[hfi]->width * PagePrintWidth /
                            _htmlBrowserWidth;
      hfimage_height[hfi] = hfimage_width[hfi] * hfimage[hfi]->height /
                            hfimage[hfi]->width;

      if (hfimage_height[hfi] > maxhfheight)
        maxhfheight = hfimage_height[hfi];
    }
    else
      hfimage_width[hfi] = hfimage_height[hfi] = 0.0f;
  }

  find_background(document);
  get_color((uchar *)LinkColor, link_color);

 /*
  * Initialize page rendering variables...
  */

  num_pages   = 0;
  alloc_pages = 0;
  pages       = NULL;

  memset(list_types, 0267, sizeof(list_types));
  memset(list_values, 0, sizeof(list_values));
  memset(chapter_starts, -1, sizeof(chapter_starts));
  memset(chapter_ends, -1, sizeof(chapter_starts));

  doc_time       = time(NULL);
  doc_date       = localtime(&doc_time);
  num_headings   = 0;
  alloc_headings = 0;
  heading_pages  = NULL;
  heading_tops   = NULL;
  num_links      = 0;
  alloc_links    = 0;
  links          = NULL;
  num_pages      = 0;

  DEBUG_printf(("pspdf_export: TitlePage = %d, TitleImage = \"%s\"\n",
                TitlePage, TitleImage));

  if (TitlePage)
  {
#ifdef WIN32
    if (TitleImage[0] &&
        stricmp(file_extension(TitleImage), "bmp") != 0 &&
	stricmp(file_extension(TitleImage), "gif") != 0 &&
	stricmp(file_extension(TitleImage), "jpg") != 0 &&
	stricmp(file_extension(TitleImage), "png") != 0)
#else
    if (TitleImage[0] &&
        strcmp(file_extension(TitleImage), "bmp") != 0 &&
	strcmp(file_extension(TitleImage), "gif") != 0 &&
	strcmp(file_extension(TitleImage), "jpg") != 0 &&
	strcmp(file_extension(TitleImage), "png") != 0)
#endif // WIN32
    {
      DEBUG_printf(("pspdf_export: Generating a titlepage using \"%s\"\n",
                    TitleImage));

      // Find the title file...
      if ((title_file = file_find(Path, TitleImage)) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to find title file \"%s\"!", TitleImage);
	return (1);
      }

      // Write a title page from HTML source...
      if ((fp = fopen(title_file, "rb")) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to open title file \"%s\" - %s!",
                       TitleImage, strerror(errno));
	return (1);
      }

      t = htmlReadFile(NULL, fp, file_directory(TitleImage));
      htmlFixLinks(t, t, (uchar *)file_directory(TitleImage));
      fclose(fp);

      page            = 0;
      title_page      = 1;
      current_heading = NULL;
      x               = 0.0f;
      bottom          = 0.0f;
      top             = PagePrintLength;
      y               = top;
      needspace       = 0;
      left            = 0.0f;
      right           = PagePrintWidth;

      parse_doc(t, &left, &right, &bottom, &top, &x, &y, &page, NULL,
                &needspace);

      if (PageDuplex && (num_pages & 1))
	check_pages(num_pages);

      htmlDeleteTree(t);
    }
    else
    {
     /*
      * Create a standard title page...
      */

      if ((timage = image_load(TitleImage, !OutputColor)) != NULL)
      {
	timage_width  = timage->width * PagePrintWidth / _htmlBrowserWidth;
	timage_height = timage_width * timage->height / timage->width;
      }
      else
        timage_width = timage_height = 0.0f;

      check_pages(0);
      if (PageDuplex)
        check_pages(1);

      height = 0.0;

      if (timage != NULL)
	height += timage_height + _htmlSpacings[SIZE_P];
      if (doc_title != NULL)
	height += _htmlSpacings[SIZE_H1] + _htmlSpacings[SIZE_P];
      if (author != NULL)
	height += _htmlSpacings[SIZE_P];
      if (docnumber != NULL)
	height += _htmlSpacings[SIZE_P];
      if (copyright != NULL)
	height += _htmlSpacings[SIZE_P];

      y = 0.5f * (PagePrintLength + height);

      if (timage != NULL)
      {
	new_render(0, RENDER_IMAGE, 0.5f * (PagePrintWidth - timage_width),
                   y - timage_height, timage_width, timage_height, timage);
	y -= timage_height + _htmlSpacings[SIZE_P];
      }

      get_color(_htmlTextColor, rgb);

      if (doc_title != NULL)
      {
	width = get_width(doc_title, _htmlHeadingFont, STYLE_BOLD, SIZE_H1);
	r     = new_render(0, RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - _htmlSpacings[SIZE_H1], width,
			   _htmlSizes[SIZE_H1], doc_title);

	r->data.text.typeface = _htmlHeadingFont;
	r->data.text.style    = STYLE_BOLD;
	r->data.text.size     = _htmlSizes[SIZE_H1];
	memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	y -= _htmlSpacings[SIZE_H1];

	if (docnumber != NULL)
	{
	  width = get_width(docnumber, _htmlBodyFont, STYLE_NORMAL, SIZE_P);
	  r     = new_render(0, RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                             y - _htmlSpacings[SIZE_P], width,
			     _htmlSizes[SIZE_P], docnumber);

	  r->data.text.typeface = _htmlBodyFont;
	  r->data.text.style    = STYLE_NORMAL;
	  r->data.text.size     = _htmlSizes[SIZE_P];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	  y -= _htmlSpacings[SIZE_P];
	}

	y -= _htmlSpacings[SIZE_P];
      }

      if (author != NULL)
      {
	width = get_width(author, _htmlBodyFont, STYLE_NORMAL, SIZE_P);
	r     = new_render(0, RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - _htmlSpacings[SIZE_P], width, _htmlSizes[SIZE_P],
			   author);

	r->data.text.typeface = _htmlBodyFont;
	r->data.text.style    = STYLE_NORMAL;
	r->data.text.size     = _htmlSizes[SIZE_P];
	memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	y -= _htmlSpacings[SIZE_P];
      }

      if (copyright != NULL)
      {
	width = get_width(copyright, _htmlBodyFont, STYLE_NORMAL, SIZE_P);
	r     = new_render(0, RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - _htmlSpacings[SIZE_P], width, _htmlSizes[SIZE_P],
			   copyright);

	r->data.text.typeface = _htmlBodyFont;
	r->data.text.style    = STYLE_NORMAL;
	r->data.text.size     = _htmlSizes[SIZE_P];
	memcpy(r->data.text.rgb, rgb, sizeof(rgb));
      }
    }

    for (page = 0; page < num_pages; page ++)
      // Safe because page_text is more than 6 chars
      strcpy((char *)pages[page].page_text, (page & 1) ? "eltit" : "title");
  }
  else
    page = 0;

 /*
  * Parse the document...
  */

  if (OutputType == OUTPUT_BOOK)
    chapter = 0;
  else
  {
    chapter           = 1;
    TocDocCount       = 1;
    chapter_starts[1] = num_pages;
  }

  title_page      = 0;
  current_heading = NULL;
  x               = 0.0f;
  needspace       = 0;
  left            = 0.0f;
  right           = PagePrintWidth;

  // Adjust top margin as needed...
  float adjust, image_adjust, temp_adjust;

  if (maxhfheight > HeadFootSize)
    image_adjust = maxhfheight + HeadFootSize;
  else
    image_adjust = 2 * HeadFootSize;

  for (adjust = 0.0, pos = 0; pos < 3; pos ++)
  {
    if (Header[pos] &&
        (strstr(Header[pos], "$IMAGE") != NULL ||
	 strstr(Header[pos], "$HFIMAGE") != NULL))
      temp_adjust = image_adjust;
    else if (Header1[pos] &&
	     (strstr(Header1[pos], "$IMAGE") != NULL ||
	      strstr(Header1[pos], "$HFIMAGE") != NULL))
      temp_adjust = image_adjust;
    else if (Header[pos] || Header1[pos])
      temp_adjust = 2 * HeadFootSize;
    else
      temp_adjust = 0.0;

    if (temp_adjust > adjust)
      adjust = temp_adjust;
  }

  top = PagePrintLength - adjust;

  // Adjust bottom margin as needed...
  for (adjust = 0.0, pos = 0; pos < 3; pos ++)
  {
    if (Footer[pos] &&
        (strstr(Footer[pos], "$IMAGE") != NULL ||
	 strstr(Footer[pos], "$HFIMAGE") != NULL))
      temp_adjust = image_adjust;
    else if (Footer[pos])
      temp_adjust = 2 * HeadFootSize;
    else
      temp_adjust = 0.0;

    if (temp_adjust > adjust)
      adjust = temp_adjust;
  }

  bottom = adjust;

  y = top;

  parse_doc(document, &left, &right, &bottom, &top, &x, &y, &page, NULL,
            &needspace);

  if (PageDuplex && (num_pages & 1))
  {
    if (PSLevel == 0)
      chapter_ends[chapter] = num_pages - 1;

    check_pages(num_pages);

    if (PSLevel > 0)
      chapter_ends[chapter] = num_pages - 1;
  }
  else
    chapter_ends[chapter] = num_pages - 1;

  for (chapter = 1; chapter <= TocDocCount; chapter ++)
  {
    for (page = chapter_starts[chapter]; page <= chapter_ends[chapter]; page ++)
    {
      pspdf_prepare_page(page);
      if (chapter == TocDocCount)
        fprintf(stderr, "page %d: %s\n", page, pages[page].page_text);
    }
  }

 /*
  * Parse the table-of-contents if necessary...
  */

  if (TocLevels > 0 && num_headings > 0)
  {
    // Restore default page size, etc...
    PageWidth  = toc_width;
    PageLength = toc_length;
    PageLeft   = toc_left;
    PageRight  = toc_right;
    PageBottom = toc_bottom;
    PageTop    = toc_top;
    Landscape  = toc_landscape;
    PageDuplex = toc_duplex;

    if (Landscape)
    {
      PagePrintWidth  = PageLength - PageLeft - PageRight;
      PagePrintLength = PageWidth - PageTop - PageBottom;
    }
    else
    {
      PagePrintWidth  = PageWidth - PageLeft - PageRight;
      PagePrintLength = PageLength - PageTop - PageBottom;
    }

    // Adjust top margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (TocHeader[pos])
	break;

    if (pos == 3)
      top = PagePrintLength;
    else if (maxhfheight > HeadFootSize)
      top = PagePrintLength - maxhfheight - HeadFootSize;
    else
      top = PagePrintLength - 2 * HeadFootSize;

    // Adjust bottom margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (TocFooter[pos])
	break;

    if (pos == 3)
      bottom = 0.0f;
    else if (maxhfheight > HeadFootSize)
      bottom = maxhfheight + HeadFootSize;
    else
      bottom = 2 * HeadFootSize;

    y                 = 0.0;
    page              = num_pages - 1;
    heading           = 0;
    chapter_starts[0] = num_pages;
    chapter           = 0;

    parse_contents(toc, 0, PagePrintWidth, bottom, top, &y, &page, &heading, 0);
    if (PageDuplex && (num_pages & 1))
      check_pages(num_pages);
    chapter_ends[0] = num_pages - 1;

    for (page = chapter_starts[0]; page <= chapter_ends[0]; page ++)
      pspdf_prepare_page(page);
  }

  if (TocDocCount > MAX_CHAPTERS)
    TocDocCount = MAX_CHAPTERS;

 /*
  * Do we have any pages?
  */

  if (num_pages > 0 && TocDocCount > 0)
  {
   /*
    * Yes, write the document to disk...
    */

    pspdf_prepare_outpages();

    pspdf_debug_stats();

    progress_error(HD_ERROR_NONE, "PAGES: %d", num_outpages);

    if (PSLevel > 0)
      ps_write_document(author, creator, copyright, keywords, subject);
    else
      pdf_write_document(author, creator, copyright, keywords, subject,
                         document, toc);
  }
  else
  {
   /*
    * No, show an error...
    */

    pspdf_debug_stats();

    progress_error(HD_ERROR_NO_PAGES,
                   "Error: no pages generated! (did you remember to use webpage mode?");
  }

 /*
  * Free memory...
  */

  if (doc_title != NULL)
    free(doc_title);

  if (alloc_links)
  {
    free(links);

    num_links    = 0;
    alloc_links  = 0;
    links        = NULL;
  }

  for (i = 0; i < num_pages; i ++)
  {
    if ((i == 0 || pages[i].chapter != pages[i - 1].chapter) &&
        pages[i].chapter)
      free(pages[i].chapter);

    if ((i == 0 || pages[i].heading != pages[i - 1].heading) &&
        pages[i].heading)
      free(pages[i].heading);

    if (!pages[i].heading)
      continue;

    for (j = 0; j < 3; j ++)
    {
      if (!pages[i].header[j])
        continue;

      if (i == 0 || pages[i].header[j] != pages[i - 1].header[j])
        free(pages[i].header[j]);
    }

    for (j = 0; j < 3; j ++)
    {
      if (!pages[i].header1[j])
        continue;

      if (i == 0 || pages[i].header1[j] != pages[i - 1].header1[j])
        free(pages[i].header1[j]);
    }

    for (j = 0; j < 3; j ++)
    {
      if (!pages[i].footer[j])
        continue;

      if (i == 0 || pages[i].footer[j] != pages[i - 1].footer[j])
        free(pages[i].footer[j]);
    }
  }

  for (i = 0; i < 3; i ++)
  {
    Header[i]    = NULL;
    Header1[i]   = NULL;
    Footer[i]    = NULL;
    TocHeader[i] = NULL;
    TocFooter[i] = NULL;
  }

  if (alloc_pages)
  {
    free(pages);
    free(outpages);

    num_pages   = 0;
    alloc_pages = 0;
    pages       = NULL;
  }

  if (alloc_headings)
  {
    free(heading_pages);
    free(heading_tops);

    num_headings   = 0;
    alloc_headings = 0;
    heading_pages  = NULL;
    heading_tops   = NULL;
  }

  return (0);
}



//
// 'pspdf_debug_stats()' - Display debug statistics for render memory use.
//

static void
pspdf_debug_stats()
{
  const char	*debug;			// HTMLDOC_DEBUG env var
  int		i;			// Looping var
  render_t	*r;			// Render node
  int		bytes;			// Number of bytes


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

  bytes = alloc_headings * sizeof(int) * 2;

  bytes += alloc_pages * sizeof(page_t);
  for (i = 0; i < num_pages; i ++)
  {
    for (r = pages[i].start; r != NULL; r = r->next)
    {
      bytes += sizeof(render_t);

      if (r->type == RENDER_TEXT)
        bytes += strlen((char *)r->data.text.buffer);
    }
  }

  bytes += num_outpages * sizeof(outpage_t);
  bytes += alloc_links * sizeof(link_t);
  bytes += alloc_objects * sizeof(int);

  progress_error(HD_ERROR_NONE, "DEBUG: Render Data = %d kbytes",
                 (bytes + 1023) / 1024);
}


/*
 * 'pspdf_transform_coords()' - Transform page coordinates.
 */

static void
pspdf_transform_coords(page_t *p,	// I - Page
                       float  &x,	// IO - X coordinate
		       float  &y)	// IO - Y coordinate
{
  float tx, ty;				// Temporary X and Y


  tx = x;
  ty = y;
  x  = tx * p->outmatrix[0][0] + ty * p->outmatrix[0][1] + p->outmatrix[0][2];
  y  = tx * p->outmatrix[1][0] + ty * p->outmatrix[1][1] + p->outmatrix[1][2];
}


/*
 * 'pspdf_transform_page()' - Transform a page.
 */

static void
pspdf_transform_page(int outpage,	// I - Output page
                     int pos,		// I - Position on page
                     int page)		// I - Input page
{
  outpage_t	*op;			// Current output page
  page_t	*bp;			// Current base page
  page_t	*p;			// Current input page
  int		x, y;			// Position on output page
  float		w, l,			// Width and length of subpage
		tx, ty;			// Translation values for subpage
  float		pw, pl;			// Printable width and length of full page


  DEBUG_printf(("pspdf_transform_page(outpage = %d, pos = %d, page = %d)\n",
                outpage, pos, page));

  if (pos > 15)
    progress_error(HD_ERROR_INTERNAL_ERROR, "Internal error: pos = %d", pos);

  op             = outpages + outpage;
  op->pages[pos] = page;
  bp             = pages + op->pages[0];
  p              = pages + page;
  p->outpage     = outpage;
  pw             = bp->width;
  pl             = bp->length;

  DEBUG_printf(("    width = %d, length = %d\n", p->width, p->length));

  switch (op->nup)
  {
    default :
    case 1 :
        p->outmatrix[0][0] = 1.0f;
        p->outmatrix[1][0] = 0.0f;
        p->outmatrix[0][1] = 0.0f;
        p->outmatrix[1][1] = 1.0f;
        p->outmatrix[0][2] = 0.0f;
        p->outmatrix[1][2] = 0.0f;
	break;

    case 2 :
	x = pos & 1;

        l = pw;
        w = l * p->width / p->length;

        if (w > (pl * 0.5f))
        {
          w = pl * 0.5f;
          l = w * p->length / p->width;
        }

        tx = 0.5 * (pl * 0.5 - w);
        ty = 0.5 * (pw - l);

        p->outmatrix[0][0] = 0.0f;
        p->outmatrix[1][0] = w / p->width;
        p->outmatrix[0][1] = -w / p->width;
        p->outmatrix[1][1] = 0.0f;
        p->outmatrix[0][2] = ty + pl * w / p->width;
        p->outmatrix[1][2] = tx + x * pl / 2;
	break;

    case 4 :
        x = pos & 1;
	y = 1 - pos / 2;

        w = pw * 0.5;
	l = w * p->length / p->width;

	if (l > (pl * 0.5))
	{
	  l = pl * 0.5;
	  w = l * p->width / p->length;
	}

        tx = 0.5 * (pw * 0.5 - w);
        ty = 0.5 * (pl * 0.5 - l);

        p->outmatrix[0][0] = w / p->width;
        p->outmatrix[1][0] = 0.0f;
        p->outmatrix[0][1] = 0.0f;
        p->outmatrix[1][1] = w / p->width;
        p->outmatrix[0][2] = tx + x * pw / 2;
        p->outmatrix[1][2] = ty + y * pl / 2;
	break;

    case 6 :
        x = pos % 3;
	y = pos / 3;

        l = pw * 0.5;
        w = l * p->width / p->length;

        if (w > (pl * 0.333f))
        {
          w = pl * 0.333f;
          l = w * p->length / p->width;
        }

        tx = 0.5 * (pl * 0.333 - w);
        ty = 0.5 * (pw * 0.5 - l);

        p->outmatrix[0][0] = 0.0f;
        p->outmatrix[1][0] = w / p->width;
        p->outmatrix[0][1] = -w / p->width;
        p->outmatrix[1][1] = 0.0f;
        p->outmatrix[0][2] = ty + y * pw / 2 + pl * w / p->width;
        p->outmatrix[1][2] = tx + x * pl / 3;
	break;

    case 9 :
        x = pos % 3;
	y = 2 - pos / 3;

        w = pw * 0.333;
	l = w * p->length / p->width;

	if (l > (pl * 0.333))
	{
	  l = pl * 0.333;
	  w = l * p->width / p->length;
	}

        tx = 0.5 * (pw * 0.333 - w);
        ty = 0.5 * (pl * 0.333 - l);

        p->outmatrix[0][0] = w / p->width;
        p->outmatrix[1][0] = 0.0f;
        p->outmatrix[0][1] = 0.0f;
        p->outmatrix[1][1] = w / p->width;
        p->outmatrix[0][2] = tx + x * pw / 3;
        p->outmatrix[1][2] = ty + y * pl / 3;
	break;

    case 16 :
        x = pos & 3;
	y = 3 - pos / 4;

        w = pw * 0.25;
	l = w * p->length / p->width;

	if (l > (pl * 0.25))
	{
	  l = pl * 0.25;
	  w = l * p->width / p->length;
	}

        tx = 0.5 * (pw * 0.25 - w);
        ty = 0.5 * (pl * 0.25 - l);

        p->outmatrix[0][0] = w / p->width;
        p->outmatrix[1][0] = 0.0f;
        p->outmatrix[0][1] = 0.0f;
        p->outmatrix[1][1] = w / p->width;
        p->outmatrix[0][2] = tx + x * pw / 4;
        p->outmatrix[1][2] = ty + y * pl / 4;
	break;
  }
}


/*
 * 'pspdf_prepare_outpages()' - Prepare output pages...
 */

static void
pspdf_prepare_outpages()
{
  int		c, i, j;	/* Looping vars */
  int		nup;		/* Current number-up value */
  page_t	*page;		/* Current page */
  outpage_t	*outpage;	/* Current output page */


  // Allocate an output page array...
  outpages = (outpage_t *)malloc(sizeof(outpage_t) * num_pages);

  memset(outpages, -1, sizeof(outpage_t) * num_pages);

  num_outpages = 0;
  outpage      = outpages;

  // Handle the title page, as needed...
  if (TitlePage)
  {
    for (i = 0, j = 0, nup = -1, page = pages;
         i < chapter_starts[1];
	 i ++, page ++)
    {
      if (nup != page->nup)
      {
        if (j)
	{
	  // Break the current output page...
	  outpage ++;
	  num_outpages ++;
	}

	nup = page->nup;
	j   = 0;
      }

      if (!j)
	outpage->nup = nup;

      pspdf_transform_page(num_outpages, j, i);
      j ++;

      if (j >= nup)
      {
        j = 0;
	outpage ++;
	num_outpages ++;
      }
    }

    if (j)
    {
      // Break the current output page...
      outpage ++;
      num_outpages ++;
    }
  }

  // Loop through each chapter, adding pages as needed...
  if (OutputType == OUTPUT_BOOK && TocLevels > 0)
    c = 0;
  else
    c = 1;

  for (; c <= TocDocCount; c ++)
  {
    if (chapter_starts[c] < 0)
      continue;

    chapter_outstarts[c] = num_outpages;

    for (i = chapter_starts[c], j = 0, nup = -1, page = pages + i;
         i <= chapter_ends[c];
	 i ++, page ++)
    {
      if (nup != page->nup)
      {
        if (j)
	{
	  // Break the current output page...
	  outpage ++;
	  num_outpages ++;
	}

	nup = page->nup;
	j   = 0;
      }

      if (!j)
	outpage->nup = nup;

      pspdf_transform_page(num_outpages, j, i);
      j ++;

      if (j >= nup)
      {
        j = 0;
	outpage ++;
	num_outpages ++;
      }
    }

    if (j)
    {
      // Break the current output page...
      outpage ++;
      num_outpages ++;
    }

    chapter_outends[c] = num_outpages;
  }

#ifdef DEBUG
  for (c = 0; c <= TocDocCount; c ++)
    printf("chapter_outstarts[%d] = %d, chapter_outends[%d] = %d\n",
           c, chapter_outstarts[c], c, chapter_outends[c]);

  printf("num_outpages = %d\n", num_outpages);
  for (i = 0, outpage = outpages; i < num_outpages; i ++, outpage ++)
  {
    printf("outpage[%d]:\tnup=%d, pages=[", i, outpage->nup);
    for (j = 0; j < outpage->nup; j ++)
      printf(" %d", outpage->pages[j]);
    puts(" ]");
    page = pages + outpage->pages[0];
    printf("\t\twidth = %d, length = %d\n", page->width, page->length);
  }

  for (c = 0; c <= TocDocCount; c ++)
    printf("chapter_starts[%d] = %d, chapter_ends[%d] = %d\n",
           c, chapter_starts[c], c, chapter_ends[c]);

  for (i = 0; i < num_pages; i ++)
    printf("pages[%d]->outpage = %d\n", i, pages[i].outpage);

  for (i = 0; i < num_headings; i ++)
    printf("heading_pages[%d] = %d\n", i, heading_pages[i]);

  for (i = 0; i < num_links; i ++)
    printf("links[%d].name = \"%s\", page = %d\n", i,
           links[i].name, links[i].page);
#endif // DEBUG
}


/*
 * 'pspdf_prepare_page()' - Add headers/footers to page before writing...
 */

static void
pspdf_prepare_page(int page)		/* I - Page number */
{
  int	print_page;			/* Printed page # */
  char	page_text[64];			/* Page number text */
  int	top;				/* Top of page */


  DEBUG_printf(("pspdf_prepare_page(%d)\n", page));

 /*
  * Make a page number; use roman numerals for the table of contents
  * and arabic numbers for all others...
  */

  if (chapter == 0 && OutputType == OUTPUT_BOOK)
  {
    print_page = page - chapter_starts[0] + 1;
    strlcpy(page_text, format_number(print_page, 'i'), sizeof(page_text));
  }
  else if (chapter < 0)
  {
    print_page = 0;
    // Safe because page_text is more than 6 chars
    strcpy(page_text, (page & 1) ? (char *)"eltit" : (char *)"title");
  }
  else
  {
    print_page = page - chapter_starts[1] + 1;
    strlcpy(page_text, format_number(print_page, '1'), sizeof(page_text));
  }

  DEBUG_printf(("BEFORE page %d page_text is \"%s\"...\n", page, page_text));

  DEBUG_printf(("    header[0] = \"%s\"\n", pages[page].header[0]));
  DEBUG_printf(("    header[1] = \"%s\"\n", pages[page].header[1]));
  DEBUG_printf(("    header[2] = \"%s\"\n", pages[page].header[2]));

 /*
  * Add page headings...
  */

  if (pages[page].landscape)
  {
    PagePrintWidth  = pages[page].length - pages[page].right - pages[page].left;
    PagePrintLength = pages[page].width - pages[page].top - pages[page].bottom;
  }
  else
  {
    PagePrintWidth  = pages[page].width - pages[page].right - pages[page].left;
    PagePrintLength = pages[page].length - pages[page].top - pages[page].bottom;
  }

  top = (int)(PagePrintLength - HeadFootSize);

  if (chapter == 0)
  {
   /*
    * Add table-of-contents header & footer...
    */

    pspdf_prepare_heading(page, print_page, pages[page].header, top,
                          page_text, sizeof(page_text));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          page_text, sizeof(page_text));
  }
  else if (chapter > 0 && !title_page)
  {
   /*
    * Add chapter header & footer...
    */

    if (page > chapter_starts[chapter] || OutputType != OUTPUT_BOOK)
      pspdf_prepare_heading(page, print_page, pages[page].header, top,
                            page_text, sizeof(page_text));
    else
      pspdf_prepare_heading(page, print_page, pages[page].header1, top,
                            page_text, sizeof(page_text));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          page_text, sizeof(page_text));
  }

 /*
  * Copy the page number for the TOC...
  */

  strlcpy(pages[page].page_text, page_text, sizeof(pages[page].page_text));

  DEBUG_printf(("AFTER page %d page_text is \"%s\"...\n", page, page_text));
}


/*
 * 'pspdf_prepare_heading()' - Add headers/footers to page before writing...
 */

static void
pspdf_prepare_heading(int   page,	// I - Page number
                      int   print_page,	// I - Printed page number
		      uchar **format,	// I - Page headings
		      int   y,		// I - Baseline of heading
		      char  *page_text,	// O - Page number text
		      int   page_len)	// I - Size of page text
{
  int		pos,			// Position in heading
		dir;			// Direction of page
  char		*number;		// Page number
  char		buffer[1024],		// String buffer
		*bufptr,		// Pointer into buffer
		*formatptr;		// Pointer into format string
  int		formatlen;		// Length of format command string
  render_t	*temp;			// Render structure for titles, etc.


  DEBUG_printf(("pspdf_prepare_heading(%d, %d, [\"%s\",\"%s\",\"%s\"], %d, %p, %d)\n",
                page, print_page, format[0], format[1], format[2], y,
		page_text, page_len));

 /*
  * Add page headings...
  */

  if (PageDuplex && (page & 1))
  {
    dir    = -1;
    format += 2;
  }
  else
    dir = 1;

  for (pos = 0; pos < 3; pos ++, format += dir)
  {
   /*
    * Add the appropriate object...
    */

    if (!*format)
      continue;

    temp = NULL;

    if (strncasecmp((char *)*format, "$LOGOIMAGE", 10) == 0 && logo_image)
    {
      // Insert the logo image...
      if (y < (PagePrintLength / 2))
	temp = new_render(page, RENDER_IMAGE, 0, y, logo_width,
	                  logo_height, logo_image);
      else // Offset from top
	temp = new_render(page, RENDER_IMAGE, 0,
	                  y + HeadFootSize - logo_height,
	                  logo_width, logo_height, logo_image);
    }
    else if (strncasecmp((char *)*format, "$HFIMAGE", 8) == 0)
    {
      int	hfi;			// Header/footer image index
      char	*hfp;			// Pointer into $HFIMAGE


      hfi = strtol((char*)((*format) + 8), &hfp, 10);

      if (hfi < 0 || hfi >= MAX_HF_IMAGES || !(isspace(*hfp) || !*hfp))
        progress_error(HD_ERROR_BAD_HF_STRING,
	               "Bad $HFIMAGE... substitution on page %d.", page + 1);
      else
      {
        if (y < (PagePrintLength / 2))
          temp = new_render(page, RENDER_IMAGE, 0, y, hfimage_width[hfi],
                            hfimage_height[hfi], hfimage[hfi]);
        else
          temp = new_render(page, RENDER_IMAGE, 0,
                            y + HeadFootSize - hfimage_height[hfi],
                            hfimage_width[hfi], hfimage_height[hfi],
			    hfimage[hfi]);
      }
    }
    else
    {
      // Otherwise format the text...
      buffer[sizeof(buffer) - 1] = '\0';

      for (bufptr = buffer, formatptr = (char *)*format; *formatptr;)
      {
        if (*formatptr == '$')
	{
	  if (formatptr[1] == '$')
	  {
	    if (bufptr < (buffer + sizeof(buffer) - 1))
	      *bufptr++ = '$';

	    formatptr += 2;
	    continue;
	  }
	  else if (!formatptr[1])
	    break;

          formatptr ++;
	  for (formatlen = 1; isalpha(formatptr[formatlen]); formatlen ++);

	  if (formatlen == 4 && strncasecmp(formatptr, "PAGE", 4) == 0)
	  {
	    if (formatptr[4] == '(' && formatptr[5] && formatptr[6] == ')')
            {
	      number = format_number(print_page, formatptr[5]);
	      formatptr += 7;
	    }
	    else
	    {
	      number = format_number(print_page, '1');
	      formatptr += 4;
	    }

            strlcpy(bufptr, number, sizeof(buffer) - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 5 && strncasecmp(formatptr, "PAGES", 5) == 0)
	  {
	    if (formatptr[5] == '(' && formatptr[6] && formatptr[7] == ')')
            {
	      number = format_number(chapter_ends[TocDocCount] -
	                             chapter_starts[1] + 1, formatptr[6]);
	      formatptr += 8;
	    }
	    else
	    {
	      number = format_number(chapter_ends[TocDocCount] -
	                             chapter_starts[1] + 1, '1');
	      formatptr += 5;
	    }

            strlcpy(bufptr, number, sizeof(buffer) - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 11 && strncasecmp(formatptr, "CHAPTERPAGE", 11) == 0)
	  {
	    int chapter_page;

	    chapter_page = print_page - chapter_starts[::chapter] +
	                   chapter_starts[1];

	    if (formatptr[11] == '(' && formatptr[12] && formatptr[13] == ')')
            {
	      number = format_number(chapter_page, formatptr[12]);
	      formatptr += 14;
	    }
	    else
	    {
	      number = format_number(chapter_page, '1');
	      formatptr += 11;
	    }

            strlcpy(bufptr, number, sizeof(buffer) - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 12 && strncasecmp(formatptr, "CHAPTERPAGES", 12) == 0)
	  {
	    if (formatptr[12] == '(' && formatptr[13] && formatptr[14] == ')')
            {
	      number = format_number(chapter_ends[::chapter] -
	                             chapter_starts[::chapter] + 1,
				     formatptr[13]);
	      formatptr += 15;
	    }
	    else
	    {
	      number = format_number(chapter_ends[::chapter] -
	                             chapter_starts[::chapter] + 1, '1');
	      formatptr += 12;
	    }

            strlcpy(bufptr, number, sizeof(buffer) - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 5 && strncasecmp(formatptr, "TITLE", 5) == 0)
	  {
            formatptr += 5;
	    if (doc_title)
	    {
              strlcpy(bufptr, (char *)doc_title,
	              sizeof(buffer) - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "CHAPTER", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].chapter)
	    {
              strlcpy(bufptr, (char *)(pages[page].chapter),
	              sizeof(buffer) - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "HEADING", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].heading)
	    {
              strlcpy(bufptr, (char *)(pages[page].heading),
	              sizeof(buffer) - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 4 && strncasecmp(formatptr, "TIME", 4) == 0)
	  {
            formatptr += 4;
            strftime(bufptr, sizeof(buffer) - 1 - (bufptr - buffer), "%X",
	             doc_date);
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 4 && strncasecmp(formatptr, "DATE", 4) == 0)
	  {
            formatptr += 4;
            strftime(bufptr, sizeof(buffer) - 1 - (bufptr - buffer), "%x",
	             doc_date);
	    bufptr += strlen(bufptr);
	  }
	  else
	  {
            progress_error(HD_ERROR_BAD_HF_STRING,
	        	   "Bad header/footer $ command on page %d.", page + 1);

            strlcpy(bufptr, formatptr - 1, sizeof(buffer) - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	    formatptr += formatlen;
	  }
	}
	else if (bufptr < (buffer + sizeof(buffer) - 1))
	  *bufptr++ = *formatptr++;
	else
	  break;
      }

      *bufptr = '\0';

      temp = new_render(page, RENDER_TEXT, 0, y,
                	get_width((uchar *)buffer, HeadFootType,
			          HeadFootStyle, SIZE_P) * HeadFootSize /
			    _htmlSizes[SIZE_P],
	        	HeadFootSize, (uchar *)buffer);

      if (strstr((char *)*format, "$PAGE") ||
          strstr((char *)*format, "$CHAPTERPAGE"))
        strlcpy(page_text, buffer, page_len);
    }

    if (temp == NULL)
      continue;

   /*
    * Justify the object...
    */

    switch (pos)
    {
      case 0 : /* Left justified */
          break;
      case 1 : /* Centered */
          temp->x = (PagePrintWidth - temp->width) * 0.5;
          break;
      case 2 : /* Right justified */
          temp->x = PagePrintWidth - temp->width;
          break;
    }

   /*
    * Set the text font and color...
    */

    if (temp->type == RENDER_TEXT)
    {
      temp->data.text.typeface = HeadFootType;
      temp->data.text.style    = HeadFootStyle;
      temp->data.text.size     = HeadFootSize;

      get_color(_htmlTextColor, temp->data.text.rgb);
    }
  }
}


/*
 * 'ps_write_document()' - Write all render entities to PostScript file(s).
 */

static void
ps_write_document(uchar *author,	/* I - Author of document */
        	  uchar *creator,	/* I - Application that generated the HTML file */
        	  uchar *copyright,	/* I - Copyright (if any) on the document */
                  uchar *keywords,	/* I - Search keywords */
		  uchar *subject)	/* I - Subject */
{
  FILE		*out;			/* Output file */
  int		page;			/* Current page # */
  int		first;			/* First chapter */


 /*
  * Write the title page(s)...
  */

  chapter = -1;
  out     = NULL;

  if (!OutputFiles)
  {
    out = open_file();

    if (out == NULL)
    {
      progress_error(HD_ERROR_WRITE_ERROR,
                     "Unable to open output file - %s\n", strerror(errno));
      return;
    }

    write_prolog(out, num_outpages, author, creator, copyright, keywords, subject);
  }

  if (OutputType == OUTPUT_BOOK && TocLevels > 0)
    first = 0;
  else
    first = 1;

  if (TitlePage)
  {
    if (OutputFiles)
    {
      out = open_file();
      write_prolog(out, chapter_outstarts[first], author, creator, copyright,
                   keywords, subject);
    }

    for (page = 0; page < chapter_outstarts[first]; page ++)
      ps_write_outpage(out, page);

    if (OutputFiles)
    {
      write_trailer(out, 0);

      progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

      fclose(out);
    }
  }

  for (chapter = first; chapter <= TocDocCount; chapter ++)
  {
    if (chapter_starts[chapter] < 0)
      continue;

    if (OutputFiles)
    {
      out = open_file();
      if (out == NULL)
      {
        progress_error(HD_ERROR_WRITE_ERROR,
	               "Unable to create output file - %s\n", strerror(errno));
        return;
      }

      write_prolog(out, chapter_outends[chapter] - chapter_outstarts[chapter],
                   author, creator, copyright, keywords, subject);
    }

    for (page = chapter_outstarts[chapter];
         page < chapter_outends[chapter];
         page ++)
      ps_write_outpage(out, page);

   /*
    * Close the output file as necessary...
    */

    if (OutputFiles)
    {
      write_trailer(out, 0);

      progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

      fclose(out);
    }
  }

 /*
  * Close the output file as necessary...
  */

  if (!OutputFiles)
  {
    write_trailer(out, 0);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

    if (out != stdout)
      fclose(out);
  }

  if (Verbosity)
    progress_hide();
}


/*
 * 'ps_write_outpage()' - Write an output page.
 */

static void
ps_write_outpage(FILE *out,	/* I - Output file */
                 int  outpage)	/* I - Output page number */
{
  int		file_page;	/* Current page # in document */
  page_t	*p;		/* Current page */
  outpage_t	*op;		/* Current output page */
  int		i;		/* Looping var */


  if (outpage < 0 || outpage >= num_outpages)
    return;

  op = outpages + outpage;
  p  = pages + op->pages[0];

  DEBUG_printf(("ps_write_outpage(%p, %d)\n", out, outpage));

 /*
  * Let the user know which page we are writing...
  */

  if (Verbosity)
  {
    progress_show("Writing page %s...", p->page_text);
    progress_update(100 * outpage / num_outpages);
  }

 /*
  * Figure out the page number in the file...
  */

  if (OutputFiles && chapter >= 0)
    file_page = outpage - chapter_outstarts[chapter] + 1;
  else if (chapter < 0)
    file_page = outpage + 1;
  else if (chapter == 0)
  {
    if (TitlePage)
      file_page = outpage + 1;
    else
      file_page = outpage - chapter_outstarts[0] + 1;
  }
  else
  {
    if (TitlePage)
      file_page = outpage + 1;
    else
      file_page = outpage - chapter_outstarts[1] + 1;
  }

 /*
  * Output the page prolog...
  */

  fprintf(out, "%%%%Page: (%s) %d\n", p->page_text, file_page);
  if (op->nup == 1)
  {
    if (p->duplex && !(file_page & 1))
      fprintf(out, "%%%%PageBoundingBox: %d %d %d %d\n",
              p->right, p->bottom, p->width - p->left, p->length - p->top);
    else
      fprintf(out, "%%%%PageBoundingBox: %d %d %d %d\n",
              p->left, p->bottom, p->width - p->right, p->length - p->top);
  }
  else
    fprintf(out, "%%%%PageBoundingBox: 0 0 %d %d\n", p->width, p->length);

  if (PSLevel > 1 && PSCommands)
  {
    fputs("%%BeginPageSetup\n", out);

    if (p->width == 612 && p->length == 792)
      fputs("%%BeginFeature: *PageSize Letter\n", out);
    else if (p->width == 612 && p->length == 1008)
      fputs("%%BeginFeature: *PageSize Legal\n", out);
    else if (p->width == 792 && p->length == 1224)
      fputs("%%BeginFeature: *PageSize Tabloid\n", out);
    else if (p->width == 842 && p->length == 1190)
      fputs("%%BeginFeature: *PageSize A3\n", out);
    else if (p->width == 595 && p->length == 842)
      fputs("%%BeginFeature: *PageSize A4\n", out);
    else
      fprintf(out, "%%%%BeginFeature: *PageSize w%dh%d\n", p->width,
	      p->length);

    fprintf(out, "%d %d SetPageSize\n", p->width, p->length);
    fputs("%%EndFeature\n", out);

    if (p->duplex)
    {
      if (p->landscape)
      {
	fputs("%%BeginFeature: *Duplex DuplexTumble\n", out);
	fputs("true true SetDuplexMode\n", out);
        fputs("%%EndFeature\n", out);
      }
      else
      {
	fputs("%%BeginFeature: *Duplex DuplexNoTumble\n", out);
	fputs("true false SetDuplexMode\n", out);
        fputs("%%EndFeature\n", out);
      }
    }
    else
    {
      fputs("%%BeginFeature: *Duplex None\n", out);
      fputs("false false SetDuplexMode\n", out);
      fputs("%%EndFeature\n", out);
    }

    if (p->media_color[0])
    {
      fprintf(out, "%%%%BeginFeature: *MediaColor %s\n", p->media_color);
      fprintf(out, "(%s) SetMediaColor\n", p->media_color);
      fputs("%%EndFeature\n", out);
    }

    if (p->media_position)
    {
      fprintf(out, "%%%%BeginFeature: *InputSlot Tray%d\n",
              p->media_position);
      fprintf(out, "%d SetMediaPosition\n", p->media_position);
      fputs("%%EndFeature\n", out);
    }

    if (p->media_type[0])
    {
      fprintf(out, "%%%%BeginFeature: *MediaType %s\n", p->media_type);
      fprintf(out, "(%s) SetMediaType\n", p->media_type);
      fputs("%%EndFeature\n", out);
    }

    fputs("%%EndPageSetup\n", out);
  }

 /*
  * Render all of the pages...
  */

  switch (op->nup)
  {
    case 1 :
        ps_write_page(out, op->pages[0]);
	break;

    default :
        for (i = 0; i < op->nup; i ++)
	{
	  if (op->pages[i] < 0)
	    break;

          p = pages + op->pages[i];

          fprintf(out, "GS[%.3f %.3f %.3f %.3f %.3f %.3f]CM\n",
	          p->outmatrix[0][0], p->outmatrix[1][0],
	          p->outmatrix[0][1], p->outmatrix[1][1],
	          p->outmatrix[0][2], p->outmatrix[1][2]);
          ps_write_page(out, op->pages[i]);
	  fputs("GR\n", out);
	}
	break;
  }

 /*
  * Output the page trailer...
  */

  fputs("SP\n", out);
  fflush(out);
}


/*
 * 'ps_write_page()' - Write all render entities on a page to a PostScript file.
 */

static void
ps_write_page(FILE  *out,	/* I - Output file */
              int   page)	/* I - Page number */
{
  render_t	*r,		/* Render pointer */
		*next;		/* Next render */
  page_t	*p;		/* Current page */
  const char	*debug;		/* HTMLDOC_DEBUG environment variable */


  if (page < 0 || page >= alloc_pages)
    return;

  p = pages + page;

  DEBUG_printf(("ps_write_page(%p, %d)\n", out, page));

 /*
  * Clear the render cache...
  */

  render_typeface = -1;
  render_style    = -1;
  render_size     = -1;
  render_rgb[0]   = -1.0f;
  render_rgb[1]   = -1.0f;
  render_rgb[2]   = -1.0f;
  render_x        = -1.0f;
  render_y        = -1.0f;
  render_spacing  = -1.0f;

 /*
  * Setup the page...
  */

  fputs("GS\n", out);

  if (p->landscape)
  {
    if (p->duplex && (page & 1))
      fprintf(out, "0 %d T -90 RO\n", p->length);
    else
      fprintf(out, "%d 0 T 90 RO\n", p->width);
  }

  write_background(page, out);

  if (p->duplex && (page & 1))
    fprintf(out, "%d %d T\n", p->right, p->bottom);
  else
    fprintf(out, "%d %d T\n", p->left, p->bottom);

 /*
  * Render all graphics elements...
  */

  for (r = p->start; r != NULL; r = r->next)
    switch (r->type)
    {
      case RENDER_BOX :
	  set_color(out, r->data.box);
	  set_pos(out, r->x, r->y);
	  if (r->height > 0.0f)
            fprintf(out, " %.1f %.1f F\n", r->width, r->height);
	  else
            fprintf(out, " %.1f L\n", r->width);

	  render_x = -1.0f;
	  break;

      case RENDER_IMAGE :
          if (r->width > 0.01f && r->height > 0.01f)
            write_image(out, r);
          break;
    }

 /*
  * Render all text elements, freeing used memory as we go...
  */

  for (r = p->start, next = NULL; r != NULL; r = next)
  {
    if (r->type == RENDER_TEXT)
      write_text(out, r);

    next = r->next;
    free(r);
  }

  p->start = NULL;

  if ((debug = getenv("HTMLDOC_DEBUG")) != NULL && strstr(debug, "margin"))
  {
    // Show printable area...
    fprintf(out, "1 0 1 C 0 0 %d %d B\n", p->width - p->right - p->left,
        	 p->length - p->top - p->bottom);
  }

 /*
  * Output the page trailer...
  */

  fputs("GR\n", out);
}


/*
 * 'ps_write_background()' - Write a background image...
 */

static void
ps_write_background(FILE *out)		/* I - Output file */
{
  int	y,				/* Current line */
	pwidth;				/* Pixel width */


  if (!background_image->pixels)
    image_load(background_image->filename, !OutputColor, 1);

  pwidth = background_image->width * background_image->depth;

  fputs("/BG[", out);
  for (y = 0; y < background_image->height; y ++)
  {
    putc('<', out);
    ps_hex(out, background_image->pixels + y * pwidth, pwidth);
    putc('>', out);
  }
  fputs("]def", out);

  image_unload(background_image);
}


/*
 * 'pdf_write_document()' - Write all render entities to a PDF file.
 */

static void
pdf_write_document(uchar  *author,	// I - Author of document
        	   uchar  *creator,	// I - Application that generated the HTML file
        	   uchar  *copyright,	// I - Copyright (if any) on the document
                   uchar  *keywords,	// I - Search keywords
		   uchar  *subject,	// I - Subject
		   tree_t *doc,		// I - Document
                   tree_t *toc)		// I - Table of contents tree
{
  int		i;			// Looping variable
  FILE		*out;			// Output file
  int		outpage,		// Current page #
		heading;		// Current heading #
  int		bytes;			// Number of bytes
  char		buffer[8192];		// Copy buffer
  int		num_images;		// Number of images in document
  image_t	**images;		// Pointers to images
  render_t	temp;			// Dummy rendering data...


  // Open the output file...
  out = open_file();
  if (out == NULL)
  {
    progress_error(HD_ERROR_WRITE_ERROR,
                   "Unable to write document file - %s\n", strerror(errno));
    return;
  }

  // Clear the objects array...
  num_objects   = 0;
  alloc_objects = 0;
  objects       = NULL;

  // Write the prolog...
  write_prolog(out, num_outpages, author, creator, copyright, keywords, subject);

  // Write images as needed...
  num_images = image_getlist(&images);

  for (i = 0; i < num_images; i ++)
  {
    int	hfi;				// Header/footer image index


    for (hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (images[i] == hfimage[hfi])
        break;

    if (images[i]->use > 1 || images[i]->mask ||
        (images[i]->width * images[i]->height * images[i]->depth) > 65536 ||
	images[i] == background_image ||
	images[i] == logo_image ||
	hfi < MAX_HF_IMAGES)
    {
      progress_show("Writing image %d (%s)...", i + 1, images[i]->filename);
      progress_update(100 * i / num_images);

      temp.data.image = images[i];
      write_image(out, &temp, 1);
    }
  }

  // Write links and target names...
  pdf_write_links(out);
  if (PDFVersion >= 12)
    pdf_write_names(out);

  // Verify that everything is working so far...
  pdf_start_object(out);

  if (pages_object != num_objects)
    progress_error(HD_ERROR_INTERNAL_ERROR,
                   "Internal error: pages_object != num_objects");

  fputs("/Type/Pages", out);
  fprintf(out, "/Count %d", num_outpages);
  fputs("/Kids[", out);

  for (outpage = 0; outpage < num_outpages; outpage ++)
    fprintf(out, "%d 0 R\n", pages_object + outpage * 2 + 1);

  fputs("]", out);
  pdf_end_object(out);

  for (outpage = 0; outpage < num_outpages; outpage ++)
    pdf_write_outpage(out, outpage);

  if (OutputType == OUTPUT_BOOK && TocLevels > 0)
  {
   /*
    * Write the outline tree using the table-of-contents...
    */

    heading = 0;
#ifdef DEBUG_TOC
    pdf_text_contents(out, toc);
#endif // DEBUG_TOC
    pdf_write_contents(out, toc, 0, 0, 0, &heading);
  }
  else
  {
   /*
    * Write the outline tree using the HTML files.
    */

    pdf_write_files(out, doc);
  }

 /*
  * Write the trailer and close the output file...
  */

  write_trailer(out, 0);

  progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

  if (CGIMode)
  {
    // In CGI mode, we only produce PDF output to stdout...
    printf("Content-Type: application/pdf\r\n"
	   "Content-Length: %ld\r\n"
	   "Content-Disposition: inline; filename=\"htmldoc.pdf\"\r\n"
	   "Accept-Ranges: none\r\n"
	   "X-Creator: HTMLDOC " SVERSION "\r\n"
	   "\r\n", ftell(out));
  }

  fclose(out);

  //
  // If we are sending the output to stdout, copy the temp file now...
  //

  if (!OutputPath[0])
  {
#ifdef WIN32
    // Make sure we are in binary mode...  stupid Microsoft!
    setmode(1, O_BINARY);
#elif defined(__EMX__)
   // OS/2 has a setmode for FILE's...
   fflush(stdout);
   _fsetmode(stdout, "b");
#endif // WIN32 || __EMX__

    // Open the temporary file and copy it to stdout...
    out = fopen(stdout_filename, "rb");

    while ((bytes = fread(buffer, 1, sizeof(buffer), out)) > 0)
      fwrite(buffer, 1, bytes, stdout);

    // Close the temporary file (it is removed when the program exits...)
    fclose(out);
  }

  // Clear the objects array...
  if (alloc_objects)
  {
    free(objects);

    num_objects   = 0;
    alloc_objects = 0;
    objects       = NULL;
  }

  if (Verbosity)
    progress_hide();
}


/*
 * 'pdf_write_resources()' - Write the resources dictionary for a page.
 */

static void
pdf_write_resources(FILE *out,		/* I - Output file */
                    int  outpage)	/* I - Output page for resources */
{
  int		i;			/* Looping var */
  outpage_t	*op;			/* Current output page */
  page_t	*p;			/* Current page */
  render_t	*r;			/* Render pointer */
  int		fonts_used[TYPE_MAX * STYLE_MAX];
					/* Non-zero if the page uses a font */
  int		images_used;		/* Non-zero if the page uses an image */
  int		text_used;		/* Non-zero if the page uses text */
  static const char *effects[] =	/* Effects and their commands */
		{
		  "",
		  "/S/Box/M/I",
		  "/S/Box/M/O",
		  "/S/Dissolve",
		  "/S/Glitter/Di 270",
		  "/S/Glitter/Di 315",
		  "/S/Glitter/Di 0",
		  "/S/Blinds/Dm/H",
		  "/S/Split/Dm/H/M/I",
		  "/S/Split/Dm/H/M/O",
		  "/S/Blinds/Dm/V",
		  "/S/Split/Dm/V/M/I",
		  "/S/Split/Dm/V/M/O",
		  "/S/Wipe/Di 270",
		  "/S/Wipe/Di 180",
		  "/S/Wipe/Di 0",
		  "/S/Wipe/Di 90"
		};


  memset(fonts_used, 0, sizeof(fonts_used));
  fonts_used[HeadFootType * 4 + HeadFootStyle] = 1;
  images_used = background_image != NULL;
  text_used   = 0;

  op = outpages + outpage;
  for (i = 0; i < op->nup; i ++)
  {
    if (op->pages[i] < 0)
      break;

    p = pages + op->pages[i];

    for (r = p->start; r != NULL; r = r->next)
      if (r->type == RENDER_IMAGE)
	images_used = 1;
      else if (r->type == RENDER_TEXT)
      {
	text_used = 1;
	fonts_used[r->data.text.typeface * 4 + r->data.text.style] = 1;
      }
  }

  fputs("/Resources<<", out);

  if (!images_used)
    fputs("/ProcSet[/PDF/Text]", out);
  else if (PDFVersion >= 12)
  {
    if (OutputColor)
      fputs("/ProcSet[/PDF/Text/ImageB/ImageC/ImageI]", out);
    else
      fputs("/ProcSet[/PDF/Text/ImageB/ImageI]", out);
  }
  else
  {
    if (OutputColor)
      fputs("/ProcSet[/PDF/Text/ImageB/ImageC]", out);
    else
      fputs("/ProcSet[/PDF/Text/ImageB]", out);
  }

  if (text_used)
  {
    fputs("/Font<<", out);
    for (i = 0; i < (TYPE_MAX * STYLE_MAX); i ++)
      if (fonts_used[i])
	fprintf(out, "/F%x %d 0 R", i, font_objects[i]);
    fputs(">>", out);
  }

  fputs("/XObject<<", out);

  for (i = 0; i < op->nup; i ++)
  {
    if (op->pages[i] < 0)
      break;

    p = pages + op->pages[i];

    for (r = p->start; r != NULL; r = r->next)
      if (r->type == RENDER_IMAGE && r->data.image->obj)
	fprintf(out, "/I%d %d 0 R", r->data.image->obj, r->data.image->obj);
  }

  if (background_image)
    fprintf(out, "/I%d %d 0 R", background_image->obj,
            background_image->obj);

  fputs(">>>>", out);

  if (PDFEffect)
    fprintf(out, "/Dur %.0f/Trans<</Type/Trans/D %.1f%s>>", PDFPageDuration,
            PDFEffectDuration, effects[PDFEffect]);
}


/*
 * 'pdf_write_outpage()' - Write an output page.
 */

static void
pdf_write_outpage(FILE *out,	/* I - Output file */
                  int  outpage)	/* I - Output page number */
{
  int		i;		/* Looping var */
  page_t	*p;		/* Current page */
  outpage_t	*op;		/* Output page */


  DEBUG_printf(("pdf_write_outpage(out = %p, outpage = %d)\n", out, outpage));

  if (outpage < 0 || outpage >= num_outpages)
    return;

  op = outpages + outpage;
  p  = pages + op->pages[0];

  DEBUG_printf(("op->pages[0] = %d (%dx%d)\n", op->pages[0], p->width,
                p->length));

 /*
  * Let the user know which page we are writing...
  */

  if (Verbosity)
  {
    progress_show("Writing page %s...", p->page_text);
    progress_update(100 * outpage / num_outpages);
  }

 /*
  * Output the page prolog...
  */

  pdf_start_object(out);

  fputs("/Type/Page", out);
  fprintf(out, "/Parent %d 0 R", pages_object);
  fprintf(out, "/Contents %d 0 R", num_objects + 1);
  if (p->landscape)
    fprintf(out, "/MediaBox[0 0 %d %d]", p->length, p->width);
  else
    fprintf(out, "/MediaBox[0 0 %d %d]", p->width, p->length);

  pdf_write_resources(out, outpage);

 /*
  * Actions (links)...
  */

  if (op->annot_object > 0)
    fprintf(out, "/Annots %d 0 R", op->annot_object);

  pdf_end_object(out);

  pdf_start_object(out);

  if (Compression)
    fputs("/Filter/FlateDecode", out);

  pdf_start_stream(out);

  flate_open_stream(out);

 /*
  * Render all of the pages...
  */

  switch (op->nup)
  {
    case 1 :
        pdf_write_page(out, op->pages[0]);
	break;

    default :
        for (i = 0; i < op->nup; i ++)
	{
	  if (op->pages[i] < 0)
	    break;

          p = pages + op->pages[i];

          flate_printf(out, "q %.3f %.3f %.3f %.3f %.3f %.3f cm\n",
	               p->outmatrix[0][0], p->outmatrix[1][0],
	               p->outmatrix[0][1], p->outmatrix[1][1],
	               p->outmatrix[0][2], p->outmatrix[1][2]);
          pdf_write_page(out, op->pages[i]);
	  flate_puts("Q\n", out);
	}
	break;
  }

 /*
  * Close out the page...
  */

  flate_close_stream(out);

  pdf_end_object(out);
}


/*
 * 'pdf_write_page()' - Write a page to a PDF file.
 */

static void
pdf_write_page(FILE  *out,	/* I - Output file */
               int   page)	/* I - Page number */
{
  render_t	*r,		/* Render pointer */
		*next;		/* Next render */
  float		box[3];		/* RGB color for boxes */
  page_t	*p;		/* Current page */
  const char	*debug;		/* HTMLDOC_DEBUG environment variable */


  if (page < 0 || page >= alloc_pages)
    return;

  p = pages + page;

 /*
  * Clear the render cache...
  */

  render_rgb[0]   = -1.0f;
  render_rgb[1]   = -1.0f;
  render_rgb[2]   = -1.0f;
  render_x        = -1.0f;
  render_y        = -1.0f;

 /*
  * Output the page header...
  */

  flate_puts("q\n", out);
  write_background(page, out);

  if (p->duplex && (page & 1))
    flate_printf(out, "1 0 0 1 %d %d cm\n", p->right,
                 p->bottom);
  else
    flate_printf(out, "1 0 0 1 %d %d cm\n", p->left,
                 p->bottom);

 /*
  * Render all graphics elements...
  */

  box[0] = -1.0f;
  box[1] = -1.0f;
  box[2] = -1.0f;

  for (r = p->start; r != NULL; r = r->next)
    switch (r->type)
    {
      case RENDER_IMAGE :
          if (r->width > 0.01f && r->height > 0.01f)
            write_image(out, r);
          break;

      case RENDER_BOX :
	  if (r->height == 0.0)
	  {
            if (box[0] != r->data.box[0] ||
		box[1] != r->data.box[1] ||
		box[2] != r->data.box[2])
            {
              box[0] = r->data.box[0];
	      box[1] = r->data.box[1];
	      box[2] = r->data.box[2];

	      if (OutputColor)
        	flate_printf(out, "%.2f %.2f %.2f RG\n", box[0], box[1], box[2]);
              else
        	flate_printf(out, "%.2f G\n",
		             box[0] * 0.31f + box[1] * 0.61f + box[2] * 0.08f);
            }

            flate_printf(out, "%.1f %.1f m %.1f %.1f l S\n",
                	 r->x, r->y, r->x + r->width, r->y);
	  }
	  else
	  {
            set_color(out, r->data.box);
            flate_printf(out, "%.1f %.1f %.1f %.1f re f\n",
                	 r->x, r->y, r->width, r->height);
	  }
	  break;
    }

 /*
  * Render all text elements, freeing used memory as we go...
  */

  flate_puts("BT\n", out);

  render_typeface = -1;
  render_style    = -1;
  render_size     = -1;
  render_x        = -1.0f;
  render_y        = -1.0f;
  render_spacing  = -1.0f;

  for (r = p->start, next = NULL; r != NULL; r = next)
  {
    if (r->type == RENDER_TEXT)
      write_text(out, r);

    next = r->next;
    free(r);
  }

  p->start = NULL;

  flate_puts("ET\n", out);

  if ((debug = getenv("HTMLDOC_DEBUG")) != NULL && strstr(debug, "margin"))
  {
    // Show printable area...
    flate_printf(out, "1 0 1 RG 0 0 %d %d re S\n", p->width - p->right - p->left,
        	 p->length - p->top - p->bottom);
  }

 /*
  * Output the page trailer...
  */

  flate_puts("Q\n", out);
}


#ifdef DEBUG_TOC
static void
pdf_text_contents(FILE *out, tree_t *toc, int indent)
{
  static const char *spaces = "                                "
                              "                                ";

  if (indent > 16)
    indent = 16;

  while (toc)
  {
    fprintf(out, "%% %s<%s>", spaces + 64 - 4 * indent,
            _htmlMarkups[toc->markup]);

    switch (toc->markup)
    {
      case MARKUP_A :
          tree_t *temp;

          for (temp = toc->child; temp; temp = temp->next)
	    fputs((char *)temp->data, out);
          break;

      default :
          fputs("\n", out);
	  pdf_text_contents(out, toc->child, indent + 1);
	  fprintf(out, "%% %s", spaces + 64 - 4 * indent);
          break;
    }

    fprintf(out, "</%s>\n", _htmlMarkups[toc->markup]);

    toc = toc->next;
  }
}
#endif // DEBUG_TOC


/*
 * 'pdf_write_contents()' - Write the table of contents as outline records to
 *                          a PDF file.
 */

static void
pdf_write_contents(FILE   *out,			/* I - Output file */
                   tree_t *toc,			/* I - Table of contents tree */
                   int    parent,		/* I - Parent outline object */
                   int    prev,			/* I - Previous outline object */
                   int    next,			/* I - Next outline object */
                   int    *heading)		/* IO - Current heading # */
{
  int		i,				/* Looping var */
		thisobj,			/* This object */
		entry,				/* TOC entry object */
		count;				/* Number of entries at this level */
  uchar		*text;				/* Entry text */
  tree_t	*temp;				/* Looping var */
  int		*entry_counts,			/* Number of sub-entries for this entry */
		*entry_objects;			/* Objects for each entry */
  tree_t	**entries;			/* Pointers to each entry */
  float		x, y;				/* Position of link */


 /*
  * Make an object for this entry...
  */

  if (toc == NULL)
  {
   /*
    * This is for the Table of Contents page...
    */

    thisobj = pdf_start_object(out);

    fprintf(out, "/Parent %d 0 R", parent);

    fputs("/Title", out);
    write_utf16(out, (uchar *)TocTitle);

    x = 0.0f;
    y = PagePrintLength + PageBottom;
    pspdf_transform_coords(pages + chapter_starts[0], x, y);

    fprintf(out, "/Dest[%d 0 R/XYZ %.0f %.0f 0]",
            pages_object + 2 * chapter_outstarts[0] + 1, x, y);

    if (prev > 0)
      fprintf(out, "/Prev %d 0 R", prev);

    if (next > 0)
      fprintf(out, "/Next %d 0 R", next);

    pdf_end_object(out);
    return;
  }

 /*
  * Allocate the arrays...  Add 1 to hold the TOC at the top level...
  */

  if ((entry_counts = (int *)calloc(sizeof(int), num_headings + 1)) == NULL)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for %d headings - %s",
                   num_headings, strerror(errno));
    return;
  }

  if ((entry_objects = (int *)calloc(sizeof(int), num_headings + 1)) == NULL)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for %d headings - %s",
                   num_headings, strerror(errno));
    free(entry_counts);
    return;
  }

  if ((entries = (tree_t **)calloc(sizeof(tree_t *), num_headings + 1)) == NULL)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for %d headings - %s",
                   num_headings, strerror(errno));
    free(entry_objects);
    free(entry_counts);
    return;
  }

  if (parent == 0 && TocLevels > 0)
  {
   /*
    * Add the table of contents to the top-level contents...
    */

    entries[0]       = NULL;
    entry_objects[0] = num_objects + 2;
    entry            = num_objects + 3;
    count            = 1;
  }
  else
  {
    entry = num_objects + 2;
    count = 0;
  }

 /*
  * Find and count the children (entries)...
  */

  if (toc->markup == MARKUP_B && toc->next && toc->next->markup == MARKUP_UL)
    temp = toc->next->child;
  else if (toc->markup == MARKUP_LI && toc->last_child &&
           toc->last_child->markup == MARKUP_UL)
    temp = toc->last_child->child;
  else
    temp = toc->child;

  for (; temp && count <= num_headings; temp = temp->next)
  {
    if (temp->markup == MARKUP_B)
    {
      entries[count]       = temp;
      entry_objects[count] = entry;

      if (temp->next && temp->next->markup == MARKUP_UL)
        entry_counts[count] = pdf_count_headings(temp->next->child);
      else
        entry_counts[count] = 0;

      entry += entry_counts[count] + 1;
      count ++;
    }
    else if (temp->markup == MARKUP_LI)
    {
      entries[count]       = temp;
      entry_objects[count] = entry;

      if (temp->last_child && temp->last_child->markup == MARKUP_UL)
        entry_counts[count] = pdf_count_headings(temp->last_child);
      else
        entry_counts[count] = 0;

      entry += entry_counts[count] + 1;
      count ++;
    }
  }

 /*
  * Output the top-level object...
  */

  thisobj = pdf_start_object(out);

  if (parent == 0)
    outline_object = thisobj;
  else
    fprintf(out, "/Parent %d 0 R", parent);

  if (count > 0)
  {
    fprintf(out, "/Count %d", parent == 0 ? count : -count);
    fprintf(out, "/First %d 0 R", entry_objects[0]);
    fprintf(out, "/Last %d 0 R", entry_objects[count - 1]);
  }

  if (parent > 0 && toc->child && toc->child->markup == MARKUP_A)
  {
    if ((text = htmlGetText(toc->child->child)) != NULL)
    {
      fputs("/Title", out);
      write_utf16(out, text);
      free(text);
    }

    i = heading_pages[*heading];
    x = 0.0f;
    y = heading_tops[*heading] + pages[i].bottom;
    pspdf_transform_coords(pages + i, x, y);

    fprintf(out, "/Dest[%d 0 R/XYZ %.0f %.0f 0]",
            pages_object + 2 * pages[i].outpage + 1, x, y);

    (*heading) ++;
  }

  if (prev > 0)
    fprintf(out, "/Prev %d 0 R", prev);

  if (next > 0)
    fprintf(out, "/Next %d 0 R", next);

  pdf_end_object(out);

  for (i = 0; i < count ; i ++)
    pdf_write_contents(out, entries[i], thisobj, i > 0 ? entry_objects[i - 1] : 0,
                       i < (count - 1) ? entry_objects[i + 1] : 0,
                       heading);

  free(entry_objects);
  free(entry_counts);
  free(entries);
}


//
// 'pdf_write_files()' - Write an outline of HTML files.
//

static void
pdf_write_files(FILE   *out,		// I - Output file
                tree_t *doc)		// I - Document tree
{
  int		i,			// Looping var
		num_files,		// Number of FILE elements
		alloc_text;		// Allocated text?
  uchar		*text;			// Entry text
  tree_t	*temp;			// Current node
  link_t	*link;			// Link to file...
  float		x, y;			// Position of link


  // Figure out the number of (top-level) files in the document...
  for (num_files = 0, temp = doc; temp; temp = temp->next)
    if (temp->markup == MARKUP_FILE)
      num_files ++;

  if (!num_files)
  {
    // No files to outline...
    outline_object = 0;

    return;
  }

  // Write the outline dictionary...
  outline_object = pdf_start_object(out);

  fprintf(out, "/Count %d", num_files);
  fprintf(out, "/First %d 0 R", outline_object + 1);
  fprintf(out, "/Last %d 0 R", outline_object + num_files);

  pdf_end_object(out);

  // Now write the outline items...
  for (i = 0, temp = doc; temp; temp = temp->next)
    if (temp->markup == MARKUP_FILE)
    {
      alloc_text = 0;

      if ((text = get_title(temp->child)) != NULL)
        alloc_text = 1;
      else if ((text = htmlGetVariable(temp, (uchar *)"_HD_FILENAME")) == NULL)
        text = (uchar *)"Unknown";

      pdf_start_object(out);

      fprintf(out, "/Parent %d 0 R", outline_object);

      fputs("/Title", out);
      write_utf16(out, text);
      if (alloc_text)
        free(text);

      if ((link = find_link(htmlGetVariable(temp, (uchar *)"_HD_FILENAME"))) != NULL)
      {
	x = 0.0f;
	y = link->top + pages[link->page].bottom;
	pspdf_transform_coords(pages + link->page, x, y);

	fprintf(out, "/Dest[%d 0 R/XYZ %.0f %.0f 0]",
        	pages_object + 2 * pages[link->page].outpage + 1, x, y);
      }

      if (i > 0)
        fprintf(out, "/Prev %d 0 R", outline_object + i);

      if (i < (num_files - 1))
        fprintf(out, "/Next %d 0 R", outline_object + i + 2);

      pdf_end_object(out);

      i ++;
    }
}


/*
 * 'pdf_count_headings()' - Count the number of headings under this TOC
 *                          entry.
 */

static int			/* O - Number of headings found */
pdf_count_headings(tree_t *toc)	/* I - TOC entry */
{
  int	headings;		/* Number of headings */


  for (headings = 0; toc != NULL; toc = toc->next)
  {
    if (toc->markup == MARKUP_A)
      headings ++;
    if (toc->child != NULL)
      headings += pdf_count_headings(toc->child);
  }

  return (headings);
}


/*
 * PDF object state variables...
 */

static int	pdf_stream_length = 0;
static int	pdf_stream_start = 0;
static int	pdf_object_type = 0;


/*
 * 'pdf_start_object()' - Start a new PDF object...
 */

static int			// O - Object number
pdf_start_object(FILE *out,	// I - File to write to
                 int  array)	// I - 1 = array, 0 = dictionary
{
  int	*temp;			// Temporary integer pointer


  num_objects ++;

  // Allocate memory as necessary...
  if (num_objects >= alloc_objects)
  {
    alloc_objects += ALLOC_OBJECTS;

    if (alloc_objects == ALLOC_OBJECTS)
      temp = (int *)malloc(sizeof(int) * alloc_objects);
    else
      temp = (int *)realloc(objects, sizeof(int) * alloc_objects);

    if (temp == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for %d objects - %s",
                     alloc_objects, strerror(errno));
      alloc_objects -= ALLOC_OBJECTS;
      return (0);
    }

    objects = temp;
  }

  objects[num_objects] = ftell(out);
  fprintf(out, "%d 0 obj", num_objects);

  pdf_object_type = array;

  fputs(pdf_object_type ? "[" : "<<", out);

  return (num_objects);
}


/*
 * 'pdf_start_stream()' - Start a new PDF stream...
 */

static void
pdf_start_stream(FILE *out)	// I - File to write to
{
  // Write the "/Length " string, get the position, and then write 10
  // zeroes to cover the maximum size of a stream.

  fputs("/Length ", out);
  pdf_stream_length = ftell(out);
  fputs("0000000000>>stream\n", out);
  pdf_stream_start = ftell(out);
}


/*
 * 'pdf_end_object()' - End a PDF object...
 */

static void
pdf_end_object(FILE *out)	// I - File to write to
{
  int	length;			// Total length of stream


  if (pdf_stream_start)
  {
    // For streams, go back and update the length field in the
    // object dictionary...
    length = ftell(out) - pdf_stream_start;

    fseek(out, pdf_stream_length, SEEK_SET);
    fprintf(out, "%-10d", length);
    fseek(out, 0, SEEK_END);

    pdf_stream_start = 0;

    fputs("endstream\n", out);
  }
  else
    fputs(pdf_object_type ? "]" : ">>", out);

  fputs("endobj\n", out);
}


/*
 * 'pdf_write_links()' - Write annotation link objects for each page in the
 *                       document.
 */

static void
pdf_write_links(FILE *out)		/* I - Output file */
{
  int		i,			/* Looping var */
		outpage,		/* Current page */
		lobj,			/* Current link */
		num_lobjs,		/* Number of links on this page */
		alloc_lobjs,		/* Number of links to allocate */
		*lobjs;			/* Link objects */
  float		x, y;			/* Position of last link */
  render_t	*r,			/* Current render primitive */
		*rlast,			/* Last render link primitive */
		*rprev;			/* Previous render primitive */
  link_t	*link;			/* Local link */
  page_t	*p;			/* Current page */
  outpage_t	*op;			/* Current output page */


 /*
  * First combine adjacent, identical links...
  */

  for (outpage = 0, op = outpages; outpage < num_outpages; outpage ++, op ++)
  {
    for (i = 0; i < op->nup; i ++)
    {
      if (op->pages[i] < 0)
        break;

      p = pages + op->pages[i];

      for (r = p->start, x = 0.0f, y = 0.0f, rlast = NULL, rprev = NULL;
           r != NULL;
	   rprev = r, r = r->next)
	if (r->type == RENDER_LINK)
	{
          if (fabs(r->x - x) < 0.1f && fabs(r->y - y) < 0.1f &&
	      rlast != NULL && strcmp((const char *)rlast->data.link,
	                              (const char *)r->data.link) == 0)
	  {
	    // Combine this primitive with the previous one in rlast...
	    rlast->width = r->x + r->width - rlast->x;
	    x            = rlast->x + rlast->width;

	    // Delete this render primitive...
	    rprev->next = r->next;
	    free(r);
	    r = rprev;
	  }
	  else
	  {
	    // Can't combine; just save this info for later use...
	    rlast = r;
	    x     = r->x + r->width;
	    y     = r->y;
	  }
	}
    }
  }

 /*
  * Setup the initial pages_object number...
  */

  pages_object = num_objects + 1;

 /*
  * Add space for named links in PDF 1.2 output...
  */

  if (PDFVersion >= 12)
    pages_object += num_links + 3;

 /*
  * Stop here if we won't be generating links in the output...
  */

  if (!Links)
    return;

 /*
  * Figure out how many link objects we'll have...
  */

  for (outpage = 0, op = outpages, alloc_lobjs = 0;
       outpage < num_pages;
       outpage ++, op ++)
  {
    num_lobjs = 0;

    for (i = 0; i < op->nup; i ++)
    {
      if (op->pages[i] < 0)
        break;

      p = pages + op->pages[i];

      for (r = p->start; r != NULL; r = r->next)
	if (r->type == RENDER_LINK)
	{
          if (find_link(r->data.link) != NULL)
            num_lobjs ++;
          else
            num_lobjs += 2;
	}
    }

    if (num_lobjs > 0)
      pages_object += num_lobjs + 1;

    if (num_lobjs > alloc_lobjs)
      alloc_lobjs = num_lobjs;
  }

  if (alloc_lobjs == 0)
    return;

 /*
  * Allocate memory for the links...
  */

  if ((lobjs = (int *)malloc(sizeof(int) * alloc_lobjs)) == NULL)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for %d link objects - %s",
                   alloc_lobjs, strerror(errno));
    return;
  }

 /*
  * Then generate annotation objects for all the links...
  */

  for (outpage = 0, op = outpages; outpage < num_pages; outpage ++, op ++)
  {
    num_lobjs = 0;

    for (i = 0; i < op->nup; i ++)
    {
      if (op->pages[i] < 0)
        break;

      p = pages + op->pages[i];

      for (r = p->start; r != NULL; r = r->next)
	if (r->type == RENDER_LINK)
	{
          if ((link = find_link(r->data.link)) != NULL)
	  {
	   /*
            * Local link...
            */
	    float x1, y1, x2, y2;

            lobjs[num_lobjs ++] = pdf_start_object(out);

            fputs("/Subtype/Link", out);

            if (PageDuplex && (op->pages[i] & 1))
	    {
              x1 = r->x + p->right;
	      y1 = r->y + p->bottom - 2;
              x2 = r->x + r->width + p->right;
	      y2 = r->y + r->height + p->bottom;
	    }
            else
	    {
              x1 = r->x + p->left;
	      y1 = r->y + p->bottom - 2;
              x2 = r->x + r->width + p->left;
	      y2 = r->y + r->height + p->bottom;
	    }

            pspdf_transform_coords(p, x1, y1);
            pspdf_transform_coords(p, x2, y2);
            fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]", x1, y1, x2, y2);

            fputs("/Border[0 0 0]", out);

            x1 = 0.0f;
	    y1 = link->top + pages[link->page].bottom;
            pspdf_transform_coords(pages + link->page, x1, y1);
	    fprintf(out, "/Dest[%d 0 R/XYZ %.0f %.0f 0]",
        	    pages_object + 2 * pages[link->page].outpage + 1,
        	    x1, y1);
	    pdf_end_object(out);
	  }
	  else
	  {
	   /*
            * Remote link...
            */

            pdf_start_object(out);

	    if (PDFVersion >= 12 &&
        	file_method((char *)r->data.link) == NULL)
	    {
#ifdef WIN32
              if (strcasecmp(file_extension((char *)r->data.link), "pdf") == 0)
#else
              if (strcmp(file_extension((char *)r->data.link), "pdf") == 0)
#endif /* WIN32 */
              {
	       /*
		* Link to external PDF file...
		*/

        	fputs("/S/GoToR", out);
        	fputs("/D[0/XYZ null null 0]", out);
        	fputs("/F", out);
		write_string(out, r->data.link, 0);
              }
	      else
              {
	       /*
		* Link to external filename...
		*/

        	fputs("/S/Launch", out);
        	fputs("/F", out);
		write_string(out, r->data.link, 0);

		if (StrictHTML)
		  progress_error(HD_ERROR_UNRESOLVED_LINK,
		                 "Unable to resolve link to \"%s\"!",
		                 r->data.link);
              }
	    }
	    else
	    {
	     /*
	      * Link to web file...
	      */

              fputs("/S/URI", out);
              fputs("/URI", out);
	      write_string(out, r->data.link, 0);
	    }

            pdf_end_object(out);

            lobjs[num_lobjs ++] = pdf_start_object(out);

            fputs("/Subtype/Link", out);
            if (PageDuplex && (outpage & 1))
              fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                      r->x + PageRight, r->y + PageBottom,
                      r->x + r->width + PageRight, r->y + r->height + PageBottom);
            else
              fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                      r->x + PageLeft, r->y + PageBottom - 2,
                      r->x + r->width + PageLeft, r->y + r->height + PageBottom);
            fputs("/Border[0 0 0]", out);
	    fprintf(out, "/A %d 0 R", num_objects - 1);
            pdf_end_object(out);
	  }
	}
    }

    if (num_lobjs > 0)
    {
      outpages[outpage].annot_object = pdf_start_object(out, 1);

      for (lobj = 0; lobj < num_lobjs; lobj ++)
        fprintf(out, "%d 0 R%s", lobjs[lobj],
	        lobj < (num_lobjs - 1) ? "\n" : "");

      pdf_end_object(out);
    }
  }

  free(lobjs);
}


/*
 * 'pdf_write_names()' - Write named destinations for each link.
 */

static void
pdf_write_names(FILE *out)		/* I - Output file */
{
  int		i;			/* Looping var */
  uchar		*s;			/* Current character in name */
  link_t	*link;			/* Local link */


 /*
  * Convert all link names to lowercase...
  */

  for (i = num_links, link = links; i > 0; i --, link ++)
    for (s = link->name; *s != '\0'; s ++)
      *s = tolower(*s);

 /*
  * Write the root name tree entry...
  */

  names_object = pdf_start_object(out);
  fprintf(out, "/Dests %d 0 R", num_objects + 1);
  pdf_end_object(out);

 /*
  * Write the name tree child list...
  */

  pdf_start_object(out);
  fprintf(out, "/Kids[%d 0 R]", num_objects + 1);
  pdf_end_object(out);

 /*
  * Write the leaf node for the name tree...
  */

  pdf_start_object(out);

  fputs("/Limits[", out);
  write_string(out, links[0].name, 0);
  write_string(out, links[num_links - 1].name, 0);
  fputs("]", out);

  fputs("/Names[", out);
  for (i = 1, link = links; i <= num_links; i ++, link ++)
  {
    write_string(out, link->name, 0);
    fprintf(out, "%d 0 R", num_objects + i);
  }
  fputs("]", out);

  pdf_end_object(out);

  for (i = num_links, link = links; i > 0; i --, link ++)
  {
    pdf_start_object(out);
    float x, y;

    x = 0.0f;
    y = link->top + pages[link->page].bottom;
    pspdf_transform_coords(pages + link->page, x, y);
    fprintf(out, "/D[%d 0 R/XYZ %.0f %.0f 0]",
            pages_object + 2 * pages[link->page].outpage + 1, x, y);
    pdf_end_object(out);
  }
}


/*
 * 'render_contents()' - Render a single heading.
 */

static void
render_contents(tree_t *t,		/* I - Tree to parse */
                float  left,		/* I - Left margin */
                float  right,		/* I - Printable width */
                float  bottom,		/* I - Bottom margin */
                float  top,		/* I - Printable top */
                float  *y,		/* IO - Y position */
                int    *page,		/* IO - Page # */
	        int    heading,		/* I - Heading # */
	        tree_t *chap)		/* I - Chapter heading */
{
  float		x,
		width,
		numberwidth,
		height,
		rgb[3];
  int		hpage;
  uchar		number[1024],
		*nptr,
		*link;
  tree_t	*flat,
		*temp,
		*next;
  render_t	*r;
#define dot_width  (_htmlSizes[SIZE_P] * _htmlWidths[t->typeface][t->style]['.'])


  DEBUG_printf(("render_contents(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, left, right, bottom, top, *y, *page, heading, chap));

  if (!t)
    return;

 /*
  * Put the text...
  */

  flat = flatten_tree(t->child->child);

  for (height = 0.0, temp = flat; temp != NULL; temp = temp->next)
    if (temp->height > height)
      height = temp->height;

  height *= _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P];

  if (t->indent)
    x = left + 18.0f + 18.0f * t->indent;
  else
    x = left;

  *y -= height;

 /*
  * Get the width of the page number, leave room for three dots...
  */

  if (heading >= 0 && heading < num_headings)
  {
    hpage       = heading_pages[heading];
    numberwidth = get_width((uchar *)pages[hpage].page_text,
                            t->typeface, t->style, t->size) +
	          3.0f * dot_width;

    fprintf(stderr, "heading %d on page %d: %s\n", heading, hpage, pages[hpage].page_text);
  }
  else
  {
    hpage       = 0;
    numberwidth = 0.0f;
  }

  for (temp = flat; temp != NULL; temp = next)
  {
    rgb[0] = temp->red / 255.0f;
    rgb[1] = temp->green / 255.0f;
    rgb[2] = temp->blue / 255.0f;

    if ((x + temp->width) >= (right - numberwidth))
    {
     /*
      * Too wide to fit, continue on the next line
      */

      *y -= _htmlSpacings[SIZE_P];
      x  = left + 36.0f * t->indent;
    }

    if (*y < bottom)
    {
      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);

      width = get_width((uchar *)TocTitle, _htmlHeadingFont, STYLE_BOLD, SIZE_H1);
      *y = top - _htmlSpacings[SIZE_H1];
      x  = left + 0.5f * (right - left - width);
      r = new_render(*page, RENDER_TEXT, x, *y, 0, 0, TocTitle);
      r->data.text.typeface = _htmlHeadingFont;
      r->data.text.style    = STYLE_BOLD;
      r->data.text.size     = _htmlSizes[SIZE_H1];
      get_color(_htmlTextColor, r->data.text.rgb);

      *y -= _htmlSpacings[SIZE_H1];

      if (t->indent)
	x = left + 18.0f + 18.0f * t->indent;
      else
	x = left;

      if (chap != t)
      {
        *y += height;
        render_contents(chap, left, right, bottom, top, y, page, -1, 0);
	*y -= _htmlSpacings[SIZE_P];
      }
    }

    if (temp->link != NULL)
    {
      link = htmlGetVariable(temp->link, (uchar *)"HREF");

     /*
      * Add a page link...
      */

      if (file_method((char *)link) == NULL &&
	  file_target((char *)link) != NULL)
	link = (uchar *)file_target((char *)link) - 1; // Include # sign

      new_render(*page, RENDER_LINK, x, *y, temp->width,
	         temp->height, link);

      if (PSLevel == 0 && Links)
      {
        memcpy(rgb, link_color, sizeof(rgb));

	temp->red   = (int)(link_color[0] * 255.0);
	temp->green = (int)(link_color[1] * 255.0);
	temp->blue  = (int)(link_color[2] * 255.0);

        if (LinkStyle)
	  new_render(*page, RENDER_BOX, x, *y - 1, temp->width, 0,
	             link_color);
      }
    }

    switch (temp->markup)
    {
      case MARKUP_A :
          if ((link = htmlGetVariable(temp, (uchar *)"NAME")) != NULL)
          {
           /*
            * Add a target link...
            */

            add_link(link, *page, (int)(*y + height));
          }
          break;

      case MARKUP_NONE :
          if (temp->data == NULL)
            break;

	  if (temp->underline)
	    new_render(*page, RENDER_BOX, x, *y - 1, temp->width, 0, rgb);

	  if (temp->strikethrough)
	    new_render(*page, RENDER_BOX, x, *y + temp->height * 0.25f,
		       temp->width, 0, rgb);

          r = new_render(*page, RENDER_TEXT, x, *y, 0, 0, temp->data);
          r->data.text.typeface = temp->typeface;
          r->data.text.style    = temp->style;
          r->data.text.size     = _htmlSizes[temp->size];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

          if (temp->superscript)
            r->y += height - temp->height;
          else if (temp->subscript)
            r->y -= height * _htmlSizes[0] / _htmlSpacings[0] -
		    temp->height;
	  break;

      case MARKUP_IMG :
	  update_image_size(temp);
	  new_render(*page, RENDER_IMAGE, x, *y, temp->width, temp->height,
		     image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));
	  break;

      default :
	  break;
    }

    x += temp->width;
    next = temp->next;
    free(temp);
  }

  if (numberwidth > 0.0f)
  {
   /*
    * Draw dots leading up to the page number...
    */

    width = numberwidth - 3.0 * dot_width + x;

    for (nptr = number;
         nptr < (number + sizeof(number) - 1) && width < right;
	 width += dot_width)
      *nptr++ = '.';
    nptr --;

    strlcpy((char *)nptr, pages[hpage].page_text,
            sizeof(number) - (nptr - number));

    r = new_render(*page, RENDER_TEXT, right - width + x, *y, 0, 0, number);
    r->data.text.typeface = t->typeface;
    r->data.text.style    = t->style;
    r->data.text.size     = _htmlSizes[t->size];
    memcpy(r->data.text.rgb, rgb, sizeof(rgb));
  }
}


/*
 * 'count_headings()' - Count the number of headings in the TOC.
 */

static int
count_headings(tree_t *t)		// I - Tree to count
{
  int	count;				// Number of headings...


  count = 0;

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_B :
      case MARKUP_LI :
          count ++;
	  if (t->last_child && t->last_child->markup == MARKUP_UL)
	    count += count_headings(t->last_child);
	  break;

      default :
          count += count_headings(t->child);
          break;
    }

    t = t->next;
  }

  return (count);
}


/*
 * 'parse_contents()' - Parse the table of contents and produce a
 *                      rendering list...
 */

static void
parse_contents(tree_t *t,		/* I - Tree to parse */
               float  left,		/* I - Left margin */
               float  right,		/* I - Printable width */
               float  bottom,		/* I - Bottom margin */
               float  top,		/* I - Printable top */
               float  *y,		/* IO - Y position */
               int    *page,		/* IO - Page # */
               int    *heading,		/* IO - Heading # */
	       tree_t *chap)		/* I - Chapter heading */
{
  DEBUG_printf(("parse_contents(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, left, right, bottom, top, *y, *page, *heading, chap));

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_B :	/* Top-level TOC */
          if (t->prev != NULL)	/* Advance one line prior to top-levels... */
            *y -= _htmlSpacings[SIZE_P];

          if (*y < (bottom + _htmlSpacings[SIZE_P] * 3))
	    *y = 0; // Force page break

          chap = t;

      case MARKUP_LI :	/* Lower-level TOC */
          DEBUG_printf(("parse_contents: heading=%d, page = %d\n", *heading,
                        heading_pages[*heading]));

         /*
          * Put the text unless the author has flagged it otherwise...
          */

          if (htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
	  {
            render_contents(t, left, right, bottom, top, y, page,
	                    *heading, chap);

           /*
	    * Update current headings for header/footer strings in TOC.
	    */

	    check_pages(*page);

	    if (t->markup == MARKUP_B &&
		pages[*page].chapter == pages[*page - 1].chapter)
	      pages[*page].chapter = htmlGetText(t->child->child);

	    if (pages[*page].heading == pages[*page - 1].heading)
	      pages[*page].heading = htmlGetText(t->child->child);

           /*
            * Next heading...
            */

            (*heading) ++;

            if (t->last_child->markup == MARKUP_UL)
              parse_contents(t->last_child, left, right, bottom, top, y,
	                     page, heading, chap);
          }
	  else if (t->next != NULL && t->next->markup == MARKUP_UL)
	  {
	   /*
	    * Skip children of omitted heading...
	    */

	    t = t->next;

	    (*heading) += count_headings(t->child) + 1;
	  }
	  else
	    (*heading) ++;
          break;

      default :
          parse_contents(t->child, left, right, bottom, top, y, page, heading,
	                 chap);
          break;
    }

    t = t->next;
  }
}


/*
 * 'parse_doc()' - Parse a document tree and produce rendering list output.
 */

static void
parse_doc(tree_t *t,		/* I - Tree to parse */
          float  *left,		/* I - Left margin */
          float  *right,	/* I - Printable width */
          float  *bottom,	/* I - Bottom margin */
          float  *top,		/* I - Printable top */
          float  *x,		/* IO - X position */
          float  *y,		/* IO - Y position */
          int    *page,		/* IO - Page # */
	  tree_t *cpara,	/* I - Current paragraph */
	  int    *needspace)	/* I - Need whitespace before this element */
{
  int		i;		/* Looping var */
  tree_t	*para,		/* Phoney paragraph tree entry */
		*temp;		/* Paragraph entry */
  var_t		*var;		/* Variable entry */
  uchar		*name;		/* ID name */
  uchar		*style;		/* STYLE attribute */
  float		width,		/* Width of horizontal rule */
		height,		/* Height of rule */
		rgb[3];		/* RGB color of rule */


  DEBUG_printf(("parse_doc(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, cpara=%p, needspace=%d\n",
                t, *left, *right, *bottom, *top, *x, *y, *page, cpara,
	        *needspace));
  DEBUG_printf(("    title_page = %d, chapter = %d\n", title_page, chapter));

  if (cpara == NULL)
    para = htmlNewTree(NULL, MARKUP_P, NULL);
  else
    para = cpara;

  while (t != NULL)
  {
    if (((t->markup == MARKUP_H1 && OutputType == OUTPUT_BOOK) ||
         (t->markup == MARKUP_FILE && OutputType == OUTPUT_WEBPAGES)) &&
	!title_page)
    {
      // New page on H1 in book mode or file in webpage mode...
      if (para->child != NULL && chapter > 0)
      {
        parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
        htmlDeleteTree(para->child);
        para->child = para->last_child = NULL;
      }

      if ((chapter > 0 && OutputType == OUTPUT_BOOK) ||
          ((*page > 1 || *y < *top) && OutputType == OUTPUT_WEBPAGES))
      {
        if (*y < *top)
          (*page) ++;

        if (PageDuplex && (*page & 1))
          (*page) ++;

        if (Verbosity)
          progress_show("Formatting page %d", *page);

        chapter_ends[chapter] = *page - 1;
      }

      // Make sure header and footer strings are correct...
      check_pages(*page);

      memcpy(pages[*page].header, Header, sizeof(pages[*page].header));
      memcpy(pages[*page].header1, Header1, sizeof(pages[*page].header1));
      memcpy(pages[*page].footer, Footer, sizeof(pages[*page].footer));

      // Bump the chapter/file count...
      chapter ++;
      if (chapter >= MAX_CHAPTERS)
      {
	progress_error(HD_ERROR_TOO_MANY_CHAPTERS,
	               "Too many chapters/files in document (%d > %d)!",
	               chapter, MAX_CHAPTERS);
        chapter = MAX_CHAPTERS - 1;
      }
      else
        chapter_starts[chapter] = *page;

      if (chapter > TocDocCount)
	TocDocCount = chapter;

      *y         = *top;
      *x         = *left;
      *needspace = 0;
    }

    if ((name = htmlGetVariable(t, (uchar *)"ID")) != NULL)
    {
     /*
      * Add a link target using the ID=name variable...
      */

      add_link(name, *page, (int)*y);
    }
    else if (t->markup == MARKUP_FILE)
    {
     /*
      * Add a file link...
      */

      uchar	newname[256],	/* New filename */
		*sep;		/* "?" separator in links */


      // Strip any trailing HTTP GET data stuff...
      strlcpy((char *)newname, (char *)htmlGetVariable(t, (uchar *)"_HD_FILENAME"),
              sizeof(newname));

      if ((sep = (uchar *)strchr((char *)newname, '?')) != NULL)
        *sep = '\0';

      // Add the link
      add_link(newname, *page, (int)*y);
    }

    if (chapter == 0 && !title_page)
    {
      // Need to handle page comments before the first heading...
      if (t->markup == MARKUP_COMMENT)
        parse_comment(t, left, right, bottom, top, x, y, page, para,
	              *needspace);

      if (t->child != NULL)
        parse_doc(t->child, left, right, bottom, top, x, y, page, para,
	          needspace);

      t = t->next;
      continue;
    }

    // Check for some basic stylesheet stuff...
    if ((style = htmlGetStyle(t, (uchar *)"page-break-before:")) != NULL &&
	strcasecmp((char *)style, "avoid") != 0)
    {
      // Advance to the next page...
      (*page) ++;
      *x         = *left;
      *y         = *top;
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (PageDuplex && ((*page) & 1) &&
          strcasecmp((char *)style, "right") == 0)
	(*page) ++;
      else if (PageDuplex && !((*page) & 1) &&
               strcasecmp((char *)style, "left") == 0)
	(*page) ++;

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    // Process the markup...
    switch (t->markup)
    {
      case MARKUP_IMG :
          update_image_size(t);
      case MARKUP_NONE :
      case MARKUP_BR :
          if (para->child == NULL)
          {
	    if (t->parent == NULL)
	    {
              para->halignment = ALIGN_LEFT;
              para->indent     = 0;
	    }
	    else
	    {
              para->halignment = t->parent->halignment;
              para->indent     = t->parent->indent;
	    }
          }

	  // Skip heading whitespace...
          if (para->child == NULL && t->markup == MARKUP_NONE &&
	      t->data != NULL && strcmp((char *)t->data, " ") == 0)
	    break;

          if ((temp = htmlAddTree(para, t->markup, t->data)) != NULL)
          {
	    temp->link          = t->link;
            temp->width         = t->width;
            temp->height        = t->height;
            temp->typeface      = t->typeface;
            temp->style         = t->style;
            temp->size          = t->size;
            temp->underline     = t->underline;
            temp->strikethrough = t->strikethrough;
            temp->superscript   = t->superscript;
            temp->subscript     = t->subscript;
            temp->halignment    = t->halignment;
            temp->valignment    = t->valignment;
            temp->red           = t->red;
            temp->green         = t->green;
            temp->blue          = t->blue;
            for (i = 0, var = t->vars; i < t->nvars; i ++, var ++)
              htmlSetVariable(temp, var->name, var->value);
          }
          break;

      case MARKUP_TABLE :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_table(t, *left, *right, *bottom, *top, x, y, page, *needspace);
	  *needspace = 0;
          break;

      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          parse_heading(t, *left, *right, *bottom, *top, x, y, page, *needspace);
	  *needspace = 1;
          break;

      case MARKUP_BLOCKQUOTE :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          *left  += 36;
	  *right -= 36;

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *left  -= 36;
	  *right += 36;

          *x         = *left;
          *needspace = 1;
          break;

      case MARKUP_CENTER :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

            *needspace = 1;
          }

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case MARKUP_P :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case MARKUP_DIV :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }
          break;

      case MARKUP_PRE :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          parse_pre(t, *left, *right, *bottom, *top, x, y, page, *needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case MARKUP_DIR :
      case MARKUP_MENU :
      case MARKUP_UL :
      case MARKUP_OL :
          init_list(t);
      case MARKUP_DL :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          if (t->indent == 1)
	    *needspace = 1;

	  *left += 36.0f;
          *x    = *left;

          parse_doc(t->child, left, right, bottom, top, x, y, page, para,
	            needspace);

          *left -= 36.0f;

          if (t->indent == 1)
	    *needspace = 1;
          break;

      case MARKUP_LI :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          parse_list(t, left, right, bottom, top, x, y, page, *needspace);

          *x         = *left;
          *needspace = t->next && t->next->markup != MARKUP_LI &&
	               t->next->markup != MARKUP_UL &&
		       t->next->markup != MARKUP_OL;
          break;

      case MARKUP_DT :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

	  *left -= 36.0f;
          *x    = *left;

          parse_doc(t->child, left, right, bottom, top, x, y, page,
	            NULL, needspace);

	  *left      += 36.0f;
          *x         = *left;
          *needspace = 0;
          break;

      case MARKUP_DD :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 0;
          break;

      case MARKUP_HR :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          if (htmlGetVariable(t, (uchar *)"BREAK") == NULL)
	  {
	   /*
	    * Generate a horizontal rule...
	    */

            if ((name = htmlGetVariable(t, (uchar *)"WIDTH")) == NULL)
	      width = *right - *left;
	    else
	    {
	      if (strchr((char *)name, '%') != NULL)
	        width = atoi((char *)name) * (*right - *left) / 100;
	      else
                width = atoi((char *)name) * PagePrintWidth / _htmlBrowserWidth;
            }

            if ((name = htmlGetVariable(t, (uchar *)"SIZE")) == NULL)
	      height = 2;
	    else
	      height = atoi((char *)name) * PagePrintWidth / _htmlBrowserWidth;

            switch (t->halignment)
	    {
	      case ALIGN_LEFT :
	          *x = *left;
		  break;
	      case ALIGN_CENTER :
	          *x = *left + (*right - *left - width) * 0.5f;
		  break;
	      case ALIGN_RIGHT :
	          *x = *right - width;
		  break;
	    }

            if (*y < (*bottom + height + _htmlSpacings[SIZE_P]))
	    {
	     /*
	      * Won't fit on this page...
	      */

              (*page) ++;
	      if (Verbosity)
	        progress_show("Formatting page %d", *page);
              *y = *top;
            }

            (*y)   -= height + _htmlSpacings[SIZE_P];
            rgb[0] = t->red / 255.0f;
            rgb[1] = t->green / 255.0f;
            rgb[2] = t->blue / 255.0f;

            new_render(*page, RENDER_BOX, *x, *y + _htmlSpacings[SIZE_P] * 0.5,
	               width, height, rgb);
	  }
	  else
	  {
	   /*
	    * <HR BREAK> generates a page break...
	    */

            (*page) ++;
	    if (Verbosity)
	      progress_show("Formatting page %d", *page);
            *y = *top;
	  }

          *x         = *left;
          *needspace = 0;
          break;

      case MARKUP_COMMENT :
          // Check comments for commands...
          parse_comment(t, left, right, bottom, top, x, y, page, para,
	                *needspace);
          break;

      case MARKUP_HEAD : // Ignore document HEAD section
      case MARKUP_TITLE : // Ignore title and meta stuff
      case MARKUP_META :
      case MARKUP_SCRIPT : // Ignore script stuff
      case MARKUP_INPUT : // Ignore form stuff
      case MARKUP_SELECT :
      case MARKUP_OPTION :
      case MARKUP_TEXTAREA :
          break;

      case MARKUP_STYLE :
          break;

      case MARKUP_A :
          if (htmlGetVariable(t, (uchar *)"NAME") != NULL)
	  {
	   /*
	    * Add this named destination to the paragraph tree...
	    */

            if (para->child == NULL)
            {
              para->halignment = t->halignment;
              para->indent     = t->indent;
            }

            if ((temp = htmlAddTree(para, t->markup, t->data)) != NULL)
            {
	      temp->link          = t->link;
              temp->width         = t->width;
              temp->height        = t->height;
              temp->typeface      = t->typeface;
              temp->style         = t->style;
              temp->size          = t->size;
              temp->underline     = t->underline;
              temp->strikethrough = t->strikethrough;
              temp->superscript   = t->superscript;
              temp->subscript     = t->subscript;
              temp->halignment    = t->halignment;
              temp->valignment    = t->valignment;
              temp->red           = t->red;
              temp->green         = t->green;
              temp->blue          = t->blue;
              for (i = 0, var = t->vars; i < t->nvars; i ++, var ++)
        	htmlSetVariable(temp, var->name, var->value);
            }
	  }

      default :
	  if (t->child != NULL)
            parse_doc(t->child, left, right, bottom, top, x, y, page, para,
	              needspace);
          break;
    }


    // Check for some basic stylesheet stuff...
    if ((style = htmlGetStyle(t, (uchar *)"page-break-after:")) != NULL &&
	strcasecmp((char *)style, "avoid") != 0)
    {
      // Advance to the next page...
      (*page) ++;
      *x         = *left;
      *y         = *top;
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (PageDuplex && ((*page) & 1) &&
          strcasecmp((char *)style, "right") == 0)
	(*page) ++;
      else if (PageDuplex && !((*page) & 1) &&
               strcasecmp((char *)style, "left") == 0)
	(*page) ++;

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    // Move to the next node...
    t = t->next;
  }

  if (para->child != NULL && cpara != para)
  {
    parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
    *needspace  = 0;
  }

  if (cpara != para)
    htmlDeleteTree(para);

  DEBUG_printf(("LEAVING parse_doc(), x = %.1f, y = %.1f, page = %d\n",
                *x, *y, *page));
}


/*
 * 'parse_heading()' - Parse a heading tree and produce rendering list output.
 */

static void
parse_heading(tree_t *t,	/* I - Tree to parse */
              float  left,	/* I - Left margin */
              float  right,	/* I - Printable width */
              float  bottom,	/* I - Bottom margin */
              float  top,	/* I - Printable top */
              float  *x,	/* IO - X position */
              float  *y,	/* IO - Y position */
              int    *page,	/* IO - Page # */
              int    needspace)	/* I - Need whitespace? */
{
  int	*temp;			// Temporary integer array pointer


  DEBUG_printf(("parse_heading(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                t, left, right, bottom, top, *x, *y, *page, needspace));

  if (((t->markup - MARKUP_H1) < TocLevels || TocLevels == 0) && !title_page)
    current_heading = t->child;

  if (*y < (5 * _htmlSpacings[SIZE_P] + bottom))
  {
    (*page) ++;
    *y = top;
    if (Verbosity)
      progress_show("Formatting page %d", *page);
  }

  check_pages(*page);

  if (t->markup == MARKUP_H1 && !title_page)
    pages[*page].chapter = htmlGetText(current_heading);

  if ((pages[*page].heading == NULL || t->markup == MARKUP_H1 ||
      (*page > 0 && pages[*page].heading == pages[*page - 1].heading)) &&
      !title_page)
  {
    pages[*page].heading  = htmlGetText(current_heading);
    pages[*page].headnode = current_heading;
  }

  if ((t->markup - MARKUP_H1) < TocLevels && !title_page)
  {
    DEBUG_printf(("H%d: heading_pages[%d] = %d\n", t->markup - MARKUP_H1 + 1,
                  num_headings, *page - 1));
    fprintf(stderr, "H%d: heading_pages[%d] = %d\n", t->markup - MARKUP_H1 + 1, num_headings, *page - 1);

    // See if we need to resize the headings arrays...
    if (num_headings >= alloc_headings)
    {
      alloc_headings += ALLOC_HEADINGS;

      if (num_headings == 0)
        temp = (int *)malloc(sizeof(int) * alloc_headings);
      else
        temp = (int *)realloc(heading_pages, sizeof(int) * alloc_headings);

      if (temp == NULL)
      {
        progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for %d headings - %s",
	               alloc_headings, strerror(errno));
	alloc_headings -= ALLOC_HEADINGS;
	return;
      }

      memset(temp + alloc_headings - ALLOC_HEADINGS, 0,
             sizeof(int) * ALLOC_HEADINGS);

      heading_pages = temp;

      if (num_headings == 0)
        temp = (int *)malloc(sizeof(int) * alloc_headings);
      else
        temp = (int *)realloc(heading_tops, sizeof(int) * alloc_headings);

      if (temp == NULL)
      {
        progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for %d headings - %s",
	               alloc_headings, strerror(errno));
	alloc_headings -= ALLOC_HEADINGS;
	return;
      }

      memset(temp + alloc_headings - ALLOC_HEADINGS, 0,
             sizeof(int) * ALLOC_HEADINGS);

      heading_tops = temp;
    }

    heading_pages[num_headings] = *page;
    heading_tops[num_headings]  = (int)(*y + 4 * _htmlSpacings[SIZE_P]);
    num_headings ++;
  }

  parse_paragraph(t, left, right, bottom, top, x, y, page, needspace);

  if (t->halignment == ALIGN_RIGHT && t->markup == MARKUP_H1 &&
      OutputType == OUTPUT_BOOK && !title_page)
  {
   /*
    * Special case - chapter heading for users manual...
    */

    *y = bottom + 0.5f * (top - bottom);
  }
}


/*
 * 'parse_paragraph()' - Parse a paragraph tree and produce rendering list
 *                       output.
 */

static void
parse_paragraph(tree_t *t,	/* I - Tree to parse */
        	float  left,	/* I - Left margin */
        	float  right,	/* I - Printable width */
        	float  bottom,	/* I - Bottom margin */
        	float  top,	/* I - Printable top */
        	float  *x,	/* IO - X position */
        	float  *y,	/* IO - Y position */
        	int    *page,	/* IO - Page # */
        	int    needspace)/* I - Need whitespace? */
{
  int		whitespace;	/* Non-zero if a fragment ends in whitespace */
  tree_t	*flat,
		*start,
		*end,
		*prev,
		*temp;
  float		width,
		height,
		offset,
		spacing,
		borderspace,
		temp_y,
		temp_width,
		temp_height;
  float		format_width, image_y, image_left, image_right;
  float		char_spacing;
  int		num_chars;
  render_t	*r;
  uchar		*align,
		*hspace,
		*vspace,
		*link,
		*border;
  float		rgb[3];
  uchar		line[10240],
		*lineptr,
		*dataptr;
  tree_t	*linetype;
  float		linex,
		linewidth;
  int		firstline;


  DEBUG_printf(("parse_paragraph(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                t, left, right, bottom, top, *x, *y, *page, needspace));

  flat        = flatten_tree(t->child);
  image_left  = left;
  image_right = right;
  image_y     = 0;

  if (flat == NULL)
    DEBUG_puts("parse_paragraph: flat == NULL!");

  // Add leading whitespace...
  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

 /*
  * First scan for images with left/right alignment tags...
  */

  for (temp = flat, prev = NULL; temp != NULL;)
  {
    if (temp->markup == MARKUP_IMG)
      update_image_size(temp);

    if (temp->markup == MARKUP_IMG &&
        (align = htmlGetVariable(temp, (uchar *)"ALIGN")))
    {
      if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	borderspace = atof((char *)border);
      else if (temp->link)
	borderspace = 1;
      else
	borderspace = 0;

      borderspace *= PagePrintWidth / _htmlBrowserWidth;

      if (strcasecmp((char *)align, "LEFT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, (uchar *)"VSPACE")) != NULL)
	  *y -= atoi((char *)vspace);

        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  (*page) ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->red / 255.0f;
	    rgb[1] = temp->green / 255.0f;
	    rgb[2] = temp->blue / 255.0f;
	  }

	  // Top
          new_render(*page, RENDER_BOX, image_left, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          new_render(*page, RENDER_BOX, image_left,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, RENDER_BOX, image_left + temp->width + borderspace,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          new_render(*page, RENDER_BOX, image_left,
	             *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        *y -= borderspace;

        new_render(*page, RENDER_IMAGE, image_left + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));

        if (temp->link &&
	    (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
        {
	 /*
	  * Add a page link...
	  */

	  if (file_method((char *)link) == NULL)
	  {
	    if (file_target((char *)link) != NULL)
	      link = (uchar *)file_target((char *)link) - 1; // Include # sign
	    else
	      link = (uchar *)file_basename((char *)link);
	  }

	  new_render(*page, RENDER_LINK, image_left + borderspace,
	             *y - temp->height, temp->width, temp->height, link);
        }

        *y -= borderspace;

        if (vspace != NULL)
	  *y -= atoi((char *)vspace);

        image_left += temp->width + 2 * borderspace;
	temp_y     = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

        if ((hspace = htmlGetVariable(temp, (uchar *)"HSPACE")) != NULL)
	  image_left += atoi((char *)hspace);

        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;

        free(temp);
        temp = prev;
      }
      else if (strcasecmp((char *)align, "RIGHT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, (uchar *)"VSPACE")) != NULL)
	  *y -= atoi((char *)vspace);

        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  (*page) ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

        image_right -= temp->width + 2 * borderspace;

        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->red / 255.0f;
	    rgb[1] = temp->green / 255.0f;
	    rgb[2] = temp->blue / 255.0f;
	  }

	  // Top
          new_render(*page, RENDER_BOX, image_right, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          new_render(*page, RENDER_BOX, image_right,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, RENDER_BOX, image_right + temp->width + borderspace,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          new_render(*page, RENDER_BOX, image_right, *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        *y -= borderspace;

        new_render(*page, RENDER_IMAGE, image_right + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));

        if (temp->link &&
	    (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
        {
	 /*
	  * Add a page link...
	  */

	  if (file_method((char *)link) == NULL)
	  {
	    if (file_target((char *)link) != NULL)
	      link = (uchar *)file_target((char *)link) - 1; // Include # sign
	    else
	      link = (uchar *)file_basename((char *)link);
	  }

	  new_render(*page, RENDER_LINK, image_right + borderspace,
	             *y - temp->height, temp->width, temp->height, link);
        }

        *y -= borderspace;

        if (vspace != NULL)
	  *y -= atoi((char *)vspace);

	temp_y = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

        if ((hspace = htmlGetVariable(temp, (uchar *)"HSPACE")) != NULL)
	  image_right -= atoi((char *)hspace);

        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;

        free(temp);
        temp = prev;
      }
    }

    if (temp != NULL)
    {
      prev = temp;
      temp = temp->next;
    }
    else
      temp = flat;
  }

 /*
  * Then format the text and inline images...
  */

  format_width = image_right - image_left;
  firstline    = 1;

  DEBUG_printf(("format_width = %.1f\n", format_width));

  // Make stupid compiler warnings go away (if you can't put
  // enough smarts in the compiler, don't add the warning!)
  offset      = 0.0f;
  temp_width  = 0.0f;
  temp_height = 0.0f;
  lineptr     = NULL;
  linex       = 0.0f;
  linewidth   = 0.0f;

  while (flat != NULL)
  {
    start = flat;
    end   = flat;
    width = 0.0;

    while (flat != NULL)
    {
      // Get fragments...
      temp_width = 0.0;
      temp       = flat;
      whitespace = 0;

      while (temp != NULL && !whitespace)
      {
        if (temp->markup == MARKUP_NONE && temp->data[0] == ' ')
	{
          if (temp == start)
            temp_width -= _htmlWidths[temp->typeface][temp->style][' '] *
                          _htmlSizes[temp->size];
          else if (temp_width > 0.0f)
	    whitespace = 1;
	}
        else
          whitespace = 0;

        if (whitespace)
	  break;

        if (temp->markup == MARKUP_IMG)
	{
	  if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	    borderspace = atof((char *)border);
	  else if (temp->link)
	    borderspace = 1;
	  else
	    borderspace = 0;

          borderspace *= PagePrintWidth / _htmlBrowserWidth;

          temp_width += 2 * borderspace;
	}

        prev       = temp;
        temp       = temp->next;
        temp_width += prev->width;

        if ((temp_width >= format_width && prev->markup == MARKUP_IMG) ||
	    prev->markup == MARKUP_BR)
	  break;
      }

      if ((width + temp_width) <= format_width)
      {
        width += temp_width;
        end  = temp;
        flat = temp;

        if (prev->markup == MARKUP_BR)
          break;
      }
      else if (width == 0.0)
      {
        width += temp_width;
        end  = temp;
        flat = temp;
        break;
      }
      else
        break;
    }

    if (start == end)
    {
      end   = start->next;
      flat  = start->next;
      width = start->width;
    }

    for (height = 0.0, num_chars = 0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;

      if (temp->markup == MARKUP_NONE)
        num_chars += strlen((char *)temp->data);

      if (temp->height > height)
        height = temp->height;
    }

    for (spacing = 0.0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;

      if (temp->markup != MARKUP_IMG)
        temp_height = temp->height * _htmlSpacings[0] / _htmlSizes[0];
      else
      {
	if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	  borderspace = atof((char *)border);
	else if (temp->link)
	  borderspace = 1;
	else
	  borderspace = 0;

        borderspace *= PagePrintWidth / _htmlBrowserWidth;

        temp_height = temp->height + 2 * borderspace;
      }

      if (temp_height > spacing)
        spacing = temp_height;
    }

    if (firstline && end != NULL && *y < (bottom + height + _htmlSpacings[t->size]))
    {
      // Go to next page since only 1 line will fit on this one...
      (*page) ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    firstline = 0;

    if (height == 0.0f)
      height = spacing;

    for (temp = start; temp != end; temp = temp->next)
      if (temp->markup != MARKUP_A)
        break;

    if (temp != NULL && temp->markup == MARKUP_NONE && temp->data[0] == ' ')
    {
      // Drop leading space...
      for (dataptr = temp->data; *dataptr; dataptr ++)
        *dataptr = dataptr[1];
      *dataptr = '\0';

      temp_width = _htmlWidths[temp->typeface][temp->style][' '] *
                   _htmlSizes[temp->size];
      temp->width -= temp_width;
      num_chars --;
    }

    if (end != NULL)
      temp = end->prev;
    else
      temp = NULL;

    if (*y < (spacing + bottom))
    {
      (*page) ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    *y -= height;

    DEBUG_printf(("    y = %.1f, width = %.1f, height = %.1f\n", *y, width,
                  height));

    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / PagePrintLength));

    char_spacing = 0.0f;
    whitespace   = 0;
    temp         = start;
    linetype     = NULL;

    rgb[0] = temp->red / 255.0f;
    rgb[1] = temp->green / 255.0f;
    rgb[2] = temp->blue / 255.0f;

    switch (t->halignment)
    {
      case ALIGN_LEFT :
          linex = image_left;
	  break;

      case ALIGN_CENTER :
          linex = image_left + 0.5f * (format_width - width);
	  break;

      case ALIGN_RIGHT :
          linex = image_right - width;
	  break;

      case ALIGN_JUSTIFY :
          linex = image_left;
	  if (flat != NULL && flat->prev->markup != MARKUP_BR && num_chars > 1)
	    char_spacing = (format_width - width) / (num_chars - 1);
	  break;
    }

    while (temp != end)
    {
      if (temp->link != NULL && PSLevel == 0 && Links &&
          temp->markup == MARKUP_NONE)
      {
	temp->red   = (int)(link_color[0] * 255.0);
	temp->green = (int)(link_color[1] * 255.0);
	temp->blue  = (int)(link_color[2] * 255.0);
      }

     /*
      * See if we are doing a run of characters in a line and need to
      * output this run...
      */

      if (linetype != NULL &&
	  (temp->markup != MARKUP_NONE ||
	   temp->typeface != linetype->typeface ||
	   temp->style != linetype->style ||
	   temp->size != linetype->size ||
	   temp->superscript != linetype->superscript ||
	   temp->subscript != linetype->subscript ||
	   temp->red != linetype->red ||
	   temp->green != linetype->green ||
	   temp->blue != linetype->blue))
      {
        r = new_render(*page, RENDER_TEXT, linex - linewidth, *y,
	               linewidth, linetype->height, line);
	r->data.text.typeface = linetype->typeface;
	r->data.text.style    = linetype->style;
	r->data.text.size     = _htmlSizes[linetype->size];
	r->data.text.spacing  = char_spacing;
        memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	if (linetype->superscript)
          r->y += height - linetype->height;
        else if (linetype->subscript)
          r->y -= height - linetype->height;

        free(linetype);
        linetype = NULL;
      }

      switch (temp->markup)
      {
        case MARKUP_A :
            if ((link = htmlGetVariable(temp, (uchar *)"NAME")) != NULL)
            {
             /*
              * Add a target link...
              */

              add_link(link, *page, (int)(*y + height));
            }

	default :
	    temp_width = temp->width;
            break;

        case MARKUP_NONE :
            if (temp->data == NULL)
              break;

	    if (((temp->width - right + left) > 0.001 ||
	         (temp->height - top + bottom) > 0.001)  && OverflowErrors)
	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Text on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);

            if (linetype == NULL)
            {
	      linetype  = temp;
	      lineptr   = line;
	      linewidth = 0.0;

	      rgb[0] = temp->red / 255.0f;
	      rgb[1] = temp->green / 255.0f;
	      rgb[2] = temp->blue / 255.0f;
	    }

            strlcpy((char *)lineptr, (char *)temp->data,
	            sizeof(line) - (lineptr - line));

            temp_width = temp->width + char_spacing * strlen((char *)lineptr);

	    if (temp->underline || (temp->link && LinkStyle && PSLevel == 0))
	      new_render(*page, RENDER_BOX, linex, *y - 1, temp_width, 0, rgb);

	    if (temp->strikethrough)
	      new_render(*page, RENDER_BOX, linex, *y + temp->height * 0.25f,
	                 temp_width, 0, rgb);

            linewidth  += temp_width;
            lineptr    += strlen((char *)lineptr);

            if (lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;

	case MARKUP_IMG :
	    if (((temp->width - right + left) > 0.001 ||
	         (temp->height - top + bottom) > 0.001) && OverflowErrors)
	    {
	      DEBUG_printf(("IMAGE: %.3fx%.3f > %.3fx%.3f\n",
	                    temp->width, temp->height,
			    right - left, top - bottom));

	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Image on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);
            }

	    if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	      borderspace = atof((char *)border);
	    else if (temp->link)
	      borderspace = 1;
	    else
	      borderspace = 0;

            borderspace *= PagePrintWidth / _htmlBrowserWidth;

            temp_width += 2 * borderspace;

	    switch (temp->valignment)
	    {
	      case ALIGN_TOP :
		  offset = height - temp->height - 2 * borderspace;
		  break;
	      case ALIGN_MIDDLE :
		  offset = 0.5f * (height - temp->height) - borderspace;
		  break;
	      case ALIGN_BOTTOM :
		  offset = 0.0f;
	    }

            if (borderspace > 0.0f)
	    {
	      // Top
              new_render(*page, RENDER_BOX, linex,
	                 *y + offset + temp->height + borderspace,
			 temp->width + 2 * borderspace, borderspace, rgb);
	      // Left
              new_render(*page, RENDER_BOX, linex, *y + offset,
                	 borderspace, temp->height + 2 * borderspace, rgb);
	      // Right
              new_render(*page, RENDER_BOX, linex + temp->width + borderspace,
	                 *y + offset, borderspace,
			 temp->height + 2 * borderspace, rgb);
	      // Bottom
              new_render(*page, RENDER_BOX, linex, *y + offset,
                	 temp->width + 2 * borderspace, borderspace, rgb);
	    }

	    new_render(*page, RENDER_IMAGE, linex + borderspace,
	               *y + offset + borderspace, temp->width, temp->height,
		       image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));
            whitespace = 0;
	    temp_width = temp->width + 2 * borderspace;
	    break;
      }

      if (temp->link != NULL &&
          (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
      {
       /*
	* Add a page link...
	*/

	if (file_method((char *)link) == NULL)
	{
	  if (file_target((char *)link) != NULL)
	    link = (uchar *)file_target((char *)link) - 1; // Include # sign
	  else
	    link = (uchar *)file_basename((char *)link);
	}

	new_render(*page, RENDER_LINK, linex, *y + offset, temp->width,
	           temp->height, link);
      }

      linex += temp_width;
      prev = temp;
      temp = temp->next;
      if (prev != linetype)
        free(prev);
    }

   /*
    * See if we have a run of characters that hasn't been output...
    */

    if (linetype != NULL)
    {
      r = new_render(*page, RENDER_TEXT, linex - linewidth, *y,
                     linewidth, linetype->height, line);
      r->data.text.typeface = linetype->typeface;
      r->data.text.style    = linetype->style;
      r->data.text.spacing  = char_spacing;
      r->data.text.size     = _htmlSizes[linetype->size];
      memcpy(r->data.text.rgb, rgb, sizeof(rgb));

      if (linetype->superscript)
        r->y += height - linetype->height;
      else if (linetype->subscript)
        r->y -= height - linetype->height;

      free(linetype);
    }

   /*
    * Update the margins after we pass below the images...
    */

    *y -= spacing - height;

    if (*y < image_y)
    {
      image_left   = left;
      image_right  = right;
      format_width = image_right - image_left;
    }
  }

  *x = left;
  if (*y > image_y && image_y > 0.0f)
    *y = image_y;

  DEBUG_printf(("LEAVING parse_paragraph(), x = %.1f, y = %.1f, page = %d\n",
                *x, *y, *page));
}


/*
 * 'parse_pre()' - Parse preformatted text and produce rendering list output.
 */

static void
parse_pre(tree_t *t,		/* I - Tree to parse */
          float  left,		/* I - Left margin */
          float  right,		/* I - Printable width */
          float  bottom,	/* I - Bottom margin */
          float  top,		/* I - Printable top */
          float  *x,		/* IO - X position */
          float  *y,		/* IO - Y position */
          int    *page,		/* IO - Page # */
          int    needspace)	/* I - Need whitespace? */
{
  tree_t	*flat, *start, *next;
  uchar		*link,
		line[10240],
		*lineptr,
		*dataptr;
  int		col;
  float		width,
		height,
		rgb[3];
  render_t	*r;


  REF(right);

  DEBUG_printf(("parse_pre(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  flat = flatten_tree(t->child);

  if (flat == NULL)
    return;

  if (flat->markup == MARKUP_NONE && flat->data != NULL)
  {
    // Skip leading blank line, if present...
    for (dataptr = flat->data; isspace(*dataptr); dataptr ++);

    if (!*dataptr)
    {
      next = flat->next;
      free(flat);
      flat = next;
    }
  }

  while (flat != NULL)
  {
    for (height = 0.0f, start = flat; flat != NULL; flat = flat->next)
    {
      if (flat->height > height)
        height = flat->height;

      if (flat->markup == MARKUP_BR ||
          (flat->markup == MARKUP_NONE && flat->data &&
	   flat->data[strlen((char *)flat->data) - 1] == '\n'))
        break;
    }

    if (flat)
      flat = flat->next;

    if (*y < (height + bottom))
    {
      (*page) ++;
      *y = top;

      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    *x = left;
    *y -= height;

    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / PagePrintLength));

    col = 0;
    while (start != flat)
    {
      rgb[0] = start->red / 255.0f;
      rgb[1] = start->green / 255.0f;
      rgb[2] = start->blue / 255.0f;

      if (start->link &&
	  (link = htmlGetVariable(start->link, (uchar *)"_HD_FULL_HREF")) != NULL)
      {
       /*
	* Add a page link...
	*/

	if (file_method((char *)link) == NULL)
	{
	  if (file_target((char *)link) != NULL)
	    link = (uchar *)file_target((char *)link) - 1; // Include # sign
	  else
	    link = (uchar *)file_basename((char *)link);
	}

	new_render(*page, RENDER_LINK, *x, *y, start->width,
	           start->height, link);

	if (PSLevel == 0 && Links)
	{
          memcpy(rgb, link_color, sizeof(rgb));

	  start->red   = (int)(link_color[0] * 255.0);
	  start->green = (int)(link_color[1] * 255.0);
	  start->blue  = (int)(link_color[2] * 255.0);

          if (LinkStyle)
	    new_render(*page, RENDER_BOX, *x, *y - 1, start->width, 0,
	               link_color);
	}
      }

      switch (start->markup)
      {
	case MARKUP_A :
            if ((link = htmlGetVariable(start, (uchar *)"NAME")) != NULL)
            {
             /*
              * Add a target link...
              */

              add_link(link, *page, (int)(*y + height));
            }
            break;

	case MARKUP_NONE :
            for (lineptr = line, dataptr = start->data;
		 *dataptr != '\0' && lineptr < (line + sizeof(line) - 1);
		 dataptr ++)
              if (*dataptr == '\n')
		break;
              else if (*dataptr == '\t')
              {
        	do
        	{
                  *lineptr++ = ' ';
                  col ++;
        	}
        	while (col & 7);
              }
              else if (*dataptr != '\r')
              {
        	*lineptr++ = *dataptr;
        	col ++;
              }

            *lineptr = '\0';

            width = get_width(line, start->typeface, start->style, start->size);
            r = new_render(*page, RENDER_TEXT, *x, *y, width, 0, line);
            r->data.text.typeface = start->typeface;
            r->data.text.style    = start->style;
            r->data.text.size     = _htmlSizes[start->size];
            memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	    if (start->underline)
	      new_render(*page, RENDER_BOX, *x, *y - 1, start->width, 0, rgb);

	    if (start->strikethrough)
	      new_render(*page, RENDER_BOX, *x, *y + start->height * 0.25f,
	        	 start->width, 0, rgb);

            *x += start->width;
            break;

	case MARKUP_IMG :
	    new_render(*page, RENDER_IMAGE, *x, *y, start->width, start->height,
		       image_find((char *)htmlGetVariable(start, (uchar *)"REALSRC")));

            *x += start->width;
            col ++;
	    break;

	default :
            break;
      }

      next = start->next;
      free(start);
      start = next;

    }

    if ((*x - right) > 0.001 && OverflowErrors)
      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	             "Preformatted text on page %d too long - "
		     "truncation or overlapping may occur!", *page + 1);

    *y -= _htmlSpacings[t->size] - _htmlSizes[t->size];
  }

  *x = left;
}


#ifdef TABLE_DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x) puts(x)
#  define DEBUG
#  undef DEBUG_printf
#  define DEBUG_printf(x) printf x
#endif /* TABLE_DEBUG */

/*
 * 'parse_table()' - Parse a table and produce rendering output.
 */

static void
parse_table(tree_t *t,			// I - Tree to parse
            float  left,		// I - Left margin
            float  right,		// I - Printable width
            float  bottom,		// I - Bottom margin
            float  top,			// I - Printable top
            float  *x,			// IO - X position
            float  *y,			// IO - Y position
            int    *page,		// IO - Page #
            int    needspace)		// I - Need whitespace?
{
  int		col,
		row,
		tcol,
		colspan,
		rowspan,
		num_cols,
		num_rows,
		alloc_rows,
		regular_cols,
		tempspace,
		col_spans[MAX_COLUMNS],
		row_spans[MAX_COLUMNS];
  char		col_fixed[MAX_COLUMNS],
		col_percent[MAX_COLUMNS];
  float		col_lefts[MAX_COLUMNS],
		col_rights[MAX_COLUMNS],
		col_width,
		col_widths[MAX_COLUMNS],
		col_swidths[MAX_COLUMNS],
		col_min,
		col_mins[MAX_COLUMNS],
		col_smins[MAX_COLUMNS],
		col_pref,
		col_prefs[MAX_COLUMNS],
		col_height,
		cellpadding,
		cellspacing,
		border,
		border_left,
		border_size,
		width,
		pref_width,
		span_width,
		regular_width,
		actual_width,
		table_width,
		table_height,
		min_width,
		temp_width,
		table_y,
		row_y, row_starty, temp_y,
		temp_bottom,
		temp_top;
  int		row_page, temp_page, table_page;
  uchar		*var,
		*height_var;		// Row HEIGHT variable
  tree_t	*temprow,
		*tempcol,
		*tempnext,
		***cells,
		*caption;		// Caption for bottom, if any
  int		do_valign;		// True if we should do vertical alignment of cells
  float		row_height,		// Total height of the row
		temp_height;		// Temporary holder
  int		cell_page[MAX_COLUMNS],	// Start page for cell
		cell_endpage[MAX_COLUMNS];
					// End page for cell
  float		cell_y[MAX_COLUMNS],	// Row or each cell
		cell_endy[MAX_COLUMNS],	// Row or each cell
		cell_height[MAX_COLUMNS],
					// Height of each cell in a row
		span_heights[MAX_COLUMNS];
					// Height of spans
  render_t	*cell_bg[MAX_COLUMNS];	// Background rectangles
  render_t	*cell_start[MAX_COLUMNS];
					// Start of the content for a cell in the row
  render_t	*cell_end[MAX_COLUMNS];	// End of the content for a cell in a row
  uchar		*bgcolor;
  float		rgb[3],
		bgrgb[3];
  const char	*htmldoc_debug;		// HTMLDOC_DEBUG env var
  int		table_debug;		// Do table debugging?


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("parse_table(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;   /* Empty table... */

 /*
  * Check debug mode...
  */

  if ((htmldoc_debug = getenv("HTMLDOC_DEBUG")) != NULL &&
      (strstr(htmldoc_debug, "table") || strstr(htmldoc_debug, "all")))
    table_debug = 1;
  else
    table_debug = 0;

 /*
  * Figure out the # of rows, columns, and the desired widths...
  */

  memset(col_spans, 0, sizeof(col_spans));
  memset(col_fixed, 0, sizeof(col_fixed));
  memset(col_percent, 0, sizeof(col_percent));
  memset(col_widths, 0, sizeof(col_widths));
  memset(col_swidths, 0, sizeof(col_swidths));
  memset(col_mins, 0, sizeof(col_mins));
  memset(col_smins, 0, sizeof(col_smins));
  memset(col_prefs, 0, sizeof(col_prefs));

  cells = NULL;

  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_width = atof((char *)var) * (right - left) / 100.0f;
    else
      table_width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    table_width = right - left;

  if ((var = htmlGetVariable(t, (uchar *)"HEIGHT")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_height = atof((char *)var) * (top - bottom) / 100.0f;
    else
      table_height = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    table_height = -1.0f;

  DEBUG_printf(("table_width = %.1f\n", table_width));

  if ((var = htmlGetVariable(t, (uchar *)"CELLPADDING")) != NULL)
    cellpadding = atoi((char *)var);
  else
    cellpadding = 1.0f;

  if ((var = htmlGetVariable(t, (uchar *)"CELLSPACING")) != NULL)
    cellspacing = atoi((char *)var);
  else
    cellspacing = 0.0f;

  if ((var = htmlGetVariable(t, (uchar *)"BORDER")) != NULL)
  {
    if ((border = atof((char *)var)) == 0.0 && var[0] != '0')
      border = 1.0f;

    cellpadding += border;
  }
  else
    border = 0.0f;

  if (table_debug && border == 0.0f)
    border = 0.01f;

  rgb[0] = t->red / 255.0f;
  rgb[1] = t->green / 255.0f;
  rgb[2] = t->blue / 255.0f;

  if ((var = htmlGetVariable(t, (uchar *)"BORDERCOLOR")) != NULL)
    get_color(var, rgb, 0);

  if (border == 0.0f && cellpadding > 0.0f)
  {
   /*
    * Ah, the strange table formatting nightmare that is HTML.
    * Netscape and MSIE assign an invisible border width of 1
    * pixel if no border is specified...
    */

    cellpadding += 1.0f;
  }

  border_size = border - 1.0f;

  cellspacing *= PagePrintWidth / _htmlBrowserWidth;
  cellpadding *= PagePrintWidth / _htmlBrowserWidth;
  border      *= PagePrintWidth / _htmlBrowserWidth;
  border_size *= PagePrintWidth / _htmlBrowserWidth;

  DEBUG_printf(("border = %.1f, cellpadding = %.1f\n", border,
                cellpadding));

  temp_bottom = bottom - cellpadding;
  temp_top    = top + cellpadding;

  memset(row_spans, 0, sizeof(row_spans));
  memset(span_heights, 0, sizeof(span_heights));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0, caption = NULL;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;

    if (temprow->markup == MARKUP_CAPTION)
    {
      if ((var = htmlGetVariable(temprow, (uchar *)"ALIGN")) == NULL ||
          strcasecmp((char *)var, "bottom"))
      {
       /*
        * Show caption at top...
	*/

        parse_paragraph(temprow, left, right, bottom, top, x, y, page, needspace);
        needspace = 1;
      }
      else
      {
       /*
        * Flag caption for bottom of table...
	*/

        caption = temprow;
      }
    }
    else if (temprow->markup == MARKUP_TR ||
             ((temprow->markup == MARKUP_TBODY || temprow->markup == MARKUP_THEAD ||
               temprow->markup == MARKUP_TFOOT) && temprow->child != NULL))
    {
      // Descend into table body as needed...
      if (temprow->markup == MARKUP_TBODY || temprow->markup == MARKUP_THEAD ||
          temprow->markup == MARKUP_TFOOT)
        temprow = temprow->child;

      // Figure out the next row...
      if ((tempnext = temprow->next) == NULL)
        if (temprow->parent->markup == MARKUP_TBODY ||
            temprow->parent->markup == MARKUP_THEAD ||
            temprow->parent->markup == MARKUP_TFOOT)
          tempnext = temprow->parent->next;

      // Allocate memory for the table as needed...
      if (num_rows >= alloc_rows)
      {
        alloc_rows += ALLOC_ROWS;

        if (alloc_rows == ALLOC_ROWS)
	  cells = (tree_t ***)malloc(sizeof(tree_t **) * alloc_rows);
	else
	  cells = (tree_t ***)realloc(cells, sizeof(tree_t **) * alloc_rows);

        if (cells == (tree_t ***)0)
	{
	  progress_error(HD_ERROR_OUT_OF_MEMORY,
                         "Unable to allocate memory for table!");
	  return;
	}
      }

      if ((cells[num_rows] = (tree_t **)calloc(sizeof(tree_t *), MAX_COLUMNS)) == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for table!");
	return;
      }

#ifdef DEBUG
      printf("BEFORE row %d: num_cols = %d\n", num_rows, num_cols);

      if (num_rows)
        for (col = 0; col < num_cols; col ++)
	  printf("    col %d: row_spans[] = %d\n", col, row_spans[col]);
#endif // DEBUG

      // Figure out the starting column...
      if (num_rows)
      {
	for (col = 0, rowspan = 9999; col < num_cols; col ++)
	  if (row_spans[col] < rowspan)
	    rowspan = row_spans[col];

	for (col = 0; col < num_cols; col ++)
	  row_spans[col] -= rowspan;

	for (col = 0; row_spans[col] && col < num_cols; col ++)
          cells[num_rows][col] = cells[num_rows - 1][col];
      }
      else
        col = 0;

      for (tempcol = temprow->child;
           tempcol != NULL && col < MAX_COLUMNS;
           tempcol = tempcol->next)
        if (tempcol->markup == MARKUP_TD || tempcol->markup == MARKUP_TH)
        {
	  // Handle colspan and rowspan stuff...
          if ((var = htmlGetVariable(tempcol, (uchar *)"COLSPAN")) != NULL)
            colspan = atoi((char *)var);
          else
            colspan = 1;

          if ((var = htmlGetVariable(tempcol, (uchar *)"ROWSPAN")) != NULL)
	  {
            row_spans[col] = atoi((char *)var);

	    if (row_spans[col] == 1)
	      row_spans[col] = 0;

	    for (tcol = 1; tcol < colspan; tcol ++)
              row_spans[col + tcol] = row_spans[col];
          }

          // Compute the cell size...
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min,
	                            &col_pref, &col_height);
          if ((var = htmlGetVariable(tempcol, (uchar *)"WIDTH")) != NULL)
	  {
	    if (var[strlen((char *)var) - 1] == '%')
	    {
              col_width -= 2.0 * cellpadding - cellspacing;

	      if (colspan <= 1)
	        col_percent[col] = 1;
	    }
	    else
	    {
              col_width -= 2.0 * cellpadding;
	    }
	  }
	  else
	    col_width = 0.0f;

          tempcol->height = col_height;

	  DEBUG_printf(("%d,%d: colsp=%d, rowsp=%d, width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
	                col, num_rows, colspan, row_spans[col], col_width,
			col_min, col_pref, col_height));

          // Add widths to columns...
          if (colspan > 1)
          {
	    if (colspan > col_spans[col])
	      col_spans[col] = colspan;

	    if (col_width > col_swidths[col])
	      col_swidths[col] = col_width;

	    if (col_min > col_smins[col])
	      col_smins[col] = col_min;

	    temp_width = col_width / colspan;
	    for (int i = 0; i < colspan; i ++)
	    {
	      if (temp_width > col_widths[col + i])
	        col_widths[col + i] = temp_width;
	    }

#if 0
	    temp_width = col_pref / colspan;
	    for (int i = 0; i < colspan; i ++)
	    {
	      if (temp_width > col_prefs[col + i])
	        col_prefs[col + i] = temp_width;
	    }

	    temp_width = col_width / colspan;
	    for (int i = 0; i < colspan; i ++)
	    {
	      if (temp_width > col_widths[col + i])
	        col_widths[col + i] = temp_width;
	    }
#endif // 0
          }
	  else
	  {
	    if (col_width > 0.0f)
	      col_fixed[col] = 1;

	    if (col_width > col_widths[col])
	      col_widths[col] = col_width;

	    if (col_pref > col_prefs[col])
	      col_prefs[col] = col_pref;

	    if (col_min > col_mins[col])
	      col_mins[col] = col_min;
          }

	  while (colspan > 0 && col < MAX_COLUMNS)
	  {
            cells[num_rows][col] = tempcol;
            col ++;
            colspan --;
          }

          while (row_spans[col] && col < num_cols)
	  {
            cells[num_rows][col] = cells[num_rows - 1][col];
	    col ++;
	  }
        }

      if (col > num_cols)
        num_cols = col;

#ifdef DEBUG
      printf("AFTER row %d: num_cols = %d\n", num_rows, num_cols);

      for (col = 0; col < num_cols; col ++)
        printf("    col %d: row_spans[] = %d\n", col, row_spans[col]);
#endif // DEBUG

      num_rows ++;

      for (col = 0; col < num_cols; col ++)
        if (row_spans[col])
	  row_spans[col] --;
    }
  }

 /*
  * OK, some people apparently create HTML tables with no columns or
  * rows...  If this happened, return immediately...
  */

  if (num_cols == 0)
    return;

 /*
  * Now figure out the width of the table...
  */

  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      width = atof((char *)var) * (right - left) / 100.0f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
  {
    for (col = 0, width = 0.0; col < num_cols; col ++)
      width += col_prefs[col];

    width += (2 * cellpadding + cellspacing) * num_cols - cellspacing;

    if (width > (right - left))
      width = right - left;
  }

 /*
  * Compute the width of each column based on the printable width.
  */

  DEBUG_printf(("\nTABLE: %dx%d\n\n", num_cols, num_rows));

  actual_width  = (2 * cellpadding + cellspacing) * num_cols -
                  cellspacing;
  regular_width = (width - actual_width) / num_cols;

  DEBUG_printf(("    width = %.1f, actual_width = %.1f, regular_width = %.1f\n\n",
                width, actual_width, regular_width));
  DEBUG_puts("    Col  Width   Min     Pref    Fixed?  Percent?");
  DEBUG_puts("    ---  ------  ------  ------  ------  --------");

#ifdef DEBUG
  for (col = 0; col < num_cols; col ++)
    printf("    %-3d  %-6.1f  %-6.1f  %-6.1f  %-6s  %s\n", col, col_widths[col],
           col_mins[col], col_prefs[col], col_fixed[col] ? "YES" : "NO",
	   col_percent[col] ? "YES" : "NO");

  puts("");
#endif /* DEBUG */

 /*
  * The first pass just handles columns with a specified width...
  */

  DEBUG_puts("PASS 1: fixed width handling\n");

  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (col_widths[col] > 0.0f)
    {
      if (col_mins[col] > col_widths[col])
      {
        DEBUG_printf(("    updating column %d to width=%.1f\n", col,
	              col_mins[col]));

        col_widths[col] = col_mins[col];
      }

      actual_width += col_widths[col];
    }
    else
    {
      regular_cols ++;

      actual_width += col_mins[col];
    }

  DEBUG_printf(("    actual_width = %.1f, regular_cols = %d\n\n", actual_width,
                regular_cols));

 /*
  * Pass two uses the "preferred" width whenever possible, and the
  * minimum otherwise...
  */

  DEBUG_puts("PASS 2: preferred width handling\n");

  for (col = 0, pref_width = 0.0f; col < num_cols; col ++)
    if (col_widths[col] == 0.0f)
      pref_width += col_prefs[col] - col_mins[col];

  DEBUG_printf(("    pref_width = %.1f\n", pref_width));

  if (pref_width > 0.0f)
  {
    if ((regular_width = (width - actual_width) / pref_width) < 0.0f)
      regular_width = 0.0f;
    else if (regular_width > 1.0f)
      regular_width = 1.0f;

    DEBUG_printf(("    regular_width = %.1f\n", regular_width));

    for (col = 0; col < num_cols; col ++)
      if (col_widths[col] == 0.0f)
      {
	pref_width = (col_prefs[col] - col_mins[col]) * regular_width;

	if ((actual_width + pref_width) > width)
	{
          if (col == (num_cols - 1) && (width - actual_width) >= col_mins[col])
	    col_widths[col] = width - actual_width;
	  else
	    col_widths[col] = col_mins[col];
	}
	else
          col_widths[col] = pref_width + col_mins[col];

        DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));

	actual_width += col_widths[col] - col_mins[col];
      }
  }
  else
  {
   /*
    * Assign min widths for all cells...
    */

    for (col = 0; col < num_cols; col ++)
      if (col_widths[col] == 0.0f)
        col_widths[col] = col_mins[col];
  }

  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));

 /*
  * Pass three enforces any hard or minimum widths for COLSPAN'd
  * columns...
  */

  DEBUG_puts("PASS 3: colspan handling\n\n");

  for (col = 0; col < num_cols; col ++)
  {
    DEBUG_printf(("    col %d, colspan %d\n", col, col_spans[col]));

    if (col_spans[col] > 1)
    {
      for (colspan = 0, span_width = 0.0f; colspan < col_spans[col]; colspan ++)
        span_width += col_widths[col + colspan];

      pref_width = 0.0f;

      if (span_width < col_swidths[col])
        pref_width = col_swidths[col];
      if (span_width < col_smins[col] && pref_width < col_smins[col])
        pref_width = col_smins[col];

      for (colspan = 0; colspan < col_spans[col]; colspan ++)
        if (col_fixed[col + colspan])
	{
          span_width -= col_widths[col + colspan];
	  pref_width -= col_widths[col + colspan];
	}

      DEBUG_printf(("    col_swidths=%.1f, col_smins=%.1f, span_width=%.1f, pref_width=%.1f\n",
                    col_swidths[col], col_smins[col], span_width, pref_width));

      if (pref_width > 0.0f && pref_width > span_width)
      {
        if (span_width >= 1.0f)
	{
          // Expand cells proportionately...
	  regular_width = pref_width / span_width;

	  for (colspan = 0; colspan < col_spans[col]; colspan ++)
	    if (!col_fixed[col + colspan])
	    {
	      actual_width -= col_widths[col + colspan];
	      col_widths[col + colspan] *= regular_width;
	      actual_width += col_widths[col + colspan];

              DEBUG_printf(("    col_widths[%d] = %.1f\n", col + colspan,
	                    col_widths[col + colspan]));
	    }
        }
	else
	{
	  // Divide the space up equally between columns, since the
	  // colspan area is always by itself... (this hack brought
	  // to you by Yahoo! and their single cell tables with
	  // colspan=2 :)

	  regular_width = pref_width / col_spans[col];

	  for (colspan = 0; colspan < col_spans[col]; colspan ++)
	  {
	    actual_width += regular_width;
	    col_widths[col + colspan] += regular_width;

            DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
	  }
	}
      }
    }
  }

  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));

 /*
  * Pass four divides up the remaining space amongst the columns...
  */

  DEBUG_puts("PASS 4: divide remaining space, if any...\n");

  if (width > actual_width)
  {
    for (col = 0, colspan = 0; col < num_cols; col ++)
      if (!col_fixed[col] || col_percent[col])
        colspan ++;

    if (colspan > 0)
    {
      regular_width = (width - actual_width) / num_cols;

      for (col = 0; col < num_cols; col ++)
        if (!col_fixed[col])
	{
	  col_widths[col] += regular_width;
	  DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
	}
    }
  }
  else
    width = actual_width;

  DEBUG_puts("");

 /*
  * The final pass is only run if the width > table_width...
  */

  DEBUG_puts("PASS 5: Squeeze table as needed...");

  if (width > table_width)
  {
   /*
    * Squeeze the table to fit the requested width or the printable width
    * as determined at the beginning...
    */

    for (col = 0, min_width = -cellspacing; col < num_cols; col ++)
      min_width += col_mins[col] + 2 * cellpadding + cellspacing;

    DEBUG_printf(("    table_width = %.1f, width = %.1f, min_width = %.1f\n",
                  table_width, width, min_width));

    temp_width = table_width - min_width;
    if (temp_width < 0.0f)
      temp_width = 0.0f;

    width -= min_width;
    if (width < 1.0f)
      width = 1.0f;

    for (col = 0; col < num_cols; col ++)
    {
      col_widths[col] = col_mins[col] +
                        temp_width * (col_widths[col] - col_mins[col]) / width;

      DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
    }

    for (col = 0, width = -cellspacing; col < num_cols; col ++)
      width += col_widths[col] + 2 * cellpadding + cellspacing;

    DEBUG_printf(("    new width = %.1f, max width = %.1f\n", width,
                  right - left));
  }

  if ((width - right + left) > 0.001f && OverflowErrors)
    progress_error(HD_ERROR_CONTENT_TOO_LARGE,
                   "Table on page %d too wide - "
		   "truncation or overlapping may occur!", *page + 1);

  DEBUG_puts("");

  DEBUG_printf(("Final table width = %.1f, alignment = %d\n",
                width, t->halignment));

  switch (t->halignment)
  {
    case ALIGN_LEFT :
        *x = left + cellpadding;
        break;
    case ALIGN_CENTER :
        *x = left + 0.5f * (right - left - width) + cellpadding;
        break;
    case ALIGN_RIGHT :
        *x = right - width + cellpadding;
        break;
  }

  for (col = 0; col < num_cols; col ++)
  {
    col_lefts[col]  = *x;
    col_rights[col] = *x + col_widths[col];
    *x = col_rights[col] + 2 * cellpadding + cellspacing;

    DEBUG_printf(("left[%d] = %.1f, right[%d] = %.1f\n", col, col_lefts[col], col,
                  col_rights[col]));
  }

 /*
  * Now render the whole table...
  */

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  if (table_debug)
  {
    check_pages(*page);

    render_t *r;
    char table_text[255];

    snprintf(table_text, sizeof(table_text), "t=%p", t);
    r = new_render(*page, RENDER_TEXT, left, *y,
                   get_width((uchar *)table_text, TYPE_COURIER, STYLE_NORMAL, 3),
		   _htmlSizes[3], table_text);

    r->data.text.typeface = TYPE_COURIER;
    r->data.text.style    = STYLE_NORMAL;
    r->data.text.size     = _htmlSizes[3];
  }

  memset(row_spans, 0, sizeof(row_spans));
  memset(cell_start, 0, sizeof(cell_start));
  memset(cell_end, 0, sizeof(cell_end));
  memset(cell_height, 0, sizeof(cell_height));
  memset(cell_bg, 0, sizeof(cell_bg));

  table_page = *page;
  table_y    = *y;

  for (row = 0; row < num_rows; row ++)
  {
    height_var = NULL;

    if (cells[row][0] != NULL)
    {
     /*
      * Do page comments...
      */

      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->markup == MARKUP_COMMENT)
        parse_comment(cells[row][0]->parent->prev,
                      &left, &right, &temp_bottom, &temp_top, x, y,
		      page, NULL, 0);

     /*
      * Get height...
      */

      if ((height_var = htmlGetVariable(cells[row][0]->parent,
                           	        (uchar *)"HEIGHT")) == NULL)
	for (col = 0; col < num_cols; col ++)
	  if (htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN") == NULL)
	    if ((height_var = htmlGetVariable(cells[row][col],
                                              (uchar *)"HEIGHT")) != NULL)
	      break;
    }

    if (cells[row][0] != NULL && height_var != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (height_var[strlen((char *)height_var) - 1] == '%')
	temp_height = atof((char *)height_var) * 0.01f *
	              (PagePrintLength - 2 * cellpadding);
      else
        temp_height = atof((char *)height_var) * PagePrintWidth / _htmlBrowserWidth;

      if (table_height > 0.0f && temp_height > table_height)
        temp_height = table_height;

      temp_height -= 2 * cellpadding;
    }
    else
    {
      // Use min height computed from get_cell_size()...
      for (col = 0, temp_height = _htmlSpacings[SIZE_P];
           col < num_cols;
	   col ++)
        if (cells[row][col] != NULL &&
	    cells[row][col]->height > temp_height &&
	    !htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN"))
	  temp_height = cells[row][col]->height;

      if (table_height > 0.0)
      {
	// Table height specified; make sure it'll fit...
	if (temp_height > table_height)
          temp_height = table_height;
	temp_height -= 2 * cellpadding;
      }
      else if (temp_height > (PageLength / 8) && height_var == NULL)
	temp_height = PageLength / 8;
    }

    DEBUG_printf(("BEFORE row = %d, temp_height = %.1f, *y = %.1f, *page = %d\n",
                  row, temp_height, *y, *page));

    if (*y < (bottom + 2 * cellpadding + temp_height) &&
        temp_height <= (top - bottom - 2 * cellpadding))
    {
      DEBUG_puts("NEW PAGE");

      *y = top;
      (*page) ++;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    do_valign  = 1;
    row_y      = *y - cellpadding;
    row_starty = row_y;
    row_page   = *page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row_y = %.1f, *y = %.1f, row_page = %d\n",
                  row_y, *y, row_page));

    for (col = 0, rowspan = 9999; col < num_cols; col += colspan)
    {
      if (row_spans[col] == 0)
      {
        if ((var = htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN")) != NULL)
          row_spans[col] = atoi((char *)var);

        if (row_spans[col] == 1)
	  row_spans[col] = 0;

        if (row_spans[col] > (num_rows - row))
	  row_spans[col] = num_rows - row;

	span_heights[col] = 0.0f;
      }

      if (row_spans[col] < rowspan)
	rowspan = row_spans[col];

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
    }

    if (!rowspan)
      rowspan = 1;

    for (col = 0; col < num_cols;)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      DEBUG_printf(("    col = %d, colspan = %d, left = %.1f, right = %.1f, cell = %p\n",
                    col, colspan, col_lefts[col], col_rights[col + colspan], cells[row][col]));

      *x        = col_lefts[col];
      temp_y    = *y - cellpadding;
      temp_page = *page;
      tempspace = 0;

      if (row == 0 || cells[row][col] != cells[row - 1][col])
      {
        check_pages(*page);

        if (cells[row][col] == NULL)
	  bgcolor = NULL;
	else if ((bgcolor = htmlGetVariable(cells[row][col],
                                            (uchar *)"BGCOLOR")) != NULL)
	{
	  memcpy(bgrgb, background_color, sizeof(bgrgb));

          get_color(bgcolor, bgrgb, 0);

	  width       = col_rights[col + colspan] - col_lefts[col] +
        	        2 * cellpadding;
	  border_left = col_lefts[col] - cellpadding;

          cell_bg[col] = new_render(*page, RENDER_BOX, border_left, row_y,
                                    width + border, 0.0, bgrgb);
	}
	else
	{
	  cell_bg[col] = NULL;

          new_render(*page, RENDER_TEXT, -1.0f, -1.0f, 0.0, 0.0, (void *)"");
	}

        DEBUG_printf(("cell_bg[%d] = %p, pages[%d].end = %p\n",
	              col, cell_bg[col], *page, pages[*page].end));

	cell_start[col] = pages[*page].end;
	cell_page[col]  = temp_page;
	cell_y[col]     = temp_y;

        if (table_debug)
	{
	  check_pages(*page);

	  render_t *r;
	  char table_text[255];

	  snprintf(table_text, sizeof(table_text), "cell=%p [%d,%d]",
	           cells[row][col], row, col);
	  r = new_render(temp_page, RENDER_TEXT, *x, temp_y,
                	 get_width((uchar *)table_text, TYPE_COURIER, STYLE_NORMAL, 1),
			 _htmlSizes[1], table_text);

	  r->data.text.typeface = TYPE_COURIER;
	  r->data.text.style    = STYLE_NORMAL;
	  r->data.text.size     = _htmlSizes[1];
	}

        if (cells[row][col] != NULL && cells[row][col]->child != NULL)
	{
	  DEBUG_printf(("    parsing cell %d,%d; width = %.1f\n", row, col,
	                col_rights[col + colspan] - col_lefts[col]));

          bottom += cellpadding;
	  top    -= cellpadding;

          parse_doc(cells[row][col]->child,
                    col_lefts + col, col_rights + col + colspan,
                    &bottom, &top,
                    x, &temp_y, &temp_page, NULL, &tempspace);

          bottom -= cellpadding;
	  top    += cellpadding;
        }

        cell_endpage[col] = temp_page;
        cell_endy[col]    = temp_y;
        cell_height[col]  = *y - cellpadding - temp_y;
        cell_end[col]     = pages[*page].end;

        if (cell_start[col] == NULL)
	  cell_start[col] = pages[*page].start;

        DEBUG_printf(("row = %d, col = %d, y = %.1f, cell_y = %.1f, cell_height = %.1f\n",
	              row, col, *y - cellpadding, temp_y, cell_height[col]));
        DEBUG_printf(("cell_start[%d] = %p, cell_end[%d] = %p\n",
	              col, cell_start[col], col, cell_end[col]));
      }

      if (row_spans[col] == 0 &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > row_height)
        row_height = cell_height[col];

      if (row_spans[col] <= rowspan)
      {
	if (cell_page[col] != cell_endpage[col])
	  do_valign = 0;

        if (cell_endpage[col] > row_page)
	{
	  row_page = cell_endpage[col];
	  row_y    = cell_endy[col];
	}
	else if (cell_endy[col] < row_y && cell_endpage[col] == row_page)
	  row_y = cell_endy[col];
      }

      DEBUG_printf(("**** col = %d, row = %d, row_y = %.1f, row_page = %d\n",
                    col, row, row_y, row_page));

      for (col ++; colspan > 0; colspan --, col ++)
      {
        cell_start[col]   = NULL;
        cell_page[col]    = cell_page[col - 1];
        cell_y[col]       = cell_y[col - 1];
	cell_end[col]     = NULL;
        cell_endpage[col] = cell_endpage[col - 1];
        cell_endy[col]    = cell_endy[col - 1];
	cell_height[col]  = cell_height[col - 1];
      }
    }

    DEBUG_printf(("row = %d, row_y = %.1f, row_height = %.1f\n", row, row_y, row_height));

    for (col = 0; col < num_cols; col += colspan)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;

      if (row_spans[col])
        span_heights[col] += row_height;

      DEBUG_printf(("col = %d, cell_y = %.1f, cell_page = %d, cell_endpage = %d, row_spans = %d, span_heights = %.1f, cell_height = %.1f\n",
                    col, cell_y[col], cell_page[col], cell_endpage[col],
		    row_spans[col], span_heights[col], cell_height[col]));

      if (row_spans[col] == rowspan &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > span_heights[col])
      {
        temp_height = cell_height[col] - span_heights[col];
	row_height  += temp_height;
	DEBUG_printf(("Adjusting row-span height by %.1f, new row_height = %.1f\n",
	              temp_height, row_height));

	for (tcol = 0; tcol < num_cols; tcol ++)
	  if (row_spans[tcol])
	  {
	    span_heights[tcol] += temp_height;
	    DEBUG_printf(("col = %d, span_heights = %.1f\n", tcol, span_heights[tcol]));
	  }
      }
    }

    DEBUG_printf(("AFTER row = %d, row_page = %d, row_y = %.1f, row_height = %.1f, *y = %.1f, do_valign = %d\n",
                  row, row_page, row_y, row_height, *y, do_valign));

   /*
    * Do the vertical alignment
    */

    if (do_valign)
    {
      if (height_var != NULL)
      {
        // Hardcode the row height...
        if (height_var[strlen((char *)height_var) - 1] == '%')
	  temp_height = atof((char *)height_var) * 0.01f * PagePrintLength;
	else
          temp_height = atof((char *)height_var) * PagePrintWidth / _htmlBrowserWidth;

        if (table_height > 0 && temp_height > table_height)
          temp_height = table_height;

        temp_height -= 2 * cellpadding;

        if (temp_height > row_height)
	{
	  // Only enforce the height if it is > the actual row height.
	  row_height = temp_height;
          row_y      = *y - temp_height;
	}
      }

      for (col = 0; col < num_cols; col += colspan + 1)
      {
        render_t	*p;
        float		delta_y;


        for (colspan = 1; (col + colspan) < num_cols; colspan ++)
          if (cells[row][col] != cells[row][col + colspan])
            break;

        colspan --;

        if (cell_start[col] == NULL || row_spans[col] > rowspan ||
	    cells[row][col] == NULL || cells[row][col]->child == NULL)
	  continue;

        if (row_spans[col])
          switch (cells[row][col]->valignment)
	  {
            case ALIGN_MIDDLE :
        	delta_y = (span_heights[col] - cell_height[col]) * 0.5f;
        	break;

            case ALIGN_BOTTOM :
        	delta_y = span_heights[col] - cell_height[col];
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }
	else
          switch (cells[row][col]->valignment)
	  {
            case ALIGN_MIDDLE :
        	delta_y = (row_height - cell_height[col]) * 0.5f;
        	break;

            case ALIGN_BOTTOM :
        	delta_y = row_height - cell_height[col];
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }

	DEBUG_printf(("row = %d, col = %d, valign = %d, cell_height = %.1f, span_heights = %.1f, delta_y = %.1f\n",
	              row, col, cells[row][col]->valignment,
		      cell_height[col], span_heights[col], delta_y));

        if (delta_y > 0.0f)
	{
	  if (cell_start[col] == cell_end[col])
	    p = cell_start[col];
	  else
	    p = cell_start[col]->next;

          for (; p != NULL; p = p->next)
	  {
	    DEBUG_printf(("aligning %p (%s), y was %.1f, now %.1f\n",
	                  p, p->data.text.buffer, p->y, p->y - delta_y));

            p->y -= delta_y;
            if (p == cell_end[col])
	      break;
          }
        }
#ifdef DEBUG
        else
	{
	  if (cell_start[col] == cell_end[col])
	    p = cell_start[col];
	  else
	    p = cell_start[col]->next;

          for (; p != NULL; p = p->next)
	  {
	    printf("NOT aligning %p\n", p);

            if (p == cell_end[col])
	      break;
          }
	}
#endif /* DEBUG */
      }
    }

    // Update all current columns with ROWSPAN <= rowspan to use the same
    // end page and row...
    for (col = 0, temp_page = -1, temp_y = 99999999; col < num_cols; col ++)
      if (row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
      {
        if (cell_endpage[col] > temp_page)
	{
          temp_page = cell_endpage[col];
	  temp_y    = cell_endy[col];
	}
        else if (cell_endpage[col] == temp_page && cell_endy[col] < temp_y)
	  temp_y = cell_endy[col];
      }

    for (col = 0; col < num_cols; col ++)
      if (row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
      {
        cell_endpage[col] = temp_page;
	cell_endy[col]    = temp_y;
      }

    row_y -= cellpadding;

    border_left = col_lefts[0] - cellpadding;
    width       = col_rights[num_cols - 1] - col_lefts[0] + 2 * cellpadding;

    for (bgcolor = NULL, col = 0; col < num_cols; col ++)
      if (row_spans[col] <= rowspan &&
          cells[row][col] &&
	  !htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN") &&
          (bgcolor = htmlGetVariable(cells[row][col]->parent,
                                     (uchar *)"BGCOLOR")) != NULL)
        break;

    if (bgcolor)
    {
      memcpy(bgrgb, background_color, sizeof(bgrgb));

      get_color(bgcolor, bgrgb, 0);

      if (row_page > *page)
      {
        // Draw background on multiple pages...

	// Bottom of first page...
        new_render(*page, RENDER_BOX, border_left, bottom,
	           width, row_starty - bottom + cellpadding, bgrgb,
		   pages[*page].start);

        // Intervening pages...
        for (temp_page = *page + 1; temp_page < row_page; temp_page ++)
	{
          new_render(temp_page, RENDER_BOX, border_left, bottom,
                     width, top - bottom, bgrgb, pages[temp_page].start);
        }

        // Top of last page...
	check_pages(*page);

        new_render(row_page, RENDER_BOX, border_left, row_y,
	           width, top - row_y, bgrgb,
		   pages[row_page].start);
      }
      else
      {
        // Draw background in row...
        new_render(row_page, RENDER_BOX, border_left, row_y,
	           width, row_height + 2 * cellpadding, bgrgb,
		   pages[row_page].start);
      }
    }

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      for (colspan = 0; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
	else if (row_spans[col + colspan] > 0)
	{
          DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
	        	col, row_spans[col + colspan],
			row_spans[col + colspan] - rowspan));
          row_spans[col + colspan] -= rowspan;
	}

      colspan --;

      width = col_rights[col + colspan] - col_lefts[col] +
              2 * cellpadding;

      if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
          row_spans[col] > 0)
        continue;

      DEBUG_printf(("DRAWING BORDER+BACKGROUND: col=%d, row=%d, cell_page=%d, cell_y=%.1f\n"
                    "                           cell_endpage=%d, cell_endy=%.1f\n",
		    col, row, cell_page[col], cell_y[col],
		    cell_endpage[col], cell_endy[col]));

      if ((bgcolor = htmlGetVariable(cells[row][col],
                                     (uchar *)"BGCOLOR")) != NULL)
      {
        memcpy(bgrgb, background_color, sizeof(bgrgb));

        get_color(bgcolor, bgrgb, 0);
      }

      border_left = col_lefts[col] - cellpadding;

      if (cell_page[col] != cell_endpage[col])
      {
       /*
        * Crossing a page boundary...
        */

        if (border > 0)
	{
	 /*
	  * +---+---+---+
	  * |   |   |   |
	  */

	  // Top
          new_render(cell_page[col], RENDER_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          new_render(cell_page[col], RENDER_BOX, border_left, bottom,
                     border, cell_y[col] - bottom + cellpadding + border, rgb);
	  // Right
          new_render(cell_page[col], RENDER_BOX,
	             border_left + width, bottom,
		     border, cell_y[col] - bottom + cellpadding + border, rgb);
        }

        if (bgcolor != NULL)
        {
	  cell_bg[col]->y      = bottom;
	  cell_bg[col]->height = cell_y[col] - bottom + cellpadding + border;
	}

        for (temp_page = cell_page[col] + 1; temp_page < cell_endpage[col]; temp_page ++)
	{
	 /*
	  * |   |   |   |
	  * |   |   |   |
	  */

	  if (border > 0.0f)
	  {
	    // Left
            new_render(temp_page, RENDER_BOX, border_left, bottom,
                       border, top - bottom, rgb);
	    // Right
            new_render(temp_page, RENDER_BOX,
	               border_left + width, bottom,
		       border, top - bottom, rgb);
          }

	  if (bgcolor != NULL)
            new_render(temp_page, RENDER_BOX, border_left, bottom,
                       width + border, top - bottom, bgrgb,
		       pages[temp_page].start);
        }

        if (border > 0.0f)
	{
	 /*
	  * |   |   |   |
	  * +---+---+---+
	  */

	  // Left
          new_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
                     border, top - row_y, rgb);
	  // Right
          new_render(cell_endpage[col], RENDER_BOX,
	             border_left + width, row_y,
                     border, top - row_y, rgb);
	  // Bottom
          new_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
        }

        if (bgcolor != NULL)
	{
	  check_pages(cell_endpage[col]);

          new_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
	             width + border, top - row_y, bgrgb,
		     pages[cell_endpage[col]].start);
	}
      }
      else
      {
       /*
	* +---+---+---+
	* |   |   |   |
	* +---+---+---+
	*/

        if (border > 0.0f)
	{
	  // Top
          new_render(cell_page[col], RENDER_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          new_render(cell_page[col], RENDER_BOX, border_left, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Right
          new_render(cell_page[col], RENDER_BOX,
	             border_left + width, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Bottom
          new_render(cell_page[col], RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
	}

        if (bgcolor != NULL)
	{
	  cell_bg[col]->y      = row_y;
	  cell_bg[col]->height = cell_y[col] - row_y + cellpadding + border;
	}
      }
    }

    *page = row_page;
    *y    = row_y;

    if (row < (num_rows - 1))
      (*y) -= cellspacing;

    DEBUG_printf(("END row = %d, *y = %.1f, *page = %d\n", row, *y, *page));
  }

 /*
  * Handle table background color...
  */

  if ((bgcolor = htmlGetVariable(t, (uchar *)"BGCOLOR")) != NULL)
  {
    memcpy(bgrgb, background_color, sizeof(bgrgb));

    get_color(bgcolor, bgrgb, 0);

    border_left = col_lefts[0] - cellpadding;
    width       = col_rights[num_cols - 1] - col_lefts[0] + 2 * cellpadding;

    if (table_page != *page)
    {
      // Draw background on multiple pages...

      // Bottom of first page...
      new_render(table_page, RENDER_BOX, border_left, bottom,
	         width, table_y - bottom, bgrgb,
		 pages[table_page].start);

      // Intervening pages...
      for (temp_page = table_page + 1; temp_page < *page; temp_page ++)
      {
        new_render(temp_page, RENDER_BOX, border_left, bottom,
                   width, top - bottom, bgrgb, pages[temp_page].start);
      }

      // Top of last page...
      check_pages(*page);

      new_render(*page, RENDER_BOX, border_left, *y,
	         width, top - *y, bgrgb, pages[*page].start);
    }
    else
    {
      // Draw background in row...
      new_render(table_page, RENDER_BOX, border_left, *y,
	         width, table_y - *y, bgrgb, pages[table_page].start);
    }
  }

  *x = left;

  if (caption)
  {
   /*
    * Show caption at bottom...
    */

    parse_paragraph(caption, left, right, bottom, top, x, y, page, needspace);
    needspace = 1;
  }

 /*
  * Free memory for the table...
  */

  if (num_rows > 0)
  {
    for (row = 0; row < num_rows; row ++)
      free(cells[row]);

    free(cells);
  }
}
#ifdef TABLE_DEBUG
#  undef DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x)
#  undef DEBUG_printf
#  define DEBUG_printf(x)
#endif /* TABLE_DEBUG */


/*
 * 'parse_list()' - Parse a list entry and produce rendering output.
 */

static void
parse_list(tree_t *t,		/* I - Tree to parse */
           float  *left,	/* I - Left margin */
           float  *right,	/* I - Printable width */
           float  *bottom,	/* I - Bottom margin */
           float  *top,		/* I - Printable top */
           float  *x,		/* IO - X position */
           float  *y,		/* IO - Y position */
           int    *page,	/* IO - Page # */
           int    needspace)	/* I - Need whitespace? */
{
  uchar		number[255];	/* List number (for numbered types) */
  uchar		*value;		/* VALUE= variable */
  int		typeface;	/* Typeface of list number */
  float		width;		/* Width of list number */
  render_t	*r;		/* Render primitive */
  int		oldpage;	/* Old page value */
  float		oldy;		/* Old Y value */
  float		tempx;		/* Temporary X value */


  DEBUG_printf(("parse_list(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, *left, *right, *x, *y, *page));

  if (needspace && *y < *top)
  {
    *y        -= _htmlSpacings[t->size];
    needspace = 0;
  }

  check_pages(*page);

  oldy    = *y;
  oldpage = *page;
  r       = pages[*page].end;
  tempx   = *x;

  if (t->indent == 0)
  {
    // Adjust left margin when no UL/OL/DL is being used...
    *left += _htmlSizes[t->size];
    tempx += _htmlSizes[t->size];
  }

  parse_doc(t->child, left, right, bottom, top, &tempx, y, page, NULL,
            &needspace);

  // Handle when paragraph wrapped to new page...
  if (*page != oldpage)
  {
    // First see if anything was added to the old page...
    if ((r != NULL && r->next == NULL) || pages[oldpage].end == NULL)
    {
      // No, put the symbol on the next page...
      oldpage = *page;
      oldy    = *top;
    }
  }

  if ((value = htmlGetVariable(t, (uchar *)"VALUE")) != NULL)
  {
    if (isdigit(value[0]))
      list_values[t->indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values[t->indent] = value[0] - 'A' + 1;
    else
      list_values[t->indent] = value[0] - 'a' + 1;
  }

  switch (list_types[t->indent])
  {
    case 'a' :
    case 'A' :
    case '1' :
    case 'i' :
    case 'I' :
        strlcpy((char *)number, format_number(list_values[t->indent],
	                                      list_types[t->indent]),
		sizeof(number));
        strlcat((char *)number, ". ", sizeof(number));
        typeface = t->typeface;
        break;

    default :
        sprintf((char *)number, "%c ", list_types[t->indent]);
        typeface = TYPE_SYMBOL;
        break;
  }

  width = get_width(number, typeface, t->style, t->size);

  r = new_render(oldpage, RENDER_TEXT, *left - width, oldy - _htmlSizes[t->size],
                 width, _htmlSpacings[t->size], number);
  r->data.text.typeface = typeface;
  r->data.text.style    = t->style;
  r->data.text.size     = _htmlSizes[t->size];
  r->data.text.rgb[0]   = t->red / 255.0f;
  r->data.text.rgb[1]   = t->green / 255.0f;
  r->data.text.rgb[2]   = t->blue / 255.0f;

  list_values[t->indent] ++;

  if (t->indent == 0)
  {
    // Adjust left margin when no UL/OL/DL is being used...
    *left -= _htmlSizes[t->size];
  }
}


/*
 * 'init_list()' - Initialize the list type and value as necessary.
 */

static void
init_list(tree_t *t)		/* I - List entry */
{
  uchar		*type,		/* TYPE= variable */
		*value;		/* VALUE= variable */
  static uchar	*symbols = (uchar *)"\327\267\250\340";


  if ((type = htmlGetVariable(t, (uchar *)"TYPE")) != NULL)
  {
    if (strlen((char *)type) == 1)
      list_types[t->indent] = type[0];
    else if (strcasecmp((char *)type, "disc") == 0 ||
             strcasecmp((char *)type, "circle") == 0)
      list_types[t->indent] = symbols[1];
    else
      list_types[t->indent] = symbols[2];
  }
  else if (t->markup == MARKUP_UL)
    list_types[t->indent] = symbols[t->indent & 3];
  else if (t->markup == MARKUP_OL)
    list_types[t->indent] = '1';

  if ((value = htmlGetVariable(t, (uchar *)"VALUE")) == NULL)
    value = htmlGetVariable(t, (uchar *)"START");

  if (value != NULL)
  {
    if (isdigit(value[0]))
      list_values[t->indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values[t->indent] = value[0] - 'A' + 1;
    else
      list_values[t->indent] = value[0] - 'a' + 1;
  }
  else if (t->markup == MARKUP_OL)
    list_values[t->indent] = 1;
}


/*
 * 'parse_comment()' - Parse a comment for HTMLDOC comments.
 */

#ifdef COMMENT_DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x) puts(x)
#  define DEBUG
#  undef DEBUG_printf
#  define DEBUG_printf(x) printf x
#endif /* COMMENT_DEBUG */

static void
parse_comment(tree_t *t,	/* I - Tree to parse */
              float  *left,	/* I - Left margin */
              float  *right,	/* I - Printable width */
              float  *bottom,	/* I - Bottom margin */
              float  *top,	/* I - Printable top */
              float  *x,	/* IO - X position */
              float  *y,	/* IO - Y position */
              int    *page,	/* IO - Page # */
	      tree_t *para,	/* I - Current paragraph */
	      int    needspace)	/* I - Need whitespace? */
{
  int		i;		/* Looping var */
  const char	*comment;	/* Comment text */
  char		*ptr,		/* Pointer into value string */
		buffer[1024];	/* Buffer for strings */
  int		pos,		/* Position (left, center, right) */
		tof;		/* Top of form */


  DEBUG_printf(("parse_comment(t=%p, left=%.1f, right=%.1f, bottom=%.1f, "
                "top=%.1f, x=%.1f, y=%.1f, page=%d, para=%p, needspace=%d\n",
                t, *left, *right, *bottom, *top, *x, *y, *page, para,
		needspace));

  if (t->data == NULL)
    return;

  if (para != NULL && para->child != NULL && para->child->next == NULL &&
      para->child->child == NULL && para->child->markup == MARKUP_NONE &&
      strcmp((const char *)para->child->data, " ") == 0)
  {
    // Remove paragraph consisting solely of whitespace...
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
  }

  // Mark if we are at the top of form...
  tof = (*y >= *top);

  DEBUG_printf(("BEFORE tof=%d, *y=%.1f, *top=%.1f, *page=%d, t->data=\"%s\"\n",
        	tof, *y, *top, *page, t->data));
  DEBUG_printf((" PagePrintWidth = %d\n", PagePrintWidth));
  DEBUG_printf(("PagePrintLength = %d\n", PagePrintLength));
  DEBUG_printf(("      PageWidth = %d\n", PageWidth));
  DEBUG_printf(("     PageLength = %d\n", PageLength));
  DEBUG_printf(("       PageLeft = %d\n", PageLeft));
  DEBUG_printf(("     PageBottom = %d\n", PageBottom));
  DEBUG_printf(("      PageRight = %d\n", PageRight));
  DEBUG_printf(("        PageTop = %d\n", PageTop));
  DEBUG_printf(("      Landscape = %d\n", Landscape));


  for (comment = (const char *)t->data; *comment;)
  {
    // Skip leading whitespace...
    while (isspace(*comment))
      comment ++;

    if (!*comment)
      break;

    if (strncasecmp(comment, "PAGE BREAK", 10) == 0 &&
	(!comment[10] || isspace(comment[10])))
    {
     /*
      * <!-- PAGE BREAK --> generates a page break...
      */

      comment += 10;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = *left;
      *y = *top;

      tof = 1;
    }
    else if (strncasecmp(comment, "NEW PAGE", 8) == 0 &&
	     (!comment[8] || isspace(comment[8])))
    {
     /*
      * <!-- NEW PAGE --> generates a page break...
      */

      comment += 8;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = *left;
      *y = *top;

      tof = 1;
    }
    else if (strncasecmp(comment, "NEW SHEET", 9) == 0 &&
	     (!comment[9] || isspace(comment[9])))
    {
     /*
      * <!-- NEW SHEET --> generate a page break to a new sheet...
      */

      comment += 9;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (NumberUp == 1)
      {
        // NEW SHEET breaks to the next sheet of paper...
        (*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;
      }
      else
      {
        // NEW SHEET breaks to the next side/sheet...
        (*page) ++;

	for (i = *page - 1; i >= 0; i --)
	  if (pages[i].nup != NumberUp)
	    break;

        i ++;
	for (i = *page - i; (i % NumberUp) != 0; i ++, (*page) ++);
      }

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      *x = *left;
      *y = *top;

      tof = 1;
    }
    else if (strncasecmp(comment, "HALF PAGE", 9) == 0 &&
             (!comment[9] || isspace(comment[9])))
    {
     /*
      * <!-- HALF PAGE --> Go to the next half page.  If in the
      * top half of a page, go to the bottom half.  If in the
      * bottom half, go to the next page.
      */
      float halfway;


      comment += 9;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      halfway = 0.5f * (*top + *bottom);

      if (*y <= halfway)
      {
	(*page) ++;
	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*x = *left;
	*y = *top;

        tof = 1;
      }
      else
      {
	*x = *left;
	*y = halfway;

        tof = 0;
      }
    }
    else if (strncasecmp(comment, "NEED ", 5) == 0)
    {
     /*
      * <!-- NEED amount --> generate a page break if there isn't
      * enough remaining space...
      */

      comment += 5;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if ((*y - get_measurement(comment, _htmlSpacings[SIZE_P])) < *bottom)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      // Skip amount...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA COLOR ", 12) == 0)
    {
      // Media color for page...
      comment += 12;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      // Get color...
      if (*comment == '\"')
      {
	for (ptr = pages[*page].media_color, comment ++;
             *comment && *comment != '\"';
	     comment ++)
          if (ptr < (pages[*page].media_color +
	             sizeof(pages[*page].media_color) - 1))
	    *ptr++ = *comment;

        if (*comment == '\"')
	  comment ++;
      }
      else
      {
	for (ptr = pages[*page].media_color;
             *comment && !isspace(*comment);
	     comment ++)
          if (ptr < (pages[*page].media_color +
	             sizeof(pages[*page].media_color) - 1))
	    *ptr++ = *comment;
      }

      *ptr = '\0';
    }
    else if (strncasecmp(comment, "MEDIA POSITION ", 15) == 0)
    {
      // Media position for page...
      comment += 15;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      pages[*page].media_position = atoi(comment);

      // Skip position...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA TYPE ", 11) == 0)
    {
      // Media type for page...
      comment += 11;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      // Get type...
      if (*comment == '\"')
      {
	for (ptr = pages[*page].media_type, comment ++;
             *comment && *comment != '\"';
	     comment ++)
          if (ptr < (pages[*page].media_type +
	             sizeof(pages[*page].media_type) - 1))
	    *ptr++ = *comment;

        if (*comment == '\"')
	  comment ++;
      }
      else
      {
	for (ptr = pages[*page].media_type;
             *comment && !isspace(*comment);
	     comment ++)
          if (ptr < (pages[*page].media_type +
	             sizeof(pages[*page].media_type) - 1))
	    *ptr++ = *comment;
      }

      *ptr = '\0';
    }
    else if (strncasecmp(comment, "MEDIA SIZE ", 11) == 0)
    {
      // Media size...
      comment += 11;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

        tof = 1;
      }

      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      check_pages(*page);

      *right = PagePrintWidth - *right;
      *top   = PagePrintLength - *top;

      set_page_size(comment);

      if (Landscape)
      {
	PagePrintWidth  = PageLength - PageLeft - PageRight;
	PagePrintLength = PageWidth - PageTop - PageBottom;
      }
      else
      {
	PagePrintWidth  = PageWidth - PageLeft - PageRight;
	PagePrintLength = PageLength - PageTop - PageBottom;
      }

      *right = PagePrintWidth - *right;
      *top   = PagePrintLength - *top;

      *x = *left;
      *y = *top;

      pages[*page].width  = PageWidth;
      pages[*page].length = PageLength;

      // Skip width...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA LEFT ", 11) == 0)
    {
      // Left margin...
      comment += 11;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      *right   = PagePrintWidth - *right;
      PageLeft = pages[*page].left = get_measurement(comment);

      if (Landscape)
	PagePrintWidth = PageLength - PageRight - PageLeft;
      else
	PagePrintWidth = PageWidth - PageRight - PageLeft;

      *right = PagePrintWidth - *right;

      // Skip left...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA RIGHT ", 12) == 0)
    {
      // Right margin...
      comment += 12;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      *right    = PagePrintWidth - *right;
      PageRight = pages[*page].right = get_measurement(comment);

      if (Landscape)
	PagePrintWidth = PageLength - PageRight - PageLeft;
      else
	PagePrintWidth = PageWidth - PageRight - PageLeft;

      *right = PagePrintWidth - *right;

      // Skip right...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA BOTTOM ", 13) == 0)
    {
      // Bottom margin...
      comment += 13;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      *top       = PagePrintLength - *top;
      PageBottom = pages[*page].bottom = get_measurement(comment);

      if (Landscape)
        PagePrintLength = PageWidth - PageTop - PageBottom;
      else
        PagePrintLength = PageLength - PageTop - PageBottom;

      *top = PagePrintLength - *top;
      *y   = *top;

      // Skip bottom...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA TOP ", 10) == 0)
    {
      // Top margin...
      comment += 10;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);

        tof = 1;
      }

      *x = *left;

      check_pages(*page);

      *top    = PagePrintLength - *top;
      PageTop = pages[*page].top = get_measurement(comment);

      if (Landscape)
        PagePrintLength = PageWidth - PageTop - PageBottom;
      else
        PagePrintLength = PageLength - PageTop - PageBottom;

      *top = PagePrintLength - *top;
      *y   = *top;

      // Skip top...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA LANDSCAPE ", 16) == 0)
    {
      // Landscape on/off...
      comment += 16;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

        tof = 1;
      }

      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      *x = *left;

      check_pages(*page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
      {
        if (Landscape)
	{
	  *right         = PageLength - PageRight - *right;
	  PagePrintWidth = PageWidth - PageRight - PageLeft;
	  *right         = PageWidth - PageRight - *right;

	  *top            = PageWidth - PageTop - *top;
	  PagePrintLength = PageLength - PageTop - PageBottom;
	  *top            = PageLength - PageTop - *top;
        }

        Landscape = pages[*page].landscape = 0;
      }
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
        if (!Landscape)
	{
	  *top            = PageLength - PageTop - *top;
	  PagePrintLength = PageWidth - PageTop - PageBottom;
	  *top            = PageWidth - PageTop - *top;

	  *right         = PageWidth - PageRight - *right;
	  PagePrintWidth = PageLength - PageRight - PageLeft;
	  *right         = PageLength - PageRight - *right;
        }

        Landscape = pages[*page].landscape = 1;
      }

      *y = *top;

      // Skip landscape...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA DUPLEX ", 13) == 0)
    {
      // Duplex printing on/off...
      comment += 13;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (!tof)
      {
	(*page) ++;

	*y = *top;
        tof = 1;
      }

      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      *x = *left;

      check_pages(*page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
        PageDuplex = pages[*page].duplex = 0;
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
	if ((*page) & 1)
	{
	  (*page) ++;

          check_pages(*page);

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
	}

        PageDuplex = pages[*page].duplex = 1;
      }

      // Skip duplex...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "HEADER ", 7) == 0)
    {
      // Header string...
      comment += 7;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (strncasecmp(comment, "LEFT", 4) == 0 && isspace(comment[4]))
      {
        pos     = 0;
	comment += 4;
      }
      else if (strncasecmp(comment, "CENTER", 6) == 0 && isspace(comment[6]))
      {
        pos     = 1;
	comment += 6;
      }
      else if (strncasecmp(comment, "RIGHT", 5) == 0 && isspace(comment[5]))
      {
        pos     = 2;
	comment += 5;
      }
      else
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad HEADER position: \"%s\"", comment);
	return;
      }

      while (isspace(*comment))
	comment ++;

      if (*comment != '\"')
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad HEADER string: \"%s\"", comment);
	return;
      }

      for (ptr = buffer, comment ++; *comment && *comment != '\"'; comment ++)
      {
        if (*comment == '\\')
	  comment ++;

	if (ptr < (buffer + sizeof(buffer) - 1))
	  *ptr++ = *comment;
      }

      if (*comment == '\"')
        comment ++;

      *ptr = '\0';

      if (ptr > buffer)
        Header[pos] = strdup(buffer);
      else
        Header[pos] = NULL;

      if (tof)
      {
        DEBUG_printf(("Setting header %d for page %d to \"%s\"...\n",
	              pos, *page, Header[pos] ? Header[pos] : "(null)"));

	check_pages(*page);

	pages[*page].header[pos] = (uchar *)Header[pos];
      }

      // Adjust top margin as needed...
      float adjust, image_adjust, temp_adjust;

      if (maxhfheight > HeadFootSize)
	image_adjust = maxhfheight + HeadFootSize;
      else
	image_adjust = 2 * HeadFootSize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Header[pos] &&
	    (strstr(Header[pos], "$IMAGE") != NULL ||
	     strstr(Header[pos], "$HFIMAGE") != NULL))
	  temp_adjust = image_adjust;
	else if (Header1[pos] &&
		 (strstr(Header1[pos], "$IMAGE") != NULL ||
		  strstr(Header1[pos], "$HFIMAGE") != NULL))
	  temp_adjust = image_adjust;
	else if (Header[pos] || Header1[pos])
	  temp_adjust = 2 * HeadFootSize;
	else
	  temp_adjust = 0.0;

	if (temp_adjust > adjust)
	  adjust = temp_adjust;
      }

      *top = PagePrintLength - adjust;

      if (tof)
        *y = *top;
    }
    else if (strncasecmp(comment, "HEADER1 ", 8) == 0)
    {
      // First page header string...
      comment += 8;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (strncasecmp(comment, "LEFT", 4) == 0 && isspace(comment[4]))
      {
        pos     = 0;
	comment += 4;
      }
      else if (strncasecmp(comment, "CENTER", 6) == 0 && isspace(comment[6]))
      {
        pos     = 1;
	comment += 6;
      }
      else if (strncasecmp(comment, "RIGHT", 5) == 0 && isspace(comment[5]))
      {
        pos     = 2;
	comment += 5;
      }
      else
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad HEADER1 position: \"%s\"", comment);
	return;
      }

      while (isspace(*comment))
	comment ++;

      if (*comment != '\"')
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad HEADER1 string: \"%s\"", comment);
	return;
      }

      for (ptr = buffer, comment ++; *comment && *comment != '\"'; comment ++)
      {
        if (*comment == '\\')
	  comment ++;

	if (ptr < (buffer + sizeof(buffer) - 1))
	  *ptr++ = *comment;
      }

      if (*comment == '\"')
        comment ++;

      *ptr = '\0';

      if (ptr > buffer)
        Header1[pos] = strdup(buffer);
      else
        Header1[pos] = NULL;

      // Adjust top margin as needed...
      float adjust, image_adjust, temp_adjust;

      if (maxhfheight > HeadFootSize)
	image_adjust = maxhfheight + HeadFootSize;
      else
	image_adjust = 2 * HeadFootSize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Header[pos] &&
	    (strstr(Header[pos], "$IMAGE") != NULL ||
	     strstr(Header[pos], "$HFIMAGE") != NULL))
	  temp_adjust = image_adjust;
	else if (Header1[pos] &&
		 (strstr(Header1[pos], "$IMAGE") != NULL ||
		  strstr(Header1[pos], "$HFIMAGE") != NULL))
	  temp_adjust = image_adjust;
	else if (Header[pos] || Header1[pos])
	  temp_adjust = 2 * HeadFootSize;
	else
	  temp_adjust = 0.0;

	if (temp_adjust > adjust)
	  adjust = temp_adjust;
      }

      *top = PagePrintLength - adjust;

      if (tof)
        *y = *top;
    }
    else if (strncasecmp(comment, "FOOTER ", 7) == 0)
    {
      // Footer string...
      comment += 7;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (strncasecmp(comment, "LEFT", 4) == 0 && isspace(comment[4]))
      {
        pos     = 0;
	comment += 4;
      }
      else if (strncasecmp(comment, "CENTER", 6) == 0 && isspace(comment[6]))
      {
        pos     = 1;
	comment += 6;
      }
      else if (strncasecmp(comment, "RIGHT", 5) == 0 && isspace(comment[5]))
      {
        pos     = 2;
	comment += 5;
      }
      else
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad FOOTER position: \"%s\"", comment);
	return;
      }

      while (isspace(*comment))
	comment ++;

      if (*comment != '\"')
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad FOOTER string: \"%s\"", comment);
	return;
      }

      for (ptr = buffer, comment ++; *comment && *comment != '\"'; comment ++)
      {
        if (*comment == '\\')
	  comment ++;

	if (ptr < (buffer + sizeof(buffer) - 1))
	  *ptr++ = *comment;
      }

      if (*comment == '\"')
        comment ++;

      *ptr = '\0';

      if (ptr > buffer)
        Footer[pos] = strdup(buffer);
      else
        Footer[pos] = NULL;

      if (tof)
      {
	check_pages(*page);

	pages[*page].footer[pos] = (uchar *)Footer[pos];
      }

      // Adjust bottom margin as needed...
      float adjust, image_adjust, temp_adjust;

      if (maxhfheight > HeadFootSize)
	image_adjust = maxhfheight + HeadFootSize;
      else
	image_adjust = 2 * HeadFootSize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Footer[pos] &&
	    (strstr(Footer[pos], "$IMAGE") != NULL ||
	     strstr(Footer[pos], "$HFIMAGE") != NULL))
	  temp_adjust = image_adjust;
	else if (Footer[pos])
	  temp_adjust = 2 * HeadFootSize;
	else
	  temp_adjust = 0.0;

	if (temp_adjust > adjust)
	  adjust = temp_adjust;
      }

      *bottom = adjust;
    }
    else if (strncasecmp(comment, "NUMBER-UP ", 10) == 0)
    {
      // N-up printing...
      comment += 10;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      NumberUp = strtol(comment, (char **)&comment, 10);

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= *top);
      }

      if (tof)
      {
	check_pages(*page);

        pages[*page].nup = NumberUp;
      }
    }
    else
      break;
  }

  DEBUG_printf(("LEAVING parse_comment() x=%.1f, y=%.1f, page=%d\n",
                *x, *y, *page));
  DEBUG_printf((" PagePrintWidth = %d\n", PagePrintWidth));
  DEBUG_printf(("PagePrintLength = %d\n", PagePrintLength));
  DEBUG_printf(("      PageWidth = %d\n", PageWidth));
  DEBUG_printf(("     PageLength = %d\n", PageLength));
  DEBUG_printf(("       PageLeft = %d\n", PageLeft));
  DEBUG_printf(("     PageBottom = %d\n", PageBottom));
  DEBUG_printf(("      PageRight = %d\n", PageRight));
  DEBUG_printf(("        PageTop = %d\n", PageTop));
  DEBUG_printf(("      Landscape = %d\n", Landscape));
}

#ifdef COMMENT_DEBUG
#  undef DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x)
#  undef DEBUG_printf
#  define DEBUG_printf(x)
#endif /* COMMENT_DEBUG */


/*
 * 'find_background()' - Find the background image/color for the given document.
 */

static void
find_background(tree_t *t)	/* I - Document to search */
{
  uchar		*var;		/* BGCOLOR/BACKGROUND variable */


 /*
  * First see if the --bodycolor or --bodyimage options have been
  * specified...
  */

  if (BodyImage[0] != '\0')
  {
    background_image = image_load(BodyImage, !OutputColor);
    return;
  }
  else if (BodyColor[0] != '\0')
  {
    get_color((uchar *)BodyColor, background_color, 0);
    return;
  }

 /*
  * If not, search the document tree...
  */

  while (t != NULL && background_image == NULL &&
         background_color[0] == 1.0 && background_color[1] == 1.0 &&
	 background_color[2] == 1.0)
  {
    if (t->markup == MARKUP_BODY)
    {
      if ((var = htmlGetVariable(t, (uchar *)"BACKGROUND")) != NULL)
        background_image = image_load((char *)var, !OutputColor);

      if ((var = htmlGetVariable(t, (uchar *)"BGCOLOR")) != NULL)
        get_color(var, background_color, 0);
    }

    if (t->child != NULL)
      find_background(t->child);

    t = t->next;
  }
}


/*
 * 'write_background()' - Write the background image/color for to the current
 *                        page.
 */

static void
write_background(int  page,	/* I - Page we are writing for */
                 FILE *out)	/* I - File to write to */
{
  float	x, y;
  float	width, height;
  int	page_width, page_length;


  if (Landscape)
  {
    page_length = pages[page].width;
    page_width  = pages[page].length;
  }
  else
  {
    page_width  = pages[page].width;
    page_length = pages[page].length;
  }

  if (background_color[0] != 1.0 ||
      background_color[1] != 1.0 ||
      background_color[2] != 1.0)
  {
    if (PSLevel > 0)
    {
      render_x = -1.0;
      render_y = -1.0;
      set_color(out, background_color);
      fprintf(out, "0 0 M %d %d F\n", page_width, page_length);
    }
    else
    {
      set_color(out, background_color);
      flate_printf(out, "0 0 %d %d re f\n", page_width, page_length);
    }
  }

  if (background_image != NULL)
  {
    width  = background_image->width * 72.0f / _htmlPPI;
    height = background_image->height * 72.0f / _htmlPPI;

    if (width < 1.0f)
      width = 1.0f;
    if (height < 1.0f)
      height = 1.0f;

    switch (PSLevel)
    {
      case 0 :
          for (x = 0.0; x < page_width; x += width)
            for (y = page_length; y >= 0.0f;)
            {
	      y -= height;
  	      flate_printf(out, "q %.1f 0 0 %.1f %.1f %.1f cm", width, height, x, y);
              flate_printf(out, "/I%d Do\n", background_image->obj);
	      flate_puts("Q\n", out);
            }
	  break;

      default :
          fprintf(out, "0 %.1f %d{/y exch neg %d add def\n",
	          height, page_length + (int)height - 1, page_length);
	  fprintf(out, "0 %.1f %d{/x exch def\n",
	          width, page_width);
          fprintf(out, "GS[%.1f 0 0 %.1f x y]CM/iy -1 def\n", width, height);
	  fprintf(out, "%d %d 8[%d 0 0 %d 0 %d]",
	          background_image->width, background_image->height,
                  background_image->width, -background_image->height,
		  background_image->height);
          fputs("{/iy iy 1 add def BG iy get}", out);
	  if (background_image->depth == 1)
	    fputs("image\n", out);
	  else
	    fputs("false 3 colorimage\n", out);
	  fputs("GR}for}for\n", out);
          break;
    }
  }
}


/*
 * 'new_render()' - Allocate memory for a new rendering structure.
 */

static render_t *			/* O - New render structure */
new_render(int      page,		/* I - Page number (0-n) */
           int      type,		/* I - Type of render primitive */
           float    x,			/* I - Horizontal position */
           float    y,			/* I - Vertical position */
           float    width,		/* I - Width */
           float    height,		/* I - Height */
           void     *data,		/* I - Data */
	   render_t *insert)		/* I - Insert before here... */
{
  render_t		*r;		/* New render primitive */
  size_t		datalen = 0;	/* Length of data */
  static render_t	dummy;		/* Dummy var for errors... */


  DEBUG_printf(("new_render(page=%d, type=%d, x=%.1f, y=%.1f, width=%.1f, height=%.1f, data=%p, insert=%p)\n",
                page, type, x, y, width, height, data, insert));

  check_pages(page);

  if (page < 0 || page >= alloc_pages)
  {
    progress_error(HD_ERROR_INTERNAL_ERROR,
                   "Page number (%d) out of range (1...%d)\n", page + 1,
                   alloc_pages);
    memset(&dummy, 0, sizeof(dummy));
    return (&dummy);
  }

  if ((type != RENDER_TEXT && type != RENDER_LINK) || data == NULL)
    r = (render_t *)calloc(sizeof(render_t), 1);
  else
  {
    datalen = strlen((char *)data);
    r       = (render_t *)calloc(sizeof(render_t) + datalen, 1);
  }

  if (r == NULL)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory on page %s\n", page + 1);
    memset(&dummy, 0, sizeof(dummy));
    return (&dummy);
  }

  r->type   = type;
  r->x      = x;
  r->y      = y;
  r->width  = width;
  r->height = height;

  switch (type)
  {
    case RENDER_TEXT :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
	// Safe because buffer is allocated...
        memcpy((char *)r->data.text.buffer, (char *)data, datalen);
        get_color(_htmlTextColor, r->data.text.rgb);
        break;
    case RENDER_IMAGE :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
        r->data.image = (image_t *)data;
        break;
    case RENDER_BOX :
        memcpy(r->data.box, data, sizeof(r->data.box));
        break;
    case RENDER_LINK :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
	// Safe because buffer is allocated...
        memcpy((char *)r->data.link, (char *)data, datalen);
        break;
  }

  if (insert)
  {
    if (insert->prev)
      insert->prev->next = r;
    else
      pages[page].start = r;

    r->prev      = insert->prev;
    r->next      = insert;
    insert->prev = r;
  }
  else
  {
    if (pages[page].end != NULL)
      pages[page].end->next = r;
    else
      pages[page].start = r;

    r->next         = NULL;
    r->prev         = pages[page].end;
    pages[page].end = r;
  }

  DEBUG_printf(("    returning r = %p\n", r));

  return (r);
}


/*
 * 'check_pages()' - Allocate memory for more pages as needed...
 */

static void
check_pages(int page)	// I - Current page
{
  page_t	*temp;	// Temporary page pointer


  DEBUG_printf(("check_pages(%d)\n", page));

  // See if we need to allocate memory for the page...
  if (page >= alloc_pages)
  {
    // Yes, allocate enough for ALLOC_PAGES more pages...
    while (page >= alloc_pages)
      alloc_pages += ALLOC_PAGES;

    // Do the pages pointers...
    if (num_pages == 0)
      temp = (page_t *)malloc(sizeof(page_t) * alloc_pages);
    else
      temp = (page_t *)realloc(pages, sizeof(page_t) * alloc_pages);

    if (temp == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for %d pages - %s",
	             alloc_pages, strerror(errno));
      alloc_pages -= ALLOC_PAGES;
      return;
    }

    memset(temp + num_pages, 0, (alloc_pages - num_pages) * sizeof(page_t));

    pages = temp;
  }

  // Initialize the page data as needed...
  for (temp = pages + num_pages; num_pages <= page; num_pages ++, temp ++)
  {
    if (!temp->width)
    {
      if (num_pages == 0 || !temp[-1].width || !temp[-1].length || chapter == 0)
      {
	temp->width     = PageWidth;
	temp->length    = PageLength;
	temp->left      = PageLeft;
	temp->right     = PageRight;
	temp->top       = PageTop;
	temp->bottom    = PageBottom;
	temp->duplex    = PageDuplex;
	temp->landscape = Landscape;
	temp->nup       = NumberUp;
      }
      else
      {
	memcpy(temp, temp - 1, sizeof(page_t));
	temp->start = NULL;
	temp->end   = NULL;
      }

      if (chapter == 0)
      {
	memcpy(temp->header, TocHeader, sizeof(temp->header));
	memcpy(temp->footer, TocFooter, sizeof(temp->footer));
      }
      else
      {
	memcpy(temp->header, Header, sizeof(temp->header));
	memcpy(temp->header1, Header1, sizeof(temp->header1));
	memcpy(temp->footer, Footer, sizeof(temp->footer));

        if (current_heading != temp->headnode)
	{
	  temp->heading  = htmlGetText(current_heading);
	  temp->headnode = current_heading;
	}
      }

      memcpy(temp->background_color, background_color,
             sizeof(temp->background_color));
      temp->background_image = background_image;
    }
  }
}


/*
 * 'add_link()' - Add a named link...
 */

static void
add_link(uchar *name,		/* I - Name of link */
         int   page,		/* I - Page # */
         int   top)		/* I - Y position */
{
  link_t	*temp;		/* New name */


  if (name == NULL)
    return;

  DEBUG_printf(("add_link(name=\"%s\", page=%d, top=%d)\n", name, page, top));

  if ((temp = find_link(name)) != NULL)
  {
    temp->page = page;
    temp->top  = top;
  }
  else
  {
    // See if we need to allocate memory for links...
    if (num_links >= alloc_links)
    {
      // Allocate more links...
      alloc_links += ALLOC_LINKS;

      if (num_links == 0)
        temp = (link_t *)malloc(sizeof(link_t) * alloc_links);
      else
        temp = (link_t *)realloc(links, sizeof(link_t) * alloc_links);

      if (temp == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for %d links - %s",
	               alloc_links, strerror(errno));
        alloc_links -= ALLOC_LINKS;
	return;
      }

      links = temp;
    }

    // Add a new link...
    temp = links + num_links;
    num_links ++;

    strlcpy((char *)temp->name, (char *)name, sizeof(temp->name));
    temp->page = page;
    temp->top  = top;

    if (num_links > 1)
      qsort(links, num_links, sizeof(link_t),
            (compare_func_t)compare_links);
  }
}


/*
 * 'find_link()' - Find a named link...
 */

static link_t *
find_link(uchar *name)	/* I - Name to find */
{
  link_t	key,	/* Search key */
		*match;	/* Matching name entry */


  if (name == NULL || num_links == 0)
    return (NULL);

  if (name[0] == '#')
    name ++;

  strlcpy((char *)key.name, (char *)name, sizeof(key.name));
  match = (link_t *)bsearch(&key, links, num_links, sizeof(link_t),
                            (compare_func_t)compare_links);

  return (match);
}


/*
 * 'compare_links()' - Compare two named links.
 */

static int			/* O - 0 = equal, -1 or 1 = not equal */
compare_links(link_t *n1,	/* I - First name */
              link_t *n2)	/* I - Second name */
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


#ifdef TABLE_DEBUG
#  undef DEBUG_printf
#  undef DEBUG_puts
#  define DEBUG_printf(x) printf x
#  define DEBUG_puts(x) puts(x)
#endif /* TABLE_DEBUG */

//
// 'get_cell_size()' - Compute the minimum width of a cell.
//

static float				// O - Required width of cell
get_cell_size(tree_t *t,		// I - Cell
              float  left,		// I - Left margin
	      float  right,		// I - Right margin
	      float  *minwidth,		// O - Minimum width
	      float  *prefwidth,	// O - Preferred width
	      float  *minheight)	// O - Minimum height
{
  tree_t	*temp,			// Current tree entry
		*next;			// Next tree entry
  uchar		*var;			// Attribute value
  int		nowrap;			// NOWRAP attribute?
  float		width,			// Width of cell
		frag_width,		// Fragment required width
		frag_height,		// Fragment height
		frag_pref,		// Fragment preferred width
		frag_min,		// Fragment minimum width
		minh,			// Local minimum height
		minw,			// Local minimum width
		prefw,			// Local preferred width
		format_width;		// Working format width for images


  DEBUG_printf(("get_cell_size(%p, %.1f, %.1f, %p, %p, %p)\n",
                t, left, right, minwidth, prefwidth, minheight));

  // First see if the width has been specified for this cell...
  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL &&
      (var[strlen((char *)var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      width = (right - left) * atoi((char *)var) * 0.01f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    width = 0.0f;

  if ((format_width = right - left) <= 0.0f)
    format_width = PagePrintWidth;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = htmlGetVariable(t, (uchar *)"HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      minh = PagePrintLength * atoi((char *)var) * 0.01f;
    else
      minh = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    minh = 0.0f;

  nowrap = (htmlGetVariable(t, (uchar *)"NOWRAP") != NULL);

  DEBUG_printf(("nowrap = %d\n", nowrap));

  for (temp = t->child, frag_width = 0.0f, frag_pref = 0.0f;
       temp != NULL;
       temp = next)
  {
    // Point to next markup, if any...
    next = temp->child;

    switch (temp->markup)
    {
      case MARKUP_TABLE :
	  // Update widths...
	  if (frag_pref > prefw)
	    prefw = frag_pref;

	  if (frag_width > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	        	  frag_width, minw));
	    minw = frag_width;
	  }

	  if (nowrap && frag_pref > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for break...\n",
	        	  frag_pref, minw));
	    minw = frag_pref;
	  }

          // For nested tables, compute the width of the table.
          frag_width = get_table_size(temp, left, right, &frag_min,
	                              &frag_pref, &frag_height);

	  if (frag_pref > prefw)
	    prefw = frag_pref;

	  if (frag_min > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for nested table...\n",
	                  frag_min, minw));
	    minw = frag_min;
	  }

	  frag_width = 0.0f;
	  frag_pref  = 0.0f;
	  frag_min   = 0.0f;
	  next       = NULL;
	  break;

      case MARKUP_IMG :
          // Update the image width as needed...
	  if (temp->markup == MARKUP_IMG)
	    update_image_size(temp);
      case MARKUP_NONE :
      case MARKUP_SPACER :
          frag_height = temp->height;

#ifdef TABLE_DEBUG2
          if (temp->markup == MARKUP_NONE)
	    printf("FRAG(%s) = %.1f\n", temp->data, temp->width);
	  else if (temp->markup == MARKUP_SPACER)
	    printf("SPACER = %.1f\n", temp->width);
	  else
	    printf("IMG(%s) = %.1f\n", htmlGetVariable(temp, (uchar *)"SRC"),
	           temp->width);
#endif // TABLE_DEBUG2

          // Handle min/preferred widths separately...
          if (temp->width > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for fragment...\n",
	                  temp->width, minw));
	    minw = temp->width;
	  }

          if (temp->preformatted && temp->data != NULL &&
              temp->data[strlen((char *)temp->data) - 1] == '\n')
          {
	    // End of a line - check preferred width...
	    frag_pref += temp->width + 1;

            if (frag_pref > prefw)
              prefw = frag_pref;

            if (temp->preformatted && frag_pref > minw)
	    {
	      DEBUG_printf(("Setting minw to %.1f (was %.1f) for preformatted...\n",
	                    frag_pref, minw));
              minw = frag_pref;
	    }

	    frag_pref = 0.0f;
          }
          else if (temp->data != NULL)
	    frag_pref += temp->width + 1;
	  else if ((frag_pref + temp->width) > format_width)
	  {
	    // parse_paragraph() will force a break
            if (frag_pref > prefw)
              prefw = frag_pref;

	    frag_pref = temp->width;
	  }
	  else
	    frag_pref += temp->width;

          if (temp->preformatted && temp->data != NULL &&
              temp->data[strlen((char *)temp->data) - 1] == '\n')
	  {
	    // Check required width...
            frag_width += temp->width + 1;

            if (frag_width > minw)
	    {
	      DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	                    frag_width, minw));
              minw = frag_width;
	    }

            frag_width = 0.0f;
	  }
          else if (!temp->preformatted && temp->data != NULL &&
	           (isspace(temp->data[0]) ||
	 	    isspace(temp->data[strlen((char *)temp->data) - 1])))
	  {
	    // Check required width...
	    if (isspace(temp->data[0]))
	      frag_width = temp->width + 1;
	    else
              frag_width += temp->width + 1;

            if (frag_width > minw)
	    {
	      DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	                    frag_width, minw));
              minw = frag_width;
	    }

	    if (!isspace(temp->data[0]))
              frag_width = 0.0f;

            DEBUG_printf(("frag_width=%.1f after whitespace processing...\n",
	                  frag_width));
	  }
	  else if (temp->data != NULL)
            frag_width += temp->width + 1;
	  else if ((frag_width + temp->width) > format_width)
	    // parse_paragraph() will force a break
	    frag_width = temp->width;
	  else
	    frag_width += temp->width;
	  break;

      case MARKUP_ADDRESS :
      case MARKUP_BLOCKQUOTE :
      case MARKUP_BR :
      case MARKUP_CENTER :
      case MARKUP_DD :
      case MARKUP_DIV :
      case MARKUP_DT :
      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
      case MARKUP_HR :
      case MARKUP_LI :
      case MARKUP_P :
      case MARKUP_PRE :
          DEBUG_printf(("BREAK at %.1f\n", frag_pref));

	  if (frag_pref > prefw)
	    prefw = frag_pref;

          if (frag_width > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	                  frag_width, minw));
            minw = frag_width;
	  }

	  if (nowrap && frag_pref > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for break...\n",
	                  frag_pref, minw));
	    minw = frag_pref;
	  }

          frag_pref   = 0.0f;
	  frag_width  = 0.0f;

      default :
          frag_height = 0.0f;
	  break;
    }

    // Update minimum height...
    if (frag_height > minh)
      minh = frag_height;

    // Update next pointer as needed...
    if (next == NULL)
      next = temp->next;

    if (next == NULL)
    {
      // This code is almost funny if you say it fast... :)
      for (next = temp->parent; next != NULL && next != t; next = next->parent)
	if (next->next != NULL)
	  break;

      if (next == t)
	next = NULL;
      else if (next)
	next = next->next;
    }
  }

  // Check the last fragment's width...
  if (frag_pref > prefw)
    prefw = frag_pref;

  if (frag_width > minw)
  {
    DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	          frag_width, minw));
    minw = frag_width;
  }

  // Handle the "NOWRAP" option...
  if (nowrap && prefw > minw)
  {
    DEBUG_printf(("Setting minw to %.1f (was %.1f) for NOWRAP...\n",
	          prefw, minw));
    minw = prefw;
  }

  // Return the required, minimum, and preferred size of the cell...
  *minwidth  = minw;
  *prefwidth = prefw;
  *minheight = minh;

  DEBUG_printf(("get_cell_size(): width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
                width, minw, prefw, minh));

  return (width);
}


//
// 'get_table_size()' - Compute the minimum width of a table.
//

static float				// O - Minimum width of table
get_table_size(tree_t *t,		// I - Table
               float  left,		// I - Left margin
	       float  right,		// I - Right margin
	       float  *minwidth,	// O - Minimum width
	       float  *prefwidth,	// O - Preferred width
	       float  *minheight)	// O - Minimum height
{
  tree_t	*temp,			// Current tree entry
		*next;			// Next tree entry
  uchar		*var;			// Attribute value
  float		width,			// Required width of table
		minw,			// Minimum width of table
		minh,			// Minimum height of table
		prefw,			// Preferred width of table
		cell_width,		// Cell required width
		cell_pref,		// Cell preferred width
		cell_min,		// Cell minimum width
		cell_height,		// Cell minimum height
		row_width,		// Row required width
		row_pref,		// Row preferred width
		row_min,		// Row minimum width
		row_height,		// Row minimum height
		border,			// Border around cells
		cellpadding,		// Padding inside cells
		cellspacing;		// Spacing around cells
  int		columns,		// Current number of columns
		max_columns,		// Maximum columns
		rows;			// Number of rows


  DEBUG_printf(("get_table_size(%p, %.1f, %.1f, %p, %p, %p)\n",
                t, left, right, minwidth, prefwidth, minheight));

  // First see if the width has been specified for this table...
  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL &&
      (var[strlen((char *)var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      width = (right - left) * atoi((char *)var) * 0.01f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    width = 0.0f;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = htmlGetVariable(t, (uchar *)"HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      minh = PagePrintLength * atoi((char *)var) * 0.01f;
    else
      minh = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    minh = 0.0f;

  // Update the size as needed...
  for (temp = t->child, row_width = 0.0f, row_min = 0.0f, row_pref = 0.0f,
	   row_height = 0.0f, columns = 0, rows = 0, max_columns = 0;
       temp != NULL;
       temp = next)
  {
    // Point to next markup, if any...
    next = temp->child;

    // Start a new row or add the cell width as needed...
    if (temp->markup == MARKUP_TR)
    {
      minh += row_height;

      row_width  = 0.0f;
      row_pref   = 0.0f;
      row_min    = 0.0f;
      row_height = 0.0f;
      rows ++;
      columns = 0;
    }
    else if (temp->markup == MARKUP_TD || temp->markup == MARKUP_TH)
    {
      // Update columns...
      columns ++;
      if (columns > max_columns)
	max_columns = columns;

      // Get widths of cell...
      cell_width = get_cell_size(temp, left, right, &cell_min, &cell_pref,
                                 &cell_height);

      // Update row widths...
      row_width += cell_width;
      row_pref  += cell_pref;
      row_min   += cell_min;

      if (cell_height > row_height)
	row_height = cell_height;

      // Check current row widths against table...
      if (row_pref > prefw)
	prefw = row_pref;

      if (row_min > minw)
	minw = row_min;
    }

    // Update next pointer as needed...
    if (next == NULL)
      next = temp->next;

    if (next == NULL)
    {
      // This code is almost funny if you say it fast... :)
      for (next = temp->parent; next != NULL && next != t; next = next->parent)
	if (next->next != NULL)
	  break;

      if (next == t)
	next = NULL;
      else if (next)
	next = next->next;
    }
  }

  // Make sure last row is counted in min height calcs.
  minh += row_height;

  // Add room for spacing and padding...
  if ((var = htmlGetVariable(t, (uchar *)"CELLPADDING")) != NULL)
    cellpadding = atoi((char *)var);
  else
    cellpadding = 1.0f;

  if ((var = htmlGetVariable(t, (uchar *)"CELLSPACING")) != NULL)
    cellspacing = atoi((char *)var);
  else
    cellspacing = 0.0f;

  if ((var = htmlGetVariable(t, (uchar *)"BORDER")) != NULL)
  {
    if ((border = atof((char *)var)) == 0.0 && var[0] != '0')
      border = 1.0f;

    cellpadding += border;
  }
  else
    border = 0.0f;

  if (border == 0.0f && cellpadding > 0.0f)
  {
   /*
    * Ah, the strange table formatting nightmare that is HTML.
    * Netscape and MSIE assign an invisible border width of 1
    * pixel if no border is specified...
    */

    cellpadding += 1.0f;
  }

  cellspacing *= PagePrintWidth / _htmlBrowserWidth;
  cellpadding *= PagePrintWidth / _htmlBrowserWidth;

  DEBUG_printf(("ADDING %.1f for table space for %d columns...\n",
                max_columns * (2 * cellpadding + cellspacing) - cellspacing,
		max_columns));

  if (width > 0.0f)
    width += max_columns * (2 * cellpadding + cellspacing) - cellspacing;

  minw  += max_columns * (2 * cellpadding + cellspacing) - cellspacing;
  prefw += max_columns * (2 * cellpadding + cellspacing) - cellspacing;
  minh  += rows * (2 * cellpadding + cellspacing) - cellspacing;

  // Return the required, minimum, and preferred size of the table...
  *minwidth  = minw;
  *prefwidth = prefw;
  *minheight = minh;

  DEBUG_printf(("get_table_size(): width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
                width, minw, prefw, minh));

  return (width);
}

#ifdef TABLE_DEBUG
#  undef DEBUG_printf
#  undef DEBUG_puts
#  define DEBUG_printf(x)
#  define DEBUG_puts(x)
#endif /* TABLE_DEBUG */


/*
 * 'flatten_tree()' - Flatten an HTML tree to only include the text, image,
 *                    link, and break markups.
 */

static tree_t *			/* O - Flattened markup tree */
flatten_tree(tree_t *t)		/* I - Markup tree to flatten */
{
  tree_t	*temp,		/* New tree node */
		*flat;		/* Flattened tree */


  flat = NULL;

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_NONE :
          if (t->data == NULL)
	    break;
      case MARKUP_BR :
      case MARKUP_SPACER :
      case MARKUP_IMG :
	  temp = (tree_t *)calloc(sizeof(tree_t), 1);
	  memcpy(temp, t, sizeof(tree_t));
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;

          if (temp->markup == MARKUP_IMG)
            update_image_size(temp);
          break;

      case MARKUP_A :
          if (htmlGetVariable(t, (uchar *)"NAME") != NULL)
          {
	    temp = (tree_t *)calloc(sizeof(tree_t), 1);
	    memcpy(temp, t, sizeof(tree_t));
	    temp->parent = NULL;
	    temp->child  = NULL;
	    temp->prev   = flat;
	    temp->next   = NULL;
	    if (flat != NULL)
              flat->next = temp;
            flat = temp;
          }
	  break;

      case MARKUP_P :
      case MARKUP_PRE :
      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
      case MARKUP_UL :
      case MARKUP_DIR :
      case MARKUP_MENU :
      case MARKUP_OL :
      case MARKUP_DL :
      case MARKUP_LI :
      case MARKUP_DD :
      case MARKUP_DT :
      case MARKUP_TR :
      case MARKUP_CAPTION :
	  temp = (tree_t *)calloc(sizeof(tree_t), 1);
	  temp->markup = MARKUP_BR;
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;
          break;

      default :
          break;
    }

    if (t->child != NULL && t->markup != MARKUP_UNKNOWN)
    {
      temp = flatten_tree(t->child);

      if (temp != NULL)
        temp->prev = flat;
      if (flat != NULL)
        flat->next = temp;
      else
        flat = temp;
    }

    if (flat != NULL)
      while (flat->next != NULL)
        flat = flat->next;

    t = t->next;
  }

  if (flat == NULL)
    return (NULL);

  while (flat->prev != NULL)
    flat = flat->prev;

  return (flat);
}


/*
 * 'update_image_size()' - Update the size of an image based upon the
 *                         printable width.
 */

static void
update_image_size(tree_t *t)	/* I - Tree entry */
{
  image_t	*img;		/* Image file */
  uchar		*width,		/* Width string */
		*height;	/* Height string */


  width  = htmlGetVariable(t, (uchar *)"WIDTH");
  height = htmlGetVariable(t, (uchar *)"HEIGHT");

  if (width != NULL && height != NULL)
  {
    if (width[strlen((char *)width) - 1] == '%')
      t->width = atof((char *)width) * PagePrintWidth / 100.0f;
    else
      t->width = atoi((char *)width) * PagePrintWidth / _htmlBrowserWidth;

    if (height[strlen((char *)height) - 1] == '%')
      t->height = atof((char *)height) * PagePrintWidth / 100.0f;
    else
      t->height = atoi((char *)height) * PagePrintWidth / _htmlBrowserWidth;

    return;
  }

  img = image_find((char *)htmlGetVariable(t, (uchar *)"REALSRC"));

  if (img == NULL)
    return;

  if (width != NULL)
  {
    if (width[strlen((char *)width) - 1] == '%')
      t->width = atof((char *)width) * PagePrintWidth / 100.0f;
    else
      t->width = atoi((char *)width) * PagePrintWidth / _htmlBrowserWidth;

    t->height = t->width * img->height / img->width;
  }
  else if (height != NULL)
  {
    if (height[strlen((char *)height) - 1] == '%')
      t->height = atof((char *)height) * PagePrintWidth / 100.0f;
    else
      t->height = atoi((char *)height) * PagePrintWidth / _htmlBrowserWidth;

    t->width = t->height * img->width / img->height;
  }
  else
  {
    t->width  = img->width * PagePrintWidth / _htmlBrowserWidth;
    t->height = img->height * PagePrintWidth / _htmlBrowserWidth;
  }
}


/*
 * 'get_width()' - Get the width of a string in points.
 */

static float			/* O - Width in points */
get_width(uchar *s,		/* I - String to scan */
          int   typeface,	/* I - Typeface code */
          int   style,		/* I - Style code */
          int   size)		/* I - Size */
{
  uchar	*ptr;			/* Current character */
  float	width;			/* Current width */


  DEBUG_printf(("get_width(\"%s\", %d, %d, %d)\n",
                s == NULL ? "(null)" : (const char *)s,
                typeface, style, size));

  if (s == NULL)
    return (0.0);

  for (width = 0.0, ptr = s; *ptr != '\0'; ptr ++)
    width += _htmlWidths[typeface][style][*ptr];

  return (width * _htmlSizes[size]);
}


/*
 * 'get_title()' - Get the title string for a document.
 */

static uchar *		/* O - Title string */
get_title(tree_t *doc)	/* I - Document */
{
  uchar	*temp;


  while (doc != NULL)
  {
    if (doc->markup == MARKUP_TITLE)
      return (htmlGetText(doc->child));
    else if (doc->child != NULL)
      if ((temp = get_title(doc->child)) != NULL)
        return (temp);
    doc = doc->next;
  }

  return (NULL);
}


/*
 * 'open_file()' - Open an output file for the current chapter.
 */

static FILE *		/* O - File pointer */
open_file(void)
{
  char	filename[255];	/* Filename */


  if (OutputFiles && PSLevel > 0)
  {
    if (chapter == -1)
      snprintf(filename, sizeof(filename), "%s/cover.ps", OutputPath);
    else if (chapter == 0)
      snprintf(filename, sizeof(filename), "%s/contents.ps", OutputPath);
    else
      snprintf(filename, sizeof(filename), "%s/doc%d.ps", OutputPath, chapter);

    return (fopen(filename, "wb+"));
  }
  else if (OutputFiles)
  {
    snprintf(filename, sizeof(filename), "%s/doc.pdf", OutputPath);

    return (fopen(filename, "wb+"));
  }
  else if (OutputPath[0] != '\0')
    return (fopen(OutputPath, "wb+"));
  else if (PSLevel == 0)
    return (file_temp(stdout_filename, sizeof(stdout_filename)));
  else
    return (stdout);
}


/*
 * 'set_color()' - Set the current text color...
 */

static void
set_color(FILE  *out,	/* I - File to write to */
          float *rgb)	/* I - RGB color */
{
  if (rgb[0] == render_rgb[0] &&
      rgb[1] == render_rgb[1] &&
      rgb[2] == render_rgb[2])
    return;

  render_rgb[0] = rgb[0];
  render_rgb[1] = rgb[1];
  render_rgb[2] = rgb[2];

  if (OutputColor)
  {
    // Output RGB color...
    if (PSLevel > 0)
      fprintf(out, "%.2f %.2f %.2f C ", rgb[0], rgb[1], rgb[2]);
    else
      flate_printf(out, "%.2f %.2f %.2f rg ", rgb[0], rgb[1], rgb[2]);
  }
  else
  {
    // Output grayscale...
    if (PSLevel > 0)
      fprintf(out, "%.2f G ",
              rgb[0] * 0.31f + rgb[1] * 0.61f + rgb[2] * 0.08f);
    else
      flate_printf(out, "%.2f g ",
                   rgb[0] * 0.31f + rgb[1] * 0.61f + rgb[2] * 0.08f);
  }
}


/*
 * 'set_font()' - Set the current text font.
 */

static void
set_font(FILE  *out,			/* I - File to write to */
         int   typeface,		/* I - Typeface code */
         int   style,			/* I - Style code */
         float size)			/* I - Size */
{
  char	sizes[255],	/* Formatted string for size... */
	*s;		/* Pointer to end of string */


  if (typeface == render_typeface &&
      style == render_style &&
      size == render_size)
    return;

 /*
  * Format size and strip trailing 0's and decimals...
  */

  sprintf(sizes, "%.1f", size);

  for (s = sizes + strlen(sizes) - 1; s > sizes && *s == '0'; s --)
    *s = '\0';

  if (*s == '.')
    *s = '\0';

 /*
  * Set the new typeface, style, and size.
  */

  if (PSLevel > 0)
  {
    if (size != render_size)
      fprintf(out, "%s FS", sizes);

    fprintf(out, "/F%x SF ", typeface * 4 + style);
  }
  else
    flate_printf(out, "/F%x %s Tf ", typeface * 4 + style, sizes);

  render_typeface = typeface;
  render_style    = style;
  render_size     = size;
}


/*
 * 'set_pos()' - Set the current text position.
 */

static void
set_pos(FILE  *out,			/* I - File to write to */
        float x,			/* I - X position */
        float y)			/* I - Y position */
{
  char	xs[255],			/* Formatted string for X... */
	ys[255],			/* Formatted string for Y... */
	*s;				/* Pointer to end of string */


  if (fabs(render_x - x) < 0.1 && fabs(render_y - y) < 0.1)
    return;

 /*
  * Format X and Y...
  */

  if (PSLevel > 0 || render_x == -1.0)
  {
    sprintf(xs, "%.3f", x);
    sprintf(ys, "%.3f", y);
  }
  else
  {
    sprintf(xs, "%.3f", x - render_startx);
    sprintf(ys, "%.3f", y - render_y);
  }

 /*
  * Strip trailing 0's and decimals...
  */

  for (s = xs + strlen(xs) - 1; s > xs && *s == '0'; s --)
    *s = '\0';

  if (*s == '.')
    *s = '\0';

  for (s = ys + strlen(ys) - 1; s > ys && *s == '0'; s --)
    *s = '\0';

  if (*s == '.')
    *s = '\0';

  if (PSLevel > 0)
    fprintf(out, "%s %s M", xs, ys);
  else
    flate_printf(out, "%s %s Td", xs, ys);

  render_x = render_startx = x;
  render_y = y;
}


/*
 * 'ps_hex()' - Print binary data as a series of hexadecimal numbers.
 */

static void
ps_hex(FILE  *out,			/* I - File to print to */
       uchar *data,			/* I - Data to print */
       int   length)			/* I - Number of bytes to print */
{
  int		col;
  static const char *hex = "0123456789ABCDEF";


  col = 0;
  while (length > 0)
  {
   /*
    * Put the hex uchars out to the file; note that we don't use fprintf()
    * for speed reasons...
    */

    putc(hex[*data >> 4], out);
    putc(hex[*data & 15], out);

    data ++;
    length --;

    col = (col + 1) % 40;
    if (col == 0)
      putc('\n', out);
  }

  if (col > 0)
    putc('\n', out);
}



#ifdef HTMLDOC_ASCII85
/*
 * 'ps_ascii85()' - Print binary data as a series of base-85 numbers.
 */

static void
ps_ascii85(FILE  *out,			/* I - File to print to */
	   uchar *data,			/* I - Data to print */
	   int   length,		/* I - Number of bytes to print */
	   int   eod)			/* I - 1 = end-of-data */
{
  unsigned	b;			/* Current 32-bit word */
  uchar		c[5];			/* Base-85 encoded characters */
  static int	col = 0;		/* Column */
  static uchar	leftdata[4];		/* Leftover data at the end */
  static int	leftcount = 0;		/* Size of leftover data */


  length += leftcount;

  while (length > 3)
  {
    switch (leftcount)
    {
      case 0 :
          b = (((((data[0] << 8) | data[1]) << 8) | data[2]) << 8) | data[3];
	  break;
      case 1 :
          b = (((((leftdata[0] << 8) | data[0]) << 8) | data[1]) << 8) | data[2];
	  break;
      case 2 :
          b = (((((leftdata[0] << 8) | leftdata[1]) << 8) | data[0]) << 8) | data[1];
	  break;
      case 3 :
          b = (((((leftdata[0] << 8) | leftdata[1]) << 8) | leftdata[2]) << 8) | data[0];
	  break;
    }

    if (col >= 76)
    {
      col = 0;
      putc('\n', out);
    }

    if (b == 0)
    {
      putc('z', out);
      col ++;
    }
    else
    {
      c[4] = (b % 85) + '!';
      b /= 85;
      c[3] = (b % 85) + '!';
      b /= 85;
      c[2] = (b % 85) + '!';
      b /= 85;
      c[1] = (b % 85) + '!';
      b /= 85;
      c[0] = b + '!';

      fwrite(c, 1, 5, out);
      col += 5;
    }

    data      += 4 - leftcount;
    length    -= 4 - leftcount;
    leftcount = 0;
  }

  if (length > 0)
  {
    // Copy any remainder into the leftdata array...
    if ((length - leftcount) > 0)
      memcpy(leftdata + leftcount, data, length - leftcount);

    memset(leftdata + length, 0, 4 - length);

    leftcount = length;
  }

  if (eod)
  {
    // Do the end-of-data dance...
    if (col >= 76)
    {
      col = 0;
      putc('\n', out);
    }

    if (leftcount > 0)
    {
      // Write the remaining bytes as needed...
      b = (((((leftdata[0] << 8) | leftdata[1]) << 8) | leftdata[2]) << 8) |
          leftdata[3];

      c[4] = (b % 85) + '!';
      b /= 85;
      c[3] = (b % 85) + '!';
      b /= 85;
      c[2] = (b % 85) + '!';
      b /= 85;
      c[1] = (b % 85) + '!';
      b /= 85;
      c[0] = b + '!';

      fwrite(c, leftcount + 1, 1, out);

      leftcount = 0;
    }

    fputs("~>\n", out);
    col = 0;
  }
}
#endif // HTMLDOC_ASCII85


/*
 * JPEG library destination data manager.  These routines direct
 * compressed data from libjpeg into the PDF or PostScript file.
 */

static FILE			*jpg_file;	/* JPEG file */
static uchar			jpg_buf[8192];	/* JPEG buffer */
static jpeg_destination_mgr	jpg_dest;	/* JPEG destination manager */
static struct jpeg_error_mgr	jerr;		/* JPEG error handler */


/*
 * 'jpg_init()' - Initialize the JPEG destination.
 */

static void
jpg_init(j_compress_ptr cinfo)		/* I - Compressor info */
{
  (void)cinfo;

  jpg_dest.next_output_byte = jpg_buf;
  jpg_dest.free_in_buffer   = sizeof(jpg_buf);
}


/*
 * 'jpg_empty()' - Empty the JPEG output buffer.
 */

static boolean				/* O - True if buffer written OK */
jpg_empty(j_compress_ptr cinfo)		/* I - Compressor info */
{
  (void)cinfo;

  if (PSLevel > 0)
#ifdef HTMLDOC_ASCII85
    ps_ascii85(jpg_file, jpg_buf, sizeof(jpg_buf));
#else
    ps_hex(jpg_file, jpg_buf, sizeof(jpg_buf));
#endif // HTMLDOC_ASCII85
  else
    flate_write(jpg_file, jpg_buf, sizeof(jpg_buf));

  jpg_dest.next_output_byte = jpg_buf;
  jpg_dest.free_in_buffer   = sizeof(jpg_buf);

  return (TRUE);
}


/*
 * 'jpg_term()' - Write the last JPEG data to the file.
 */

static void
jpg_term(j_compress_ptr cinfo)		/* I - Compressor info */
{
  int nbytes;				/* Number of bytes to write */


  (void)cinfo;

  nbytes = sizeof(jpg_buf) - jpg_dest.free_in_buffer;

  if (PSLevel > 0)
#ifdef HTMLDOC_ASCII85
    ps_ascii85(jpg_file, jpg_buf, nbytes);
#else
    ps_hex(jpg_file, jpg_buf, nbytes);
#endif // HTMLDOC_ASCII85
  else
    flate_write(jpg_file, jpg_buf, nbytes);
}


/*
 * 'jpg_setup()' - Setup the JPEG compressor for writing an image.
 */

static void
jpg_setup(FILE           *out,	/* I - Output file */
          image_t        *img,	/* I - Output image */
          j_compress_ptr cinfo)	/* I - Compressor info */
{
  int	i;			// Looping var


  jpg_file    = out;
  cinfo->err  = jpeg_std_error(&jerr);

  jpeg_create_compress(cinfo);

  cinfo->dest = &jpg_dest;
  jpg_dest.init_destination    = jpg_init;
  jpg_dest.empty_output_buffer = jpg_empty;
  jpg_dest.term_destination    = jpg_term;

  cinfo->image_width      = img->width;
  cinfo->image_height     = img->height;
  cinfo->input_components = img->depth;
  cinfo->in_color_space   = img->depth == 1 ? JCS_GRAYSCALE : JCS_RGB;

  jpeg_set_defaults(cinfo);
  jpeg_set_quality(cinfo, OutputJPEG, TRUE);

  // Update things when writing to PS files...
  if (PSLevel)
  {
    // Adobe uses sampling == 1
    for (i = 0; i < img->depth; i ++)
    {
      cinfo->comp_info[i].h_samp_factor = 1;
      cinfo->comp_info[i].v_samp_factor = 1;
    }
  }

  cinfo->write_JFIF_header  = FALSE;
  cinfo->write_Adobe_marker = TRUE;

  jpeg_start_compress(cinfo, TRUE);
}


/*
 * 'compare_rgb()' - Compare two RGB colors...
 */

static int				/* O - -1 if rgb1<rgb2, etc. */
compare_rgb(unsigned *rgb1,		/* I - First color */
            unsigned *rgb2)		/* I - Second color */
{
  return (*rgb1 - *rgb2);
}


/*
 * 'write_image()' - Write an image to the given output file...
 */

static void
write_image(FILE     *out,		/* I - Output file */
            render_t *r,		/* I - Image to write */
	    int      write_obj)		/* I - Write an object? */
{
  int		i, j, k, m,		/* Looping vars */
		ncolors;		/* Number of colors */
  uchar		*pixel,			/* Current pixel */
		*indices,		/* New indexed pixel array */
		*indptr;		/* Current index */
  int		indwidth,		/* Width of indexed line */
		indbits;		/* Bits per index */
  int		max_colors;		/* Max colors to use */
  unsigned	colors[256],		/* Colormap values */
		key,			/* Color key */
		*match;			/* Matching color value */
  uchar		grays[256],		/* Grayscale usage */
		cmap[256][3];		/* Colormap */
  image_t 	*img;			/* Image */
  struct jpeg_compress_struct cinfo;	/* JPEG compressor */
  uchar		*data,			/* PS Level 3 image data */
		*dataptr,		/* Pointer into image data */
		*maskptr;		/* Pointer into mask data */


 /*
  * See if we can optimize the image as indexed without color loss...
  */

  img      = r->data.image;
  ncolors  = 0;
  indices  = NULL;
  indwidth = 0;

  if (!img->pixels && !img->obj)
    image_load(img->filename, !OutputColor, 1);

  // Note: Acrobat 6 tries to decrypt the colormap of indexed in-line images twice, which
  //       is 1) not consistent with prior Acrobat releases and 2) in violation of their
  //       PDF spec.  The "img->use > 1 || !Encryption" test prevents the use of indexed
  //       in-line images when encryption is enabled.
  //
  //       We are filing a bug on this with Adobe, but if history is any indicator, we are
  //       stuck with this workaround forever...
  if (PSLevel != 1 && PDFVersion >= 12 && img->obj == 0 && (img->use > 1 || !Encryption))
  {
    if (img->depth == 1)
    {
     /*
      * Greyscale image...
      */

      memset(grays, 0, sizeof(grays));

      for (i = img->width * img->height, pixel = img->pixels;
	   i > 0;
	   i --, pixel ++)
	if (!grays[*pixel])
	{
          if (ncolors >= 16)
	    break;

	  grays[*pixel] = 1;
	  ncolors ++;
	}

      if (i == 0)
      {
	for (i = 0, j = 0; i < 256; i ++)
	  if (grays[i])
	  {
	    colors[j] = (((i << 8) | i) << 8) | i;
	    grays[i]  = j;
	    j ++;
	  }
      }
      else
        ncolors = 0;
    }
    else
    {
     /*
      * Color image...
      */

      if (OutputJPEG && !Compression)
        max_colors = 16;
      else
        max_colors = 256;

      for (i = img->width * img->height, pixel = img->pixels, match = NULL;
	   i > 0;
	   i --, pixel += 3)
      {
        key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	if (!match || *match != key)
	{
          if (ncolors > 0)
            match = (unsigned *)bsearch(&key, colors, ncolors, sizeof(unsigned),
                                        (compare_func_t)compare_rgb);
          else
            match = NULL;
        }

        if (match == NULL)
        {
          if (ncolors >= max_colors)
            break;

          colors[ncolors] = key;
          ncolors ++;

          if (ncolors > 1)
            qsort(colors, ncolors, sizeof(unsigned),
                  (compare_func_t)compare_rgb);
        }
      }

      if (i > 0)
        ncolors = 0;
    }
  }

  if (ncolors > 0)
  {
    if (PSLevel == 3 && img->mask)
      indbits = 8;
    else if (ncolors <= 2)
      indbits = 1;
    else if (ncolors <= 4)
      indbits = 2;
    else if (ncolors <= 16)
      indbits = 4;
    else
      indbits = 8;

    indwidth = (img->width * indbits + 7) / 8;
    indices  = (uchar *)calloc(indwidth, img->height + 1);
					// height + 1 for PS odd-row-count bug

    if (img->depth == 1)
    {
     /*
      * Convert a grayscale image...
      */

      switch (indbits)
      {
        case 1 :
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 7; j > 0; j --, k = (k + 7) & 7, pixel ++)
		switch (k)
		{
		  case 7 :
	              *indptr = grays[*pixel] << 7;
		      break;
		  default :
	              *indptr |= grays[*pixel] << k;
		      break;
		  case 0 :
	              *indptr++ |= grays[*pixel];
		      break;
        	}

	      if (k != 7)
		indptr ++;
	    }
	    break;

        case 2 :
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0; j > 0; j --, k = (k + 1) & 3, pixel ++)
		switch (k)
		{
		  case 0 :
	              *indptr = grays[*pixel] << 6;
		      break;
		  case 1 :
	              *indptr |= grays[*pixel] << 4;
		      break;
		  case 2 :
	              *indptr |= grays[*pixel] << 2;
		      break;
		  case 3 :
	              *indptr++ |= grays[*pixel];
		      break;
        	}

	      if (k)
		indptr ++;
	    }
	    break;

        case 4 :
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0; j > 0; j --, k ^= 1, pixel ++)
		if (k)
		  *indptr++ |= grays[*pixel];
		else
		  *indptr = grays[*pixel] << 4;

	      if (k)
		indptr ++;
	    }
	    break;
      }
    }
    else
    {
     /*
      * Convert a color image...
      */

      switch (indbits)
      {
        case 1 :
	    for (i = img->height, pixel = img->pixels, indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 7;
	           j > 0;
		   j --, k = (k + 7) & 7, pixel += 3)
	      {
                key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

		if (*match != key)
        	  match = (unsigned *)bsearch(&key, colors, ncolors,
		                              sizeof(unsigned),
                            	              (compare_func_t)compare_rgb);
	        m = match - colors;

		switch (k)
		{
		  case 7 :
	              *indptr = m << 7;
		      break;
		  default :
	              *indptr |= m << k;
		      break;
		  case 0 :
	              *indptr++ |= m;
		      break;
        	}
	      }

	      if (k != 7)
	        indptr ++;
	    }
	    break;

        case 2 :
	    for (i = img->height, pixel = img->pixels, indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0;
	           j > 0;
		   j --, k = (k + 1) & 3, pixel += 3)
	      {
                key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

		if (*match != key)
        	  match = (unsigned *)bsearch(&key, colors, ncolors,
		                              sizeof(unsigned),
                            	              (compare_func_t)compare_rgb);
	        m = match - colors;

		switch (k)
		{
		  case 0 :
	              *indptr = m << 6;
		      break;
		  case 1 :
	              *indptr |= m << 4;
		      break;
		  case 2 :
	              *indptr |= m << 2;
		      break;
		  case 3 :
	              *indptr++ |= m;
		      break;
        	}
	      }

	      if (k)
	        indptr ++;
	    }
	    break;

        case 4 :
	    for (i = img->height, pixel = img->pixels, indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0; j > 0; j --, k ^= 1, pixel += 3)
	      {
                key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

		if (*match != key)
        	  match = (unsigned *)bsearch(&key, colors, ncolors,
		                              sizeof(unsigned),
                            	              (compare_func_t)compare_rgb);
	        m = match - colors;

		if (k)
		  *indptr++ |= m;
		else
		  *indptr = m << 4;
	      }

	      if (k)
	        indptr ++;
	    }
	    break;

        case 8 :
	    for (i = img->height, pixel = img->pixels, indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width; j > 0; j --, pixel += 3, indptr ++)
	      {
                key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

		if (*match != key)
        	  match = (unsigned *)bsearch(&key, colors, ncolors,
		                              sizeof(unsigned),
                            	              (compare_func_t)compare_rgb);
	        *indptr = match - colors;
	      }
	    }
	    break;
      }
    }
  }
  else
    indbits = 8;

  if (ncolors == 1)
  {
   /*
    * Adobe doesn't like 1 color images...
    */

    ncolors   = 2;
    colors[1] = 0;
  }

 /*
  * Now write the image...
  */

  switch (PSLevel)
  {
    case 0 : /* PDF */
        if (!write_obj)
	  flate_printf(out, "q %.1f 0 0 %.1f %.1f %.1f cm\n", r->width, r->height,
	               r->x, r->y);

        if (img->obj)
	{
	  if (img->mask && PDFVersion < 13)
	    write_imagemask(out, r);

	  flate_printf(out, "/I%d Do Q\n", img->obj);
	  break;
	}

        if (img->mask && write_obj && PDFVersion >= 13)
	{
	  // We have a mask image, write it!
          pdf_start_object(out);
	  fputs("/Type/XObject/Subtype/Image", out);
          fputs("/ColorSpace/DeviceGray", out);
	  if (img->maskscale == 8)
	    fprintf(out, "/Width %d/Height %d/BitsPerComponent 8",
	            img->width, img->height);
          else
	    fprintf(out, "/Width %d/Height %d/BitsPerComponent 1/ImageMask true",
	            img->width * img->maskscale, img->height * img->maskscale);
          if (Compression)
            fputs("/Filter/FlateDecode", out);

          pdf_start_stream(out);
          flate_open_stream(out);
	  if (img->maskscale == 8)
  	    flate_write(out, img->mask, img->width * img->height);
	  else
  	    flate_write(out, img->mask,
	                img->maskwidth * img->height * img->maskscale);
	  flate_close_stream(out);

          pdf_end_object(out);
	}

        if (write_obj)
	{
	  // Write an image object...
	  img->obj = pdf_start_object(out);

	  fputs("/Type/XObject/Subtype/Image", out);
	  if (img->mask && PDFVersion >= 13)
	  {
	    if (img->maskscale == 8)
	      fprintf(out, "/SMask %d 0 R", img->obj - 1);
	    else
	      fprintf(out, "/Mask %d 0 R", img->obj - 1);
	  }

	  if (ncolors > 0)
	  {
	    for (i = 0; i < ncolors; i ++)
	    {
	      cmap[i][0] = colors[i] >> 16;
	      cmap[i][1] = colors[i] >> 8;
	      cmap[i][2] = colors[i];
	    }

	    if (Encryption)
	    {
	      // Encrypt the colormap...
	      encrypt_init();
	      rc4_encrypt(&encrypt_state, cmap[0], cmap[0], ncolors * 3);
	    }

	    fprintf(out, "/ColorSpace[/Indexed/DeviceRGB %d<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	      fprintf(out, "%02X%02X%02X", cmap[i][0], cmap[i][1],
	              cmap[i][2]);
	    fputs(">]", out);
          }
	  else if (img->depth == 1)
            fputs("/ColorSpace/DeviceGray", out);
          else
            fputs("/ColorSpace/DeviceRGB", out);

#ifdef HTMLDOC_INTERPOLATION
          if (ncolors != 2)
            fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

          if (Compression && (ncolors || !OutputJPEG))
            fputs("/Filter/FlateDecode", out);
	  else if (OutputJPEG && ncolors == 0)
	  {
	    if (Compression)
	      fputs("/Filter[/FlateDecode/DCTDecode]", out);
	    else
	      fputs("/Filter/DCTDecode", out);
	  }

  	  fprintf(out, "/Width %d/Height %d/BitsPerComponent %d",
	          img->width, img->height, indbits);
          pdf_start_stream(out);
          flate_open_stream(out);

          if (OutputJPEG && ncolors == 0)
	  {
	    jpg_setup(out, img, &cinfo);

	    for (i = img->height, pixel = img->pixels;
	         i > 0;
	         i --, pixel += img->width * img->depth)
	      jpeg_write_scanlines(&cinfo, &pixel, 1);

	    jpeg_finish_compress(&cinfo);
	    jpeg_destroy_compress(&cinfo);
	  }
          else
	  {
	    if (ncolors > 0)
   	      flate_write(out, indices, indwidth * img->height);
	    else
  	      flate_write(out, img->pixels,
	                  img->width * img->height * img->depth);
          }

          flate_close_stream(out);
          pdf_end_object(out);
	}
	else
	{
	  // Put the image in-line...
          flate_puts("BI", out);

	  if (ncolors > 0)
	  {
	    flate_printf(out, "/CS[/I/RGB %d<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	      flate_printf(out, "%02X%02X%02X", colors[i] >> 16,
	        	   (colors[i] >> 8) & 255, colors[i] & 255);
	    flate_puts(">]", out);
          }
	  else if (img->depth == 1)
            flate_puts("/CS/G", out);
          else
            flate_puts("/CS/RGB", out);

          if (ncolors != 2)
            flate_puts("/I true", out);

  	  flate_printf(out, "/W %d/H %d/BPC %d", img->width, img->height, indbits);

	  if (ncolors > 0)
	  {
  	    flate_puts(" ID\n", out);
  	    flate_write(out, indices, indwidth * img->height, 1);
	  }
	  else if (OutputJPEG)
	  {
  	    flate_puts("/F/DCT ID\n", out);

	    jpg_setup(out, img, &cinfo);

	    for (i = img->height, pixel = img->pixels;
	         i > 0;
	         i --, pixel += img->width * img->depth)
	      jpeg_write_scanlines(&cinfo, &pixel, 1);

	    jpeg_finish_compress(&cinfo);
	    jpeg_destroy_compress(&cinfo);
          }
	  else
	  {
  	    flate_puts(" ID\n", out);
  	    flate_write(out, img->pixels, img->width * img->height * img->depth, 1);
          }

	  flate_write(out, (uchar *)"\nEI\nQ\n", 6, 1);
	}
        break;

    case 1 : /* PostScript, Level 1 */
        fputs("GS", out);
	fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	        r->x, r->y);

	if (img->mask)
	  write_imagemask(out, r);

	fprintf(out, "/picture %d string def\n", img->width * img->depth);

	if (img->depth == 1)
	  fprintf(out, "%d %d 8 [%d 0 0 %d 0 %d] {currentfile picture readhexstring pop} image\n",
        	  img->width, img->height,
        	  img->width, -img->height,
        	  img->height);
	else
	  fprintf(out, "%d %d 8 [%d 0 0 %d 0 %d] {currentfile picture readhexstring pop} false 3 colorimage\n",
        	  img->width, img->height,
        	  img->width, -img->height,
        	  img->height);

	ps_hex(out, img->pixels, img->width * img->height * img->depth);

	fputs("GR\n", out);
        break;
    case 3 : /* PostScript, Level 3 */
        // Fallthrough to Level 2 output if compression is disabled and
	// we aren't doing transparency...
        if ((Compression && (!OutputJPEG || ncolors > 0)) ||
	    (img->mask && img->maskscale == 8))
	{
          fputs("GS", out);
	  fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	          r->x, r->y);

	  if (img->mask && img->maskscale != 8)
	    write_imagemask(out, r);

          if (ncolors > 0)
          {
	    if (ncolors <= 2)
	      ncolors = 2; /* Adobe doesn't like 1 color images... */

	    fprintf(out, "[/Indexed/DeviceRGB %d\n<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	    {
	      fprintf(out, "%02X%02X%02X", colors[i] >> 16,
	              (colors[i] >> 8) & 255, colors[i] & 255);
	      if ((i % 13) == 12)
	        putc('\n', out);
            }
	    fputs(">]setcolorspace\n", out);

	    if (img->mask && img->maskscale == 8)
	      fprintf(out, "<<"
	                   "/ImageType 3"
			   "/InterleaveType 1"
			   "/MaskDict<<"
	                   "/ImageType 1"
	                   "/Width %d"
	                   "/Height %d"
	                   "/BitsPerComponent 8"
	                   "/ImageMatrix[%d 0 0 %d 0 %d]"
			   "/Decode[0 1]"
	                   ">>\n"
			   "/DataDict",
	            img->width, img->height,
        	    img->width, -img->height, img->height);

	    fprintf(out, "<<"
	                 "/ImageType 1"
	                 "/Width %d"
	                 "/Height %d"
	                 "/BitsPerComponent %d"
	                 "/ImageMatrix[%d 0 0 %d 0 %d]"
	                 "/Decode[0 %d]",
	            img->width, img->height, indbits,
        	    img->width, -img->height, img->height,
        	    (1 << indbits) - 1);

#ifdef HTMLDOC_INTERPOLATION
            if (ncolors != 2)
	      fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
            fputs("/DataSource currentfile/ASCII85Decode filter", out);
#else
            fputs("/DataSource currentfile/ASCIIHexDecode filter", out);
#endif // HTMLDOC_ASCII85

            if (Compression)
	      fputs("/FlateDecode filter", out);

	    fputs(">>\n", out);

	    if (img->mask && img->maskscale == 8)
	      fputs(">>\n", out);

	    fputs("image\n", out);

            flate_open_stream(out);

	    if (img->mask && img->maskscale == 8)
	    {
	      data = (uchar *)malloc(img->width * 2);

	      for (i = 0, maskptr = img->mask, indptr = indices;
	           i < img->height;
		   i ++)
	      {
	        for (j = img->width, dataptr = data; j > 0; j --)
		{
		  *dataptr++ = *maskptr++;
		  *dataptr++ = *indptr++;
		}

		flate_write(out, data, img->width * 2);
	      }

	      free(data);
	    }
	    else
	      flate_write(out, indices, indwidth * img->height);

	    flate_close_stream(out);
          }
          else
          {
	    if (img->depth == 1)
	      fputs("/DeviceGray setcolorspace", out);
	    else
	      fputs("/DeviceRGB setcolorspace", out);

	    if (img->mask && img->maskscale == 8)
	      fprintf(out, "<<"
	                   "/ImageType 3"
			   "/InterleaveType 1"
			   "/MaskDict<<"
	                   "/ImageType 1"
	                   "/Width %d"
	                   "/Height %d"
	                   "/BitsPerComponent 8"
	                   "/ImageMatrix[%d 0 0 %d 0 %d]"
			   "/Decode[0 1]"
	                   ">>\n"
			   "/DataDict",
	            img->width, img->height,
        	    img->width, -img->height, img->height);

	    fprintf(out, "<<"
	                 "/ImageType 1"
	                 "/Width %d"
	                 "/Height %d"
	                 "/BitsPerComponent 8"
	                 "/ImageMatrix[%d 0 0 %d 0 %d]"
	                 "/Decode[%s]",
	            img->width, img->height,
        	    img->width, -img->height, img->height,
        	    img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_INTERPOLATION
	    fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
            fputs("/DataSource currentfile/ASCII85Decode filter", out);
#else
            fputs("/DataSource currentfile/ASCIIHexDecode filter", out);
#endif // HTMLDOC_ASCII85

            if (Compression)
	      fputs("/FlateDecode filter", out);

	    fputs(">>\n", out);

	    if (img->mask && img->maskscale == 8)
	      fputs(">>\n", out);

	    fputs("image\n", out);

            flate_open_stream(out);

	    if (img->mask && img->maskscale == 8)
	    {
	      data = (uchar *)malloc(img->width * (img->depth + 1));

	      for (i = 0, maskptr = img->mask, pixel = img->pixels;
	           i < img->height;
		   i ++)
	      {
	        if (img->depth == 1)
		{
	          for (j = img->width, dataptr = data; j > 0; j --)
		  {
		    *dataptr++ = *maskptr++;
		    *dataptr++ = *pixel++;
		  }
		}
		else
		{
	          for (j = img->width, dataptr = data; j > 0; j --)
		  {
		    *dataptr++ = *maskptr++;
		    *dataptr++ = *pixel++;
		    *dataptr++ = *pixel++;
		    *dataptr++ = *pixel++;
		  }
		}

		flate_write(out, data, img->width * (img->depth + 1));
	      }

	      free(data);
	    }
	    else
	      flate_write(out, img->pixels,
	                  img->width * img->height * img->depth);

	    flate_close_stream(out);
          }

	  fputs("GR\n", out);
          break;
	}

    case 2 : /* PostScript, Level 2 */
        fputs("GS", out);
	fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	        r->x, r->y);

	if (img->mask)
	  write_imagemask(out, r);

        if (ncolors > 0)
        {
	  fprintf(out, "[/Indexed/DeviceRGB %d\n<", ncolors - 1);
	  for (i = 0; i < ncolors; i ++)
	  {
	    fprintf(out, "%02X%02X%02X", colors[i] >> 16,
	            (colors[i] >> 8) & 255, colors[i] & 255);
	    if ((i % 13) == 12)
	      putc('\n', out);
          }

	  fputs(">]setcolorspace\n", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent %d"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[0 %d]",
	          img->width, img->height, indbits,
        	  img->width, -img->height, img->height,
        	  (1 << indbits) - 1);

#ifdef HTMLDOC_INTERPOLATION
          if (ncolors != 2)
	    fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
	  fputs("/DataSource currentfile/ASCII85Decode filter>>image\n", out);

          ps_ascii85(out, indices, indwidth * img->height, 1);
#else
	  fputs("/DataSource currentfile/ASCIIHexDecode filter>>image\n", out);

          ps_hex(out, indices, indwidth * img->height);
	  // End of data marker...
	  fputs(">\n", out);
#endif /* HTMLDOC_ASCII85 */
        }
	else if (OutputJPEG)
	{
	  if (img->depth == 1)
	    fputs("/DeviceGray setcolorspace\n", out);
	  else
	    fputs("/DeviceRGB setcolorspace\n", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent 8"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[%s]",
	          img->width, img->height,
        	  img->width, -img->height, img->height,
        	  img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_INTERPOLATION
	  fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
	  fputs("/DataSource currentfile/ASCII85Decode filter/DCTDecode filter"
	        ">>image\n", out);
#else
	  fputs("/DataSource currentfile/ASCIIHexDecode filter/DCTDecode filter"
	        ">>image\n", out);
#endif // HTMLDOC_ASCII85

	  jpg_setup(out, img, &cinfo);

	  for (i = img->height, pixel = img->pixels;
	       i > 0;
	       i --, pixel += img->width * img->depth)
	    jpeg_write_scanlines(&cinfo, &pixel, 1);

	  jpeg_finish_compress(&cinfo);
	  jpeg_destroy_compress(&cinfo);

#ifdef HTMLDOC_ASCII85
          ps_ascii85(out, (uchar *)"", 0, 1);
#else
	  // End of data marker...
	  fputs(">\n", out);
#endif // HTMLDOC_ASCII85
        }
        else
        {
	  if (img->depth == 1)
	    fputs("/DeviceGray setcolorspace\n", out);
	  else
	    fputs("/DeviceRGB setcolorspace\n", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent 8"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[%s]",
	          img->width, img->height,
        	  img->width, -img->height, img->height,
        	  img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_INTERPOLATION
	  fputs("/Interpolate true", out);
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
          fputs("/DataSource currentfile/ASCII85Decode filter"
	        ">>image\n", out);

	  ps_ascii85(out, img->pixels, img->width * img->height *
	                               img->depth, 1);
#else
          fputs("/DataSource currentfile/ASCIIHexDecode filter"
	        ">>image\n", out);

          ps_hex(out, img->pixels, img->width * img->depth * img->height);
	  // End of data marker...
	  fputs(">\n", out);
#endif // HTMLDOC_ASCII85
        }

	fputs("GR\n", out);
        break;
  }

  if (ncolors > 0)
    free(indices);

  image_unload(img);
}


/*
 * 'write_imagemask()' - Write an imagemask to the output file...
 */

static void
write_imagemask(FILE     *out,		/* I - Output file */
                render_t *r)		/* I - Image to write */
{
  image_t	*img;			/* Current image */
  int		x, y;			/* Position in mask image */
  int		startx, count;		/* Start and count */
  uchar		*ptr,			/* Pointer into mask image */
		byte,			/* Current byte */
		bit;			/* Current bit */
  float		scalex, scaley;		/* 1/(w-1) and 1/(h-1) scaling factors */
  int		width, height;		/* Scaled width and height */


  img    = r->data.image;
  width  = img->width * img->maskscale;
  height = img->height * img->maskscale;
  scalex = 1.0f / width;
  scaley = 1.0f / height;

  switch (PSLevel)
  {
    case 0 : // PDF
        break;

    default : // PostScript
        fputs("\nnewpath\n", out);
        break;
  }

  for (y = 0; y < height; y ++)
  {
    for (x = 0, ptr = img->mask + (height - y - 1) * img->maskwidth,
             bit = 128, byte = *ptr++, startx = 0, count = 0;
         x < width;
	 x ++)
    {
      if (!(bit & byte))
      {
        if (!count)
	  startx = x;

        count ++;
      }
      else if (count)
      {
	switch (PSLevel)
	{
	  case 0 : // PDF
	      flate_printf(out, "%.6f %.6f %.6f %.6f re\n",
			   (float)startx * scalex,
			   (float)y * scaley,
			   (float)count * scalex,
			   1.0f * scaley);
              break;

	  default : // PostScript
	      fprintf(out, "%.6f %.6f %.6f %.6f re\n",
		      (float)startx * scalex,
		      (float)y * scaley,
		      (float)count * scalex,
		      1.0f * scaley);
              break;
	}

	count = 0;
      }

      if (bit > 1)
        bit >>= 1;
      else
      {
        bit  = 128;
	byte = *ptr++;
      }
    }

    if (count)
    {
      switch (PSLevel)
      {
	case 0 : // PDF
	    flate_printf(out, "%.6f %.6f %.6f %.6f re\n",
			 (float)startx * scalex,
			 (float)y * scaley,
			 (float)count * scalex,
			 1.0f * scaley);
            break;

	default : // PostScript
	    fprintf(out, "%.6f %.6f %.6f %.6f re\n",
		    (float)startx * scalex,
		    (float)y * scaley,
		    (float)count * scalex,
		    1.0f * scaley);
            break;
      }
    }
  }

  switch (PSLevel)
  {
    case 0 : // PDF
        flate_puts("W n\n", out);
        break;

    default : // PostScript
        fputs("clip\n", out);
        break;
  }
}


/*
 * 'write_prolog()' - Write the file prolog...
 */

static void
write_prolog(FILE  *out,		/* I - Output file */
             int   page_count,		/* I - Number of pages (0 if not known) */
             uchar *author,		/* I - Author of document */
             uchar *creator,		/* I - Application that generated the HTML file */
             uchar *copyright,		/* I - Copyright (if any) on the document */
             uchar *keywords,		/* I - Search keywords */
	     uchar *subject)		/* I - Subject */
{
  FILE		*prolog;		/* PostScript prolog file */
  int		i, j,			/* Looping vars */
		encoding_object;	/* Font encoding object */
  int		page;			/* Current page */
  render_t	*r;			/* Current render data */
  int		fonts_used[TYPE_MAX][STYLE_MAX];
					/* Whether or not a font is used */
  int		font_desc[TYPE_MAX][STYLE_MAX];
					/* Font descriptor objects */
  char		temp[1024];		/* Temporary string */
  md5_state_t	md5;			/* MD5 state */
  md5_byte_t	digest[16];		/* MD5 digest value */
  rc4_context_t	rc4;			/* RC4 context */
  uchar		owner_pad[32],		/* Padded owner password */
		owner_key[32],		/* Owner key */
		user_pad[32],		/* Padded user password */
		user_key[32];		/* User key */
  uchar		perm_bytes[4];		/* Permission bytes */
  unsigned	perm_value;		/* Permission value, unsigned */
  static unsigned char pad[32] =
		{			/* Padding for passwords */
		  0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41,
		  0x64, 0x00, 0x4e, 0x56, 0xff, 0xfa, 0x01, 0x08,
		  0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68, 0x3e, 0x80,
		  0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a
		};


 /*
  * See what fonts are used...
  */

  memset(fonts_used, 0, sizeof(fonts_used));
  fonts_used[HeadFootType][HeadFootStyle] = 1;

  for (page = 0; page < num_pages; page ++)
    for (r = pages[page].start; r != NULL; r = r->next)
      if (r->type == RENDER_TEXT)
	fonts_used[r->data.text.typeface][r->data.text.style] = 1;

#ifdef DEBUG
  puts("The following fonts were used:");
  for (i = 0; i < TYPE_MAX; i ++)
    for (j = 0; j < STYLE_MAX; j ++)
      if (fonts_used[i][j])
        printf("    %s\n", _htmlFonts[i][j]);
#endif // DEBUG

 /*
  * Generate the heading...
  */

  if (PSLevel > 0)
  {
   /*
    * Write PostScript prolog stuff...
    */

    if (XRXComments)
    {
      int start, end;	// Start and end of document pages...
      int count;	// Number of exception pages in this range...


      // The following comments are Xerox job ticket information that
      // is used on the high-end Laser Printing Systems rather than
      // embedded commands...
      fputs("%XRXbegin: 001.0300\n", out);
      fputs("%XRXPDLformat: PS-Adobe\n", out);
      if (doc_title)
	fprintf(out, "%%XRXtitle: %s\n", doc_title);

      if (OutputFiles)
      {
        // Output a single chapter...
	if (chapter < 0)
	{
	  start = 0;
	  end   = chapter_outstarts[1] - 1;
	}
	else
	{
	  start = chapter_outstarts[chapter];
	  end   = chapter_outends[chapter];
	}
      }
      else
      {
        start = 0;
	end   = 0;
      }

      if (pages[outpages[start].pages[0]].duplex)
      {
	if (pages[outpages[start].pages[0]].landscape)
	  fputs("%XRXrequirements: duplex(tumble)\n", out);
	else
	  fputs("%XRXrequirements: duplex\n", out);
      }
      else
	fputs("%XRXrequirements: simplex\n", out);

      fputs("%XRXdisposition: PRINT\n", out);
      fputs("%XRXsignature: False\n", out);
      fprintf(out, "%%XRXpaperType-size: %.0f %.0f\n",
              pages[outpages[start].pages[0]].width * 25.4f / 72.0f,
              pages[outpages[start].pages[0]].length * 25.4f / 72.0f);
      if (pages[outpages[start].pages[0]].media_type[0])
	fprintf(out, "%%XRXpaperType-preFinish: %s 0 0\n",
        	pages[start].media_type);
      if (pages[outpages[start].pages[0]].media_color[0])
	fprintf(out, "%%XRXdocumentPaperColors: %c%s\n",
        	tolower(pages[start].media_color[0]),
		pages[start].media_color + 1);

      if (OutputFiles)
      {
        // Handle document settings per-chapter...
	for (i = start + 1; i < end; i += count)
	{
	  if (pages[outpages[i].pages[0]].width != pages[0].width ||
	      pages[outpages[i].pages[0]].length != pages[0].length ||
	      strcmp(pages[outpages[i].pages[0]].media_type,
	             pages[0].media_type) != 0 ||
	      strcmp(pages[outpages[i].pages[0]].media_color,
	             pages[0].media_color) != 0 ||
	      pages[outpages[i].pages[0]].duplex != pages[0].duplex)
	  {
	    for (count = 1; (i + count) <= end; count ++)
	      if (pages[outpages[i].pages[0]].width !=
	              pages[outpages[i + count].pages[0]].width ||
		  pages[outpages[i].pages[0]].length !=
		      pages[outpages[i + count].pages[0]].length ||
		  strcmp(pages[outpages[i].pages[0]].media_type,
		         pages[outpages[i + count].pages[0]].media_type) != 0 ||
		  strcmp(pages[outpages[i].pages[0]].media_color,
		         pages[outpages[i + count].pages[0]].media_color) != 0 ||
		  pages[outpages[i].pages[0]].duplex !=
		      pages[outpages[i + count].pages[0]].duplex)
		break;

	    fprintf(out, "%%XRXpageExceptions: %d %d %.0f %.0f %c%s opaque %s 0 0\n",
	            i + 1, i + count,
		    pages[outpages[i].pages[0]].width * 25.4f / 72.0f,
		    pages[outpages[i].pages[0]].length * 25.4f / 72.0f,
		    tolower(pages[outpages[i].pages[0]].media_color[0]),
		    pages[outpages[i].pages[0]].media_color + 1,
		    pages[outpages[i].pages[0]].media_type[0] ?
		        pages[outpages[i].pages[0]].media_type : "Plain");

	    if (pages[outpages[i].pages[0]].duplex &&
	        pages[outpages[i].pages[0]].landscape)
	      fprintf(out, "%%XRXpageExceptions-plex: %d %d duplex(tumble)\n",
	              i + 1, i + count);
	    else if (pages[outpages[i].pages[0]].duplex)
	      fprintf(out, "%%XRXpageExceptions-plex: %d %d duplex\n",
	              i + 1, i + count);
            else
	      fprintf(out, "%%XRXpageExceptions-plex: %d %d simplex\n",
	              i + 1, i + count);
	  }
	  else
	    count = 1;
        }
      }
      else
      {
        // All pages are in a single file...
        for (j = (TocLevels == 0); j <= TocDocCount; j ++)
	{
	  start = chapter_outstarts[j];
	  end   = chapter_outends[j];

	  for (i = start + 1; i < end; i += count)
	  {
	    if (pages[outpages[i].pages[0]].width != pages[0].width ||
		pages[outpages[i].pages[0]].length != pages[0].length ||
		strcmp(pages[outpages[i].pages[0]].media_type,
		       pages[0].media_type) != 0 ||
		strcmp(pages[outpages[i].pages[0]].media_color,
		       pages[0].media_color) != 0 ||
		pages[outpages[i].pages[0]].duplex != pages[0].duplex)
	    {
	      for (count = 1; (i + count) < end; count ++)
		if (pages[outpages[i].pages[0]].width !=
		        pages[outpages[i + count].pages[0]].width ||
		    pages[outpages[i].pages[0]].length !=
		        pages[outpages[i + count].pages[0]].length ||
		    strcmp(pages[outpages[i].pages[0]].media_type,
		           pages[outpages[i + count].pages[0]].media_type) != 0 ||
		    strcmp(pages[outpages[i].pages[0]].media_color,
		           pages[outpages[i + count].pages[0]].media_color) != 0 ||
		    pages[outpages[i].pages[0]].duplex !=
		        pages[outpages[i + count].pages[0]].duplex)
		  break;

	      fprintf(out, "%%XRXpageExceptions: %d %d %.0f %.0f %c%s opaque %s 0 0\n",
	              i + 1, i + count,
		      pages[outpages[i].pages[0]].width * 25.4f / 72.0f,
		      pages[outpages[i].pages[0]].length * 25.4f / 72.0f,
		      tolower(pages[outpages[i].pages[0]].media_color[0]),
		      pages[outpages[i].pages[0]].media_color + 1,
		      pages[outpages[i].pages[0]].media_type[0] ?
		          pages[outpages[i].pages[0]].media_type : "Plain");

	      if (pages[outpages[i].pages[0]].duplex && pages[outpages[i].pages[0]].landscape)
		fprintf(out, "%%XRXpageExceptions-plex: %d %d duplex(tumble)\n",
	        	i + 1, i + count);
	      else if (pages[outpages[i].pages[0]].duplex)
		fprintf(out, "%%XRXpageExceptions-plex: %d %d duplex\n",
	        	i + 1, i + count);
              else
		fprintf(out, "%%XRXpageExceptions-plex: %d %d simplex\n",
	        	i + 1, i + count);
	    }
	    else
	      count = 1;
          }
	}
      }

      fputs("%XRXend\n", out);
    }

    fputs("%!PS-Adobe-3.0\n", out);
    if (Landscape)
      fprintf(out, "%%%%BoundingBox: 0 0 %d %d\n", PageLength, PageWidth);
    else
      fprintf(out, "%%%%BoundingBox: 0 0 %d %d\n", PageWidth, PageLength);
    fprintf(out,"%%%%LanguageLevel: %d\n", PSLevel);
    fputs("%%Creator: htmldoc " SVERSION " Copyright 2011 by Michael R Sweet, "
          "All Rights Reserved.\n", out);
    fprintf(out, "%%%%CreationDate: D:%04d%02d%02d%02d%02d%02d%+03d%02d\n",
            doc_date->tm_year + 1900, doc_date->tm_mon + 1, doc_date->tm_mday,
            doc_date->tm_hour, doc_date->tm_min, doc_date->tm_sec,
	    (int)(-timezone / 3600),
	    (int)(((timezone < 0 ? -timezone : timezone) / 60) % 60));
    if (doc_title != NULL)
      fprintf(out, "%%%%Title: %s\n", doc_title);
    if (author != NULL)
      fprintf(out, "%%%%Author: %s\n", author);
    if (creator != NULL)
      fprintf(out, "%%%%Generator: %s\n", creator);
    if (copyright != NULL)
      fprintf(out, "%%%%Copyright: %s\n", copyright);
    if (keywords != NULL)
      fprintf(out, "%%%%Keywords: %s\n", keywords);
    if (subject != NULL)
      fprintf(out, "%%%%Subject: %s\n", keywords);
    if (page_count > 0)
      fprintf(out, "%%%%Pages: %d\n", page_count);
    else
      fputs("%%Pages: (atend)\n", out);

    if (!EmbedFonts)
    {
      fputs("%%DocumentNeededResources:\n", out);

      for (i = 0; i < TYPE_MAX; i ++)
        for (j = 0; j < STYLE_MAX; j ++)
          if (fonts_used[i][j] && _htmlStandardFonts[i])
            fprintf(out, "%%%%+ font %s\n", _htmlFonts[i][j]);
    }

    fputs("%%DocumentProvidedResources:\n", out);

    for (i = 0; i < TYPE_MAX; i ++)
      for (j = 0; j < STYLE_MAX; j ++)
        if (fonts_used[i][j] && (EmbedFonts || !_htmlStandardFonts[i]))
          fprintf(out, "%%%%+ font %s\n", _htmlFonts[i][j]);
    fputs("%%DocumentData: Clean7bit\n", out);
    fputs("%%EndComments\n", out);

    fputs("%%BeginProlog\n", out);

   /*
    * Embed fonts?
    */

    for (i = 0; i < TYPE_MAX; i ++)
    {
      if (EmbedFonts || !_htmlStandardFonts[i])
	for (j = 0; j < STYLE_MAX; j ++)
          if (fonts_used[i][j])
	    write_type1(out, (typeface_t)i, (style_t)j);
    }

   /*
    * Procedures used throughout the document...
    */

    fputs("%%BeginResource: procset htmldoc-page 1.8 25\n", out);
    fputs("/BD{bind def}bind def", out);
    fputs("/B{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto\n"
          "closepath stroke}BD", out);
    fputs("/C{setrgbcolor}BD\n", out);
    fputs("/CM{concat}BD", out);
    fputs("/DF{findfont dup length dict begin{1 index/FID ne{def}{pop pop}\n"
          "ifelse}forall/Encoding fontencoding def currentdict end definefont pop}BD\n", out);
    fputs("/F{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath fill}BD\n", out);
    fputs("/FS{/hdFontSize exch def}BD", out);
    fputs("/G{setgray}BD\n", out);
    fputs("/GS{gsave}BD", out);
    fputs("/GR{grestore}BD", out);
    fputs("/J{0 exch ashow}BD\n", out);
    fputs("/L{0 rlineto stroke}BD", out);
    fputs("/M{moveto}BD", out);
    fputs("/re{4 2 roll moveto 1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath}BD\n", out);
    fputs("/RO{rotate}BD", out);
    fputs("/S{show}BD", out);
    fputs("/SC{dup scale}BD\n", out);
    fputs("/SF{findfont hdFontSize scalefont setfont}BD", out);
    fputs("/SP{showpage}BD", out);
    fputs("/T{translate}BD\n", out);
    fputs("%%EndResource\n", out);

   /*
    * Output the font encoding for the current character set...  For now we
    * just support 8-bit fonts since true Unicode support needs a very large
    * number of extra fonts that aren't normally available on a PS printer.
    */

    fputs("/fontencoding[\n", out);
    for (i = 0, j = 0; i < 256; i ++)
    {
      if (_htmlGlyphs[i])
        j += strlen(_htmlGlyphs[i]) + 1;
      else
        j += 8;

      if (j > 80)
      {
	if (_htmlGlyphs[i])
          j = strlen(_htmlGlyphs[i]) + 1;
	else
          j = 8;

        putc('\n', out);
      }

      putc('/', out);
      if (_htmlGlyphs[i])
        fputs(_htmlGlyphs[i], out);
      else
        fputs(".notdef", out);
    }

    fputs("]def\n", out);

   /*
    * Fonts...
    */

    for (i = 0; i < TYPE_MAX; i ++)
      for (j = 0; j < STYLE_MAX; j ++)
        if (fonts_used[i][j])
        {
	  if (i < TYPE_SYMBOL)
	    fprintf(out, "/F%x/%s DF\n", i * 4 + j, _htmlFonts[i][j]);
	  else
	    fprintf(out, "/F%x/%s findfont definefont pop\n", i * 4 + j,
	            _htmlFonts[i][j]);
        }

    if (PSCommands)
    {
      snprintf(temp, sizeof(temp), "%s/data/prolog.ps", _htmlData);
      if ((prolog = fopen(temp, "rb")) != NULL)
      {
	while (fgets(temp, sizeof(temp), prolog) != NULL)
          fputs(temp, out);

	fclose(prolog);
      }
      else
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
                       "Unable to open data file \"%s\" - %s", temp,
                       strerror(errno));

	fputs("%%BeginResource: procset htmldoc-device 1.8 22\n", out);
	fputs("languagelevel 1 eq{/setpagedevice{pop}BD}if\n", out);
	fputs("/SetDuplexMode{<</Duplex 3 index/Tumble 5 index>>setpagedevice "
              "pop pop}BD\n", out);
	fputs("/SetMediaColor{pop}BD\n", out);
	fputs("/SetMediaType{pop}BD\n", out);
	fputs("/SetMediaPosition{pop}BD\n", out);
	fputs("/SetPageSize{2 array astore<</PageSize 2 index/ImageableArea "
              "null>>setpagedevice pop}BD\n", out);
	fputs("%%EndResource\n", out);
      }
    }

    if (background_image != NULL)
      ps_write_background(out);

    fputs("%%EndProlog\n", out);
  }
  else
  {
   /*
    * Write PDF prolog stuff...
    */

    fprintf(out, "%%PDF-%.1f\n", 0.1 * PDFVersion);
    fputs("%\342\343\317\323\n", out);
    num_objects = 0;

   /*
    * Compute the file ID...
    */

    md5_init(&md5);
    md5_append(&md5, (md5_byte_t *)OutputPath, sizeof(OutputPath));
    md5_append(&md5, (md5_byte_t *)&doc_time, sizeof(doc_time));
    md5_finish(&md5, file_id);

   /*
    * Setup encryption stuff as necessary...
    */

    if (Encryption)
    {
     /*
      * Copy and pad the user password...
      */

      strlcpy((char *)user_pad, UserPassword, sizeof(user_pad));

      if ((i = strlen(UserPassword)) < 32)
	memcpy(user_pad + i, pad, 32 - i);

      if (OwnerPassword[0])
      {
       /*
        * Copy and pad the owner password...
	*/

        strlcpy((char *)owner_pad, OwnerPassword, sizeof(owner_pad));

	if ((i = strlen(OwnerPassword)) < 32)
	  memcpy(owner_pad + i, pad, 32 - i);
      }
      else
      {
       /*
        * Generate a pseudo-random owner password...
	*/

	srand(time(NULL));

	for (i = 0; i < 32; i ++)
	  owner_pad[i] = rand();
      }

     /*
      * What is the key length?
      *
      * Acrobat 4.0 and earlier (PDF 1.3 and earlier) allow a maximum of
      * 40-bits.  Acrobat 5.0 and newer support 128-bits.
      */

      if (PDFVersion > 13)
        encrypt_len = 16;	// 128 bits
      else
        encrypt_len = 5;	// 40 bits

     /*
      * Compute the owner key...
      */

      md5_init(&md5);
      md5_append(&md5, owner_pad, 32);
      md5_finish(&md5, digest);

      if (encrypt_len > 5)
      {
        // MD5 the result 50 more times...
	for (i = 0; i < 50; i ++)
	{
          md5_init(&md5);
          md5_append(&md5, digest, 16);
          md5_finish(&md5, digest);
	}

        // Copy the padded user password...
        memcpy(owner_key, user_pad, 32);

        // Encrypt the result 20 times...
	for (i = 0; i < 20; i ++)
	{
	  // XOR each byte in the key with the loop counter...
	  for (j = 0; j < encrypt_len; j ++)
	    encrypt_key[j] = digest[j] ^ i;

          rc4_init(&rc4, encrypt_key, encrypt_len);
          rc4_encrypt(&rc4, owner_key, owner_key, 32);
	}
      }
      else
      {
        rc4_init(&rc4, digest, encrypt_len);
        rc4_encrypt(&rc4, user_pad, owner_key, 32);
      }

     /*
      * Figure out the permissions word; the new N-bit security
      * handler adds several new permission bits, which we must
      * simulate...
      */

      perm_value = (unsigned)Permissions;

      if (encrypt_len > 5)
      {
        // N-bit encryption...
	if (!(perm_value & PDF_PERM_COPY))
	  perm_value &= ~0x00240000;	// Mask additional copy perms...
      }

     /*
      * Compute the encryption key...
      */

      md5_init(&md5);
      md5_append(&md5, user_pad, 32);
      md5_append(&md5, owner_key, 32);

      perm_bytes[0] = perm_value;
      perm_bytes[1] = perm_value >> 8;
      perm_bytes[2] = perm_value >> 16;
      perm_bytes[3] = perm_value >> 24;

      md5_append(&md5, perm_bytes, 4);
      md5_append(&md5, file_id, 16);
      md5_finish(&md5, digest);

      if (encrypt_len > 5)
      {
        // MD5 the result 50 times..
        for (i = 0; i < 50; i ++)
	{
	  md5_init(&md5);
	  md5_append(&md5, digest, 16);
	  md5_finish(&md5, digest);
	}
      }

      memcpy(encrypt_key, digest, encrypt_len);

     /*
      * Compute the user key...
      */

      if (encrypt_len > 5)
      {
        md5_init(&md5);
        md5_append(&md5, pad, 32);
        md5_append(&md5, file_id, 16);
        md5_finish(&md5, user_key);

        memset(user_key + 16, 0, 16);

        // Encrypt the result 20 times...
        for (i = 0; i < 20; i ++)
	{
	  // XOR each byte in the key with the loop counter...
	  for (j = 0; j < encrypt_len; j ++)
	    digest[j] = encrypt_key[j] ^ i;

          rc4_init(&rc4, digest, encrypt_len);
          rc4_encrypt(&rc4, user_key, user_key, 16);
	}
      }
      else
      {
        rc4_init(&rc4, encrypt_key, encrypt_len);
        rc4_encrypt(&rc4, pad, user_key, 32);
      }

     /*
      * Write the encryption dictionary...
      */

      encrypt_object = pdf_start_object(out);

      fputs("/Filter/Standard/O<", out);
      for (i = 0; i < 32; i ++)
        fprintf(out, "%02x", owner_key[i]);
      fputs(">/U<", out);
      for (i = 0; i < 32; i ++)
        fprintf(out, "%02x", user_key[i]);
      fputs(">", out);

      if (encrypt_len > 5)
      {
        // N-bit encryption...
        fprintf(out, "/P %d/V 2/R 3/Length %d", (int)perm_value, encrypt_len * 8);
      }
      else
        fprintf(out, "/P %d/V 1/R 2", (int)perm_value);

      pdf_end_object(out);
    }
    else
      encrypt_object = 0;

   /*
    * Write info object...
    */

    info_object = pdf_start_object(out);

    fputs("/Producer", out);
    write_string(out, (uchar *)"htmldoc " SVERSION " Copyright 1997-2006 Easy "
                               "Software Products, All Rights Reserved.", 0);
    fputs("/CreationDate", out);
    sprintf(temp, "D:%04d%02d%02d%02d%02d%02d%+03d%02d",
            doc_date->tm_year + 1900, doc_date->tm_mon + 1, doc_date->tm_mday,
            doc_date->tm_hour, doc_date->tm_min, doc_date->tm_sec,
	    (int)(-timezone / 3600),
	    (int)(((timezone < 0 ? -timezone : timezone) / 60) % 60));
    write_string(out, (uchar *)temp, 0);

    if (doc_title != NULL)
    {
      fputs("/Title", out);
      write_utf16(out, doc_title);
    }

    if (author != NULL || copyright != NULL)
    {
      if (author && copyright)
        snprintf(temp, sizeof(temp), "%s, %s", author, copyright);
      else if (author)
        strlcpy(temp, (const char *)author, sizeof(temp));
      else
        strlcpy(temp, (const char *)copyright, sizeof(temp));

      fputs("/Author", out);
      write_utf16(out, (uchar *)temp);
    }

    if (creator != NULL)
    {
      fputs("/Creator", out);
      write_utf16(out, creator);
    }

    if (keywords != NULL)
    {
      fputs("/Keywords", out);
      write_utf16(out, keywords);
    }

    if (subject != NULL)
    {
      fputs("/Subject", out);
      write_utf16(out, subject);
    }

    pdf_end_object(out);

   /*
    * Write the font encoding for the selected character set.  Note that
    * we *should* be able to use the WinAnsiEncoding value for ISO-8859-1
    * to make smaller files, however Acrobat Exchange does not like it
    * despite the fact that it is defined in the PDF specification...
    */

    encoding_object = pdf_start_object(out);

    fputs("/Type/Encoding", out);
    fputs("/Differences[", out);
    for (i = 0, j = -1; i < 256; i ++)
      if (_htmlGlyphs[i])
      {
       /*
        * Output a character index if we had blank ones...
	*/

        if (j != (i - 1))
	  fprintf(out, " %d", i);

        fprintf(out, "/%s", _htmlGlyphs[i]);
	j = i;
      }

    fputs("]", out);
    pdf_end_object(out);

    memset(font_desc, 0, sizeof(font_desc));

   /*
    * Build font descriptors for the EmbedFonts fonts...
    */

    for (i = 0; i < TYPE_MAX; i ++)
      if (EmbedFonts || !_htmlStandardFonts[i])
	for (j = 0; j < STYLE_MAX; j ++)
          if (fonts_used[i][j])
	    font_desc[i][j] = write_type1(out, (typeface_t )i, (style_t)j);

    for (i = 0; i < TYPE_MAX; i ++)
      for (j = 0; j < STYLE_MAX; j ++)
        if (fonts_used[i][j])
        {
	  font_objects[i * STYLE_MAX + j] = pdf_start_object(out);

	  fputs("/Type/Font", out);
	  fputs("/Subtype/Type1", out);
	  fprintf(out, "/BaseFont/%s", _htmlFonts[i][j]);

          if (font_desc[i][j])
	  {
	    // Embed Type1 font...
	    fputs("/FirstChar 0", out);
	    fputs("/LastChar 255", out);
	    fprintf(out, "/Widths %d 0 R", font_desc[i][j] + 1);
	    fprintf(out, "/FontDescriptor %d 0 R", font_desc[i][j]);
	  }

	  if (i < TYPE_SYMBOL) /* Use native encoding for symbols */
	    fprintf(out, "/Encoding %d 0 R", encoding_object);

          pdf_end_object(out);
        }
  }
}


/*
 * 'write_string()' - Write a text entity.
 */

static void
write_string(FILE  *out,		/* I - Output file */
             uchar *s,			/* I - String */
	     int   compress)		/* I - Compress output? */
{
  int	i;				/* Looping var */


  if (Encryption && !compress && PSLevel == 0)
  {
    int		len,			// Length of string
		bytes;			// Current bytes encrypted
    uchar	news[1024];		// New string


   /*
    * Write an encrypted string...
    */

    putc('<', out);
    encrypt_init();

    for (len = strlen((char *)s); len > 0; len -= bytes, s += bytes)
    {
      if (len > (int)sizeof(news))
        bytes = (int)sizeof(news);
      else
        bytes = len;

      rc4_encrypt(&encrypt_state, s, news, bytes);

      for (i = 0; i < bytes; i ++)
        fprintf(out, "%02x", news[i]);
    }

    putc('>', out);
  }
  else
  {
    if (compress)
      flate_write(out, (uchar *)"(", 1);
    else
      putc('(', out);

    while (*s != '\0')
    {
      if (*s == 160) /* &nbsp; */
      {
	if (compress)
          flate_write(out, (uchar *)" ", 1);
	else
          putc(' ', out);
      }
      else if (*s < 32 || *s > 126)
      {
	if (compress)
          flate_printf(out, "\\%o", *s);
	else
          fprintf(out, "\\%o", *s);
      }
      else if (compress)
      {
	if (*s == '(' || *s == ')' || *s == '\\')
          flate_write(out, (uchar *)"\\", 1);

	flate_write(out, s, 1);
      }
      else
      {
	if (*s == '(' || *s == ')' || *s == '\\')
          putc('\\', out);

	putc(*s, out);
      }

      s ++;
    }

    if (compress)
      flate_write(out, (uchar *)")", 1);
    else
      putc(')', out);
  }
}


/*
 * 'write_text()' - Write a text entity.
 */

static void
write_text(FILE     *out,	/* I - Output file */
           render_t *r)		/* I - Text entity */
{
  uchar	*ptr;			/* Pointer into text */


  // Quick optimization - don't output spaces...
  for (ptr = r->data.text.buffer; *ptr; ptr ++)
    if (!isspace(*ptr) && *ptr != 0xa0)
      break;

  if (!*ptr)
    return;

  // Not just whitespace - send it out...
  set_color(out, r->data.text.rgb);
  set_font(out, r->data.text.typeface, r->data.text.style, r->data.text.size);
  set_pos(out, r->x, r->y);

  if (PSLevel > 0)
  {
    if (r->data.text.spacing > 0.0f)
      fprintf(out, " %.3f", r->data.text.spacing);
  }
  else if (r->data.text.spacing != render_spacing)
    flate_printf(out, " %.3f Tc", render_spacing = r->data.text.spacing);

  write_string(out, r->data.text.buffer, PSLevel == 0);

  if (PSLevel > 0)
  {
    if (r->data.text.spacing > 0.0f)
      fputs("J\n", out);
    else
      fputs("S\n", out);
  }
  else
    flate_puts("Tj\n", out);

  render_x += r->width;
}


/*
 * 'write_trailer()' - Write the file trailer.
 */

static void
write_trailer(FILE *out,		/* I - Output file */
              int  num_file_pages)	/* I - Number of pages in file */
{
  int		i, j, k,		/* Looping vars */
		type,			/* Type of number */
		offset,			/* Offset to xref table in PDF file */
		start;			/* Start page number */
  page_t	*page;			/* Start page of chapter */
  char		prefix[64],		/* Prefix string */
		*prefptr;		/* Pointer into prefix string */
  static const char *modes[] =		/* Page modes */
		{
		  "UseNone",
		  "UseOutlines",
		  "FullScreen"
		};
  static const char *layouts[] =	/* Page layouts */
		{
		  "SinglePage",
		  "OneColumn",
		  "TwoColumnLeft",
		  "TwoColumnRight"
		};


  if (PSLevel > 0)
  {
   /*
    * PostScript...
    */

    fputs("%%Trailer\n", out);
    if (num_file_pages > 0)
      fprintf(out, "%%%%Pages: %d\n", num_file_pages);

    fputs("%%EOF\n", out);
  }
  else
  {
   /*
    * PDF...
    */

    root_object = pdf_start_object(out);

    fputs("/Type/Catalog", out);
    fprintf(out, "/Pages %d 0 R", pages_object);

    if (PDFVersion >= 12)
    {
      if (names_object)
        fprintf(out, "/Names %d 0 R", names_object);

      fprintf(out, "/PageLayout/%s", layouts[PDFPageLayout]);
    }

    if (outline_object > 0)
      fprintf(out, "/Outlines %d 0 R", outline_object);

    switch (PDFFirstPage)
    {
      case PDF_PAGE_1 :
          if (TitlePage)
	  {
            fprintf(out, "/OpenAction[%d 0 R/XYZ null null 0]",
                    pages_object + 1);
            break;
	  }
          break;
      case PDF_TOC :
          if (TocLevels > 0)
	  {
            fprintf(out, "/OpenAction[%d 0 R/XYZ null null 0]",
                    pages_object + 2 * chapter_outstarts[0] + 1);
	    break;
	  }
          break;
      case PDF_CHAPTER_1 :
          fprintf(out, "/OpenAction[%d 0 R/XYZ null null 0]",
                  pages_object + 2 * chapter_outstarts[1] + 1);
          break;
    }

    fprintf(out, "/PageMode/%s", modes[PDFPageMode]);

    if (PDFVersion > 12 && NumberUp == 1)
    {
      // Output the PageLabels tree...
      fputs("/PageLabels<</Nums[", out);

      i = 0;

      if (TitlePage)
      {
        fputs("0<</P", out);
	write_string(out, (uchar *)"title", 0);
	fputs(">>", out);
	if (PageDuplex)
	{
	  fputs("1<</P", out);
	  write_string(out, (uchar *)"eltit", 0);
	  fputs(">>", out);
	}
	i += PageDuplex + 1;
      }

      if (TocLevels > 0 && OutputType == OUTPUT_BOOK)
      {
        type = 'r';

        for (j = 0; j < 3; j ++)
	  if ((TocHeader[j] && strstr(TocHeader[j], "$PAGE(1)")) ||
	      (TocFooter[j] && strstr(TocFooter[j], "$PAGE(1)")))
	    type = 'D';
	  else if ((TocHeader[j] && strstr(TocHeader[j], "$PAGE(I)")) ||
	           (TocFooter[j] && strstr(TocFooter[j], "$PAGE(I)")))
	    type = 'R';
	  else if ((TocHeader[j] && strstr(TocHeader[j], "$PAGE(a)")) ||
	           (TocFooter[j] && strstr(TocFooter[j], "$PAGE(a)")))
	    type = 'a';
	  else if ((TocHeader[j] && strstr(TocHeader[j], "$PAGE(A)")) ||
	           (TocFooter[j] && strstr(TocFooter[j], "$PAGE(A)")))
	    type = 'A';

        fprintf(out, "%d<</S/%c>>", i, type);

        i += chapter_ends[0] - chapter_starts[0] + 1;
      }

      for (j = 1; j <= TocDocCount; j ++)
      {
        page  = pages + chapter_starts[j];
	start = chapter_starts[j] - chapter_starts[1] + 1;
	type  = 'D';

        prefix[0] = '\0';

	for (k = 0; k < 3; k ++)
	{
	  if (page->header[k] && strstr((char *)page->header[k], "PAGE"))
	    strlcpy(prefix, (char *)page->header[k], sizeof(prefix));
	  else if (page->footer[k] && strstr((char *)page->footer[k], "PAGE"))
	    strlcpy(prefix, (char *)page->footer[k], sizeof(prefix));

	  if ((page->header[k] && strstr((char *)page->header[k], "PAGE(i)")) ||
	      (page->footer[k] && strstr((char *)page->footer[k], "PAGE(i)")))
	    type = 'r';
	  else if ((page->header[k] && strstr((char *)page->header[k], "PAGE(I)")) ||
	           (page->footer[k] && strstr((char *)page->footer[k], "PAGE(I)")))
	    type = 'R';
	  else if ((page->header[k] && strstr((char *)page->header[k], "PAGE(a)")) ||
	           (page->footer[k] && strstr((char *)page->footer[k], "PAGE(a)")))
	    type = 'a';
	  else if ((page->header[k] && strstr((char *)page->header[k], "PAGE(A)")) ||
	           (page->footer[k] && strstr((char *)page->footer[k], "PAGE(A)")))
	    type = 'A';

	  if ((page->header[k] && strstr((char *)page->header[k], "$CHAPTERPAGE")) ||
	      (page->footer[k] && strstr((char *)page->footer[k], "$CHAPTERPAGE")))
	    start = 1;
        }

        if ((prefptr = strstr(prefix, "$PAGE")) == NULL)
	  prefptr = strstr(prefix, "$CHAPTERPAGE");
	fprintf(out, "%d<</S/%c/St %d", i, type, start);
	if (prefptr)
	{
	  *prefptr = '\0';
	  fputs("/P", out);
	  write_string(out, (uchar *)prefix, 0);
	}
	fputs(">>", out);

        i += chapter_ends[j] - chapter_starts[j] + 1;
      }

      fputs("]>>", out);
    }

    pdf_end_object(out);

    offset = ftell(out);

    fputs("xref\n", out);
    fprintf(out, "0 %d \n", num_objects + 1);
    fputs("0000000000 65535 f \n", out);
    for (i = 1; i <= num_objects; i ++)
      fprintf(out, "%010d 00000 n \n", objects[i]);

    fputs("trailer\n", out);
    fputs("<<", out);
    fprintf(out, "/Size %d", num_objects + 1);
    fprintf(out, "/Root %d 0 R", root_object);
    fprintf(out, "/Info %d 0 R", info_object);
    fputs("/ID[<", out);
    for (i = 0; i < 16; i ++)
      fprintf(out, "%02x", file_id[i]);
    fputs("><", out);
    for (i = 0; i < 16; i ++)
      fprintf(out, "%02x", file_id[i]);
    fputs(">]", out);

    if (Encryption)
      fprintf(out, "/Encrypt %d 0 R", encrypt_object);

    fputs(">>\n", out);
    fputs("startxref\n", out);
    fprintf(out, "%d\n", offset);
    fputs("%%EOF\n", out);
  }
}


/*
 * 'write_type1()' - Write an embedded Type 1 font.
 */

static int				/* O - Object number */
write_type1(FILE       *out,		/* I - File to write to */
            typeface_t typeface,	/* I - Typeface */
	    style_t    style)		/* I - Style */
{
  char		filename[1024];		/* PFA filename */
  FILE		*fp;			/* PFA file */
  int		ch;			/* Character value */
  int		width;			/* Width value */
  char		glyph[64],		/* Glyph name */
		line[1024],		/* Line from AFM file */
		*lineptr,		/* Pointer into line */
		*dataptr;		/* Pointer for data */
  int		ascent,			/* Ascent above baseline */
		cap_height,		/* Ascent of CAPITALS */
		x_height,		/* Ascent of lowercase */
		descent,		/* Decent below baseline */
		bbox[4],		/* Bounding box */
		italic_angle;		/* Angle for italics */
  int		widths[256];		/* Character widths */
  int		length1,		/* Length1 value for font */
		length2,		/* Length2 value for font */
		length3;		/* Length3 value for font */
  static int	tflags[] =		/* PDF typeface flags */
		{
		  33,			/* Courier */
		  34,			/* Times-Roman */
		  32,			/* Helvetica */
		  33,			/* Monospace */
		  34,			/* Serif */
		  32,			/* Sans */
		  4,			/* Symbol */
		  4			/* Dingbats */
		};
  static int	sflags[] =		/* PDF style flags */
		{
		  0,			/* Normal */
		  0,			/* Bold */
		  64,			/* Italic */
		  64			/* Bold-Italic */
		};


 /*
  * This function writes a Type1 font, either as an object for PDF
  * output or as an in-line font in PostScript output.  This is useful
  * because the Type1 fonts that Adobe ships typically do not include
  * the full set of characters required by some of the ISO character
  * sets.
  */

 /*
  * Try to open the PFA file for the Type1 font...
  */

  snprintf(filename, sizeof(filename), "%s/fonts/%s.pfa", _htmlData,
           _htmlFonts[typeface][style]);
  if ((fp = fopen(filename, "r")) == NULL)
  {
#ifndef DEBUG
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open font file %s!", filename);
#endif /* !DEBUG */
    return (0);
  }

 /*
  * Write the font (object)...
  */

  if (PSLevel)
  {
   /*
    * Embed a Type1 font in the PostScript output...
    */

    fprintf(out, "%%%%BeginResource: font %s\n", _htmlFonts[typeface][style]);

    line[0] = '\0';

    while (fgets(line, sizeof(line), fp) != NULL)
      fputs(line, out);

    if (line[strlen(line) - 1] != '\n')
      fputs("\n", out);

    fputs("%%EndResource\n", out);

    fclose(fp);
  }
  else
  {
   /*
    * Embed a Type1 font object in the PDF output...
    */

    length1 = 0;
    length2 = 0;
    length3 = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
      length1 += strlen(line);
      if (strstr(line, "currentfile eexec") != NULL)
        break;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
      if (!strcmp(line, "00000000000000000000000000000000"
                        "00000000000000000000000000000000\n"))
        break;

      length2 += (strlen(line) - 1) / 2;
    }

    length3 = strlen(line);
    while (fgets(line, sizeof(line), fp) != NULL)
      length3 += strlen(line);

    rewind(fp);

    pdf_start_object(out);
    fprintf(out, "/Length1 %d", length1);
    fprintf(out, "/Length2 %d", length2);
    fprintf(out, "/Length3 %d", length3);
    if (Compression)
      fputs("/Filter/FlateDecode", out);
    pdf_start_stream(out);
    flate_open_stream(out);

    while (fgets(line, sizeof(line), fp) != NULL)
    {
      flate_puts(line, out);

      if (strstr(line, "currentfile eexec") != NULL)
        break;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
      if (!strcmp(line, "00000000000000000000000000000000"
                        "00000000000000000000000000000000\n"))
        break;

      for (lineptr = line, dataptr = line; isxdigit(*lineptr); lineptr += 2)
      {
        if (isdigit(lineptr[0]))
	  ch = (lineptr[0] - '0') << 4;
	else
	  ch = (tolower(lineptr[0] & 255) - 'a' + 10) << 4;

        if (isdigit(lineptr[1]))
	  ch |= lineptr[1] - '0';
	else
	  ch |= tolower(lineptr[1] & 255) - 'a' + 10;

        *dataptr++ = ch;
      }

      flate_write(out, (uchar *)line, dataptr - line);
    }

    flate_puts(line, out);
    while (fgets(line, sizeof(line), fp) != NULL)
      flate_puts(line, out);

    flate_close_stream(out);

    pdf_end_object(out);

    fclose(fp);

   /*
    * Try to open the AFM file for the Type1 font...
    */

    snprintf(filename, sizeof(filename), "%s/fonts/%s.afm", _htmlData,
             _htmlFonts[typeface][style]);
    if ((fp = fopen(filename, "r")) == NULL)
    {
#ifndef DEBUG
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open font width file %s!", filename);
#endif /* !DEBUG */
      return (0);
    }

   /*
    * Set the default values (Courier)...
    */

    for (ch = 0; ch < 256; ch ++)
      widths[ch] = 600;

    ascent       = 629;
    cap_height   = 562;
    x_height     = 426;
    descent      = -157;
    bbox[0]      = -28;
    bbox[1]      = -250;
    bbox[2]      = 628;
    bbox[3]      = 805;
    italic_angle = 0;

   /*
    * Read the AFM file...
    */

    while (fgets(line, sizeof(line), fp) != NULL)
    {
      if (strncmp(line, "ItalicAngle ", 12) == 0)
	italic_angle = atoi(line + 12);
      else if (strncmp(line, "FontBBox ", 9) == 0)
	sscanf(line + 9, "%d%d%d%d", bbox + 0, bbox + 1, bbox + 2, bbox + 3);
      else if (strncmp(line, "CapHeight ", 10) == 0)
	cap_height = atoi(line + 10);
      else if (strncmp(line, "XHeight ", 8) == 0)
	x_height = atoi(line + 8);
      else if (strncmp(line, "Ascender ", 9) == 0)
	ascent = atoi(line + 9);
      else if (strncmp(line, "Descender ", 10) == 0)
	descent = atoi(line + 10);
      else if (strncmp(line, "C ", 2) == 0)
      {
	if (typeface < TYPE_SYMBOL)
	{
	 /*
	  * Handle encoding of Courier, Times, and Helvetica using
	  * assigned charset...
	  */

	  if (sscanf(line, "%*s%*s%*s%*s%d%*s%*s%63s", &width, glyph) != 2)
	    continue;

	  for (ch = 0; ch < 256; ch ++)
	    if (_htmlGlyphs[ch] && strcmp(_htmlGlyphs[ch], glyph) == 0)
	      break;

	  if (ch < 256)
	    widths[ch] = width;
	}
	else
	{
	 /*
	  * Symbol font uses its own encoding...
	  */

	  if (sscanf(line, "%*s%d%*s%*s%d", &ch, &width) != 2)
	    continue;

	  if (ch >= 0 && ch < 256)
	    widths[ch] = width;
	}
      }
    }

    fclose(fp);

   /*
    * Write the font descriptor...
    */

    pdf_start_object(out);
    fputs("/Type/FontDescriptor", out);
    fprintf(out, "/Ascent %d", ascent);
    fprintf(out, "/Descent %d", descent);
    fprintf(out, "/CapHeight %d", cap_height);
    fprintf(out, "/XHeight %d", x_height);
    fprintf(out, "/FontBBox[%d %d %d %d]", bbox[0], bbox[1], bbox[2], bbox[3]);
    fprintf(out, "/ItalicAngle %d", italic_angle);
    fprintf(out, "/StemV %d", widths['v']);
    fprintf(out, "/Flags %d", tflags[typeface] | sflags[style]);
    fprintf(out, "/FontName/%s", _htmlFonts[typeface][style]);
    fprintf(out, "/FontFile %d 0 R", num_objects - 1);
    pdf_end_object(out);

   /*
    * Write the character widths...
    */

    pdf_start_object(out, 1);
    fprintf(out, "%d", widths[0]);
    for (ch = 1; ch < 256; ch ++)
      fprintf(out, " %d", widths[ch]);
    pdf_end_object(out);
  }

 /*
  * Return the font descriptor...
  */

  return (num_objects - 1);
}


/*
 * 'write_utf16()' - Write a UTF-16 string...
 */

static void
write_utf16(FILE  *out,			// I - File to write to
            uchar *s)			// I - String to write
{
  uchar *sptr;				// Pointer into string


 /*
  * We start by checking to see if the string is composed only of
  * ASCII characters; if so, we can just write a normal string...
  */

  for (sptr = s; *sptr && !(*sptr & 0x80); sptr ++);
  if (!*sptr)
  {
   /*
    * Write an ASCII string...
    */

    write_string(out, s, 0);
  }
  else if (Encryption)
  {
   /*
    * Convert the string to Unicode and encrypt...
    */

    int		ch;			// Character value
    uchar	unicode[2],		// Unicode character
		enicode[2];		// Encrypted unicode character


    putc('<', out);
    encrypt_init();

    unicode[0] = 0xfe;			// Start with BOM
    unicode[1] = 0xff;

    rc4_encrypt(&encrypt_state, unicode, enicode, 2);

    fprintf(out, "%02x%02x", enicode[0], enicode[1]);

    for (sptr = s; *sptr; sptr ++)
    {
      ch         = _htmlUnicode[*sptr];
      unicode[0] = ch >> 8;
      unicode[1] = ch;

      rc4_encrypt(&encrypt_state, unicode, enicode, 2);

      fprintf(out, "%02x%02x", enicode[0], enicode[1]);
    }

    putc('>', out);
  }
  else
  {
   /*
    * Convert the string to Unicode...
    */

    fputs("<feff", out);		// Start with BOM
    for (sptr = s; *sptr; sptr ++)
      fprintf(out, "%04x", _htmlUnicode[*sptr]);
    putc('>', out);
  }
}


/*
 * 'encrypt_init()' - Initialize the RC4 encryption context for the current
 *                    object.
 */

static void
encrypt_init(void)
{
  int		i;			/* Looping var */
  uchar		data[21],		/* Key data */
		*dataptr;		/* Pointer to key data */
  md5_state_t	md5;			/* MD5 state */
  md5_byte_t	digest[16];		/* MD5 digest value */


 /*
  * Compute the key data for the MD5 hash.
  */

  for (i = 0, dataptr = data; i < encrypt_len; i ++)
    *dataptr++ = encrypt_key[i];

  *dataptr++ = num_objects;
  *dataptr++ = num_objects >> 8;
  *dataptr++ = num_objects >> 16;
  *dataptr++ = 0;
  *dataptr++ = 0;

 /*
  * Hash it...
  */

  md5_init(&md5);
  md5_append(&md5, data, encrypt_len + 5);
  md5_finish(&md5, digest);

 /*
  * Initialize the RC4 context using the first N+5 bytes of the digest...
  */

  if (encrypt_len > 11)
    rc4_init(&encrypt_state, digest, 16);
  else
    rc4_init(&encrypt_state, digest, encrypt_len + 5);
}


/*
 * 'flate_open_stream()' - Open a deflated output stream.
 */

static void
flate_open_stream(FILE *out)		/* I - Output file */
{
  if (Encryption && !PSLevel)
    encrypt_init();

  if (!Compression)
    return;

  compressor_active = 1;
  compressor.zalloc = (alloc_func)0;
  compressor.zfree  = (free_func)0;
  compressor.opaque = (voidpf)0;

  deflateInit(&compressor, Compression);

  compressor.next_out  = (Bytef *)comp_buffer;
  compressor.avail_out = sizeof(comp_buffer);
}


/*
 * 'flate_close_stream()' - Close a deflated output stream.
 */

static void
flate_close_stream(FILE *out)		/* I - Output file */
{
  int	status;				/* Deflate status */


  if (!Compression)
  {
#ifdef HTMLDOC_ASCII85
    if (PSLevel)
      ps_ascii85(out, (uchar *)"", 0, 1);
#endif // HTMLDOC_ASCII85

    return;
  }

  while ((status = deflate(&compressor, Z_FINISH)) != Z_STREAM_END)
  {
    if (status < Z_OK && status != Z_BUF_ERROR)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY, "deflate() failed (%d)", status);
      return;
    }

    if (PSLevel)
#ifdef HTMLDOC_ASCII85
      ps_ascii85(out, comp_buffer,
                 (uchar *)compressor.next_out - (uchar *)comp_buffer);
#else
      ps_hex(out, comp_buffer,
             (uchar *)compressor.next_out - (uchar *)comp_buffer);
#endif // HTMLDOC_ASCII85
    else
    {
      if (Encryption)
        rc4_encrypt(&encrypt_state, comp_buffer, comp_buffer,
	            (uchar *)compressor.next_out - (uchar *)comp_buffer);

      fwrite(comp_buffer, (uchar *)compressor.next_out - (uchar *)comp_buffer,
             1, out);
    }

    compressor.next_out  = (Bytef *)comp_buffer;
    compressor.avail_out = sizeof(comp_buffer);
  }

  if ((uchar *)compressor.next_out > (uchar *)comp_buffer)
  {
    if (PSLevel)
#ifdef HTMLDOC_ASCII85
      ps_ascii85(out, comp_buffer,
                 (uchar *)compressor.next_out - (uchar *)comp_buffer);
#else
      ps_hex(out, comp_buffer,
             (uchar *)compressor.next_out - (uchar *)comp_buffer);
#endif // HTMLDOC_ASCII85
    else
    {
      if (Encryption)
        rc4_encrypt(&encrypt_state, comp_buffer, comp_buffer,
	            (uchar *)compressor.next_out - (uchar *)comp_buffer);

      fwrite(comp_buffer, (uchar *)compressor.next_out - (uchar *)comp_buffer,
             1, out);
    }

  }

  deflateEnd(&compressor);

  compressor_active = 0;

#ifdef HTMLDOC_ASCII85
  if (PSLevel)
    ps_ascii85(out, (uchar *)"", 0, 1);
#else
  if (PSLevel)
  {
    // End of data marker...
    fputs(">\n", out);
  }
#endif // HTMLDOC_ASCII85
}


/*
 * 'flate_puts()' - Write a character string to a compressed stream.
 */

static void
flate_puts(const char *s,		/* I - String to write */
           FILE       *out)		/* I - Output file */
{
  flate_write(out, (uchar *)s, strlen(s));
}


/*
 * 'flate_printf()' - Write a formatted character string to a compressed stream.
 */

static void
flate_printf(FILE       *out,		/* I - Output file */
             const char *format,	/* I - Format string */
             ...)			/* I - Additional args as necessary */
{
  int		length;			/* Length of output string */
  char		buf[10240];		/* Output buffer */
  va_list	ap;			/* Argument pointer */


  va_start(ap, format);
  length = vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  flate_write(out, (uchar *)buf, length);
}


/*
 * 'flate_write()' - Write data to a compressed stream.
 */

static void
flate_write(FILE  *out,			/* I - Output file */
            uchar *buf,			/* I - Buffer */
            int   length,		/* I - Number of bytes to write */
	    int   flush)		/* I - Flush when writing data? */
{
  int	status;				/* Deflate status */


  if (compressor_active)
  {
    compressor.next_in  = buf;
    compressor.avail_in = length;

    while (compressor.avail_in > 0)
    {
      if (compressor.avail_out < (int)(sizeof(comp_buffer) / 8))
      {
	if (PSLevel)
#ifdef HTMLDOC_ASCII85
	  ps_ascii85(out, comp_buffer,
                     (uchar *)compressor.next_out - (uchar *)comp_buffer);
#else
	  ps_hex(out, comp_buffer,
                 (uchar *)compressor.next_out - (uchar *)comp_buffer);
#endif // HTMLDOC_ASCII85
	else
	{
	  if (Encryption)
            rc4_encrypt(&encrypt_state, comp_buffer, comp_buffer,
	        	(uchar *)compressor.next_out - (uchar *)comp_buffer);

	  fwrite(comp_buffer,
	         (uchar *)compressor.next_out - (uchar *)comp_buffer, 1, out);
	}

	compressor.next_out  = (Bytef *)comp_buffer;
	compressor.avail_out = sizeof(comp_buffer);
      }

      status = deflate(&compressor, flush ? Z_FULL_FLUSH : Z_NO_FLUSH);

      if (status < Z_OK && status != Z_BUF_ERROR)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY, "deflate() failed (%d)", status);
	return;
      }

      flush = 0;
    }
  }
  else if (Encryption && !PSLevel)
  {
    int		i,		// Looping var
		bytes;		// Number of bytes to encrypt/write
    uchar	newbuf[1024];	// New encrypted data buffer


    for (i = 0; i < length; i += sizeof(newbuf))
    {
      if ((bytes = length - i) > (int)sizeof(newbuf))
        bytes = sizeof(newbuf);

      rc4_encrypt(&encrypt_state, buf + i, newbuf, bytes);
      fwrite(newbuf, bytes, 1, out);
    }
  }
  else if (PSLevel)
#ifdef HTMLDOC_ASCII85
    ps_ascii85(out, buf, length);
#else
    ps_hex(out, buf, length);
#endif // HTMLDOC_ASCII85
  else
    fwrite(buf, length, 1, out);
}


/*
 * End of "$Id$".
 */
