//
// "$Id: ps-pdf.cxx,v 1.90 2000/10/16 03:25:08 mike Exp $"
//
//   Common PostScript + PDF output routines for HTMLDOC, a HTML document
//   processing program.
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
// Contents:
//
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "htmldoc.h"

#ifdef MAC		// MacOS-specific header file...
#  include <Files.h>
#endif // MAC

#if defined(WIN32) || defined(__EMX__)
#  include <io.h>
#else
#  include <unistd.h>
#endif // WIN32 || __EMX__

#include <fcntl.h>


//
// 'HTMLDOC::pspdf_export()' - Export PostScript/PDF file(s)...
//

int
HTMLDOC::pspdf_export(HDtree *document,	// I - Document to export
             HDtree *toc)	// I - Table of contents for document
{
  uchar		*title,		// Title text
		*author,	// Author of document
		*creator,	// HTML file creator (Netscape, etc)
		*copyright,	// File copyright
		*docnumber,	// Document number
		*keywords;	// Search keywords
  HDtree	*t;		// Title page document tree
  FILE		*fp;		// Title page file
  float		x, y,		// Current page position
		width,		// Width of title, author, etc
		height;		// Height of title area
  int		page,		// Current page #
		heading;	// Current heading #
  float		top, bottom;	// Top and bottom margins...
  HDtree	*timage;	// Title image
  float		timage_width,	// Title image width
		timage_height;	// Title image height
  HDrender	*r;		// Rendering structure...
  float		rgb[3];		// Text color
  int		needspace;	// Need whitespace


  // Figure out the printable area of the output page...
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

  // Get the document title, author, etc...
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

  // Initialize page rendering variables...
  memset(pages, 0, sizeof(pages));
  memset(page_chapters, 0, sizeof(page_chapters));
  memset(page_headings, 0, sizeof(page_headings));
  memset(endpages, 0, sizeof(pages));
  memset(list_types, 0267, sizeof(list_types));
  memset(list_values, 0, sizeof(list_values));
  memset(chapter_starts, -1, sizeof(chapter_starts));
  memset(chapter_ends, -1, sizeof(chapter_starts));

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
      // Create a standard title page...
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

  // Parse the document...
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

  // Parse the table-of-contents if necessary...
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

  // Do we have any pages?
  if (num_pages > 0)
  {
    // Yes, write the document to disk...
    if (PSLevel > 0)
      ps_write_document(title, author, creator, copyright, keywords);
    else
      pdf_write_document(title, author, creator, copyright, keywords, toc);
  }
  else
  {
    // No, show an error...
    progress_error("Error: no pages generated! (did you remember to use webpage mode?");
  }

  // Free memory...
  if (title != NULL)
    free(title);

  for (int i = 0; i < num_pages; i ++)
  {
    if (i == 0 || page_headings[i] != page_headings[i - 1])
      free(page_headings[i]);
  }

  return (0);
}


//
// 'HTMLDOC::pspdf_prepare_page()' - Add headers/footers to page before writing...
//

char *
HTMLDOC::pspdf_prepare_page(int   page,			// I - Page number
                   int   *file_page,		// O - File page number
        	   uchar *title,		// I - Title string
        	   float title_width,		// I - Width of title string
                   uchar **page_heading,	// IO - Page heading string
	           uchar **page_chapter)	// IO - Page chapter string
{
  int		print_page;			// Printed page #
  float		chapter_width,			// Width of page chapter
		heading_width;			// Width of page heading
  char		*page_text;			// Page number text


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

  // Get the new heading if necessary...
  if (page_chapters[page] != NULL)
    *page_chapter = page_chapters[page];
  if (page_headings[page] != NULL)
    *page_heading = page_headings[page];

  // Make a page number; use roman numerals for the table of contents
  // and arabic numbers for all others...
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

  // Add page headings...
  chapter_width = get_width(*page_chapter, HeadFootType, HeadFootStyle, SIZE_P) *
                  HeadFootSize / _htmlSizes[SIZE_P];
  heading_width = get_width(*page_heading, HeadFootType, HeadFootStyle, SIZE_P) *
                  HeadFootSize / _htmlSizes[SIZE_P];

  if (chapter == 0)
  {
    // Add table-of-contents header & footer...
    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading, heading_width, TocHeader,
			  PagePrintLength);
    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading,  heading_width, TocFooter, 0);
  }
  else if (chapter > 0)
  {
    // Add chapter header & footer...
    if (page > chapter_starts[chapter] || !OutputBook)
      pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                            chapter_width, *page_heading, heading_width, Header,
			    PagePrintLength);
    pspdf_prepare_heading(page, print_page, title, title_width, *page_chapter,
                          chapter_width, *page_heading, heading_width, Footer, 0);
  }

  return (page_text);
}


