/*
 * "$Id: render.h,v 1.21.2.8 2004/03/30 03:49:15 mike Exp $"
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
 *       Hollywood, Maryland 20636-3142 USA
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

#  include <config.h> // TEMPORARY
#  include "tree.h"
#  include "image.h"
#  include "md5.h"
#  include "rc4.h"
#  include <zlib.h>
#  include <string.h>
//#  include <sys/types.h>

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
  //* Style for node
  hdStyle	*style;

  union
  {
    //* Text data
    const char	*text;

    //* Image pointer
    hdImage	*image;

    //* Box color
    float	box[3];

    //* Link data
    const char	*link;
  }		data;

 /**
  * The constructor creates a new render node with the specified data.
  *
  * @param t hdRenderType The type of render primitive, HD_RENDERTYPE_TEXT,
  * HD_RENDERTYPE_IMAGE, HD_RENDERTYPE_BOX, HD_RENDERTYPE_BACKGROUND,
  * HD_RENDERTYPE_LINK, or HD_RENDERTYPE_FORM.
  * @param xx float The X position in points.
  * @param yy float The Y position in points.
  * @param w float The width in points.
  * @param h float The height in points.
  * @param s hdStyle* The style associated with the element.
  * @param d const&nbsp;void* The data associated with the node, if any.
  * @param copy_d bool Whether the data should be copied (true) or not (false).
  */
  hdRenderNode(hdRenderType t, float xx, float yy, float w, float h,
               hdStyle *s, const void *d = (const void *)0);

  hdRenderNode();
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
  const char	*title;
  //* Chapter text
  const char	*chapter;
  //* Heading text
  const char	*heading;
  //* Headers
  const char	*header[3];
  //* Footers
  const char	*footer[3];
  //* Media color
  const char	*media_color;
  //* Media type
  const char	*media_type;
  //* Media position
  int		media_position;
  //* Page number for TOC
  char		page_text[64];
  //* Background
  hdStyle	*background;

  //* Number up pages
  int		nup;
  //* Output page #
  int		outpage;
  //* Transform matrix
  float		outmatrix[2][3];
  //* Formatted headers
  const char	*formatted_header[3];
  //* Formatted footers
  const char	*formatted_footer[3];
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
  const char	*filename;
  //* Reference name
  const char	*name;
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


/**
 * The hdRender class handles rendering a document to one or more pages.
 */

class hdRender
{
  public:

  hdRender();
  virtual ~hdRender();

 /**
  * The <tt>compression_level()</tt> method returns the current compression
  * level for the renderer.
  *
  * @return The compression level from 0 (none) to 9 (maximum).
  */
  int		compression_level(void) const { return (compression_level_); }

 /**
  * The <tt>compression_level()</tt> method sets the current compression
  * level for the renderer.
  *
  * @param c int The compression level from 0 (none) to 9 (maximum).
  */
  void		compression_level(int c);

 /**
  * The <tt>doc_time()</tt> method returns the current document time value.
  *
  * @return The UTC time value in seconds.
  */
  time_t	doc_time(void) const { return (doc_time_); }

 /**
  * The <tt>doc_time()</tt> method sets the current document time value.
  * If 0, the current time value is used.
  *
  * @param t time_t The UTC time value in seconds.
  */
  void		doc_time(time_t t = 0);

 /**
  * The <tt>export_doc</tt> method exports the given document to the
  * specified file or directory.
  *
  * @param path const&nbsp;char* The output path.
  * @param multiple bool Whether to output multiple files.
  * @param type hdDocType The type of document.
  * @param document tree_t The document tree to render.
  * @param toc tree_t The table-of-contents to render, if any.
  */
  virtual int	export_doc(const char *path, bool multiple, hdDocType type,
		           tree_t *document, tree_t *toc = 0) = 0;

 /**
  * The <tt>jpeg_quality</tt> method returns the current JPEG quality
  * level.
  *
  * @return The JPEG quality from 0 (disabled) to 100 (best)
  */
  int		jpeg_quality(void) const { return (jpeg_quality_); }

 /**
  * The <tt>jpeg_quality</tt> method sets the current JPEG quality level.
  *
  * @param q int The JPEG quality from 0 (disabled) to 100 (best)
  */
  void		jpeg_quality(int q);

