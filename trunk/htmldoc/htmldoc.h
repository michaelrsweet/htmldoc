//
// "$Id: htmldoc.h,v 1.21 2000/11/06 19:53:03 mike Exp $"
//
//   Header file for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
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

#ifndef _HTMLDOC_H_
#  define _HTMLDOC_H_

//
// Include necessary headers.
//

#  include <stdio.h>
#  include <stdlib.h>
#  include <errno.h>
#  include <ctype.h>
#  include <time.h>
#  include <math.h>

#  include "debug.h"
#  include "html.h"
#  include "image.h"
#  include "md5.h"
#  include "progress.h"
#  include "rc4.h"

#  ifdef WIN32	    // Include all 8 million Windows header files...
#    include <windows.h>
#  endif // WIN32

#  include <zlib.h>

extern "C" {		/* Workaround for JPEG header problems... */
#  include <jpeglib.h>	/* JPEG/JFIF image definitions */
}


//
// Macro to get rid of "unreferenced variable xyz" warnings...
//

#  define REF(x)	(void)x;


//
// Constants...
//

enum			//// PDF page mode
{
  PDF_DOCUMENT,
  PDF_OUTLINE,
  PDF_FULLSCREEN
};

enum			//// PDF page layout
{
  PDF_SINGLE,
  PDF_ONE_COLUMN,
  PDF_TWO_COLUMN_LEFT,
  PDF_TWO_COLUMN_RIGHT
};

enum			//// PDF first page
{
  PDF_PAGE_1,
  PDF_TOC,
  PDF_CHAPTER_1
};

enum			//// PDF transition effect
{
  PDF_NONE,
  PDF_BOX_INWARD,
  PDF_BOX_OUTWARD,
  PDF_DISSOLVE,
  PDF_GLITTER_DOWN,
  PDF_GLITTER_DOWN_RIGHT,
  PDF_GLITTER_RIGHT,
  PDF_HORIZONTAL_BLINDS,
  PDF_HORIZONTAL_SWEEP_INWARD,
  PDF_HORIZONTAL_SWEEP_OUTWARD,
  PDF_VERTICAL_BLINDS,
  PDF_VERTICAL_SWEEP_INWARD,
  PDF_VERTICAL_SWEEP_OUTWARD,
  PDF_WIPE_DOWN,
  PDF_WIPE_LEFT,
  PDF_WIPE_RIGHT,
  PDF_WIPE_UP
};

enum			//// PDF document permissions
{
  PDF_PERM_PRINT = 4,
  PDF_PERM_MODIFY = 8,
  PDF_PERM_COPY = 16,
  PDF_PERM_ANNOTATE = 32
};

enum HDparam		//// HTMLDOC parameters...
{
  HD_BASE_SIZE,
  HD_BODY_COLOR,
  HD_BODY_FONT,
  HD_BODY_IMAGE,
  HD_BROWSER_WIDTH,
  HD_CHARSET,
  HD_COMPRESSION,
  HD_DATADIR,
  HD_ENCRYPTION,
  HD_ERRORS,
  HD_FOOTER,
  HD_HEADER,
  HD_HEADING_FONT,
  HD_HEAD_FOOT_SIZE,
  HD_HEAD_FOOT_STYLE,
  HD_HEAD_FOOT_TYPE,
  HD_HELPDIR,
  HD_HTML_EDITOR,
  HD_LANDSCAPE,
  HD_LINE_SPACING,
  HD_LINK_COLOR,
  HD_LINK_STYLE,
  HD_LOGO_IMAGE,
  HD_OUTPUT_BOOK,
  HD_OUTPUT_COLOR,
  HD_OUTPUT_FILES,
  HD_OUTPUT_FORMAT,
  HD_OUTPUT_JPEG,
  HD_OUTPUT_PATH,
  HD_OWNER_PASSWORD,
  HD_PAGE_BOTTOM,
  HD_PAGE_DUPLEX,
  HD_PAGE_LEFT,
  HD_PAGE_LENGTH,
  HD_PAGE_PRINT_LENGTH,
  HD_PAGE_PRINT_WIDTH,
  HD_PAGE_RIGHT,
  HD_PAGE_TOP,
  HD_PAGE_WIDTH,
  HD_PATH,
  HD_PDF_EFFECT,
  HD_PDF_EFFECT_DURATION,
  HD_PDF_FIRST_PAGE,
  HD_PDF_PAGE_DURATION,
  HD_PDF_PAGE_LAYOUT,
  HD_PDF_PAGE_MODE,
  HD_PDF_VERSION,
  HD_PERMISSIONS,
  HD_PS_COMMANDS,
  HD_PS_LEVEL,
  HD_TEXT_COLOR,
  HD_TITLE_FILE,
  HD_TITLE_PAGE,
  HD_TOC_DOC_COUNT,
  HD_TOC_FOOTER,
  HD_TOC_HEADER,
  HD_TOC_LEVELS,
  HD_TOC_LINKS,
  HD_TOC_NUMBERS,
  HD_TOC_TITLE,
  HD_USER_PASSWORD,
  HD_VERBOSITY
};

