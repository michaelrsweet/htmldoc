/*
 * "$Id: ps-pdf.cxx,v 1.89.2.3 2000/12/01 21:46:48 mike Exp $"
 *
 *   PostScript + PDF output routines for HTMLDOC, a HTML document processing
 *   program.
 *
 *   Just in case you didn't notice it, this file is too big; it will be
 *   broken into more manageable pieces once we make all of the output
 *   "drivers" into classes...
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
 *   pspdf_export()          - Export PostScript/PDF file(s)...
 *   pspdf_prepare_page()    - Add headers/footers to page before writing...
 *   pspdf_prepare_heading() - Add headers/footers to page before writing...
 *   ps_write_document()     - Write all render entities to PostScript file(s).
 *   ps_write_page()         - Write all render entities on a page to a
 *                             PostScript file.
 *   ps_write_background()   - Write a background image...
 *   pdf_write_document()    - Write all render entities to a PDF file.
 *   pdf_write_resources()   - Write the resources dictionary for a page.
 *   pdf_write_page()        - Write a page to a PDF file.
 *   pdf_write_contents()    - Write the table of contents as outline records
 *                             to a PDF file.
 *   pdf_count_headings()    - Count the number of headings under this TOC
 *   pdf_write_background()  - Write a background image...
 *   pdf_write_links()       - Write annotation link objects for each page in
 *                             the document.
 *   pdf_write_names()       - Write named destinations for each link.
 *   parse_contents()        - Parse the table of contents and produce a
 *   parse_doc()             - Parse a document tree and produce rendering
 *                             list output.
 *   parse_heading()         - Parse a heading tree and produce rendering list
 *                             output.
 *   parse_paragraph()       - Parse a paragraph tree and produce rendering
 *                             list output.
 *   parse_pre()             - Parse preformatted text and produce rendering
 *                             list output.
 *   parse_table()           - Parse a table and produce rendering output.
 *   parse_list()            - Parse a list entry and produce rendering output.
 *   init_list()             - Initialize the list type and value as necessary.
 *   parse_comment()         - Parse a comment for HTMLDOC comments.
 *   real_prev()             - Return the previous non-link markup in the tree.
 *   real_next()             - Return the next non-link markup in the tree.
 *   find_background()       - Find the background image/color for the given
 *                             document.
 *   write_background()      - Write the background image/color for to the
 *                             current page.
 *   new_render()            - Allocate memory for a new rendering structure.
 *   add_link()              - Add a named link...
 *   find_link()             - Find a named link...
 *   compare_links()         - Compare two named links.
 *   copy_tree()             - Copy a markup tree...
 *   flatten_tree()          - Flatten an HTML tree to only include the text,
 *                             image, link, and break markups.
 *   update_image_size()     - Update the size of an image based upon the
 *   get_width()             - Get the width of a string in points.
 *   get_title()             - Get the title string for a document.
 *   open_file()             - Open an output file for the current chapter.
 *   set_color()             - Set the current text color...
 *   set_font()              - Set the current text font.
 *   set_pos()               - Set the current text position.
 *   ps_hex()                - Print binary data as a series of hexadecimal
 *                             numbers.
 *   ps_ascii85()            - Print binary data as a series of base-85
 *                             numbers.
 *   jpg_init()              - Initialize the JPEG destination.
 *   jpg_empty()             - Empty the JPEG output buffer.
 *   jpg_term()              - Write the last JPEG data to the file.
 *   jpg_setup()             - Setup the JPEG compressor for writing an image.
 *   compare_rgb()           - Compare two RGB colors...
 *   write_image()           - Write an image to the given output file...
 *   write_prolog()          - Write the file prolog...
 *   write_string()          - Write a text entity.
 *   write_text()            - Write a text entity.
 *   write_trailer()         - Write the file trailer.
 *   encrypt_init()          - Initialize the RC4 encryption context for
 *                             the current object.
 *   flate_open_stream()     - Open a deflated output stream.
 *   flate_close_stream()    - Close a deflated output stream.
 *   flate_puts()            - Write a character string to a compressed stream.
 *   flate_printf()          - Write a formatted character string to a
 *                             compressed stream.
 *   flate_write()           - Write data to a compressed stream.
 */

/*
 * Include necessary headers.
 */

/*#define DEBUG*/
#include "htmldoc.h"
#include "md5.h"
#include "rc4.h"
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#ifdef MAC		// MacOS-specific header file...
#  include <Files.h>
#endif // MAC

#if defined(WIN32) || defined(__EMX__)
#  include <io.h>
#else
#  include <unistd.h>
#endif // WIN32 || __EMX__

#include <fcntl.h>

#include <zlib.h>

extern "C" {		/* Workaround for JPEG header problems... */
#include <jpeglib.h>	/* JPEG/JFIF image definitions */
}


/*
 * Constants...
 */

#define RENDER_TEXT	0
#define RENDER_IMAGE	1
#define RENDER_BOX	2
#define RENDER_FBOX	3
#define RENDER_LINK	4


/*
 * Structures...
 */

typedef struct render_str	/**** Render entity structure ****/
{
  struct render_str	*next;	/* Next rendering entity */
  int	type;			/* Type of entity */
  float	x,			/* Position in points */
	y,			/* ... */
	width,			/* Size in points */
	height;			/* ... */
  union
  {
    struct
    {
      int	typeface,	/* Typeface for text */
		style;		/* Style of text */
      float	size;		/* Size of text in points */
      float	rgb[3];		/* Color of text */
      uchar	buffer[1];	/* String buffer */
    }   	text;
    image_t	*image;		/* Image pointer */
    float	box[3];		/* Box color */
    float	fbox[3];	/* Filled box color */
    uchar	link[1];	/* Link URL */
  }	data;
} render_t;

typedef struct			/**** Named link position structure */
{
  short		page,		/* Page # */
		top;		/* Top position */
  uchar		name[124];	/* Reference name */
} link_t;


/*
 * Local globals...
 */

static time_t	doc_time;	/* Current time */
static struct tm *doc_date;	/* Current date */

static int	title_page;
static int	chapter,
		chapter_starts[MAX_CHAPTERS],
		chapter_ends[MAX_CHAPTERS];

static int	num_headings,
		heading_pages[MAX_HEADINGS],
		heading_tops[MAX_HEADINGS];

static int	num_pages;
static render_t	*pages[MAX_PAGES],
		*endpages[MAX_PAGES];
static uchar	*page_chapters[MAX_PAGES],
		*page_headings[MAX_PAGES];
static tree_t	*current_heading;

static int	num_links;
static link_t	links[MAX_LINKS];

static uchar	list_types[16];
static int	list_values[16];

static char	stdout_filename[256];
static int	num_objects,
		objects[MAX_OBJECTS],
		root_object,
		info_object,
		outline_object,
		pages_object,
		names_object,
		encrypt_object,
		annots_objects[MAX_PAGES],
		background_object = 0,
		font_objects[16];

static image_t	*logo_image = NULL;
static float	logo_width,
		logo_height;
static image_t	*background_image = NULL;
static float	background_color[3] = { 1.0, 1.0, 1.0 },
		link_color[3] = { 0.0, 0.0, 1.0 };

static int	render_typeface,
		render_style;
static float	render_size,
		render_rgb[3],
		render_x,
		render_y,
		render_startx;

static z_stream		compressor;
static uchar		comp_buffer[64 * 1024];
static uchar		encrypt_key[16];
static rc4_context_t	encrypt_state;
static md5_byte_t	file_id[16];


/*
 * Local functions...
 */

static char	*pspdf_prepare_page(int page, int *file_page, uchar *title,
		                    float title_width, uchar **page_chapter,
				    uchar **page_heading);
static void	pspdf_prepare_heading(int page, int print_page, uchar *title,
		        	      float title_width, uchar *chapter,
				      float chapter_width, uchar *heading,
				      float heading_width, char *format, int y);
static void	ps_write_document(uchar *title, uchar *author, uchar *creator,
		                  uchar *copyright, uchar *keywords);
static void	ps_write_page(FILE *out, int page, uchar *title,
		              float title_width, uchar **page_chapter,
			      uchar **page_heading);
static void	ps_write_background(FILE *out);
static void	pdf_write_document(uchar *title, uchar *author, uchar *creator,
		                   uchar *copyright, uchar *keywords,
				   tree_t *toc);
static void	pdf_write_page(FILE *out, int page, uchar *title,
		               float title_width, uchar **page_chapter,
			       uchar **page_heading);
static void	pdf_write_resources(FILE *out, int page);
static void	pdf_write_contents(FILE *out, tree_t *toc, int parent,
		                   int prev, int next, int *heading);
static void	pdf_write_background(FILE *out);
static void	pdf_write_links(FILE *out);
static void	pdf_write_names(FILE *out);
static int	pdf_count_headings(tree_t *toc);

static void	encrypt_init(void);
static void	flate_open_stream(FILE *out);
static void	flate_close_stream(FILE *out);
static void	flate_puts(char *s, FILE *out);
static void	flate_printf(FILE *out, char *format, ...);
static void	flate_write(FILE *out, uchar *inbuf, int length, int flush=0);	

static void	parse_contents(tree_t *t, float left, float width, float bottom,
		               float length, float *y, int *page, int *heading);
static void	parse_doc(tree_t *t, float left, float width, float bottom, float length,
		          float *x, float *y, int *page, tree_t *cpara,
			  int *needspace);
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
static void	parse_list(tree_t *t, float left, float width, float bottom,
		           float length, float *x, float *y, int *page,
			   int needspace);
static void	init_list(tree_t *t);
static void	parse_comment(tree_t *t, float left, float width, float bottom,
		              float length, float *x, float *y, int *page,
			      tree_t *para, int needspace);

static tree_t	*real_prev(tree_t *t);
static tree_t	*real_next(tree_t *t);

static void	add_link(uchar *name, int page, int top);
static link_t	*find_link(uchar *name);
static int	compare_links(link_t *n1, link_t *n2);

static void	find_background(tree_t *t);
static void	write_background(FILE *out);

static render_t	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data, int insert = 0);
static void	copy_tree(tree_t *parent, tree_t *t);
static tree_t	*flatten_tree(tree_t *t, float padding = 0.0f);
static float	get_width(uchar *s, int typeface, int style, int size);
static void	update_image_size(tree_t *t);
static uchar	*get_title(tree_t *doc);
static FILE	*open_file(void);
static void	set_color(FILE *out, float *rgb);
static void	set_font(FILE *out, int typeface, int style, float size);
static void	set_pos(FILE *out, float x, float y);
static void	write_prolog(FILE *out, int pages, uchar *title, uchar *author,
		             uchar *creator, uchar *copyright, uchar *keywords);
static void	ps_hex(FILE *out, uchar *data, int length);
static void	ps_ascii85(FILE *out, uchar *data, int length);
static void	jpg_init(j_compress_ptr cinfo);
static boolean	jpg_empty(j_compress_ptr cinfo);
static void	jpg_term(j_compress_ptr cinfo);
static void	jpg_setup(FILE *out, image_t *img, j_compress_ptr cinfo);
static int	compare_rgb(uchar *rgb1, uchar *rgb2);
static void	write_image(FILE *out, render_t *r);
static void	write_string(FILE *out, uchar *s, int compress);
static void	write_text(FILE *out, render_t *r);
static void	write_trailer(FILE *out, int pages);


/*
 * 'pspdf_export()' - Export PostScript/PDF file(s)...
 */

int
pspdf_export(tree_t *document,	/* I - Document to export */
             tree_t *toc)	/* I - Table of contents for document */
{
  uchar		*title,		/* Title text */
		*author,	/* Author of document */
		*creator,	/* HTML file creator (Netscape, etc) */
		*copyright,	/* File copyright */
		*docnumber,	/* Document number */
		*keywords;	/* Search keywords */
  tree_t	*t;		/* Title page document tree */
  FILE		*fp;		/* Title page file */
  float		x, y,		/* Current page position */
		width,		/* Width of title, author, etc */
		height;		/* Height of title area */
  int		page,		/* Current page # */
		heading;	/* Current heading # */
  float		top, bottom;	/* Top and bottom margins... */
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

 /*
  * Get the document title, author, etc...
  */

  title      = get_title(document);
  author     = htmlGetMeta(document, (uchar *)"author");
  creator    = htmlGetMeta(document, (uchar *)"generator");
  copyright  = htmlGetMeta(document, (uchar *)"copyright");
  docnumber  = htmlGetMeta(document, (uchar *)"docnumber");
  keywords   = htmlGetMeta(document, (uchar *)"keywords");
  logo_image = image_load(LogoImage, !OutputColor);

  if (logo_image != NULL)
  {
    logo_width  = logo_image->width * PagePrintWidth / _htmlBrowserWidth;
    logo_height = logo_width * logo_image->height / logo_image->width;
  }
  else
    logo_width = logo_height = 0.0f;

  find_background(document);
  get_color((uchar *)LinkColor, link_color, 0);

 /*
  * Initialize page rendering variables...
  */

  memset(pages, 0, sizeof(pages));
  memset(page_chapters, 0, sizeof(page_chapters));
  memset(page_headings, 0, sizeof(page_headings));
  memset(endpages, 0, sizeof(pages));
  memset(list_types, 0267, sizeof(list_types));
  memset(list_values, 0, sizeof(list_values));
  memset(chapter_starts, -1, sizeof(chapter_starts));
  memset(chapter_ends, -1, sizeof(chapter_starts));

  doc_time     = time(NULL);
  doc_date     = localtime(&doc_time);
  num_headings = 0;
  num_links    = 0;

  if (TitlePage)
  {
#if defined(WIN32) || defined(__EMX__)
    if (stricmp(file_extension(TitleImage), "htm") == 0 ||
	stricmp(file_extension(TitleImage), "html") == 0 ||
	stricmp(file_extension(TitleImage), "shtml") == 0)
#else
    if (strcmp(file_extension(TitleImage), "htm") == 0 ||
	strcmp(file_extension(TitleImage), "html") == 0 ||
	strcmp(file_extension(TitleImage), "shtml") == 0)
#endif // WIN32 || __EMX__
    {
      // Write a title page from HTML source...
      if ((fp = fopen(TitleImage, "rb")) == NULL)
      {
	progress_error("Unable to open title file \"%s\" - %s!",
                       TitleImage, strerror(errno));
	return (1);
      }

      t = htmlReadFile(NULL, fp, file_directory(TitleImage));
      fclose(fp);

      num_pages       = 0;
      page            = 0;
      title_page      = 1;
      current_heading = NULL;
      x               = 0.0;
      bottom          = 0;
      top             = PagePrintLength;
      y               = top;
      needspace       = 0;

      parse_doc(t, 0, PagePrintWidth, bottom, top, &x, &y, &page, NULL,
                &needspace);

      if (PageDuplex && (num_pages & 1))
	num_pages ++;

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

      num_pages = PageDuplex ? 2 : 1;

      height = 0.0;

      if (timage != NULL)
	height += timage_height + _htmlSpacings[SIZE_P];
      if (title != NULL)
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

      if (title != NULL)
      {
	width = get_width(title, _htmlHeadingFont, STYLE_BOLD, SIZE_H1);
	r     = new_render(0, RENDER_TEXT, (PagePrintWidth - width) * 0.5f,
                	   y - _htmlSpacings[SIZE_H1], width,
			   _htmlSizes[SIZE_H1], title);

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
  }
  else
    num_pages = 0;

 /*
  * Parse the document...
  */

  if (OutputBook)
    chapter = 0;
  else
  {
    chapter           = 1;
    TocDocCount       = 1;
    chapter_starts[1] = num_pages;
  }

  title_page      = 0;
  page            = num_pages;
  current_heading = NULL;
  x               = 0.0;
  bottom          = 0;
  top             = PagePrintLength;
  needspace       = 0;

  if (strncmp(Header, "...", 3) != 0)
  {
    if (strchr(Header, 'l') != NULL && logo_height > HeadFootSize)
      top -= logo_height + HeadFootSize;
    else
      top -= 2.0f * HeadFootSize;
  }

  if (strncmp(Footer, "...", 3) != 0)
  {
    if (strchr(Footer, 'l') != NULL && logo_height > HeadFootSize)
      bottom += logo_height + HeadFootSize;
    else
      bottom += 2.0f * HeadFootSize;
  }

  y = top;

  parse_doc(document, 0, PagePrintWidth, bottom, top, &x, &y, &page, NULL,
            &needspace);

  if (PageDuplex && (num_pages & 1))
    num_pages ++;
  chapter_ends[chapter] = num_pages - 1;

 /*
  * Parse the table-of-contents if necessary...
  */

  if (TocLevels > 0)
  {
    y                 = 0.0;
    page              = num_pages - 1;
    heading           = 0;
    chapter_starts[0] = num_pages;
    bottom            = 0;
    top               = PagePrintLength;

    if (strncmp(TocHeader, "...", 3) != 0)
    {
      if (strchr(TocHeader, 'l') != NULL && logo_height > HeadFootSize)
	top -= logo_height + HeadFootSize;
      else
	top -= 2.0f * HeadFootSize;
    }

    if (strncmp(TocFooter, "...", 3) != 0)
    {
      if (strchr(TocFooter, 'l') != NULL && logo_height > HeadFootSize)
	bottom += logo_height + HeadFootSize;
      else
	bottom += 2.0f * HeadFootSize;
    }

    parse_contents(toc, 0, PagePrintWidth, bottom, top, &y, &page, &heading);
    if (PageDuplex && (num_pages & 1))
      num_pages ++;
    chapter_ends[0] = num_pages - 1;
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

    if (PSLevel > 0)
      ps_write_document(title, author, creator, copyright, keywords);
    else
      pdf_write_document(title, author, creator, copyright, keywords, toc);
  }
  else
  {
   /*
    * No, show an error...
    */

    progress_error("Error: no pages generated! (did you remember to use webpage mode?");
  }

 /*
  * Free memory...
  */

  if (title != NULL)
    free(title);

  for (int i = 0; i < num_pages; i ++)
  {
    if (i == 0 || page_headings[i] != page_headings[i - 1])
      free(page_headings[i]);
  }

  return (0);
}


/*
 * 'pspdf_prepare_page()' - Add headers/footers to page before writing...
 */

static char *
pspdf_prepare_page(int   page,			/* I - Page number */
                   int   *file_page,		/* O - File page number */
        	   uchar *title,		/* I - Title string */
        	   float title_width,		/* I - Width of title string */
                   uchar **page_heading,	/* IO - Page heading string */
	           uchar **page_chapter)	/* IO - Page chapter string */
{
  int		print_page;			/* Printed page # */
  float		chapter_width,			/* Width of page chapter */
		heading_width;			/* Width of page heading */
  char		*page_text;			/* Page number text */


  DEBUG_printf(("pspdf_prepare_page(%d, %08x, \"%s\", %.1f, \"%s\")\n",
                page, file_page, title ? title : "(null)", title_width,
		*page_heading ? *page_heading : "(null)"));

  if (OutputFiles && chapter >= 0)
    *file_page = page - chapter_starts[chapter] + 1;
  else if (chapter < 0)
    *file_page = page + 1;
  else if (chapter == 0)
  {
    *file_page = page - chapter_starts[0] + 1;

    if (TitlePage)
      *file_page += chapter_starts[1];
  }
  else
  {
    *file_page = page - chapter_starts[1] + 1;

    if (TocLevels > 0)
      *file_page += chapter_ends[0] - chapter_starts[0] + 1;

    if (TitlePage)
      *file_page += chapter_starts[1];
  }

 /*
  * Get the new heading if necessary...
  */

  if (page_chapters[page] != NULL)
    *page_chapter = page_chapters[page];
  if (page_headings[page] != NULL)
    *page_heading = page_headings[page];

 /*
  * Make a page number; use roman numerals for the table of contents
  * and arabic numbers for all others...
  */

  if (chapter == 0)
  {
    print_page = page - chapter_starts[0] + 1;
    page_text  = format_number(print_page, 'i');
  }
  else if (chapter < 0)
    page_text = (page & 1) ? (char *)"eltit" : (char *)"title";
  else
  {
    print_page = page - chapter_starts[1] + 1;
    page_text  = format_number(print_page, '1');
  }

  if (Verbosity)
  {
    progress_show("Writing page %s...", page_text);
    progress_update(100 * page / num_pages);
  }

 /*
  * Add page headings...
  */

  chapter_width = get_width(*page_chapter, HeadFootType, HeadFootStyle, SIZE_P) *
                  HeadFootSize / _htmlSizes[SIZE_P];
  heading_width = get_width(*page_heading, HeadFootType, HeadFootStyle, SIZE_P) *
                  HeadFootSize / _htmlSizes[SIZE_P];

  if (chapter == 0)
  {
   /*
    * Add table-of-contents header & footer...
    */

    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading, heading_width, TocHeader,
			  PagePrintLength);
    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading,  heading_width, TocFooter, 0);
  }
  else if (chapter > 0)
  {
   /*
    * Add chapter header & footer...
    */

    if (page > chapter_starts[chapter] || !OutputBook)
      pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                            chapter_width, *page_heading, heading_width, Header,
			    PagePrintLength);
    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading, heading_width, Footer, 0);
  }

  return (page_text);
}


