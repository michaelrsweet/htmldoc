/*
 * "$Id: render.h,v 1.21.2.3 2004/03/22 21:14:46 mike Exp $"
 *
 *   Render class definitions for HTMLDOC, a HTML document processing
 *   program.
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
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 */

#ifndef HTMLDOC_RENDER_H
#  define HTMLDOC_RENDER_H

/*
 * Include necessary headers.
 */

#  include "margin.h"
#  include "md5.h"
#  include "rc4.h"
#  include <zlib.h>

extern "C" {				// Workaround for JPEG header probs...
#  include <jpeglib.h>			// JPEG/JFIF image definitions

typedef int	(*hdCompareFunc)(const void *, const void *);
}


/**
 * Render node types...
 */

enum hdRenderType
{
  //* Text
  HD_RENDERTYPE_TEXT = 1,
  //* Image
  HD_RENDERTYPE_IMAGE = 2,
  //* Box
  HD_RENDERTYPE_BOX = 4,
  //* Background area
  HD_RENDERTYPE_BACKGROUND = 6,
  //* Link
  HD_RENDERTYPE_LINK = 8,
  //* Form field
  HD_RENDERTYPE_FORM = 16
};


//
// Structures...
//

/**
 * The hdRenderText structure 
 */
#if 0
struct hdRenderText
{
  //* Font for text
  hdStyleFont	*font;
  //* Size of text in points
  float		font_size;
  //* Inter-character spacing
  float		char_spacing;
  //* Color of text
  float		rgb[3];
  //* Did we allocate the string?
  int		alloc_string;
  //* String pointer
  char		*string;
};


/**
 * The hdRenderURL structure describes a single link on a page.
 */
struct hdRenderURL
{
  //* Did we allocate the URL?
  int		alloc_url;
  //* Link URL
  char		*url;
};
#endif // 0

/**
 * The hdRenderNode structure describes rendering primitives used when
 * producing PostScript and PDF output.
 */
struct hdRenderNode
{
  //* Previous rendering node
  hdRenderNode	*prev;
  //* Next rendering node
  hdRenderNode	*next;
  //* Type of node
  hdRenderType	type;
  //* X position in points
  float		x;
  //* Y position in points
  float		y;
  //* Width in points
  float		width;
  //* Height in points
  float		height;

#if 0
  union
  {
    //* Text data
    hdRenderText text;

    //* Image pointer
    hdImage	*image;

    //* Box color
    float	box[3];

    //* Background data
    hdStyle	*background;

    //* Link data
    hdRenderURL	link;
  }	data;
#else
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
#endif // 0

#if 0
 /**
  * The constructor creates a new render node with the specified data.
  *
  * @param t The type of render primitive, HD_RENDERTYPE_TEXT,
  * HD_RENDERTYPE_IMAGE, HD_RENDERTYPE_BOX, HD_RENDERTYPE_BACKGROUND,
  * HD_RENDERTYPE_LINK, or HD_RENDERTYPE_FORM.
  * @param xx The X position in points.
  * @param yy The Y position in points.
  * @param w The width in points.
  * @param h The height in points.
  * @param d The data associated with the node, if any.
  * @param alloc_d Whether the data was allocated (1) or not (0).
  */
  hdRenderNode(hdRenderType t, float xx, float yy, float w, float h,
               const void *d = (const void *)0, int alloc_d = 0);

 /**
  * The destructor frees all memory associated with the node, including
  * the data if it was allocated.
  */
  ~hdRenderNode();
#endif // 0
};


/**
 * The hdRenderPage structure holds all of the rendering data for a
 * page.
 */
struct hdRenderPage
{
  //* First node on page
  hdRenderNode	*first;
  //* Last node on page
  hdRenderNode	*last;

