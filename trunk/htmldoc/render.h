//
// "$Id: render.h,v 1.1 2002/02/26 05:16:03 mike Exp $"
//
//   Render class definitions for HTMLDOC.
//
//   Copyright 1997-2002 by Easy Software Products.
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
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//

#ifndef _HTMLDOC_RENDER_H_
#  define _HTMLDOC_RENDER_H_

//
// Include necessary headers.
//

#  include "style.h"
#  include "image.h"
#  include "tree.h"
#  include "margin.h"


//
// Render node types...
//

enum hdRenderType
{
  HD_RENDERTYPE_TEXT = 1,
  HD_RENDERTYPE_IMAGE = 2,
  HD_RENDERTYPE_BOX = 4,
  HD_RENDERTYPE_LINK = 8,
  HD_RENDERTYPE_FORM = 16
};


//
// Structures...
//

struct hdRenderNode		//// Render node structure
{
  hdRenderNode	*next;			// Next rendering node
  hdRenderType	type;			// Type of node
  float		x,			// Position in points
		y,			// ...
		width,			// Size in points
		height;			// ...

  union
  {
    struct
    {
      hdStyleFont *font;		// Font for text
      float	font_size;		// Size of text in points
      float	char_spacing;		// Inter-character spacing
      float	rgb[3];			// Color of text
      int	alloc_string;		// Did we allocate the string?
      char	*string;		// String pointer
    }   	text;

    hdImage	*image;			// Image pointer

    float	box[3];			// Box color

    struct
    {
      int	alloc_url;		// Did we allocate the URL?
      char 	*url;			// Link URL
    }		link;
  }	data;

  hdRenderNode(hdRenderType t, float xx, float yy, float w, float h,
               const char *s = (const char *)0, int alloc_s = 0);
  ~hdRenderNode();
};

struct hdRenderPage		//// Page render information
{
  hdRenderNode	*first,			// First node on page
		*last;			// Last node on page
  
  int		width,			// Width of page in points
		length,			// Length of page in points
		left,			// Left margin in points
		right,			// Right margin in points
		top,			// Top margin in points
		bottom,			// Bottom margin in points
		duplex,			// Duplex this page?
		landscape;		// Landscape orientation?
  char		*chapter,		// Chapter text
		*heading,		// Heading text
		*header[3],		// Headers
		*footer[3];		// Footers
  char		media_color[64],	// Media color
		media_type[64];		// Media type
  int		media_position;		// Media position
  int		page_object,		// Page object
		annot_object;		// Annotation object
  char		page_text[64];		// Page number for TOC
  hdImage	*background_image;	// Background image
  float		background_color[3];	// Background color
};


struct hdRenderLink		//// Named link position structure
{
  short		page,			// Page #
		top;			// Top position
  char		*name;			// Reference name
};


//
// Main rendering structure/class...
//

struct hdRender
{
  time_t	doc_time;	// Current time
  struct tm	*doc_date;	// Current date

  int		title_page;
  int		chapter,
		chapter_starts[MAX_CHAPTERS],
		chapter_ends[MAX_CHAPTERS];

  int		num_headings,
		alloc_headings,
		*heading_pages,
		*heading_tops;

  int		num_pages,
		alloc_pages;
  hdRenderPage	*pages;
  hdTree	*current_heading;

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
		font_objects[16];

  char		*doc_title;
  hdImage	*logo_image;
  float		logo_width,
		logo_height;
  hdImage	*background_image;
  float		background_color[3],
		link_color[3];

  hdStyleFont	*render_font;
  float		render_size,
		render_rgb[3],
		render_x,
		render_y,
		render_startx,
		render_spacing;

  char		encrypt_key[16];
  int		encrypt_len;
  hdMD5Byte	file_id[16];


  void	pspdf_prepare_page(int page);
  void	pspdf_prepare_heading(int page, int print_page, char **format,
		                      int y, char *page_text, int page_len);
  void	ps_write_document(char *author, char *creator,
		                  char *copyright, char *keywords);
  void	ps_write_page(hdFile *out, int page);
  void	ps_write_background(hdFile *out);
  void	pdf_write_document(char *author, char *creator,
		                   char *copyright, char *keywords,
				   hdTree *toc);
  void	pdf_write_page(hdFile *out, int page);
  void	pdf_write_resources(hdFile *out, int page);
  void	pdf_write_contents(hdFile *out, hdTree *toc, int parent,
		                   int prev, int next, int *heading);
  void	pdf_write_links(hdFile *out);
  void	pdf_write_names(hdFile *out);
  int	pdf_count_headings(hdTree *toc);