  int		StrictHTML	VALUE(0);	/* Do strict HTML checking */
  int		TitlePage	VALUE(1),	/* Need a title page */
		TocLevels	VALUE(3),	/* Number of table-of-contents levels */
		TocLinks	VALUE(1),	/* Generate links */
		TocNumbers	VALUE(0),	/* Generate heading numbers */
		TocDocCount	VALUE(0);	/* Number of chapters */
  int		OutputType	VALUE(OUTPUT_BOOK);
						/* Output a "book", etc. */
  char		OutputPath[255]	VALUE("");	/* Output directory/name */
  int		OutputFiles	VALUE(0),	/* Generate multiple files? */
		OutputColor	VALUE(1);	/* Output color images */
  int		OutputJPEG	VALUE(0);	/* JPEG compress images? */
  int		PDFVersion	VALUE(13);	/* Version of PDF to support */
  int		PDFPageMode	VALUE(PDF_OUTLINE),
						/* PageMode attribute */
		PDFPageLayout	VALUE(PDF_SINGLE),
						/* PageLayout attribute */
		PDFFirstPage	VALUE(PDF_CHAPTER_1),
						/* First page */
		PDFEffect	VALUE(PDF_NONE);/* Page transition effect */
  float		PDFEffectDuration VALUE(1.0),	/* Page effect duration */
		PDFPageDuration	VALUE(10.0);	/* Page duration */
  int		Encryption	VALUE(0),	/* Encrypt the PDF file? */
		Permissions	VALUE(-4);	/* File permissions? */
  char		OwnerPassword[33] VALUE(""),	/* Owner password */
		UserPassword[33] VALUE("");	/* User password */
  int		EmbedFonts	VALUE(0);	/* Embed fonts? */
  int		PSLevel		VALUE(2),	/* Language level (0 for PDF) */
		PSCommands	VALUE(0),	/* Output PostScript commands? */
		XRXComments	VALUE(0);	/* Output Xerox comments? */
  int		PageWidth	VALUE(595),	/* Page width in points */
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

//  typeface_t	HeadFootType	VALUE(TYPE_HELVETICA);
						/* Typeface for header & footer */
//  style_t	HeadFootStyle	VALUE(STYLE_NORMAL);
						/* Type style */
  float		HeadFootSize	VALUE(11.0f);	/* Size of header & footer */

  char		*Header[3]	NULL3,		/* Header for regular pages */
		*TocHeader[3]	NULL3,		/* Header for TOC pages */
		*Footer[3]	NULL3,		/* Regular page footer */
		*TocFooter[3]	NULL3,		/* Footer for TOC pages */
		TocTitle[1024]	VALUE("Table of Contents");
						/* TOC title string */

  char		TitleImage[1024] VALUE(""),	/* Title page image */
		LogoImage[1024]	VALUE(""),	/* Logo image */
		BodyColor[255]	VALUE(""),	/* Body color */
		BodyImage[1024]	VALUE(""),	/* Body image */
		LinkColor[255]	VALUE("");	/* Link color */

  char		HFImage[HD_MAX_HF_IMAGES][1024];	/* Header/footer images */

  int		LinkStyle	VALUE(1);	/* 1 = underline, 0 = plain */
  int		Links		VALUE(1);	/* 1 = generate links, 0 = no links */
  char		Path[2048]	VALUE(""),	/* Search path */
		Proxy[1024]	VALUE("");	/* Proxy URL */

  protected:

  //* Current stylesheet
  hdStyleSheet	*stylesheet;
  //* H1 heading style
  hdStyle	*h1_style;
  //* P paragraph style
  hdStyle	*p_style;
  //* IMG image style
  hdStyle	*img_style;
  //* P.htmldoc_hf paragraph style
  hdStyle	*p_hf_style;
  //* IMG.htmldoc_hf image style
  hdStyle	*img_hf_style;

  //* Output path
  char		path_[1024];
  //* Output multiple files?
  bool		multiple_;
  //* Document type
  hdDocType	type_;

  //* Current date
  struct tm	*doc_date_;
  //* Current date/time string
  char		doc_datetime_[255];
  //* Current time
  time_t	doc_time_;

  //* Flate compression level
  int		compression_level_;
  //* JPEG compression quality
  int		jpeg_quality_;

  int		title_page;
  int		chapter,
		chapter_outstarts[HD_MAX_CHAPTERS],
		chapter_outends[HD_MAX_CHAPTERS],
		chapter_starts[HD_MAX_CHAPTERS],
		chapter_ends[HD_MAX_CHAPTERS];

  int		num_headings,
		alloc_headings,
		*heading_pages,
		*heading_tops;

  int		num_pages,
		alloc_pages;
  hdRenderPage	*pages;
  hdTree	*current_heading;

  int		num_outpages;
  hdRenderOutPage *outpages;

  int		num_links,
		alloc_links;
  hdRenderLink	*links;

  char		list_types[16];
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
		font_objects[100],
		font_desc[100];
  int		num_fonts;
  hdStyleFont	*fonts[100];