enum			//// Render types
{
  RENDER_TEXT,
  RENDER_IMAGE,
  RENDER_BOX,
  RENDER_FBOX,
  RENDER_LINK
};

enum			//// Output types
{
  OUTPUT_HTML,
  OUTPUT_PS,
  OUTPUT_PDF
};


//
// Structures...
//

struct HDrender			//// Render entity structure
{
  HDrender	*next;		// Next rendering entity
  int		type;		// Type of entity
  float		x,		// Position in points
		y,		// ...
		width,		// Size in points
		height;		// ...
  union
  {
    struct
    {
      int	typeface,	// Typeface for text
		style;		// Style of text
      float	size;		// Size of text in points
      float	rgb[3];		// Color of text
      uchar	buffer[1];	// String buffer
    }   	text;
    HDimage	*image;		// Image pointer
    float	box[3];		// Box color
    float	fbox[3];	// Filled box color
    uchar	link[1];	// Link URL
  }		data;
};

struct HDlink			//// Named link position structure
{
  short		page,		// Page #
		top;		// Top position
  uchar		*filename,	// File for link
		name[124];	// Reference name
};


//
// HTMLDOC class...
//

class HTMLDOC
{
  FILE		*out_;			// Output file

  HDtree	*doc_,			// Document tree
		*toc_;			// Table of contents tree

  int		num_files_,		// Number of files in document
		alloc_files_;		// Allocated files
  char		**files_;		// Filenames

  int		in_title_page_;		// Formatting title page?

  int		chapter_,		// Current chapter
		num_chapters_;		// Number of chapters
  int		chapter_starts_[MAX_CHAPTERS],
		chapter_ends_[MAX_CHAPTERS];
					// Pages for chapters

  int		num_headings_,		// Number of headings
		alloc_headings_,	// Allocated headings
		*heading_pages_,	// Heading pages
		*heading_tops_;		// Heading tops

  int		num_pages_,		// Number of pages
		alloc_pages_;		// Allocated pages
  HDrender	**pages_,		// Page data
		**endpages_;		// End of page data
  uchar		**page_chapters_,	// Current chapter heading
		**page_headings_;	// Current heading for each page
  HDtree	*current_heading_;	// Current heading this page

  int		num_links_,		// Number of links
		alloc_links_;		// Allocated links
  HDlink	*links_;		// Link data

  uchar		list_types_[16];	// Type of lists
  int		list_values_[16];	// Value of lists

  char		tempfile_[1024];	// Temporary file for PDF output
  int		num_objects_,		// Number of PDF objects
		alloc_objects_,		// Allocated PDF objects
		**objects_,		// PDF objects
		root_object_,		// Root object in PDF file
		info_object_,		// Info object in PDF file
		outline_object_,	// Outline object in PDF file
		pages_object_,		// Root page object in PDF file
		names_object_,		// Names object in PDF file
		encrypt_object_,	// Encryption object in PDF file
		*annots_objects_,	// Annotation objects in PDF file
		background_object_,	// Background image object
		font_objects_[16];	// Font objects
  HDimage	*logo_image_;		// Logo image
  float		logo_width_,		// Printed logo width
		logo_height_;		// Printed logo height
  HDimage	*title_image_,		// Title image
		*body_image_;		// Background image
  float		body_rgb_[3],		// Background color
		text_rgb_[3],		// Text color
		link_rgb_[3];		// Link color

