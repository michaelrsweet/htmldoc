//
// "$Id: ps.cxx,v 1.1 2000/10/16 03:25:09 mike Exp $"
//
//   PostScript output routines for HTMLDOC, a HTML document processing
//   program.
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


//
// 'HTMLDOC::ps_write_document()' - Write all render entities to PostScript file(s).
//

void
HTMLDOC::ps_write_document(uchar *title,		// I - Title on all pages
        	  uchar *author,	// I - Author of document
        	  uchar *creator,	// I - Application that generated the HTML file
        	  uchar *copyright,	// I - Copyright (if any) on the document
                  uchar *keywords)	// I - Search keywords
{
  uchar		*page_chapter,	// Current chapter text
		*page_heading;	// Current heading text
  FILE		*out;		// Output file
  int		page;		// Current page #
  float		title_width;	// Width of title string


  // Get the document title width...
  if (title != NULL)
    title_width = HeadFootSize / _htmlSizes[SIZE_P] *
                  get_width(title, HeadFootType, HeadFootStyle, SIZE_P);

  // Write the title page(s)...
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

    // Close the output file as necessary...
    if (OutputFiles)
    {
      write_trailer(out, 0);
      fclose(out);
    }
  }

  // Close the output file as necessary...
  if (!OutputFiles)
  {
    write_trailer(out, 0);
    if (out != stdout)
      fclose(out);
  }

  if (Verbosity)
    progress_hide();
}


//
// 'HTMLDOC::ps_write_page()' - Write all render entities on a page to a PostScript file.
//

void
HTMLDOC::ps_write_page(FILE  *out,		// I - Output file
              int   page,		// I - Page number
              uchar *title,		// I - Title string
              float title_width,	// I - Width of title string
              uchar **page_heading,	// IO - Page heading string
	      uchar **page_chapter)	// IO - Page chapter string
{
  int		file_page;	// Current page # in document
  char		*page_text;	// Page number text
  HDrender	*r,		// Render pointer
		*next;		// Next render


  if (page < 0 || page >= MAX_PAGES)
    return;

  DEBUG_printf(("ps_write_page(%08x, %d, \"%s\", %.1f, \"%s\")\n",
                out, page, title ? title : "(null)", title_width,
		*page_heading ? *page_heading : "(null)"));

  // Add headers/footers as needed...
  page_text = pspdf_prepare_page(page, &file_page, title, title_width,
                                 page_chapter, page_heading);

  // Clear the render cache...
  HDrenderypeface = -1;
  render_style    = -1;
  render_size     = -1;
  render_rgb[0]   = 0.0;
  render_rgb[1]   = 0.0;
  render_rgb[2]   = 0.0;
  render_x        = -1.0;
  render_y        = -1.0;

  // Output the page prolog...
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

  // Render all page elements, freeing used memory as we go...
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

  // Output the page trailer...
  fputs("GR\n", out);
  fputs("SP\n", out);
}


//
// 'HTMLDOC::ps_write_background()' - Write a background image...
//

void
HTMLDOC::ps_write_background(FILE *out)		// I - Output file
{
  int	y,				// Current line
	pwidth;				// Pixel width


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


//
// End of "$Id: ps.cxx,v 1.1 2000/10/16 03:25:09 mike Exp $".
//