  char		*doc_title;
  hdImage	*logo_image;
  float		logo_width,
		logo_height;

  hdImage	*hfimage[HD_MAX_HF_IMAGES];
  float		hfimage_width[HD_MAX_HF_IMAGES],
		hfimage_height[HD_MAX_HF_IMAGES];
  float		maxhfheight;

  hdImage	*background_image;
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
  char		comp_buffer[8192];
  char		encrypt_key[16];
  int		encrypt_len;
  rc4_context_t	encrypt_state;
  hdByte	file_id[16];

 /*
  * JPEG library destination data manager.  These routines direct
  * compressed data from libjpeg into the PDF or PostScript file.
  */

  FILE			*jpg_file;	/* JPEG file */
  char			jpg_buf[8192];	/* JPEG buffer */
  jpeg_destination_mgr	jpg_dest;	/* JPEG destination manager */
  struct jpeg_error_mgr	jerr;		/* JPEG error handler */


  void		debug_stats();

  void		transform_coords(hdRenderPage *p, float &x, float  &y);
  void		transform_page(int outpage, int pos, int page);

  void		prepare_outpages();
  void		prepare_page(int page);
  void		prepare_heading(int page, int print_page, char **format,
			        int y, char *page_text, int page_len,
				int render_heading = 1);

  void		ps_write_document(const char *author, const char *creator,
			          const char *copyright, const char *keywords,
				  const char *subject);
  void		ps_write_outpage(FILE *out, int outpage);
  void		ps_write_page(FILE *out, int page);
  void		ps_write_background(FILE *out);
  void		pdf_write_document(const char *author, const char *creator,
			           const char *copyright, const char *keywords,
				   const char *subject, hdTree *toc);
  void		pdf_write_outpage(FILE *out, int outpage);
  void		pdf_write_page(FILE *out, int page);
  void		pdf_write_resources(FILE *out, int page);
  void		pdf_write_contents(FILE *out, hdTree *toc, int parent,
			           int prev, int next, int *heading);
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
  void		flate_write(FILE *out, const char *inbuf, int length,
		            int flush=0);	

  void		render_contents(hdTree *t, hdMargin *margins, float *y,
		                int *page, int heading, hdTree *chap);
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

  char		format_number_[1024];
  const char	*format_number(int n, char f);
  int		get_measurement(const char *s, float mul = 1.0f);

  hdTree	*real_prev(hdTree *t);
  hdTree	*real_next(hdTree *t);

  void		check_pages(int page);

  void		add_link(const char *name, int page, int top);
  hdRenderLink	*find_link(const char *name);
  static int	compare_links(hdRenderLink *n1, hdRenderLink *n2);

  void		find_background(hdTree *t);
  void		write_background(int page, FILE *out);

  hdRenderNode	*new_render(int page, hdRenderType type, hdStyle *s,
		            float x, float y, float width, float height,
			    const void *data, hdRenderNode *insert = 0);
  void		copy_tree(hdTree *parent, hdTree *t);
  float		get_cell_size(hdTree *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
  float		get_table_size(hdTree *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
  hdTree	*flatten_tree(hdTree *t);
  float		get_width(char *s, int typeface, int style, int size);
  void		update_image_size(hdTree *t);
  char		*get_title(hdTree *doc);
  FILE		*open_file(void);
  void		set_color(FILE *out, float *rgb);
  void		set_font(FILE *out, int typeface, int style, float size);
  void		set_pos(FILE *out, float x, float y);
  void		write_prolog(FILE *out, int pages, const char *author,
		             const char *creator, const char *copyright,
			     const char *keywords, const char *subject);
  void		ps_hex(FILE *out, const hdByte *data, int length);
  void		ps_ascii85(FILE *out, const hdByte *data, int length);
  static void	jpg_init(j_compress_ptr cinfo);
  static boolean jpg_empty(j_compress_ptr cinfo);
  static void	jpg_term(j_compress_ptr cinfo);
  void		jpg_setup(FILE *out, hdImage *img, j_compress_ptr cinfo);
  static int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
  void		write_image(FILE *out, hdRenderNode *r, int write_obj = 0);
  void		write_imagemask(FILE *out, hdRenderNode *r);
  void		write_string(FILE *out, const char *s, int compress);
  void		write_text(FILE *out, hdRenderNode *r);
  void		write_trailer(FILE *out, int pages);
  int		write_type1(FILE *out, hdStyleFont *font);
};


#endif // !HTMLDOC_RENDER_H


/*
 * End of "$Id: render.h,v 1.21.2.8 2004/03/30 03:49:15 mike Exp $".
 */