  int		render_typeface_,	// Current text typeface
		render_style_;		// Current text style
  float		render_size_,		// Current text size
		render_rgb_[3],		// Current color
		render_x_,		// Current X position
		render_y_,		// Current Y position
		render_startx_;		// Current starting X position

  z_stream	compressor_;		// Compressor for Flate compression
  uchar		comp_buffer_[64 * 1024];// Compression buffer
  uchar		encrypt_key_[16];	// Encryption key
  rc4_context_t	encrypt_state_;		// Encryption context
  md5_byte_t	file_id_[16];		// File ID

  uchar		jpeg_buffer_[8192];	// JPEG buffer
  jpeg_destination_mgr	jpeg_dest_;	// JPEG destination manager
  struct jpeg_error_mgr	jpeg_err_;	// JPEG error handler

  int		verbosity_;		// Verbosity
  int		errors_;		// Number of errors
  int		compression_;		// Non-zero means compress PDFs
  int		title_page_,		// Need a title page
		toc_levels_,		// Number of table-of-contents levels
		toc_links_,		// Generate links
		toc_numbers_;		// Generate heading numbers
  int		output_book_;		// Output a "book"
  char		output_path_[1024];	// Output directory/name
  int		output_format_,		// Output format (HTML/PS/PDF)
		output_files_,		// Generate multiple files?
		output_color_;		// Output color images
  int		output_jpeg_;		// JPEG compress images?
  float		pdf_version_;		// Version of PDF to support
  int		pdf_page_mode_,		// PageMode attribute
		pdf_page_layout_,	// PageLayout attribute
		pdf_first_page_,	// First page
		pdf_effect_;		// Page transition effect
  float		pdf_effect_duration_,	// Page effect duration
		pdf_page_duration_;	// Page duration
  int		encryption_,		// Encrypt the PDF file?
		permissions_;		// File permissions?
  char		owner_password_[33],	// Owner password
		user_password_[33];	// User password
  int		ps_level_,		// Language level (0 for PDF)
		ps_commands_;		// Output PostScript commands?
  int		page_width_,		// Page width in points
		page_length_,		// Page length in points
		page_left_,		// Left margin
		page_right_,		// Right margin
		page_top_,		// Top margin
		page_bottom_,		// Bottom margin
		page_print_width_,	// Printable width
		page_print_length_,	// Printable length
		page_duplex_,		// Adjust margins/pages for duplexing?
		landscape_;		// Landscape orientation?

  char		char_set_[128];		// Character set
  HDtypeface	body_font_,		// Body typeface
		heading_font_;		// Heading typeface
  float		base_size_,		// Base font size
		line_spacing_;		// Line spacing
  HDtypeface	head_foot_type_;	// Typeface for header & footer
  HDstyle	head_foot_style_;	// Type style
  float		head_foot_size_;	// Size of header & footer

  char		header_[4],		// Header for regular pages
		toc_header_[4],		// Header for TOC pages
		footer_[4],		// Regular page footer
		toc_footer_[4],		// Footer for TOC pages
		toc_title_[1024];	// TOC title string

  char		title_file_[1024],	// Title page image file
		logo_file_[1024],	// Logo image file
		body_color_[255],	// Body color
		body_file_[1024],	// Body image file
		text_color_[255],	// Text color
		link_color_[255];	// Link color
  int		link_style_;		// 1 = underline_, 0 = plain
  int		browser_width_;		// Browser width