  int	pdf_start_object(hdFile *out, int array = 0);
  void	pdf_start_stream(hdFile *out);
  void	pdf_end_object(hdFile *out);

  void	encrypt_init(void);
  void	flate_open_stream(hdFile *out);
  void	flate_close_stream(hdFile *out);
  void	flate_puts(const char *s, hdFile *out);
  void	flate_printf(hdFile *out, const char *format, ...);
  void	flate_write(hdFile *out, char *inbuf, int length, int flush=0);	

  void	parse_contents(hdTree *t, float left, float width, float bottom,
		       float length, float *y, int *page, int *heading,
		       hdTree *chap);
  void	parse_doc(hdTree *t, float *left, float *right, float *bottom,
		  float *top, float *x, float *y, int *page,
		  hdTree *cpara, int *needspace);
  void	parse_heading(hdTree *t, float left, float width, float bottom,
		      float length, float *x, float *y, int *page,
		      int needspace);
  void	parse_paragraph(hdTree *t, float left, float width, float bottom,
		        float length, float *x, float *y, int *page,
			int needspace);
  void	parse_pre(hdTree *t, float left, float width, float bottom,
		  float length, float *x, float *y, int *page,
		  int needspace);
  void	parse_table(hdTree *t, float left, float width, float bottom,
		    float length, float *x, float *y, int *page,
		    int needspace);
  void	parse_list(hdTree *t, float *left, float *width, float *bottom,
		   float *length, float *x, float *y, int *page,
		   int needspace);
  void	init_list(hdTree *t);
  void	parse_comment(hdTree *t, float *left, float *width, float *bottom,
		              float *length, float *x, float *y, int *page,
			      hdTree *para, int needspace);

  hdTree	*real_prev(hdTree *t);
  hdTree	*real_next(hdTree *t);

  void	check_pages(int page);

  void	add_link(char *name, int page, int top);
  hdRenderLink	*find_link(char *name);
  int	compare_links(hdRenderLink *n1, hdRenderLink *n2);

  void	find_background(hdTree *t);
  void	write_background(int page, hdFile *out);

  hdRenderNode	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data,
			    int insert = 0);
  void	copy_tree(hdTree *parent, hdTree *t);
  float	get_cell_size(hdTree *t, float left, float right,
		              float *minwidth, float *prefwidth,
			      float *minheight);
  float	get_table_size(hdTree *t, float left, float right,
		               float *minwidth, float *prefwidth,
			       float *minheight);
  hdTree	*flatten_tree(hdTree *t);
  float	get_width(char *s, int typeface, int style, int size);
  void	update_image_size(hdTree *t);
  char	*get_title(hdTree *doc);
  hdFile	*open_file(void);
  void	set_color(hdFile *out, float *rgb);
  void	set_font(hdFile *out, int typeface, int style, float size);
  void	set_pos(hdFile *out, float x, float y);
  void	write_prolog(hdFile *out, int pages, char *author,
		             char *creator, char *copyright, char *keywords);
  void	ps_hex(hdFile *out, char *data, int length);
  void	ps_ascii85(hdFile *out, char *data, int length);
  void	jpg_init(j_compress_ptr cinfo);
  boolean	jpg_empty(j_compress_ptr cinfo);
  void	jpg_term(j_compress_ptr cinfo);
  void	jpg_setup(hdFile *out, hdImage *img, j_compress_ptr cinfo);
  int	compare_rgb(unsigned *rgb1, unsigned *rgb2);
  void	write_image(hdFile *out, hdRenderNode *r, int write_obj = 0);
  void	write_imagemask(hdFile *out, hdRenderNode *r);
  void	write_string(hdFile *out, char *s, int compress);
  void	write_text(hdFile *out, hdRenderNode *r);
  void	write_trailer(hdFile *out, int pages);
  int	write_truetype(hdFile *out, hdStyleFont *f);
};


#endif // !_HTMLDOC_RENDER_H_

//
// End of "$Id: render.h,v 1.1 2002/02/26 05:16:03 mike Exp $".
//