/*
 * 'pspdf_prepare_heading()' - Add headers/footers to page before writing...
 */

static void
pspdf_prepare_heading(int   page,		/* I - Page number */
                      int   print_page,         /* I - Printed page number */
        	      uchar *title,		/* I - Title string */
        	      float title_width,	/* I - Width of title string */
        	      uchar *chapter,		/* I - Page chapter string */
		      float chapter_width,	/* I - Width of chapter */
        	      uchar *heading,		/* I - Page heading string */
		      float heading_width,	/* I - Width of heading */
		      char  *format,		/* I - Format of heading */
		      int   y)			/* I - Baseline of heading */
{
  int		pos,		/* Position in heading */
		dir;		/* Direction of page */
  char		*number;	/* Page number */
  char		nofN[256];	/* n/N page number */
  char		date[256];	/* Date string */
  render_t	*temp;		/* Render structure for titles, etc. */


  DEBUG_printf(("pspdf_prepare_heading(%d, %d, \"%s\", %.1f, \"%s\", %.1f, \"%s\", %d)\n",
                page, print_page, title ? title : "(null)", title_width,
		heading ? heading : "(null)", heading_width, format, y));

 /*
  * Return right away if there is nothing to do...
  */

  if (strncmp(format, "...", 3) == 0)
    return;

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

    switch (*format)
    {
      case '.' :
      default :
          temp = NULL;
	  break;

      case '1' :
      case 'i' :
      case 'I' :
      case 'a' :
      case 'A' :
          number = format_number(print_page, *format);
	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)number, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, number);
          break;

      case '/' : /* n/N */
          strcpy(nofN, format_number(print_page, '1'));
	  strcat(nofN, "/");
	  strcat(nofN, format_number(chapter_ends[TocDocCount] -
	                             chapter_starts[1] + 1, '1'));

	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)nofN, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, nofN);
          break;

      case 'd' : /* Date */
          strftime(date, sizeof(date), "%x", doc_date);

	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)date, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, date);
          break;

      case 'D' : /* Date and time */
          strftime(date, sizeof(date), "%c", doc_date);

	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)date, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, date);
          break;

      case 'T' : /* Time */
          strftime(date, sizeof(date), "%X", doc_date);

	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)date, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, date);
          break;

      case 't' :
          if (title != NULL)
	    temp = new_render(page, RENDER_TEXT, 0, y, title_width,
	                      HeadFootSize, title);
          else
	    temp = NULL;
          break;

      case 'c' :
          if (chapter != NULL)
	    temp = new_render(page, RENDER_TEXT, 0, y, chapter_width,
	                      HeadFootSize, chapter);
          else
	    temp = NULL;
          break;

      case 'C' :
          number = format_number(print_page - chapter_starts[::chapter] + 2, '1');
	  temp   = new_render(page, RENDER_TEXT, 0, y,
                              HeadFootSize / _htmlSizes[SIZE_P] *
			      get_width((uchar *)number, HeadFootType,
			                HeadFootStyle, SIZE_P),
			      HeadFootSize, number);
          break;

      case 'h' :
          if (heading != NULL)
	    temp = new_render(page, RENDER_TEXT, 0, y, heading_width,
	                      HeadFootSize, heading);
          else
	    temp = NULL;
          break;

      case 'l' :
          if (logo_image != NULL)
	  {
	    if (y < (PagePrintLength / 2))
	      temp = new_render(page, RENDER_IMAGE, 0, y, logo_width,
	                        logo_height, logo_image);
            else // Offset from top
	      temp = new_render(page, RENDER_IMAGE, 0,
	                        y + HeadFootSize - logo_height,
	                	logo_width, logo_height, logo_image);
          }
	  else
	    temp = NULL;
	  break;
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
ps_write_document(uchar *title,		/* I - Title on all pages */
        	  uchar *author,	/* I - Author of document */
        	  uchar *creator,	/* I - Application that generated the HTML file */
        	  uchar *copyright,	/* I - Copyright (if any) on the document */
                  uchar *keywords)	/* I - Search keywords */
{
  uchar		*page_chapter,	/* Current chapter text */
		*page_heading;	/* Current heading text */
  FILE		*out;		/* Output file */
  int		page;		/* Current page # */
  float		title_width;	/* Width of title string */


 /*
  * Get the document title width...
  */

  if (title != NULL)
    title_width = HeadFootSize / _htmlSizes[SIZE_P] *
                  get_width(title, HeadFootType, HeadFootStyle, SIZE_P);

 /*
  * Write the title page(s)...
  */

  chapter      = -1;
  page_chapter = NULL;
  page_heading = NULL;

  if (!OutputFiles)
  {
    out = open_file();

    if (out == NULL)
    {
      progress_error("Unable to open output file - %s\n", strerror(errno));
      return;
    }

    write_prolog(out, num_pages, title, author, creator, copyright, keywords);
  }

  if (TitlePage)
  {
    if (OutputFiles)
    {
      out = open_file();
      write_prolog(out, PageDuplex + 1, title, author, creator, copyright,
                   keywords);
    }

    for (page = 0; page < chapter_starts[1]; page ++)
      ps_write_page(out, page, NULL, 0.0, &page_chapter, &page_heading);

    if (OutputFiles)
    {
      write_trailer(out, 0);
      fclose(out);
    }
  }

  if (TocLevels > 0)
    chapter = 0;
  else
    chapter = 1;

  for (; chapter <= TocDocCount; chapter ++)
  {
    if (chapter_starts[chapter] < 0)
      continue;

    if (OutputFiles)
    {
      out = open_file();
      if (out == NULL)
      {
        progress_error("Unable to create output file - %s\n", strerror(errno));
        return;
      }

      write_prolog(out, chapter_ends[chapter] - chapter_starts[chapter] + 1,
                   title, author, creator, copyright, keywords);
    }

    for (page = chapter_starts[chapter], page_heading = NULL;
         page <= chapter_ends[chapter];
         page ++)
      ps_write_page(out, page, title, title_width, &page_chapter, &page_heading);

   /*
    * Close the output file as necessary...
    */

    if (OutputFiles)
    {
      write_trailer(out, 0);
      fclose(out);
    }
  }

 /*
  * Close the output file as necessary...
  */

  if (!OutputFiles)
  {
    write_trailer(out, 0);
    if (out != stdout)
      fclose(out);
  }

  if (Verbosity)
    progress_hide();
}


/*
 * 'ps_write_page()' - Write all render entities on a page to a PostScript file.
 */

static void
ps_write_page(FILE  *out,		/* I - Output file */
              int   page,		/* I - Page number */
              uchar *title,		/* I - Title string */
              float title_width,	/* I - Width of title string */
              uchar **page_heading,	/* IO - Page heading string */
	      uchar **page_chapter)	/* IO - Page chapter string */
{
  int		file_page;	/* Current page # in document */
  char		*page_text;	/* Page number text */
  render_t	*r,		/* Render pointer */
		*next;		/* Next render */


  if (page < 0 || page >= MAX_PAGES)
    return;

  DEBUG_printf(("ps_write_page(%08x, %d, \"%s\", %.1f, \"%s\")\n",
                out, page, title ? title : "(null)", title_width,
		*page_heading ? *page_heading : "(null)"));

 /*
  * Add headers/footers as needed...
  */

  page_text = pspdf_prepare_page(page, &file_page, title, title_width,
                                 page_chapter, page_heading);

 /*
  * Clear the render cache...
  */

  render_typeface = -1;
  render_style    = -1;
  render_size     = -1;
  render_rgb[0]   = 0.0;
  render_rgb[1]   = 0.0;
  render_rgb[2]   = 0.0;
  render_x        = -1.0;
  render_y        = -1.0;

 /*
  * Output the page prolog...
  */

  fprintf(out, "%%%%Page: %s %d\n", page_text, file_page);

  fputs("GS\n", out);

  if (Landscape && !PSCommands)
  {
    if (PageDuplex && (page & 1))
      fprintf(out, "0 %d T -90 rotate\n", PageLength);
    else
      fprintf(out, "%d 0 T 90 rotate\n", PageWidth);
  }

  write_background(out);

  if (PageDuplex && (page & 1))
    fprintf(out, "%d %d T\n", PageRight, PageBottom);
  else
    fprintf(out, "%d %d T\n", PageLeft, PageBottom);

 /*
  * Render all page elements, freeing used memory as we go...
  */

  for (r = pages[page], next = NULL; r != NULL; r = next)
  {
    switch (r->type)
    {
      case RENDER_IMAGE :
          write_image(out, r);
          break;
      case RENDER_TEXT :
          write_text(out, r);
          break;
      case RENDER_BOX :
          set_color(out, r->data.box);
          set_pos(out, r->x, r->y);
          if (r->height > 0.0)
            fprintf(out, " %.1f %.1f B\n", r->width, r->height);
          else
            fprintf(out, " %.1f L\n", r->width);
          render_x = -1;
          break;
      case RENDER_FBOX :
          set_color(out, r->data.box);
          set_pos(out, r->x, r->y);
          if (r->height > 0.0)
            fprintf(out, " %.1f %.1f F\n", r->width, r->height);
          else
            fprintf(out, " %.1f L\n", r->width);
          render_x = -1;
          break;
    }

    next = r->next;
    free(r);
  }

 /*
  * Output the page trailer...
  */

  fputs("GR\n", out);
  fputs("SP\n", out);
}


/*
 * 'ps_write_background()' - Write a background image...
 */

static void
ps_write_background(FILE *out)		/* I - Output file */
{
  int	y,				/* Current line */
	pwidth;				/* Pixel width */


  pwidth = background_image->width * background_image->depth;

  fputs("/BG[", out);
  for (y = 0; y < background_image->height; y ++)
  {
    putc('<', out);
    ps_hex(out, background_image->pixels + y * pwidth, pwidth);
    putc('>', out);
  }
  fputs("]def", out);
}


/*
 * 'pdf_write_document()' - Write all render entities to a PDF file.
 */

static void
pdf_write_document(uchar  *title,	/* I - Title for all pages */
        	   uchar  *author,	/* I - Author of document */
        	   uchar  *creator,	/* I - Application that generated the HTML file */
        	   uchar  *copyright,	/* I - Copyright (if any) on the document */
                   uchar  *keywords,	/* I - Search keywords */
                   tree_t *toc)		/* I - Table of contents tree */
{
  uchar		*page_chapter,	/* Current chapter text */
		*page_heading;	/* Current heading text */
  FILE		*out;		/* Output file */
  int		page,		/* Current page # */
		heading;	/* Current heading # */
  float		title_width;	/* Width of title string */
  int		bytes;		/* Number of bytes */
  char		buffer[8192];	/* Copy buffer */


  if (title != NULL)
    title_width = HeadFootSize / _htmlSizes[SIZE_P] *
                  get_width(title, HeadFootType, HeadFootStyle, SIZE_P);

  out = open_file();
  if (out == NULL)
  {
    progress_error("Unable to write document file - %s\n", strerror(errno));
    return;
  }

  write_prolog(out, num_pages, title, author, creator, copyright, keywords);

  pdf_write_links(out);
  if (PDFVersion >= 1.2)
    pdf_write_names(out);

  num_objects ++;
  if (pages_object != num_objects)
    progress_error("Internal error: pages_object != num_objects");

  objects[num_objects] = ftell(out);
  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fputs("/Type/Pages", out);
  if (Landscape)
    fprintf(out, "/MediaBox[0 0 %d %d]", PageLength, PageWidth);
  else
    fprintf(out, "/MediaBox[0 0 %d %d]", PageWidth, PageLength);

  fprintf(out, "/Count %d", num_pages);
  fputs("/Kids[", out);

  if (TitlePage)
    for (page = 0; page < chapter_starts[1]; page ++)
      fprintf(out, "%d 0 R\n", pages_object + 1 + page * 3);

  if (TocLevels > 0)
    chapter = 0;
  else
    chapter = 1;

  for (; chapter <= TocDocCount; chapter ++)
    for (page = chapter_starts[chapter]; page <= chapter_ends[chapter]; page ++)
      if (page < MAX_PAGES)
        fprintf(out, "%d 0 R\n", pages_object + 3 * page + 1);
  fputs("]", out);
  fputs(">>", out);
  fputs("endobj\n", out);

  page_chapter = NULL;
  page_heading = NULL;
  chapter      = -1;

  if (TitlePage)
    for (page = 0; page < chapter_starts[1]; page ++)
      pdf_write_page(out, page, NULL, 0.0, &page_chapter, &page_heading);

  for (chapter = 1; chapter <= TocDocCount; chapter ++)
  {
    if (chapter_starts[chapter] < 0)
      continue;

    for (page = chapter_starts[chapter], page_heading = NULL;
         page <= chapter_ends[chapter];
         page ++)
      pdf_write_page(out, page, title, title_width, &page_chapter, &page_heading);
  }

  if (TocLevels > 0)
  {
    for (chapter = 0, page = chapter_starts[0], page_heading = NULL;
	 page <= chapter_ends[0];
	 page ++)
      pdf_write_page(out, page, title, title_width, &page_chapter, &page_heading);

   /*
    * Write the outline tree...
    */

    heading = 0;
    pdf_write_contents(out, toc, 0, 0, 0, &heading);
  }
  else
    outline_object = 0;

 /*
  * Write the trailer and close the output file...
  */

  write_trailer(out, 0);

#ifdef MAC
  //
  // On the MacOS, files are not associated with applications by extensions.
  // Instead, it uses a pair of values called the type & creator.  
  // This block of code sets the those values for PDF files.
  //

  FCBPBRec    fcbInfo;	// File control block information
  Str32	name;		// Name of file
  FInfo	fInfo;		// File type/creator information
  FSSpec	fSpec;		// File specification


  memset(&fcbInfo, 0, sizeof(FCBPBRec));
  fcbInfo.ioRefNum  = out->handle;
  fcbInfo.ioNamePtr = name;
  if (!PBGetFCBInfoSync(&fcbInfo))
    if (FSMakeFSSpec(fcbInfo.ioFCBVRefNum, fcbInfo.ioFCBParID, name, &fSpec) == noErr)
    {
      FSpGetFInfo(&fSpec, &fInfo);
      fInfo.fdType    = 'PDF ';
      fInfo.fdCreator = 'CARO';
      FSpSetFInfo(&fSpec, &fInfo);
    }

  //
  // Now that the PDF file is associated with that type, close the file.
  //

  fclose(out);
#else
  fclose(out);
#endif // MAC

  //
  // If we are sending the output to stdout, copy the temp file now...
  //

  if (!OutputPath[0])
  {
#if defined(WIN32) || defined(__EMX__)
    // Make sure we are in binary mode...  stupid Microsoft!
    setmode(1, O_BINARY);
#endif // WIN32 || __EMX__

    // Open the temporary file and copy it to stdout...
    out = fopen(stdout_filename, "rb");

    while ((bytes = fread(buffer, 1, sizeof(buffer), out)) > 0)
      fwrite(buffer, 1, bytes, stdout);

    // Close and remove the temporary file...
    fclose(out);
    unlink(stdout_filename);
  }

  if (Verbosity)
    progress_hide();
}