  void		add_link(uchar *name, int page, int top,
		         uchar *filename = (uchar *)0);
  void		add_link(uchar *name, uchar *filename) { add_link(name, 0, 0, filename); }
  static int	compare_links(HDlink *n1, HDlink *n2);
  int		compare_rgb(uchar *rgb1, uchar *rgb2);
  void		encrypt_init(void);
  void		find_background(HDtree *t);
  HDlink	*find_link(uchar *name);
  void		flate_open();
  void		flate_close();
  void		flate_puts(char *s);
  void		flate_printf(char *format, ...);
  void		flate_write(uchar *inbuf, int length, int flush=0);	
  uchar		*get_title(HDtree *t);
  float		get_width(uchar *s, int typeface, int style, int size);
  int		html_write_all(HDtree *t, int col);
  void		html_write_document(uchar *title, uchar *author, uchar *creator,
		                    uchar *copyright, uchar *keywords);
  void		html_write_footer(HDtree *t);
  void		html_write_header(uchar *filename, uchar *title,
		                  uchar *author, uchar *copyright,
				  uchar *docnumber, HDtree *t);
  void		html_write_title(uchar *title, uchar *author,
			         uchar *copyright, uchar *docnumber);
  void		init_list(HDtree *t);
  boolean	jpg_empty(j_compress_ptr cinfo);
  void		jpg_init(j_compress_ptr cinfo);
  void		jpg_setup(HDimage *img, j_compress_ptr cinfo);
  void		jpg_term(j_compress_ptr cinfo);
  HDrender	*new_render(int page, int type, float x, float y,
		            float width, float height, void *data,
			    int insert = 0);
  FILE		*open_file(void);
  void		parse_comment(HDtree *t, float left, float width, float bottom,
		              float length, float *x, float *y, int *page,
			      HDtree *para, int needspace);
  void		parse_contents(HDtree *t, float left, float width, float bottom,
		               float length, float *y, int *page, int *heading);
  void		parse_doc(HDtree *t, float left, float width, float bottom, float length,
		          float *x, float *y, int *page, HDtree *cpara,
			  int *needspace);
  void		parse_heading(HDtree *t, float left, float width, float bottom,
		              float length, float *x, float *y, int *page,
			      int needspace);
  void		parse_paragraph(HDtree *t, float left, float width, float bottom,
		                float length, float *x, float *y, int *page,
			        int needspace);
  void		parse_pre(HDtree *t, float left, float width, float bottom,
		          float length, float *x, float *y, int *page,
			  int needspace);
  void		parse_table(HDtree *t, float left, float width, float bottom,
		            float length, float *x, float *y, int *page,
			    int needspace);
  void		parse_list(HDtree *t, float left, float width, float bottom,
		           float length, float *x, float *y, int *page,
			   int needspace);
  int		pdf_count_headings(HDtree *toc);
  void		pdf_write_contents(HDtree *toc, int parent,
		                   int prev, int next, int *heading);
  void		pdf_write_document(uchar *title, uchar *author, uchar *creator,
		                   uchar *copyright, uchar *keywords);
  void		pdf_write_images();
  void		pdf_write_links();
  void		pdf_write_names();
  void		pdf_write_page(int page, uchar *title,
		               float title_width, uchar **page_chapter,
			       uchar **page_heading);
  void		pdf_write_prolog(int pages, uchar *title,
		                 uchar *author, uchar *creator,
				 uchar *copyright, uchar *keywords);
  void		pdf_write_resources(int page);
  void		prepare_heading(int page, int print_page, uchar *title,
		        	float title_width, uchar *chapter,
				float chapter_width, uchar *heading,
				float heading_width, char *format, int y);
  char		*prepare_page(int page, int *file_page, uchar *title,
		              float title_width, uchar **page_chapter,
			      uchar **page_heading);
  void		ps_ascii85(uchar *data, int length);
  void		ps_hex(uchar *data, int length);
  void		ps_write_background();
  void		ps_write_document(uchar *title, uchar *author, uchar *creator,
		                  uchar *copyright, uchar *keywords);
  void		ps_write_page(int page, uchar *title,
		              float title_width, uchar **page_chapter,
			      uchar **page_heading);
  void		ps_write_prolog(int pages, uchar *title,
		                uchar *author, uchar *creator,
			        uchar *copyright, uchar *keywords);
  void		scan_links(HDtree *t, uchar *filename);
  void		set_color(float *rgb);
  void		set_font(int typeface, int style, float size);
  void		set_pos(float x, float y);
  void		update_links(HDtree *t, uchar *filename);
  void		write_image(HDrender *r);
  void		write_string(uchar *s, int compress);
  void		write_text(HDrender *r);
  void		write_trailer(int pages);