//
// 'HTMLDOC::pspdf_prepare_heading()' - Add headers/footers to page before writing...
//

void
HTMLDOC::pspdf_prepare_heading(int   page,		// I - Page number
                      int   print_page,         // I - Printed page number
        	      uchar *title,		// I - Title string
        	      float title_width,	// I - Width of title string
        	      uchar *chapter,		// I - Page chapter string
		      float chapter_width,	// I - Width of chapter
        	      uchar *heading,		// I - Page heading string
		      float heading_width,	// I - Width of heading
		      char  *format,		// I - Format of heading
		      int   y)			// I - Baseline of heading
{
  int		pos,		// Position in heading
		dir;		// Direction of page
  char		*number;	// Page number
  HDrender	*temp;		// Render structure for titles, etc.


  DEBUG_printf(("pspdf_prepare_heading(%d, %d, \"%s\", %.1f, \"%s\", %.1f, \"%s\", %d)\n",
                page, print_page, title ? title : "(null)", title_width,
		heading ? heading : "(null)", heading_width, format, y));

  // Return right away if there is nothing to do...
  if (strncmp(format, "...", 3) == 0)
    return;

  // Add page headings...
  if (PageDuplex && (page & 1))
  {
    dir    = -1;
    format += 2;
  }
  else
    dir = 1;

  for (pos = 0; pos < 3; pos ++, format += dir)
  {
    // Add the appropriate object...
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

    // Justify the object...
    switch (pos)
    {
      case 0 : // Left justified
          break;
      case 1 : // Centered
          temp->x = (PagePrintWidth - temp->width) * 0.5;
          break;
      case 2 : // Right justified
          temp->x = PagePrintWidth - temp->width;
          break;
    }

    // Set the text font and color...
    if (temp->type == RENDER_TEXT)
    {
      temp->data.text.typeface = HeadFootType;
      temp->data.text.style    = HeadFootStyle;
      temp->data.text.size     = HeadFootSize;

      get_color(_htmlTextColor, temp->data.text.rgb);
    }
  }
}


//
// 'HTMLDOC::write_background()' - Write the background image/color for to the current
//                        page.
//

void
HTMLDOC::write_background(FILE *out)	// I - File to write to
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


//
// 'HTMLDOC::open_file()' - Open an output file for the current chapter.
//