/*
 * 'pdf_write_resources()' - Write the resources dictionary for a page.
 */

static void
pdf_write_resources(FILE *out,	/* I - Output file */
                    int  page)	/* I - Page for resources */
{
  int		i;		/* Looping var */
  render_t	*r;		/* Render pointer */
  int		fonts_used[16];	/* Non-zero if the page uses a font */
  int		images_used;	/* Non-zero if the page uses an image */
  int		text_used;	/* Non-zero if the page uses text */
  static char	*effects[] =	/* Effects and their commands */
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
  images_used = background_object > 0;
  text_used   = 0;

  for (r = pages[page]; r != NULL; r = r->next)
    if (r->type == RENDER_IMAGE)
      images_used = 1;
    else if (r->type == RENDER_TEXT)
    {
      text_used = 1;
      fonts_used[r->data.text.typeface * 4 + r->data.text.style] = 1;
    }

  fputs("/Resources<<", out);

  if (!images_used)
    fputs("/ProcSet[/PDF/Text]", out);
  else if (PDFVersion >= 1.2)
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
    for (i = 0; i < 16; i ++)
      if (fonts_used[i])
	fprintf(out, "/F%1x %d 0 R", i, font_objects[i]);
    fputs(">>", out);
  }

  if (background_object > 0)
    fprintf(out, "/XObject<</BG %d 0 R>>", background_object);

  if (PDFEffect)
    fprintf(out, "/Dur %.0f/Trans<</D %.1f%s>>", PDFPageDuration,
            PDFEffectDuration, effects[PDFEffect]);

  fputs(">>", out);
}


/*
 * 'pdf_write_page()' - Write a page to a PDF file.
 */

static void
pdf_write_page(FILE  *out,		/* I - Output file */
               int   page,		/* I - Page number */
               uchar  *title,		/* I - Title string */
               float title_width,	/* I - Width of title string */
               uchar  **page_heading,	/* IO - Page heading string */
	       uchar  **page_chapter)	/* IO - Page chapter string */
{
  int		file_page,	/* Current page # in file */
		length,		/* Stream length */
		last_render;	/* Last type of render */
  render_t	*r,		/* Render pointer */
		*next;		/* Next render */


  if (page < 0 || page >= MAX_PAGES)
    return;

 /*
  * Add headers/footers as needed...
  */

  pspdf_prepare_page(page, &file_page, title, title_width, page_heading, page_chapter);

 /*
  * Clear the render cache...
  */

  render_typeface = -1;
  render_style    = -1;
  render_size     = -1;
  render_rgb[0]   = 0.0;
  render_rgb[1]   = 0.0;
  render_rgb[2]   = 0.0;
  render_x        = -1.0;
  render_y        = -1.0;

 /*
  * Output the page prolog...
  */

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fputs("/Type/Page", out);
  fprintf(out, "/Parent %d 0 R", pages_object);
  fprintf(out, "/Contents %d 0 R", num_objects + 1);

  pdf_write_resources(out, page);

 /*
  * Actions (links)...
  */

  if (annots_objects[page] > 0)
    fprintf(out, "/Annots %d 0 R", annots_objects[page]);

  fputs(">>", out);
  fputs("endobj\n", out);

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fprintf(out, "/Length %d 0 R", num_objects + 1);
  if (Compression)
    fputs("/Filter/FlateDecode", out);
  fputs(">>", out);
  fputs("stream\n", out);

  length = ftell(out);

  flate_open_stream(out);

  flate_puts("q\n", out);
  write_background(out);

  if (PageDuplex && (page & 1))
    flate_printf(out, "1 0 0 1 %d %d cm\n", PageRight, PageBottom);
  else
    flate_printf(out, "1 0 0 1 %d %d cm\n", PageLeft, PageBottom);

  flate_puts("BT\n", out);
  last_render = RENDER_TEXT;

 /*
  * Render all page elements, freeing used memory as we go...
  */

  for (r = pages[page], next = NULL; r != NULL; r = next)
  {
    if (r->type != last_render)
    {
      if (r->type == RENDER_TEXT)
      {
	render_x = -1.0;
	render_y = -1.0;
        flate_puts("BT\n", out);
      }
      else if (last_render == RENDER_TEXT)
        flate_puts("ET\n", out);

      last_render = r->type;
    }

    switch (r->type)
    {
      case RENDER_IMAGE :
          write_image(out, r);
          break;
      case RENDER_TEXT :
          write_text(out, r);
          break;
      case RENDER_BOX :
          if (r->height == 0.0)
            flate_printf(out, "%.2f %.2f %.2f RG %.1f %.1f m %.1f %.1f l S\n",
                       r->data.box[0], r->data.box[1], r->data.box[2],
                       r->x, r->y, r->x + r->width, r->y);
          else
            flate_printf(out, "%.2f %.2f %.2f RG %.1f %.1f %.1f %.1f re S\n",
                       r->data.box[0], r->data.box[1], r->data.box[2],
                       r->x, r->y, r->width, r->height);
          break;
      case RENDER_FBOX :
          if (r->height == 0.0)
            flate_printf(out, "%.2f %.2f %.2f RG %.1f %.1f m %.1f %.1f l S\n",
                       r->data.box[0], r->data.box[1], r->data.box[2],
                       r->x, r->y, r->x + r->width, r->y);
          else
          {
            set_color(out, r->data.fbox);
            flate_printf(out, "%.1f %.1f %.1f %.1f re f\n",
                       r->x, r->y, r->width, r->height);
          }
          break;
    }

    next = r->next;
    free(r);
  }

 /*
  * Output the page trailer...
  */

  if (last_render == RENDER_TEXT)
   flate_puts("ET\n", out);

  flate_puts("Q\n", out);
  flate_close_stream(out);
  length = ftell(out) - length;
  fputs("endstream\n", out);
  fputs("endobj\n", out);

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj\n", num_objects);
  fprintf(out, "%d\n", length);
  fputs("endobj\n", out);
}


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
  int		entry_counts[MAX_HEADINGS],	/* Number of sub-entries for this entry */
		entry_objects[MAX_HEADINGS];	/* Objects for each entry */
  tree_t	*entries[MAX_HEADINGS];		/* Pointers to each entry */


 /*
  * Make an object for this entry...
  */

  num_objects ++;
  thisobj = num_objects;

  if (toc == NULL)
  {
   /*
    * This is for the Table of Contents page...
    */

    objects[thisobj] = ftell(out);
    fprintf(out, "%d 0 obj", thisobj);
    fputs("<<", out);
    fprintf(out, "/Parent %d 0 R", parent);

    fputs("/Title", out);
    write_string(out, (uchar *)TocTitle, 0);

    fprintf(out, "/Dest[%d 0 R/XYZ null %d null]",
            pages_object + 3 * chapter_starts[0] + 1,
            PagePrintLength + PageBottom);

    if (prev > 0)
      fprintf(out, "/Prev %d 0 R", prev);

    if (next > 0)
      fprintf(out, "/Next %d 0 R", next);

    fputs(">>", out);
    fputs("endobj\n", out);
  }
  else
  {
   /*
    * Find and count the children (entries)...
    */

    if (toc->markup == MARKUP_B || toc->markup == MARKUP_LI)
    {
      if (toc->next != NULL && toc->next->markup == MARKUP_UL)
	temp = toc->next->child;
      else
	temp = NULL;
    }
    else
      temp = toc->child;

    if (parent == 0 && TocLevels > 0)
    {
     /*
      * Add the table of contents to the top-level contents...
      */

      entries[0]       = NULL;
      entry_objects[0] = thisobj + 1;
      entry            = thisobj + 2;
      count            = 1;
    }
    else
    {
      entry = thisobj + 1;
      count = 0;
    }

    for (; temp != NULL && count < MAX_HEADINGS; temp = temp->next)
      if (temp->markup == MARKUP_B || temp->markup == MARKUP_LI)
      {
	entries[count]       = temp;
	entry_objects[count] = entry;
	if (temp->next != NULL && temp->next->markup == MARKUP_UL)
          entry_counts[count] = pdf_count_headings(temp->next->child);
	else
          entry_counts[count] = 0;
	entry += entry_counts[count] + 1;
	count ++;
      }

   /*
    * Output the top-level object...
    */

    objects[thisobj] = ftell(out);
    fprintf(out, "%d 0 obj", thisobj);
    fputs("<<", out);
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

    if (toc->markup == MARKUP_B || toc->markup == MARKUP_LI)
    {
      if ((text = htmlGetText(toc->child)) != NULL)
      {
	fputs("/Title", out);
	write_string(out, text, 0);
	free(text);
      }

      if (heading_pages[*heading] > 0)
	fprintf(out, "/Dest[%d 0 R/XYZ null %d null]",
        	pages_object + 3 * heading_pages[*heading] + ((PageDuplex && TitlePage) ? 4 : 1),
        	heading_tops[*heading]);

      (*heading) ++;
    }

    if (prev > 0)
      fprintf(out, "/Prev %d 0 R", prev);

    if (next > 0)
      fprintf(out, "/Next %d 0 R", next);

    fputs(">>", out);
    fputs("endobj\n", out);

    for (i = 0; i < count ; i ++)
      pdf_write_contents(out, entries[i], thisobj, i > 0 ? entry_objects[i - 1] : 0,
                	 i < (count - 1) ? entry_objects[i + 1] : 0,
                	 heading);
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
    if (toc->markup == MARKUP_B || toc->markup == MARKUP_LI)
      headings ++;
    else if (toc->markup == MARKUP_UL && toc->child != NULL)
      headings += pdf_count_headings(toc->child);

  return (headings);
}


/*
 * 'pdf_write_background()' - Write a background image...
 */

static void
pdf_write_background(FILE *out)		/* I - Output file */
{
  int	length;				/* Length of image */


  num_objects ++;
  background_object = num_objects;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fputs("/Type/XObject", out);
  fputs("/Subtype/Image", out);
  fputs("/Name/BG", out);
  if (background_image->depth == 1)
    fputs("/ColorSpace/DeviceGray", out);
  else
    fputs("/ColorSpace/DeviceRGB", out);
  fputs("/Interpolate true", out);
  fprintf(out, "/Width %d/Height %d/BitsPerComponent 8",
      	  background_image->width, background_image->height); 
  fprintf(out, "/Length %d 0 R", num_objects + 1);
  if (Compression)
    fputs("/Filter/FlateDecode", out);
  fputs(">>", out);
  fputs("stream\n", out);

  length = ftell(out);

  flate_open_stream(out);
  flate_write(out, background_image->pixels,
              background_image->width * background_image->height *
	      background_image->depth);
  flate_close_stream(out);

  length = ftell(out) - length;
  fputs("endstream\n", out);
  fputs("endobj\n", out);

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj\n", num_objects);
  fprintf(out, "%d\n", length);
  fputs("endobj\n", out);

}


/*
 * 'pdf_write_links()' - Write annotation link objects for each page in the
 *                       document.
 */