  //* Bitwise OR of all nodes
  unsigned	types;
  //* Width of page in points
  int		width;
  //* Length of page in points
  int		length;
  //* Left margin in points
  int		left;
  //* Right margin in points
  int		right;
  //* Top margin in points
  int		top;
  //* Bottom margin in points
  int		bottom;
  //* Duplex this page?
  int		duplex;
  //* Landscape orientation?
  int		landscape;
  //* Title text
  uchar		*title;
  //* Chapter text
  uchar		*chapter;
  //* Heading text
  uchar		*heading;
  //* Headers
  uchar		*header[3];
  //* Footers
  uchar		*footer[3];
  //* Media color
  char		media_color[64];
  //* Media type
  char		media_type[64];
  //* Media position
  int		media_position;
  //* Page number for TOC
  char		page_text[64];
//  //* Background
//  hdStyle	*background;
  //* Background image
  image_t	*background_image;
  //* Background color
  float		background_color[3];

  //* Number up pages
  int		nup;
  //* Output page #
  int		outpage;
  //* Transform matrix
  float		outmatrix[2][3];
};


/**
 * The hdRenderOutPage structure holds the output page information which
 * maps an output page to 1-16 hdRenderPage's.
 */

struct hdRenderOutPage
{
  //* Number up pages
  int		nup;
  //* Pages on this output page
  int		pages[16];
  //* Page object
  int		page_object;
  //* Annotation object
  int		annot_object;
};


/**
 * The hdRenderLink structure holds the positions and pages of
 * named links.
 */
struct hdRenderLink
{
  //* Page number
  short		page;
  //* Top position
  short		top;
  //* Filename
  uchar		*filename;
  //* Reference name
  uchar		*name;
};


/**
 * The hdRenderChapter structure holds the start and end pages for
 * a chapter.
 */
struct hdRenderChapter
{
  //* First page in chapter
  int		first;
  //* Last page in chapter
  int		last;
};

#if 0
/**
 * The hdRenderHeading structure holds the page and position of each
 * heading in a document.
 */

struct hdRenderHeading
{
  //* Page number
  int		page;
  //* Top position
  int		top;
  //* HTML tree node with heading
  hdTree	*node;
};
#endif // 0

/**
 * The hdRender class handles rendering a document to one or more pages.
 */

class hdRender
{
  public:

  hdRender();
  ~hdRender();

  int export_doc(tree_t *document, tree_t *toc);

  protected:

  time_t	doc_time;		// Current time
  struct tm	*doc_date;		// Current date

  int		title_page;
  int		chapter,
		chapter_outstarts[MAX_CHAPTERS],
		chapter_outends[MAX_CHAPTERS],
		chapter_starts[MAX_CHAPTERS],
		chapter_ends[MAX_CHAPTERS];

  int		num_headings,
		alloc_headings,
		*heading_pages,
		*heading_tops;

  int		num_pages,
		alloc_pages;
  hdRenderPage	*pages;
  tree_t	*current_heading;

  int		num_outpages;
  hdRenderOutPage *outpages;

  int		num_links,
		alloc_links;
  hdRenderLink	*links;

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

 /*
  * JPEG library destination data manager.  These routines direct
  * compressed data from libjpeg into the PDF or PostScript file.
  */

  FILE			*jpg_file;	/* JPEG file */
  uchar			jpg_buf[8192];	/* JPEG buffer */
  jpeg_destination_mgr	jpg_dest;	/* JPEG destination manager */
  struct jpeg_error_mgr	jerr;		/* JPEG error handler */


  void		debug_stats();

  void		transform_coords(hdRenderPage *p, float &x, float  &y);
  void		transform_page(int outpage, int pos, int page);

  void		prepare_outpages();
  void		prepare_page(int page);
  void		prepare_heading(int page, int print_page, uchar **format,
			        int y, char *page_text, int page_len,
				int render_heading = 1);

  void		ps_write_document(uchar *author, uchar *creator,
			          uchar *copyright, uchar *keywords,
				  uchar *subject);
  void		ps_write_outpage(FILE *out, int outpage);
  void		ps_write_page(FILE *out, int page);
  void		ps_write_background(FILE *out);
  void		pdf_write_document(uchar *author, uchar *creator,
			           uchar *copyright, uchar *keywords,
				   uchar *subject, tree_t *toc);
  void		pdf_write_outpage(FILE *out, int outpage);
  void		pdf_write_page(FILE *out, int page);
  void		pdf_write_resources(FILE *out, int page);
  void		pdf_write_contents(FILE *out, tree_t *toc, int parent,
			           int prev, int next, int *heading);
  void		pdf_write_links(FILE *out);
  void		pdf_write_names(FILE *out);
  int		pdf_count_headings(tree_t *toc);

