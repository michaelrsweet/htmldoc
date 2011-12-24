//
// "$Id$"
//
//   PostScript + PDF output routines for HTMLDOC, a HTML document processing
//   program.
//
//   Just in case you didn't notice it, this file is too big; it will be
//   broken into more manageable pieces once we make all of the output
//   "drivers" into classes...
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//

//
// Include necessary headers.
//

/*#define DEBUG*/
#include "htmldoc.h"
#include "margin.h"
#include "md5.h"
#include "rc4.h"
#include "array.h"
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


/*
 * Output options...
 */

#define HTMLDOC_ASCII85
//#define HTMLDOC_INTERPOLATION


//
// Constants...
//

#define HD_RENDER_TEXT	0		/* Text fragment */
#define HD_RENDER_IMAGE	1		/* Image */
#define HD_RENDER_BOX	2		/* Box */
#define HD_RENDER_LINK	3		/* Hyperlink */
#define HD_RENDER_BG	4		/* Background */


//
// Structures...
//

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
      hdChar	buffer[1];		/* String buffer */
    }   	text;
    hdImage	*image;			/* Image pointer */
    float	box[3];			/* Box color */
    hdChar	link[1];		/* Link URL */
    hdStyle	*bg;			/* Background style */
  }		data;
};

struct hdRenderLink			/**** Named link position structure */
{
  short		page,			/* Page # */
		top;			/* Top position */
  hdChar	name[124];		/* Reference name */
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
  hdChar	*chapter,		// Chapter text
		*heading;		// Heading text
  hdTree	*headnode;		// Heading node
  hdChar	*header[3],		// Headers for regular pages
		*header1[3],		// Headers for first pages
		*footer[3];		// Footers for all pages
  char		media_color[64],	// Media color
		media_type[64];		// Media type
  int		media_position;		// Media position
  char		page_text[64];		// Page number for TOC
  hdStyle	*background;		// Background style
  hdImage	*background_image;	// Background image
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


//
// Timezone offset for dates, below...
//

#ifdef HAVE_TM_GMTOFF
#  define timezone (doc_date->tm_gmtoff)
#elif defined(__CYGWIN__)
#  define timezone _timezone
#endif /* HAVE_TM_GMTOFF */


//
// Local globals...
//

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
static hdPage	*pages = NULL;
static hdTree	*current_heading;

static int	num_outpages = 0;
static hdOutPage *outpages = NULL;

static int	num_links = 0,
		alloc_links = 0;
static hdRenderLink	*links = NULL;

static int	list_indent = 0;
static hdChar	list_types[16];
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
		font_objects[HD_FONT_FACE_MAX * HD_FONT_INTERNAL_MAX];

static hdChar	*doc_title = NULL;
static float	logo_width,
		logo_height;

static float	hdimage_width[MAX_HF_IMAGES],
		hdimage_height[MAX_HF_IMAGES];
static float    maxhfheight,
		max_hfsize;

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

static hdChar		encrypt_key[16];
static int		encrypt_len;
static hdByte		file_id[16];


inline bool				// O - true if base font, false otherwise
is_base_font(int face)			// I - Font face */
{
  return (face < HD_FONT_FACE_MONOSPACE || face == HD_FONT_FACE_SYMBOL);
}


//
// Local functions...
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static void	pspdf_debug_stats();

static void	pspdf_transform_coords(hdPage *p, float &x, float  &y);
static void	pspdf_transform_page(int outpage, int pos, int page);

static void	pspdf_prepare_outpages();
static void	pspdf_prepare_page(int page);
static void	pspdf_prepare_heading(int page, int print_page, hdChar **format,
		                      int y, char *page_text, int page_len);
static void	ps_write_document(hdChar *author, hdChar *creator,
		                  hdChar *copyright, hdChar *keywords,
				  hdChar *subject);
static void	ps_write_outpage(hdFile *out, int outpage);
static void	ps_write_page(hdFile *out, int page);
static void	ps_write_background(hdFile *out);
static void	pdf_write_document(hdChar *author, hdChar *creator,
		                   hdChar *copyright, hdChar *keywords,
				   hdChar *subject, hdTree *doc, hdTree *toc);
static void	pdf_write_outpage(hdFile *out, int outpage);
static void	pdf_write_page(hdFile *out, int page);
static void	pdf_write_resources(hdFile *out, int page);
#ifdef DEBUG_TOC
static void	pdf_text_contents(hdFile *out, hdTree *toc, int indent = 0);
#endif // DEBUG_TOC
static void	pdf_write_contents(hdFile *out, hdTree *toc, int parent,
		                   int prev, int next, int *heading);
static void	pdf_write_files(hdFile *out, hdTree *doc);
static void	pdf_write_links(hdFile *out);
static void	pdf_write_names(hdFile *out);
static int	pdf_count_headings(hdTree *toc);

static int	pdf_start_object(hdFile *out, int array = 0);
static hdFile	*pdf_start_stream(hdFile *out, hdArray &filters, int width = 0,
				  int height = 0, int depth = 0);
static void	pdf_end_object(hdFile *out);
static void	pdf_end_stream(hdFile *out, hdArray &filters);

static hdRC4Filter	*encrypt_init(hdFile *out);

static void	parse_contents(hdTree *t, hdMargin *margins, float *y, int *page, int *heading,
			       hdTree *chap);
static void	parse_doc(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			  hdTree *cpara, float *needspace);
static void	parse_heading(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			      float *needspace);
static void	parse_paragraph(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			        float *needspace);
static void	parse_pre(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			  float *needspace);
static void	parse_table(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			    float *needspace);
static void	parse_list(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			   float *needspace);
static void	init_list(hdTree *t);
static void	parse_comment(hdTree *t, hdMargin *margins, float *x, float *y, int *page,
			      hdTree *para, float *needspace);

static hdTree	*real_prev(hdTree *t);
static hdTree	*real_next(hdTree *t);

static void	check_pages(int page);

static void	add_link(hdChar *name, int page, int top);
static hdRenderLink	*find_link(hdChar *name);
static int	compare_links(hdRenderLink *n1, hdRenderLink *n2);

static void	find_background(hdTree *t);
static void	write_background(hdFile *out, int page);

static hdRender	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data,
			    hdRender *insert = 0);
static void	copy_tree(hdTree *parent, hdTree *t);
static float	get_cell_size(hdTree *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
static float	get_table_size(hdTree *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
static hdTree	*flatten_tree(hdTree *t);
static void	update_image_size(hdTree *t);
static hdChar	*get_title(hdTree *doc);
static hdFile	*open_file(void);
static void	set_color(hdFile *out, float *rgb);
static void	set_font(hdFile *out, int typeface, int style, float size);
static void	set_pos(hdFile *out, float x, float y);
static void	write_prolog(hdFile *out, int pages, hdChar *author,
		             hdChar *creator, hdChar *copyright,
			     hdChar *keywords, hdChar *subject);
static int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
static void	write_image(hdFile *out, hdRender *r, int write_obj = 0);
static void	write_imagemask(hdFile *out, hdRender *r);
static void	write_string(hdFile *out, hdChar *s);
static void	write_text(hdFile *out, hdRender *r);
static void	write_trailer(hdFile *out, int pages);
static int	write_type1(hdFile *out, hdFontFace typeface,
			    hdFontStyle style);
static void	write_utf16(hdFile *out, hdChar *s);
static int	update_index(hdTree *t, int last_page = -1);


/*
 * 'pspdf_export()' - Export PostScript/PDF file(s)...
 */

int
pspdf_export(hdTree *document,		/* I - Document to export */
             hdTree *toc,		/* I - Table of contents for document */
	     hdTree *ind)		// I - Index of document
{
  int		i, j;			/* Looping vars */
  char		title_file[1024],	/* Location of title file */
		title_dirname[1024];	/* Directory of title file */
  hdChar	*author,		/* Author of document */
		*creator,		/* HTML file creator (Netscape, etc) */
		*copyright,		/* File copyright */
		*docnumber,		/* Document number */
		*keywords,		/* Search keywords */
		*subject;		/* Subject */
  hdTree	*t;			/* Title page document tree */
  hdFile	*fp;			/* Title page file */
  float		x, y;			/* Current page position */
  float		width,			/* Width of title, author, etc */
		height;			/* Height of title page area */
  hdMargin	*margins;		/* Margin stack */
  int		pos,			/* Current header/footer position */
		page,			/* Current page # */
		heading,		/* Current heading # */
		toc_duplex,		/* Duplex TOC pages? */
		toc_landscape,		/* Do TOC in landscape? */
		toc_width,		/* Width of TOC pages */
		toc_length,		/* Length of TOC pages */
		toc_left,		/* TOC page margins */
		toc_right,
		toc_bottom,
		toc_top;
  float		timage_width,		/* Title image width */
		timage_height;		/* Title image height */
  hdStyle	*temp,			/* Current style for title page */
		*body_title,		/* Body style for title page */
		*h1_title,		/* Heading style for title page */
		*p_title;		/* Paragraph style for title page */
  hdRender	*r;			/* Rendering structure... */
  float		rgb[3];			/* Text color */
  float		needspace;		/* Needed whitespace */


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
  author      = htmlGetMeta(document, "author");
  creator     = htmlGetMeta(document, "generator");
  copyright   = htmlGetMeta(document, "copyright");
  docnumber   = htmlGetMeta(document, "docnumber");
  keywords    = htmlGetMeta(document, "keywords");
  subject     = htmlGetMeta(document, "subject");
  maxhfheight = 0.0f;

  if (LogoImage)
  {
    logo_width  = LogoImage->width() * PagePrintWidth / _htmlStyleSheet->browser_width;
    logo_height = logo_width * LogoImage->height() / LogoImage->width();

    if (logo_height > maxhfheight)
      maxhfheight = logo_height;
  }
  else
    logo_width = logo_height = 0.0f;

  for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
    if (HFImage[hfi])
    {
      hdimage_width[hfi]  = HFImage[hfi]->width() * PagePrintWidth /
                            _htmlStyleSheet->browser_width;
      hdimage_height[hfi] = hdimage_width[hfi] * HFImage[hfi]->height() /
                            HFImage[hfi]->width();

      if (hdimage_height[hfi] > maxhfheight)
        maxhfheight = hdimage_height[hfi];
    }
    else
      hdimage_width[hfi] = hdimage_height[hfi] = 0.0f;

  find_background(document);
  get_color((hdChar *)LinkColor, link_color);

 /*
  * Initialize page rendering variables...
  */

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
  alloc_pages    = 0;
  pages          = NULL;

  DEBUG_printf(("pspdf_export: TitlePage = %d, TitleImage = \"%s\"\n",
                TitlePage, TitleImage));

  if (TitlePage)
  {
    if (TitleFile[0] && !TitleImage)
    {
      DEBUG_printf(("pspdf_export: Generating a titlepage using \"%s\"\n",
                    TitleFile));

      // Find the title file...
      if (!hdFile::find(Path, TitleFile, title_file, sizeof(title_file)))
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to find title file \"%s\"!", TitleFile);
	return (1);
      }

      // Write a title page from HTML source...
      if ((fp = hdFile::open(title_file, HD_FILE_READ)) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to open title file \"%s\" - %s!",
                       TitleFile, strerror(errno));
	return (1);
      }

      t = htmlReadFile(NULL, fp, hdFile::dirname(TitleFile, title_dirname,
                                                 sizeof(title_dirname)));
      htmlFixLinks(t, t, title_dirname);
      delete fp;

      title_page      = 1;
      current_heading = NULL;
      x               = 0.0f;
      y               = PagePrintLength;
      page            = 0;
      needspace       = 0;
      margins         = new hdMargin(0.0f, PagePrintWidth, 0.0f, PagePrintLength);

      parse_doc(t, margins, &x, &y, &page, NULL, &needspace);

      if (PageDuplex && (num_pages & 1))
	check_pages(num_pages);

      delete margins;

      htmlDeleteTree(t);
    }
    else
    {
     /*
      * Create a standard title page...
      */

      if (TitleImage)
      {
	timage_width  = TitleImage->width() * PagePrintWidth /
	                _htmlStyleSheet->browser_width;
	timage_height = timage_width * TitleImage->height() /
	                TitleImage->width();
      }
      else
        timage_width = timage_height = 0.0f;

      check_pages(0);
      if (PageDuplex)
        check_pages(1);

      height = 0.0;

      hdStyleSelector h1_selector(HD_ELEMENT_H1, NULL, NULL, "_HD_TITLE");
      hdStyleSelector p_selector(HD_ELEMENT_P, NULL, NULL, "_HD_TITLE");

      body_title = _htmlStyleSheet->find_style(HD_ELEMENT_BODY, "HD_TITLE");
      temp       = _htmlStyleSheet->find_style(HD_ELEMENT_H1, "HD_TITLE");
      h1_title   = new hdStyle(1, &h1_selector, body_title);
      h1_title->inherit(temp);
      _htmlStyleSheet->add_style(h1_title);

      temp       = _htmlStyleSheet->find_style(HD_ELEMENT_P, "HD_TITLE");
      p_title    = new hdStyle(1, &p_selector, body_title);
      p_title->inherit(temp);
      _htmlStyleSheet->add_style(p_title);

      if (TitleImage)
	height += timage_height + p_title->line_height;
      if (doc_title != NULL)
	height += h1_title->line_height + p_title->line_height;
      if (author != NULL)
	height += p_title->line_height;
      if (docnumber != NULL)
	height += p_title->line_height;
      if (copyright != NULL)
	height += p_title->line_height;

      y = 0.5f * (PagePrintLength + height);

      if (TitleImage)
      {
	new_render(0, HD_RENDER_IMAGE, 0.5f * (PagePrintWidth - timage_width),
                   y - timage_height, timage_width, timage_height, TitleImage);
	y -= timage_height + p_title->line_height;
      }

      rgb[0] = p_title->color[0] / 255.0;
      rgb[1] = p_title->color[1] / 255.0;
      rgb[2] = p_title->color[2] / 255.0;

      if (doc_title != NULL)
      {
	width = h1_title->get_width(doc_title);
	r     = new_render(0, HD_RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - h1_title->line_height, width,
			   h1_title->font_size, doc_title);

	r->data.text.typeface = h1_title->font->typeface;
	r->data.text.style    = h1_title->font->style;
	r->data.text.size     = h1_title->font_size;
	r->data.text.rgb[0]   = h1_title->color[0] / 255.0;
	r->data.text.rgb[1]   = h1_title->color[1] / 255.0;
	r->data.text.rgb[2]   = h1_title->color[2] / 255.0;

	y -= h1_title->line_height;

	if (docnumber != NULL)
	{
	  width = p_title->get_width(docnumber);
	  r     = new_render(0, HD_RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                             y - p_title->line_height, width,
			     p_title->font_size, docnumber);

	  r->data.text.typeface = p_title->font->typeface;
	  r->data.text.style    = p_title->font->style;
	  r->data.text.size     = p_title->font_size;
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	  y -= p_title->line_height;
	}

	y -= p_title->line_height;
      }

      if (author != NULL)
      {
	width = p_title->get_width(author);
	r     = new_render(0, HD_RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - p_title->line_height, width, p_title->font_size,
			   author);

	r->data.text.typeface = p_title->font->typeface;
	r->data.text.style    = p_title->font->style;
	r->data.text.size     = p_title->font_size;
	memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	y -= p_title->line_height;
      }

      if (copyright != NULL)
      {
	width = p_title->get_width(copyright);
	r     = new_render(0, HD_RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - p_title->line_height, width, p_title->font_size,
			   copyright);

	r->data.text.typeface = p_title->font->typeface;
	r->data.text.style    = p_title->font->style;
	r->data.text.size     = p_title->font_size;
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

  if (OutputType == HD_OUTPUT_BOOK)
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
  margins         = new hdMargin(0.0f, PagePrintWidth, 0.0f, PagePrintLength);

  // Adjust top margin as needed...
  float adjust, image_adjust, temp_adjust;

  hdStyle *header_style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");
  if (header_style->font_size_rel &&
      !strcmp(header_style->font_size_rel, "inherit"))
    header_style->inherit(&(_htmlStyleSheet->def_style));

  hdStyle *footer_style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
  if (footer_style->font_size_rel &&
      !strcmp(footer_style->font_size_rel, "inherit"))
    footer_style->inherit(&(_htmlStyleSheet->def_style));

  if (header_style->font_size > footer_style->font_size)
    max_hfsize = header_style->font_size;
  else
    max_hfsize = footer_style->font_size;

  if (maxhfheight > max_hfsize)
    image_adjust = maxhfheight + max_hfsize;
  else
    image_adjust = 2 * max_hfsize;

  for (adjust = 0.0, pos = 0; pos < 3; pos ++)
  {
    if (Header[pos])
    {
      if (strstr(Header[pos], "$IMAGE") != NULL ||
          strstr(Header[pos], "$HFIMAGE") != NULL)
        temp_adjust = image_adjust;
      else
        temp_adjust = 2 * max_hfsize;

      if (temp_adjust > adjust)
        adjust = temp_adjust;
    }

    if (Header1[pos])
    {
      if (strstr(Header1[pos], "$IMAGE") != NULL ||
          strstr(Header1[pos], "$HFIMAGE") != NULL)
        temp_adjust = image_adjust;
      else
        temp_adjust = 2 * max_hfsize;

      if (temp_adjust > adjust)
        adjust = temp_adjust;
    }
  }

  margins->adjust_top(-adjust);

  // Adjust bottom margin as needed...
  for (adjust = 0.0, pos = 0; pos < 3; pos ++)
  {
    if (Footer[pos])
    {
      if (strstr(Footer[pos], "$IMAGE") != NULL ||
          strstr(Footer[pos], "$HFIMAGE") != NULL)
        temp_adjust = image_adjust;
      else
        temp_adjust = 2 * max_hfsize;

      if (temp_adjust > adjust)
        adjust = temp_adjust;
    }
  }

  margins->adjust_bottom(adjust);

  y = margins->top();

  parse_doc(document, margins, &x, &y, &page, NULL, &needspace);

  if (PageDuplex && (num_pages & 1))
    check_pages(num_pages);

  chapter_ends[chapter] = num_pages - 1;

  for (chapter = 1; chapter <= TocDocCount; chapter ++)
    for (page = chapter_starts[chapter]; page <= chapter_ends[chapter]; page ++)
      pspdf_prepare_page(page);

  if (ind)
  {
    update_index(ind);

    chapter = TocDocCount;

    parse_doc(ind, margins, &x, &y, &page, NULL, &needspace);

    if (PageDuplex && (num_pages & 1))
      check_pages(num_pages);

    chapter_ends[chapter] = num_pages - 1;

    for (page = chapter_starts[chapter]; page < num_pages; page ++)
      pspdf_prepare_page(page);
  }

  delete margins;

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

    margins = new hdMargin(0.0f, PagePrintWidth, 0.0f, PagePrintLength);
    
    // Adjust top margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (TocHeader[pos])
	break;

    if (pos < 3)
    {
      if (maxhfheight > max_hfsize)
	margins->adjust_top(-(maxhfheight + max_hfsize));
      else
	margins->adjust_top(-2 * max_hfsize);
    }

    // Adjust bottom margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (TocFooter[pos])
	break;

    if (pos < 3)
    {
      if (maxhfheight > max_hfsize)
	margins->adjust_bottom(maxhfheight + max_hfsize);
      else
	margins->adjust_bottom(2 * max_hfsize);
    }

    y                 = 0.0;
    page              = num_pages - 1;
    heading           = 0;
    chapter_starts[0] = num_pages;
    chapter           = 0;

    parse_contents(toc, margins, &y, &page, &heading, 0);

    delete margins;

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
  hdRender	*r;			// Render node
  int		bytes;			// Number of bytes
  int		nodes;			// Number of nodes


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

  bytes = alloc_headings * sizeof(int) * 2;

  bytes += alloc_pages * sizeof(hdPage);
  for (i = 0, nodes = 0; i < num_pages; i ++)
  {
    for (r = pages[i].start; r != NULL; r = r->next, nodes ++)
    {
      bytes += sizeof(hdRender);

      if (r->type == HD_RENDER_TEXT)
        bytes += strlen((char *)r->data.text.buffer);
    }
  }

  bytes += num_outpages * sizeof(hdOutPage);
  bytes += alloc_links * sizeof(hdRenderLink);
  bytes += alloc_objects * sizeof(int);

  progress_error(HD_ERROR_NONE, "DEBUG: Render Data = %dk, %d nodes",
                 (bytes + 1023) / 1024, nodes);
}


/*
 * 'pspdf_transform_coords()' - Transform page coordinates.
 */

static void
pspdf_transform_coords(hdPage *p,	// I - Page
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
  hdOutPage	*op;			// Current output page
  hdPage	*bp;			// Current base page
  hdPage	*p;			// Current input page
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
  hdPage	*page;		/* Current page */
  hdOutPage	*outpage;	/* Current output page */


  // Allocate an output page array...
  outpages = (hdOutPage *)malloc(sizeof(hdOutPage) * num_pages);

  memset(outpages, -1, sizeof(hdOutPage) * num_pages);

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
  if (OutputType == HD_OUTPUT_BOOK && TocLevels > 0)
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

  if (chapter == 0 && OutputType == HD_OUTPUT_BOOK)
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

  top = (int)(PagePrintLength - max_hfsize);

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

    if (page > chapter_starts[chapter] || OutputType != HD_OUTPUT_BOOK)
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
pspdf_prepare_heading(int    page,	// I - Page number
                      int    print_page,// I - Printed page number
		      hdChar **format,	// I - Page headings
		      int    y,		// I - Baseline of heading
		      char   *page_text,// O - Page number text
		      int    page_len)	// I - Size of page text
{
  int		pos,			// Position in heading
		dir;			// Direction of page
  char		*number;		// Page number
  char		buffer[1024],		// String buffer
		*bufptr,		// Pointer into buffer
		*formatptr;		// Pointer into format string
  int		formatlen;		// Length of format command string
  hdRender	*temp;			// Render structure for titles, etc.
  hdStyle	*style;			// Style data


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

  if (y == 0)
    style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_FOOTER");
  else
    style = _htmlStyleSheet->get_style(HD_ELEMENT_P, "HD_HEADER");

  if (style->font_size_rel &&
      !strcmp(style->font_size_rel, "inherit"))
    style->inherit(&(_htmlStyleSheet->def_style));

  for (pos = 0; pos < 3; pos ++, format += dir)
  {
   /*
    * Add the appropriate object...
    */

    if (!*format)
      continue;

    temp = NULL;

    if (strncasecmp((char *)*format, "$LOGOIMAGE", 10) == 0 && LogoImage)
    {
      // Insert the logo image...
      if (y < (PagePrintLength / 2))
	temp = new_render(page, HD_RENDER_IMAGE, 0, y, logo_width,
	                  logo_height, LogoImage);
      else // Offset from top
	temp = new_render(page, HD_RENDER_IMAGE, 0,
	                  y + max_hfsize - logo_height,
	                  logo_width, logo_height, LogoImage);
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
          temp = new_render(page, HD_RENDER_IMAGE, 0, y, hdimage_width[hfi],
                            hdimage_height[hfi], HFImage[hfi]);
        else
          temp = new_render(page, HD_RENDER_IMAGE, 0,
                            y + max_hfsize - hdimage_height[hfi],
                            hdimage_width[hfi], hdimage_height[hfi],
			    HFImage[hfi]);
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

      temp = new_render(page, HD_RENDER_TEXT, 0, y,
                	style->get_width((hdChar *)buffer),
	        	style->font_size, (hdChar *)buffer);

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

    if (temp->type == HD_RENDER_TEXT)
    {
      temp->data.text.typeface = style->font->typeface;
      temp->data.text.style    = style->font->style;
      temp->data.text.size     = style->font_size;
      temp->data.text.rgb[0]   = style->color[0] / 255.0;
      temp->data.text.rgb[1]   = style->color[1] / 255.0;
      temp->data.text.rgb[2]   = style->color[2] / 255.0;
    }
  }
}


/*
 * 'ps_write_document()' - Write all render entities to PostScript file(s).
 */

static void
ps_write_document(hdChar *author,	/* I - Author of document */
        	  hdChar *creator,	/* I - Application that generated the HTML file */
        	  hdChar *copyright,	/* I - Copyright (if any) on the document */
                  hdChar *keywords,	/* I - Search keywords */
		  hdChar *subject)	/* I - Subject */
{
  hdFile	*out;			/* Output file */
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

  if (OutputType == HD_OUTPUT_BOOK && TocLevels > 0)
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

      progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)out->size());

      delete out;
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

      progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)out->size());

      delete out;
    }
  }

 /*
  * Close the output file as necessary...
  */

  if (!OutputFiles)
  {
    write_trailer(out, 0);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)out->size());

    delete out;
  }

  if (Verbosity)
    progress_hide();
}


/*
 * 'ps_write_outpage()' - Write an output page.
 */

static void
ps_write_outpage(hdFile *out,		/* I - Output file */
                 int    outpage)	/* I - Output page number */
{
  int		file_page;		/* Current page # in document */
  hdPage	*p;			/* Current page */
  hdOutPage	*op;			/* Current output page */
  int		i;			/* Looping var */


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

  out->printf("%%%%Page: (%s) %d\n", p->page_text, file_page);
  if (op->nup == 1)
  {
    if (p->duplex && !(file_page & 1))
      out->printf("%%%%PageBoundingBox: %d %d %d %d\n",
                  p->right, p->bottom, p->width - p->left, p->length - p->top);
    else
      out->printf("%%%%PageBoundingBox: %d %d %d %d\n",
                  p->left, p->bottom, p->width - p->right, p->length - p->top);
  }
  else
    out->printf("%%%%PageBoundingBox: 0 0 %d %d\n", p->width, p->length);

  if (PSLevel > 1 && PSCommands)
  {
    out->puts("%%BeginPageSetup\n");

    if (p->width == 612 && p->length == 792)
      out->puts("%%BeginFeature: *PageSize Letter\n");
    else if (p->width == 612 && p->length == 1008)
      out->puts("%%BeginFeature: *PageSize Legal\n");
    else if (p->width == 792 && p->length == 1224)
      out->puts("%%BeginFeature: *PageSize Tabloid\n");
    else if (p->width == 842 && p->length == 1190)
      out->puts("%%BeginFeature: *PageSize A3\n");
    else if (p->width == 595 && p->length == 842)
      out->puts("%%BeginFeature: *PageSize A4\n");
    else
      out->printf("%%%%BeginFeature: *PageSize w%dh%d\n", p->width, p->length);

    out->printf("%d %d SetPageSize\n", p->width, p->length);
    out->puts("%%EndFeature\n");

    if (p->duplex)
    {
      if (p->landscape)
      {
	out->puts("%%BeginFeature: *Duplex DuplexTumble\n");
	out->puts("true true SetDuplexMode\n");
        out->puts("%%EndFeature\n");
      }
      else
      {
	out->puts("%%BeginFeature: *Duplex DuplexNoTumble\n");
	out->puts("true false SetDuplexMode\n");
        out->puts("%%EndFeature\n");
      }
    }
    else
    {
      out->puts("%%BeginFeature: *Duplex None\n");
      out->puts("false false SetDuplexMode\n");
      out->puts("%%EndFeature\n");
    }

    if (p->media_color[0])
    {
      out->printf("%%%%BeginFeature: *MediaColor %s\n", p->media_color);
      out->printf("(%s) SetMediaColor\n", p->media_color);
      out->puts("%%EndFeature\n");
    }

    if (p->media_position)
    {
      out->printf("%%%%BeginFeature: *InputSlot Tray%d\n",
              p->media_position);
      out->printf("%d SetMediaPosition\n", p->media_position);
      out->puts("%%EndFeature\n");
    }

    if (p->media_type[0])
    {
      out->printf("%%%%BeginFeature: *MediaType %s\n", p->media_type);
      out->printf("(%s) SetMediaType\n", p->media_type);
      out->puts("%%EndFeature\n");
    }

    out->puts("%%EndPageSetup\n");
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

          out->printf("GS[%g %g %g %g %g %g]CM\n",
	              p->outmatrix[0][0], p->outmatrix[1][0],
	              p->outmatrix[0][1], p->outmatrix[1][1],
	              p->outmatrix[0][2], p->outmatrix[1][2]);
          ps_write_page(out, op->pages[i]);
	  out->puts("GR\n");
	}
	break;
  }

 /*
  * Output the page trailer...
  */

  out->puts("SP\n");
}


/*
 * 'ps_write_page()' - Write all render entities on a page to a PostScript file.
 */

static void
ps_write_page(hdFile *out,		/* I - Output file */
              int    page)		/* I - Page number */
{
  hdRender	*r,			/* Render pointer */
		*next;			/* Next render */
  hdPage	*p;			/* Current page */
  const char	*debug;			/* HTMLDOC_DEBUG environment variable */


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

  out->puts("GS\n");

  if (p->landscape)
  {
    if (p->duplex && (page & 1))
      out->printf("0 %d T -90 RO\n", p->length);
    else
      out->printf("%d 0 T 90 RO\n", p->width);
  }

  write_background(out, page);

  if (p->duplex && (page & 1))
    out->printf("%d %d T\n", p->right, p->bottom);
  else
    out->printf("%d %d T\n", p->left, p->bottom);

 /*
  * Render all graphics elements...
  */

  for (r = p->start; r != NULL; r = r->next)
    switch (r->type)
    {
      case HD_RENDER_BOX :
	  set_color(out, r->data.box);
	  set_pos(out, r->x, r->y);
	  if (r->height > 0.0f)
            out->printf(" %.1f %.1f F\n", r->width, r->height);
	  else
            out->printf(" %.1f L\n", r->width);

	  render_x = -1.0f;
	  break;

      case HD_RENDER_IMAGE :
          if (r->width > 0.01f && r->height > 0.01f)
            write_image(out, r);
          break;
    }

 /*
  * Render all text elements, freeing used memory as we go...
  */

  for (r = p->start, next = NULL; r != NULL; r = next)
  {
    if (r->type == HD_RENDER_TEXT)
      write_text(out, r);

    next = r->next;
    free(r);
  }

  p->start = NULL;

  if ((debug = getenv("HTMLDOC_DEBUG")) != NULL && strstr(debug, "margin"))
  {
    // Show printable area...
    out->printf("1 0 1 C 0 0 %d %d B\n", p->width - p->right - p->left,
        	p->length - p->top - p->bottom);
  }

 /*
  * Output the page trailer...
  */

  out->puts("GR\n");
}