static void
pdf_write_links(FILE *out)		/* I - Output file */
{
  int		page,			/* Current page */
		lobj,			/* Current link */
		num_lobjs,		/* Number of links on this page */
		lobjs[2 * MAX_LINKS];	/* Link objects */
  float		x, y;			/* Position of last link */
  render_t	*r,			/* Current render primitive */
		*rlast,			/* Last render link primitive */
		*rprev;			/* Previous render primitive */
  link_t	*link;			/* Local link */


 /*
  * First combine adjacent, identical links...
  */

  for (page = 0; page < num_pages; page ++)
    for (r = pages[page], x = 0.0f, y = 0.0f, rlast = NULL, rprev = NULL;
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

 /*
  * Figure out how many link objects we'll have...
  */

  pages_object = num_objects + 1;

  for (page = 0; page < num_pages; page ++)
  {
    num_lobjs = 0;

    for (r = pages[page]; r != NULL; r = r->next)
      if (r->type == RENDER_LINK)
      {
        if (find_link(r->data.link) != NULL)
          num_lobjs ++;
        else
          num_lobjs += 2;
      }

    if (num_lobjs > 0)
      pages_object += num_lobjs + 1;
  }

 /*
  * Add space for named links for PDF 1.2 output...
  */

  if (PDFVersion >= 1.2)
    pages_object += num_links + 3;

 /*
  * Then generate annotation objects for all the links...
  */

  memset(annots_objects, 0, sizeof(annots_objects));

  for (page = 0; page < num_pages; page ++)
  {
    num_lobjs = 0;

    for (r = pages[page]; r != NULL; r = r->next)
      if (r->type == RENDER_LINK)
      {
        if ((link = find_link(r->data.link)) != NULL)
	{
	 /*
          * Local link...
          */

          num_objects ++;
          lobjs[num_lobjs ++] = num_objects;
          objects[num_objects] = ftell(out);

          fprintf(out, "%d 0 obj", num_objects);
          fputs("<<", out);
          fputs("/Subtype/Link", out);
          if (PageDuplex && (page & 1))
            fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                    r->x + PageRight, r->y + PageBottom - 2,
                    r->x + r->width + PageRight, r->y + r->height + PageBottom);
          else
            fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                    r->x + PageLeft, r->y + PageBottom - 2,
                    r->x + r->width + PageLeft, r->y + r->height + PageBottom);
          fputs("/Border[0 0 0]", out);
	  fprintf(out, "/Dest[%d 0 R/XYZ null %d 0]",
        	  pages_object + 3 * link->page + 4,
        	  link->top);
          fputs(">>", out);
          fputs("endobj\n", out);
	}
	else
	{
	 /*
          * Remote link...
          */

          num_objects ++;
          objects[num_objects] = ftell(out);

          fprintf(out, "%d 0 obj", num_objects);
          fputs("<<", out);
	  if (PDFVersion >= 1.2 &&
              file_method((char *)r->data.link) == NULL &&
#if defined(WIN32) || defined(__EMX__)
              strcasecmp(file_extension((char *)r->data.link), "pdf") == 0)
#else
              strcmp(file_extension((char *)r->data.link), "pdf") == 0)
#endif /* WIN32 || __EMX__ */
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
	    * Link to web file...
	    */

            fputs("/S/URI", out);
            fputs("/URI", out);
	    write_string(out, r->data.link, 0);
	  }

          fputs(">>", out);
          fputs("endobj\n", out);

          num_objects ++;
          lobjs[num_lobjs ++] = num_objects;
          objects[num_objects] = ftell(out);

          fprintf(out, "%d 0 obj", num_objects);
          fputs("<<", out);
          fputs("/Subtype/Link", out);
          if (PageDuplex && (page & 1))
            fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                    r->x + PageRight, r->y + PageBottom,
                    r->x + r->width + PageRight, r->y + r->height + PageBottom);
          else
            fprintf(out, "/Rect[%.1f %.1f %.1f %.1f]",
                    r->x + PageLeft, r->y + PageBottom - 2,
                    r->x + r->width + PageLeft, r->y + r->height + PageBottom);
          fputs("/Border[0 0 0]", out);
	  fprintf(out, "/A %d 0 R", num_objects - 1);
          fputs(">>", out);
          fputs("endobj\n", out);
	}
      }

    if (num_lobjs > 0)
    {
      num_objects ++;
      annots_objects[page] = num_objects;
      objects[num_objects] = ftell(out);

      fprintf(out, "%d 0 obj", num_objects);
      fputs("[", out);
      for (lobj = 0; lobj < num_lobjs; lobj ++)
        fprintf(out, "%d 0 R\n", lobjs[lobj]);
      fputs("]", out);
      fputs("endobj\n", out);
    }
  }
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

  num_objects ++;
  names_object = num_objects;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fprintf(out, "/Dests %d 0 R", num_objects + 1);
  fputs(">>", out);
  fputs("endobj\n", out);

 /*
  * Write the name tree child list...
  */

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fprintf(out, "/Kids[%d 0 R]", num_objects + 1);
  fputs(">>", out);
  fputs("endobj\n", out);

 /*
  * Write the leaf node for the name tree...
  */

  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);

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

  fputs(">>", out);
  fputs("endobj\n", out);

  for (i = num_links, link = links; i > 0; i --, link ++)
  {
    num_objects ++;
    objects[num_objects] = ftell(out);

    fprintf(out, "%d 0 obj", num_objects);
    fputs("<<", out);
    fprintf(out, "/D[%d 0 R/XYZ null %d null]", 
            pages_object + 3 * link->page + ((TocLevels > 0 && PageDuplex) ? 4 : 1),
            link->top);
    fputs(">>", out);
    fputs("endobj\n", out);
  }

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
               int    *heading)		/* IO - Heading # */
{
  float		x,
		width,
		numberwidth,
		height,
		rgb[3];
  uchar		number[255],
		*nptr,
		*link;
  tree_t	*flat,
		*temp,
		*next;
  render_t	*r;
#define dot_width  (_htmlSizes[SIZE_P] * _htmlWidths[t->typeface][t->style]['.'])


  DEBUG_printf(("parse_contents(t=%08x, y=%.1f, page=%d, heading=%d)\n",
                t, *y, *page, *heading));

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_B :	/* Top-level TOC */
          if (t->prev != NULL)	/* Advance one line prior to top-levels... */
            *y -= _htmlSpacings[SIZE_P];

      case MARKUP_LI :	/* Lower-level TOC */
          DEBUG_printf(("parse_contents: heading=%d, page = %d\n", *heading,
                        heading_pages[*heading]));

         /*
          * Put the text...
          */

          flat = flatten_tree(t->child);

	  for (height = 0.0, temp = flat; temp != NULL; temp = temp->next)
	    if (temp->height > height)
              height = temp->height;

          height *= _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P];

          x  = left + 36.0f * t->indent;
	  *y -= height;

	 /*
	  * Get the width of the page number, leave room for three dots...
	  */

          sprintf((char *)number, "%d", heading_pages[*heading]);
          numberwidth = get_width(number, t->typeface, t->style, t->size) +
	                3.0f * dot_width;

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
              width = get_width((uchar *)TocTitle, TYPE_HELVETICA, STYLE_BOLD, SIZE_H1);
              *y = top - _htmlSpacings[SIZE_H1];
              x  = left + 0.5f * (right - left - width);
              r = new_render(*page, RENDER_TEXT, x, *y, 0, 0, TocTitle);
              r->data.text.typeface = TYPE_HELVETICA;
              r->data.text.style    = STYLE_BOLD;
              r->data.text.size     = _htmlSizes[SIZE_H1];
	      get_color(_htmlTextColor, r->data.text.rgb);

              *y -= _htmlSpacings[SIZE_H1];
	      x  = left + 36.0f * t->indent;
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

	      if (PSLevel == 0)
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

        	    add_link(link, *page, (int)(*y + 6 * height));
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
			     image_find((char *)htmlGetVariable(temp, (uchar *)"SRC")));
		  break;
	    }

	    x += temp->width;
	    next = temp->next;
	    free(temp);
	  }

         /*
          * Draw dots leading up to the page number...
          */

          sprintf((char *)number, "%d", heading_pages[*heading]);
          width = get_width(number, t->typeface, t->style, t->size) + x;

          for (nptr = number; width < right; width += dot_width, nptr ++)
            *nptr = '.';
          nptr --;
          sprintf((char *)nptr, "%d", heading_pages[*heading]);

          r = new_render(*page, RENDER_TEXT, right - width + x, *y, 0, 0, number);
          r->data.text.typeface = t->typeface;
          r->data.text.style    = t->style;
          r->data.text.size     = _htmlSizes[t->size];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

         /*
          * Next heading...
          */

          (*heading) ++;
          break;

      default :
          parse_contents(t->child, left, right, bottom, top, y, page, heading);
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
          float  left,		/* I - Left margin */
          float  right,		/* I - Printable width */
          float  bottom,	/* I - Bottom margin */
          float  top,		/* I - Printable top */
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
  float		width,		/* Width of horizontal rule */
		height,		/* Height of rule */
		rgb[3];		/* RGB color of rule */


  DEBUG_printf(("parse_doc(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d, cpara = %08x\n",
                t, left, right, *x, *y, *page, cpara));

  if (cpara == NULL)
    para = htmlNewTree(NULL, MARKUP_P, NULL);
  else
    para = cpara;

  while (t != NULL)
  {
    if (((t->markup == MARKUP_H1 && OutputBook) ||
         (t->markup == MARKUP_FILE && !OutputBook)) && !title_page)
    {
      // New page on H1 in book mode or file in webpage mode...
      if (para->child != NULL)
      {
        parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
        htmlDeleteTree(para->child);
        para->child = para->last_child = NULL;
      }

      if ((chapter > 0 && OutputBook) ||
          ((*page > 1 || *y < top) && !OutputBook))
      {
        (*page) ++;
        if (PageDuplex && (*page & 1))
          (*page) ++;

        if (Verbosity)
          progress_show("Formatting page %d", *page);

        if (OutputBook)
          chapter_ends[chapter] = *page - 1;
      }

      if (OutputBook)
      {
        chapter ++;
	if (chapter >= MAX_CHAPTERS)
	{
	  progress_error("Too many chapters in document (%d > %d)!",
	                 chapter, MAX_CHAPTERS);
          chapter = MAX_CHAPTERS - 1;
	}
	else
          chapter_starts[chapter] = *page;

	if (chapter > TocDocCount)
	  TocDocCount = chapter;
      }

      *y         = top;
      *x         = left;
      *needspace = 0;
    }

    if ((name = htmlGetVariable(t, (uchar *)"ID")) != NULL)
    {
     /*
      * Add a link target using the ID=name variable...
      */

      add_link(name, *page, (int)(*y + 3 * t->height));
    }
    else if (t->markup == MARKUP_FILE)
    {
     /*
      * Add a file link...
      */

      add_link(htmlGetVariable(t, (uchar *)"FILENAME"), *page + OutputBook,
               (int)top);
    }

    if (chapter == 0 && !title_page)
    {
      if (t->child != NULL)
        parse_doc(t->child, left, right, bottom, top, x, y, page, para,
	          needspace);

      t = t->next;
      continue;
    }

    switch (t->markup)
    {
      case MARKUP_IMG :
          update_image_size(t);
      case MARKUP_NONE :
      case MARKUP_BR :
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

            copy_tree(temp, t->child);
          }
          break;

      case MARKUP_TABLE :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_table(t, left, right, bottom, top, x, y, page, *needspace);
          break;

      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_heading(t, left, right, bottom, top, x, y, page, 1);
	  *needspace = 1;
          break;

      case MARKUP_BLOCKQUOTE :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, left + 36, right - 36, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = left;
          *needspace = 1;
          break;

      case MARKUP_CENTER :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          *needspace = 1;

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = left;
          *needspace = 1;
          break;

      case MARKUP_P :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

	  *needspace = 1;

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = left;
          *needspace = 1;
          break;

      case MARKUP_PRE :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_pre(t, left, right, bottom, top, x, y, page, *needspace);

          *x         = left;
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
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          *x = left + 36.0f;

          parse_doc(t->child, left + 36, right, bottom, top, x, y, page, para,
	            needspace);
          break;

      case MARKUP_LI :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_list(t, left, right, bottom, top, x, y, page, *needspace);

          *x         = left;
          *needspace = 0;
          break;

      case MARKUP_DT :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          *x = left - 36.0f;

          parse_doc(t->child, left - 36.0f, right, bottom, top, x, y, page,
	            NULL, needspace);

          *x         = left;
          *needspace = 0;
          break;

      case MARKUP_DD :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          parse_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = left;
          *needspace = 0;
          break;

      case MARKUP_HR :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          if (htmlGetVariable(t, (uchar *)"BREAK") == NULL)
	  {
	   /*
	    * Generate a horizontal rule...
	    */

            if ((name = htmlGetVariable(t, (uchar *)"WIDTH")) == NULL)
	      width = right - left;
	    else
	    {
	      if (strchr((char *)name, '%') != NULL)
	        width = atoi((char *)name) * (right - left) / 100;
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
	          *x = left;
		  break;
	      case ALIGN_CENTER :
	          *x = left + (right - left - width) * 0.5f;
		  break;
	      case ALIGN_RIGHT :
	          *x = right - width;
		  break;
	    }

            if (*y < (bottom + height + _htmlSpacings[SIZE_P]))
	    {
	     /*
	      * Won't fit on this page...
	      */

              (*page) ++;
	      if (Verbosity)
	        progress_show("Formatting page %d", *page);
              *y = top;
            }

            (*y)   -= height + _htmlSpacings[SIZE_P];
            rgb[0] = t->red / 255.0f;
            rgb[1] = t->green / 255.0f;
            rgb[2] = t->blue / 255.0f;

            new_render(*page, RENDER_FBOX, *x, *y + _htmlSpacings[SIZE_P] * 0.5,
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
            *y = top;
	  }

          *x = left;
          break;

      case MARKUP_COMMENT :
          // Check comments for commands...
          parse_comment(t, left, right, bottom, top, x, y, page, para,
	                *needspace);
          break;

      case MARKUP_TITLE :
      case MARKUP_META :
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

    t = t->next;
  }

  if (para->child != NULL && cpara != para)
  {
    parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
    *needspace  = 1;
  }

  if (cpara != para)
    htmlDeleteTree(para);
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
  DEBUG_printf(("parse_heading(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (((t->markup - MARKUP_H1) < TocLevels || TocLevels == 0) && !title_page)
    current_heading = t->child;

  if (*y < (5 * _htmlSpacings[SIZE_P] + bottom))
  {
    (*page) ++;
    *y = top;
    if (Verbosity)
      progress_show("Formatting page %d", *page);
  }

  if (t->markup == MARKUP_H1 && !title_page)
    page_chapters[*page] = htmlGetText(current_heading);

  if ((page_headings[*page] == NULL || t->markup == MARKUP_H1) && !title_page)
    page_headings[*page] = htmlGetText(current_heading);

  if ((t->markup - MARKUP_H1) < TocLevels && !title_page)
  {
    DEBUG_printf(("H%d: heading_pages[%d] = %d\n", t->markup - MARKUP_H1 + 1,
                  num_headings, *page - 1));
    heading_pages[num_headings] = *page - chapter_starts[1] + 1;
    heading_tops[num_headings]  = (int)(*y + 2 * _htmlSpacings[SIZE_P]);
    num_headings ++;
  }

  parse_paragraph(t, left, right, bottom, top, x, y, page, needspace);

  if (t->halignment == ALIGN_RIGHT && t->markup == MARKUP_H1 && OutputBook &&
      !title_page)
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
		temp_y,
		temp_width,
		temp_height;
  float		format_width, image_y, image_left, image_right;
  render_t	*r;
  uchar		*align,
		*link;
  float		rgb[3];
  uchar		line[10240],
		*lineptr;
  tree_t	*linetype;
  float		linex,
		linewidth;
  int		firstline;


  if (*page > MAX_PAGES)
    return;

  DEBUG_printf(("parse_paragraph(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  flat        = flatten_tree(t->child);
  image_left  = left;
  image_right = right;
  image_y     = 0;

  if (flat == NULL)
    DEBUG_puts("parse_paragraph: flat == NULL!");

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
      if (strcasecmp((char *)align, "LEFT") == 0)
      {
        if (*y < (bottom + temp->height))
        {
	  (*page) ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

        new_render(*page, RENDER_IMAGE, image_left, *y - temp->height,
	           temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"SRC")));

        image_left += temp->width;
	temp_y     = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;

        free(temp);
        temp = prev;
      }
      else if (strcasecmp((char *)align, "RIGHT") == 0)
      {
        if (*y < (bottom + temp->height))
        {
	  (*page) ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }

        image_right -= temp->width;

        new_render(*page, RENDER_IMAGE, image_right, *y - temp->height,
                   temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"SRC")));

	temp_y = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

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

  while (flat != NULL)
  {
    start = flat;
    end   = flat;
    width = 0.0;

    while (flat != NULL)
    {
      temp_width = 0.0;
      temp       = flat;
      whitespace = 0;

      do
      {
        if ((temp_width == 0.0 || whitespace) &&
            temp->markup == MARKUP_NONE && temp->data[0] == ' ')
          temp_width -= _htmlWidths[temp->typeface][temp->style][' '] *
                        _htmlSizes[temp->size];

        if (temp->markup == MARKUP_NONE && temp->data[strlen((char *)temp->data) - 1] == ' ')
          whitespace = 1;
        else
          whitespace = 0;

        prev       = temp;
        temp       = temp->next;
        temp_width += prev->width;
      }
      while (temp != NULL && !whitespace && prev->markup != MARKUP_BR);

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

    for (height = 0.0, spacing = 0.0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;

      if (temp->height > height &&
          (temp->markup != MARKUP_IMG || temp->valignment == ALIGN_TOP))
        height = temp->height;
      else if (temp->markup == MARKUP_IMG && temp->valignment == ALIGN_MIDDLE &&
               (0.5f * temp->height) > height)
        height = 0.5f * temp->height;

      if (temp->markup != MARKUP_IMG)
        temp_height = temp->height * _htmlSpacings[0] / _htmlSizes[0];
      else
      {
        switch (temp->valignment)
	{
	  case ALIGN_TOP :
              temp_height = temp->height;
	      break;
	  case ALIGN_MIDDLE :
	      if ((0.5f * temp->height) > _htmlSizes[t->size])
	        temp_height = temp->height;
	      else
	        temp_height = 0.5f * temp->height + _htmlSizes[t->size];
              break;
	  case ALIGN_BOTTOM :
	      temp_height = temp->height + _htmlSizes[t->size];
              break;
	}
      }

      if (temp_height > spacing)
        spacing = temp_height;
    }

    if (firstline && end != NULL && *y < (bottom + 2.0f * height))
    {
      // Go to next page since only 1 line will fit on this one...
      (*page) ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    firstline = 0;

    if (height == 0.0)
      height = spacing;

    if (start != NULL && start->markup == MARKUP_NONE && start->data[0] == ' ')
    {
     /*
      * Remove leading space...
      */

      strcpy((char *)start->data, (char *)start->data + 1);
      temp_width = _htmlWidths[start->typeface][start->style][' '] *
                   _htmlSizes[start->size];
      start->width -= temp_width;
      width        -= temp_width;
    }

    if (prev != NULL && prev->markup == MARKUP_NONE &&
        prev->data[strlen((char *)prev->data) - 1] == ' ')
    {
     /*
      * Remove trailing space...
      */

      prev->data[strlen((char *)prev->data) - 1] = '\0';
      temp_width = _htmlWidths[prev->typeface][prev->style][' '] *
                   _htmlSizes[prev->size];
      prev->width -= temp_width;
      width       -= temp_width;
    }

    if (*y < (height + bottom))
    {
      (*page) ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    *y -= height;

    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / PagePrintLength));

    if (t->halignment == ALIGN_LEFT)
      *x = image_left;
    else if (t->halignment == ALIGN_CENTER)
      *x = image_left + 0.5f * (format_width - width);
    else
      *x = image_right - width;

    whitespace = 0;
    temp       = start;
    linetype   = NULL;
    linex      = 0.0;

    rgb[0] = temp->red / 255.0f;
    rgb[1] = temp->green / 255.0f;
    rgb[2] = temp->blue / 255.0f;

    while (temp != end)
    {
      if (temp->link != NULL)
      {
        link = htmlGetVariable(temp->link, (uchar *)"HREF");

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

	new_render(*page, RENDER_LINK, *x, *y, temp->width,
	           temp->height, link);

	if (PSLevel == 0)
	{
	  temp->red   = (int)(link_color[0] * 255.0);
	  temp->green = (int)(link_color[1] * 255.0);
	  temp->blue  = (int)(link_color[2] * 255.0);

          if (LinkStyle)
	    new_render(*page, RENDER_BOX, *x, *y - 1, temp->width, 0,
	               link_color);
	}
      }

     /*
      * See if we are doing a run of characters in a line and need to
      * output this run...
      */

      if (linetype != NULL &&
	  (fabs(linex - *x) > 0.1 ||
	   temp->markup != MARKUP_NONE ||
	   temp->typeface != linetype->typeface ||
	   temp->style != linetype->style ||
	   temp->size != linetype->size ||
	   temp->superscript != linetype->superscript ||
	   temp->red != linetype->red ||
	   temp->green != linetype->green ||
	   temp->blue != linetype->blue))
      {
        switch (linetype->valignment)
	{
	  case ALIGN_TOP :
	      offset = height - linetype->height;
	      break;
	  case ALIGN_MIDDLE :
	      offset = 0.5f * (height - linetype->height);
	      break;
	  case ALIGN_BOTTOM :
	      offset = 0.0f;
	}

        r = new_render(*page, RENDER_TEXT, linex - linewidth, *y + offset,
	               linewidth, linetype->height, line);
	r->data.text.typeface = linetype->typeface;
	r->data.text.style    = linetype->style;
	r->data.text.size     = _htmlSizes[linetype->size];
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

              add_link(link, *page, (int)(*y + 6 * height));
            }
            break;

        case MARKUP_NONE :
            if (temp->data == NULL)
              break;

	    switch (temp->valignment)
	    {
	      case ALIGN_TOP :
		  offset = height - temp->height;
		  break;
	      case ALIGN_MIDDLE :
		  offset = 0.5f * (height - temp->height);
		  break;
	      case ALIGN_BOTTOM :
		  offset = 0.0f;
	    }

            if (linetype == NULL)
            {
	      linetype  = temp;
	      lineptr   = line;
	      linex     = *x;
	      linewidth = 0.0;

	      rgb[0] = temp->red / 255.0f;
	      rgb[1] = temp->green / 255.0f;
	      rgb[2] = temp->blue / 255.0f;

	      linetype->valignment = ALIGN_MIDDLE;
	    }

	    if (temp->underline)
	      new_render(*page, RENDER_BOX, *x, *y + offset - 1, temp->width, 0, rgb);

	    if (temp->strikethrough)
	      new_render(*page, RENDER_BOX, *x, *y + offset + temp->height * 0.25f,
	                 temp->width, 0, rgb);

            if ((temp == start || whitespace) && temp->data[0] == ' ')
	    {
	      strcpy((char *)lineptr, (char *)temp->data + 1);
              temp->width -= get_width((uchar *)" ", temp->typeface,
	                               temp->style, temp->size);
            }
	    else
	      strcpy((char *)lineptr, (char *)temp->data);

            lineptr   += strlen((char *)lineptr);
            linewidth += temp->width;
	    linex     += temp->width;

            if (lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;

	case MARKUP_IMG :
	    switch (temp->valignment)
	    {
	      case ALIGN_TOP :
		  offset = height - temp->height;
		  break;
	      case ALIGN_MIDDLE :
		  offset = -0.5f * temp->height;
		  break;
	      case ALIGN_BOTTOM :
		  offset = -temp->height;
	    }

	    new_render(*page, RENDER_IMAGE, *x, *y + offset, temp->width, temp->height,
		       image_find((char *)htmlGetVariable(temp, (uchar *)"SRC")));
            whitespace = 0;
	    break;
      }

      *x += temp->width;
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
      switch (linetype->valignment)
      {
	case ALIGN_TOP :
	    offset = height - linetype->height;
	    break;
	case ALIGN_MIDDLE :
	    offset = 0.5f * (height - linetype->height);
	    break;
	case ALIGN_BOTTOM :
	    offset = 0.0f;
      }

      r = new_render(*page, RENDER_TEXT, linex - linewidth, *y + offset,
                     linewidth, linetype->height, line);
      r->data.text.typeface = linetype->typeface;
      r->data.text.style    = linetype->style;
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
  tree_t	*flat, *next;
  uchar		*link,
		line[10240],
		*lineptr,
		*dataptr;
  int		col;
  float		width,
		rgb[3];
  render_t	*r;


  REF(right);

  DEBUG_printf(("parse_pre(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  col  = 0;
  flat = flatten_tree(t->child);

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
    rgb[0] = flat->red / 255.0f;
    rgb[1] = flat->green / 255.0f;
    rgb[2] = flat->blue / 255.0f;

    if (col == 0)
    {
      if (*y < (_htmlSpacings[t->size] + bottom))
      {
        (*page) ++;
        *y = top;

	if (Verbosity)
	  progress_show("Formatting page %d", *page);
      }

      *x = left;
      *y -= _htmlSizes[t->size];

      if (Verbosity)
        progress_update(100 - (int)(100 * (*y) / PagePrintLength));
    }

    if (flat->link != NULL)
    {
      link = htmlGetVariable(flat->link, (uchar *)"HREF");

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

      new_render(*page, RENDER_LINK, *x, *y, flat->width,
	         flat->height, link);

      if (PSLevel == 0)
      {
        memcpy(rgb, link_color, sizeof(rgb));

	flat->red   = (int)(link_color[0] * 255.0);
	flat->green = (int)(link_color[1] * 255.0);
	flat->blue  = (int)(link_color[2] * 255.0);

        if (LinkStyle)
	  new_render(*page, RENDER_BOX, *x, *y - 1, flat->width, 0,
	             link_color);
      }
    }

    switch (flat->markup)
    {
      case MARKUP_A :
          if ((link = htmlGetVariable(flat, (uchar *)"NAME")) != NULL)
          {
           /*
            * Add a target link...
            */

            add_link(link, *page, (int)(*y + 6 * t->height));
          }
          break;

      case MARKUP_BR :
          col = 0;
          *y  -= _htmlSpacings[t->size] - _htmlSizes[t->size];
          break;

      case MARKUP_NONE :
          for (lineptr = line, dataptr = flat->data;
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

          width = get_width(line, flat->typeface, flat->style, flat->size);
          r = new_render(*page, RENDER_TEXT, *x, *y, width, 0, line);
          r->data.text.typeface = flat->typeface;
          r->data.text.style    = flat->style;
          r->data.text.size     = _htmlSizes[flat->size];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	  if (flat->underline)
	    new_render(*page, RENDER_BOX, *x, *y - 1, flat->width, 0, rgb);

	  if (flat->strikethrough)
	    new_render(*page, RENDER_BOX, *x, *y + flat->height * 0.25f,
	               flat->width, 0, rgb);

          *x += flat->width;

          if (*dataptr == '\n')
          {
            col = 0;
            *y  -= _htmlSpacings[t->size] - _htmlSizes[t->size];
          }
          break;

      case MARKUP_IMG :
	  new_render(*page, RENDER_IMAGE, *x, *y, flat->width, flat->height,
		     image_find((char *)htmlGetVariable(flat, (uchar *)"SRC")));

          *x += flat->width;
          col ++;
	  break;
    }

    next = flat->next;
    free(flat);
    flat = next;
  }

  *x = left;
}


//#undef DEBUG_printf
//#define DEBUG_printf(x) printf x
/*
 * 'parse_table()' - Parse a table and produce rendering output.
 */

static void
parse_table(tree_t *t,		/* I - Tree to parse */
            float  left,	/* I - Left margin */
            float  right,	/* I - Printable width */
            float  bottom,	/* I - Bottom margin */
            float  top,		/* I - Printable top */
            float  *x,		/* IO - X position */
            float  *y,		/* IO - Y position */
            int    *page,	/* IO - Page # */
            int    needspace)	/* I - Need whitespace? */
{
  int		col,
		row,
		tcol,
		colspan,
		num_cols,
		num_rows,
		alloc_rows,
		regular_cols,
		preformatted,
		tempspace,
		col_spans[MAX_COLUMNS],
		row_spans[MAX_COLUMNS];
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
		cellpadding,
		cellspacing,
		border,
		width,
		pref_width,
		table_width,
		span_width,
		regular_width,
		actual_width,
		row_y, temp_y;
  int		row_page, temp_page;
  uchar		*var;
  tree_t	*temprow,
		*tempcol,
		*flat,
		*next,
		***cells;
  int		do_valign;			// True if we should do vertical alignment of cells
  float		row_height,			// Total height of the row
		temp_height;			// Temporary holder
  int		cell_page[MAX_COLUMNS],		// Start page for cell
		cell_endpage[MAX_COLUMNS];	// End page for cell
  float		cell_y[MAX_COLUMNS],		// Row or each cell
		cell_endy[MAX_COLUMNS],		// Row or each cell
		cell_height[MAX_COLUMNS],	// Height of each cell in a row
		span_heights[MAX_COLUMNS];	// Height of spans
  render_t	*cell_start[MAX_COLUMNS];	// Start of the content for a cell in the row
  render_t	*cell_end[MAX_COLUMNS];		// End of the content for a cell in a row
  uchar		*bgcolor;
  float		rgb[3],
		bgrgb[3];


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("parse_table(t=%08x, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;   /* Empty table... */

  rgb[0] = t->red / 255.0f;
  rgb[1] = t->green / 255.0f;
  rgb[2] = t->blue / 255.0f;

 /*
  * Figure out the # of rows, columns, and the desired widths...
  */

  memset(col_spans, 0, sizeof(col_spans));
  memset(col_widths, 0, sizeof(col_widths));
  memset(col_swidths, 0, sizeof(col_swidths));
  memset(col_mins, 0, sizeof(col_mins));
  memset(col_smins, 0, sizeof(col_smins));
  memset(col_prefs, 0, sizeof(col_prefs));

  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_width = atof((char *)var) * (right - left) / 100.0f;
    else
      table_width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;
  }
  else
    table_width = right - left;

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
  }
  else
    border = 0.0f;

  memset(row_spans, 0, sizeof(row_spans));
  memset(span_heights, 0, sizeof(span_heights));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0;
       temprow != NULL;
       temprow = temprow->next)
    if (temprow->markup == MARKUP_CAPTION)
    {
      parse_paragraph(temprow, left, right, bottom, top, x, y, page, needspace);
      needspace = 1;
    }
    else if (temprow->markup == MARKUP_TR ||
             (temprow->markup == MARKUP_TBODY && temprow->child != NULL))
    {
      // Descend into table body as needed...
      if (temprow->markup == MARKUP_TBODY)
        temprow = temprow->child;

      // Allocate memory for the table as needed...
      if (num_rows >= alloc_rows)
      {
        alloc_rows += MAX_ROWS;

        if (alloc_rows == MAX_ROWS)
	  cells = (tree_t ***)malloc(sizeof(tree_t **) * alloc_rows);
	else
	  cells = (tree_t ***)realloc(cells, sizeof(tree_t **) * alloc_rows);

        if (cells == (tree_t ***)0)
	{
	  progress_error("Unable to allocate memory for table!");
	  return;
	}
      }	

      if ((cells[num_rows] = (tree_t **)calloc(sizeof(tree_t *), MAX_COLUMNS)) == NULL)
      {
	progress_error("Unable to allocate memory for table!");
	return;
      }

      for (col = 0; row_spans[col] && col < num_cols; col ++)
        cells[num_rows][col] = cells[num_rows - 1][col];

      for (tempcol = temprow->child;
           tempcol != NULL && col < MAX_COLUMNS;
           tempcol = tempcol->next)
        if (tempcol->markup == MARKUP_TD || tempcol->markup == MARKUP_TH)
        {
          if ((var = htmlGetVariable(tempcol, (uchar *)"COLSPAN")) != NULL)
            colspan = atoi((char *)var);
          else
            colspan = 1;

          if ((var = htmlGetVariable(tempcol, (uchar *)"ROWSPAN")) != NULL)
            row_spans[col] = atoi((char *)var);

          DEBUG_printf(("num_rows = %d, col = %d, colspan = %d (%s)\n",
	                num_rows, col, colspan, var));

          if ((var = htmlGetVariable(tempcol, (uchar *)"WIDTH")) != NULL &&
	      colspan == 1)
	  {
            if (var[strlen((char *)var) - 1] == '%')
              col_width = atof((char *)var) * table_width / 100.0f -
	                  2.0 * (cellpadding + cellspacing + border);
            else
              col_width = atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth;

            col_min  = col_width;
	    col_pref = col_width;
	  }
	  else
	  {
            flat       = flatten_tree(tempcol->child);
            width      = 0.0f;
	    pref_width = 0.0f;

	    col_width  = 0.0f;
	    col_pref   = 0.0f;
	    col_min    = 0.0f;

            while (flat != NULL)
            {
              if (flat->markup == MARKUP_BR ||
                  (flat->preformatted &&
                   flat->data != NULL &&
                   flat->data[strlen((char *)flat->data) - 1] == '\n'))
              {
		pref_width += flat->width + 1;

                if (pref_width > col_pref)
                  col_pref = pref_width;

                if (flat->preformatted && pref_width > col_width)
                  col_width = pref_width;

		pref_width = 0.0f;
              }
              else if (flat->data != NULL)
		pref_width += flat->width + 1;
	      else
		pref_width += flat->width;

              if (flat->markup == MARKUP_BR ||
                  (flat->preformatted &&
                   flat->data != NULL &&
                   flat->data[strlen((char *)flat->data) - 1] == '\n') ||
		  (!flat->preformatted &&
		   flat->data != NULL &&
		   (isspace(flat->data[0]) ||
		    isspace(flat->data[strlen((char *)flat->data) - 1]))))
              {
                width += flat->width + 1;

                if (width > col_min)
                  col_min = width;

                if (flat->preformatted && width > col_width)
                  col_width = width;

                width = 0.0f;
              }
              else if (flat->data != NULL)
                width += flat->width + 1;
	      else
		width += flat->width;

              if (flat->width > col_min)
	        col_min = flat->width;

              preformatted = flat->preformatted;

              next = flat->next;
              free(flat);
              flat = next;
            }

            if (width > col_min)
              col_min = width;

            if (pref_width > col_pref)
              col_pref = pref_width;

            if (preformatted && width > col_width)
              col_width = width;

            if (preformatted && width > col_width)
              col_width = width;

	    if (htmlGetVariable(tempcol, (uchar *)"NOWRAP") != NULL &&
	        col_pref > col_width)
	      col_width = col_pref;
	  }

          // Add widths to columns...
          if (colspan > 1)
          {
	    if (col_spans[col] > colspan)
	      col_spans[col] = colspan;

	    if (col_width > col_swidths[col])
	      col_swidths[col] = col_width;

	    if (col_min > col_smins[col])
	      col_smins[col] = col_pref;
          }
	  else
	  {
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

      num_rows ++;

      for (col = 0; col < num_cols; col ++)
        if (row_spans[col])
	  row_spans[col] --;
    }


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

    width += 2 * (border + cellpadding + cellspacing) * num_cols;

    if (width > (right - left))
      width = right - left;
  }

 /*
  * Compute the width of each column based on the printable width.
  */

  actual_width  = 2 * (border + cellpadding + cellspacing) * num_cols;
  regular_width = (width - actual_width) / num_cols;

 /*
  * The first pass just handles columns with a specified width...
  */

  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (col_widths[col] > 0.0f)
    {
      if (col_mins[col] > col_widths[col])
        col_widths[col] = col_mins[col];

      actual_width += col_widths[col];
    }
    else
      regular_cols ++;

 /*
  * Pass two uses the "preferred" width whenever possible, and the
  * minimum otherwise...
  */

  for (col = 0, pref_width = 0.0f; col < num_cols; col ++)
    if (col_widths[col] == 0.0f)
      pref_width += col_prefs[col];

  if (pref_width > 0.0f)
  {
    if ((regular_width = (width - actual_width) / pref_width) < 0.0f)
      regular_width = 0.0f;
    else if (regular_width > 1.0f)
      regular_width = 1.0f;

    for (col = 0; col < num_cols; col ++)
      if (col_widths[col] == 0.0f)
      {
	pref_width = col_prefs[col] * regular_width;
	if (pref_width < col_mins[col] &&
	    (actual_width + col_mins[col]) <= width)
          pref_width = col_mins[col];

	if ((actual_width + pref_width) > width)
	{
          if (col == (num_cols - 1) && (width - actual_width) >= col_mins[col])
	    col_widths[col] = width - actual_width;
	  else
	    col_widths[col] = col_mins[col];
	}
	else
          col_widths[col] = pref_width;

	actual_width += col_widths[col];
      }
  }

 /*
  * Pass three enforces any hard or minimum widths for COLSPAN'd
  * columns...
  */

  for (col = 0; col < num_cols; col ++)
    if (col_spans[col] > 1)
    {
      for (colspan = 0, span_width = 0.0f; colspan < col_spans[col]; colspan ++)
        span_width += col_widths[col + colspan];

      span_width += 2 * (border + cellpadding + cellspacing) *
                    (col_spans[col] - 1);
      pref_width = 0.0f;

      if (span_width < col_swidths[col])
        pref_width = col_swidths[col];
      if (span_width < col_smins[col] || pref_width < col_smins[col])
        pref_width = col_smins[col];

      if (pref_width > 0.0f)
      {
        // Expand cells proportionately...
	regular_width = pref_width / span_width;

	for (colspan = 0; colspan < col_spans[col]; colspan ++)
	{
	  actual_width -= col_widths[col + colspan];
	  col_widths[col + colspan] *= regular_width;
	  actual_width += col_widths[col + colspan];
	}
      }
    }

 /*
  * The final pass divides up the remaining space amongst the columns...
  */

  if (width > actual_width)
  {
    regular_width = (width - actual_width) / num_cols;

    for (col = 0; col < num_cols; col ++)
      col_widths[col] += regular_width;
  }

  switch (t->halignment)
  {
    case ALIGN_LEFT :
        *x = left + border + cellpadding;
        break;
    case ALIGN_CENTER :
        *x = left + 0.5f * (right - left - width) + border + cellpadding;
        break;
    case ALIGN_RIGHT :
        *x = right - width + border + cellpadding;
        break;
  }

  for (col = 0; col < num_cols; col ++)
  {
    col_lefts[col]  = *x;
    col_rights[col] = *x + col_widths[col];
    *x = col_rights[col] + 2 * (border + cellpadding) + cellspacing;

    DEBUG_printf(("left[%d] = %.1f, right[%d] = %.1f\n", col, col_lefts[col], col,
                  col_rights[col]));
  }

 /*
  * Now render the whole table...
  */

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  memset(row_spans, 0, sizeof(row_spans));
  memset(cell_start, 0, sizeof(cell_start));
  memset(cell_end, 0, sizeof(cell_end));
  memset(cell_height, 0, sizeof(cell_height));

  for (row = 0; row < num_rows; row ++)
  {
    if (cells[row][0] != NULL)
    {
     /*
      * Do page comments...
      */

      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->markup == MARKUP_COMMENT)
        parse_comment(cells[row][0]->parent->prev,
                      left, right, bottom + border + cellpadding,
                      top - border - cellpadding, x, y, page, NULL, 0);

     /*
      * Get height...
      */

      if ((var = htmlGetVariable(t, (uchar *)"HEIGHT")) == NULL)
	if ((var = htmlGetVariable(cells[row][0]->parent,
                        	   (uchar *)"HEIGHT")) == NULL)
	  for (col = 0; col < num_cols; col ++)
	    if ((var = htmlGetVariable(cells[row][col],
                                       (uchar *)"HEIGHT")) != NULL)
	      break;
    }

    if (cells[row][0] != NULL && var != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (var[strlen((char *)var) - 1] == '%')
	temp_height = atof((char *)var) * 0.01f * PagePrintLength;
      else
        temp_height = atof((char *)var) * PagePrintWidth / _htmlBrowserWidth;

      if (htmlGetVariable(t, (uchar *)"HEIGHT") != NULL)
        temp_height /= num_rows;

      temp_height -= 2 * (border + cellpadding);
    }
    else
      temp_height = _htmlSpacings[SIZE_P];

    if (*y < (bottom + 2 * (border + cellpadding) + temp_height) &&
        temp_height < (top - bottom - 2 * (border + cellpadding)))
    {
      *y = top;
      (*page) ++;

      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }

    do_valign  = 1;
    row_y      = *y - (border + cellpadding);
    row_page   = *page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row = %d, row_y = %.1f, *y = %.1f\n", row, row_y, *y));

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      if (row_spans[col] == 0)
      {
        if ((var = htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN")) != NULL)
          row_spans[col] = atoi((char *)var);

	span_heights[col] = 0.0f;
      }

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      DEBUG_printf(("    col = %d, colspan = %d, left = %.1f, right = %.1f, cell = %p\n",
                    col, colspan, col_lefts[col], col_rights[col + colspan], cells[row][col]));

      *x        = col_lefts[col];
      temp_y    = *y - (border + cellpadding);
      temp_page = *page;
      tempspace = 0;

      if (cells[row][col] != NULL && cells[row][col]->child != NULL &&
          (row == 0 || cells[row][col] != cells[row - 1][col]))
      {
	cell_start[col] = endpages[*page];
	cell_page[col]  = temp_page;
	cell_y[col]     = temp_y;

	parse_doc(cells[row][col]->child,
                  col_lefts[col], col_rights[col + colspan],
                  bottom + border + cellpadding,
                  top - border - cellpadding,
                  x, &temp_y, &temp_page, NULL, &tempspace);

        cell_endpage[col] = temp_page;
        cell_endy[col]    = temp_y;
        cell_height[col]  = *y - (border + cellpadding) - temp_y;
        cell_end[col]     = endpages[*page];

        if (cell_start[col] == NULL)
	  cell_start[col] = pages[*page];

        DEBUG_printf(("row = %d, col = %d, y = %.1f, cell_y = %.1f, cell_height = %.1f\n",
	              row, col, *y - (border + cellpadding), temp_y, cell_height[col]));
      }

      if (row_spans[col] == 0 &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > row_height)
        row_height = cell_height[col];

      if (row_spans[col] < 2)
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
    }

    DEBUG_printf(("row = %d, row_y = %.1f, row_height = %.1f\n", row, row_y, row_height));

    for (col = 0; col < num_cols; col += colspan)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;

      if (row_spans[col])
        span_heights[col] += row_height;

      DEBUG_printf(("col = %d, row_spans = %d, span_heights = %.1f, cell_height = %.1f\n",
                    col, row_spans[col], span_heights[col], cell_height[col]));

      if (row_spans[col] == 1 &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > span_heights[col])
      {
        temp_height = cell_height[col] - span_heights[col];
	row_height  += temp_height;
	row_y       -= temp_height;
	DEBUG_printf(("Adjusting row-span height by %.1f, row_height = %.1f, row_y = %.1f\n",
	              temp_height, row_height, row_y));

	for (tcol = 0; tcol < num_cols; tcol ++)
	  if (row_spans[tcol])
	  {
	    span_heights[tcol] += temp_height;
	    DEBUG_printf(("col = %d, span_heights = %.1f\n", tcol, span_heights[tcol]));
	  }
      }
    }

    DEBUG_printf(("AFTER row = %d, row_y = %.1f, row_height = %.1f, *y = %.1f, do_valign = %d\n",
                  row, row_y, row_height, *y, do_valign));

   /*
    * Do the vertical alignment
    */

    if (do_valign)
    {
      if (var != NULL)
      {
        // Hardcode the row height...
        if (var[strlen((char *)var) - 1] == '%')
	  temp_height = atof((char *)var) * 0.01f * PagePrintLength;
	else
          temp_height = atof((char *)var) * PagePrintWidth / _htmlBrowserWidth;

	if (htmlGetVariable(t, (uchar *)"HEIGHT") != NULL)
          temp_height /= num_rows;

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

        if (cell_start[col] == NULL || cell_start[col] == cell_end[col] ||
	    row_spans[col] > 1 || cells[row][col] == NULL)
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

	DEBUG_printf(("row = %d, col = %d, cell_height = %.1f, span_heights = %.1f, delta_y = %.1f\n",
	              row, col, cell_height[col], span_heights[col], delta_y));

        if (delta_y > 0.0f)
	{
          for (p = cell_start[col]->next; p != NULL; p = p->next)
	  {
            p->y -= delta_y;
            if (p == cell_end[col])
	      break;
          }
        }
      }
    }

    row_y -= 2 * (border + cellpadding);

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      if (row_spans[col] > 0)
      {
        DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
	              col, row_spans[col], row_spans[col] - 1));
        row_spans[col] --;
      }

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      width = col_rights[col + colspan] - col_lefts[col] +
              2 * cellpadding + 2 * border;

      if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
          ((row + 1) < num_rows && cells[row][col] == cells[row + 1][col]))
        continue;

      if ((bgcolor = htmlGetVariable(cells[row][col], (uchar *)"BGCOLOR")) == NULL)
        if ((bgcolor = htmlGetVariable(cells[row][col]->parent, (uchar *)"BGCOLOR")) == NULL)
	  bgcolor = htmlGetVariable(t, (uchar *)"BGCOLOR");

      if (bgcolor != NULL)
        get_color(bgcolor, bgrgb);

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

          new_render(*page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                     bottom, 0.0f,
                     cell_y[col] - bottom, rgb);
          new_render(*page, RENDER_BOX, col_rights[col] + cellpadding + border,
                     bottom, 0.0f,
                     cell_y[col] - bottom, rgb);
          new_render(*page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                     cell_y[col], width, 0.0f, rgb);
        }

        if (bgcolor != NULL)
          new_render(*page, RENDER_FBOX, col_lefts[col] - cellpadding - border,
                     bottom, width, cell_y[col] - bottom, bgrgb, 1);

        for (temp_page = cell_page[col] + 1; temp_page != cell_endpage[col]; temp_page ++)
	{
	 /*
	  * |   |   |   |
	  * |   |   |   |
	  */

	  if (border > 0)
	  {
            new_render(temp_page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                       bottom, 0.0f, top - bottom, rgb);
            new_render(temp_page, RENDER_BOX, col_rights[col] + cellpadding + border,
                       bottom, 0.0f, top - bottom, rgb);
          }

	  if (bgcolor != NULL)
            new_render(temp_page, RENDER_FBOX, col_lefts[col] - cellpadding - border,
                       bottom, width, top - bottom, bgrgb, 1);
        }

        if (border > 0)
	{
	 /*
	  * |   |   |   |
	  * +---+---+---+
	  */

          new_render(row_page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                     row_y, 0.0f, top - row_y, rgb);
          new_render(row_page, RENDER_BOX, col_rights[col] + cellpadding + border,
                     row_y, 0.0f, top - row_y, rgb);
          new_render(row_page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                     row_y, width, 0.0f, rgb);
        }

        if (bgcolor != NULL)
          new_render(row_page, RENDER_FBOX, col_lefts[col] - cellpadding - border,
                     row_y, width, top - row_y, bgrgb, 1);
      }
      else
      {
        if (border > 0)
          new_render(*page, RENDER_BOX, col_lefts[col] - cellpadding - border,
                     row_y, width, cell_y[col] - row_y + border + cellpadding, rgb);

        if (bgcolor != NULL)
          new_render(*page, RENDER_FBOX, col_lefts[col] - cellpadding - border,
                     row_y, width, cell_y[col] - row_y + border + cellpadding, bgrgb, 1);
      }
    }

    *page = row_page;
    *y    = row_y - cellspacing;
  }

  *x = left;

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
#undef DEBUG_printf
#define DEBUG_printf(x)