  int		pdf_start_object(FILE *out, int array = 0);
  void		pdf_start_stream(FILE *out);
  void		pdf_end_object(FILE *out);

  void		encrypt_init(void);
  void		flate_open_stream(FILE *out);
  void		flate_close_stream(FILE *out);
  void		flate_puts(const char *s, FILE *out);
  void		flate_printf(FILE *out, const char *format, ...);
  void		flate_write(FILE *out, uchar *inbuf, int length, int flush=0);	

  void		render_contents(tree_t *t, hdMargin *margins, float *y,
		                int *page, int heading, tree_t *chap);
  int		count_headings(tree_t *t);
  void		parse_contents(tree_t *t, hdMargin *margins, float *y,
			       int *page, int *heading, tree_t *chap);
  void		parse_doc(tree_t *t, hdMargin *margins, float *x, float *y, int *page,
			  tree_t *cpara, int *needspace);
  void		parse_heading(tree_t *t, hdMargin *margins, float *x, float *y, int *page,
			      int needspace);
  void		parse_paragraph(tree_t *t, hdMargin *margins, float *x,
			        float *y, int *page, int needspace);
  void		parse_pre(tree_t *t, hdMargin *margins, float *x, float *y,
			  int *page, int needspace);
  void		parse_table(tree_t *t, hdMargin *margins, float *x, float *y,
			    int *page, int needspace);
  void		parse_list(tree_t *t, hdMargin *margins, float *x, float *y,
			   int *page, int needspace);
  void		init_list(tree_t *t);
  void		parse_comment(tree_t *t, hdMargin *margins, float *x, float *y,
			      int *page, tree_t *para, int needspace);

  tree_t	*real_prev(tree_t *t);
  tree_t	*real_next(tree_t *t);

  void		check_pages(int page);

  void		add_link(uchar *name, int page, int top);
  hdRenderLink	*find_link(uchar *name);
  static int	compare_links(hdRenderLink *n1, hdRenderLink *n2);

  void		find_background(tree_t *t);
  void		write_background(int page, FILE *out);

  hdRenderNode	*new_render(int page, hdRenderType type, float x, float y,
		            float width, float height, void *data,
			    hdRenderNode *insert = 0);
  void		copy_tree(tree_t *parent, tree_t *t);
  float		get_cell_size(tree_t *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
  float		get_table_size(tree_t *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
  tree_t	*flatten_tree(tree_t *t);
  float		get_width(uchar *s, int typeface, int style, int size);
  void		update_image_size(tree_t *t);
  uchar		*get_title(tree_t *doc);
  FILE		*open_file(void);
  void		set_color(FILE *out, float *rgb);
  void		set_font(FILE *out, int typeface, int style, float size);
  void		set_pos(FILE *out, float x, float y);
  void		write_prolog(FILE *out, int pages, uchar *author,
		             uchar *creator, uchar *copyright,
			     uchar *keywords, uchar *subject);
  void		ps_hex(FILE *out, uchar *data, int length);
  void		ps_ascii85(FILE *out, uchar *data, int length);
  static void	jpg_init(j_compress_ptr cinfo);
  static boolean jpg_empty(j_compress_ptr cinfo);
  static void	jpg_term(j_compress_ptr cinfo);
  void		jpg_setup(FILE *out, image_t *img, j_compress_ptr cinfo);
  static int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
  void		write_image(FILE *out, hdRenderNode *r, int write_obj = 0);
  void		write_imagemask(FILE *out, hdRenderNode *r);
  void		write_string(FILE *out, uchar *s, int compress);
  void		write_text(FILE *out, hdRenderNode *r);
  void		write_trailer(FILE *out, int pages);
  int		write_type1(FILE *out, typeface_t typeface,
			    style_t style);
};


#endif // !HTMLDOC_RENDER_H


/*
 * End of "$Id: render.h,v 1.21.2.3 2004/03/22 21:14:46 mike Exp $".
 */