/*
 * 'ps_write_background()' - Write a background image...
 */

static void
ps_write_background(hdFile *out)	/* I - Output file */
{
  int	y,				/* Current line */
	pwidth;				/* Pixel width */


  if (!BodyImage->pixels())
    BodyImage->load();

  pwidth = BodyImage->width() * BodyImage->depth();

  out->puts("/BG[");
  for (y = 0; y < BodyImage->height(); y ++)
  {
    out->put('<');
    hdASCIIHexFilter *hex = new hdASCIIHexFilter(out);
    hex->write(BodyImage->pixels() + y * pwidth, pwidth);
    delete hex;
    out->put('>');
  }
  out->puts("]def");

  BodyImage->free();
}


/*
 * 'pdf_write_document()' - Write all render entities to a PDF file.
 */

static void
pdf_write_document(hdChar  *author,	// I - Author of document
        	   hdChar  *creator,	// I - Application that generated the HTML file
        	   hdChar  *copyright,	// I - Copyright (if any) on the document
                   hdChar  *keywords,	// I - Search keywords
		   hdChar  *subject,	// I - Subject
		   hdTree *doc,		// I - Document
                   hdTree *toc)		// I - Table of contents tree
{
  int		i;			// Looping variable
  hdFile	*out;			// Output file
  int		outpage,		// Current page #
		heading;		// Current heading #
  int		bytes;			// Number of bytes
  char		buffer[8192];		// Copy buffer
  int		num_images;		// Number of images in document
  hdImage	**images;		// Pointers to images
  hdRender	temp;			// Dummy rendering data...


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
  write_prolog(out, num_outpages, author, creator, copyright, keywords,
               subject);

  // Write images as needed...
  num_images = hdImage::num_images();
  images     = hdImage::images();

  for (i = 0; i < num_images; i ++)
  {
    int	hfi;				// Header/footer image index


    for (hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (images[i] == HFImage[hfi])
        break;

    if (images[i]->use() > 1 || images[i]->mask() ||
        (images[i]->width() * images[i]->height() *
	 images[i]->depth()) > 65536 ||
	images[i] == BodyImage ||
	images[i] == LogoImage ||
	hfi < MAX_HF_IMAGES)
    {
      progress_show("Writing image %d (%s)...", i + 1, images[i]->uri());
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

  out->printf("/Type/Pages/Count %d/Kids[", num_outpages);

  for (outpage = 0; outpage < num_outpages; outpage ++)
    out->printf("%d 0 R\n", pages_object + outpage * 2 + 1);

  out->puts("]");
  pdf_end_object(out);

  for (outpage = 0; outpage < num_outpages; outpage ++)
    pdf_write_outpage(out, outpage);

  if (OutputType == HD_OUTPUT_BOOK && TocLevels > 0)
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

  progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)out->size());

  //
  // If we are sending the output to stdout, copy the temp file now...
  //

  if (!OutputPath[0])
  {
#ifdef WIN32
    // Make sure we are in binary mode...  stupid Microsoft!
    setmode(1, O_BINARY);
#endif // WIN32

    if (CGIMode)
    {
      // In CGI mode, we only produce PDF output to stdout...
      printf("Content-Type: application/pdf\r\n"
	     "Content-Length: %ld\r\n"
	     "Content-Disposition: inline; filename=\"htmldoc.pdf\"\r\n"
	     "Accept-Ranges: none\r\n"
	     "X-Creator: HTMLDOC " SVERSION "\r\n"
	     "\r\n", (long)out->size());
    }

    // Copy the temporary file to stdout...
    out->seek(0, SEEK_SET);

    while ((bytes = out->read(buffer, sizeof(buffer))) > 0)
      fwrite(buffer, 1, bytes, stdout);

    // Remove the temporary file...
    unlink(stdout_filename);
  }

  delete out;

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
pdf_write_resources(hdFile *out,	/* I - Output file */
                    int    outpage)	/* I - Output page for resources */
{
  int		i;			/* Looping var */
  hdOutPage	*op;			/* Current output page */
  hdPage	*p;			/* Current page */
  hdRender	*r;			/* Render pointer */
  int		fonts_used[HD_FONT_FACE_MAX * HD_FONT_INTERNAL_MAX];
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
  images_used = BodyImage != NULL;
  text_used   = 0;

  op = outpages + outpage;
  for (i = 0; i < op->nup; i ++)
  {
    if (op->pages[i] < 0)
      break;

    p = pages + op->pages[i];

    for (r = p->start; r != NULL; r = r->next)
      if (r->type == HD_RENDER_IMAGE)
	images_used = 1;
      else if (r->type == HD_RENDER_TEXT)
      {
	text_used = 1;
	fonts_used[r->data.text.typeface * 4 + r->data.text.style] = 1;
      }
  }

  out->puts("/Resources<<");

  if (!images_used)
    out->puts("/ProcSet[/PDF/Text]");
  else if (PDFVersion >= 12)
  {
    if (OutputColor)
      out->puts("/ProcSet[/PDF/Text/ImageB/ImageC/ImageI]");
    else
      out->puts("/ProcSet[/PDF/Text/ImageB/ImageI]");
  }
  else
  {
    if (OutputColor)
      out->puts("/ProcSet[/PDF/Text/ImageB/ImageC]");
    else
      out->puts("/ProcSet[/PDF/Text/ImageB]");
  }

  if (text_used)
  {
    out->puts("/Font<<");
    for (i = 0; i < (HD_FONT_FACE_MAX * HD_FONT_INTERNAL_MAX); i ++)
      if (fonts_used[i] && font_objects[i])
	out->printf("/F%x %d 0 R", i, font_objects[i]);
    out->puts(">>");
  }

  out->puts("/XObject<<");

  for (i = 0; i < op->nup; i ++)
  {
    if (op->pages[i] < 0)
      break;

    p = pages + op->pages[i];

    for (r = p->start; r != NULL; r = r->next)
      if (r->type == HD_RENDER_IMAGE && r->data.image->obj())
	out->printf("/I%d %d 0 R", r->data.image->obj(), r->data.image->obj());
  }

  if (BodyImage)
    out->printf("/I%d %d 0 R", BodyImage->obj(), BodyImage->obj());

  out->puts(">>>>");

  if (PDFEffect)
    out->printf("/Dur %.0f/Trans<</Type/Trans/D %.1f%s>>", PDFPageDuration,
            PDFEffectDuration, effects[PDFEffect]);
}


/*
 * 'pdf_write_outpage()' - Write an output page.
 */

static void
pdf_write_outpage(hdFile *out,		/* I - Output file */
                  int    outpage)	/* I - Output page number */
{
  int		i;			/* Looping var */
  hdPage	*p;			/* Current page */
  hdOutPage	*op;			/* Output page */
  hdFile	*filter;		/* Output stream */
  hdArray	filters;		// Stream filters


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

  out->puts("");
  out->printf("/Type/Page/Parent %d 0 R/Contents %d 0 R", pages_object,
              num_objects + 1);
  if (p->landscape)
    out->printf("/MediaBox[0 0 %d %d]", p->length, p->width);
  else
    out->printf("/MediaBox[0 0 %d %d]", p->width, p->length);

  pdf_write_resources(out, outpage);

 /*
  * Actions (links)...
  */

  if (op->annot_object > 0)
    out->printf("/Annots %d 0 R", op->annot_object);

  pdf_end_object(out);

  pdf_start_object(out);
  filter = pdf_start_stream(out, filters);

 /*
  * Render all of the pages...
  */

  switch (op->nup)
  {
    case 1 :
        pdf_write_page(filter, op->pages[0]);
	break;

    default :
        for (i = 0; i < op->nup; i ++)
	{
	  if (op->pages[i] < 0)
	    break;

          p = pages + op->pages[i];

          filter->printf("q %g %g %g %g %g %g cm\n",
	                 p->outmatrix[0][0], p->outmatrix[1][0],
	                 p->outmatrix[0][1], p->outmatrix[1][1],
	                 p->outmatrix[0][2], p->outmatrix[1][2]);
          pdf_write_page(filter, op->pages[i]);
	  filter->puts("Q\n");
	}
	break;
  }

 /*
  * Close out the page...
  */

  pdf_end_stream(out, filters);
}


/*
 * 'pdf_write_page()' - Write a page to a PDF file.
 */

static void
pdf_write_page(hdFile *out,		/* I - Output file */
               int    page)		/* I - Page number */
{
  hdRender	*r,			/* Render pointer */
		*next;			/* Next render */
  float		box[3];			/* RGB color for boxes */
  hdPage	*p;			/* Current page */
  const char	*debug;			/* HTMLDOC_DEBUG environment variable */


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
  render_startx   = 0.0f;

 /*
  * Output the page header...
  */

  out->puts("q\n");
  write_background(out, page);

  if (p->duplex && (page & 1))
    out->printf("1 0 0 1 %d %d cm\n", p->right, p->bottom);
  else
    out->printf("1 0 0 1 %d %d cm\n", p->left, p->bottom);

 /*
  * Render all graphics elements...
  */

  box[0] = -1.0f;
  box[1] = -1.0f;
  box[2] = -1.0f;

  for (r = p->start; r != NULL; r = r->next)
    switch (r->type)
    {
      case HD_RENDER_IMAGE :
          if (r->width > 0.01f && r->height > 0.01f)
            write_image(out, r);
          break;

      case HD_RENDER_BOX :
	  if (r->height == 0.0)
	  {
            if (box[0] != r->data.box[0] ||
		box[1] != r->data.box[1] ||
		box[2] != r->data.box[2])
            {
              box[0] = r->data.box[0];
	      box[1] = r->data.box[1];
	      box[2] = r->data.box[2];

	      if (OutputColor &&
	          (box[0] != box[1] || box[1] != box[2] || box[0] != box[2]))
        	out->printf("%g %g %g RG\n", box[0], box[1], box[2]);
              else
        	out->printf("%g G\n",
		             box[0] * 0.31f + box[1] * 0.61f + box[2] * 0.08f);
            }

            out->printf("%.1f %.1f m %.1f %.1f l S\n",
                	 r->x, r->y, r->x + r->width, r->y);
	  }
	  else
	  {
            set_color(out, r->data.box);
            out->printf("%g %g %g %g re f\n",
                	 r->x, r->y, r->width, r->height);
	  }
	  break;
    }

 /*
  * Render all text elements, freeing used memory as we go...
  */

  out->puts("BT\n");

  render_x        = -1.0f;
  render_y        = -1.0f;
  render_startx   = 0.0f;
  render_spacing  = -1.0f;
  render_typeface = -1;
  render_style    = -1;
  render_size     = -1;

  for (r = p->start, next = NULL; r != NULL; r = next)
  {
    if (r->type == HD_RENDER_TEXT)
      write_text(out, r);

    next = r->next;
    free(r);
  }

  p->start = NULL;

  out->puts("ET\n");

  if ((debug = getenv("HTMLDOC_DEBUG")) != NULL && strstr(debug, "margin"))
  {
    // Show printable area...
    out->printf("1 0 1 RG 0 0 %d %d re S\n", p->width - p->right - p->left,
		p->length - p->top - p->bottom);
  }

 /*
  * Output the page trailer...
  */

  out->puts("Q\n");
}


#ifdef DEBUG_TOC
static void
pdf_text_contents(hdFile *out, hdTree *toc, int indent)
{
  static const char *spaces = "                                "
                              "                                ";

  if (indent > 16)
    indent = 16;

  while (toc)
  {
    out->printf("%% %s<%s>", spaces + 64 - 4 * indent,
                _htmlStyleSheet->get_element(toc->element));

    switch (toc->element)
    {
      case HD_ELEMENT_A :
          hdTree *temp;

          for (temp = toc->child; temp; temp = temp->next)
	    out->puts((char *)temp->data);
          break;

      default :
          out->puts("\n");
	  pdf_text_contents(out, toc->child, indent + 1);
	  out->printf("%% %s", spaces + 64 - 4 * indent);
          break;
    }

    out->printf("</%s>\n", _htmlStyleSheet->get_element(toc->element));

    toc = toc->next;
  }
}
#endif // DEBUG_TOC


/*
 * 'pdf_write_contents()' - Write the table of contents as outline records to
 *                          a PDF file.
 */

static void
pdf_write_contents(hdFile *out,		/* I - Output file */
                   hdTree *toc,		/* I - Table of contents tree */
                   int    parent,	/* I - Parent outline object */
                   int    prev,		/* I - Previous outline object */
                   int    next,		/* I - Next outline object */
                   int    *heading)	/* IO - Current heading # */
{
  int		i,			/* Looping var */
		thisobj,		/* This object */
		entry,			/* TOC entry object */
		count;			/* Number of entries at this level */
  hdChar	*text;			/* Entry text */
  hdTree	*temp;			/* Looping var */
  int		*entry_counts,		/* Number of sub-entries for this entry */
		*entry_objects;		/* Objects for each entry */
  hdTree	**entries;		/* Pointers to each entry */
  float		x, y;			/* Position of link */


 /*
  * Make an object for this entry...
  */

  if (toc == NULL)
  {
   /*
    * This is for the Table of Contents page...
    */

    thisobj = pdf_start_object(out);

    out->printf("/Parent %d 0 R", parent);

    out->puts("/Title");
    write_utf16(out, (hdChar *)TocTitle);

    x = 0.0f;
    y = PagePrintLength + PageBottom;
    pspdf_transform_coords(pages + chapter_starts[0], x, y);

    out->printf("/Dest[%d 0 R/XYZ %.0f %.0f 0]",
                pages_object + 2 * chapter_outstarts[0] + 1, x, y);

    if (prev > 0)
      out->printf("/Prev %d 0 R", prev);

    if (next > 0)
      out->printf("/Next %d 0 R", next);

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

  if ((entries = (hdTree **)calloc(sizeof(hdTree *), num_headings + 1)) == NULL)
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

  if (toc->element == HD_ELEMENT_B && toc->next &&
      toc->next->element == HD_ELEMENT_UL)
    temp = toc->next->child;
  else if (toc->element == HD_ELEMENT_LI && toc->last_child &&
           toc->last_child->element == HD_ELEMENT_UL)
    temp = toc->last_child->child;
  else
    temp = toc->child;

  for (; temp && count <= num_headings; temp = temp->next)
  {
    if (temp->element == HD_ELEMENT_B)
    {
      entries[count]       = temp;
      entry_objects[count] = entry;

      if (temp->next && temp->next->element == HD_ELEMENT_UL)
        entry_counts[count] = pdf_count_headings(temp->next->child);
      else
        entry_counts[count] = 0;

      entry += entry_counts[count] + 1;
      count ++;
    }
    else if (temp->element == HD_ELEMENT_LI)
    {
      entries[count]       = temp;
      entry_objects[count] = entry;

      if (temp->last_child && temp->last_child->element == HD_ELEMENT_UL)
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
    out->printf("/Parent %d 0 R", parent);

  if (count > 0)
    out->printf("/Count %d/First %d 0 R/Last %d 0 R",
                parent == 0 ? count : -count, entry_objects[0],
		entry_objects[count - 1]);

  if (parent > 0 && toc->child && toc->child->element == HD_ELEMENT_A)
  {
    if ((text = htmlGetText(toc->child->child)) != NULL)
    {
      out->puts("/Title");
      write_utf16(out, text);
      free(text);
    }

    i = heading_pages[*heading];
    x = 0.0f;
    y = heading_tops[*heading] + pages[i].bottom;
    pspdf_transform_coords(pages + i, x, y);

    out->printf("/Dest[%d 0 R/XYZ %.0f %.0f 0]",
            pages_object + 2 * pages[i].outpage + 1, x, y);

    (*heading) ++;
  }

  if (prev > 0)
    out->printf("/Prev %d 0 R", prev);

  if (next > 0)
    out->printf("/Next %d 0 R", next);

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
pdf_write_files(hdFile *out,		// I - Output file
                hdTree *doc)		// I - Document tree
{
  int		i,			// Looping var
		num_files,		// Number of FILE elements
		entry,			// Entry object
		alloc_text;		// Allocated text?
  hdChar	*text;			// Entry text
  hdTree	*temp;			// Current node
  hdRenderLink	*link;			// Link to file...
  float		x, y;			// Position of link


  // Figure out the number of (top-level) files in the document...
  for (num_files = 0, temp = doc; temp; temp = temp->next)
    if (temp->element == HD_ELEMENT_FILE)
      num_files ++;

  if (!num_files)
  {
    // No files to outline...
    outline_object = 0;
  
    return;
  }

  // Write the outline dictionary...
  outline_object = pdf_start_object(out);

  out->printf("/Count %d/First %d 0 R/Last %d 0 R", num_files,
              outline_object + 1, outline_object + num_files);

  pdf_end_object(out);

  // Now write the outline items...
  for (i = 0, temp = doc; temp; temp = temp->next)
    if (temp->element == HD_ELEMENT_FILE)
    {
      alloc_text = 0;

      if ((text = get_title(temp->child)) != NULL)
        alloc_text = 1;
      else if ((text = htmlGetAttr(temp, "_HD_FILENAME")) == NULL)
        text = (hdChar *)"Unknown";

      entry = pdf_start_object(out);
      
      out->printf("/Parent %d 0 R", outline_object);

      out->puts("/Title");
      write_utf16(out, text);
      if (alloc_text)
        free(text);

      if ((link = find_link(htmlGetAttr(temp, "_HD_FILENAME"))) != NULL)
      {
	x = 0.0f;
	y = link->top + pages[link->page].bottom;
	pspdf_transform_coords(pages + link->page, x, y);

	out->printf("/Dest[%d 0 R/XYZ %.0f %.0f 0]",
        	pages_object + 2 * pages[link->page].outpage + 1, x, y);
      }

      if (i > 0)
        out->printf("/Prev %d 0 R", outline_object + i);
 
      if (i < (num_files - 1))
        out->printf("/Next %d 0 R", outline_object + i + 2);

      pdf_end_object(out);

      i ++;
    }
}


/*
 * 'pdf_count_headings()' - Count the number of headings under this TOC
 *                          entry.
 */

static int				/* O - Number of headings found */
pdf_count_headings(hdTree *toc)		/* I - TOC entry */
{
  int	nheadings;			/* Number of headings */


  for (nheadings = 0; toc != NULL; toc = toc->next)
  {
    if (toc->element == HD_ELEMENT_A)
      nheadings ++;
    if (toc->child != NULL)
      nheadings += pdf_count_headings(toc->child);
  }

  return (nheadings);
}


/*
 * PDF object state variables...
 */

static size_t	pdf_stream_length = 0;
static size_t	pdf_stream_start = 0;
static int	pdf_object_type = 0;


/*
 * 'pdf_start_object()' - Start a new PDF object...
 */

static int				// O - Object number
pdf_start_object(hdFile *out,		// I - File to write to
                 int    array)		// I - 1 = array, 0 = dictionary
{
  int	*temp;				// Temporary integer pointer


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

  objects[num_objects] = out->pos();
  out->printf("%d 0 obj", num_objects);

  pdf_object_type = array;

  out->puts(pdf_object_type ? "[" : "<<");

  return (num_objects);
}


//
// 'pdf_start_stream()' - Start a new PDF stream...
//

static hdFile *
pdf_start_stream(hdFile  *out,		// I  - File to write to
                 hdArray &filters,	// IO - Array of filters
		 int     width,		// I  - Width for JPEG filter, 0 otherwise
		 int     height,	// I  - Height for JPEG filter, 0 otherwise
		 int     depth)		// I  - Depth for JPEG filter, 0 otherwise
{
  hdFile	*filter;		// Top-most filter


  if (Encryption)
  {
    filter = encrypt_init(out);
    filters.add(filter);
  }
  else
    filter = out;

  if (OutputJPEG && width > 0 && height > 0 && depth > 0)
  {
    if (Compression)
    {
      out->puts("/Filter[/FlateDecode/DCTDecode]");

      filter = new hdFlateFilter(filter, Compression);
      filters.add(filter);
    }
    else
      out->puts("/Filter/DCTDecode");

    filter = new hdJPEGFilter(filter, width, height, depth, OutputJPEG);
    filters.add(filter);
  }
  else if (Compression)
  {
    out->puts("/Filter/FlateDecode");
    filter = new hdFlateFilter(filter, Compression);
    filters.add(filter);
  }

  // Write the "/Length " string, get the position, and then write 10
  // zeroes to cover the maximum size of a stream.
  out->puts("/Length ");
  pdf_stream_length = out->pos();
  out->puts("0000000000>>stream\n");
  pdf_stream_start = out->pos();

  return (filter);
}


/*
 * 'pdf_end_object()' - End a PDF object...
 */

static void
pdf_end_object(hdFile *out)		// I - File to write to
{
  out->puts(pdf_object_type ? "]" : ">>");
  out->puts("endobj\n");
}


//
// 'pdf_end_stream()' - End a PDF stream object.
//

static void
pdf_end_stream(hdFile  *out,		// I  - Output file
               hdArray &filters)	// IO - Array of filters
{
  hdFile	*filter;		// Current filter
  size_t	length;			// Total length of stream


  // Close out all filters, starting with the last one...
  while ((filter = (hdFile *)filters.last()) != NULL)
  {
    filters.remove(filter);
    delete filter;
  }

  // Update the length of the object...
  length = out->pos() - pdf_stream_start;

  out->seek(pdf_stream_length, SEEK_SET);
  out->printf("%-10ld", (long)length);
  out->seek(0, SEEK_END);
  pdf_stream_start = 0;

  out->puts("endstream\n");
  out->puts("endobj\n");
}


/*
 * 'pdf_write_links()' - Write annotation link objects for each page in the
 *                       document.
 */

static void
pdf_write_links(hdFile *out)		/* I - Output file */
{
  int		i,			/* Looping var */
		outpage,		/* Current page */
		lobj,			/* Current link */
		num_lobjs,		/* Number of links on this page */
		alloc_lobjs,		/* Number of links to allocate */
		*lobjs;			/* Link objects */
  float		x, y;			/* Position of last link */
  hdRender	*r,			/* Current render primitive */
		*rlast,			/* Last render link primitive */
		*rprev;			/* Previous render primitive */
  hdRenderLink	*link;			/* Local link */
  hdPage	*p;			/* Current page */
  hdOutPage	*op;			/* Current output page */


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
	if (r->type == HD_RENDER_LINK)
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
	if (r->type == HD_RENDER_LINK)
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
	if (r->type == HD_RENDER_LINK)
	{
          if ((link = find_link(r->data.link)) != NULL)
	  {
	   /*
            * Local link...
            */
	    float x1, y1, x2, y2;

            lobjs[num_lobjs ++] = pdf_start_object(out);

            out->puts("/Subtype/Link");

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
            out->printf("/Rect[%.1f %.1f %.1f %.1f]", x1, y1, x2, y2);

            out->puts("/Border[0 0 0]");

            x1 = 0.0f;
	    y1 = link->top + pages[link->page].bottom;
            pspdf_transform_coords(pages + link->page, x1, y1);
	    out->printf("/Dest[%d 0 R/XYZ %.0f %.0f 0]",
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
        	hdFile::scheme((char *)r->data.link) == NULL)
	    {
	      char ext[256];		// Extension

#ifdef WIN32
              if (!strcasecmp(hdFile::extension((char *)r->data.link, ext,
	                                        sizeof(ext)), "pdf"))
#else
              if (!strcmp(hdFile::extension((char *)r->data.link, ext,
					    sizeof(ext)), "pdf"))
#endif // WIN32
              {
	       /*
		* Link to external PDF file...
		*/

        	out->puts("/S/GoToR");
        	out->puts("/D[0/XYZ null null 0]");
        	out->puts("/F");
		write_string(out, r->data.link);
              }
	      else
              {
	       /*
		* Link to external filename...
		*/

        	out->puts("/S/Launch");
        	out->puts("/F");
		write_string(out, r->data.link);

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

              out->puts("/S/URI");
              out->puts("/URI");
	      write_string(out, r->data.link);
	    }

            pdf_end_object(out);

            lobjs[num_lobjs ++] = pdf_start_object(out);

            out->puts("/Subtype/Link");
            if (PageDuplex && (outpage & 1))
              out->printf("/Rect[%.1f %.1f %.1f %.1f]",
                      r->x + PageRight, r->y + PageBottom,
                      r->x + r->width + PageRight, r->y + r->height + PageBottom);
            else
              out->printf("/Rect[%.1f %.1f %.1f %.1f]",
                      r->x + PageLeft, r->y + PageBottom - 2,
                      r->x + r->width + PageLeft, r->y + r->height + PageBottom);
            out->puts("/Border[0 0 0]");
	    out->printf("/A %d 0 R", num_objects - 1);
            pdf_end_object(out);
	  }
	}
    }

    if (num_lobjs > 0)
    {
      outpages[outpage].annot_object = pdf_start_object(out, 1);

      for (lobj = 0; lobj < num_lobjs; lobj ++)
        out->printf("%d 0 R%s", lobjs[lobj],
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
pdf_write_names(hdFile *out)		/* I - Output file */
{
  int		i;			/* Looping var */
  hdChar	*s;			/* Current character in name */
  hdRenderLink	*link;			/* Local link */


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
  out->printf("/Dests %d 0 R", num_objects + 1);
  pdf_end_object(out);

 /*
  * Write the name tree child list...
  */

  pdf_start_object(out);
  out->printf("/Kids[%d 0 R]", num_objects + 1);
  pdf_end_object(out);

 /*
  * Write the leaf node for the name tree...
  */

  pdf_start_object(out);

  out->puts("/Limits[");
  write_string(out, links[0].name);
  write_string(out, links[num_links - 1].name);
  out->puts("]");

  out->puts("/Names[");
  for (i = 1, link = links; i <= num_links; i ++, link ++)
  {
    write_string(out, link->name);
    out->printf("%d 0 R", num_objects + i);
  }
  out->puts("]");

  pdf_end_object(out);

  for (i = num_links, link = links; i > 0; i --, link ++)
  {
    pdf_start_object(out);
    float x, y;

    x = 0.0f;
    y = link->top + pages[link->page].bottom;
    pspdf_transform_coords(pages + link->page, x, y);
    out->printf("/D[%d 0 R/XYZ %.0f %.0f 0]", 
            pages_object + 2 * pages[link->page].outpage + 1, x, y);
    pdf_end_object(out);
  }
}


/*
 * 'render_contents()' - Render a single heading.
 */

static void
render_contents(hdTree   *t,		/* I - Tree to parse */
                hdMargin *margins,	/* I  - Margins */
                float    *y,		/* IO - Y position */
                int      *page,		/* IO - Page # */
	        int      heading,	/* I - Heading # */
	        hdTree   *chap)		/* I - Chapter heading */
{
  float		x,
		width,
		numberwidth,
		height,
		rgb[3];
  int		hpage;
  hdChar	number[1024],
		*nptr,
		*link;
  hdTree	*flat,
		*temp,
		*next;
  hdRender	*r;
  float		dot_width;


  DEBUG_printf(("render_contents(t=%p(%s), margins=(%.1f, %.1f, %.1f, %.1f, %d), y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, _htmlStyleSheet->get_element(t->element),
		margins->left(), margins->right(), margins->bottom(),
		margins->top(), margins->level(), *y, *page, heading, chap));

  if (!t)
    return;

 /*
  * Put the text...
  */

  DEBUG_printf(("    t->style->font_size=%.1f, line_height=%.1f\n",
                t->style->font_size, t->style->line_height));

  flat = flatten_tree(t->child->child);

  for (height = t->style->line_height, temp = flat; temp != NULL; temp = temp->next)
    if (temp->style->line_height > height)
      height = temp->style->line_height;

  DEBUG_printf(("    height=%.1f\n", height));

  x = margins->left();

  *y -= height;

 /*
  * Get the width of the page number, leave room for three dots...
  */

  dot_width = t->style->get_width((hdChar *)".");

  if (heading >= 0 && heading < num_headings)
  {
    hpage       = heading_pages[heading];
    numberwidth = t->style->get_width((hdChar *)pages[hpage].page_text) +
	          3.0f * dot_width;
  }
  else
  {
    hpage       = 0;
    numberwidth = 0.0f;
  }

  for (temp = flat; temp != NULL; temp = next)
  {
    rgb[0] = temp->style->color[0] / 255.0f;
    rgb[1] = temp->style->color[1] / 255.0f;
    rgb[2] = temp->style->color[2] / 255.0f;

    if ((x + temp->width) >= (margins->right() - numberwidth))
    {
     /*
      * Too wide to fit, continue on the next line
      */

      *y -= t->style->line_height;
      x  = margins->left();
    }

    if (*y < margins->bottom0())
    {
      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);

      // Find/create private style for table-of-contents title...
      hdStyleSelector selector(HD_ELEMENT_H1, NULL, NULL, "_HD_TOC");
      hdStyle *style = _htmlStyleSheet->find_style(1, &selector, true);

      if (!style)
      {
	hdStyle *body_style = _htmlStyleSheet->find_style(HD_ELEMENT_BODY,
                                                          "HD_TOC");
	hdStyle *h1_style = _htmlStyleSheet->find_style(HD_ELEMENT_H1,
	                                                "HD_TOC");
        style = new hdStyle(1, &selector, body_style);
	style->inherit(h1_style);
	_htmlStyleSheet->add_style(style);
      }

      width = style->get_width((hdChar *)TocTitle);
      *y = margins->top() - style->line_height;
      x  = margins->left0() + 0.5f * (margins->width0() - width);
      r = new_render(*page, HD_RENDER_TEXT, x, *y, width, style->font_size,
                     TocTitle);
      r->data.text.typeface = style->font->typeface;
      r->data.text.style    = style->font->style;
      r->data.text.size     = style->font_size;
      r->data.text.rgb[0]   = style->color[0] / 255.0f;
      r->data.text.rgb[1]   = style->color[1] / 255.0f;
      r->data.text.rgb[2]   = style->color[2] / 255.0f;

      *y -= style->line_height;

      x = margins->left();

      if (chap != t)
      {
        *y += height;
	margins->adjust_left(-x);
        render_contents(chap, margins, y, page, -1, 0);
	margins->adjust_left(x);
	*y -= t->style->line_height;
      }
    }

    if (temp->link != NULL)
    {
      link = htmlGetAttr(temp->link, "HREF");

     /*
      * Add a page link...
      */

      if (hdFile::scheme((char *)link) == NULL &&
	  hdFile::target((char *)link) != NULL)
	link = (hdChar *)hdFile::target((char *)link) - 1; // Include # sign

      new_render(*page, HD_RENDER_LINK, x, *y, temp->width,
	         temp->height, link);

      if (PSLevel == 0 && Links && LinkStyle)
	new_render(*page, HD_RENDER_BOX, x, *y - 1, temp->width, 0, rgb);
    }

    switch (temp->element)
    {
      case HD_ELEMENT_A :
          if ((link = htmlGetAttr(temp, "NAME")) != NULL)
          {
           /*
            * Add a target link...
            */

            add_link(link, *page, (int)(*y + height));
          }
          break;

      case HD_ELEMENT_NONE :
          if (temp->data == NULL)
            break;

	  if (temp->style->text_decoration == HD_TEXT_DECORATION_UNDERLINE)
	    new_render(*page, HD_RENDER_BOX, x, *y - 1, temp->width, 0, rgb);

	  if (temp->style->text_decoration == HD_TEXT_DECORATION_LINE_THROUGH)
	    new_render(*page, HD_RENDER_BOX, x, *y + temp->height * 0.25f,
		       temp->width, 0, rgb);

          r = new_render(*page, HD_RENDER_TEXT, x, *y, 0, 0, temp->data);
          r->data.text.typeface = temp->style->font->typeface;
          r->data.text.style    = temp->style->font->style;
          r->data.text.size     = temp->style->font_size;
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

          if (temp->style->vertical_align == HD_VERTICAL_ALIGN_SUPER)
            r->y += height - temp->height;
          else if (temp->style->vertical_align == HD_VERTICAL_ALIGN_SUB)
            r->y -= temp->style->line_height - temp->height;
	  break;

      case HD_ELEMENT_IMG :
	  update_image_size(temp);
	  new_render(*page, HD_RENDER_IMAGE, x, *y, temp->width, temp->height,
		     hdImage::find((char *)htmlGetAttr(temp, "_HD_SRC")));
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
         nptr < (number + sizeof(number) - 1) && width < margins->right();
	 width += dot_width)
      *nptr++ = '.';
    nptr --;

    strlcpy((char *)nptr, pages[hpage].page_text,
            sizeof(number) - (nptr - number));

    r = new_render(*page, HD_RENDER_TEXT, margins->right() - width + x, *y, 0, 0, number);
    r->data.text.typeface = t->style->font->typeface;
    r->data.text.style    = t->style->font->style;
    r->data.text.size     = t->style->font_size;
    memcpy(r->data.text.rgb, rgb, sizeof(rgb));
  }
}


/*
 * 'count_headings()' - Count the number of headings in the TOC.
 */

static int
count_headings(hdTree *t)		// I - Tree to count
{
  int	count;				// Number of headings...


  count = 0;

  while (t != NULL)
  {
    switch (t->element)
    {
      case HD_ELEMENT_B :
      case HD_ELEMENT_LI :
          count ++;
	  if (t->last_child && t->last_child->element == HD_ELEMENT_UL)
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
parse_contents(hdTree   *t,		/* I - Tree to parse */
               hdMargin *margins,	/* I - Margins */
               float    *y,		/* IO - Y position */
               int      *page,		/* IO - Page # */
               int      *heading,	/* IO - Heading # */
	       hdTree   *chap)		/* I - Chapter heading */
{
  DEBUG_printf(("parse_contents(t=%p(%s), margins=(%.1f, %.1f, %.1f, %.1f, %d), y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, _htmlStyleSheet->get_element(t->element),
		margins->left(), margins->right(), margins->bottom(),
		margins->top(), margins->level(), *y, *page, *heading, chap));

  while (t != NULL)
  {
    DEBUG_printf(("    t->element=%s, t->style->font_size=%.1f, "
                  "line_height=%.1f\n",
        	  _htmlStyleSheet->get_element(t->element),
		  t->style ? t->style->font_size : -1.0,
		  t->style ? t->style->line_height : -1.0));

    switch (t->element)
    {
      case HD_ELEMENT_B :	/* Top-level TOC */
          if (t->prev != NULL)	/* Advance one line prior to top-levels... */
            *y -= t->style->line_height;

          if (*y < (margins->bottom0() + t->style->line_height * 3))
	    *y = 0; // Force page break

          chap = t;

      case HD_ELEMENT_LI :	/* Lower-level TOC */
          DEBUG_printf(("parse_contents: heading=%d, page = %d\n", *heading,
                        heading_pages[*heading]));

         /*
          * Put the text unless the author has flagged it otherwise...
          */

          if (htmlGetAttr(t, "_HD_OMIT_TOC") == NULL)
	  {
            render_contents(t, margins, y, page, *heading, chap);

           /*
	    * Update current headings for header/footer strings in TOC.
	    */

	    check_pages(*page);

	    if (t->element == HD_ELEMENT_B &&
		pages[*page].chapter == pages[*page - 1].chapter)
	      pages[*page].chapter = htmlGetText(t->child->child);

	    if (pages[*page].heading == pages[*page - 1].heading)
	      pages[*page].heading = htmlGetText(t->child->child);

           /*
            * Next heading...
            */

            (*heading) ++;

            if (t->last_child->element == HD_ELEMENT_UL)
              parse_contents(t->last_child, margins, y, page, heading, chap);
          }
	  else if (t->next != NULL && t->next->element == HD_ELEMENT_UL)
	  {
	   /*
	    * Skip children of omitted heading...
	    */

	    t = t->next;

	    (*heading) += count_headings(t->child) + 1;
	  }
          break;

      case HD_ELEMENT_UL :
	  margins->adjust_left(t->style->margin[HD_POS_LEFT]);

          parse_contents(t->child, margins, y, page, heading, chap);

	  margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
          break;

      default :
          if (t->child)
            parse_contents(t->child, margins, y, page, heading, chap);
          break;
    }

    t = t->next;
  }
}


/*
 * 'parse_doc()' - Parse a document tree and produce rendering list output.
 */

static void
parse_doc(hdTree   *t,			/* I - Tree to parse */
          hdMargin *margins,		/* I - Left margin */
          float    *x,			/* IO - X position */
          float    *y,			/* IO - Y position */
          int      *page,		/* IO - Page # */
	  hdTree   *cpara,		/* I - Current paragraph */
	  float    *needspace)		/* I - Needed whitespace before this */
{
  int		i;			/* Looping var */
  hdTree	*para,			/* Phoney paragraph tree entry */
		*temp;			/* Paragraph entry */
  hdTreeAttr	*var;			/* Attribute entry */
  hdChar	*name;			/* ID name */
  float		width,			/* Width of horizontal rule */
		height,			/* Height of rule */
		rgb[3];			/* RGB color of rule */


  DEBUG_printf(("parse_doc(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d, cpara=%p, needspace=%g\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page, cpara, *needspace));
  DEBUG_printf(("    title_page = %d, chapter = %d\n", title_page, chapter));

  if (cpara == NULL)
  {
    para = htmlNewTree(NULL, HD_ELEMENT_P, NULL);
    htmlUpdateStyle(para, NULL);

//    printf("paragraph style: margin-top=%g(%s), margin-bottom=%g(%s)\n",
//           para->style->margin[HD_POS_TOP], para->style->margin_rel[HD_POS_TOP],
//	   para->style->margin[HD_POS_BOTTOM],
//	   para->style->margin_rel[HD_POS_BOTTOM]);
  }
  else
    para = cpara;

  while (t != NULL)
  {
    if (((t->element == HD_ELEMENT_H1 && OutputType == HD_OUTPUT_BOOK) ||
         (t->element == HD_ELEMENT_FILE && OutputType == HD_OUTPUT_WEBPAGES)) &&
	!title_page)
    {
      // New page on H1 in book mode or file in webpage mode...
      if (para->child != NULL && chapter > 0)
      {
        parse_paragraph(para, margins, x, y, page, needspace);
        htmlDeleteTree(para->child);
        para->child = para->last_child = NULL;
      }

#if 0
      if ((chapter > 0 && OutputType == HD_OUTPUT_BOOK) ||
          ((*page > 1 || *y < margins->top()) &&
	   OutputType == HD_OUTPUT_WEBPAGES))
#endif //0
      if (*page > 1 || *y < margins->top())
      {
        if (*y < margins->top())
          (*page) ++;

        if (PageDuplex && (*page & 1))
          (*page) ++;

	margins->clear(margins->top(), *page);

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

      *y         = margins->top();
      *x         = margins->left();
      *needspace = 0;
    }

    if ((name = htmlGetAttr(t, "ID")) != NULL)
    {
     /*
      * Add a link target using the ID=name variable...
      */

      add_link(name, *page, (int)*y);
    }
    else if (t->element == HD_ELEMENT_FILE)
    {
     /*
      * Add a file link...
      */

      hdChar	newname[256],	/* New filename */
		*sep;		/* "?" separator in links */


      // Strip any trailing HTTP GET data stuff...
      strlcpy((char *)newname, (char *)htmlGetAttr(t, "_HD_FILENAME"),
              sizeof(newname));

      if ((sep = (hdChar *)strchr((char *)newname, '?')) != NULL)
        *sep = '\0';

      // Add the link
      add_link(newname, *page, (int)*y);
    }

    if (chapter == 0 && !title_page)
    {
      // Need to handle page comments before the first heading...
      if (t->element == HD_ELEMENT_COMMENT)
        parse_comment(t, margins, x, y, page, para, needspace);

      if (t->child != NULL)
        parse_doc(t->child, margins, x, y, page, para, needspace);

      t = t->next;
      continue;
    }

    // Check for some basic stylesheet stuff...
    if (t->style &&
        t->style->page_break_before != HD_PAGE_BREAK_AVOID &&
        t->style->page_break_before != HD_PAGE_BREAK_AUTO)
    {
      // Advance to the next page...
      (*page) ++;
      *x         = margins->left();
      *y         = margins->top();
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (PageDuplex && ((*page) & 1) &&
          t->style->page_break_before == HD_PAGE_BREAK_LEFT)
	(*page) ++;
      else if (PageDuplex && !((*page) & 1) &&
               t->style->page_break_before == HD_PAGE_BREAK_RIGHT)
	(*page) ++;

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    // Process the markup...
    switch (t->element)
    {
      case HD_ELEMENT_IMG :
          update_image_size(t);
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_BR :
          if (para->child == NULL)
          {
	    if (!t->parent)
	      para->style = _htmlStyleSheet->find_style(HD_ELEMENT_BODY);
	    else
	      para->style = t->parent->style;
          }

	  // Skip heading whitespace...
          if (para->child == NULL && t->element == HD_ELEMENT_NONE &&
	      t->data != NULL && strcmp((char *)t->data, " ") == 0)
	    break;

          if ((temp = htmlAddTree(para, t->element, t->data)) != NULL)
          {
	    temp->link   = t->link;
            temp->width  = t->width;
            temp->height = t->height;
            temp->style  = t->style;

            for (i = 0, var = t->attrs; i < t->nattrs; i ++, var ++)
              htmlSetAttr(temp, var->name, var->value);
          }
          break;

      case HD_ELEMENT_TABLE :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_table(t, margins, x, y, page, needspace);
          break;

      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_H7 :
      case HD_ELEMENT_H8 :
      case HD_ELEMENT_H9 :
      case HD_ELEMENT_H10 :
      case HD_ELEMENT_H11 :
      case HD_ELEMENT_H12 :
      case HD_ELEMENT_H13 :
      case HD_ELEMENT_H14 :
      case HD_ELEMENT_H15 :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_heading(t, margins, x, y, page, needspace);
          break;

      case HD_ELEMENT_BLOCKQUOTE :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          margins->adjust_left(t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

          parse_doc(t->child, margins, x, y, page, NULL, needspace);

          margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);

          *x = margins->left();
          break;

      case HD_ELEMENT_CENTER :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, margins, x, y, page, NULL, needspace);

          *x = margins->left();
          break;

      case HD_ELEMENT_P :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, margins, x, y, page, NULL, needspace);

          *x = margins->left();
          break;

      case HD_ELEMENT_DIV :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, margins, x, y, page, NULL, needspace);
          break;

      case HD_ELEMENT_PRE :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_pre(t, margins, x, y, page, needspace);

          *x = margins->left();
          break;

      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_OL :
          init_list(t);
      case HD_ELEMENT_DL :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          t->style->update(_htmlStyleSheet);
          margins->adjust_left(t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

          *x = margins->left();

          parse_doc(t->child, margins, x, y, page, para, needspace);

          margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);

          if ((t->element == HD_ELEMENT_DIR ||
	       t->element == HD_ELEMENT_MENU ||
	       t->element == HD_ELEMENT_UL ||
	       t->element == HD_ELEMENT_OL) && list_indent > 0)
	    list_indent --;
          break;

      case HD_ELEMENT_LI :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          margins->adjust_left(t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

          parse_list(t, margins, x, y, page, needspace);

          margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);
          *x = margins->left();
          break;

      case HD_ELEMENT_DT :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          margins->adjust_left(t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);
          *x = margins->left();

          parse_doc(t->child, margins, x, y, page, NULL, needspace);

          margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);
          *x = margins->left();
          break;

      case HD_ELEMENT_DD :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          margins->adjust_left(t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);
          *x = margins->left();

          parse_doc(t->child, margins, x, y, page, NULL, needspace);

          margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
	  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);
          *x = margins->left();
          break;

      case HD_ELEMENT_HR :
          if (para->child != NULL)
          {
            parse_paragraph(para, margins, x, y, page, needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

	 /*
	  * Generate a horizontal rule...
	  */

          if ((name = htmlGetAttr(t, "WIDTH")) == NULL)
	    width = margins->width();
	  else
	  {
	    if (strchr((char *)name, '%') != NULL)
	      width = atoi((char *)name) * (margins->width()) / 100;
	    else
              width = atoi((char *)name) * PagePrintWidth / _htmlStyleSheet->browser_width;
          }

          if ((name = htmlGetAttr(t, "SIZE")) == NULL)
	    height = 2;
	  else
	    height = atoi((char *)name) * PagePrintWidth / _htmlStyleSheet->browser_width;

          switch (t->style->text_align)
	  {
	    case HD_TEXT_ALIGN_LEFT :
	    case HD_TEXT_ALIGN_JUSTIFY :
	    case HD_TEXT_ALIGN_INHERIT :
	        *x = margins->left();
		break;
	    case HD_TEXT_ALIGN_CENTER :
	        *x = margins->left() + (margins->width() - width) * 0.5f;
		break;
	    case HD_TEXT_ALIGN_RIGHT :
	        *x = margins->right() - width;
		break;
	  }

          if (*y < (margins->bottom0() + height + t->style->line_height))
	  {
	   /*
	    * Won't fit on this page...
	    */

            (*page) ++;
	    if (Verbosity)
	      progress_show("Formatting page %d", *page);

            margins->clear(margins->top(), *page);

           *y = margins->top();
          }

          (*y)   -= height + t->style->line_height;
          rgb[0] = t->style->color[0] / 255.0f;
          rgb[1] = t->style->color[1] / 255.0f;
          rgb[2] = t->style->color[2] / 255.0f;

          new_render(*page, HD_RENDER_BOX, *x, *y + t->style->line_height * 0.5,
	             width, height, rgb);

          *x         = margins->left();
	  *needspace = t->style->margin[HD_POS_BOTTOM];
          break;

      case HD_ELEMENT_COMMENT :
          // Check comments for commands...
          parse_comment(t, margins, x, y, page, para, needspace);
          break;

      case HD_ELEMENT_HEAD : // Ignore document HEAD section
      case HD_ELEMENT_TITLE : // Ignore title and meta stuff
      case HD_ELEMENT_META :
      case HD_ELEMENT_SCRIPT : // Ignore script stuff
      case HD_ELEMENT_INPUT : // Ignore form stuff
      case HD_ELEMENT_SELECT :
      case HD_ELEMENT_OPTION :
      case HD_ELEMENT_TEXTAREA :
          break;

      case HD_ELEMENT_STYLE :
          break;

      case HD_ELEMENT_A :
          if (htmlGetAttr(t, "NAME") != NULL)
	  {
	   /*
	    * Add this named destination to the paragraph tree...
	    */

            if (para->child == NULL)
              para->style = t->style;

            if ((temp = htmlAddTree(para, t->element, t->data)) != NULL)
            {
	      temp->link   = t->link;
              temp->width  = t->width;
              temp->height = t->height;
              temp->style  = t->style;

              for (i = 0, var = t->attrs; i < t->nattrs; i ++, var ++)
        	htmlSetAttr(temp, var->name, var->value);
            }
	  }

      default :
	  if (t->child != NULL)
            parse_doc(t->child, margins, x, y, page, para, needspace);
          break;
    }


    // Check for some basic stylesheet stuff...
    if (t->style->page_break_after != HD_PAGE_BREAK_AVOID &&
        t->style->page_break_after != HD_PAGE_BREAK_AUTO)
    {
      // Advance to the next page...
      (*page) ++;
      *x         = margins->left();
      *y         = margins->top();
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (PageDuplex && ((*page) & 1) &&
          t->style->page_break_after == HD_PAGE_BREAK_RIGHT)
	(*page) ++;
      else if (PageDuplex && !((*page) & 1) &&
          t->style->page_break_after == HD_PAGE_BREAK_LEFT)
	(*page) ++;

      margins->clear(margins->top(), *page);

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    // Move to the next node...
    t = t->next;
  }

  if (para->child != NULL && cpara != para)
  {
    parse_paragraph(para, margins, x, y, page, needspace);
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
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
parse_heading(hdTree   *t,		/* I  - Tree to parse */
              hdMargin *margins,	/* I  - Margins */
              float    *x,		/* IO - X position */
              float    *y,		/* IO - Y position */
              int      *page,		/* IO - Page # */
              float    *needspace)	/* IO - Needed whitespace? */
{
  int	*temp;				// Temporary integer array pointer


  DEBUG_printf(("parse_heading(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d, needspace=%g\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page, *needspace));

  if (((t->element - HD_ELEMENT_H1) < TocLevels || TocLevels == 0) &&
      !title_page)
    current_heading = t->child;

  if (*y < (5 * t->style->line_height + margins->bottom0()))
  {
    (*page) ++;
    *y = margins->top();

    if (Verbosity)
      progress_show("Formatting page %d", *page);
  }

  DEBUG_printf(("Before %s: y=%.1f, line_height=%.1f\n",
                _htmlStyleSheet->get_element(t->element), *y,
		t->style->line_height));

  check_pages(*page);

  if (t->element == HD_ELEMENT_H1 && !title_page)
    pages[*page].chapter = htmlGetText(current_heading);

  if ((pages[*page].heading == NULL || t->element == HD_ELEMENT_H1 ||
      (*page > 0 && pages[*page].heading == pages[*page - 1].heading)) &&
      !title_page)
  {
    pages[*page].heading  = htmlGetText(current_heading);
    pages[*page].headnode = current_heading;
  }

  if ((t->element - HD_ELEMENT_H1) < TocLevels && !title_page)
  {
    DEBUG_printf(("H%d: heading_pages[%d] = %d\n", t->element - HD_ELEMENT_H1 + 1,
                  num_headings, *page - 1));

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
    heading_tops[num_headings]  = (int)(*y + 2 * t->style->line_height);
    num_headings ++;
  }

  parse_paragraph(t, margins, x, y, page, needspace);

  if (t->style->text_align == HD_TEXT_ALIGN_RIGHT && t->element == HD_ELEMENT_H1 &&
      OutputType == HD_OUTPUT_BOOK && !title_page)
  {
   /*
    * Special case - chapter heading for users manual...
    */

    *y = margins->bottom0() + 0.5f * margins->length();

    margins->clear(*y, *page);
  }

  DEBUG_printf(("After %s: y=%.1f\n", _htmlStyleSheet->get_element(t->element),
                *y));
}


/*
 * 'parse_paragraph()' - Parse a paragraph tree and produce rendering list
 *                       output.
 */

static void
parse_paragraph(hdTree   *t,		/* I  - Tree to parse */
                hdMargin *margins,	/* I  - Margins */
        	float    *x,		/* IO - X position */
        	float    *y,		/* IO - Y position */
        	int      *page,		/* IO - Page # */
        	float    *needspace)	/* IO - Needed whitespace? */
{
  int		whitespace;		/* Non-zero if a fragment ends in whitespace */
  hdTree	*flat,
		*start,
		*end,
		*prev,
		*temp;
  float		width,
		height,
		offset,
		spacing,
		borderspace,
		hspace_value,
		vspace_value,
		temp_y,
		temp_width,
		temp_height;
  float		char_spacing;
  int		num_chars;
  hdRender	*r;
  hdChar	*align,
		*hspace,
		*vspace,
		*link,
		*border;
  float		rgb[3];
  hdChar	line[10240],
		*lineptr,
		*dataptr;
  hdTree	*linetype;
  float		linex,
		linewidth;
  int		firstline;


  DEBUG_printf(("parse_paragraph(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page, needspace));

  flat = flatten_tree(t->child);

  if (flat == NULL)
    DEBUG_puts("parse_paragraph: flat == NULL!");

  // Add leading whitespace...
  if (*needspace < t->style->margin[HD_POS_TOP])
    *needspace = t->style->margin[HD_POS_TOP];

//  printf("%s.needspace=%g, font-size=%g(%s), margin-top=%g(%s), margin-bottom=%g(%s)\n",
//         _htmlStyleSheet->get_element(t->element), *needspace,
//	 t->style->font_size, t->style->font_size_rel,
//	 t->style->margin[HD_POS_TOP], t->style->margin_rel[HD_POS_TOP],
//	 t->style->margin[HD_POS_BOTTOM], t->style->margin_rel[HD_POS_BOTTOM]);

  if (*y < margins->top() && *needspace)
    *y -= *needspace;

  margins->clear(*y, *page);
  margins->adjust_left(t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

 /*
  * First scan for images with left/right alignment tags...
  */

  for (temp = flat, prev = NULL; temp != NULL;)
  {
    if (temp->element == HD_ELEMENT_IMG)
      update_image_size(temp);

    if (temp->element == HD_ELEMENT_IMG &&
        (align = htmlGetAttr(temp, "ALIGN")))
    {
      if ((border = htmlGetAttr(temp, "BORDER")) != NULL)
	borderspace = atof((char *)border);
      else if (temp->link)
	borderspace = 1;
      else
	borderspace = 0;

      borderspace *= PagePrintWidth / _htmlStyleSheet->browser_width;

      if ((hspace = htmlGetAttr(temp, "HSPACE")) != NULL)
	hspace_value = atoi((char *)hspace) * PagePrintWidth /
	               _htmlStyleSheet->browser_width;
      else
	hspace_value = 0;

      if ((vspace = htmlGetAttr(temp, "VSPACE")) != NULL)
	vspace_value = atoi((char *)vspace) * PagePrintWidth /
	               _htmlStyleSheet->browser_width;
      else
	vspace_value = 0;

      if (strcasecmp((char *)align, "LEFT") == 0)
      {
        if ((temp->width + 2 * borderspace + hspace_value) >= margins->width() &&
	    margins->level() > 0)
	{
	  // Break past current images/tables...
	  margins->end(*y, *page);
	}

        if (*y < (margins->bottom0() + temp->height + 2 * borderspace + 2 * vspace_value))
        {
	  (*page) ++;
	  *y = margins->top();

          margins->clear(*y, *page);

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

	temp_y = *y - vspace_value;

        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->style->color[0] / 255.0f;
	    rgb[1] = temp->style->color[1] / 255.0f;
	    rgb[2] = temp->style->color[2] / 255.0f;
	  }

	  // Top
          new_render(*page, HD_RENDER_BOX, margins->left(), temp_y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          new_render(*page, HD_RENDER_BOX, margins->left(),
	             temp_y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, HD_RENDER_BOX, margins->left() + temp->width + borderspace,
	             temp_y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          new_render(*page, HD_RENDER_BOX, margins->left(),
	             temp_y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        temp_y -= borderspace;

        new_render(*page, HD_RENDER_IMAGE, margins->left() + borderspace,
	           temp_y - temp->height, temp->width, temp->height,
		   hdImage::find((char *)htmlGetAttr(temp, "_HD_SRC")));

        if (temp->link &&
	    (link = htmlGetAttr(temp->link, "_HD_FULL_HREF")) != NULL)
        {
	 /*
	  * Add a page link...
	  */

	  char baselink[1024];		// Basename of link

	  if (hdFile::scheme((char *)link) == NULL)
	  {
	    if (hdFile::target((char *)link) != NULL)
	      link = (hdChar *)hdFile::target((char *)link) - 1; // Include # sign
	    else
	      link = (hdChar *)hdFile::basename((char *)link, baselink,
	                                        sizeof(baselink));
	  }

	  new_render(*page, HD_RENDER_LINK, margins->left() + borderspace,
	             temp_y - temp->height, temp->width, temp->height, link);
        }

        temp_y -= borderspace + vspace_value + temp->height;

        margins->push(margins->left() + temp->width + 2 * borderspace +
	              hspace_value, margins->right(), temp_y, *page);

        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;

        free(temp);
        temp = prev;
      }
      else if (strcasecmp((char *)align, "RIGHT") == 0)
      {
        if ((temp->width + 2 * borderspace + hspace_value) >= margins->width() &&
	    margins->level() > 0)
	{
	  // Break past current images/tables...
	  margins->end(*y, *page);
	}

        if (*y < (margins->bottom0() + temp->height + 2 * borderspace + 2 * vspace_value))
        {
	  (*page) ++;
	  *y = margins->top();

          margins->clear(*y, *page);

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

	temp_y = *y - vspace_value;

        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->style->color[0] / 255.0f;
	    rgb[1] = temp->style->color[1] / 255.0f;
	    rgb[2] = temp->style->color[2] / 255.0f;
	  }

	  // Top
          new_render(*page, HD_RENDER_BOX,
	             margins->right() - temp->width - 2 * borderspace,
		     temp_y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          new_render(*page, HD_RENDER_BOX,
	             margins->right() - temp->width - 2 * borderspace,
	             temp_y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, HD_RENDER_BOX,
	             margins->right() - borderspace,
	             temp_y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          new_render(*page, HD_RENDER_BOX,
	             margins->right() - temp->width - 2 * borderspace,
		     temp_y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        temp_y -= borderspace;

        new_render(*page, HD_RENDER_IMAGE,
	           margins->right() - borderspace - temp->width,
	           temp_y - temp->height, temp->width, temp->height,
		   hdImage::find((char *)htmlGetAttr(temp, "_HD_SRC")));

        if (temp->link &&
	    (link = htmlGetAttr(temp->link, "_HD_FULL_HREF")) != NULL)
        {
	 /*
	  * Add a page link...
	  */

          char baselink[1024];		// Base name of link

	  if (hdFile::scheme((char *)link) == NULL)
	  {
	    if (hdFile::target((char *)link) != NULL)
	      link = (hdChar *)hdFile::target((char *)link) - 1; // Include # sign
	    else
	      link = (hdChar *)hdFile::basename((char *)link, baselink,
	                                        sizeof(baselink));
	  }

	  new_render(*page, HD_RENDER_LINK, margins->right() + borderspace,
	             temp_y - temp->height, temp->width, temp->height, link);
        }

        temp_y -= borderspace + vspace_value + temp->height;

        margins->push(margins->left(), margins->right() - temp->width -
	                               2 * borderspace - hspace_value,
                      temp_y, *page);

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

  firstline = 1;

  DEBUG_printf(("margins->width() = %.1f\n", margins->width()));

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

    if (flat->width >= margins->width() && margins->level() > 0)
      margins->end(*y, *page);

    while (flat != NULL)
    {
      // Get fragments...
      temp_width = 0.0;
      temp       = flat;
      whitespace = 0;

      DEBUG_printf(("    \"%s\" %.1fx%.1f (%.1f/%.1f)\n",
                    temp->element == HD_ELEMENT_NONE ? (char *)temp->data :
		        temp->element == HD_ELEMENT_IMG ? "(image)" : "(break)",
	            temp->width, temp->height,
		    temp->style->font_size, temp->style->line_height));

      while (temp != NULL && !whitespace)
      {
        if (temp->element == HD_ELEMENT_NONE && temp->data[0] == ' ')
	{
          if (temp == start)
            temp_width -= temp->style->get_width((hdChar *)" ");
          else if (temp_width > 0.0f)
	    whitespace = 1;
	}
        else
          whitespace = 0;

        if (whitespace)
	  break;

        if (temp->element == HD_ELEMENT_IMG)
	{
	  if ((border = htmlGetAttr(temp, "BORDER")) != NULL)
	    borderspace = atof((char *)border);
	  else if (temp->link)
	    borderspace = 1;
	  else
	    borderspace = 0;

          borderspace *= PagePrintWidth / _htmlStyleSheet->browser_width;

          temp_width += 2 * borderspace;
	}

        prev       = temp;
        temp       = temp->next;
        temp_width += prev->width;
        
        if ((temp_width >= margins->width() && prev->element == HD_ELEMENT_IMG) ||
	    prev->element == HD_ELEMENT_BR)
	  break;
      }

      if ((width + temp_width) <= margins->width())
      {
        width += temp_width;
        end  = temp;
        flat = temp;

        if (prev->element == HD_ELEMENT_BR)
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

      if (temp->element == HD_ELEMENT_NONE)
        num_chars += strlen((char *)temp->data);

      if (temp->height > height &&
          (temp->element != HD_ELEMENT_IMG ||
	   temp->style->vertical_align != HD_VERTICAL_ALIGN_MIDDLE))
        height = temp->height;
      else if ((0.5 * temp->height) > height &&
               temp->element == HD_ELEMENT_IMG &&
               temp->style->vertical_align == HD_VERTICAL_ALIGN_MIDDLE)
        height = 0.5 * temp->height;
    }

    for (spacing = 0.0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;

      if (temp->element != HD_ELEMENT_IMG)
        temp_height = temp->style->line_height;
      else
      {
	if ((border = htmlGetAttr(temp, "BORDER")) != NULL)
	  borderspace = atof((char *)border);
	else if (temp->link)
	  borderspace = 1;
	else
	  borderspace = 0;

        borderspace *= PagePrintWidth / _htmlStyleSheet->browser_width;

        temp_height = temp->height + 2 * borderspace;
      }

#ifdef DEBUG
      if (temp_height > 20)
      {
        if (temp->element == HD_ELEMENT_NONE)
          printf("\"%s\": temp_height=%.1f\n", temp->data, temp_height);
	else
	  printf("%s: temp_height=%.1f\n",
	         _htmlStyleSheet->get_element(temp->element), temp_height);
      }
#endif // DEBUG

      if (temp_height > spacing)
        spacing = temp_height;
    }

    if (firstline && end != NULL && *y < (margins->bottom0() + height +
                                          t->style->line_height))
    {
      // Go to next page since only 1 line will fit on this one...
      (*page) ++;
      *y = margins->top();

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    firstline = 0;

    if (height == 0.0f)
      height = spacing;

    for (temp = start; temp != end; temp = temp->next)
      if (temp->element != HD_ELEMENT_A)
        break;

    if (temp != NULL && temp->element == HD_ELEMENT_NONE && temp->data[0] == ' ')
    {
      // Drop leading space...
      for (dataptr = temp->data; *dataptr; dataptr ++)
        *dataptr = dataptr[1];
      *dataptr = '\0';

      temp_width = temp->style->get_width((hdChar *)" ");
      temp->width -= temp_width;
      num_chars --;
    }

    if (end != NULL)
      temp = end->prev;
    else
      temp = NULL;

    if (*y < (spacing + margins->bottom0()))
    {
      (*page) ++;
      *y = margins->top();

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

    rgb[0] = temp->style->color[0] / 255.0f;
    rgb[1] = temp->style->color[1] / 255.0f;
    rgb[2] = temp->style->color[2] / 255.0f;

    switch (t->style->text_align)
    {
      case HD_TEXT_ALIGN_LEFT :
      case HD_TEXT_ALIGN_INHERIT :
          linex = margins->left();
	  break;

      case HD_TEXT_ALIGN_CENTER :
          linex = margins->left() + 0.5f * (margins->width() - width);
	  break;

      case HD_TEXT_ALIGN_RIGHT :
          linex = margins->right() - width;
	  break;

      case HD_TEXT_ALIGN_JUSTIFY :
          linex = margins->left();
	  if (flat != NULL && flat->prev->element != HD_ELEMENT_BR &&
	      num_chars > 1)
	    char_spacing = (margins->width() - width) / (num_chars - 1);
	  break;
    }

    while (temp != end)
    {
      if (temp->link != NULL && PSLevel == 0 && Links &&
          temp->element == HD_ELEMENT_NONE)
      {
	temp->style->color[0] = (int)(link_color[0] * 255.0);
	temp->style->color[1] = (int)(link_color[1] * 255.0);
	temp->style->color[2] = (int)(link_color[2] * 255.0);
      }

     /*
      * See if we are doing a run of characters in a line and need to
      * output this run...
      */

      if (linetype != NULL &&
	  (temp->element != HD_ELEMENT_NONE || temp->style != linetype->style))
      {
        switch (linetype->style->vertical_align)
	{
	  case HD_VERTICAL_ALIGN_TEXT_TOP :
	  case HD_VERTICAL_ALIGN_SUPER :
	  case HD_VERTICAL_ALIGN_TOP :
	      offset = height - linetype->height;
	      break;
	  case HD_VERTICAL_ALIGN_MIDDLE :
	      offset = 0.5f * (height - linetype->height);
	      break;
	  case HD_VERTICAL_ALIGN_BOTTOM :
	  case HD_VERTICAL_ALIGN_BASELINE :
	  case HD_VERTICAL_ALIGN_TEXT_BOTTOM :
	  case HD_VERTICAL_ALIGN_INHERIT :
	      offset = 0.0f;
	      break;

	  case HD_VERTICAL_ALIGN_SUB :
              offset = linetype->height - height;
	      break;
	}

        r = new_render(*page, HD_RENDER_TEXT, linex - linewidth, *y + offset,
	               linewidth, linetype->height, line);
	r->data.text.typeface = linetype->style->font ?
				    linetype->style->font->typeface :
				    _htmlBodyFont;
	r->data.text.style    = linetype->style->font ?
				    linetype->style->font->style :
				    HD_FONT_INTERNAL_NORMAL;
	r->data.text.size     = linetype->style->font_size;
	r->data.text.spacing  = char_spacing;
        memcpy(r->data.text.rgb, rgb, sizeof(rgb));

        free(linetype);
        linetype = NULL;
      }

      switch (temp->element)
      {
        case HD_ELEMENT_A :
            if ((link = htmlGetAttr(temp, "NAME")) != NULL)
            {
             /*
              * Add a target link...
              */

              add_link(link, *page, (int)(*y + height));
            }

	default :
	    temp_width = temp->width;
            break;

        case HD_ELEMENT_NONE :
            if (temp->data == NULL)
              break;

	    if (((temp->width - margins->width()) > 0.001 ||
	         (temp->height - margins->length()) > 0.001)  && OverflowErrors)
	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Text on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);

            switch (temp->style->vertical_align)
	    {
	      case HD_VERTICAL_ALIGN_TEXT_TOP :
	      case HD_VERTICAL_ALIGN_SUPER :
	      case HD_VERTICAL_ALIGN_TOP :
		  offset = height - temp->height;
		  break;
	      case HD_VERTICAL_ALIGN_MIDDLE :
		  offset = 0.5f * (height - temp->height);
		  break;
	      case HD_VERTICAL_ALIGN_BOTTOM :
	      case HD_VERTICAL_ALIGN_BASELINE :
	      case HD_VERTICAL_ALIGN_TEXT_BOTTOM :
	      case HD_VERTICAL_ALIGN_INHERIT :
		  offset = 0.0f;
		  break;

	      case HD_VERTICAL_ALIGN_SUB :
        	  offset = temp->height - height;
		  break;
	    }

            if (linetype == NULL)
            {
	      linetype  = temp;
	      lineptr   = line;
	      linewidth = 0.0;

	      rgb[0] = temp->style->color[0] / 255.0f;
	      rgb[1] = temp->style->color[1] / 255.0f;
	      rgb[2] = temp->style->color[2] / 255.0f;
	    }

            strlcpy((char *)lineptr, (char *)temp->data,
	            sizeof(line) - (lineptr - line));

            temp_width = temp->width + char_spacing * strlen((char *)lineptr);

	    if (temp->style->text_decoration == HD_TEXT_DECORATION_UNDERLINE ||
	        (temp->link && LinkStyle && PSLevel == 0))
	      new_render(*page, HD_RENDER_BOX, linex, *y + offset - 1,
	                 temp_width, 0, rgb);

	    if (temp->style->text_decoration == HD_TEXT_DECORATION_LINE_THROUGH)
	      new_render(*page, HD_RENDER_BOX, linex, *y + offset + temp->height * 0.25f,
	                 temp_width, 0, rgb);

            linewidth  += temp_width;
            lineptr    += strlen((char *)lineptr);

            if (lineptr > line && lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;

	case HD_ELEMENT_IMG :
	    if (((temp->width - margins->width()) > 0.001 ||
	         (temp->height - margins->length()) > 0.001) && OverflowErrors)
	    {
	      DEBUG_printf(("IMAGE: %.3fx%.3f > %.3fx%.3f\n",
	                    temp->width, temp->height,
			    margins->width(), margins->length()));

	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Image on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);
            }

	    if ((border = htmlGetAttr(temp, "BORDER")) != NULL)
	      borderspace = atof((char *)border);
	    else if (temp->link)
	      borderspace = 1;
	    else
	      borderspace = 0;

            borderspace *= PagePrintWidth / _htmlStyleSheet->browser_width;

            temp_width += 2 * borderspace;

            switch (temp->style->vertical_align)
	    {
	      case HD_VERTICAL_ALIGN_TEXT_TOP :
	      case HD_VERTICAL_ALIGN_SUPER :
	      case HD_VERTICAL_ALIGN_TOP :
		  offset = height - temp->height - 2 * borderspace;
		  break;
	      case HD_VERTICAL_ALIGN_MIDDLE :
		  offset = 0.5f * (height - temp->height) - borderspace;
		  break;
	      case HD_VERTICAL_ALIGN_BOTTOM :
	      case HD_VERTICAL_ALIGN_BASELINE :
	      case HD_VERTICAL_ALIGN_TEXT_BOTTOM :
	      case HD_VERTICAL_ALIGN_INHERIT :
		  offset = 0.0f;
		  break;

	      case HD_VERTICAL_ALIGN_SUB :
        	  offset = temp->height - height;
		  break;
	    }

            if (borderspace > 0.0f)
	    {
	      // Top
              new_render(*page, HD_RENDER_BOX, linex,
	                 *y + offset + temp->height + borderspace,
			 temp->width + 2 * borderspace, borderspace, rgb);
	      // Left
              new_render(*page, HD_RENDER_BOX, linex, *y + offset,
                	 borderspace, temp->height + 2 * borderspace, rgb);
	      // Right
              new_render(*page, HD_RENDER_BOX,
	                 linex + temp->width + borderspace,
	                 *y + offset, borderspace,
			 temp->height + 2 * borderspace, rgb);
	      // Bottom
              new_render(*page, HD_RENDER_BOX, linex, *y + offset,
                	 temp->width + 2 * borderspace, borderspace, rgb);
	    }

	    new_render(*page, HD_RENDER_IMAGE, linex + borderspace,
	               *y + offset + borderspace, temp->width, temp->height,
		       hdImage::find((char *)htmlGetAttr(temp, "_HD_SRC")));
            whitespace = 0;
	    temp_width = temp->width + 2 * borderspace;
	    break;
      }

      if (temp->link != NULL &&
          (link = htmlGetAttr(temp->link, "_HD_FULL_HREF")) != NULL)
      {
       /*
	* Add a page link...
	*/

        char baselink[1024];		// Base name of link

	if (hdFile::scheme((char *)link) == NULL)
	{
	  if (hdFile::target((char *)link) != NULL)
	    link = (hdChar *)hdFile::target((char *)link) - 1; // Include # sign
	  else
	    link = (hdChar *)hdFile::basename((char *)link, baselink,
	                                      sizeof(baselink));
	}

	new_render(*page, HD_RENDER_LINK, linex, *y + offset, temp->width,
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
      switch (linetype->style->vertical_align)
      {
	case HD_VERTICAL_ALIGN_TEXT_TOP :
	case HD_VERTICAL_ALIGN_SUPER :
	case HD_VERTICAL_ALIGN_TOP :
	    offset = height - linetype->height;
	    break;
	case HD_VERTICAL_ALIGN_MIDDLE :
	    offset = 0.5f * (height - linetype->height);
	    break;
	case HD_VERTICAL_ALIGN_BOTTOM :
	case HD_VERTICAL_ALIGN_BASELINE :
	case HD_VERTICAL_ALIGN_TEXT_BOTTOM :
	case HD_VERTICAL_ALIGN_INHERIT :
	    offset = 0.0f;
	    break;

	case HD_VERTICAL_ALIGN_SUB :
            offset = linetype->height - height;
	    break;
      }

      r = new_render(*page, HD_RENDER_TEXT, linex - linewidth, *y + offset,
                     linewidth, linetype->height, line);
      if (linetype->style->font)
      {
	r->data.text.typeface = linetype->style->font->typeface;
	r->data.text.style    = linetype->style->font->style;
      }

      r->data.text.spacing = char_spacing;
      r->data.text.size    = linetype->style->font_size;
      memcpy(r->data.text.rgb, rgb, sizeof(rgb));

      free(linetype);
    }

   /*
    * Update the margins after we pass below the images...
    */

    *y -= spacing - height;

    margins->clear(*y, *page);
  }

  margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);

  *x = margins->left();
  *needspace = t->style->margin[HD_POS_BOTTOM];

  DEBUG_printf(("LEAVING parse_paragraph(), x = %.1f, y = %.1f, page = %d\n",
                *x, *y, *page));
}


/*
 * 'parse_pre()' - Parse preformatted text and produce rendering list output.
 */

static void
parse_pre(hdTree   *t,			/* I - Tree to parse */
          hdMargin *margins,		/* I - Margins */
          float    *x,			/* IO - X position */
          float    *y,			/* IO - Y position */
          int      *page,		/* IO - Page # */
          float    *needspace)		/* I - Need whitespace? */
{
  hdTree	*flat, *start, *next;
  hdChar	*link,
		line[10240],
		*lineptr,
		*dataptr;
  int		col;
  float		width,
		height,
		rgb[3];
  hdRender	*r;


  DEBUG_printf(("parse_pre(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page));

  if (t->child == NULL)
    return;

  // Add leading whitespace...
  if (*needspace < t->style->margin[HD_POS_TOP])
    *needspace = t->style->margin[HD_POS_TOP];

  if (*y < margins->top() && *needspace)
    *y -= *needspace;

  margins->clear(*y, *page);
  margins->adjust_left(t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

  flat = flatten_tree(t->child);

  if (flat == NULL)
    return;

  if (flat->element == HD_ELEMENT_NONE && flat->data != NULL)
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

      if (flat->element == HD_ELEMENT_BR ||
          (flat->element == HD_ELEMENT_NONE && flat->data &&
	   flat->data[strlen((char *)flat->data) - 1] == '\n'))
        break;
    }

    if (flat)
      flat = flat->next;

    if (*y < (height + margins->bottom0()))
    {
      (*page) ++;
      *y = margins->top();

      if (Verbosity)
	progress_show("Formatting page %d", *page);
    }

    *x = margins->left();
    *y -= height;

    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / PagePrintLength));

    col = 0;
    while (start != flat)
    {
      rgb[0] = start->style->color[0] / 255.0f;
      rgb[1] = start->style->color[1] / 255.0f;
      rgb[2] = start->style->color[2] / 255.0f;

      if (start->link &&
	  (link = htmlGetAttr(start->link, "_HD_FULL_HREF")) != NULL)
      {
       /*
	* Add a page link...
	*/

        char	baselink[1024];		// Base name of link

	if (hdFile::scheme((char *)link) == NULL)
	{
	  if (hdFile::target((char *)link) != NULL)
	    link = (hdChar *)hdFile::target((char *)link) - 1; // Include # sign
	  else
	    link = (hdChar *)hdFile::basename((char *)link, baselink,
	                                      sizeof(baselink));
	}

	new_render(*page, HD_RENDER_LINK, *x, *y, start->width,
	           start->height, link);

	if (PSLevel == 0 && Links)
	{
          memcpy(rgb, link_color, sizeof(rgb));

          if (LinkStyle)
	    new_render(*page, HD_RENDER_BOX, *x, *y - 1, start->width, 0,
	               link_color);
	}
      }

      switch (start->element)
      {
	case HD_ELEMENT_A :
            if ((link = htmlGetAttr(start, "NAME")) != NULL)
            {
             /*
              * Add a target link...
              */

              add_link(link, *page, (int)(*y + height));
            }
            break;

	case HD_ELEMENT_NONE :
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

            width = start->style->get_width(line);
            r = new_render(*page, HD_RENDER_TEXT, *x, *y, width, 0, line);
            r->data.text.typeface = start->style->font->typeface;
            r->data.text.style    = start->style->font->style;
            r->data.text.size     = start->style->font_size;
            memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	    if (start->style->text_decoration == HD_TEXT_DECORATION_UNDERLINE)
	      new_render(*page, HD_RENDER_BOX, *x, *y - 1, start->width, 0, rgb);

	    if (start->style->text_decoration == HD_TEXT_DECORATION_LINE_THROUGH)
	      new_render(*page, HD_RENDER_BOX, *x, *y + start->height * 0.25f,
	        	 start->width, 0, rgb);

            *x += start->width;
            break;

	case HD_ELEMENT_IMG :
	    new_render(*page, HD_RENDER_IMAGE, *x, *y, start->width, start->height,
		       hdImage::find((char *)htmlGetAttr(start, "_HD_SRC")));

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

    if ((*x - margins->right()) > 0.001 && OverflowErrors)
      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	             "Preformatted text on page %d too long - "
		     "truncation or overlapping may occur!", *page + 1);

    *y -= t->style->line_height - t->style->font_size;
  }

  margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);

  *x = margins->left();
  *needspace = t->style->margin[HD_POS_BOTTOM];
}


#ifdef TABLE_DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x) puts(x)
#  define DEBUG
#  undef DEBUG_printf
#  define DEBUG_printf(x) printf x
#endif /* TABLE_DEBUG */


struct hdTableColumn			//// Table column state
{
  bool		fixed,			// Fixed-width column?
		percent;		// Percentage-width column?
  int		colspan,		// Current colspan
		rowspan,		// Current rowspan
		start_page,		// Start page
		end_page;		// End page
  float		start_y,		// Start Y position
		end_y,			// End Y position
		left,			// Left X position
		right,			// Right X position
		width,			// Width of column
		min_width,		// Minimum width of column
		pref_width,		// Preferred width of column
		span_width,		// Spanned width of column
		min_span_width,		// Minimum spanned width of column
		height,			// Height of current column
		span_height;		// Height of spanned column
  hdRender	*bg_render,		// Background rectangles
		*start_render,		// Start of the column content
		*end_render;		// End of the column content
};


/*
 * 'render_table_row()' - Render a table row.
 */

static void
render_table_row(int           num_cols,// I  - Number of columns
		 hdTableColumn *cols,	// I  - Columns
                 hdTree        **cells,	// I  - Cells in this row
		 float         *y,	// IO - Y position
		 int           *page)	// IO - Page #
{
}


/*
 * 'parse_table()' - Parse a table and produce rendering output.
 */

static void
parse_table(hdTree   *t,		// I - Tree to parse
            hdMargin *margins,		// I - Margins
            float    *x,		// IO - X position
            float    *y,		// IO - Y position
            int      *page,		// IO - Page #
            float    *needspace)	// I - Need whitespace?
{
  int		col,			// Current column
		row,			// Current row
		tcol,			// Temporary column
		colspan,		// Current colspan
		rowspan,		// Current rowspan
		num_cols,		// Number of columns
		num_rows,		// Number of rows
		alloc_rows,		// Allocated rows
		regular_cols,		// Number of regular-size columns
		header_row;		// Header row
  hdTableColumn	columns[MAX_COLUMNS],	// Normal columns
		headers[MAX_COLUMNS];	// Header columns
  float		col_width,		// Current column width
		col_min,		// Current minimum column width
		col_pref,		// Current preferred column width
		col_height,		// Current column height
		cellpadding,		// Cell padding to use
		cellspacing,		// Cell spacing to use
		border,			// Border
		border_left,		// Border left margin
		border_size,		// Total border size
		width,			// Table width
		pref_width,		// Preferred table width
		span_width,		// Width of span
		regular_width,		// Width of all normal sized columns
		actual_width,		// Actual width of table
		table_width,		// Requested table width
		table_height,		// Requested table height
		min_width,		// Minimum table width
		temp_width,		// Current width
		tempspace,		// Temporary spacing
		table_y,		// Table start position
		row_y,			// Row end position
		row_starty,		// Row start position
		temp_y;			// Temporary position
  int		row_page,		// Row start page
		temp_page,		// Temporary page
		table_page;		// Table start page
  hdChar	*var,			// Attribute
		*height_var;		// Row HEIGHT variable
  hdTree	*temprow,		// Current row
		*tempcol,		// Current column
		*tempnext,		// Next row/column
		***cells,		// Table cell grid
		*caption;		// Table caption, if any
  bool		do_valign;		// True if we should do vertical alignment of cells
  float		row_height,		// Total height of the row
		temp_height;		// Temporary holder
  hdMargin	*cell_margins;		// Margins for cell
  hdChar	*bgcolor;		// Background color, if any
  float		rgb[3],			// Border color
		bgrgb[3];		// Background color
  const char	*htmldoc_debug;		// HTMLDOC_DEBUG env var
  int		table_debug;		// Do table debugging?


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("parse_table(t=%p, margins=(%.1f, %.1f,%.1f, %.1f), "
                "x=%.1f, y=%.1f, page=%d\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page));

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

  // Add leading whitespace...
  if (*needspace < t->style->margin[HD_POS_TOP])
    *needspace = t->style->margin[HD_POS_TOP];

  if (*y < margins->top() && *needspace)
    *y -= *needspace;

  margins->clear(*y, *page);
  margins->adjust_left(t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

 /*
  * Figure out the # of rows, columns, and the desired widths...
  */

  cells = NULL;

  if ((var = htmlGetAttr(t, "WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_width = atof((char *)var) * margins->width() / 100.0f;
    else
      table_width = atoi((char *)var) * PagePrintWidth /
                    _htmlStyleSheet->browser_width;
  }
  else
    table_width = margins->width();

  if ((var = htmlGetAttr(t, "HEIGHT")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_height = atof((char *)var) * margins->length() / 100.0f;
    else
      table_height = atoi((char *)var) * PagePrintWidth /
                     _htmlStyleSheet->browser_width;
  }
  else
    table_height = -1.0f;

  DEBUG_printf(("table_width = %.1f\n", table_width));

  if ((var = htmlGetAttr(t, "CELLPADDING")) != NULL)
    cellpadding = atoi((char *)var);
  else
    cellpadding = 1.0f;

  if ((var = htmlGetAttr(t, "CELLSPACING")) != NULL)
    cellspacing = atoi((char *)var);
  else
    cellspacing = 0.0f;

  if ((var = htmlGetAttr(t, "BORDER")) != NULL)
  {
    if ((border = atof((char *)var)) == 0.0 && var[0] != '0')
      border = 1.0f;

    cellpadding += border;
  }
  else
    border = 0.0f;

  if (table_debug && border == 0.0f)
    border = 0.01f;

  rgb[0] = t->style->color[0] / 255.0f;
  rgb[1] = t->style->color[1] / 255.0f;
  rgb[2] = t->style->color[2] / 255.0f;

  if ((var = htmlGetAttr(t, "BORDERCOLOR")) != NULL)
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

  cellspacing *= PagePrintWidth / _htmlStyleSheet->browser_width;
  cellpadding *= PagePrintWidth / _htmlStyleSheet->browser_width;
  border      *= PagePrintWidth / _htmlStyleSheet->browser_width;
  border_size *= PagePrintWidth / _htmlStyleSheet->browser_width;

  DEBUG_printf(("border=%.1f, cellpadding=%.1f\n", border, cellpadding));

//  temp_bottom = margins->bottom() - cellpadding;
//  temp_top    = margins->top() + cellpadding;
  margins->adjust_bottom(-cellpadding);
  margins->adjust_top(cellpadding);

  memset(columns, 0, sizeof(columns));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0,
           caption = NULL, row = 0;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;

    if (temprow->element == HD_ELEMENT_CAPTION)
    {
      if ((var = htmlGetAttr(temprow, "ALIGN")) == NULL ||
          strcasecmp((char *)var, "bottom"))
      {
       /*
        * Show caption at top...
	*/

        *needspace = 0;
        parse_paragraph(temprow, margins, x, y, page, needspace);
      }
      else
      {
       /*
        * Flag caption for bottom of table...
	*/

        caption = temprow;
      }
    }
    else if (temprow->element == HD_ELEMENT_TR ||
             ((temprow->element == HD_ELEMENT_TBODY ||
	       temprow->element == HD_ELEMENT_THEAD ||
               temprow->element == HD_ELEMENT_TFOOT) && temprow->child != NULL))
    {
      // Descend into table body as needed...
      if (temprow->element == HD_ELEMENT_THEAD)
        header_row = row;

      if (temprow->element == HD_ELEMENT_TBODY ||
          temprow->element == HD_ELEMENT_THEAD ||
          temprow->element == HD_ELEMENT_TFOOT)
        temprow = temprow->child;

      // Figure out the next row...
      if ((tempnext = temprow->next) == NULL)
        if (temprow->parent->element == HD_ELEMENT_TBODY ||
            temprow->parent->element == HD_ELEMENT_THEAD ||
            temprow->parent->element == HD_ELEMENT_TFOOT)
          tempnext = temprow->parent->next;

      // Allocate memory for the table as needed...
      if (num_rows >= alloc_rows)
      {
        alloc_rows += ALLOC_ROWS;

        if (alloc_rows == ALLOC_ROWS)
	  cells = (hdTree ***)malloc(sizeof(hdTree **) * alloc_rows);
	else
	  cells = (hdTree ***)realloc(cells, sizeof(hdTree **) * alloc_rows);

        if (cells == (hdTree ***)0)
	{
	  progress_error(HD_ERROR_OUT_OF_MEMORY,
                         "Unable to allocate memory for table!");
	  return;
	}
      }	

      if ((cells[num_rows] = (hdTree **)calloc(sizeof(hdTree *), MAX_COLUMNS)) == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for table!");
	return;
      }

#ifdef DEBUG
      printf("BEFORE row %d: num_cols=%d\n", num_rows, num_cols);

      if (num_rows)
        for (col = 0; col < num_cols; col ++)
	  printf("    columns[%d].rowspan=%d\n", col, columns[col].rowspan);
#endif // DEBUG

      // Figure out the starting column...
      if (num_rows)
      {
	for (col = 0, rowspan = 9999; col < num_cols; col ++)
	  if (columns[col].rowspan < rowspan)
	    rowspan = columns[col].rowspan;

	for (col = 0; col < num_cols; col ++)
	  columns[col].rowspan -= rowspan;

	for (col = 0; columns[col].rowspan && col < num_cols; col ++)
          cells[num_rows][col] = cells[num_rows - 1][col];
      }
      else
        col = 0;

      for (tempcol = temprow->child;
           tempcol != NULL && col < MAX_COLUMNS;
           tempcol = tempcol->next)
        if (tempcol->element == HD_ELEMENT_TD || tempcol->element == HD_ELEMENT_TH)
        {
	  // Handle colspan and rowspan stuff...
          if ((var = htmlGetAttr(tempcol, "COLSPAN")) != NULL)
            colspan = atoi((char *)var);
          else
            colspan = 1;

          if ((var = htmlGetAttr(tempcol, "ROWSPAN")) != NULL)
	  {
            columns[col].rowspan = atoi((char *)var);

	    if (columns[col].rowspan == 1)
	      columns[col].rowspan = 0;

	    for (tcol = 1; tcol < colspan; tcol ++)
              columns[col + tcol].rowspan = columns[col].rowspan;
          }

          // Compute the cell size...
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min,
	                            &col_pref, &col_height);
          if ((var = htmlGetAttr(tempcol, "WIDTH")) != NULL)
	  {
	    if (var[strlen((char *)var) - 1] == '%')
	    {
              col_width -= 2.0 * cellpadding - cellspacing;

	      if (colspan <= 1)
	        columns[col].percent = true;
	    }
	    else
	    {
              col_width -= 2.0 * cellpadding;
	    }
	  }
	  else
	    col_width = 0.0f;

          tempcol->height = col_height;

	  DEBUG_printf(("%d,%d: colsp=%d, rowsp=%d, width=%.1f, minw=%.1f, "
	                "prefw=%.1f, minh=%.1f\n",
	                col, num_rows, colspan, columns[col].rowspan, col_width,
			col_min, col_pref, col_height));

          // Add widths to columns...
          if (colspan > 1)
          {
	    if (colspan > columns[col].colspan)
	      columns[col].colspan = colspan;

	    if (col_width > columns[col].span_width)
	      columns[col].span_width = col_width;

	    if (col_min > columns[col].min_span_width)
	      columns[col].min_span_width = col_min;
          }
	  else
	  {
	    if (col_width > 0.0f)
	      columns[col].fixed = true;

	    if (col_width > columns[col].width)
	      columns[col].width = col_width;

	    if (col_pref > columns[col].pref_width)
	      columns[col].pref_width = col_pref;

	    if (col_min > columns[col].min_width)
	      columns[col].min_width = col_min;
          }

	  while (colspan > 0 && col < MAX_COLUMNS)
	  {
            cells[num_rows][col] = tempcol;
            col ++;
            colspan --;
          }

          while (columns[col].rowspan && col < num_cols)
	  {
            cells[num_rows][col] = cells[num_rows - 1][col];
	    col ++;
	  }
        }

      if (col > num_cols)
        num_cols = col;

#ifdef DEBUG
      printf("AFTER row %d: num_cols=%d\n", num_rows, num_cols);

      for (col = 0; col < num_cols; col ++)
        printf("    col %d: rowspan=%d\n", col, columns[col].rowspan);
#endif // DEBUG

      num_rows ++;

      for (col = 0; col < num_cols; col ++)
        if (columns[col].rowspan)
	  columns[col].rowspan --;
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

  if ((var = htmlGetAttr(t, "WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      width = atof((char *)var) * margins->width() / 100.0f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlStyleSheet->browser_width;
  }
  else
  {
    for (col = 0, width = 0.0; col < num_cols; col ++)
      width += columns[col].pref_width;

    width += (2 * cellpadding + cellspacing) * num_cols - cellspacing;

    if (width > margins->width())
      width = margins->width();
  }

 /*
  * Compute the width of each column based on the printable width.
  */

  DEBUG_printf(("\nTABLE: %dx%d\n\n", num_cols, num_rows));

  actual_width  = (2 * cellpadding + cellspacing) * num_cols -
                  cellspacing;
  regular_width = (width - actual_width) / num_cols;

  DEBUG_printf(("    width = %.1f, actual_width=%.1f, regular_width=%.1f\n\n",
                width, actual_width, regular_width));
  DEBUG_puts("    Col  Width   Min     Pref    Fixed?  Percent?");
  DEBUG_puts("    ---  ------  ------  ------  ------  --------");

#ifdef DEBUG
  for (col = 0; col < num_cols; col ++)
    printf("    %-3d  %-6.1f  %-6.1f  %-6.1f  %-6s  %s\n", col,
           columns[col].width, columns[col].min_width, columns[col].pref_width,
	   columns[col].fixed ? "YES" : "NO",
	   columns[col].percent ? "YES" : "NO");

  puts("");
#endif /* DEBUG */

 /*
  * The first pass just handles columns with a specified width...
  */

  DEBUG_puts("PASS 1: fixed width handling\n");

  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (columns[col].width > 0.0f)
    {
      if (columns[col].min_width > columns[col].width)
        columns[col].width = columns[col].min_width;

      actual_width += columns[col].width;
    }
    else
    {
      regular_cols ++;

      actual_width += columns[col].min_width;
    }

  DEBUG_printf(("    actual_width=%.1f, regular_cols=%d\n\n", actual_width,
                regular_cols));

 /*
  * Pass two uses the "preferred" width whenever possible, and the
  * minimum otherwise...
  */

  DEBUG_puts("PASS 2: preferred width handling\n");

  for (col = 0, pref_width = 0.0f; col < num_cols; col ++)
    if (columns[col].width == 0.0f)
      pref_width += columns[col].pref_width - columns[col].min_width;

  DEBUG_printf(("    pref_width=%.1f\n", pref_width));

  if (pref_width > 0.0f)
  {
    if ((regular_width = (width - actual_width) / pref_width) < 0.0f)
      regular_width = 0.0f;
    else if (regular_width > 1.0f)
      regular_width = 1.0f;

    DEBUG_printf(("    regular_width=%.1f\n", regular_width));

    for (col = 0; col < num_cols; col ++)
      if (columns[col].width == 0.0f)
      {
	pref_width = (columns[col].pref_width - columns[col].min_width) *
	             regular_width;

	if ((actual_width + pref_width) > width)
	{
          if (col == (num_cols - 1) &&
	      (width - actual_width) >= columns[col].min_width)
	    columns[col].width = width - actual_width;
	  else
	    columns[col].width = columns[col].min_width;
	}
	else
          columns[col].width = pref_width + columns[col].min_width;

        DEBUG_printf(("    columns[%d].width=%.1f\n", col, columns[col].width));

	actual_width += columns[col].width - columns[col].min_width;
      }
  }
  else
  {
   /*
    * Assign min widths for all cells...
    */

    for (col = 0; col < num_cols; col ++)
      if (columns[col].width == 0.0f)
        columns[col].width = columns[col].min_width;
  }

  DEBUG_printf(("    actual_width=%.1f\n\n", actual_width));

 /*
  * Pass three enforces any hard or minimum widths for COLSPAN'd
  * columns...
  */

  DEBUG_puts("PASS 3: colspan handling\n\n");

  for (col = 0; col < num_cols; col ++)
  {
    DEBUG_printf(("    columns[%d].colspan=%d\n", col, columns[col].colspan));

    if (columns[col].colspan > 1)
    {
      for (colspan = 0, span_width = 0.0f;
           colspan < columns[col].colspan;
	   colspan ++)
        span_width += columns[col + colspan].width;

      pref_width = 0.0f;

      if (span_width < columns[col].span_width)
        pref_width = columns[col].span_width;
      if (span_width < columns[col].min_span_width &&
          pref_width < columns[col].min_span_width)
        pref_width = columns[col].min_span_width;

      for (colspan = 0; colspan < columns[col].colspan; colspan ++)
        if (columns[col + colspan].fixed)
	{
          span_width -= columns[col + colspan].width;
	  pref_width -= columns[col + colspan].width;
	}

      DEBUG_printf(("    columns[%d].span_width=%.1f, "
                    "columns[%d].min_span_width=%.1f, "
                    "span_width=%.1f, pref_width=%.1f\n",
                    col, columns[col].span_width,
		    col, columns[col].min_span_width,
		    span_width, pref_width));

      if (pref_width > 0.0f && pref_width > span_width)
      {
        if (span_width >= 1.0f)
	{
          // Expand cells proportionately...
	  regular_width = pref_width / span_width;

	  for (colspan = 0; colspan < columns[col].colspan; colspan ++)
	    if (!columns[col + colspan].fixed)
	    {
	      actual_width -= columns[col + colspan].width;
	      columns[col + colspan].width *= regular_width;
	      actual_width += columns[col + colspan].width;

              DEBUG_printf(("    columns[%d].width=%.1f\n", col + colspan,
	                    columns[col + colspan].width));
	    }
        }
	else
	{
	  // Divide the space up equally between columns, since the
	  // colspan area is always by itself... (this hack brought
	  // to you by Yahoo! and their single cell tables with
	  // colspan=2 :)

	  regular_width = pref_width / columns[col].colspan;

	  for (colspan = 0; colspan < columns[col].colspan; colspan ++)
	  {
	    actual_width += regular_width;
	    columns[col + colspan].width += regular_width;

            DEBUG_printf(("    columns[%d].width=%.1f\n", col + colspan,
	                   columns[col + colspan].width));
	  }
	}
      }
    }
  }

  DEBUG_printf(("    actual_width=%.1f\n\n", actual_width));

 /*
  * Pass four divides up the remaining space amongst the columns...
  */

  DEBUG_puts("PASS 4: divide remaining space, if any...\n");

  if (width > actual_width)
  {
    for (col = 0, colspan = 0; col < num_cols; col ++)
      if (!columns[col].fixed || columns[col].percent)
        colspan ++;

    if (colspan > 0)
    {
      regular_width = (width - actual_width) / num_cols;

      for (col = 0; col < num_cols; col ++)
        if (!columns[col].fixed || columns[col].percent)
	{
	  columns[col].width += regular_width;
	  DEBUG_printf(("    columns[%d].width=%.1f\n", col,
	                columns[col].widt));
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
      min_width += columns[col].min_width + 2 * cellpadding + cellspacing;

    DEBUG_printf(("    table_width=%.1f, width=%.1f, min_width=%.1f\n",
                  table_width, width, min_width));

    temp_width = table_width - min_width;
    if (temp_width < 0.0f)
      temp_width = 0.0f;

    width -= min_width;
    if (width < 1.0f)
      width = 1.0f;

    for (col = 0; col < num_cols; col ++)
    {
      columns[col].width = columns[col].min_width +
                           temp_width * (columns[col].width -
			                 columns[col].min_width) / width;

      DEBUG_printf(("    columns[%d],width=%.1f\n", col, columns[col].width));
    }

    for (col = 0, width = -cellspacing; col < num_cols; col ++)
      width += columns[col].width + 2 * cellpadding + cellspacing;

    DEBUG_printf(("    new width=%.1f, margins->width()=%.1f\n", width,
                  margins->width()));
  }

  if ((width - margins->width()) > 0.001f && OverflowErrors)
    progress_error(HD_ERROR_CONTENT_TOO_LARGE,
                   "Table on page %d too wide - "
		   "truncation or overlapping may occur!", *page + 1);

  DEBUG_puts("");

  DEBUG_printf(("Final table width=%.1f, margin[HD_POS_LEFT]=%.1f, "
                "margin[HD_POS_RIGHT]=%.1f\n",
                width, t->style->margin[HD_POS_LEFT],
		t->style->margin[HD_POS_RIGHT]));

  if (!t->style->margin_rel[HD_POS_LEFT] ||
      strcmp(t->style->margin_rel[HD_POS_LEFT], "auto"))
    *x = margins->left() + cellpadding;
  else if (!t->style->margin_rel[HD_POS_RIGHT] ||
	   strcmp(t->style->margin_rel[HD_POS_RIGHT], "auto"))
    *x = margins->right() - width + cellpadding;
  else
    *x = margins->left() + 0.5f * (margins->width() - width) + cellpadding;

  for (col = 0; col < num_cols; col ++)
  {
    columns[col].left  = *x;
    columns[col].right = *x + columns[col].width;
    *x = columns[col].right + 2 * cellpadding + cellspacing;

    DEBUG_printf(("columns[%d].left=%.1f, columns[%d].right=%.1f\n", col,
                  columns[col].left, col, columns[col].right));
  }

 /*
  * Now render the whole table...
  */

  if (*y < margins->top() && needspace)
    *y -= t->style->line_height;

  if (table_debug)
  {
    check_pages(*page);

    hdRender *r;
    char table_debug[255];

    snprintf(table_debug, sizeof(table_debug), "t=%p", t);
    r = new_render(*page, HD_RENDER_TEXT, margins->left(), *y,
                   0.0f, 11.0f, table_debug);

    r->data.text.typeface = HD_FONT_FACE_MONOSPACE;
    r->data.text.style    = HD_FONT_INTERNAL_NORMAL;
    r->data.text.size     = 11.0f;
  }

  for (col = 0; col < num_cols; col ++)
  {
    columns[col].rowspan      = 0;
    columns[col].start_render = NULL;
    columns[col].end_render   = NULL;
    columns[col].height       = 0.0f;
    columns[col].bg_render    = NULL;
  }

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
          cells[row][0]->parent->prev->element == HD_ELEMENT_COMMENT)
        parse_comment(cells[row][0]->parent->prev, margins, x, y,
		      page, NULL, 0);

     /*
      * Get height...
      */

      if ((height_var = htmlGetAttr(cells[row][0]->parent,
                           	        "HEIGHT")) == NULL)
	for (col = 0; col < num_cols; col ++)
	  if (htmlGetAttr(cells[row][col], "ROWSPAN") == NULL)
	    if ((height_var = htmlGetAttr(cells[row][col],
                                              "HEIGHT")) != NULL)
	      break;
    }

    if (cells[row][0] != NULL && height_var != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (height_var[strlen((char *)height_var) - 1] == '%')
	temp_height = atof((char *)height_var) * 0.01f *
	              (PagePrintLength - 2 * cellpadding);
      else
        temp_height = atof((char *)height_var) * PagePrintWidth /
	              _htmlStyleSheet->browser_width;

      if (table_height > 0.0f && temp_height > table_height)
        temp_height = table_height;

      temp_height -= 2 * cellpadding;
    }
    else
    {
      // Use min height computed from get_cell_size()...
      for (col = 0, temp_height = t->style->line_height;
           col < num_cols;
	   col ++)
        if (cells[row][col] != NULL &&
	    cells[row][col]->height > temp_height &&
	    !htmlGetAttr(cells[row][col], "ROWSPAN"))
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

    DEBUG_printf(("BEFORE row %d: temp_height=%.1f, *y=%.1f, *page=%d\n",
                  row, temp_height, *y, *page));

    if (*y < (margins->bottom0() + 2 * cellpadding + temp_height) &&
        temp_height <= (margins->length() - 2 * cellpadding))
    {
      DEBUG_puts("NEW PAGE");

      *y = margins->top();
      (*page) ++;

      margins->clear(*y, *page);

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    do_valign  = 1;
    row_y      = *y - cellpadding;
    row_starty = row_y;
    row_page   = *page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row_y=%.1f, *y=%.1f, row_page=%d\n", row_y, *y,
                  row_page));

    for (col = 0, rowspan = 9999; col < num_cols; col += colspan)
    {
      if (columns[col].rowspan == 0)
      {
        if ((var = htmlGetAttr(cells[row][col], "ROWSPAN")) != NULL)
          columns[col].rowspan = atoi((char *)var);

        if (columns[col].rowspan == 1)
	  columns[col].rowspan = 0;

        if (columns[col].rowspan > (num_rows - row))
	  columns[col].rowspan = num_rows - row;

	columns[col].span_height = 0.0f;
      }

      if (columns[col].rowspan < rowspan)
	rowspan = columns[col].rowspan;

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

      DEBUG_printf(("    colspan=%d, columns[%d].left=%.1f, "
                    "columns[%d].right=%.1f, cell=%p\n",
                    colspan, col, columns[col].left, col + colspan,
		    columns[col + colspan].right, cells[row][col]));

      *x        = columns[col].left;
      temp_y    = *y - cellpadding;
      temp_page = *page;
      tempspace = 0;

      if (row == 0 || cells[row][col] != cells[row - 1][col])
      {
        check_pages(*page);

        if (cells[row][col] == NULL)
	  bgcolor = NULL;
	else if ((bgcolor = htmlGetAttr(cells[row][col],
                                            "BGCOLOR")) != NULL)
	{
	  memcpy(bgrgb, background_color, sizeof(bgrgb));

          get_color(bgcolor, bgrgb, 0);

	  width       = columns[col + colspan].right - columns[col].left +
        	        2 * cellpadding;
	  border_left = columns[col].left - cellpadding;

          columns[col].bg_render = new_render(*page, HD_RENDER_BOX, border_left,
	                                      row_y, width + border, 0.0,
					      bgrgb);
	}
	else
	{
	  columns[col].bg_render = NULL;

          new_render(*page, HD_RENDER_TEXT, -1.0f, -1.0f, 0.0, 0.0, (void *)"");
	}

        DEBUG_printf(("columns[%d].bg_render=%p, pages[%d].end=%p\n",
	              col, columns[col].bg_render, *page, pages[*page].end));

	columns[col].start_render = pages[*page].end;
	columns[col].start_page   = temp_page;
	columns[col].start_y      = temp_y;

#if 0 // TODO
        if (table_debug)
	{
	  check_pages(*page);

	  render_t *r;
	  char table_text[255];

	  snprintf(table_text, sizeof(table_text), "cell=%p [%d,%d]",
	           cells[row][col], row, col);
	  r = new_render(temp_page, RENDER_TEXT, *x, temp_y,
                	 get_width((hdChar *)table_text, TYPE_COURIER, STYLE_NORMAL, 1),
			 _htmlSizes[1], table_text);

	  r->data.text.typeface = TYPE_COURIER;
	  r->data.text.style    = STYLE_NORMAL;
	  r->data.text.size     = _htmlSizes[1];
	}
#endif // 0

        if (cells[row][col] != NULL && cells[row][col]->child != NULL)
	{
	  DEBUG_printf(("    parsing cell %d,%d; width=%.1f\n", row, col,
	                columns[col + colspan].right - columns[col].left));

          cell_margins = new hdMargin(columns[col].left,
	                              columns[col + colspan].right,
	                              margins->bottom() + cellpadding,
				      margins->top() - cellpadding);

          parse_doc(cells[row][col]->child, cell_margins,
                    x, &temp_y, &temp_page, NULL, &tempspace);

          cell_margins->end(temp_y, temp_page);

          delete cell_margins;
        }

        columns[col].end_page   = temp_page;
        columns[col].end_y      = temp_y;
        columns[col].height     = *y - cellpadding - temp_y;
        columns[col].end_render = pages[*page].end;

        if (columns[col].start_render == NULL)
	  columns[col].start_render = pages[*page].start;

        DEBUG_printf(("row=%d, col=%d, y=%.1f, cell_y=%.1f, cell_height=%.1f\n",
	              row, col, *y - cellpadding, temp_y, columns[col].height));
        DEBUG_printf(("start_render=%p, end_render=%p\n",
	              columns[col].start_render, columns[col].end_render));
      }

      if (columns[col].rowspan == 0 &&
          columns[col].start_page == columns[col].end_page &&
	  columns[col].height > row_height)
        row_height = columns[col].height;

      if (columns[col].rowspan <= rowspan)
      {
	if (columns[col].start_page != columns[col].end_page)
	  do_valign = 0;

        if (columns[col].end_page > row_page)
	{
	  row_page = columns[col].end_page;
	  row_y    = columns[col].end_y;
	}
	else if (columns[col].end_y < row_y && columns[col].end_page == row_page)
	  row_y = columns[col].end_y;
      }

      DEBUG_printf(("**** col=%d, row=%d, row_y=%.1f, row_page=%d\n",
                    col, row, row_y, row_page));

      for (col ++; colspan > 0; colspan --, col ++)
      {
        columns[col].start_render = NULL;
        columns[col].start_page   = columns[col - 1].start_page;
        columns[col].start_y      = columns[col - 1].start_y;
	columns[col].end_render   = NULL;
        columns[col].end_page     = columns[col - 1].end_page;
        columns[col].end_y        = columns[col - 1].end_y;
	columns[col].height       = columns[col - 1].height;
      }
    }

    DEBUG_printf(("row=%d, row_y=%.1f, row_height=%.1f\n", row, row_y, row_height));

    for (col = 0; col < num_cols; col += colspan)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;

      if (columns[col].rowspan)
        columns[col].span_height += row_height;

      DEBUG_printf(("col=%d, start_y=%.1f, start_page=%d, end_page=%d, "
                    "rowspan=%d, span_height=%.1f, height=%.1f\n",
                    col, columns[col].start_y, columns[col].start_page,
		    columns[col].end_page, columns[col].rowspans[col],
		    columns[col].span_height, columns[col].height));

      if (columns[col].rowspan == rowspan &&
          columns[col].start_page == columns[col].end_page &&
	  columns[col].height > columns[col].span_height)
      {
        temp_height = columns[col].height - columns[col].span_height;
	row_height  += temp_height;
	DEBUG_printf(("Adjusting row-span height by %.1f, new row_height=%.1f\n",
	              temp_height, row_height));

	for (tcol = 0; tcol < num_cols; tcol ++)
	  if (columns[col].rowspan)
	  {
	    columns[col].span_height += temp_height;
	    DEBUG_printf(("columns[%d].span_height=%.1f\n", tcol,
	                  columns[tcol].span_height));
	  }
      }
    }

    DEBUG_printf(("AFTER row %d: row_page=%d, row_y=%.1f, row_height=%.1f, "
                  "*y=%.1f, do_valign=%d\n",
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
          temp_height = atof((char *)height_var) * PagePrintWidth /
	                _htmlStyleSheet->browser_width;

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
        hdRender	*p;
        float		delta_y;


        for (colspan = 1; (col + colspan) < num_cols; colspan ++)
          if (cells[row][col] != cells[row][col + colspan])
            break;

        colspan --;

        if (columns[col].start_render == NULL ||
	    columns[col].rowspan > rowspan ||
	    cells[row][col] == NULL || cells[row][col]->child == NULL)
	  continue;

        if (columns[col].rowspan)
          switch (cells[row][col]->style->vertical_align)
	  {
            case HD_VERTICAL_ALIGN_MIDDLE :
        	delta_y = (columns[col].span_height -
		           columns[col].height) * 0.5f;
        	break;

            case HD_VERTICAL_ALIGN_BOTTOM :
        	delta_y = columns[col].span_height - columns[col].height;
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }
	else
          switch (cells[row][col]->style->vertical_align)
	  {
            case HD_VERTICAL_ALIGN_MIDDLE :
        	delta_y = (row_height - columns[col].height) * 0.5f;
        	break;

            case HD_VERTICAL_ALIGN_BOTTOM :
        	delta_y = row_height - columns[col].height;
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }

	DEBUG_printf(("row=%d, col=%d, valign=%d, height=%.1f, "
	              "span_height=%.1f, delta_y=%.1f\n",
	              row, col, cells[row][col]->style->vertical_align,
		      columns[col].height, columns[col].span_height, delta_y));

        if (delta_y > 0.0f)
	{
	  if (columns[col].start_render == columns[col].end_render)
	    p = columns[col].start_render;
	  else
	    p = columns[col].start_render->next;

          for (; p != NULL; p = p->next)
	  {
	    DEBUG_printf(("aligning %p (%s), y was %.1f, now %.1f\n",
	                  p, p->data.text.buffer, p->y, p->y - delta_y));

            p->y -= delta_y;
            if (p == columns[col].end_render)
	      break;
          }
        }
#ifdef DEBUG
        else
	{
	  if (columns[col].start_render == columns[col].end_render)
	    p = columns[col].start_render;
	  else
	    p = columns[col].start_render->next;

          for (; p != NULL; p = p->next)
	  {
	    printf("NOT aligning %p\n", p);

            if (p == columns[col].end_render)
	      break;
          }
	}
#endif /* DEBUG */
      }
    }

    // Update all current columns with ROWSPAN <= rowspan to use the same
    // end page and row...
    for (col = 0, temp_page = -1, temp_y = 99999999; col < num_cols; col ++)
      if (columns[col].rowspan <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
      {
        if (columns[col].end_page > temp_page)
	{
          temp_page = columns[col].end_page;
	  temp_y    = columns[col].end_y;
	}
        else if (columns[col].end_page == temp_page &&
	         columns[col].end_y < temp_y)
	  temp_y = columns[col].end_y;
      }

    for (col = 0; col < num_cols; col ++)
      if (columns[col].rowspan <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
      {
        columns[col].end_page = temp_page;
	columns[col].end_y    = temp_y;
      }

    row_y -= cellpadding;

    border_left = columns[0].left - cellpadding;
    width       = columns[num_cols - 1].right - columns[0].left +
                  2 * cellpadding;

    for (bgcolor = NULL, col = 0; col < num_cols; col ++)
      if (columns[col].rowspan <= rowspan &&
          cells[row][col] &&
	  !htmlGetAttr(cells[row][col], "ROWSPAN") &&
          (bgcolor = htmlGetAttr(cells[row][col]->parent,
                                 "BGCOLOR")) != NULL)
        break;

    if (bgcolor)
    {
      memcpy(bgrgb, background_color, sizeof(bgrgb));

      get_color(bgcolor, bgrgb, 0);

      if (row_page > *page)
      {
        // Draw background on multiple pages...

	// Bottom of first page...
        new_render(*page, HD_RENDER_BOX, border_left, margins->bottom0(),
	           width, row_starty - margins->bottom0() + cellpadding, bgrgb,
		   pages[*page].start);

        // Intervening pages...
        for (temp_page = *page + 1; temp_page < row_page; temp_page ++)
	{
          new_render(temp_page, HD_RENDER_BOX, border_left, margins->bottom0(),
                     width, margins->length(), bgrgb, pages[temp_page].start);
        }

        // Top of last page...
	check_pages(row_page);

        new_render(row_page, HD_RENDER_BOX, border_left, row_y,
	           width, margins->top() - row_y, bgrgb,
		   pages[row_page].start);
      }
      else
      {
        // Draw background in row...
        new_render(row_page, HD_RENDER_BOX, border_left, row_y,
	           width, row_height + 2 * cellpadding, bgrgb,
		   pages[row_page].start);
      }
    }

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      for (colspan = 0; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
	else if (columns[col + colspan].rowspan > 0)
	{
          DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
	        	col, columns[col + colspan].rowspan,
			columns[col + colspan].rowspan - rowspan));
          columns[col + colspan].rowspan -= rowspan;
	}

      colspan --;

      width = columns[col + colspan].right - columns[col].left +
              2 * cellpadding;

      if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
          columns[col].rowspan > 0)
        continue;

      DEBUG_printf(("DRAWING BORDER+BACKGROUND: col=%d, row=%d, cell_page=%d, cell_y=%.1f\n"
                    "                           cell_endpage=%d, cell_endy=%.1f\n",
		    col, row, columns[col].start_page, columns[col].start_y,
		    columns[col].end_page, columns[col].end_y));

      if ((bgcolor = htmlGetAttr(cells[row][col], "BGCOLOR")) != NULL)
      {
        memcpy(bgrgb, background_color, sizeof(bgrgb));

        get_color(bgcolor, bgrgb, 0);
      }

      border_left = columns[col].left - cellpadding;

      if (columns[col].start_page != columns[col].end_page)
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
          new_render(columns[col].start_page, HD_RENDER_BOX, border_left,
                     columns[col].start_y + cellpadding,
		     width + border, border, rgb);
	  // Left
          new_render(columns[col].start_page, HD_RENDER_BOX, border_left,
	             margins->bottom0(), border,
		     columns[col].start_y - margins->bottom0() + cellpadding +
		         border, rgb);
	  // Right
          new_render(columns[col].start_page, HD_RENDER_BOX,
	             border_left + width, margins->bottom0(), border,
		     columns[col].start_y - margins->bottom0() + cellpadding +
		         border, rgb);
        }

        if (bgcolor != NULL)
        {
	  columns[col].bg_render->y      = margins->bottom0();
	  columns[col].bg_render->height = columns[col].start_y -
	                                   margins->bottom0() + cellpadding +
					   border;
	}

        for (temp_page = columns[col].start_page + 1;
	     temp_page < columns[col].end_page;
	     temp_page ++)
	{
	 /*
	  * |   |   |   |
	  * |   |   |   |
	  */

	  if (border > 0.0f)
	  {
	    // Left
            new_render(temp_page, HD_RENDER_BOX, border_left, margins->bottom0(),
                       border, margins->length(), rgb);
	    // Right
            new_render(temp_page, HD_RENDER_BOX,
	               border_left + width, margins->bottom0(),
		       border, margins->length(), rgb);
          }

	  if (bgcolor != NULL)
            new_render(temp_page, HD_RENDER_BOX, border_left, margins->bottom0(),
                       width + border, margins->length(), bgrgb,
		       pages[temp_page].start);
        }

        if (border > 0.0f)
	{
	 /*
	  * |   |   |   |
	  * +---+---+---+
	  */

	  // Left
          new_render(columns[col].end_page, HD_RENDER_BOX, border_left, row_y,
                     border, margins->top() - row_y, rgb);
	  // Right
          new_render(columns[col].end_page, HD_RENDER_BOX,
	             border_left + width, row_y,
                     border, margins->top() - row_y, rgb);
	  // Bottom
          new_render(columns[col].end_page, HD_RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
        }

        if (bgcolor != NULL)
	{
	  check_pages(columns[col].end_page);

          new_render(columns[col].end_page, HD_RENDER_BOX, border_left, row_y,
	             width + border, margins->top() - row_y, bgrgb,
		     pages[columns[col].end_page].start);
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
          new_render(columns[col].start_page, HD_RENDER_BOX, border_left,
                     columns[col].start_y + cellpadding, width + border,
		     border, rgb);
	  // Left
          new_render(columns[col].start_page, HD_RENDER_BOX, border_left, row_y,
                     border,
		     columns[col].start_y - row_y + cellpadding + border, rgb);
	  // Right
          new_render(columns[col].start_page, HD_RENDER_BOX,
	             border_left + width, row_y, border,
		     columns[col].start_y - row_y + cellpadding + border, rgb);
	  // Bottom
          new_render(columns[col].start_page, HD_RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
	}

        if (bgcolor != NULL)
	{
	  columns[col].bg_render->y      = row_y;
	  columns[col].bg_render->height = columns[col].start_y - row_y +
	                                   cellpadding + border;
	}
      }
    }

    *page = row_page;
    *y    = row_y;

    if (row < (num_rows - 1))
      (*y) -= cellspacing;

    DEBUG_printf(("END row=%d, *y=%.1f, *page=%d\n", row, *y, *page));
  }

 /*
  * Handle table background color...
  */

  if ((bgcolor = htmlGetAttr(t, "BGCOLOR")) != NULL)
  {
    memcpy(bgrgb, background_color, sizeof(bgrgb));

    get_color(bgcolor, bgrgb, 0);

    border_left = columns[0].left - cellpadding;
    width       = columns[num_cols - 1].right - columns[0].left +
                  2 * cellpadding;

    if (table_page != *page)
    {
      // Draw background on multiple pages...

      // Bottom of first page...
      new_render(table_page, HD_RENDER_BOX, border_left, margins->bottom0(),
	         width, table_y - margins->bottom0(), bgrgb,
		 pages[table_page].start);

      // Intervening pages...
      for (temp_page = table_page + 1; temp_page < *page; temp_page ++)
      {
        new_render(temp_page, HD_RENDER_BOX, border_left, margins->bottom0(),
                   width, margins->length(), bgrgb, pages[temp_page].start);
      }

      // Top of last page...
      check_pages(*page);

      new_render(*page, HD_RENDER_BOX, border_left, *y,
	         width, margins->top() - *y, bgrgb, pages[*page].start);
    }
    else
    {
      // Draw background in row...
      new_render(table_page, HD_RENDER_BOX, border_left, *y,
	         width, table_y - *y, bgrgb, pages[table_page].start);
    }
  }

  margins->adjust_bottom(cellpadding);
  margins->adjust_top(-cellpadding);

  if (caption)
  {
   /*
    * Show caption at bottom...
    */

    *needspace = 0;
    parse_paragraph(caption, margins, x, y, page, needspace);
  }

  margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);

  margins->clear(*y, *page);

  *x = margins->left();
  *needspace = t->style->margin[HD_POS_BOTTOM];

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
parse_list(hdTree   *t,			/* I - Tree to parse */
           hdMargin *margins,		/* I - Margins */
           float    *x,			/* IO - X position */
           float    *y,			/* IO - Y position */
           int      *page,		/* IO - Page # */
           float    *needspace)		/* I - Need whitespace? */
{
  hdChar	number[255];		/* List number (for numbered types) */
  hdChar	*value;			/* VALUE= variable */
  float		width;			/* Width of list number */
  hdRender	*r;			/* Render primitive */
  int		oldpage;		/* Old page value */
  float		oldy;			/* Old Y value */
  float		tempx,			/* Temporary X value */
		numberx;		/* X position for number */
  hdStyleFont	*font;			/* Font */


  DEBUG_printf(("parse_list(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page));

  // Build the prefix string...
  if ((value = htmlGetAttr(t, "VALUE")) != NULL)
  {
    if (isdigit(value[0]))
      list_values[list_indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values[list_indent] = value[0] - 'A' + 1;
    else
      list_values[list_indent] = value[0] - 'a' + 1;
  }

  switch (list_types[list_indent])
  {
    case '\0' :
        number[0] = '\0';
        font = t->style->font;
	break;

    case 'a' :
    case 'A' :
    case '1' :
    case 'i' :
    case 'I' :
        strlcpy((char *)number, format_number(list_values[list_indent],
	                                      list_types[list_indent]),
		sizeof(number));
        strlcat((char *)number, ". ", sizeof(number));
        font = t->style->font;
        break;

    default :
        sprintf((char *)number, "%c   ", list_types[list_indent]);
        font = _htmlStyleSheet->find_font("symbol", HD_FONT_INTERNAL_NORMAL);
        break;
  }

  width = font->get_width(number) * t->style->font_size;

  // Add leading whitespace...
  if (*needspace < t->style->margin[HD_POS_TOP])
    *needspace = t->style->margin[HD_POS_TOP];

  if (*y < margins->top() && *needspace)
  {
    *y -= *needspace;
    *needspace = 0;
  }

  margins->clear(*y, *page);

  // Output the list item text...
  check_pages(*page);

  margins->adjust_left(t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(-t->style->margin[HD_POS_RIGHT]);

  if (t->style &&
      t->style->list_style_position == HD_LIST_STYLE_POSITION_INSIDE)
  {
    numberx = margins->left();
    margins->push(margins->left() + width, margins->right(),
                  *y - t->style->line_height);
  }
  else
    numberx = margins->left() - width;

  tempx = margins->left();

  oldy    = *y;
  oldpage = *page;
  r       = pages[*page].end;

  parse_doc(t->child, margins, &tempx, y, page, NULL, needspace);

  margins->adjust_left(-t->style->margin[HD_POS_LEFT]);
  margins->adjust_right(t->style->margin[HD_POS_RIGHT]);
  *x = margins->left();

  // Handle when paragraph wrapped to new page...
  if (*page != oldpage)
  {
    // First see if anything was added to the old page...
    if ((r != NULL && r->next == NULL) || pages[oldpage].end == NULL)
    {
      // No, put the symbol on the next page...
      oldpage = *page;
      oldy    = margins->top();
    }
  }

  if (!list_types[list_indent])
    return;

  r = new_render(oldpage, HD_RENDER_TEXT, numberx, oldy - t->style->font_size,
                 width, t->style->font_size, number);
  r->data.text.typeface = font->typeface;
  r->data.text.style    = font->style;
  r->data.text.size     = t->style->font_size;
  r->data.text.rgb[0]   = t->style->color[0] / 255.0f;
  r->data.text.rgb[1]   = t->style->color[1] / 255.0f;
  r->data.text.rgb[2]   = t->style->color[2] / 255.0f;

  list_values[list_indent] ++;
}


/*
 * 'init_list()' - Initialize the list type and value as necessary.
 */

static void
init_list(hdTree *t)			/* I - List entry */
{
  hdChar	*type,			/* TYPE= variable */
		*value;			/* VALUE= variable */
  static hdChar	*symbols = (hdChar *)"\327\267\250\340";


  if (list_indent < (int)(sizeof(list_types) / sizeof(list_types[0])))
    list_indent ++;

  DEBUG_printf(("init_list: t->style=%p\n", t->style));

  if (t->style)
  {
    DEBUG_printf(("list_style_type=%d\n", t->style->list_style_type));
    DEBUG_printf(("margin[HD_POS_LEFT]=%g\n", t->style->margin[HD_POS_LEFT]));
    DEBUG_printf(("margin_rel[HD_POS_LEFT]=%s\n", t->style->margin_rel[HD_POS_LEFT]));

    switch (t->style->list_style_type)
    {
      default :
      case HD_LIST_STYLE_TYPE_NONE :
          list_types[list_indent] = '\0';
          break;
      case HD_LIST_STYLE_TYPE_DISC :
          list_types[list_indent] = symbols[0];
          break;
      case HD_LIST_STYLE_TYPE_CIRCLE :
          list_types[list_indent] = symbols[1];
          break;
      case HD_LIST_STYLE_TYPE_SQUARE :
          list_types[list_indent] = symbols[2];
          break;
      case HD_LIST_STYLE_TYPE_DECIMAL :
          list_types[list_indent] = '1';
          break;
      case HD_LIST_STYLE_TYPE_LOWER_ROMAN :
          list_types[list_indent] = 'i';
          break;
      case HD_LIST_STYLE_TYPE_UPPER_ROMAN :
          list_types[list_indent] = 'I';
          break;
      case HD_LIST_STYLE_TYPE_LOWER_ALPHA :
          list_types[list_indent] = 'a';
          break;
      case HD_LIST_STYLE_TYPE_UPPER_ALPHA :
          list_types[list_indent] = 'A';
          break;
    }
  }
  else if ((type = htmlGetAttr(t, "TYPE")) != NULL)
  {
    if (strlen((char *)type) == 1)
      list_types[list_indent] = type[0];
    else if (strcasecmp((char *)type, "disc") == 0 ||
             strcasecmp((char *)type, "circle") == 0)
      list_types[list_indent] = symbols[1];
    else
      list_types[list_indent] = symbols[2];
  }
  else if (t->element == HD_ELEMENT_UL)
    list_types[list_indent] = symbols[list_indent & 3];
  else if (t->element == HD_ELEMENT_OL)
    list_types[list_indent] = '1';

  if ((value = htmlGetAttr(t, "VALUE")) == NULL)
    value = htmlGetAttr(t, "START");

  if (value != NULL)
  {
    if (isdigit(value[0]))
      list_values[list_indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values[list_indent] = value[0] - 'A' + 1;
    else
      list_values[list_indent] = value[0] - 'a' + 1;
  }
  else if (t->element == HD_ELEMENT_OL)
    list_values[list_indent] = 1;
}


/*
 * 'parse_comment()' - Parse a comment for HTMLDOC comments.
 */

//#define COMMENT_DEBUG
#ifdef COMMENT_DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x) puts(x)
#  define DEBUG
#  undef DEBUG_printf
#  define DEBUG_printf(x) printf x
#endif /* COMMENT_DEBUG */

static void
parse_comment(hdTree   *t,		/* I - Tree to parse */
              hdMargin *margins,	/* I - Margins */
              float    *x,		/* IO - X position */
              float    *y,		/* IO - Y position */
              int      *page,		/* IO - Page # */
	      hdTree   *para,		/* I - Current paragraph */
	      float    *needspace)	/* I - Need whitespace? */
{
  int		i;			/* Looping var */
  const char	*comment;		/* Comment text */
  char		*ptr,			/* Pointer into value string */
		buffer[1024];		/* Buffer for strings */
  int		pos,			/* Position (left, center, right) */
		tof;			/* Top of form */
  float		right,			// Right margin
		top;			// Top margin


  DEBUG_printf(("parse_comment(t=%p, margins=(%.1f, %.1f, %.1f, %.1f), x=%.1f, y=%.1f, page=%d, para=%p, needspace=%g\n",
                t, margins->left(), margins->right(), margins->bottom(),
		margins->top(), *x, *y, *page, para, *needspace));

  if (t->data == NULL)
    return;

  if (para != NULL && para->child != NULL && para->child->next == NULL &&
      para->child->child == NULL && para->child->element == HD_ELEMENT_NONE &&
      strcmp((const char *)para->child->data, " ") == 0)
  {
    // Remove paragraph consisting solely of whitespace...
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
  }

  // Mark if we are at the top of form...
  tof = (*y >= margins->top());

  DEBUG_printf(("BEFORE tof=%d, *y=%.1f, *page=%d, top()=%.1f, right()=%.1f\n",
        	tof, *y, *page, margins->top(), margins->right()));
  DEBUG_printf(("        t->data = \"%s\"\n", t->data));
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = margins->left();
      *y = margins->top();

      tof = 1;

      margins->clear(*y, *page);
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = margins->left();
      *y = margins->top();

      tof = 1;

      margins->clear(*y, *page);
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
	parse_paragraph(para, margins, x, y, page, needspace);
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

      *x = margins->left();
      *y = margins->top();

      tof = 1;

      margins->clear(*y, *page);
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      halfway = 0.5f * (margins->top() + margins->bottom0());

      if (*y <= halfway)
      {
	(*page) ++;
	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*x = margins->left();
	*y = margins->top();

        tof = 1;
      }
      else
      {
	*x = margins->left();
	*y = halfway;

        tof = 0;
      }

      margins->clear(*y, *page);
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if ((*y - get_measurement(comment, t->style->line_height)) < margins->bottom0())
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

        margins->clear(*y, *page);
      }

      *x = margins->left();

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

        margins->clear(*y, *page);
      }

      *x = margins->left();

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

        margins->clear(*y, *page);
      }

      *x = margins->left();

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (PageDuplex && ((*page) & 1))
	  (*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

        margins->clear(*y, *page);
      }

      *x = margins->left();

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

        tof = 1;

        margins->clear(*y, *page);
      }

      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      check_pages(*page);

      right = PagePrintWidth - margins->right();
      top   = PagePrintLength - margins->top();

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

      margins->adjust_right(PagePrintWidth - right - margins->right());
      margins->adjust_top(PagePrintLength - top - margins->top());

      *x = margins->left();
      *y = margins->top();

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

	margins->clear(*y, *page);
      }

      *x = margins->left();

      check_pages(*page);

      right    = PagePrintWidth - margins->right();
      PageLeft = pages[*page].left = get_measurement(comment);

      if (Landscape)
	PagePrintWidth = PageLength - PageRight - PageLeft;
      else
	PagePrintWidth = PageWidth - PageRight - PageLeft;

      margins->adjust_right(PagePrintWidth - right - margins->right());

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
	*y = margins->top();
        tof = 1;

	margins->clear(*y, *page);
      }

      *x = margins->left();

      check_pages(*page);

      right     = PagePrintWidth - margins->right();
      PageRight = pages[*page].right = get_measurement(comment);

      if (Landscape)
	PagePrintWidth = PageLength - PageRight - PageLeft;
      else
	PagePrintWidth = PageWidth - PageRight - PageLeft;

      margins->adjust_right(PagePrintWidth - right - margins->right());

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
        tof = 1;
      }

      *x = margins->left();

      check_pages(*page);

      top        = PagePrintLength - margins->top();
      PageBottom = pages[*page].bottom = get_measurement(comment);

      if (Landscape)
        PagePrintLength = PageWidth - PageTop - PageBottom;
      else
        PagePrintLength = PageLength - PageTop - PageBottom;

      margins->adjust_top(PagePrintLength - top - margins->top());
      *y   = margins->top();

      margins->clear(*y, *page);

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);

        tof = 1;
      }

      *x = margins->left();

      check_pages(*page);

      top     = PagePrintLength - margins->top();
      PageTop = pages[*page].top = get_measurement(comment);

      if (Landscape)
        PagePrintLength = PageWidth - PageTop - PageBottom;
      else
        PagePrintLength = PageLength - PageTop - PageBottom;

      margins->adjust_top(PagePrintLength - top - margins->top());
      *y   = margins->top();

      margins->clear(*y, *page);

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
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

      *x = margins->left();

      check_pages(*page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
      {
        if (Landscape)
	{
	  right          = PageLength - PageRight - margins->right();
	  PagePrintWidth = PageWidth - PageRight - PageLeft;
	  margins->adjust_right(PageWidth - PageLength);

	  top             = PageWidth - PageTop - margins->top();
	  PagePrintLength = PageLength - PageTop - PageBottom;
	  margins->adjust_top(PageLength - PageWidth);
        }

        Landscape              = 0;
	pages[*page].landscape = 0;
      }
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
        if (!Landscape)
	{
	  top             = PageLength - PageTop - margins->top();
	  PagePrintLength = PageWidth - PageTop - PageBottom;
	  margins->adjust_top(PageWidth - PageLength);

	  right          = PageWidth - PageRight - margins->right();
	  PagePrintWidth = PageLength - PageRight - PageLeft;
	  margins->adjust_right(PageLength - PageWidth);
        }

        Landscape              = 1;
	pages[*page].landscape = 1;
      }

      *y = margins->top();

      margins->clear(*y, *page);

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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
      }

      if (!tof)
      {
	(*page) ++;

	*y = margins->top();
        tof = 1;
      }

      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      margins->clear(*y, *page);

      if (Verbosity)
	progress_show("Formatting page %d", *page);

      *x = margins->left();

      check_pages(*page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
      {
        PageDuplex          = 0;
	pages[*page].duplex = 0;
      }
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
	if ((*page) & 1)
	{
	  (*page) ++;

          check_pages(*page);

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
	}

        PageDuplex          = 1;
	pages[*page].duplex = 1;
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
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

	pages[*page].header[pos] = (hdChar *)Header[pos];
      }

      // Adjust top margin as needed...
      float adjust, image_adjust, temp_adjust;

      if (maxhfheight > max_hfsize)
	image_adjust = maxhfheight + max_hfsize;
      else
	image_adjust = 2 * max_hfsize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Header[pos])
	{
	  if (strstr(Header[pos], "$IMAGE") != NULL ||
	      strstr(Header[pos], "$HFIMAGE") != NULL)
	    temp_adjust = image_adjust;
	  else
	    temp_adjust = 2 * max_hfsize;

	  if (temp_adjust > adjust)
	    adjust = temp_adjust;
	}

	if (Header1[pos])
	{
	  if (strstr(Header1[pos], "$IMAGE") != NULL ||
	      strstr(Header1[pos], "$HFIMAGE") != NULL)
	    temp_adjust = image_adjust;
	  else
	    temp_adjust = 2 * max_hfsize;

	  if (temp_adjust > adjust)
	    adjust = temp_adjust;
	}
      }

      margins->adjust_top(PagePrintLength - adjust - margins->top());

      if (tof)
        *y = margins->top();
    }
    else if (strncasecmp(comment, "HEADER1 ", 8) == 0)
    {
      // First page header string...
      comment += 8;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
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

      if (maxhfheight > max_hfsize)
	image_adjust = maxhfheight + max_hfsize;
      else
	image_adjust = 2 * max_hfsize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Header[pos])
	{
	  if (strstr(Header[pos], "$IMAGE") != NULL ||
	      strstr(Header[pos], "$HFIMAGE") != NULL)
	    temp_adjust = image_adjust;
	  else
	    temp_adjust = 2 * max_hfsize;

	  if (temp_adjust > adjust)
	    adjust = temp_adjust;
	}

	if (Header1[pos])
	{
	  if (strstr(Header1[pos], "$IMAGE") != NULL ||
	      strstr(Header1[pos], "$HFIMAGE") != NULL)
	    temp_adjust = image_adjust;
	  else
	    temp_adjust = 2 * max_hfsize;

	  if (temp_adjust > adjust)
	    adjust = temp_adjust;
	}
      }

      margins->adjust_top(PagePrintLength - adjust - margins->top());

      if (tof)
      {
        DEBUG_printf(("Setting header1 %d for page %d to \"%s\"...\n",
	              pos, *page, Header1[pos] ? Header1[pos] : "(null)"));

	check_pages(*page);

	pages[*page].header1[pos] = (hdChar *)Header1[pos];
      }
    }
    else if (strncasecmp(comment, "FOOTER ", 7) == 0)
    {
      // Footer string...
      comment += 7;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
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

	pages[*page].footer[pos] = (hdChar *)Footer[pos];
      }

      // Adjust bottom margin as needed...
      float adjust, image_adjust, temp_adjust;

      if (maxhfheight > max_hfsize)
	image_adjust = maxhfheight + max_hfsize;
      else
	image_adjust = 2 * max_hfsize;

      for (adjust = 0.0, pos = 0; pos < 3; pos ++)
      {
	if (Footer[pos])
	{
	  if (strstr(Footer[pos], "$IMAGE") != NULL ||
	      strstr(Footer[pos], "$HFIMAGE") != NULL)
	    temp_adjust = image_adjust;
	  else
	    temp_adjust = 2 * max_hfsize;

	  if (temp_adjust > adjust)
	    adjust = temp_adjust;
	}
      }

      if (tof)
        margins->adjust_bottom(adjust - margins->bottom0());
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
	parse_paragraph(para, margins, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;

	// Mark if we are still at the top of form...
	tof = (*y >= margins->top());
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

  DEBUG_printf(("LEAVING parse_comment() x=%.1f, y=%.1f, page=%d, "
                "top()=%.1f, right()=%.1f\n",
		*x, *y, *page, margins->top(), margins->right()));
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
 * 'real_prev()' - Return the previous non-link markup in the tree.
 */

static hdTree *		/* O - Pointer to previous markup */
real_prev(hdTree *t)	/* I - Current markup */
{
  if (t == NULL)
    return (NULL);

  if (t->prev != NULL &&
      (t->prev->element == HD_ELEMENT_A || t->prev->element == HD_ELEMENT_COMMENT))
    t = t->prev;

  if (t->prev != NULL)
    return (t->prev);

  t = t->parent;
  if (t == NULL)
    return (NULL);

  if (t->element != HD_ELEMENT_A && t->element != HD_ELEMENT_EMBED &&
      t->element != HD_ELEMENT_COMMENT)
    return (t);
  else
    return (real_prev(t));
}


/*
 * 'real_next()' - Return the next non-link markup in the tree.
 */

static hdTree *		/* O - Pointer to next markup */
real_next(hdTree *t)	/* I - Current markup */
{
  if (t == NULL)
    return (NULL);

  if (t->next != NULL &&
      (t->next->element == HD_ELEMENT_A || t->next->element == HD_ELEMENT_COMMENT))
    t = t->next;

  if (t->next != NULL)
    return (t->next);

  return (real_next(t->parent));
}


/*
 * 'find_background()' - Find the background image/color for the given document.
 */

static void
find_background(hdTree *t)	/* I - Document to search */
{
  hdChar		*var;		/* BGCOLOR/BACKGROUND variable */


 /*
  * First see if the --bodycolor or --bodyimage options have been
  * specified...
  */

  if (BodyColor[0] != '\0')
    get_color((hdChar *)BodyColor, background_color, 0);

  if (BodyImage || BodyColor[0])
    return;

 /*
  * If not, search the document tree...
  */

  while (t != NULL && BodyImage == NULL &&
         background_color[0] == 1.0 && background_color[1] == 1.0 &&
	 background_color[2] == 1.0)
  {
    if (t->element == HD_ELEMENT_BODY)
    {
      if ((var = htmlGetAttr(t, "BACKGROUND")) != NULL)
        BodyImage = hdImage::find((char *)var, !OutputColor, Path);

      if ((var = htmlGetAttr(t, "BGCOLOR")) != NULL)
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
write_background(hdFile *out,		/* I - File to write to */
		 int    page)		/* I - Page we are writing for */
                 
{
  float	x, y;				// Current position
  float	width, height;			// Width and height of image
  int	page_width, page_length;	// Width and height of page


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
      out->printf("0 0 M %d %d F\n", page_width, page_length);
    }
    else
    {
      set_color(out, background_color);
      out->printf("0 0 %d %d re f\n", page_width, page_length);
    }
  }

  if (BodyImage)
  {
    width  = BodyImage->width() * 72.0f / _htmlStyleSheet->ppi;
    height = BodyImage->height() * 72.0f / _htmlStyleSheet->ppi;

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
  	      out->printf("q %.1f 0 0 %.1f %.1f %.1f cm", width, height,
	                   x, y);
              out->printf("/I%d Do\n", BodyImage->obj());
	      out->puts("Q\n");
            }
	  break;

      default :
          out->printf("0 %.1f %d{/y exch neg %d add def\n",
	              height, page_length + (int)height - 1, page_length);
	  out->printf("0 %.1f %d{/x exch def\n",
	              width, page_width);
          out->printf("GS[%.1f 0 0 %.1f x y]CM/iy -1 def\n", width, height);
	  out->printf("%d %d 8[%d 0 0 %d 0 %d]",
	              BodyImage->width(), BodyImage->height(),
                      BodyImage->width(), -BodyImage->height(),
		      BodyImage->height());
          out->puts("{/iy iy 1 add def BG iy get}");
	  if (BodyImage->depth() == 1)
	    out->puts("image\n");
	  else
	    out->puts("false 3 colorimage\n");
	  out->puts("GR}for}for\n");
          break;
    }
  }
}


/*
 * 'new_render()' - Allocate memory for a new rendering structure.
 */

static hdRender *		/* O - New render structure */
new_render(int      page,	/* I - Page number (0-n) */
           int      type,	/* I - Type of render primitive */
           float    x,		/* I - Horizontal position */
           float    y,		/* I - Vertical position */
           float    width,	/* I - Width */
           float    height,	/* I - Height */
           void     *data,	/* I - Data */
	   hdRender *insert)	/* I - Insert before here... */
{
  hdRender	*r;		/* New render primitive */
  size_t	datalen = 0;	/* Length of text/link */
  static hdRender dummy;	/* Dummy var for errors... */


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

  if ((type != HD_RENDER_TEXT && type != HD_RENDER_LINK) || data == NULL)
    r = (hdRender *)calloc(sizeof(hdRender), 1);
  else
  {
    datalen = strlen((char *)data);
    r       = (hdRender *)calloc(sizeof(hdRender) + datalen, 1);
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
    case HD_RENDER_TEXT :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
	// Safe because buffer is allocated...
        memcpy((char *)r->data.text.buffer, (char *)data, datalen);
        r->data.text.rgb[0] = _htmlStyleSheet->def_style.color[0] / 255.0;
        r->data.text.rgb[1] = _htmlStyleSheet->def_style.color[1] / 255.0;
        r->data.text.rgb[2] = _htmlStyleSheet->def_style.color[2] / 255.0;
        break;
    case HD_RENDER_IMAGE :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
        r->data.image = (hdImage *)data;
        break;
    case HD_RENDER_BOX :
        memcpy(r->data.box, data, sizeof(r->data.box));
        break;
    case HD_RENDER_LINK :
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
  hdPage	*temp;	// Temporary page pointer


  DEBUG_printf(("check_pages(%d)\n", page));

  // See if we need to allocate memory for the page...
  if (page >= alloc_pages)
  {
    // Yes, allocate enough for ALLOC_PAGES more pages...
    while (page >= alloc_pages)
      alloc_pages += ALLOC_PAGES;

    // Do the pages pointers...
    if (num_pages == 0)
      temp = (hdPage *)malloc(sizeof(hdPage) * alloc_pages);
    else
      temp = (hdPage *)realloc(pages, sizeof(hdPage) * alloc_pages);

    if (temp == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for %d pages - %s",
	             alloc_pages, strerror(errno));
      alloc_pages -= ALLOC_PAGES;
      return;
    }

    memset(temp + num_pages, 0, (alloc_pages - num_pages) * sizeof(hdPage));

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
	memcpy(temp, temp - 1, sizeof(hdPage));
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
      temp->background_image = BodyImage;
    }
  }
}


/*
 * 'add_link()' - Add a named link...
 */

static void
add_link(hdChar *name,		/* I - Name of link */
         int   page,		/* I - Page # */
         int   top)		/* I - Y position */
{
  hdRenderLink	*temp;		/* New name */


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
        temp = (hdRenderLink *)malloc(sizeof(hdRenderLink) * alloc_links);
      else
        temp = (hdRenderLink *)realloc(links, sizeof(hdRenderLink) * alloc_links);

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
      qsort(links, num_links, sizeof(hdRenderLink),
            (compare_func_t)compare_links);
  }
}


/*
 * 'find_link()' - Find a named link...
 */

static hdRenderLink *
find_link(hdChar *name)	/* I - Name to find */
{
  hdRenderLink	key,	/* Search key */
		*match;	/* Matching name entry */


  if (name == NULL || num_links == 0)
    return (NULL);

  if (name[0] == '#')
    name ++;

  strlcpy((char *)key.name, (char *)name, sizeof(key.name));
  match = (hdRenderLink *)bsearch(&key, links, num_links, sizeof(hdRenderLink),
                            (compare_func_t)compare_links);

  return (match);
}


/*
 * 'compare_links()' - Compare two named links.
 */

static int			/* O - 0 = equal, -1 or 1 = not equal */
compare_links(hdRenderLink *n1,	/* I - First name */
              hdRenderLink *n2)	/* I - Second name */
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


/*
 * 'copy_tree()' - Copy a markup tree...
 */

static void
copy_tree(hdTree *parent,	/* I - Source tree */
          hdTree *t)		/* I - Destination tree */
{
  int		i;		/* I - Looping var */
  hdTree	*temp;		/* I - New tree entry */
  hdTreeAttr		*var;		/* I - Current markup variable */


  while (t != NULL)
  {
    if ((temp = htmlAddTree(parent, t->element, t->data)) != NULL)
    {
      temp->link  = t->link;
      temp->style = t->style;

      for (i = 0, var = t->attrs; i < t->nattrs; i ++, var ++)
        htmlSetAttr(temp, var->name, var->value);

      copy_tree(temp, t->child);
    }

    t = t->next;
  }
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
get_cell_size(hdTree *t,		// I - Cell
              float  left,		// I - Left margin
	      float  right,		// I - Right margin
	      float  *minwidth,		// O - Minimum width
	      float  *prefwidth,	// O - Preferred width
	      float  *minheight)	// O - Minimum height
{
  hdTree	*temp,			// Current tree entry
		*next;			// Next tree entry
  hdChar	*var;			// Attribute value
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
  if ((var = htmlGetAttr(t, "WIDTH")) != NULL &&
      (var[strlen((char *)var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      width = (right - left) * atoi((char *)var) * 0.01f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlStyleSheet->browser_width;
  }
  else
    width = 0.0f;

  if ((format_width = right - left) <= 0.0f)
    format_width = PagePrintWidth;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = htmlGetAttr(t, "HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      minh = PagePrintLength * atoi((char *)var) * 0.01f;
    else
      minh = atoi((char *)var) * PagePrintWidth / _htmlStyleSheet->browser_width;
  }
  else
    minh = 0.0f;

  nowrap = (htmlGetAttr(t, "NOWRAP") != NULL);

  DEBUG_printf(("nowrap = %d\n", nowrap));

  for (temp = t->child, frag_width = 0.0f, frag_pref = 0.0f;
       temp != NULL;
       temp = next)
  {
    // Point to next markup, if any...
    next = temp->child;

    switch (temp->element)
    {
      case HD_ELEMENT_TABLE :
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

      case HD_ELEMENT_IMG :
          // Update the image width as needed...
	  if (temp->element == HD_ELEMENT_IMG)
	    update_image_size(temp);
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
          frag_height = temp->height;

#ifdef TABLE_DEBUG2
          if (temp->element == HD_ELEMENT_NONE)
	    printf("FRAG(%s) = %.1f\n", temp->data, temp->width);
	  else if (temp->element == HD_ELEMENT_SPACER)
	    printf("SPACER = %.1f\n", temp->width);
	  else
	    printf("IMG(%s) = %.1f\n", htmlGetAttr(temp, "SRC"),
	           temp->width);
#endif // TABLE_DEBUG2

          // Handle min/preferred widths separately...
          if (temp->width > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for fragment...\n",
	                  temp->width, minw));
	    minw = temp->width;
	  }

          if (temp->style->white_space == HD_WHITE_SPACE_PRE &&
	      temp->data != NULL &&
              temp->data[strlen((char *)temp->data) - 1] == '\n')
          {
	    // End of a line - check preferred width...
	    frag_pref += temp->width + 1;

            if (frag_pref > prefw)
              prefw = frag_pref;

            if (temp->style->white_space == HD_WHITE_SPACE_PRE && frag_pref > minw)
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

          if (temp->style->white_space == HD_WHITE_SPACE_PRE && temp->data != NULL &&
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
          else if (temp->style->white_space != HD_WHITE_SPACE_PRE && temp->data != NULL &&
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

      case HD_ELEMENT_ADDRESS :
      case HD_ELEMENT_BLOCKQUOTE :
      case HD_ELEMENT_BR :
      case HD_ELEMENT_CENTER :
      case HD_ELEMENT_DD :
      case HD_ELEMENT_DIV :
      case HD_ELEMENT_DT :
      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_H7 :
      case HD_ELEMENT_H8 :
      case HD_ELEMENT_H9 :
      case HD_ELEMENT_H10 :
      case HD_ELEMENT_H11 :
      case HD_ELEMENT_H12 :
      case HD_ELEMENT_H13 :
      case HD_ELEMENT_H14 :
      case HD_ELEMENT_H15 :
      case HD_ELEMENT_HR :
      case HD_ELEMENT_LI :
      case HD_ELEMENT_P :
      case HD_ELEMENT_PRE :
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
get_table_size(hdTree *t,		// I - Table
               float  left,		// I - Left margin
	       float  right,		// I - Right margin
	       float  *minwidth,	// O - Minimum width
	       float  *prefwidth,	// O - Preferred width
	       float  *minheight)	// O - Minimum height
{
  hdTree	*temp,			// Current tree entry
		*next;			// Next tree entry
  hdChar		*var;			// Attribute value
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
  if ((var = htmlGetAttr(t, "WIDTH")) != NULL &&
      (var[strlen((char *)var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      width = (right - left) * atoi((char *)var) * 0.01f;
    else
      width = atoi((char *)var) * PagePrintWidth / _htmlStyleSheet->browser_width;
  }
  else
    width = 0.0f;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = htmlGetAttr(t, "HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen((char *)var) - 1] == '%')
      minh = PagePrintLength * atoi((char *)var) * 0.01f;
    else
      minh = atoi((char *)var) * PagePrintWidth / _htmlStyleSheet->browser_width;
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
    if (temp->element == HD_ELEMENT_TR)
    {
      minh += row_height;

      row_width  = 0.0f;
      row_pref   = 0.0f;
      row_min    = 0.0f;
      row_height = 0.0f;
      rows ++;
      columns = 0;
    }
    else if (temp->element == HD_ELEMENT_TD || temp->element == HD_ELEMENT_TH)
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
  if ((var = htmlGetAttr(t, "CELLPADDING")) != NULL)
    cellpadding = atoi((char *)var);
  else
    cellpadding = 1.0f;

  if ((var = htmlGetAttr(t, "CELLSPACING")) != NULL)
    cellspacing = atoi((char *)var);
  else
    cellspacing = 0.0f;

  if ((var = htmlGetAttr(t, "BORDER")) != NULL)
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

  cellspacing *= PagePrintWidth / _htmlStyleSheet->browser_width;
  cellpadding *= PagePrintWidth / _htmlStyleSheet->browser_width;

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

static hdTree *			/* O - Flattened markup tree */
flatten_tree(hdTree *t)		/* I - Markup tree to flatten */
{
  hdTree	*temp,		/* New tree node */
		*flat;		/* Flattened tree */


  flat = NULL;

  while (t != NULL)
  {
    switch (t->element)
    {
      case HD_ELEMENT_NONE :
          if (t->data == NULL)
	    break;
      case HD_ELEMENT_BR :
      case HD_ELEMENT_SPACER :
      case HD_ELEMENT_IMG :
	  temp = (hdTree *)calloc(sizeof(hdTree), 1);
	  memcpy(temp, t, sizeof(hdTree));
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;

          if (temp->element == HD_ELEMENT_IMG)
            update_image_size(temp);
          break;

      case HD_ELEMENT_A :
          if (htmlGetAttr(t, "NAME") != NULL)
          {
	    temp = (hdTree *)calloc(sizeof(hdTree), 1);
	    memcpy(temp, t, sizeof(hdTree));
	    temp->parent = NULL;
	    temp->child  = NULL;
	    temp->prev   = flat;
	    temp->next   = NULL;
	    if (flat != NULL)
              flat->next = temp;
            flat = temp;
          }
	  break;

      case HD_ELEMENT_P :
      case HD_ELEMENT_PRE :
      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_H7 :
      case HD_ELEMENT_H8 :
      case HD_ELEMENT_H9 :
      case HD_ELEMENT_H10 :
      case HD_ELEMENT_H11 :
      case HD_ELEMENT_H12 :
      case HD_ELEMENT_H13 :
      case HD_ELEMENT_H14 :
      case HD_ELEMENT_H15 :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_OL :
      case HD_ELEMENT_DL :
      case HD_ELEMENT_LI :
      case HD_ELEMENT_DD :
      case HD_ELEMENT_DT :
      case HD_ELEMENT_TR :
      case HD_ELEMENT_CAPTION :
	  temp = (hdTree *)calloc(sizeof(hdTree), 1);
          temp->style   = t->style;
	  temp->element = HD_ELEMENT_BR;
	  temp->parent  = NULL;
	  temp->child   = NULL;
	  temp->prev    = flat;
	  temp->next    = NULL;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;
          break;

      default :
          break;
    }

    if (t->child != NULL)
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
update_image_size(hdTree *t)	/* I - Tree entry */
{
  hdImage	*img;		/* Image file */
  hdChar	*width,		/* Width string */
		*height;	/* Height string */


  width  = htmlGetAttr(t, "WIDTH");
  height = htmlGetAttr(t, "HEIGHT");

  if (width != NULL && height != NULL)
  {
    if (width[strlen((char *)width) - 1] == '%')
      t->width = atof((char *)width) * PagePrintWidth / 100.0f;
    else
      t->width = atoi((char *)width) * PagePrintWidth / _htmlStyleSheet->browser_width;

    if (height[strlen((char *)height) - 1] == '%')
      t->height = atof((char *)height) * PagePrintWidth / 100.0f;
    else
      t->height = atoi((char *)height) * PagePrintWidth / _htmlStyleSheet->browser_width;

    return;
  }

  img = hdImage::find((char *)htmlGetAttr(t, "_HD_SRC"));

  if (img == NULL)
    return;

  if (width != NULL)
  {
    if (width[strlen((char *)width) - 1] == '%')
      t->width = atof((char *)width) * PagePrintWidth / 100.0f;
    else
      t->width = atoi((char *)width) * PagePrintWidth / _htmlStyleSheet->browser_width;

    t->height = t->width * img->height() / img->width();
  }
  else if (height != NULL)
  {
    if (height[strlen((char *)height) - 1] == '%')
      t->height = atof((char *)height) * PagePrintWidth / 100.0f;
    else
      t->height = atoi((char *)height) * PagePrintWidth / _htmlStyleSheet->browser_width;

    t->width = t->height * img->width() / img->height();
  }
  else
  {
    t->width  = img->width() * PagePrintWidth / _htmlStyleSheet->browser_width;
    t->height = img->height() * PagePrintWidth / _htmlStyleSheet->browser_width;
  }
}


/*
 * 'get_title()' - Get the title string for a document.
 */

static hdChar *		/* O - Title string */
get_title(hdTree *doc)	/* I - Document */
{
  hdChar	*temp;


  while (doc != NULL)
  {
    if (doc->element == HD_ELEMENT_TITLE)
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

static hdFile *				/* O - File pointer */
open_file(void)
{
  char	filename[1024];			/* Filename */


  if (OutputFiles && PSLevel > 0)
  {
    if (chapter == -1)
      snprintf(filename, sizeof(filename), "%s/cover.ps", OutputPath);
    else if (chapter == 0)
      snprintf(filename, sizeof(filename), "%s/contents.ps", OutputPath);
    else
      snprintf(filename, sizeof(filename), "%s/doc%d.ps", OutputPath, chapter);

    return (hdFile::open(filename, HD_FILE_UPDATE));
  }
  else if (OutputFiles)
  {
    snprintf(filename, sizeof(filename), "%s/doc.pdf", OutputPath);

    return (hdFile::open(filename, HD_FILE_UPDATE));
  }
  else if (OutputPath[0] != '\0')
    return (hdFile::open(OutputPath, HD_FILE_UPDATE));
  else if (PSLevel == 0)
    return (hdFile::temp(stdout_filename, sizeof(stdout_filename)));
  else
    return (new hdStdFile(stdout, HD_FILE_WRITE));
}


/*
 * 'set_color()' - Set the current text color...
 */

static void
set_color(hdFile  *out,			/* I - File to write to */
          float   *rgb)			/* I - RGB color */
{
  if (rgb[0] == render_rgb[0] &&
      rgb[1] == render_rgb[1] &&
      rgb[2] == render_rgb[2])
    return;

  render_rgb[0] = rgb[0];
  render_rgb[1] = rgb[1];
  render_rgb[2] = rgb[2];

  if (OutputColor && (rgb[0] != rgb[1] || rgb[1] != rgb[2] || rgb[0] != rgb[2]))
  {
    // Output RGB color...
    if (PSLevel > 0)
      out->printf("%g %g %g C ", rgb[0], rgb[1], rgb[2]);
    else
      out->printf("%g %g %g rg ", rgb[0], rgb[1], rgb[2]);
  }
  else
  {
    // Output grayscale...
    if (PSLevel > 0)
      out->printf("%g G ", rgb[0] * 0.31f + rgb[1] * 0.61f + rgb[2] * 0.08f);
    else
      out->printf("%g g ",
                  rgb[0] * 0.31f + rgb[1] * 0.61f + rgb[2] * 0.08f);
  }
}


/*
 * 'set_font()' - Set the current text font.
 */

static void
set_font(hdFile *out,			/* I - File to write to */
         int    typeface,		/* I - Typeface code */
         int    style,			/* I - Style code */
         float  size)			/* I - Size */
{
  if (typeface == render_typeface &&
      style == render_style &&
      size == render_size)
    return;

 /*
  * Set the new typeface, style, and size.
  */

  if (PSLevel > 0)
  {
    if (size != render_size)
      out->printf("%g FS", size);

    out->printf("/F%x SF ", typeface * 4 + style);
  }
  else
    out->printf("/F%x %g Tf ", typeface * 4 + style, size);

  render_typeface = typeface;
  render_style    = style;
  render_size     = size;
}


/*
 * 'set_pos()' - Set the current text position.
 */

static void
set_pos(hdFile *out,			/* I - File to write to */
        float  x,			/* I - X position */
        float  y)			/* I - Y position */
{
  if (fabs(render_x - x) < 0.1 && fabs(render_y - y) < 0.1)
    return;

 /*
  * Format X and Y...
  */

  if (PSLevel > 0)
    out->printf("%g %g M", x, y);
  else if (render_x == -1.0)
     out->printf("%g %g Td", x, y);
  else
     out->printf("%g %g Td", x - render_startx, y - render_y);

  render_x = render_startx = x;
  render_y = y;
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
write_image(hdFile   *out,		/* I - Output file */
            hdRender *r,		/* I - Image to write */
	    int      write_obj)		/* I - Write an object? */
{
  int		i, j, k, m,		/* Looping vars */
		ncolors;		/* Number of colors */
  hdChar	*pixel,			/* Current pixel */
		*indices,		/* New indexed pixel array */
		*indptr;		/* Current index */
  int		indwidth,		/* Width of indexed line */
		indbits;		/* Bits per index */
  int		max_colors;		/* Max colors to use */
  unsigned	colors[256],		/* Colormap values */
		key,			/* Color key */
		*match;			/* Matching color value */
  hdChar	grays[256],		/* Grayscale usage */
		cmap[256][3];		/* Colormap */
  hdImage 	*img;			/* Image */
  hdChar	*data,			/* PS Level 3 image data */
		*dataptr,		/* Pointer into image data */
		*maskptr;		/* Pointer into mask data */
  hdFile	*filter;		/* Flate/JPEG/ASCII filter */
  hdArray	filters(NULL);		/* Array of filters */


 /*
  * See if we can optimize the image as indexed without color loss...
  */

  img      = r->data.image;
  ncolors  = 0;
  indices  = NULL;
  indwidth = 0;

  if (!img->pixels() && !img->obj())
    img->load();

  // Note: Acrobat 6 tries to decrypt the colormap of indexed in-line images twice, which
  //       is 1) not consistent with prior Acrobat releases and 2) in violation of their
  //       PDF spec.  The "img->use > 1 || !Encryption" test prevents the use of indexed
  //       in-line images when encryption is enabled.
  //
  //       We are filing a bug on this with Adobe, but if history is any indicator, we are
  //       stuck with this workaround forever...
  if (PSLevel != 1 && PDFVersion >= 12 && img->obj() == 0 &&
      (img->use() > 1 || !Encryption))
  {
    if (img->depth() == 1)
    {
     /*
      * Greyscale image...
      */

      memset(grays, 0, sizeof(grays));

      for (i = img->width() * img->height(), pixel = img->pixels();
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

      for (i = img->width() * img->height(), pixel = img->pixels(), match = NULL;
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
    if (PSLevel == 3 && img->mask())
      indbits = 8;
    else if (ncolors <= 2)
      indbits = 1;
    else if (ncolors <= 4)
      indbits = 2;
    else if (ncolors <= 16)
      indbits = 4;
    else
      indbits = 8;

    indwidth = (img->width() * indbits + 7) / 8;
    indices  = (hdChar *)calloc(indwidth, img->height());

    if (img->depth() == 1)
    {
     /*
      * Convert a grayscale image...
      */

      switch (indbits)
      {
        case 1 :
	    for (i = img->height(), pixel = img->pixels(), indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 7; j > 0; j --, k = (k + 7) & 7, pixel ++)
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 0; j > 0; j --, k = (k + 1) & 3, pixel ++)
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 0; j > 0; j --, k ^= 1, pixel ++)
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 7;
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 0;
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(), k = 0; j > 0; j --, k ^= 1, pixel += 3)
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
	    for (i = img->height(), pixel = img->pixels(), indptr = indices,
	             match = colors;
		 i > 0;
		 i --)
	    {
	      for (j = img->width(); j > 0; j --, pixel += 3, indptr ++)
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
	  out->printf("q %.1f 0 0 %.1f %.1f %.1f cm\n", r->width,
	              r->height, r->x, r->y);

        if (img->obj())
	{
	  if (img->mask() && PDFVersion < 13)
	    write_imagemask(out, r);

	  out->printf("/I%d Do Q\n", img->obj());
	  break;
	}

        if (img->mask() && write_obj && PDFVersion >= 13)
	{
	  // We have a mask image, write it!
          pdf_start_object(out);
	  out->puts("/Type/XObject/Subtype/Image");
          out->puts("/ColorSpace/DeviceGray");
	  if (img->maskwidth() == img->width())
	    out->printf("/Width %d/Height %d/BitsPerComponent 8",
	                img->width(), img->height());
          else
	    out->printf("/Width %d/Height %d/BitsPerComponent 1/ImageMask true",
	                img->width(), img->height());

	  filter = pdf_start_stream(out, filters);

	  filter->write(img->mask(), img->maskwidth() * img->height());

          pdf_end_stream(out, filters);
	}

        if (write_obj)
	{
	  // Write an image object...
	  img->obj(pdf_start_object(out));

	  out->puts("/Type/XObject/Subtype/Image");
	  if (img->mask() && PDFVersion >= 13)
	  {
	    if (img->maskwidth() == img->width())
	      out->printf("/SMask %d 0 R", img->obj() - 1);
	    else
	      out->printf("/Mask %d 0 R", img->obj() - 1);
	  }

	  if (ncolors > 0)
	  {
	    for (i = 0; i < ncolors; i ++)
	    {
	      cmap[i][0] = colors[i] >> 16;
	      cmap[i][1] = colors[i] >> 8;
	      cmap[i][2] = colors[i];
	    }

	    out->printf("/ColorSpace[/Indexed/DeviceRGB %d<", ncolors - 1);

	    if (Encryption)
	    {
	      // Encrypt the colormap...
	      filter = encrypt_init(out);
	      filters.add(filter);
	    }
	    else
	      filter = out;

	    filter = new hdASCIIHexFilter(filter);
	    filters.add(filter);
	    filter->write(cmap, 3 * ncolors);

	    while ((filter = (hdFile *)filters.last()) != NULL)
	    {
	      filters.remove(filter);
	      delete filter;
	    }

	    out->puts(">]");
          }
	  else if (img->depth() == 1)
            out->puts("/ColorSpace/DeviceGray");
          else
            out->puts("/ColorSpace/DeviceRGB");

#ifdef HTMLDOC_INTERPOLATION
          if (ncolors != 2)
            out->puts("/Interpolate true");
#endif // HTMLDOC_INTERPOLATION

  	  out->printf("/Width %d/Height %d/BitsPerComponent %d",
	              img->width(), img->height(), indbits);

	  if (OutputJPEG && ncolors == 0)
	    filter = pdf_start_stream(out, filters, img->width(), img->height(),
	                              img->depth());
          else
	    filter = pdf_start_stream(out, filters);

	  if (ncolors > 0)
	    filter->write(indices, indwidth * img->height());
	  else
	    filter->write(img->pixels(),
			  img->width() * img->height() * img->depth());

          pdf_end_stream(out, filters);
	}
	else
	{
	  // Put the image in-line...
          out->puts("BI");

	  if (ncolors > 0)
	  {
	    out->printf("/CS[/I/RGB %d<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	      out->printf("%02X%02X%02X", colors[i] >> 16,
	        	   (colors[i] >> 8) & 255, colors[i] & 255);
	    out->puts(">]");
          }
	  else if (img->depth() == 1)
            out->puts("/CS/G");
          else
            out->puts("/CS/RGB");

          if (ncolors != 2)
            out->puts("/I true");

  	  out->printf("/W %d/H %d/BPC %d", img->width(), img->height(),
	               indbits); 

	  if (ncolors > 0)
	  {
  	    out->puts(" ID\n");
  	    out->write(indices, indwidth * img->height());
	  }
	  else if (OutputJPEG)
	  {
  	    out->puts("/F/DCT ID\n");

	    filter = new hdJPEGFilter(out, img->width(), img->height(),
	                              img->depth(), OutputJPEG);

	    filter->write(img->pixels(),
			  img->width() * img->height() * img->depth());

            delete filter;
          }
	  else
	  {
  	    out->puts(" ID\n");
  	    out->write(img->pixels(),
	               img->width() * img->height() * img->depth());
          }

	  out->puts("\nEI\nQ\n");
	}
        break;

    case 1 : /* PostScript, Level 1 */
        out->puts("GS");
	out->printf("[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
		    r->x, r->y);

	if (img->mask())
	  write_imagemask(out, r);

	out->printf("/picture %d string def\n", img->width() * img->depth());

	if (img->depth() == 1)
	  out->printf("%d %d 8 [%d 0 0 %d 0 %d] {currentfile picture "
	              "readhexstring pop} image\n",
		      img->width(), img->height(),
		      img->width(), -img->height(),
		      img->height()); 
	else
	  out->printf("%d %d 8 [%d 0 0 %d 0 %d] {currentfile picture "
	              "readhexstring pop} false 3 colorimage\n",
		      img->width(), img->height(),
		      img->width(), -img->height(),
		      img->height()); 

	filter = new hdASCIIHexFilter(out);
	filter->write(img->pixels(),
	              img->width() * img->height() * img->depth());
        delete filter;

	out->puts("GR\n");
        break;

    case 3 : /* PostScript, Level 3 */
        // Fallthrough to Level 2 output if compression is disabled and
	// we aren't doing transparency...
        if ((Compression && (!OutputJPEG || ncolors > 0)) ||
	    (img->mask() && img->maskwidth() == img->width()))
	{
          out->puts("GS");
	  out->printf("[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	              r->x, r->y);

	  if (img->mask() && img->maskwidth() != img->width())
	    write_imagemask(out, r);

          if (ncolors > 0)
          {
	    if (ncolors <= 2)
	      ncolors = 2; /* Adobe doesn't like 1 color images... */

	    out->printf("[/Indexed/DeviceRGB %d\n<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	    {
	      out->printf("%02X%02X%02X", colors[i] >> 16,
	              (colors[i] >> 8) & 255, colors[i] & 255);
	      if ((i % 13) == 12)
	        out->put('\n');
            }
	    out->puts(">]setcolorspace\n");

	    if (img->mask() && img->maskwidth() == img->width())
	      out->printf("<<"
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
			  img->width(), img->height(),
			  img->width(), -img->height(), img->height());

	    out->printf("<<"
	                "/ImageType 1"
	                "/Width %d"
	                "/Height %d"
	                "/BitsPerComponent %d"
	                "/ImageMatrix[%d 0 0 %d 0 %d]"
	                "/Decode[0 %d]",
			img->width(), img->height(), indbits,
			img->width(), -img->height(), img->height(),
			(1 << indbits) - 1);

#ifdef HTMLDOC_INTERPOLATION
            if (ncolors != 2)
	      out->puts("/Interpolate true");
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
            out->puts("/DataSource currentfile/ASCII85Decode filter");
#else
            out->puts("/DataSource currentfile/ASCIIHexDecode filter");
#endif // HTMLDOC_ASCII85

#ifdef HTMLDOC_ASCII85
            filter = new hdASCII85Filter(out);
	    filters.add(filter);
#else
            filter = new hdASCIIHexFilter(out);
	    filters.add(filter);
#endif // HTMLDOC_ASCII85

            if (Compression)
	    {
	      out->puts("/FlateDecode filter");
	      filter = new hdFlateFilter(filter, Compression);
	      filters.add(filter);
	    }

	    out->puts(">>\n");

	    if (img->mask() && img->maskwidth() == img->width())
	      out->puts(">>\n");

	    out->puts("image\n");

	    if (img->mask() && img->maskwidth() == img->width())
	    {
	      data = (hdChar *)malloc(img->width() * 2);

	      for (i = 0, maskptr = img->mask(), indptr = indices;
	           i < img->height();
		   i ++)
	      {
	        for (j = img->width(), dataptr = data; j > 0; j --)
		{
		  *dataptr++ = *maskptr++;
		  *dataptr++ = *indptr++;
		}

		filter->write(data, img->width() * 2);
	      }

	      free(data);
	    }
	    else
	      filter->write(indices, indwidth * img->height());

	    while ((filter = (hdFile *)filters.last()) != NULL)
	    {
	      filters.remove(filter);
	      delete filter;
	    }
          }
          else
          {
	    if (img->depth() == 1)
	      out->puts("/DeviceGray setcolorspace");
	    else
	      out->puts("/DeviceRGB setcolorspace");

	    if (img->mask() && img->maskwidth() == img->width())
	      out->printf("<<"
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
			  img->width(), img->height(),
			  img->width(), -img->height(), img->height());

	    out->printf("<<"
	                "/ImageType 1"
	                "/Width %d"
	                "/Height %d"
	                "/BitsPerComponent 8"
	                "/ImageMatrix[%d 0 0 %d 0 %d]"
	                "/Decode[%s]",
			img->width(), img->height(),
			img->width(), -img->height(), img->height(),
			img->depth() == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_INTERPOLATION
	    out->puts("/Interpolate true");
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
            out->puts("/DataSource currentfile/ASCII85Decode filter");
#else
            out->puts("/DataSource currentfile/ASCIIHexDecode filter");
#endif // HTMLDOC_ASCII85

#ifdef HTMLDOC_ASCII85
            filter = new hdASCII85Filter(out);
	    filters.add(filter);
#else
            filter = new hdASCIIHexFilter(out);
	    filters.add(filter);
#endif // HTMLDOC_ASCII85

            if (Compression)
	    {
	      out->puts("/FlateDecode filter");
	      filter = new hdFlateFilter(filter, Compression);
	      filters.add(filter);
	    }

	    out->puts(">>\n");

	    if (img->mask() && img->maskwidth() == img->width())
	      out->puts(">>\n");

	    out->puts("image\n");

	    if (img->mask() && img->maskwidth() == img->width())
	    {
	      data = (hdChar *)malloc(img->width() * (img->depth() + 1));

	      for (i = 0, maskptr = img->mask(), pixel = img->pixels();
	           i < img->height();
		   i ++)
	      {
	        if (img->depth() == 1)
		{
	          for (j = img->width(), dataptr = data; j > 0; j --)
		  {
		    *dataptr++ = *maskptr++;
		    *dataptr++ = *pixel++;
		  }
		}
		else
		{
	          for (j = img->width(), dataptr = data; j > 0; j --)
		  {
		    *dataptr++ = *maskptr++;
		    *dataptr++ = *pixel++;
		    *dataptr++ = *pixel++;
		    *dataptr++ = *pixel++;
		  }
		}

		filter->write(data, img->width() * (img->depth() + 1));
	      }

	      free(data);
	    }
	    else
	      filter->write(img->pixels(),
	                    img->width() * img->height() * img->depth());

	    while ((filter = (hdFile *)filters.last()) != NULL)
	    {
	      filters.remove(filter);
	      delete filter;
	    }
          }

	  out->puts("GR\n");
          break;
	}

    case 2 : /* PostScript, Level 2 */
        out->puts("GS");
	out->printf("[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	            r->x, r->y);

	if (img->mask())
	  write_imagemask(out, r);

        if (ncolors > 0)
        {
	  out->printf("[/Indexed/DeviceRGB %d\n<", ncolors - 1);
	  for (i = 0; i < ncolors; i ++)
	  {
	    out->printf("%02X%02X%02X", colors[i] >> 16,
	            (colors[i] >> 8) & 255, colors[i] & 255);
	    if ((i % 13) == 12)
	      out->put('\n');
          }

	  out->puts(">]setcolorspace\n");

	  out->printf("<<"
	              "/ImageType 1"
	              "/Width %d"
	              "/Height %d"
	              "/BitsPerComponent %d"
	              "/ImageMatrix[%d 0 0 %d 0 %d]"
	              "/Decode[0 %d]",
		      img->width(), img->height(), indbits,
		      img->width(), -img->height(), img->height(),
		      (1 << indbits) - 1);

#ifdef HTMLDOC_INTERPOLATION
          if (ncolors != 2)
	    out->puts("/Interpolate true");
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
	  out->puts("/DataSource currentfile/ASCII85Decode filter>>image\n");

          filter = new hdASCII85Filter(out);
	  filter->write(indices, indwidth * img->height());
	  delete filter;
#else
	  out->puts("/DataSource currentfile/ASCIIHexDecode filter>>image\n");

          filter = new hdASCIIHexFilter(out);
	  filter->write(indices, indwidth * img->height());
	  delete filter;
#endif /* HTMLDOC_ASCII85 */
        }
	else if (OutputJPEG)
	{
	  if (img->depth() == 1)
	    out->puts("/DeviceGray setcolorspace\n");
	  else
	    out->puts("/DeviceRGB setcolorspace\n");

	  out->printf("<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent 8"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[%s]",
	          img->width(), img->height(),
        	  img->width(), -img->height(), img->height(),
        	  img->depth() == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_INTERPOLATION
	  out->puts("/Interpolate true");
#endif // HTMLDOC_INTERPOLATION

#ifdef HTMLDOC_ASCII85
	  out->puts("/DataSource currentfile/ASCII85Decode filter"
	            "/DCTDecode filter>>image\n");
#else
	  out->puts("/DataSource currentfile/ASCIIHexDecode filter"
	            "/DCTDecode filter>>image\n");
#endif // HTMLDOC_ASCII85

#ifdef HTMLDOC_ASCII85
	  filter = new hdASCII85Filter(out);
	  filters.add(filter);
#else
	  filter = new hdASCIIHexFilter(out);
	  filters.add(filter);
#endif // HTMLDOC_ASCII85

	  filter = new hdJPEGFilter(filter, img->width(), img->height(),
				    img->depth(), OutputJPEG);
	  filters.add(filter);

	  filter->write(img->pixels(),
			img->width() * img->height() * img->depth());

	  while ((filter = (hdFile *)filters.last()) != NULL)
	  {
	    filters.remove(filter);
	    delete filter;
	  }
        }
        else
        {
	  if (img->depth() == 1)
	    out->puts("/DeviceGray setcolorspace\n");
	  else
	    out->puts("/DeviceRGB setcolorspace\n");

	  out->printf("<<"
	              "/ImageType 1"
	              "/Width %d"
	              "/Height %d"
	              "/BitsPerComponent 8"
	              "/ImageMatrix[%d 0 0 %d 0 %d]"
	              "/Decode[%s]",
		      img->width(), img->height(),
		      img->width(), -img->height(), img->height(),
		      img->depth() == 1 ? "0 1" : "0 1 0 1 0 1");

#ifdef HTMLDOC_ASCII85
          out->puts("/DataSource currentfile/ASCII85Decode filter"
		    ">>image\n");

          filter = new hdASCII85Filter(out);
#else
          out->puts("/DataSource currentfile/ASCIIHexDecode filter"
		    ">>image\n");

          filter = new hdASCIIHexFilter(out);
#endif // HTMLDOC_ASCII85

	  filter->write(img->pixels(),
	                img->width() * img->height() * img->depth());

          delete filter;
        }

	out->puts("GR\n");
        break;
  }

  if (ncolors > 0)
    free(indices);

  img->free();
}


/*
 * 'write_imagemask()' - Write an imagemask to the output file...
 */

static void
write_imagemask(hdFile   *out,		/* I - Output file */
                hdRender *r)		/* I - Image to write */
{
  hdImage	*img;			/* Current image */
  int		x, y;			/* Position in mask image */
  int		startx, count;		/* Start and count */
  hdChar	*ptr,			/* Pointer into mask image */
		byte,			/* Current byte */
		bit;			/* Current bit */
  float		scalex, scaley;		/* 1/(w-1) and 1/(h-1) scaling factors */
  int		width, height;		/* Scaled width and height */


  img    = r->data.image;
  width  = img->width();
  height = img->height();
  scalex = 1.0f / width;
  scaley = 1.0f / height;

  switch (PSLevel)
  {
    case 0 : // PDF
        break;

    default : // PostScript
        out->puts("\nnewpath\n");
        break;
  }

  for (y = 0; y < height; y ++)
  {
    for (x = 0, ptr = img->mask() + (height - y - 1) * img->maskwidth(),
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
	      out->printf("%.6f %.6f %.6f %.6f re\n",
			  (float)startx * scalex,
			  (float)y * scaley,
			  (float)count * scalex,
			  1.0f * scaley);
              break;

	  default : // PostScript
	      out->printf("%.6f %.6f %.6f %.6f re\n",
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
	    out->printf("%.6f %.6f %.6f %.6f re\n",
			(float)startx * scalex,
			(float)y * scaley,
			(float)count * scalex,
			1.0f * scaley);
            break;

	default : // PostScript
	    out->printf("%.6f %.6f %.6f %.6f re\n",
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
        out->puts("W n\n");
        break;

    default : // PostScript
        out->puts("clip\n");
        break;
  }
}


/*
 * 'write_prolog()' - Write the file prolog...
 */

static void
write_prolog(hdFile *out,		/* I - Output file */
             int    page_count,		/* I - Number of pages (0 if not known) */
             hdChar *author,		/* I - Author of document */
             hdChar *creator,		/* I - Application that generated the HTML file */
             hdChar *copyright,		/* I - Copyright (if any) on the document */
             hdChar *keywords,		/* I - Search keywords */
	     hdChar *subject)		/* I - Subject */
{
  hdFile	*prolog;		/* PostScript prolog file */
  int		i, j,			/* Looping vars */
		encoding_object;	/* Font encoding object */
  int		page;			/* Current page */
  hdRender	*r;			/* Current render data */
  int		fonts_used[HD_FONT_FACE_MAX][HD_FONT_INTERNAL_MAX];
					/* Whether or not a font is used */
  int		font_desc[HD_FONT_FACE_MAX][HD_FONT_INTERNAL_MAX];
					/* Font descriptor objects */
  char		temp[1024];		/* Temporary string */
  hdMD5		md5;			/* MD5 state */
  hdRC4		rc4;			// RC4 context
  hdByte	digest[16];		/* MD5 digest value */
  hdByte	owner_pad[32],		/* Padded owner password */
		owner_key[32],		/* Owner key */
		user_pad[32],		/* Padded user password */
		user_key[32];		/* User key */
  hdByte	perm_bytes[4];		/* Permission bytes */
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

  for (page = 0; page < num_pages; page ++)
    for (r = pages[page].start; r != NULL; r = r->next)
      if (r->type == HD_RENDER_TEXT)
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
      out->puts("%XRXbegin: 001.0300\n");
      out->puts("%XRXPDLformat: PS-Adobe\n");
      if (doc_title)
	out->printf("%%XRXtitle: %s\n", doc_title);

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
	  out->puts("%XRXrequirements: duplex(tumble)\n");
	else
	  out->puts("%XRXrequirements: duplex\n");
      }
      else
	out->puts("%XRXrequirements: simplex\n");

      out->puts("%XRXdisposition: PRINT\n");
      out->puts("%XRXsignature: False\n");
      out->printf("%%XRXpaperType-size: %.0f %.0f\n",
		  pages[outpages[start].pages[0]].width * 25.4f / 72.0f,
		  pages[outpages[start].pages[0]].length * 25.4f / 72.0f);
      if (pages[outpages[start].pages[0]].media_type[0])
	out->printf("%%XRXpaperType-preFinish: %s 0 0\n",
		    pages[start].media_type);
      if (pages[outpages[start].pages[0]].media_color[0])
	out->printf("%%XRXdocumentPaperColors: %c%s\n",
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

	    out->printf("%%XRXpageExceptions: %d %d %.0f %.0f %c%s opaque %s 0 0\n",
			i + 1, i + count,
			pages[outpages[i].pages[0]].width * 25.4f / 72.0f,
			pages[outpages[i].pages[0]].length * 25.4f / 72.0f,
			tolower(pages[outpages[i].pages[0]].media_color[0]),
			pages[outpages[i].pages[0]].media_color + 1,
			pages[outpages[i].pages[0]].media_type[0] ?
			    pages[outpages[i].pages[0]].media_type : "Plain");

	    if (pages[outpages[i].pages[0]].duplex &&
	        pages[outpages[i].pages[0]].landscape)
	      out->printf("%%XRXpageExceptions-plex: %d %d duplex(tumble)\n",
			  i + 1, i + count);
	    else if (pages[outpages[i].pages[0]].duplex)
	      out->printf("%%XRXpageExceptions-plex: %d %d duplex\n",
			  i + 1, i + count);
            else
	      out->printf("%%XRXpageExceptions-plex: %d %d simplex\n",
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

	      out->printf("%%XRXpageExceptions: %d %d %.0f %.0f %c%s opaque %s 0 0\n",
			  i + 1, i + count,
			  pages[outpages[i].pages[0]].width * 25.4f / 72.0f,
			  pages[outpages[i].pages[0]].length * 25.4f / 72.0f,
			  tolower(pages[outpages[i].pages[0]].media_color[0]),
			  pages[outpages[i].pages[0]].media_color + 1,
			  pages[outpages[i].pages[0]].media_type[0] ?
			      pages[outpages[i].pages[0]].media_type : "Plain");

	      if (pages[outpages[i].pages[0]].duplex && pages[outpages[i].pages[0]].landscape)
		out->printf("%%XRXpageExceptions-plex: %d %d duplex(tumble)\n",
			    i + 1, i + count);
	      else if (pages[outpages[i].pages[0]].duplex)
		out->printf("%%XRXpageExceptions-plex: %d %d duplex\n",
			    i + 1, i + count);
              else
		out->printf("%%XRXpageExceptions-plex: %d %d simplex\n",
			    i + 1, i + count);
	    }
	    else
	      count = 1;
          }
	}
      }

      out->puts("%XRXend\n");
    }

    out->puts("%!PS-Adobe-3.0\n");
    if (Landscape)
      out->printf("%%%%BoundingBox: 0 0 %d %d\n", PageLength, PageWidth);
    else
      out->printf("%%%%BoundingBox: 0 0 %d %d\n", PageWidth, PageLength);
    out->printf("%%%%LanguageLevel: %d\n", PSLevel);
    out->puts("%%Creator: htmldoc " SVERSION " " COPYRIGHT "\n");
    out->printf("%%%%CreationDate: D:%04d%02d%02d%02d%02d%02d%+03d%02d\n",
		doc_date->tm_year + 1900, doc_date->tm_mon + 1, doc_date->tm_mday,
		doc_date->tm_hour, doc_date->tm_min, doc_date->tm_sec,
		(int)(-timezone / 3600),
		(int)(((timezone < 0 ? -timezone : timezone) / 60) % 60));
    if (doc_title != NULL)
      out->printf("%%%%Title: %s\n", doc_title);
    if (author != NULL)
      out->printf("%%%%Author: %s\n", author);
    if (creator != NULL)
      out->printf("%%%%Generator: %s\n", creator);
    if (copyright != NULL)
      out->printf("%%%%Copyright: %s\n", copyright);
    if (keywords != NULL)
      out->printf("%%%%Keywords: %s\n", keywords);
    if (subject != NULL)
      out->printf("%%%%Subject: %s\n", keywords);
    if (page_count > 0)
      out->printf("%%%%Pages: %d\n", page_count);
    else
      out->puts("%%Pages: (atend)\n");

    if (!EmbedFonts)
    {
      out->puts("%%DocumentNeededResources:\n");

      for (i = 0; i < HD_FONT_FACE_MAX; i ++)
        for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
          if (fonts_used[i][j] && is_base_font(i) &&
	      _htmlStyleSheet->fonts[i][j])
            out->printf("%%%%+ font %s\n",
	                _htmlStyleSheet->fonts[i][j]->ps_name);
    }

    out->puts("%%DocumentProvidedResources:\n");

    for (i = 0; i < HD_FONT_FACE_MAX; i ++)
      for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
        if (fonts_used[i][j] && (EmbedFonts || !is_base_font(i)) &&
	    _htmlStyleSheet->fonts[i][j])
          out->printf("%%%%+ font %s\n",
	          _htmlStyleSheet->fonts[i][j]->ps_name);
    out->puts("%%DocumentData: Clean7bit\n");
    out->puts("%%EndComments\n");

    out->puts("%%BeginProlog\n");

   /*
    * Embed fonts?
    */

    for (i = 0; i < HD_FONT_FACE_MAX; i ++)
    {
      if (EmbedFonts || !is_base_font(i))
	for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
          if (fonts_used[i][j] && _htmlStyleSheet->fonts[i][j])
	    write_type1(out, (hdFontFace)i, (hdFontStyle)j);
    }

   /*
    * Procedures used throughout the document...
    */

    out->puts("%%BeginResource: procset htmldoc-page 1.9 0\n");
    out->puts("/BD{bind def}bind def");
    out->puts("/B{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto\n"
	      "closepath stroke}BD");
    out->puts("/C{setrgbcolor}BD\n");
    out->puts("/CM{concat}BD");
    out->puts("/DF{findfont dup length dict begin{1 index/FID ne{def}{pop pop}\n"
	      "ifelse}forall/Encoding fontencoding def currentdict end definefont pop}BD\n");
    out->puts("/F{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath fill}BD\n");
    out->puts("/FS{/hdFontSize exch def}BD");
    out->puts("/G{setgray}BD\n");
    out->puts("/GS{gsave}BD");
    out->puts("/GR{grestore}BD");
    out->puts("/J{0 exch ashow}BD\n");
    out->puts("/L{0 rlineto stroke}BD");
    out->puts("/M{moveto}BD");
    out->puts("/re{4 2 roll moveto 1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath}BD\n");
    out->puts("/RO{rotate}BD");
    out->puts("/S{show}BD");
    out->puts("/SC{dup scale}BD\n");
    out->puts("/SF{findfont hdFontSize scalefont setfont}BD");
    out->puts("/SP{showpage}BD");
    out->puts("/T{translate}BD\n");
    out->puts("%%EndResource\n");

   /*
    * Output the font encoding for the current character set...  For now we
    * just support 8-bit fonts since true Unicode support needs a very large
    * number of extra fonts that aren't normally available on a PS printer.
    */

    out->puts("/fontencoding[\n");
    for (i = 0, j = 0; i < 256; i ++)
    {
      if (_htmlStyleSheet->glyphs[i])
        j += strlen(_htmlStyleSheet->glyphs[i]) + 1;
      else
        j += 8;

      if (j > 80)
      {
	if (_htmlStyleSheet->glyphs[i])
          j = strlen(_htmlStyleSheet->glyphs[i]) + 1;
	else
          j = 8;

        out->put('\n');
      }
	
      out->put('/');
      if (_htmlStyleSheet->glyphs[i])
        out->puts(_htmlStyleSheet->glyphs[i]);
      else
        out->puts(".notdef");
    }

    out->puts("]def\n");

   /*
    * Fonts...
    */

    for (i = 0; i < HD_FONT_FACE_MAX; i ++)
      for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
        if (fonts_used[i][j] && _htmlStyleSheet->fonts[i][j])
        {
	  if (i < 3)
	    out->printf("/F%x/%s DF\n", i * 4 + j,
	            _htmlStyleSheet->fonts[i][j]->ps_name);
	  else
	    out->printf("/F%x/%s findfont definefont pop\n", i * 4 + j,
	            _htmlStyleSheet->fonts[i][j]->ps_name);
        }

    if (PSCommands)
    {
      snprintf(temp, sizeof(temp), "%s/data/prolog.ps", _htmlData);
      if ((prolog = hdFile::open(temp, HD_FILE_READ)) != NULL)
      {
	while (prolog->gets(temp, sizeof(temp)))
          out->printf("%s\n", temp);

	delete prolog;
      }
      else
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
                       "Unable to open data file \"%s\" - %s", temp,
                       strerror(errno));

	out->puts("%%BeginResource: procset htmldoc-device 1.9 0\n");
	out->puts("languagelevel 1 eq{/setpagedevice{pop}BD}if\n");
	out->puts("/SetDuplexMode{<</Duplex 3 index/Tumble 5 index>>setpagedevice "
		  "pop pop}BD\n");
	out->puts("/SetMediaColor{pop}BD\n");
	out->puts("/SetMediaType{pop}BD\n");
	out->puts("/SetMediaPosition{pop}BD\n");
	out->puts("/SetPageSize{2 array astore<</PageSize 2 index/ImageableArea "
		  "null>>setpagedevice pop}BD\n");
	out->puts("%%EndResource\n");
      }
    }

    if (BodyImage != NULL)
      ps_write_background(out);

    out->puts("%%EndProlog\n");
  }
  else
  {
   /*
    * Write PDF prolog stuff...
    */

    out->printf("%%PDF-%.1f\n", 0.1 * PDFVersion);
    out->puts("%\342\343\317\323\n");
    num_objects = 0;

   /*
    * Compute the file ID...
    */

    md5.init();
    md5.append((hdByte *)OutputPath, sizeof(OutputPath));
    md5.append((hdByte *)&doc_time, sizeof(doc_time));
    md5.finish(file_id);

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

      md5.init();
      md5.append(owner_pad, 32);
      md5.finish(digest);

      if (encrypt_len > 5)
      {
        // MD5 the result 50 more times...
	for (i = 0; i < 50; i ++)
	{
          md5.init();
          md5.append(digest, 16);
          md5.finish(digest);
	}

        // Copy the padded user password...
        memcpy(owner_key, user_pad, 32);

        // Encrypt the result 20 times...
	for (i = 0; i < 20; i ++)
	{
	  // XOR each byte in the key with the loop counter...
	  for (j = 0; j < encrypt_len; j ++)
	    encrypt_key[j] = digest[j] ^ i;

          rc4.init(encrypt_key, encrypt_len);
          rc4.encrypt(owner_key, owner_key, 32);
	}
      }
      else
      {
        rc4.init(digest, encrypt_len);
        rc4.encrypt(user_pad, owner_key, 32);
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
	if (!(perm_value & HD_PDF_PERM_COPY))
	  perm_value &= ~0x00240000;	// Mask additional copy perms...
      }

     /*
      * Compute the encryption key...
      */

      md5.init();
      md5.append(user_pad, 32);
      md5.append(owner_key, 32);

      perm_bytes[0] = perm_value;
      perm_bytes[1] = perm_value >> 8;
      perm_bytes[2] = perm_value >> 16;
      perm_bytes[3] = perm_value >> 24;

      md5.append(perm_bytes, 4);
      md5.append(file_id, 16);
      md5.finish(digest);

      if (encrypt_len > 5)
      {
        // MD5 the result 50 times..
        for (i = 0; i < 50; i ++)
	{
	  md5.init();
	  md5.append(digest, 16);
	  md5.finish(digest);
	}
      }

      memcpy(encrypt_key, digest, encrypt_len);

     /*
      * Compute the user key...
      */

      if (encrypt_len > 5)
      {
        md5.init();
        md5.append(pad, 32);
        md5.append(file_id, 16);
        md5.finish(user_key);

        memset(user_key + 16, 0, 16);

        // Encrypt the result 20 times...
        for (i = 0; i < 20; i ++)
	{
	  // XOR each byte in the key with the loop counter...
	  for (j = 0; j < encrypt_len; j ++)
	    digest[j] = encrypt_key[j] ^ i;

          rc4.init(digest, encrypt_len);
          rc4.encrypt(user_key, user_key, 16);
	}
      }
      else
      {
        rc4.init(encrypt_key, encrypt_len);
        rc4.encrypt(pad, user_key, 32);
      }

     /*
      * Write the encryption dictionary...
      */

      encrypt_object = pdf_start_object(out);

      out->puts("/Filter/Standard/O<");
      for (i = 0; i < 32; i ++)
        out->printf("%02x", owner_key[i]);
      out->puts(">/U<");
      for (i = 0; i < 32; i ++)
        out->printf("%02x", user_key[i]);
      out->puts(">");

      if (encrypt_len > 5)
      {
        // N-bit encryption...
        out->printf("/P %d/V 2/R 3/Length %d", (int)perm_value, encrypt_len * 8);
      }
      else
        out->printf("/P %d/V 1/R 2", (int)perm_value);

      pdf_end_object(out);
    }
    else
      encrypt_object = 0;

   /*
    * Write info object...
    */

    info_object = pdf_start_object(out);

    out->puts("/Producer");
    write_string(out, (hdChar *)"htmldoc " SVERSION " " COPYRIGHT);
    out->puts("/CreationDate");
    sprintf(temp, "D:%04d%02d%02d%02d%02d%02d%+03d%02d",
            doc_date->tm_year + 1900, doc_date->tm_mon + 1, doc_date->tm_mday,
            doc_date->tm_hour, doc_date->tm_min, doc_date->tm_sec,
	    (int)(-timezone / 3600),
	    (int)(((timezone < 0 ? -timezone : timezone) / 60) % 60));
    write_string(out, (hdChar *)temp);

    if (doc_title != NULL)
    {
      out->puts("/Title");
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

      out->puts("/Author");
      write_utf16(out, (hdChar *)temp);
    }

    if (creator != NULL)
    {
      out->puts("/Creator");
      write_utf16(out, creator);
    }

    if (keywords != NULL)
    {
      out->puts("/Keywords");
      write_utf16(out, keywords);
    }

    if (subject != NULL)
    {
      out->puts("/Subject");
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

    out->puts("/Type/Encoding");
    out->puts("/Differences[");
    for (i = 0, j = -1; i < 256; i ++)
      if (_htmlStyleSheet->glyphs[i])
      {
       /*
        * Output a character index if we had blank ones...
	*/

        if (j != (i - 1))
	  out->printf(" %d", i);

        out->printf("/%s", _htmlStyleSheet->glyphs[i]);
	j = i;
      }

    out->puts("]");
    pdf_end_object(out);

    memset(font_desc, 0, sizeof(font_desc));

   /*
    * Build font descriptors for the EmbedFonts fonts...
    */

    for (i = 0; i < HD_FONT_FACE_MAX; i ++)
    {
      if (EmbedFonts || !is_base_font(i))
	for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
          if (fonts_used[i][j] && _htmlStyleSheet->fonts[i][j])
	    font_desc[i][j] = write_type1(out, (hdFontFace )i, (hdFontStyle)j);
    }

    for (i = 0; i < HD_FONT_FACE_MAX; i ++)
      for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
        if (fonts_used[i][j] && _htmlStyleSheet->fonts[i][j])
        {
	  font_objects[i * HD_FONT_INTERNAL_MAX + j] = pdf_start_object(out);

	  out->puts("/Type/Font");
	  out->puts("/Subtype/Type1");
	  out->printf("/BaseFont/%s", _htmlStyleSheet->fonts[i][j]->ps_name);

          if (font_desc[i][j])
	  {
	    // Embed Type1 font...
	    out->puts("/FirstChar 0");
	    out->puts("/LastChar 255");
	    out->printf("/Widths %d 0 R", font_desc[i][j] + 1);
	    out->printf("/FontDescriptor %d 0 R", font_desc[i][j]);
	  }

	  if (i < HD_FONT_FACE_SYMBOL) /* Use native encoding for symbols */
	    out->printf("/Encoding %d 0 R", encoding_object);

          pdf_end_object(out);
        }
  }
}


/*
 * 'write_string()' - Write a text entity.
 */

static void
write_string(hdFile *out,		/* I - Output file */
             hdChar *s)			/* I - String */
{
  if (Encryption)
  {
   /*
    * Write an encrypted string...
    */

    out->put('<');

    hdASCIIHexFilter *hex = new hdASCIIHexFilter(out);
    hdRC4Filter *rc4 = encrypt_init(hex);

    rc4->write(s, strlen((char *)s));
    delete rc4;
    delete hex;

    out->put('>');
  }
  else
  {
    out->put('(');

    while (*s)
    {
      if (*s == 160) /* &nbsp; */
	out->put(' ');
      else if (*s < 32 || *s > 126)
	out->printf("\\%o", *s);
      else if (*s == '(' || *s == ')' || *s == '\\')
      {
	out->put('\\');
	out->put(*s);
      }
      else
	out->put(*s);

      s ++;
    }

    out->put(')');
  }
}


/*
 * 'write_text()' - Write a text entity.
 */

static void
write_text(hdFile   *out,	/* I - Output file */
           hdRender *r)		/* I - Text entity */
{
  hdChar	*ptr;			/* Pointer into text */


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
      out->printf(" %g", r->data.text.spacing);
  }
  else if (r->data.text.spacing != render_spacing)
    out->printf(" %g Tc", render_spacing = r->data.text.spacing);

  write_string(out, r->data.text.buffer);

  if (PSLevel > 0)
  {
    if (r->data.text.spacing > 0.0f)
      out->puts("J\n");
    else
      out->puts("S\n");
  }
  else
    out->puts("Tj\n");

  render_x += r->width;
}


/*
 * 'write_trailer()' - Write the file trailer.
 */

static void
write_trailer(hdFile *out,		/* I - Output file */
              int    num_file_pages)	/* I - Number of pages in file */
{
  int		i, j, k,		/* Looping vars */
		type,			/* Type of number */
		start;			/* Start page number */
  size_t	offset;			/* Offset to xref table in PDF file */
  hdPage	*page;			/* Start page of chapter */
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

    out->puts("%%Trailer\n");
    if (num_file_pages > 0)
      out->printf("%%%%Pages: %d\n", num_file_pages);

    out->puts("%%EOF\n");
  }
  else
  {
   /*
    * PDF...
    */

    root_object = pdf_start_object(out);

    out->puts("/Type/Catalog");
    out->printf("/Pages %d 0 R", pages_object);

    if (PDFVersion >= 12)
    {
      if (names_object)
        out->printf("/Names %d 0 R", names_object);

      out->printf("/PageLayout/%s", layouts[PDFPageLayout]);
    }

    if (outline_object > 0)
      out->printf("/Outlines %d 0 R", outline_object);

    switch (PDFFirstPage)
    {
      case HD_PDF_PAGE_1 :
          if (TitlePage)
	  {
            out->printf("/OpenAction[%d 0 R/XYZ null null 0]",
			pages_object + 1);
            break;
	  }
          break;
      case HD_PDF_TOC :
          if (TocLevels > 0)
	  {
            out->printf("/OpenAction[%d 0 R/XYZ null null 0]",
			pages_object + 2 * chapter_outstarts[0] + 1);
	    break;
	  }
          break;
      case HD_PDF_CHAPTER_1 :
          out->printf("/OpenAction[%d 0 R/XYZ null null 0]",
		      pages_object + 2 * chapter_outstarts[1] + 1);
          break;
    }

    out->printf("/PageMode/%s", modes[PDFPageMode]);

    if (PDFVersion > 12 && NumberUp == 1)
    {
      // Output the PageLabels tree...
      out->puts("/PageLabels<</Nums[");

      i = 0;

      if (TitlePage)
      {
        out->puts("0<</P");
	write_string(out, (hdChar *)"title");
	out->puts(">>");
	if (PageDuplex)
	{
	  out->puts("1<</P");
	  write_string(out, (hdChar *)"eltit");
	  out->puts(">>");
	}
	i += PageDuplex + 1;
      }

      if (TocLevels > 0 && OutputType == HD_OUTPUT_BOOK)
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

        out->printf("%d<</S/%c>>", i, type);

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
	out->printf("%d<</S/%c/St %d", i, type, start);
	if (prefptr)
	{
	  *prefptr = '\0';
	  out->puts("/P");
	  write_string(out, (hdChar *)prefix);
	}
	out->puts(">>");

        i += chapter_ends[j] - chapter_starts[j] + 1;
      }

      out->puts("]>>");
    }

    pdf_end_object(out);

    offset = out->size();

    out->puts("xref\n");
    out->printf("0 %d \n", num_objects + 1);
    out->puts("0000000000 65535 f \n");
    for (i = 1; i <= num_objects; i ++)
      out->printf("%010d 00000 n \n", objects[i]);

    out->puts("trailer\n");
    out->puts("<<");
    out->printf("/Size %d", num_objects + 1);
    out->printf("/Root %d 0 R", root_object);
    out->printf("/Info %d 0 R", info_object);
    out->puts("/ID[<");
    for (i = 0; i < 16; i ++)
      out->printf("%02x", file_id[i]);
    out->puts("><");
    for (i = 0; i < 16; i ++)
      out->printf("%02x", file_id[i]);
    out->puts(">]");

    if (Encryption)
      out->printf("/Encrypt %d 0 R", encrypt_object);

    out->puts(">>\n");
    out->puts("startxref\n");
    out->printf("%ld\n", (long)offset);
    out->puts("%%EOF\n");
  }
}


/*
 * 'write_type1()' - Write an embedded Type 1 font.
 */

static int				/* O - Object number */
write_type1(hdFile      *out,		/* I - File to write to */
            hdFontFace  typeface,	/* I - Typeface */
	    hdFontStyle style)		/* I - Style */
{
  hdStyleFont	*font;			/* Font */
  hdFile	*fp;			/* PFA file */
  int		ch;			/* Character value */
  char		line[1024],		/* Line from AFM file */
		*lineptr,		/* Pointer into line */
		*dataptr;		/* Pointer for data */
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
		  32,			/* Sans-Serif */
		  4,			/* Symbol */
		  4,			/* Dingbats */
		  32,			/*  */
		  32,			/*  */
		  32,			/*  */
		  32,			/*  */
		  32,			/*  */
		  32,			/*  */
		  32,			/*  */
		  32			/*  */
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

  font = _htmlStyleSheet->fonts[typeface][style];

 /*
  * Try to open the PFA file for the Type1 font...
  */

  if ((fp = hdFile::open(font->font_file, HD_FILE_READ)) == NULL)
  {
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open font file %s!", font->font_file);
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

    out->printf("%%%%BeginResource: font %s\n", font->ps_name);

    line[0] = '\0';

    while (fp->gets(line, sizeof(line)))
      out->puts(line);

    out->puts("%%EndResource\n");

    delete fp;
  }
  else
  {
   /*
    * Embed a Type1 font object in the PDF output...
    */

    hdFile	*filter;		// Output filter
    hdArray	filters;		// Filters for this stream


    length1 = 0;
    length2 = 0;
    length3 = 0;

    while (fp->gets(line, sizeof(line)))
    {
      length1 += strlen(line);
      if (strstr(line, "currentfile eexec") != NULL)
        break;
    }

    while (fp->gets(line, sizeof(line)))
    {
      if (!strcmp(line, "00000000000000000000000000000000"
                        "00000000000000000000000000000000\n"))
        break;

      length2 += (strlen(line) - 1) / 2;
    }

    length3 = strlen(line);
    while (fp->gets(line, sizeof(line)))
      length3 += strlen(line);

    fp->seek(0, SEEK_SET);

    pdf_start_object(out);
    out->printf("/Length1 %d", length1);
    out->printf("/Length2 %d", length2);
    out->printf("/Length3 %d", length3);

    filter = pdf_start_stream(out, filters);

    while (fp->gets(line, sizeof(line)))
    {
      filter->puts(line);

      if (strstr(line, "currentfile eexec") != NULL)
        break;
    }

    while (fp->gets(line, sizeof(line)))
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

      filter->write((hdChar *)line, dataptr - line);
    }

    filter->puts(line);
    while (fp->gets(line, sizeof(line)))
      filter->puts(line);

    pdf_end_stream(out, filters);

    delete fp;

   /*
    * Write the font descriptor...
    */

    pdf_start_object(out);
    out->puts("/Type/FontDescriptor");
    out->printf("/Ascent %.0f", 1000.0 * font->ascender);
    out->printf("/Descent %.0f", 1000.0 * font->descender);
    out->printf("/CapHeight %.0f", 1000.0 * font->cap_height);
    out->printf("/XHeight %.0f", 1000.0 * font->x_height);
    out->printf("/FontBBox[%.0f %.0f %.0f %.0f]",
            1000.0 * font->bbox[0], 1000.0 * font->bbox[1],
            1000.0 * font->bbox[2], 1000.0 * font->bbox[3]);
    out->printf("/ItalicAngle %.1f", font->italic_angle);
    out->printf("/StemV %.0f", 1000.0 * font->widths[(int)'v']);
    out->printf("/Flags %d", tflags[typeface] | sflags[style]);
    out->printf("/FontName/%s", font->ps_name);
    out->printf("/FontFile %d 0 R", num_objects - 1);
    pdf_end_object(out);

   /*
    * Write the character widths...
    */

    pdf_start_object(out, 1);
    out->printf("%.0f", 1000.0 * font->widths[0]);
    for (ch = 1; ch < font->num_widths; ch ++)
      out->printf(" %.0f", 1000.0 * font->widths[ch]);
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
write_utf16(hdFile *out,		// I - File to write to
            hdChar *s)			// I - String to write
{
  const hdChar *sptr;			// Pointer into string


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

    write_string(out, s);
  }
  else if (Encryption)
  {
   /*
    * Convert the string to Unicode and encrypt...
    */

    int		ch;			// Character value
    hdChar	unicode[2];		// Unicode character

    out->put('<');

    hdASCIIHexFilter *hex = new hdASCIIHexFilter(out);
    hdRC4Filter *rc4 = encrypt_init(hex);

    unicode[0] = 0xfe;			// Start with BOM
    unicode[1] = 0xff;

    rc4->write(unicode, 2);

    for (sptr = s; *sptr;)
    {
      ch = _htmlStyleSheet->unichars[*sptr++];

      unicode[0] = ch >> 8;
      unicode[1] = ch;

      rc4->write(unicode, 2);
    }

    delete rc4;
    delete hex;

    out->put('>');
  }
  else
  {
   /*
    * Convert the string to Unicode...
    */

    out->puts("<feff");		// Start with BOM
    for (sptr = s; *sptr;)
      out->printf("%04x", _htmlStyleSheet->unichars[*sptr++]);
    out->put('>');
  }
}


/*
 * 'encrypt_init()' - Initialize the RC4 encryption context for the current
 *                    object.
 */

static hdRC4Filter *			// O - RC4 filter
encrypt_init(hdFile *out)		// I - Output file
{
  int		i;			/* Looping var */
  hdChar	data[21],		/* Key data */
		*dataptr;		/* Pointer to key data */
  hdMD5		md5;			/* MD5 state */
  hdByte	digest[16];		/* MD5 digest value */


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

  md5.init();
  md5.append(data, encrypt_len + 5);
  md5.finish(digest);

 /*
  * Initialize the RC4 context using the first N+5 bytes of the digest...
  */

  if (encrypt_len > 11)
    return (new hdRC4Filter(out, digest, 16));
  else
    return (new hdRC4Filter(out, digest, encrypt_len + 5));
}


/*
 * 'update_index()' - Update the index to use page numbers instead of
 *                    link numbers.
 */

static int				// O - Last page number
update_index(hdTree *t,			// I - Index tree
             int    last_page)		// I - Last page number
{
  hdRenderLink	*link;			// Link
  hdChar	*href;			// Link target


  // Loop through index nodes...
  while (t)
  {
    if (t->element == HD_ELEMENT_A &&
        (href = htmlGetAttr(t, "HREF")) != NULL &&
	(link = find_link(href + 1)) != NULL)
    {
      if (link->page == last_page)
      {
        // Remove the duplicate page number listing...
	hdTree	*prev,			// Previous entry node, if any
		*next,			// Next entry node, if any
		*first,			// First node for this listing
		*last;			// Last node for this listing


	first = t->prev;
	if (t->next)
	  last = t->next;
	else
	  last = t;

        prev        = first->prev;
	first->prev = NULL;
	next        = last->next;
	last->next  = NULL;
	prev->next  = next;

	if (next)
	  next->prev = prev;
	else
	  t->parent->last_child = prev;

        htmlDeleteTree(first);
	t = prev;
      }
      else
      {
	// Rewrite link text with real page number...
	last_page = link->page;

	free(t->child->data);
	t->child->data  = (hdChar *)strdup(pages[link->page].page_text);
	t->child->width = t->child->style->get_width(t->child->data);
      }
    }
    else if (t->child)
      update_index(t->child, last_page);

    t = t->next;
  }

  return (last_page);
}


//
// End of "$Id$".
//