/*
 * 'parse_list()' - Parse a list entry and produce rendering output.
 */

static void
parse_list(tree_t *t,		/* I - Tree to parse */
           float  left,		/* I - Left margin */
           float  right,	/* I - Printable width */
           float  bottom,	/* I - Bottom margin */
           float  top,		/* I - Printable top */
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


  DEBUG_printf(("parse_list(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (needspace && *y < top)
  {
    *y        -= _htmlSpacings[t->size];
    needspace = 0;
  }

  oldy    = *y;
  oldpage = *page;
  r       = endpages[*page];
  tempx   = *x;

  if (t->indent == 0)
  {
    // Adjust left margin when no UL/OL/DL is being used...
    left  += _htmlSizes[t->size];
    tempx += _htmlSizes[t->size];
  }

  parse_doc(t->child, left, right, bottom, top, &tempx, y, page, NULL,
            &needspace);

  // Handle when paragraph wrapped to new page...
  if (*page != oldpage)
  {
    // First see if anything was added to the old page...
    if ((r != NULL && r->next == NULL) || endpages[oldpage] == NULL)
    {
      // No, put the symbol on the next page...
      oldpage = *page;
      oldy    = top;
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
        strcpy((char *)number, format_number(list_values[t->indent],
	                                     list_types[t->indent]));
        strcat((char *)number, ". ");
        typeface = t->typeface;
        break;

    default :
        sprintf((char *)number, "%c ", list_types[t->indent]);
        typeface = TYPE_SYMBOL;
        break;
  }

  width = get_width(number, typeface, t->style, t->size);

  r = new_render(oldpage, RENDER_TEXT, left - width, oldy - _htmlSizes[t->size],
                 width, _htmlSpacings[t->size], number);
  r->data.text.typeface = typeface;
  r->data.text.style    = t->style;
  r->data.text.size     = _htmlSizes[t->size];
  r->data.text.rgb[0]   = t->red / 255.0f;
  r->data.text.rgb[1]   = t->green / 255.0f;
  r->data.text.rgb[2]   = t->blue / 255.0f;

  list_values[t->indent] ++;
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

static void
parse_comment(tree_t *t,	/* I - Tree to parse */
              float  left,	/* I - Left margin */
              float  right,	/* I - Printable width */
              float  bottom,	/* I - Bottom margin */
              float  top,	/* I - Printable top */
              float  *x,	/* IO - X position */
              float  *y,	/* IO - Y position */
              int    *page,	/* IO - Page # */
	      tree_t *para,	/* I - Current paragraph */
	      int    needspace)	/* I - Need whitespace? */
{
  const char	*comment;	/* Comment text */


  if (t->data == NULL)
    return;

  for (comment = (const char *)t->data;
       *comment && isspace(*comment);
       comment ++);		// Skip leading whitespace...

  if (strncasecmp(comment, "PAGE BREAK", 10) == 0 ||
      strncasecmp(comment, "NEW PAGE", 8) == 0)
  {
   /*
    * <!-- PAGE BREAK --> and <!-- NEW PAGE --> generate a page
    * break...
    */

    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      htmlDeleteTree(para->child);
      para->child = para->last_child = NULL;
    }

    (*page) ++;
    if (Verbosity)
      progress_show("Formatting page %d", *page);
    *x = left;
    *y = top;
  }
  else if (strncasecmp(comment, "NEW SHEET", 9) == 0)
  {
   /*
    * <!-- NEW SHEET --> generate a page break to a new sheet...
    */

    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      htmlDeleteTree(para->child);
      para->child = para->last_child = NULL;
    }

    (*page) ++;
    if (PageDuplex && ((*page) & 1))
      (*page) ++;

    if (Verbosity)
      progress_show("Formatting page %d", *page);
    *x = left;
    *y = top;
  }
  else if (strncasecmp(comment, "HALF PAGE", 9) == 0)
  {
   /*
    * <!-- HALF PAGE --> Go to the next half page.  If in the
    * top half of a page, go to the bottom half.  If in the
    * bottom half, go to the next page.
    */
    float halfway;


    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      htmlDeleteTree(para->child);
      para->child = para->last_child = NULL;
    }

    halfway = 0.5f * (top + bottom);

    if (*y <= halfway)
    {
      (*page) ++;
      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = left;
      *y = top;
    }
    else
    {
      *x = left;
      *y = halfway;
    }
  }
  else if (strncasecmp(comment, "NEED ", 5) == 0)
  {
   /*
    * <!-- NEED amount --> generate a page break if there isn't
    * enough remaining space...
    */

    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      htmlDeleteTree(para->child);
      para->child = para->last_child = NULL;
    }

    if ((*y - get_measurement(comment + 5)) < bottom)
    {
      (*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *y = top;
    }

    *x = left;
  }
}


/*
 * 'real_prev()' - Return the previous non-link markup in the tree.
 */

static tree_t *		/* O - Pointer to previous markup */
real_prev(tree_t *t)	/* I - Current markup */
{
  if (t == NULL)
    return (NULL);

  if (t->prev != NULL &&
      (t->prev->markup == MARKUP_A || t->prev->markup == MARKUP_COMMENT))
    t = t->prev;

  if (t->prev != NULL)
    return (t->prev);

  t = t->parent;
  if (t == NULL)
    return (NULL);

  if (t->markup != MARKUP_A && t->markup != MARKUP_EMBED &&
      t->markup != MARKUP_COMMENT)
    return (t);
  else
    return (real_prev(t));
}


/*
 * 'real_next()' - Return the next non-link markup in the tree.
 */

static tree_t *		/* O - Pointer to next markup */
real_next(tree_t *t)	/* I - Current markup */
{
  if (t == NULL)
    return (NULL);

  if (t->next != NULL &&
      (t->next->markup == MARKUP_A || t->next->markup == MARKUP_COMMENT))
    t = t->next;

  if (t->next != NULL)
    return (t->next);

  return (real_next(t->parent));
}


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
    get_color((uchar *)BodyColor, background_color);
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
        get_color(var, background_color);
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
write_background(FILE *out)	/* I - File to write to */
{
  float	x, y;
  float	width, height;
  int	page_width, page_length;


  if (Landscape)
  {
    page_length = PageWidth;
    page_width  = PageLength;
  }
  else
  {
    page_width  = PageWidth;
    page_length = PageLength;
  }

  if (background_image != NULL)
  {
    width  = background_image->width * PagePrintWidth / _htmlBrowserWidth;
    height = background_image->height * PagePrintWidth / _htmlBrowserWidth;

    switch (PSLevel)
    {
      case 0 :
          for (x = 0.0; x < page_width; x += width)
            for (y = 0.0; y < page_length; y += height)
            {
  	      flate_printf(out, "q %.1f 0 0 %.1f %.1f %.1f cm", width, height, x, y);
              flate_puts("/BG Do\n", out);
	      flate_puts("Q\n", out);
            }
	  break;

      case 1 :
      case 2 :
          fprintf(out, "0 %.1f %d{/y exch def 0 %.1f %d{/x exch def\n",
	          height, page_length + (int)height - 1, width, page_width);
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
  else if (background_color[0] != 1.0 ||
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
}


/*
 * 'new_render()' - Allocate memory for a new rendering structure.
 */

static render_t *		/* O - New render structure */
new_render(int   page,		/* I - Page number (0-n) */
           int   type,		/* I - Type of render primitive */
           float x,		/* I - Horizontal position */
           float y,		/* I - Vertical position */
           float width,		/* I - Width */
           float height,	/* I - Height */
           void  *data,		/* I - Data */
	   int   insert)	/* I - Insert instead of append? */
{
  render_t		*r;	/* New render primitive */
  static render_t	dummy;	/* Dummy var for errors... */


  if (page < 0 || page >= MAX_PAGES)
  {
    progress_error("Page number (%d) out of range (1...%d)\n", page + 1, MAX_PAGES);
    memset(&dummy, 0, sizeof(dummy));
    return (&dummy);
  }

  if (type == RENDER_IMAGE || data == NULL)
    r = (render_t *)calloc(sizeof(render_t), 1);
  else
    r = (render_t *)calloc(sizeof(render_t) + strlen((char *)data), 1);

  if (r == NULL)
  {
    progress_error("Unable to allocate memory on page %s\n", page + 1);
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
        strcpy((char *)r->data.text.buffer, (char *)data);
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
    case RENDER_FBOX :
        memcpy(r->data.fbox, data, sizeof(r->data.fbox));
        break;
    case RENDER_LINK :
        if (data == NULL)
        {
          free(r);
          return (NULL);
        }
        strcpy((char *)r->data.link, (char *)data);
        break;
  }

  if (insert)
  {
    r->next     = pages[page];
    pages[page] = r;
    if (r->next == NULL)
      endpages[page] = r;
  }
  else
  {
    if (endpages[page] != NULL)
      endpages[page]->next = r;
    else
      pages[page] = r;

    r->next        = NULL;
    endpages[page] = r;
  }

  if (page >= num_pages)
    num_pages = page + 1;

  return (r);
}


/*
 * 'add_link()' - Add a named link...
 */

static void
add_link(uchar *name,	/* I - Name of link */
         int   page,	/* I - Page # */
         int   top)	/* I - Y position */
{
  link_t	*temp;	/* New name */


  if (name == NULL)
    return;

  if ((temp = find_link(name)) != NULL)
  {
    temp->page = page - 1;
    temp->top  = top;
  }
  else if (num_links < MAX_LINKS)
  {
    temp = links + num_links;
    num_links ++;

    strncpy((char *)temp->name, (char *)name, sizeof(temp->name) - 1);
    temp->name[sizeof(temp->name) - 1] = '\0';
    temp->page = page - 1;
    temp->top  = top;

    if (num_links > 1)
      qsort(links, num_links, sizeof(link_t),
            (int (*)(const void *, const void *))compare_links);
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

  strncpy((char *)key.name, (char *)name, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';
  match = (link_t *)bsearch(&key, links, num_links, sizeof(link_t),
                            (int (*)(const void *, const void *))compare_links);

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


/*
 * 'copy_tree()' - Copy a markup tree...
 */

static void
copy_tree(tree_t *parent,	/* I - Source tree */
          tree_t *t)		/* I - Destination tree */
{
  int		i;		/* I - Looping var */
  tree_t	*temp;		/* I - New tree entry */
  var_t		*var;		/* I - Current markup variable */


  while (t != NULL)
  {
    if ((temp = htmlAddTree(parent, t->markup, t->data)) != NULL)
    {
      temp->link          = t->link;
      temp->typeface      = t->typeface;
      temp->style         = t->style;
      temp->size          = t->size;
      temp->halignment    = t->halignment;
      temp->valignment    = t->valignment;
      temp->red           = t->red;
      temp->green         = t->green;
      temp->blue          = t->blue;
      temp->underline     = t->underline;
      temp->strikethrough = t->strikethrough;
      temp->superscript   = t->superscript;
      temp->subscript     = t->subscript;
      for (i = 0, var = t->vars; i < t->nvars; i ++, var ++)
        htmlSetVariable(temp, var->name, var->value);

      copy_tree(temp, t->child);
    }

    t = t->next;
  }
}


/*
 * 'flatten_tree()' - Flatten an HTML tree to only include the text, image,
 *                    link, and break markups.
 */

static tree_t *			/* O - Flattened markup tree */
flatten_tree(tree_t *t,		/* I - Markup tree to flatten */
             float  padding)	/* I - Padding for table cells */
{
  tree_t	*temp,		/* New tree node */
		*flat;		/* Flattened tree */
  float		newpadding;	/* New padding for table cells */
  uchar		*var;		/* Attribute value */


  flat = NULL;

  while (t != NULL)
  {
    newpadding = padding;

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

      case MARKUP_TD :
      case MARKUP_TH :
	  temp = (tree_t *)calloc(sizeof(tree_t), 1);
	  temp->markup = MARKUP_SPACER;
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;
	  temp->width  = padding;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;
          break;

      case MARKUP_TABLE :
	  temp = (tree_t *)calloc(sizeof(tree_t), 1);
	  temp->markup = MARKUP_SPACER;
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;

	  if ((var = htmlGetVariable(t, (uchar *)"CELLPADDING")) != NULL)
	    newpadding = 2.0f * atof((char *)var);
	  else
	    newpadding = 2.0f;

	  if ((var = htmlGetVariable(t, (uchar *)"CELLSPACING")) != NULL)
	    newpadding += 2.0f * atof((char *)var);

	  if ((var = htmlGetVariable(t, (uchar *)"BORDER")) != NULL)
	  {
	    if (!isdigit(var[0]))
	      temp->width = 2.0f;
	    else
	      temp->width = 2.0f * atof((char *)var);
	  }

	  if (flat != NULL)
            flat->next = temp;
          flat = temp;
	  break;

      case MARKUP_P :
      case MARKUP_PRE :
      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
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
    }

    if (t->child != NULL)
    {
      temp = flatten_tree(t->child, newpadding);

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

  img = image_find((char *)htmlGetVariable(t, (uchar *)"SRC"));

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


  DEBUG_printf(("get_width(\"%s\", %d, %d, %d)\n", s == NULL ? "(null)" : s,
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
      sprintf(filename, "%s/cover.ps", OutputPath);
    else if (chapter == 0)
      sprintf(filename, "%s/contents.ps", OutputPath);
    else
      sprintf(filename, "%s/doc%d.ps", OutputPath, chapter);

    return (fopen(filename, "wb"));
  }
  else if (OutputFiles)
  {
    sprintf(filename, "%s/doc.pdf", OutputPath);

    return (fopen(filename, "wb"));
  }
  else if (OutputPath[0] != '\0')
    return (fopen(OutputPath, "wb"));
  else
  {
    if (PSLevel == 0)
    {
#if defined(WIN32) || defined(__EMX__)
      if (getenv("TMP") != NULL)
        sprintf(stdout_filename, "%s/XXXXXX", getenv("TMP"));
      else
        strcpy(stdout_filename, "C:/XXXXXX");
#else
      if (getenv("TMP") != NULL)
        sprintf(stdout_filename, "%s/XXXXXX", getenv("TMP"));
      else
        strcpy(stdout_filename, "/var/tmp/XXXXXX");
#endif // WIN32 || __EMX__

      return (fopen(stdout_filename, "wb"));
    }
    else
      return (stdout);
  }
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

  if (PSLevel > 0)
    fprintf(out, "%.2f %.2f %.2f C ", rgb[0], rgb[1], rgb[2]);
  else
    flate_printf(out, "%.2f %.2f %.2f rg ", rgb[0], rgb[1], rgb[2]);
}


/*
 * 'set_font()' - Set the current text font.
 */

static void
set_font(FILE  *out,		/* I - File to write to */
         int   typeface,	/* I - Typeface code */
         int   style,		/* I - Style code */
         float size)		/* I - Size */
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

  render_typeface = typeface;
  render_style    = style;
  render_size     = size;

  if (PSLevel > 0)
    fprintf(out, "%s/F%x SF ", sizes, typeface * 4 + style);
  else
    flate_printf(out, "/F%x %s Tf ", typeface * 4 + style, sizes);
}


/*
 * 'set_pos()' - Set the current text position.
 */

static void
set_pos(FILE  *out,	/* I - File to write to */
        float x,	/* I - X position */
        float y)	/* I - Y position */
{
  char	xs[255],	/* Formatted string for X... */
	ys[255],	/* Formatted string for Y... */
	*s;		/* Pointer to end of string */


  if (fabs(render_x - x) < 0.1 && fabs(render_y - y) < 0.1)
    return;

 /*
  * Format X and Y...
  */

  if (PSLevel > 0 || render_x == -1.0)
  {
    sprintf(xs, "%.1f", x);
    sprintf(ys, "%.1f", y);
  }
  else
  {
    sprintf(xs, "%.1f", x - render_startx);
    sprintf(ys, "%.1f", y - render_y);
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
ps_hex(FILE  *out,	/* I - File to print to */
       uchar *data,	/* I - Data to print */
       int   length)	/* I - Number of bytes to print */
{
  int		col;
  static char	*hex = "0123456789ABCDEF";


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


/*
 * 'ps_ascii85()' - Print binary data as a series of base-85 numbers.
 */

static void
ps_ascii85(FILE  *out,		/* I - File to print to */
	   uchar *data,		/* I - Data to print */
	   int   length)	/* I - Number of bytes to print */
{
  int		col;		/* Column */
  unsigned	b;
  uchar		c[5];
  uchar		temp[4];


  col = 0;

  while (length > 3)
  {
    b = (((((data[0] << 8) | data[1]) << 8) | data[2]) << 8) | data[3];

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

      fwrite(c, 5, 1, out);
      col += 5;
    }

    data += 4;
    length -= 4;

    if (col >= 80)
    {
      col = 0;
      putc('\n', out);
    }
  }

  if (length > 0)
  {
    memcpy(temp, data, length);
    memset(temp + length, 0, 4 - length);

    b = (((((temp[0] << 8) | temp[1]) << 8) | temp[2]) << 8) | temp[3];

    c[4] = (b % 85) + '!';
    b /= 85;
    c[3] = (b % 85) + '!';
    b /= 85;
    c[2] = (b % 85) + '!';
    b /= 85;
    c[1] = (b % 85) + '!';
    b /= 85;
    c[0] = b + '!';

    fwrite(c, length + 1, 1, out);
  }
}


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
jpg_init(j_compress_ptr cinfo)	/* I - Compressor info */
{
  (void)cinfo;

  jpg_dest.next_output_byte = jpg_buf;
  jpg_dest.free_in_buffer   = sizeof(jpg_buf);
}


/*
 * 'jpg_empty()' - Empty the JPEG output buffer.
 */

static boolean			/* O - True if buffer written OK */
jpg_empty(j_compress_ptr cinfo)	/* I - Compressor info */
{
  (void)cinfo;

  if (PSLevel > 0)
    ps_ascii85(jpg_file, jpg_buf, sizeof(jpg_buf));
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
jpg_term(j_compress_ptr cinfo)	/* I - Compressor info */
{
  int nbytes;			/* Number of bytes to write */


  (void)cinfo;

  nbytes = sizeof(jpg_buf) - jpg_dest.free_in_buffer;

  if (PSLevel > 0)
    ps_ascii85(jpg_file, jpg_buf, nbytes);
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
  jpeg_set_linear_quality(cinfo, OutputJPEG, TRUE);

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

static int			/* O - -1 if rgb1<rgb2, etc. */
compare_rgb(uchar *rgb1,	/* I - First color */
            uchar *rgb2)	/* I - Second color */
{
  if (rgb1[0] < rgb2[0])
    return (-1);
  else if (rgb1[0] > rgb2[0])
    return (1);
  else if (rgb1[1] < rgb2[1])
    return (-1);
  else if (rgb1[1] > rgb2[1])
    return (1);
  else if (rgb1[2] < rgb2[2])
    return (-1);
  else if (rgb1[2] > rgb2[2])
    return (1);
  else
    return (0);
}


/*
 * 'write_image()' - Write an image to the given output file...
 */

static void
write_image(FILE     *out,	/* I - Output file */
            render_t *r)	/* I - Image to write */
{
  int		i, j, k, m,	/* Looping vars */
		ncolors;	/* Number of colors */
  uchar		*pixel,		/* Current pixel */
		*indices,	/* New indexed pixel array */
		*indptr;	/* Current index */
  int		indwidth,	/* Width of indexed line */
		indbits;	/* Bits per index */
  uchar		colors[256][3],	/* Colormap values */
		grays[256],	/* Grayscale usage */
		*match;		/* Matching color value */
  image_t 	*img;		/* Image */
  struct jpeg_compress_struct cinfo;	/* JPEG compressor */


 /*
  * See if we can optimize the image as indexed without color loss...
  */

  img     = r->data.image;
  ncolors = 0;

  DEBUG_printf(("img->filename = %s\n", img->filename));
  DEBUG_printf(("img->width = %d, ->height = %d, ->depth = %d\n", img->width,
                img->height, img->depth));

  if (PSLevel != 1 && PDFVersion >= 1.2)
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
	  grays[*pixel] = 1;
	  ncolors ++;
	}

      if (ncolors <= 16)
      {
	for (i = 0, j = 0; i < 256; i ++)
	  if (grays[i])
	  {
	    colors[j][0] = i;
	    colors[j][1] = i;
	    colors[j][2] = i;
	    grays[i]   = j;
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

      for (i = img->width * img->height, pixel = img->pixels;
	   i > 0;
	   i --, pixel += 3)
      {
        if (ncolors > 0)
          match = (uchar *)bsearch(pixel, colors[0], ncolors, 3,
                                   (int (*)(const void *, const void *))compare_rgb);
        else
          match = NULL;

        if (match == NULL)
        {
          if (ncolors >= 256)
            break;

          colors[ncolors][0] = pixel[0];
          colors[ncolors][1] = pixel[1];
          colors[ncolors][2] = pixel[2];
          ncolors ++;

          if (ncolors > 1)
            qsort(colors[0], ncolors, 3,
                  (int (*)(const void *, const void *))compare_rgb);
        }
      }

      if (i > 0)
        ncolors = 0;
    }
  }

  if (ncolors > 0)
  {
    if (ncolors <= 2)
      indbits = 1;
    else if (ncolors <= 4)
      indbits = 2;
    else if (ncolors <= 16)
      indbits = 4;
    else
      indbits = 8;

    indwidth = (img->width * indbits + 7) / 8;
    indices  = (uchar *)malloc(indwidth * img->height);

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
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 7; j > 0; j --, k = (k + 7) & 7, pixel += 3)
	      {
        	match = (uchar *)bsearch(pixel, colors[0], ncolors, 3,
                            	         (int (*)(const void *, const void *))compare_rgb);
	        m = (match - colors[0]) / 3;

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
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0; j > 0; j --, k = (k + 1) & 3, pixel += 3)
	      {
        	match = (uchar *)bsearch(pixel, colors[0], ncolors, 3,
                           	         (int (*)(const void *, const void *))compare_rgb);
	        m = (match - colors[0]) / 3;

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
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width, k = 0; j > 0; j --, k ^= 1, pixel += 3)
	      {
        	match = (uchar *)bsearch(pixel, colors[0], ncolors, 3,
                        	         (int (*)(const void *, const void *))compare_rgb);
	        m = (match - colors[0]) / 3;

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
	    for (i = img->height, pixel = img->pixels, indptr = indices;
		 i > 0;
		 i --)
	    {
	      for (j = img->width; j > 0; j --, pixel += 3, indptr ++)
	      {
        	match = (uchar *)bsearch(pixel, colors[0], ncolors, 3,
                        	         (int (*)(const void *, const void *))compare_rgb);
	        *indptr = (match - colors[0]) / 3;
	      }
	    }
	    break;
      }
    }
  }

 /*
  * Now write the image...
  */

  switch (PSLevel)
  {
    case 0 : /* PDF */
	flate_printf(out, "q %.1f 0 0 %.1f %.1f %.1f cm\n", r->width, r->height,
	           r->x, r->y);
        flate_puts("BI", out);

	if (ncolors > 0)
	{
	  if (ncolors <= 2)
	    ncolors = 2; /* Adobe doesn't like 1 color images... */

	  flate_printf(out, "/CS[/I/RGB %d<", ncolors - 1);
	  for (i = 0; i < ncolors; i ++)
	    flate_printf(out, "%02X%02X%02X", colors[i][0], colors[i][1], colors[i][2]);
	  flate_puts(">]", out);
        }
	else if (img->depth == 1)
          flate_puts("/CS/G", out);
        else
          flate_puts("/CS/RGB", out);

        flate_puts("/I true", out);

        if (ncolors > 0)
	{
  	  flate_printf(out, "/W %d/H %d/BPC %d ID\n",
               	       img->width, img->height, indbits); 

  	  flate_write(out, indices, indwidth * img->height, 1);
	}
	else if (OutputJPEG)
	{
  	  flate_printf(out, "/W %d/H %d/BPC 8/F/DCT ID\n",
                       img->width, img->height); 

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
  	  flate_printf(out, "/W %d/H %d/BPC 8 ID\n",
               	       img->width, img->height); 

  	  flate_write(out, img->pixels,
	              img->width * img->height * img->depth, 1);
        }

	flate_write(out, (uchar *)"\nEI\nQ\n", 6, 1);
        break;

    case 1 : /* PostScript, Level 1 */
        fputs("GS", out);
	fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	        r->x, r->y);

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
        // Fallthrough to Level 2 output if compression is disabled...
        if (Compression && (!OutputJPEG || ncolors > 0))
	{
          fputs("GS", out);
	  fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	          r->x, r->y);

          if (ncolors > 0)
          {
	    if (ncolors <= 2)
	      ncolors = 2; /* Adobe doesn't like 1 color images... */

	    fprintf(out, "[/Indexed/DeviceRGB %d<", ncolors - 1);
	    for (i = 0; i < ncolors; i ++)
	      fprintf(out, "%02X%02X%02X", colors[i][0], colors[i][1], colors[i][2]);
	    fputs(">]setcolorspace", out);

	    fprintf(out, "<<"
	                 "/ImageType 1"
	                 "/Width %d"
	                 "/Height %d"
	                 "/BitsPerComponent %d"
	                 "/ImageMatrix[%d 0 0 %d 0 %d]"
	                 "/Decode[0 %d]"
		         "/Interpolate true"
	                 "/DataSource currentfile/ASCII85Decode filter"
		         "/FlateDecode filter"
	                 ">>image\n",
	            img->width, img->height, indbits,
        	    img->width, -img->height, img->height,
        	    (1 << indbits) - 1);

            flate_open_stream(out);
	    flate_write(out, indices, indwidth * img->height);
	    flate_close_stream(out);
          }
          else
          {
	    if (img->depth == 1)
	      fputs("/DeviceGray setcolorspace", out);
	    else
	      fputs("/DeviceRGB setcolorspace", out);

	    fprintf(out, "<<"
	                 "/ImageType 1"
	                 "/Width %d"
	                 "/Height %d"
	                 "/BitsPerComponent 8"
	                 "/ImageMatrix[%d 0 0 %d 0 %d]"
	                 "/Decode[%s]"
		         "/Interpolate true"
	                 "/DataSource currentfile/ASCII85Decode filter"
		         "/FlateDecode filter"
	                 ">>image\n",
	            img->width, img->height,
        	    img->width, -img->height, img->height,
        	    img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

            flate_open_stream(out);
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

        if (ncolors > 0)
        {
	  fprintf(out, "[/Indexed/DeviceRGB %d<", ncolors - 1);
	  for (i = 0; i < ncolors; i ++)
	    fprintf(out, "%02X%02X%02X", colors[i][0], colors[i][1], colors[i][2]);
	  fputs(">]setcolorspace", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent %d"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[0 %d]"
		       "/Interpolate true"
	               "/DataSource currentfile/ASCII85Decode filter"
	               ">>image\n",
	          img->width, img->height, indbits,
        	  img->width, -img->height, img->height,
        	  (1 << indbits) - 1);

	  ps_ascii85(out, indices, indwidth * img->height);
          fputs("~>\n", out);
        }
	else if (OutputJPEG)
	{
	  if (img->depth == 1)
	    fputs("/DeviceGray setcolorspace", out);
	  else
	    fputs("/DeviceRGB setcolorspace", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent 8"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[%s]"
		       "/Interpolate true"
	               "/DataSource currentfile/ASCII85Decode filter/DCTDecode filter"
	               ">>image\n",
	          img->width, img->height,
        	  img->width, -img->height, img->height,
        	  img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

	  jpg_setup(out, img, &cinfo);

	  for (i = img->height, pixel = img->pixels;
	       i > 0;
	       i --, pixel += img->width * img->depth)
	    jpeg_write_scanlines(&cinfo, &pixel, 1);

	  jpeg_finish_compress(&cinfo);
	  jpeg_destroy_compress(&cinfo);

          fputs("~>\n", out);
        }
        else
        {
	  if (img->depth == 1)
	    fputs("/DeviceGray setcolorspace", out);
	  else
	    fputs("/DeviceRGB setcolorspace", out);

	  fprintf(out, "<<"
	               "/ImageType 1"
	               "/Width %d"
	               "/Height %d"
	               "/BitsPerComponent 8"
	               "/ImageMatrix[%d 0 0 %d 0 %d]"
	               "/Decode[%s]"
		       "/Interpolate true"
	               "/DataSource currentfile/ASCII85Decode filter"
	               ">>image\n",
	          img->width, img->height,
        	  img->width, -img->height, img->height,
        	  img->depth == 1 ? "0 1" : "0 1 0 1 0 1");

	  ps_ascii85(out, img->pixels, img->width * img->height * img->depth);
          fputs("~>\n", out);
        }

	fputs("GR\n", out);
        break;
  }

  if (ncolors > 0)
    free(indices);
}


/*
 * 'write_prolog()' - Write the file prolog...
 */

static void
write_prolog(FILE *out,		/* I - Output file */
             int  page_count,	/* I - Number of pages (0 if not known) */
             uchar *title,	/* I - Title of document */
             uchar *author,	/* I - Author of document */
             uchar *creator,	/* I - Application that generated the HTML file */
             uchar *copyright,	/* I - Copyright (if any) on the document */
             uchar *keywords)	/* I - Search keywords */
{
  int		i, j,		/* Looping vars */
		encoding_object;/* Font encoding object */
  struct tm	*curdate;	/* Current date */
  int		page;		/* Current page */
  render_t	*r;		/* Current render data */
  int		fonts_used[4][4];/* Whether or not a font is used */
  char		temp[255];	/* Temporary string */
  md5_state_t	md5;		/* MD5 state */
  md5_byte_t	digest[16];	/* MD5 digest value */
  rc4_context_t	rc4;		/* RC4 context */
  uchar		owner_pad[32],	/* Padded owner password */
		owner_key[32],	/* Owner key */
		user_pad[32],	/* Padded user password */
		user_key[32];	/* User key */
  uchar		perm_bytes[4];	/* Permission bytes */
  unsigned	perm_value;	/* Permission value, unsigned */
  static unsigned char pad[32] =
		{		/* Padding for passwords */
		  0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41,
		  0x64, 0x00, 0x4e, 0x56, 0xff, 0xfa, 0x01, 0x08,
		  0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68, 0x3e, 0x80,
		  0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a
		};


 /*
  * Get the current time and date (ZULU).
  */

  curdate = gmtime(&doc_time);

 /*
  * See what fonts are used...
  */

  memset(fonts_used, 0, sizeof(fonts_used));
  fonts_used[HeadFootType][HeadFootStyle] = 1;

  for (page = 0; page < num_pages; page ++)
    for (r = pages[page]; r != NULL; r = r->next)
      if (r->type == RENDER_TEXT)
	fonts_used[r->data.text.typeface][r->data.text.style] = 1;

 /*
  * Generate the heading...
  */

  if (PSLevel > 0)
  {
   /*
    * Write PostScript prolog stuff...
    */

    fputs("%!PS-Adobe-3.0\n", out);
    if (Landscape)
      fprintf(out, "%%%%BoundingBox: 0 0 %d %d\n", PageLength, PageWidth);
    else
      fprintf(out, "%%%%BoundingBox: 0 0 %d %d\n", PageWidth, PageLength);
    fprintf(out,"%%%%LanguageLevel: %d\n", PSLevel);
    fputs("%%Creator: htmldoc " SVERSION " Copyright 1997-2000 Easy Software Products, All Rights Reserved.\n", out);
    fprintf(out, "%%%%CreationDate: D:%04d%02d%02d%02d%02d%02dZ\n",
            curdate->tm_year + 1900, curdate->tm_mon + 1, curdate->tm_mday,
            curdate->tm_hour, curdate->tm_min, curdate->tm_sec);
    if (title != NULL)
      fprintf(out, "%%%%Title: %s\n", title);
    if (author != NULL)
      fprintf(out, "%%%%Author: %s\n", author);
    if (creator != NULL)
      fprintf(out, "%%%%Generator: %s\n", creator);
    if (copyright != NULL)
      fprintf(out, "%%%%Copyright: %s\n", copyright);
    if (keywords != NULL)
      fprintf(out, "%%%%Keywords: %s\n", keywords);
    if (page_count > 0)
      fprintf(out, "%%%%Pages: %d\n", page_count);
    else
      fputs("%%Pages: (atend)\n", out);
    fputs("%%DocumentNeededResources:\n", out);
    for (i = 0; i < 4; i ++)
      for (j = 0; j < 4; j ++)
        if (fonts_used[i][j])
          fprintf(out, "%%%%+ font %s\n", _htmlFonts[i][j]);
    fputs("%%DocumentData: Clean7bit\n", out);
    fputs("%%EndComments\n", out);

    fputs("%%BeginProlog\n", out);

   /*
    * Output the font encoding for the current character set...  For now we
    * just support 8-bit fonts since true Unicode support needs a very large
    * number of extra fonts that aren't normally available on a PS printer.
    */

    fputs("/fontencoding[\n", out);
    for (i = 0; i < 256; i ++)
    {
      putc('/', out);
      if (_htmlGlyphs[i])
        fputs(_htmlGlyphs[i], out);
      else
        fputs(".notdef", out);

      if ((i & 7) == 7)
        putc('\n', out);
    }

    fputs("]def", out);

   /*
    * Fonts...
    */

    for (i = 0; i < 4; i ++)
      for (j = 0; j < 4; j ++)
        if (fonts_used[i][j])
        {
	  fprintf(out, "/%s findfont\n", _htmlFonts[i][j]);
	  if (i < 3)
	    fputs("dup length dict begin"
        	  "{1 index/FID ne{def}{pop pop}ifelse}forall"
        	  "/Encoding fontencoding def"
        	  " currentdict end\n", out);
	  fprintf(out, "/F%x exch definefont pop\n", i * 4 + j);
        }

   /*
    * Now for the macros...
    */

    fputs("/BD{bind def}bind def\n", out);
    fputs("/B{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath stroke}BD\n", out);
    if (!OutputColor)
      fputs("/C{0.08 mul exch 0.61 mul add exch 0.31 mul add setgray}BD\n", out);
    else
      fputs("/C{setrgbcolor}BD\n", out);
    fputs("/CM{concat}BD\n", out);
    fputs("/F{dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath fill}BD\n", out);
    fputs("/GS{gsave}BD", out);
    fputs("/GR{grestore}BD", out);
    fputs("/L{0 rlineto stroke}BD", out);
    fputs("/M{moveto}BD\n", out);
    fputs("/S{show}BD", out);
    fputs("/SF{findfont exch scalefont setfont}BD", out);
    fputs("/SP{showpage}BD", out);
    fputs("/T{translate}BD\n", out);

    if (background_image != NULL)
      ps_write_background(out);

    fputs("%%EndProlog\n", out);

    if (PSLevel > 1 && PSCommands)
    {
      fputs("%%BeginSetup\n", out);

      if (Landscape)
      {
        if (PageWidth == 612 && PageLength == 792)
	  fputs("%%BeginFeature: PageSize Letter.Transverse\n", out);
        if (PageWidth == 612 && PageLength == 1008)
	  fputs("%%BeginFeature: PageSize Legal.Transverse\n", out);
        if (PageWidth == 595 && PageLength == 842)
	  fputs("%%BeginFeature: PageSize A4.Transverse\n", out);
        else
	  fprintf(out, "%%%%BeginFeature: PageSize w%dh%d\n", PageLength,
	          PageWidth);

	fprintf(out, "<</PageSize[%d %d]>>setpagedevice\n", PageLength,
	        PageWidth);
        fputs("%%EndFeature\n", out);

        if (PageDuplex)
	{
	  fputs("%%BeginFeature: Duplex DuplexTumble\n", out);
	  fputs("<</Duplex true/Tumble true>>setpagedevice\n", out);
          fputs("%%EndFeature\n", out);
	}
      }
      else
      {
        if (PageWidth == 612 && PageLength == 792)
	  fputs("%%BeginFeature: PageSize Letter\n", out);
        if (PageWidth == 612 && PageLength == 1008)
	  fputs("%%BeginFeature: PageSize Legal\n", out);
        if (PageWidth == 595 && PageLength == 842)
	  fputs("%%BeginFeature: PageSize A4\n", out);
        else
	  fprintf(out, "%%%%BeginFeature: PageSize w%dh%d\n", PageWidth,
	          PageLength);

	fprintf(out, "<</PageSize[%d %d]>>setpagedevice\n", PageWidth,
	        PageLength);
        fputs("%%EndFeature\n", out);

        if (PageDuplex)
	{
	  fputs("%%BeginFeature: Duplex DuplexNoTumble\n", out);
	  fputs("<</Duplex true/Tumble false>>setpagedevice\n", out);
          fputs("%%EndFeature\n", out);
	}
      }

      if (!PageDuplex)
      {
	fputs("%%BeginFeature: Duplex None\n", out);
	fputs("<</Duplex false>>setpagedevice\n", out);
        fputs("%%EndFeature\n", out);
      }

      fputs("%%EndSetup\n", out);
    }
  }
  else
  {
   /*
    * Write PDF prolog stuff...
    */

    fprintf(out, "%%PDF-%.1f\n", PDFVersion);
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

      strncpy((char *)user_pad, UserPassword, sizeof(user_pad));

      if ((i = strlen(UserPassword)) < 32)
	memcpy(user_pad + i, pad, 32 - i);

      if (OwnerPassword[0])
      {
       /*
        * Copy and pad the owner password...
	*/

        strncpy((char *)owner_pad, OwnerPassword, sizeof(owner_pad));

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
      * Compute the owner key...
      */

      md5_init(&md5);
      md5_append(&md5, owner_pad, 32);
      md5_finish(&md5, digest);

      rc4_init(&rc4, digest, 5);
      rc4_encrypt(&rc4, user_pad, owner_key, 32);

     /*
      * Compute the encryption key...
      */

      md5_init(&md5);
      md5_append(&md5, user_pad, 32);
      md5_append(&md5, owner_key, 32);

      perm_value = (unsigned)Permissions;
      perm_bytes[0] = perm_value;
      perm_bytes[1] = perm_value >> 8;
      perm_bytes[2] = perm_value >> 16;
      perm_bytes[3] = perm_value >> 24;

      md5_append(&md5, perm_bytes, 4);
      md5_append(&md5, file_id, 16);
      md5_finish(&md5, digest);

      memcpy(encrypt_key, digest, 5);

      rc4_init(&rc4, digest, 5);
      rc4_encrypt(&rc4, pad, user_key, 32);

     /*
      * Write the encryption dictionary...
      */

      num_objects ++;
      encrypt_object = num_objects;
      objects[num_objects] = ftell(out);
      fprintf(out, "%d 0 obj", num_objects);
      fputs("<<", out);
      fputs("/Filter/Standard/R 2/O<", out);
      for (i = 0; i < 32; i ++)
        fprintf(out, "%02x", owner_key[i]);
      fputs(">/U<", out);
      for (i = 0; i < 32; i ++)
        fprintf(out, "%02x", user_key[i]);
      fprintf(out, ">/P %d/V 1", Permissions);
      fputs(">>", out);
      fputs("endobj\n", out);
    }
    else
      encrypt_object = 0;

   /*
    * Write info object...
    */

    num_objects ++;
    info_object = num_objects;
    objects[num_objects] = ftell(out);
    fprintf(out, "%d 0 obj", num_objects);
    fputs("<<", out);
    fputs("/Producer", out);
    write_string(out, (uchar *)"htmldoc " SVERSION " Copyright 1997-2000 Easy "
                               "Software Products, All Rights Reserved.", 0);
    fputs("/CreationDate", out);
    sprintf(temp, "D:%04d%02d%02d%02d%02d%02dZ",
            curdate->tm_year + 1900, curdate->tm_mon + 1, curdate->tm_mday,
            curdate->tm_hour, curdate->tm_min, curdate->tm_sec);
    write_string(out, (uchar *)temp, 0);

    if (title != NULL)
    {
      fputs("/Title", out);
      write_string(out, title, 0);
    }

    if (author != NULL)
    {
      fputs("/Author", out);
      write_string(out, author, 0);
    }

    if (creator != NULL)
    {
      fputs("/Creator", out);
      write_string(out, creator, 0);
    }

    if (keywords != NULL)
    {
      fputs("/Keywords", out);
      write_string(out, keywords, 0);
    }

    fputs(">>", out);
    fputs("endobj\n", out);

   /*
    * Write the font encoding for the selected character set.  Note that
    * we *should* be able to use the WinAnsiEncoding value for ISO-8859-1
    * to make smaller files, however Acrobat Exchange does not like it
    * despite the fact that it is defined in the PDF specification...
    */

    num_objects ++;
    encoding_object = num_objects;
    objects[num_objects] = ftell(out);

    fprintf(out, "%d 0 obj", encoding_object);
    fputs("<<", out);
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

    fputs("]>>", out);
    fputs("endobj\n", out);

    for (i = 0; i < 4; i ++)
      for (j = 0; j < 4; j ++)
        if (fonts_used[i][j])
        {
	  num_objects ++;
	  font_objects[i * 4 + j] = num_objects;
	  objects[num_objects] = ftell(out);

	  fprintf(out, "%d 0 obj", font_objects[i * 4 + j]);
	  fputs("<<", out);
	  fputs("/Type/Font", out);
	  fputs("/Subtype/Type1", out);
	  fprintf(out, "/BaseFont/%s", _htmlFonts[i][j]);
	  if (i < 3) /* Use native encoding for symbols */
	    fprintf(out, "/Encoding %d 0 R", encoding_object);
	  fputs(">>", out);
	  fputs("endobj\n", out);
        }

    if (background_image != NULL)
      pdf_write_background(out);
  }
}


/*
 * 'write_string()' - Write a text entity.
 */

static void
write_string(FILE  *out,	/* I - Output file */
             uchar *s,		/* I - String */
	     int   compress)	/* I - Compress output? */
{
  int	i;			/* Looping var */


  if (Encryption && !compress && PSLevel == 0)
  {
    int		len;		// Length of string
    uchar	news[1024];	// New string


   /*
    * Write an encrypted string...
    */

    putc('<', out);
    encrypt_init();
    rc4_encrypt(&encrypt_state, s, news, len = strlen((char *)s));
    for (i = 0; i < len; i ++)
      fprintf(out, "%02x", news[i]);
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

  write_string(out, r->data.text.buffer, PSLevel == 0);

  if (PSLevel > 0)
    fputs("S\n", out);
  else
    flate_puts("Tj\n", out);

  render_x += r->width;
}


/*
 * 'write_trailer()' - Write the file trailer.
 */

static void
write_trailer(FILE *out,	/* I - Output file */
              int  pages)	/* I - Number of pages in file */
{
  int		i, j,		/* Looping vars */
		type,		/* Type of number */
		offset;		/* Offset to xref table in PDF file */
  static char	*modes[] =	/* Page modes */
		{
		  "UseNone",
		  "UseOutlines",
		  "FullScreen"
		};
  static char	*layouts[] =	/* Page layouts */
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
    if (pages > 0)
      fprintf(out, "%%%%Pages: %d\n", pages);

    fputs("%%EOF\n", out);
  }
  else
  {
   /*
    * PDF...
    */

    num_objects ++;
    root_object = num_objects;
    objects[num_objects] = ftell(out);
    fprintf(out, "%d 0 obj", num_objects);
    fputs("<<", out);
    fputs("/Type/Catalog", out);
    fprintf(out, "/Pages %d 0 R", pages_object);

    if (PDFVersion >= 1.2)
    {
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
            fprintf(out, "/OpenAction[%d 0 R/XYZ null null null]",
                    pages_object + 1);
            break;
	  }
          break;
      case PDF_TOC :
          if (TocLevels > 0)
	  {
            fprintf(out, "/OpenAction[%d 0 R/XYZ null null null]",
                    pages_object + 3 * chapter_starts[0] + 1);
	    break;
	  }
          break;
      case PDF_CHAPTER_1 :
          fprintf(out, "/OpenAction[%d 0 R/XYZ null null null]",
                  pages_object + 3 * chapter_starts[1] + 1);
          break;
    }

    fprintf(out, "/PageMode/%s", modes[PDFPageMode]);

    if (PDFVersion > 1.2)
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

      if (TocLevels > 0)
      {
        type = 'v';
        for (j = 0; j < 3; j ++)
	  if (TocHeader[j] == '1')
	    type = 'D';
	  else if (TocHeader[j] == 'i')
	    type = 'r';
	  else if (TocHeader[j] == 'I')
	    type = 'R';
	  else if (TocFooter[j] == '1')
	    type = 'D';
	  else if (TocFooter[j] == 'i')
	    type = 'r';
	  else if (TocFooter[j] == 'I')
	    type = 'R';

        fprintf(out, "%d<</S/%c>>", i, type);

        i += chapter_ends[0] - chapter_starts[0] + 1;
      }

      type = 'D';
      for (j = 0; j < 3; j ++)
	if (Header[j] == '1')
	  type = 'D';
	else if (Header[j] == 'i')
	  type = 'r';
	else if (Header[j] == 'I')
	  type = 'R';
	else if (Footer[j] == '1')
	  type = 'D';
	else if (Footer[j] == 'i')
	  type = 'r';
	else if (Footer[j] == 'I')
	  type = 'R';

      fprintf(out, "%d<</S/%c>>", i, type);
      fputs("]>>", out);
    }

    fputs(">>", out);
    fputs("endobj\n", out);

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
 * 'encrypt_init()' - Initialize the RC4 encryption context for the current
 *                    object.
 */

static void
encrypt_init(void)
{
  uchar		data[10];	/* Key data */
  md5_state_t	md5;		/* MD5 state */
  md5_byte_t	digest[16];	/* MD5 digest value */


 /*
  * Compute the key data for the MD5 hash.
  */

  data[0] = encrypt_key[0];
  data[1] = encrypt_key[1];
  data[2] = encrypt_key[2];
  data[3] = encrypt_key[3];
  data[4] = encrypt_key[4];
  data[5] = num_objects;
  data[6] = num_objects >> 8;
  data[7] = num_objects >> 16;
  data[8] = 0;
  data[9] = 0;

 /*
  * Hash it...
  */

  md5_init(&md5);
  md5_append(&md5, data, 10);
  md5_finish(&md5, digest);

 /*
  * Initialize the RC4 context using the first 10 bytes of the digest...
  */

  rc4_init(&encrypt_state, digest, 10);
}


/*
 * 'flate_open_stream()' - Open a deflated output stream.
 */

static void
flate_open_stream(FILE *out)	/* I - Output file */
{
  if (Encryption && !PSLevel)
    encrypt_init();

  if (!Compression)
    return;

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
flate_close_stream(FILE *out)	/* I - Output file */
{
  if (!Compression)
    return;

  while (deflate(&compressor, Z_FINISH) != Z_STREAM_END)
  {
    if (PSLevel)
      ps_ascii85(out, comp_buffer,
                 (uchar *)compressor.next_out - (uchar *)comp_buffer);
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
      ps_ascii85(out, comp_buffer,
                 (uchar *)compressor.next_out - (uchar *)comp_buffer);
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

  if (PSLevel)
    fputs("~>\n", out);
}


/*
 * 'flate_puts()' - Write a character string to a compressed stream.
 */

static void
flate_puts(char *s,	/* I - String to write */
           FILE *out)	/* I - Output file */
{
  flate_write(out, (uchar *)s, strlen(s));
}


/*
 * 'flate_printf()' - Write a formatted character string to a compressed stream.
 */

static void
flate_printf(FILE *out,		/* I - Output file */
           char *format,	/* I - Format string */
           ...)			/* I - Additional args as necessary */
{
  int		length;		/* Length of output string */
  char		buf[10240];	/* Output buffer */
  va_list	ap;		/* Argument pointer */


  va_start(ap, format);
  length = vsprintf(buf, format, ap);
  va_end(ap);

  flate_write(out, (uchar *)buf, length);
}


/*
 * 'flate_write()' - Write data to a compressed stream.
 */

static void
flate_write(FILE  *out,		/* I - Output file */
            uchar *buf,		/* I - Buffer */
            int   length,	/* I - Number of bytes to write */
	    int   flush)	/* I - Flush when writing data? */
{
  if (Compression)
  {
    compressor.next_in  = buf;
    compressor.avail_in = length;

    while (compressor.avail_in > 0)
    {
      if (compressor.avail_out < (sizeof(comp_buffer) / 8))
      {
	if (PSLevel)
	  ps_ascii85(out, comp_buffer,
                     (uchar *)compressor.next_out - (uchar *)comp_buffer);
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

      deflate(&compressor, flush ? Z_FULL_FLUSH : Z_NO_FLUSH);
      flush = 0;
    }
  }
  else
  {
    if (Encryption && !PSLevel)
    {
      int	i,		// Looping var
		bytes;		// Number of bytes to encrypt/write
      uchar	newbuf[1024];	// New encrypted data buffer


      for (i = 0; i < length; i += sizeof(newbuf))
      {
        if ((bytes = length - i) > sizeof(newbuf))
	  bytes = sizeof(newbuf);

        rc4_encrypt(&encrypt_state, buf + i, newbuf, bytes);
        fwrite(newbuf, bytes, 1, out);
      }
    }
    else
      fwrite(buf, length, 1, out);
  }
}


/*
 * End of "$Id: ps-pdf.cxx,v 1.89.2.3 2000/12/01 21:46:48 mike Exp $".
 */