  public:

  HTMLDOC();
  ~HTMLDOC();

  void		add_file(const char *filename, int n=-1);
  void		build_toc();
  void		clear();
  void		clear_doc();
  void		clear_files();
  void		clear_toc();
  void		delete_file(int n);
  const char	*file(int n) { return (files_[n]); }
  void		format();
  int		get_integer(HDparam param);
  float		get_float(HDparam param);
  const char	*get_string(HDparam param);
  int		load_book(const char *filename);
  void		load_files();
  void		load_prefs();
  int		num_files() { return (num_files_); }
  int		main(int argc, char *argv[]);
  void		set_integer(HDparam param, int value);
  void		set_float(HDparam param, float value);
  void		set_page_size(const char *size);
  void		set_string(HDparam param, const char *value);
  int		save_book(const char *filename);
  void		save_prefs();
  int		write_html();
  int		write_pdf();
  int		write_ps();

  static char	*format_number(int n, char f);
  static void	get_color(const uchar *c, float *rgb, int defblack = 1);
  static int	get_measurement(const char *s);

  static char		help_dir[1024];		// Directory for help files
  static char		html_editor[1024];	// HTML editor
  static char		path[2048];		// Search path
  static HDprogress	*progress;		// Progress reporting class
};


//
// Macros to access new HTMLDOC class members instead of old globals...
//

#if 0
#  define Verbosity		verbosity_
#  define Errors		errors_
#  define Compression		compression_
#  define TitlePage		title_page_
#  define TocLevels		toc_levels_
#  define TocLinks		toc_links_
#  define TocNumbers		toc_numbers_
#  define TocDocCount		toc_doc_count_
#  define OutputBook		output_book_
#  define OutputPath		output_path_
#  define OutputFiles		output_files_
#  define OutputColor		output_color_
#  define OutputJPEG		output_jpeg_
#  define PDFVersion		pdf_version_
#  define PDFPageMode		pdf_page_mode_
#  define PDFPageLayout		pdf_page_layout_
#  define PDFFirstPage		pdf_first_page_
#  define PDFEffect		pdf_effect_
#  define PDFEffectDuration	pdf_effect_duration_
#  define PDFPageDuration	pdf_page_duration_
#  define Encryption		encryption_
#  define Permissions		permissions_
#  define OwnerPassword		owner_password_
#  define UserPassword		user_password_
#  define PSLevel		ps_level_
#  define PSCommands		ps_commands_
#  define PageWidth		page_width_
#  define PageLength		page_length_
#  define PageLeft		page_left_
#  define PageRight		page_right_
#  define PageTop		page_top_
#  define PageBottom		page_bottom_
#  define PagePrintWidth	page_print_width_
#  define PagePrintLength	page_print_length_
#  define PageDuplex		page_duplex_
#  define Landscape		landscape_
#  define HeadFootType		head_foot_type_
#  define HeadFootStyle		head_foot_style_
#  define HeadFootSize		head_foot_size_
#  define Header		header_
#  define TocHeader		toc_header_
#  define Footer		footer_
#  define TocFooter		toc_footer_
#  define TocTitle		toc_title_
#  define TitleImage		title_image_
#  define LogoImage		logo_image_
#  define BodyColor		body_color_
#  define BodyImage		body_image_
#  define LinkColor		link_color_
#  define LinkStyle		link_style_
#  define Path			path_
#  define HTMLEditor		html_editor_
#endif // 0

#endif // !_HTMLDOC_H_

//
// End of "$Id: htmldoc.h,v 1.21 2000/11/06 19:53:03 mike Exp $".
//