FILE *		// O - File pointer
HTMLDOC::open_file(void)
{
  char	filename[255];	// Filename


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


//
// 'HTMLDOC::set_color()' - Set the current text color...
//

void
HTMLDOC::set_color(FILE  *out,	// I - File to write to
          float *rgb)	// I - RGB color
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


//
// 'HTMLDOC::set_font()' - Set the current text font.
//

void
HTMLDOC::set_font(FILE  *out,		// I - File to write to
         int   typeface,	// I - Typeface code
         int   style,		// I - Style code
         float size)		// I - Size
{
  char	sizes[255],	// Formatted string for size...
	*s;		// Pointer to end of string


  if (typeface == HDrenderypeface &&
      style == render_style &&
      size == render_size)
    return;

  // Format size and strip trailing 0's and decimals...
  sprintf(sizes, "%.1f", size);

  for (s = sizes + strlen(sizes) - 1; s > sizes && *s == '0'; s --)
    *s = '\0';

  if (*s == '.')
    *s = '\0';

  // Set the new typeface, style, and size.
  HDrenderypeface = typeface;
  render_style    = style;
  render_size     = size;

  if (PSLevel > 0)
    fprintf(out, "%s/F%x SF ", sizes, typeface * 4 + style);
  else
    flate_printf(out, "/F%x %s Tf ", typeface * 4 + style, sizes);
}


//
// 'HTMLDOC::set_pos()' - Set the current text position.
//

void
HTMLDOC::set_pos(FILE  *out,	// I - File to write to
        float x,	// I - X position
        float y)	// I - Y position
{
  char	xs[255],	// Formatted string for X...
	ys[255],	// Formatted string for Y...
	*s;		// Pointer to end of string


  if (fabs(render_x - x) < 0.1 && fabs(render_y - y) < 0.1)
    return;

  // Format X and Y...
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

  // Strip trailing 0's and decimals...
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


//
// 'HTMLDOC::ps_hex()' - Print binary data as a series of hexadecimal numbers.
//

void
HTMLDOC::ps_hex(FILE  *out,	// I - File to print to
       uchar *data,	// I - Data to print
       int   length)	// I - Number of bytes to print
{
  int		col;
  static char	*hex = "0123456789ABCDEF";


  col = 0;
  while (length > 0)
  {
    // Put the hex uchars out to the file; note that we don't use fprintf()
    // for speed reasons...
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


//
// 'HTMLDOC::ps_ascii85()' - Print binary data as a series of base-85 numbers.
//

void
HTMLDOC::ps_ascii85(FILE  *out,		// I - File to print to
	   uchar *data,		// I - Data to print
	   int   length)	// I - Number of bytes to print
{
  int		col;		// Column
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


//
// 'HTMLDOC::compare_rgb()' - Compare two RGB colors...
//

int			// O - -1 if rgb1<rgb2, etc.
HTMLDOC::compare_rgb(uchar *rgb1,	// I - First color
            uchar *rgb2)	// I - Second color
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


//
// 'HTMLDOC::write_image()' - Write an image to the given output file...
//

void
HTMLDOC::write_image(FILE     *out,	// I - Output file
            HDrender *r)	// I - Image to write
{
  int		i, j, k, m,	// Looping vars
		ncolors;	// Number of colors
  uchar		*pixel,		// Current pixel
		*indices,	// New indexed pixel array
		*indptr;	// Current index
  int		indwidth,	// Width of indexed line
		indbits;	// Bits per index
  uchar		colors[256][3],	// Colormap values
		grays[256],	// Grayscale usage
		*match;		// Matching color value
  HDtree 	*img;		// Image
  struct jpeg_compress_struct cinfo;	// JPEG compressor


  // See if we can optimize the image as indexed without color loss...
  img     = r->data.image;
  ncolors = 0;

  DEBUG_printf(("img->filename = %s\n", img->filename));
  DEBUG_printf(("img->width = %d, ->height = %d, ->depth = %d\n", img->width,
                img->height, img->depth));

  if (PSLevel != 1 && PDFVersion >= 1.2)
  {
    if (img->depth == 1)
    {
      // Greyscale image...
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
      // Color image...
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
      // Convert a grayscale image...
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
      // Convert a color image...
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

  // Now write the image...
  switch (PSLevel)
  {
    case 0 : // PDF
	flate_printf(out, "q %.1f 0 0 %.1f %.1f %.1f cm\n", r->width, r->height,
	           r->x, r->y);
        flate_puts("BI", out);

	if (ncolors > 0)
	{
	  if (ncolors <= 2)
	    ncolors = 2; // Adobe doesn't like 1 color images...

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

    case 1 : // PostScript, Level 1
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
    case 3 : // PostScript, Level 3
        // Fallthrough to Level 2 output if compression is disabled...
        if (Compression && (!OutputJPEG || ncolors > 0))
	{
          fputs("GS", out);
	  fprintf(out, "[%.1f 0 0 %.1f %.1f %.1f]CM", r->width, r->height,
	          r->x, r->y);

          if (ncolors > 0)
          {
	    if (ncolors <= 2)
	      ncolors = 2; // Adobe doesn't like 1 color images...

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

    case 2 : // PostScript, Level 2
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


//
// 'HTMLDOC::write_string()' - Write a text entity.
//

void
HTMLDOC::write_string(FILE  *out,	// I - Output file
             uchar *s,		// I - String
	     int   compress)	// I - Compress output?
{
  int	i;			// Looping var


  if (Encryption && !compress && PSLevel == 0)
  {
    int		len;		// Length of string
    uchar	news[1024];	// New string


    // Write an encrypted string...
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
      if (*s == 160) // &nbsp;
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


//
// 'HTMLDOC::write_text()' - Write a text entity.
//

void
HTMLDOC::write_text(FILE     *out,	// I - Output file
           HDrender *r)		// I - Text entity
{
  uchar	*ptr;			// Pointer into text


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


//
// End of "$Id: ps-pdf.cxx,v 1.90 2000/10/16 03:25:08 mike Exp $".
//
