//
// "$Id: render-table.cxx,v 1.1 2002/03/10 03:17:24 mike Exp $"
//
//   Core rendering methods for HTMLDOC, a HTML document processing
//   program.
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
// Contents:
//
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "htmldoc.h"
#include <config.h>


//
// Timezone offset for dates, below...
//

#ifdef HAVE_TM_GMTOFF
#  define timezone (doc_date->tm_gmtoff)
#endif // HAVE_TM_GMTOFF


#if 0

//
 * 'pspdf_export()' - Export PostScript/PDF file(s)...


int
pspdf_export(tree_t *document,	// I - Document to export
             tree_t *toc)	// I - Table of contents for document
{
  const char	*title_file;	// Location of title image/file
  uchar		*author,	// Author of document
		*creator,	// HTML file creator (Netscape, etc)
		*copyright,	// File copyright
		*docnumber,	// Document number
		*keywords;	// Search keywords
  tree_t	*t;		// Title page document tree
  FILE		*fp;		// Title page file
  float		x, y,		// Current page position
		left, right,	// Left and right margins
		bottom, top,	// Bottom and top margins
		width,		// Width of , author, etc
		height;		// Height of  area
  int		pos,		// Current header/footer position
		page,		// Current page #
		heading,	// Current heading #
		toc_duplex,	// Duplex TOC pages?
		toc_landscape,	// Do TOC in landscape?
		toc_width,	// Width of TOC pages
		toc_length,	// Length of TOC pages
		toc_left,	// TOC page margins
		toc_right,
		toc_bottom,
		toc_top;
  image_t	*timage;	// Title image
  float		timage_width,	// Title image width
		timage_height;	// Title image height
  render_t	*r;		// Rendering structure...
  float		rgb[3];		// Text color
  int		needspace;	// Need whitespace


 //
  * Figure out the printable area of the output page...
 

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

 //
  * Get the document title, author, etc...
 

  doc_title  = get_title(document);
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

 //
  * Initialize page rendering variables...
 

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
      // Find the title file...
      if ((title_file = file_find(Path, TitleImage)) == NULL)
        return (1);

      // Write a title page from HTML source...
      if ((fp = fopen(title_file, "rb")) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to open title file \"%s\" - %s!",
                       TitleImage, strerror(errno));
	return (1);
      }

      t = htmlReadFile(NULL, fp, file_directory(TitleImage));
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
     //
      * Create a standard title page...
     

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
      strcpy((char *)pages[page].page_text, (page & 1) ? "eltit" : "title");
  }
  else
    page = 0;

 //
  * Parse the document...
 

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
  for (pos = 0; pos < 3; pos ++)
    if (Header[pos])
      break;

  if (pos == 3)
    top = PagePrintLength;
  else if (logo_height > HeadFootSize)
    top = PagePrintLength - logo_height - HeadFootSize;
  else
    top = PagePrintLength - 2 * HeadFootSize;

  // Adjust bottom margin as needed...
  for (pos = 0; pos < 3; pos ++)
    if (Footer[pos])
      break;

  if (pos == 3)
    bottom = 0.0f;
  else if (logo_height > HeadFootSize)
    bottom = logo_height + HeadFootSize;
  else
    bottom = 2 * HeadFootSize;

  y = top;

  parse_doc(document, &left, &right, &bottom, &top, &x, &y, &page, NULL,
            &needspace);

  if (PageDuplex && (num_pages & 1))
    check_pages(num_pages);
  chapter_ends[chapter] = num_pages - 1;

  for (chapter = 1; chapter <= TocDocCount; chapter ++)
    for (page = chapter_starts[chapter]; page <= chapter_ends[chapter]; page ++)
      pspdf_prepare_page(page);

 //
  * Parse the table-of-contents if necessary...
 

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
    else if (logo_height > HeadFootSize)
      top = PagePrintLength - logo_height - HeadFootSize;
    else
      top = PagePrintLength - 2 * HeadFootSize;

    // Adjust bottom margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (TocFooter[pos])
	break;

    if (pos == 3)
      bottom = 0.0f;
    else if (logo_height > HeadFootSize)
      bottom = logo_height + HeadFootSize;
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

 //
  * Do we have any pages?
 

  if (num_pages > 0 && TocDocCount > 0)
  {
   //
    * Yes, write the document to disk...
   

    progress_error(HD_ERROR_NONE, "PAGES: %d", num_pages);

    if (PSLevel > 0)
      ps_write_document(author, creator, copyright, keywords);
    else
      pdf_write_document(author, creator, copyright, keywords, toc);
  }
  else
  {
   //
    * No, show an error...
   

    progress_error(HD_ERROR_NO_PAGES,
                   "Error: no pages generated! (did you remember to use webpage mode?");
  }

 //
  * Free memory...
 

  if (doc_title != NULL)
    free(doc_title);

  if (alloc_links)
  {
    free(links);

    num_links    = 0;
    alloc_links  = 0;
    links        = NULL;
  }

  for (int i = 0; i < num_pages; i ++)
  {
    int j;

    if (!pages[i].heading)
      continue;

    if (i == 0 || pages[i].heading != pages[i - 1].heading)
      free(pages[i].heading);

    for (j = 0; j < 3; j ++)
    {
      if (!pages[i].header[j])
        continue;

      if (i == 0 || pages[i].header[j] != pages[i - 1].header[j])
        free(pages[i].header[j]);
    }

    for (j = 0; j < 3; j ++)
    {
      if (!pages[i].footer[j])
        continue;

      if (i == 0 || pages[i].footer[j] != pages[i - 1].footer[j])
        free(pages[i].footer[j]);
    }
  }

  if (alloc_pages)
  {
    free(pages);

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
 * 'pspdf_prepare_page()' - Add headers/footers to page before writing...


void
pspdf_prepare_page(int page)		// I - Page number
{
  int	print_page;			// Printed page #
  char	page_text[64];			// Page number text


  DEBUG_printf(("pspdf_prepare_page(%d)\n", page));

 //
  * Make a page number; use roman numerals for the table of contents
  * and arabic numbers for all others...
 

  if (chapter == 0 && OutputType == OUTPUT_BOOK)
  {
    print_page = page - chapter_starts[0] + 1;
    strncpy(page_text, format_number(print_page, 'i'), sizeof(page_text) - 1);
    page_text[sizeof(page_text) - 1] = '\0';
  }
  else if (chapter < 0)
  {
    print_page = 0;
    strcpy(page_text, (page & 1) ? (char *)"eltit" : (char *)"title");
  }
  else
  {
    print_page = page - chapter_starts[1] + 1;
    strncpy(page_text, format_number(print_page, '1'), sizeof(page_text) - 1);
    page_text[sizeof(page_text) - 1] = '\0';
  }

 //
  * Add page headings...
 

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

  if (chapter == 0)
  {
   //
    * Add table-of-contents header & footer...
   

    pspdf_prepare_heading(page, print_page, pages[page].header,
                          PagePrintLength, page_text, sizeof(page_text));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          page_text, sizeof(page_text));
  }
  else if (chapter > 0 && !title_page)
  {
   //
    * Add chapter header & footer...
   

    if (page > chapter_starts[chapter] || OutputType != OUTPUT_BOOK)
      pspdf_prepare_heading(page, print_page, pages[page].header,
                            PagePrintLength, page_text, sizeof(page_text));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          page_text, sizeof(page_text));
  }

 //
  * Copy the page number for the TOC...
 

  strncpy(pages[page].page_text, page_text, sizeof(pages[page].page_text) - 1);
  pages[page].page_text[sizeof(pages[page].page_text) - 1] = '\0';
}


//
 * 'pspdf_prepare_heading()' - Add headers/footers to page before writing...


static void
pspdf_prepare_heading(int   page,		// I - Page number
                      int   print_page,         // I - Printed page number
		      uchar **format,		// I - Page headings
		      int   y,			// I - Baseline of heading
		      char  *page_text,		// O - Page number text
		      int   page_len)		// I - Size of page text
{
  int		pos,		// Position in heading
		dir;		// Direction of page
  char		*number;	// Page number
  char		buffer[1024],	// String buffer
		*bufptr,	// Pointer into buffer
		*formatptr;	// Pointer into format string
  int		formatlen;	// Length of format command string
  render_t	*temp;		// Render structure for titles, etc.


  DEBUG_printf(("pspdf_prepare_heading(%d, %d, %p, %d, %p, %d)\n",
                page, print_page, format, y, page_text, page_len));

 //
  * Add page headings...
 

  if (PageDuplex && (page & 1))
  {
    dir    = -1;
    format += 2;
  }
  else
    dir = 1;

  for (pos = 0; pos < 3; pos ++, format += dir)
  {
   //
    * Add the appropriate object...
   

    if (!*format)
      continue;

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

            strncpy(bufptr, number, sizeof(buffer) - 1 - (bufptr - buffer));
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

            strncpy(bufptr, number, sizeof(buffer) - 1 - (bufptr - buffer));
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

            strncpy(bufptr, number, sizeof(buffer) - 1 - (bufptr - buffer));
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

            strncpy(bufptr, number, sizeof(buffer) - 1 - (bufptr - buffer));
	    bufptr += strlen(bufptr);
	  }
	  else if (formatlen == 5 && strncasecmp(formatptr, "TITLE", 5) == 0)
	  {
            formatptr += 5;
	    if (doc_title)
	    {
              strncpy(bufptr, (char *)doc_title,
	              sizeof(buffer) - 1 - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "CHAPTER", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].chapter)
	    {
              strncpy(bufptr, (char *)(pages[page].chapter),
	              sizeof(buffer) - 1 - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "HEADING", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].heading)
	    {
              strncpy(bufptr, (char *)(pages[page].heading),
	              sizeof(buffer) - 1 - (bufptr - buffer));
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
            strncpy(bufptr, formatptr - 1, sizeof(buffer) - 1 - (bufptr - buffer));
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
			          HeadFootStyle, SIZE_P),
	        	HeadFootSize, (uchar *)buffer);

      if (strstr((char *)*format, "$PAGE") ||
          strstr((char *)*format, "$CHAPTERPAGE"))
      {
        strncpy(page_text, buffer, page_len - 1);
	page_text[page_len - 1] = '\0';
      }
    }

    if (temp == NULL)
      continue;

   //
    * Justify the object...
   

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

   //
    * Set the text font and color...
   

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
 * 'render_contents()' - Render a single heading.


static void
render_contents(tree_t *t,		// I - Tree to parse
                float  left,		// I - Left margin
                float  right,		// I - Printable width
                float  bottom,		// I - Bottom margin
                float  top,		// I - Printable top
                float  *y,		// IO - Y position
                int    *page,		// IO - Page #
	        int    heading,		// I - Heading #
	        tree_t *chap)		// I - Chapter heading
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

 //
  * Put the text...
 

  flat = flatten_tree(t->child);

  for (height = 0.0, temp = flat; temp != NULL; temp = temp->next)
    if (temp->height > height)
      height = temp->height;

  height *= _htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P];

  x  = left + 36.0f * t->indent;
  *y -= height;

 //
  * Get the width of the page number, leave room for three dots...
 

  hpage = heading_pages[heading] + chapter_starts[1] - 1;

  if (heading >= 0)
    numberwidth = get_width((uchar *)pages[hpage].page_text,
                            t->typeface, t->style, t->size) +
	          3.0f * dot_width;
  else
    numberwidth = 0.0f;

  for (temp = flat; temp != NULL; temp = next)
  {
    rgb[0] = temp->red / 255.0f;
    rgb[1] = temp->green / 255.0f;
    rgb[2] = temp->blue / 255.0f;

    if ((x + temp->width) >= (right - numberwidth))
    {
     //
      * Too wide to fit, continue on the next line
     

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

     //
      * Add a page link...
     

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
           //
            * Add a target link...
           

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
   //
    * Draw dots leading up to the page number...
   

    width = numberwidth - 3.0 * dot_width + x;

    for (nptr = number;
         nptr < (number + sizeof(number) - 1) && width < right;
	 width += dot_width)
      *nptr++ = '.';
    nptr --;

    strncpy((char *)nptr, pages[hpage].page_text,
            sizeof(number) - (nptr - number) - 1);
    number[sizeof(number) - 1] = '\0';

    r = new_render(*page, RENDER_TEXT, right - width + x, *y, 0, 0, number);
    r->data.text.typeface = t->typeface;
    r->data.text.style    = t->style;
    r->data.text.size     = _htmlSizes[t->size];
    memcpy(r->data.text.rgb, rgb, sizeof(rgb));
  }
}


//
 * 'parse_contents()' - Parse the table of contents and produce a
 *                      rendering list...


static void
parse_contents(tree_t *t,		// I - Tree to parse
               float  left,		// I - Left margin
               float  right,		// I - Printable width
               float  bottom,		// I - Bottom margin
               float  top,		// I - Printable top
               float  *y,		// IO - Y position
               int    *page,		// IO - Page #
               int    *heading,		// IO - Heading #
	       tree_t *chap)		// I - Chapter heading
{
  DEBUG_printf(("parse_contents(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, left, right, bottom, top, *y, *page, *heading, chap));

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_B :	// Top-level TOC
          if (t->prev != NULL)	// Advance one line prior to top-levels...
            *y -= _htmlSpacings[SIZE_P];

          if (*y < (bottom + _htmlSpacings[SIZE_P] * 3))
	    *y = 0; // Force page break

          chap = t;

      case MARKUP_LI :	// Lower-level TOC
          DEBUG_printf(("parse_contents: heading=%d, page = %d\n", *heading,
                        heading_pages[*heading]));

         //
          * Put the text...
         

          render_contents(t, left, right, bottom, top, y, page, *heading, chap);

         //
	  * Update current headings for header/footer strings in TOC.
	 

	  check_pages(*page);

	  if (t->markup == MARKUP_B &&
	      pages[*page].chapter == pages[*page - 1].chapter)
	    pages[*page].chapter = htmlGetText(t->child);

	  if (pages[*page].heading == pages[*page - 1].heading)
	    pages[*page].heading = htmlGetText(t->child);

         //
          * Next heading...
         

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


//
 * 'parse_doc()' - Parse a document tree and produce rendering list output.


static void
parse_doc(tree_t *t,		// I - Tree to parse
          float  *left,		// I - Left margin
          float  *right,	// I - Printable width
          float  *bottom,	// I - Bottom margin
          float  *top,		// I - Printable top
          float  *x,		// IO - X position
          float  *y,		// IO - Y position
          int    *page,		// IO - Page #
	  tree_t *cpara,	// I - Current paragraph
	  int    *needspace)	// I - Need whitespace before this element
{
  int		i;		// Looping var
  tree_t	*para,		// Phoney paragraph tree entry
		*temp;		// Paragraph entry
  var_t		*var;		// Variable entry
  uchar		*name;		// ID name
  uchar		*style;		// STYLE attribute
  float		width,		// Width of horizontal rule
		height,		// Height of rule
		rgb[3];		// RGB color of rule


  DEBUG_printf(("parse_doc(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, cpara=%p, needspace=%d\n",
                t, *left, *right, *bottom, *top, *x, *y, *page, cpara,
	        *needspace));

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
      if (para->child != NULL)
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
     //
      * Add a link target using the ID=name variable...
     

      add_link(name, *page, (int)(*y + 3 * t->height));
    }
    else if (t->markup == MARKUP_FILE)
    {
     //
      * Add a file link...
     

      uchar	name[256],	// New filename
		*sep;		// "?" separator in links


      // Strip any trailing HTTP GET data stuff...
      strncpy((char *)name, (char *)htmlGetVariable(t, (uchar *)"FILENAME"),
              sizeof(name) - 1);
      name[sizeof(name) - 1] = '\0';

      if ((sep = (uchar *)strchr((char *)name, '?')) != NULL)
        *sep = '\0';

      // Add the link
      add_link(name, *page + (OutputType == OUTPUT_BOOK), (int)top);
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

          *x    = *left + 36.0f;
	  *left += 36.0f;

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
          *needspace = t->next && t->next->markup != MARKUP_LI;
          break;

      case MARKUP_DT :
          if (para->child != NULL)
          {
            parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          *x    = *left - 36.0f;
	  *left -= 36.0f;

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
	   //
	    * Generate a horizontal rule...
	   

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
	     //
	      * Won't fit on this page...
	     

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
	   //
	    * <HR BREAK> generates a page break...
	   

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

      case MARKUP_A :
          if (htmlGetVariable(t, (uchar *)"NAME") != NULL)
	  {
	   //
	    * Add this named destination to the paragraph tree...
	   

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
    *needspace  = 1;
  }

  if (cpara != para)
    htmlDeleteTree(para);
}


//
 * 'parse_heading()' - Parse a heading tree and produce rendering list output.


static void
parse_heading(tree_t *t,	// I - Tree to parse
              float  left,	// I - Left margin
              float  right,	// I - Printable width
              float  bottom,	// I - Bottom margin
              float  top,	// I - Printable top
              float  *x,	// IO - X position
              float  *y,	// IO - Y position
              int    *page,	// IO - Page #
              int    needspace)	// I - Need whitespace?
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
    pages[*page].heading = htmlGetText(current_heading);

  if ((t->markup - MARKUP_H1) < TocLevels && !title_page)
  {
    DEBUG_printf(("H%d: heading_pages[%d] = %d\n", t->markup - MARKUP_H1 + 1,
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

    heading_pages[num_headings] = *page - chapter_starts[1] + 1;
    heading_tops[num_headings]  = (int)(*y + 4 * _htmlSpacings[SIZE_P]);
    num_headings ++;
  }

  parse_paragraph(t, left, right, bottom, top, x, y, page, needspace);

  if (t->halignment == ALIGN_RIGHT && t->markup == MARKUP_H1 &&
      OutputType == OUTPUT_BOOK && !title_page)
  {
   //
    * Special case - chapter heading for users manual...
   

    *y = bottom + 0.5f * (top - bottom);
  }
}


//
 * 'parse_paragraph()' - Parse a paragraph tree and produce rendering list
 *                       output.


static void
parse_paragraph(tree_t *t,	// I - Tree to parse
        	float  left,	// I - Left margin
        	float  right,	// I - Printable width
        	float  bottom,	// I - Bottom margin
        	float  top,	// I - Printable top
        	float  *x,	// IO - X position
        	float  *y,	// IO - Y position
        	int    *page,	// IO - Page #
        	int    needspace)// I - Need whitespace?
{
  int		whitespace;	// Non-zero if a fragment ends in whitespace
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
		*lineptr;
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

 //
  * First scan for images with left/right alignment tags...
 

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
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, RENDER_BOX, image_left + temp->width + borderspace,
	             *y - temp->height - borderspace,
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
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          new_render(*page, RENDER_BOX, image_right + temp->width + borderspace,
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          new_render(*page, RENDER_BOX, image_right, *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        *y -= borderspace;

        new_render(*page, RENDER_IMAGE, image_right + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));

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

 //
  * Then format the text and inline images...
 

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

        
        if (prev->markup == MARKUP_BR)
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

      if (temp->height > height && temp->markup != MARKUP_IMG)
        height = temp->height;
      else if ((0.5 * temp->height) > height && temp->markup == MARKUP_IMG &&
               temp->valignment == ALIGN_MIDDLE)
        height = 0.5 * temp->height;

      if (temp->superscript && height)
        temp_height += height - temp_height;
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
        switch (temp->valignment)
	{
	  case ALIGN_TOP :
              temp_height = temp->height;
	      break;
	  case ALIGN_MIDDLE :
              temp_height = 0.5f * temp->height + height;
              break;
	  case ALIGN_BOTTOM :
	      temp_height = temp->height + height;
              break;
	}

	if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	  borderspace = atof((char *)border);
	else if (temp->link)
	  borderspace = 1;
	else
	  borderspace = 0;

        borderspace *= PagePrintWidth / _htmlBrowserWidth;

        temp_height += 2 * borderspace;
      }

      if (temp->subscript)
        temp_height += height - temp_height;

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

    if (height == 0.0f)
      height = spacing;

    for (temp = start; temp != end; temp = temp->next)
      if (temp->markup != MARKUP_A)
        break;

    if (temp != NULL && temp->markup == MARKUP_NONE && temp->data[0] == ' ')
    {
      strcpy((char *)temp->data, (char *)temp->data + 1);
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

     //
      * See if we are doing a run of characters in a line and need to
      * output this run...
     

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
             //
              * Add a target link...
             

              add_link(link, *page, (int)(*y + 6 * height));
            }

	default :
	    temp_width = temp->width;
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
	      linewidth = 0.0;

	      rgb[0] = temp->red / 255.0f;
	      rgb[1] = temp->green / 255.0f;
	      rgb[2] = temp->blue / 255.0f;
	    }

            strcpy((char *)lineptr, (char *)temp->data);

            temp_width = temp->width + char_spacing * strlen((char *)lineptr);

	    if (temp->underline || (temp->link && LinkStyle && PSLevel == 0))
	      new_render(*page, RENDER_BOX, linex, *y + offset - 1, temp_width, 0, rgb);

	    if (temp->strikethrough)
	      new_render(*page, RENDER_BOX, linex, *y + offset + temp->height * 0.25f,
	                 temp_width, 0, rgb);

            linewidth  += temp_width;
            lineptr    += strlen((char *)lineptr);

            if (lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;

	case MARKUP_IMG :
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
		  offset = -0.5f * temp->height - borderspace;
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

      if (temp->link != NULL)
      {
        link = htmlGetVariable(temp->link, (uchar *)"HREF");

       //
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

   //
    * See if we have a run of characters that hasn't been output...
   

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
      r->data.text.spacing  = char_spacing;
      r->data.text.size     = _htmlSizes[linetype->size];
      memcpy(r->data.text.rgb, rgb, sizeof(rgb));

      if (linetype->superscript)
        r->y += height - linetype->height;
      else if (linetype->subscript)
        r->y -= height - linetype->height;

      free(linetype);
    }

   //
    * Update the margins after we pass below the images...
   

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


//
 * 'parse_pre()' - Parse preformatted text and produce rendering list output.


static void
parse_pre(tree_t *t,		// I - Tree to parse
          float  left,		// I - Left margin
          float  right,		// I - Printable width
          float  bottom,	// I - Bottom margin
          float  top,		// I - Printable top
          float  *x,		// IO - X position
          float  *y,		// IO - Y position
          int    *page,		// IO - Page #
          int    needspace)	// I - Need whitespace?
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

  DEBUG_printf(("parse_pre(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
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

     //
      * Add a page link...
     

      if (file_method((char *)link) == NULL)
      {
	if (file_target((char *)link) != NULL)
	  link = (uchar *)file_target((char *)link) - 1; // Include # sign
	else
	  link = (uchar *)file_basename((char *)link);
      }

      new_render(*page, RENDER_LINK, *x, *y, flat->width,
	         flat->height, link);

      if (PSLevel == 0 && Links)
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
           //
            * Add a target link...
           

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
		     image_find((char *)htmlGetVariable(flat, (uchar *)"REALSRC")));

          *x += flat->width;
          col ++;
	  break;

      default :
          break;
    }

    next = flat->next;
    free(flat);
    flat = next;
  }

  *x = left;
}


#ifdef TABLE_DEBUG
#  undef DEBUG_puts
#  define DEBUG_puts(x) puts(x)
#  define DEBUG
#  undef DEBUG_printf
#  define DEBUG_printf(x) printf x
#endif // TABLE_DEBUG

//
 * 'parse_table()' - Parse a table and produce rendering output.


static void
parse_table(tree_t *t,		// I - Tree to parse
            float  left,	// I - Left margin
            float  right,	// I - Printable width
            float  bottom,	// I - Bottom margin
            float  top,		// I - Printable top
            float  *x,		// IO - X position
            float  *y,		// IO - Y position
            int    *page,	// IO - Page #
            int    needspace)	// I - Need whitespace?
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
  char		col_fixed[MAX_COLUMNS];
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
		min_width,
		temp_width,
		row_y, temp_y,
		temp_bottom,
		temp_top;
  int		row_page, temp_page;
  uchar		*var,
		*height_var;			// Row HEIGHT variable
  tree_t	*temprow,
		*tempcol,
		*tempnext,
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
  render_t	*cell_bg[MAX_COLUMNS];		// Background rectangles
  render_t	*cell_start[MAX_COLUMNS];	// Start of the content for a cell in the row
  render_t	*cell_end[MAX_COLUMNS];		// End of the content for a cell in a row
  uchar		*bgcolor;
  float		rgb[3],
		bgrgb[3];


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("parse_table(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;   // Empty table...

  rgb[0] = t->red / 255.0f;
  rgb[1] = t->green / 255.0f;
  rgb[2] = t->blue / 255.0f;

 //
  * Figure out the # of rows, columns, and the desired widths...
 

  memset(col_spans, 0, sizeof(col_spans));
  memset(col_fixed, 0, sizeof(col_fixed));
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

  if (border == 0.0f && cellpadding > 0.0f)
  {
   //
    * Ah, the strange table formatting nightmare that is HTML.
    * Netscape and MSIE assign an invisible border width of 1
    * pixel if no border is specified...
   

    cellpadding += 1.0f;
  }

  border_size = border - 1.0f;

  cellspacing *= PagePrintWidth / _htmlBrowserWidth;
  cellpadding *= PagePrintWidth / _htmlBrowserWidth;
  border      *= PagePrintWidth / _htmlBrowserWidth;
  border_size *= PagePrintWidth / _htmlBrowserWidth;

  temp_bottom = bottom - cellpadding;
  temp_top    = top + cellpadding;

  memset(row_spans, 0, sizeof(row_spans));
  memset(span_heights, 0, sizeof(span_heights));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;

    if (temprow->markup == MARKUP_CAPTION)
    {
      parse_paragraph(temprow, left, right, bottom, top, x, y, page, needspace);
      needspace = 1;
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

	    for (tcol = 1; tcol < colspan; tcol ++)
              row_spans[col + tcol] = row_spans[col];
          }

          // Compute the cell size...
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min,
	                            &col_pref, &col_height);
          if ((var = htmlGetVariable(tempcol, (uchar *)"WIDTH")) != NULL)
	  {
	    if (var[strlen((char *)var) - 1] == '%')
              col_width -= 2.0 * cellpadding - cellspacing;
	  }
	  else if (htmlGetVariable(tempcol, (uchar *)"NOWRAP") != NULL)
	    col_width = col_pref;
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

 //
  * Now figure out the width of the table...
 

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

 //
  * Compute the width of each column based on the printable width.
 

  DEBUG_printf(("\nTABLE: %dx%d\n\n", num_cols, num_rows));

  actual_width  = (2 * cellpadding + cellspacing) * num_cols -
                  cellspacing;
  regular_width = (width - actual_width) / num_cols;

  DEBUG_printf(("    width = %.1f, actual_width = %.1f, regular_width = %.1f\n\n",
                width, actual_width, regular_width));
  DEBUG_puts("    Col  Width   Min     Pref    Fixed?");
  DEBUG_puts("    ---  ------  ------  ------  ------");

#ifdef DEBUG
  for (col = 0; col < num_cols; col ++)
    printf("    %-3d  %-6.1f  %-6.1f  %-6.1f  %s\n", col, col_widths[col],
           col_mins[col], col_prefs[col], col_fixed[col] ? "YES" : "NO");

  puts("");
#endif // DEBUG

 //
  * The first pass just handles columns with a specified width...
 

  DEBUG_puts("PASS 1: fixed width handling\n");

  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (col_widths[col] > 0.0f)
    {
      if (col_mins[col] > col_widths[col])
        col_widths[col] = col_mins[col];

      actual_width += col_widths[col];
    }
    else
      regular_cols ++;

  DEBUG_printf(("    actual_width = %.1f, regular_cols = %d\n\n", actual_width,
                regular_cols));

 //
  * Pass two uses the "preferred" width whenever possible, and the
  * minimum otherwise...
 

  DEBUG_puts("PASS 2: preferred width handling\n");

  for (col = 0, pref_width = 0.0f; col < num_cols; col ++)
    if (col_widths[col] == 0.0f)
      pref_width += col_prefs[col];

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
	pref_width = col_prefs[col] * regular_width;
	if (pref_width < col_mins[col])
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

        DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));

	actual_width += col_widths[col];
      }
  }

  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));

 //
  * Pass three enforces any hard or minimum widths for COLSPAN'd
  * columns...
 

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

              DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
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

 //
  * Pass four divides up the remaining space amongst the columns...
 

  DEBUG_puts("PASS 4: divide remaining space, if any...\n");

  if (width > actual_width)
  {
    regular_width = (width - actual_width) / num_cols;

    for (col = 0; col < num_cols; col ++)
    {
      col_widths[col] += regular_width;
      DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
    }
  }
  else
    width = actual_width;

  DEBUG_puts("");

 //
  * The final pass is only run if the width > table_width...
 

  DEBUG_puts("PASS 5: Squeeze table as needed...");

  if (width > table_width)
  {
   //
    * Squeeze the table to fit the requested width or the printable width
    * as determined at the beginning...
   

    for (col = 0, min_width = -cellspacing; col < num_cols; col ++)
      min_width += col_mins[col];

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

    for (col = 0, width = 0.0f; col < num_cols; col ++)
      width += col_widths[col] + 2 * cellpadding + cellspacing;

    width -= cellspacing;
  }

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

 //
  * Now render the whole table...
 

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  memset(row_spans, 0, sizeof(row_spans));
  memset(cell_start, 0, sizeof(cell_start));
  memset(cell_end, 0, sizeof(cell_end));
  memset(cell_height, 0, sizeof(cell_height));
  memset(cell_bg, 0, sizeof(cell_bg));

  for (row = 0; row < num_rows; row ++)
  {
    height_var = NULL;

    if (cells[row][0] != NULL)
    {
     //
      * Do page comments...
     

      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->markup == MARKUP_COMMENT)
        parse_comment(cells[row][0]->parent->prev,
                      &left, &right, &temp_bottom, &temp_top, x, y,
		      page, NULL, 0);

     //
      * Get height...
     

      if ((height_var = htmlGetVariable(t, (uchar *)"HEIGHT")) == NULL)
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

      if (htmlGetVariable(t, (uchar *)"HEIGHT") != NULL)
        temp_height /= num_rows;

      temp_height -= 2 * cellpadding;
    }
    else
    {
      // Use min height computed from get_cell_size()...
      for (col = 0, temp_height = _htmlSpacings[SIZE_P];
           col < num_cols;
	   col ++)
        if (cells[row][col] != NULL &&
	    cells[row][col]->height > temp_height)
	  temp_height = cells[row][col]->height;

      if (temp_height > (PageLength / 8) && height_var == NULL)
	temp_height = PageLength / 8;
    }

    DEBUG_printf(("BEFORE row = %d, temp_height = %.1f, *y = %.1f\n",
                  row, temp_height, *y));

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
    row_page   = *page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row_y = %.1f, *y = %.1f\n", row_y, *y));

    for (col = 0, rowspan = 9999; col < num_cols; col += colspan)
    {
      if (row_spans[col] == 0)
      {
        if ((var = htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN")) != NULL)
          row_spans[col] = atoi((char *)var);

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
	else if ((bgcolor = htmlGetVariable(cells[row][col], (uchar *)"BGCOLOR")) == NULL)
          if ((bgcolor = htmlGetVariable(cells[row][col]->parent, (uchar *)"BGCOLOR")) == NULL)
	    bgcolor = htmlGetVariable(t, (uchar *)"BGCOLOR");

	if (bgcolor != NULL)
	{
          get_color(bgcolor, bgrgb, 0);

	  width       = col_rights[col + colspan] - col_lefts[col] +
        	        2 * cellpadding;
	  border_left = col_lefts[col] - cellpadding;

          cell_bg[col] = new_render(*page, RENDER_BOX, border_left, row_y,
                                    width + border, 0.0, bgrgb);
	}
	else
	  cell_bg[col] = NULL;

	cell_start[col] = pages[*page].end;
	cell_page[col]  = temp_page;
	cell_y[col]     = temp_y;

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
      }

      if (row_spans[col] == 0 &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > row_height)
        row_height = cell_height[col];

      if (row_spans[col] < (rowspan + 1))
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

      DEBUG_printf(("**** col = %d, row = %d, row_y = %.1f\n", col, row, row_y));

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

      DEBUG_printf(("col = %d, row_spans = %d, span_heights = %.1f, cell_height = %.1f\n",
                    col, row_spans[col], span_heights[col], cell_height[col]));

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

    DEBUG_printf(("AFTER row = %d, row_y = %.1f, row_height = %.1f, *y = %.1f, do_valign = %d\n",
                  row, row_y, row_height, *y, do_valign));

   //
    * Do the vertical alignment
   

    if (do_valign)
    {
      if (height_var != NULL)
      {
        // Hardcode the row height...
        if (height_var[strlen((char *)height_var) - 1] == '%')
	  temp_height = atof((char *)height_var) * 0.01f * PagePrintLength;
	else
          temp_height = atof((char *)height_var) * PagePrintWidth / _htmlBrowserWidth;

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
	    DEBUG_printf(("aligning %p, y was %.1f, now %.1f\n",
	                  p, p->y, p->y - delta_y));

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
#endif // DEBUG
      }
    }

    // Update all current columns with ROWSPAN <= rowspan to use the same
    // end page...
    for (col = 1, temp_page = cell_endpage[0]; col < num_cols; col ++)
      if (cell_endpage[col] > temp_page && row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
        temp_page = cell_endpage[col];

    for (col = 0; col < num_cols; col ++)
      if (row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
        cell_endpage[col] = temp_page;

    row_y -= cellpadding;

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      if (row_spans[col] > 0)
      {
        DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
	              col, row_spans[col], row_spans[col] - rowspan));
        row_spans[col] -= rowspan;
      }

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      width = col_rights[col + colspan] - col_lefts[col] +
              2 * cellpadding;

      if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
          row_spans[col] > 0)
        continue;

      if ((bgcolor = htmlGetVariable(cells[row][col], (uchar *)"BGCOLOR")) == NULL)
        if ((bgcolor = htmlGetVariable(cells[row][col]->parent, (uchar *)"BGCOLOR")) == NULL)
	  bgcolor = htmlGetVariable(t, (uchar *)"BGCOLOR");

      if (bgcolor != NULL)
        get_color(bgcolor, bgrgb, 0);

      border_left = col_lefts[col] - cellpadding;

      if (cell_page[col] != cell_endpage[col])
      {
       //
        * Crossing a page boundary...
       

        if (border > 0)
	{
	 //
	  * +---+---+---+
	  * |   |   |   |
	 

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

        for (temp_page = cell_page[col] + 1; temp_page != cell_endpage[col]; temp_page ++)
	{
	 //
	  * |   |   |   |
	  * |   |   |   |
	 

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
	 //
	  * |   |   |   |
	  * +---+---+---+
	 

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
          new_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
	             width + border, top - row_y, bgrgb,
		     pages[cell_endpage[col]].start);
      }
      else
      {
       //
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

  *x = left;

 //
  * Free memory for the table...
 

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
#endif // TABLE_DEBUG


//
 * 'parse_list()' - Parse a list entry and produce rendering output.


static void
parse_list(tree_t *t,		// I - Tree to parse
           float  *left,	// I - Left margin
           float  *right,	// I - Printable width
           float  *bottom,	// I - Bottom margin
           float  *top,		// I - Printable top
           float  *x,		// IO - X position
           float  *y,		// IO - Y position
           int    *page,	// IO - Page #
           int    needspace)	// I - Need whitespace?
{
  uchar		number[255];	// List number (for numbered types)
  uchar		*value;		// VALUE= variable
  int		typeface;	// Typeface of list number
  float		width;		// Width of list number
  render_t	*r;		// Render primitive
  int		oldpage;	// Old page value
  float		oldy;		// Old Y value
  float		tempx;		// Temporary X value


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


//
 * 'init_list()' - Initialize the list type and value as necessary.


static void
init_list(tree_t *t)		// I - List entry
{
  uchar		*type,		// TYPE= variable
		*value;		// VALUE= variable
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


//
 * 'parse_comment()' - Parse a comment for HTMLDOC comments.


static void
parse_comment(tree_t *t,	// I - Tree to parse
              float  *left,	// I - Left margin
              float  *right,	// I - Printable width
              float  *bottom,	// I - Bottom margin
              float  *top,	// I - Printable top
              float  *x,	// IO - X position
              float  *y,	// IO - Y position
              int    *page,	// IO - Page #
	      tree_t *para,	// I - Current paragraph
	      int    needspace)	// I - Need whitespace?
{
  const char	*comment;	// Comment text
  char		*ptr,		// Pointer into value string
		buffer[1024];	// Buffer for strings
  int		pos,		// Position (left, center, right)
		tof;		// Top of form


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
     //
      * <!-- PAGE BREAK --> generates a page break...
     

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
     //
      * <!-- NEW PAGE --> generates a page break...
     

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
     //
      * <!-- NEW SHEET --> generate a page break to a new sheet...
     

      comment += 9;

      if (para != NULL && para->child != NULL)
      {
	parse_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      (*page) ++;
      if (PageDuplex && ((*page) & 1))
	(*page) ++;

      if (Verbosity)
	progress_show("Formatting page %d", *page);
      *x = *left;
      *y = *top;

      tof = 1;
    }
    else if (strncasecmp(comment, "HALF PAGE", 9) == 0 &&
             (!comment[9] || isspace(comment[9])))
    {
     //
      * <!-- HALF PAGE --> Go to the next half page.  If in the
      * top half of a page, go to the bottom half.  If in the
      * bottom half, go to the next page.
     
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
     //
      * <!-- NEED amount --> generate a page break if there isn't
      * enough remaining space...
     

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
      PagePrintWidth      = PageWidth - PageRight - PageLeft;
      PagePrintLength     = PageLength - PageTop - PageBottom;

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

      *right         = PagePrintWidth - *right;
      PageLeft       = pages[*page].left = get_measurement(comment);
      PagePrintWidth = PageWidth - PageRight - PageLeft;
      *right         = PagePrintWidth - *right;

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

      *right         = PagePrintWidth - *right;
      PageRight      = pages[*page].right = get_measurement(comment);
      PagePrintWidth = PageWidth - PageRight - PageLeft;
      *right         = PagePrintWidth - *right;

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

      *top            = PagePrintLength - *top;
      PageBottom      = pages[*page].bottom = get_measurement(comment);
      PagePrintLength = PageLength - PageTop - PageBottom;
      *top            = PagePrintLength - *top;
      *y              = *top;

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

      *top            = PagePrintLength - *top;
      PageTop         = pages[*page].top = get_measurement(comment);
      PagePrintLength = PageLength - PageTop - PageBottom;
      *top            = PagePrintLength - *top;
      *y              = *top;

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
	  *top           = PageLength - PageTop - *top;
	  PagePrintWidth = PageWidth - PageTop - PageLeft;
	  *top           = PageWidth - PageTop - *top;

	  *right          = PageWidth - PageRight - *right;
	  PagePrintLength = PageLength - PageRight - PageBottom;
	  *right          = PageLength - PageRight - *right;
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
	break;
      }

      while (isspace(*comment))
	comment ++;

      if (*comment != '\"')
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad HEADER string: \"%s\"", comment);
	break;
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
	check_pages(*page);

	pages[*page].header[pos] = (uchar *)Header[pos];
      }

      // Adjust top margin as needed...
      for (pos = 0; pos < 3; pos ++)
        if (Header[pos])
	  break;

      if (pos < 3)
      {
	if (logo_height > HeadFootSize)
          PageTop = (int)(logo_height + HeadFootSize);
	else
          PageTop = (int)(2 * HeadFootSize);
      }

      if (tof)
        *y = *top = PagePrintLength - PageTop;
    }
    else if (strncasecmp(comment, "FOOTER ", 7) == 0)
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
	break;
      }

      while (isspace(*comment))
	comment ++;

      if (*comment != '\"')
      {
        progress_error(HD_ERROR_BAD_COMMENT,
                       "Bad FOOTER string: \"%s\"", comment);
	break;
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
      for (pos = 0; pos < 3; pos ++)
        if (Footer[pos])
	  break;

      if (pos == 3)
        PageBottom = 0;
      else if (logo_height > HeadFootSize)
        PageBottom = (int)(logo_height + HeadFootSize);
      else
        PageBottom = (int)(2 * HeadFootSize);

      if (tof)
        *bottom = PageBottom;
    }
    else
      break;
  }
}


//
 * 'new_render()' - Allocate memory for a new rendering structure.


static render_t *		// O - New render structure
new_render(int      page,	// I - Page number (0-n)
           int      type,	// I - Type of render primitive
           float    x,		// I - Horizontal position
           float    y,		// I - Vertical position
           float    width,	// I - Width
           float    height,	// I - Height
           void     *data,	// I - Data
	   render_t *insert)	// I - Insert before here...
{
  render_t		*r;	// New render primitive
  static render_t	dummy;	// Dummy var for errors...


  DEBUG_printf(("new_render(page=%d, type=%d, x=%.1f, y=%.1f, width=%.1f, height=%.1f, data=%p, insert=%d)\n",
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
    r = (render_t *)calloc(sizeof(render_t) + strlen((char *)data), 1);

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


//
 * 'check_pages()' - Allocate memory for more pages as needed...


static void
check_pages(int page)	// I - Current page
{
  page_t	*temp;	// Temporary page pointer


  DEBUG_printf(("check_pages(%d)\n", page));

  // See if we need to allocate memory for the page...
  if (page >= alloc_pages)
  {
    // Yes, allocate enough for ALLOC_PAGES more pages...
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

    memset(temp + alloc_pages - ALLOC_PAGES, 0, sizeof(page_t) * ALLOC_PAGES);

    pages = temp;
  }

  // Initialize the page data as needed...
  for (temp = pages + num_pages; num_pages <= page; num_pages ++, temp ++)
    if (!temp->width)
    {
      if (num_pages == 0 || !temp[-1].width || !temp[-1].length)
      {
	temp->width     = PageWidth;
	temp->length    = PageLength;
	temp->left      = PageLeft;
	temp->right     = PageRight;
	temp->top       = PageTop;
	temp->bottom    = PageBottom;
	temp->duplex    = PageDuplex;
	temp->landscape = Landscape;
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
	memcpy(temp->footer, Footer, sizeof(temp->footer));
      }

      memcpy(temp->background_color, background_color,
             sizeof(temp->background_color));
      temp->background_image = background_image;
    }
}


//
 * 'add_link()' - Add a named link...


static void
add_link(uchar *name,		// I - Name of link
         int   page,		// I - Page #
         int   top)		// I - Y position
{
  link_t	*temp;		// New name


  if (name == NULL)
    return;

  if ((temp = find_link(name)) != NULL)
  {
    temp->page = page - 1;
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

    strncpy((char *)temp->name, (char *)name, sizeof(temp->name) - 1);
    temp->name[sizeof(temp->name) - 1] = '\0';
    temp->page = page - 1;
    temp->top  = top;

    if (num_links > 1)
      qsort(links, num_links, sizeof(link_t),
            (compare_func_t)compare_links);
  }
}


//
 * 'find_link()' - Find a named link...


static link_t *
find_link(uchar *name)	// I - Name to find
{
  link_t	key,	// Search key
		*match;	// Matching name entry


  if (name == NULL || num_links == 0)
    return (NULL);

  if (name[0] == '#')
    name ++;

  strncpy((char *)key.name, (char *)name, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';
  match = (link_t *)bsearch(&key, links, num_links, sizeof(link_t),
                            (compare_func_t)compare_links);

  return (match);
}


//
 * 'compare_links()' - Compare two named links.


static int			// O - 0 = equal, -1 or 1 = not equal
compare_links(link_t *n1,	// I - First name
              link_t *n2)	// I - Second name
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


//
 * 'copy_tree()' - Copy a markup tree...


static void
copy_tree(tree_t *parent,	// I - Source tree
          tree_t *t)		// I - Destination tree
{
  int		i;		// I - Looping var
  tree_t	*temp;		// I - New tree entry
  var_t		*var;		// I - Current markup variable


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


#ifdef TABLE_DEBUG
#  undef DEBUG_printf
#  undef DEBUG_puts
#  define DEBUG_printf(x) printf x
#  define DEBUG_puts(x) puts(x)
#endif // TABLE_DEBUG

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
		prefw;			// Local preferred width


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

#ifdef TABLE_DEBUG
          if (temp->markup == MARKUP_NONE)
	    printf("FRAG(%s) = %.1f\n", temp->data, temp->width);
	  else if (temp->markup == MARKUP_SPACER)
	    printf("SPACER = %.1f\n", temp->width);
	  else
	    printf("IMG(%s) = %.1f\n", htmlGetVariable(temp, (uchar *)"SRC"),
	           temp->width);
#endif // TABLE_DEBUG

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
	  else
	    frag_pref += temp->width;

          if ((temp->preformatted && temp->data != NULL &&
               temp->data[strlen((char *)temp->data) - 1] == '\n') ||
	      (!temp->preformatted && temp->data != NULL &&
	       (isspace(temp->data[0]) ||
		isspace(temp->data[strlen((char *)temp->data) - 1]))))
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
	  else if (temp->data != NULL)
            frag_width += temp->width + 1;
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
      case MARKUP_HR :
      case MARKUP_LI :
      case MARKUP_P :
      case MARKUP_PRE :
          DEBUG_printf(("BREAK at %.1f\n", frag_pref));

	  if (frag_pref > prefw)
	    prefw = frag_pref;

	  if (nowrap && frag_pref > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for break...\n",
	                  frag_pref, minw));
	    minw = frag_pref;
	  }

          frag_pref   = 0.0f;
	  frag_width  = 0.0f;
          frag_height = 0.0f;

      default :
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
   //
    * Ah, the strange table formatting nightmare that is HTML.
    * Netscape and MSIE assign an invisible border width of 1
    * pixel if no border is specified...
   

    cellpadding += 1.0f;
  }

  cellspacing *= PagePrintWidth / _htmlBrowserWidth;
  cellpadding *= PagePrintWidth / _htmlBrowserWidth;

  DEBUG_printf(("ADDING %.1f for table space...\n",
                max_columns * (2 * cellpadding + cellspacing) - cellspacing));

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
#endif // TABLE_DEBUG


//
 * 'flatten_tree()' - Flatten an HTML tree to only include the text, image,
 *                    link, and break markups.


static tree_t *			// O - Flattened markup tree
flatten_tree(tree_t *t)		// I - Markup tree to flatten
{
  tree_t	*temp,		// New tree node
		*flat;		// Flattened tree


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


//
 * 'update_image_size()' - Update the size of an image based upon the
 *                         printable width.


static void
update_image_size(tree_t *t)	// I - Tree entry
{
  image_t	*img;		// Image file
  uchar		*width,		// Width string
		*height;	// Height string


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


//
 * 'get_width()' - Get the width of a string in points.


static float			// O - Width in points
get_width(uchar *s,		// I - String to scan
          int   typeface,	// I - Typeface code
          int   style,		// I - Style code
          int   size)		// I - Size
{
  uchar	*ptr;			// Current character
  float	width;			// Current width


  DEBUG_printf(("get_width(\"%s\", %d, %d, %d)\n",
                s == NULL ? "(null)" : (const char *)s,
                typeface, style, size));

  if (s == NULL)
    return (0.0);

  for (width = 0.0, ptr = s; *ptr != '\0'; ptr ++)
    width += _htmlWidths[typeface][style][*ptr];

  return (width * _htmlSizes[size]);
}


//
 * 'get_title()' - Get the title string for a document.


static uchar *		// O - Title string
get_title(tree_t *doc)	// I - Document
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


//
 * 'compare_rgb()' - Compare two RGB colors...


static int			// O - -1 if rgb1<rgb2, etc.
compare_rgb(unsigned *rgb1,	// I - First color
            unsigned *rgb2)	// I - Second color
{
  return (*rgb1 - *rgb2);
}


//
 * 'write_image()' - Write an image to the given output file...


static void
write_image(FILE     *out,	// I - Output file
            render_t *r,	// I - Image to write
	    int      write_obj)	// I - Write an object?
{
  int		i, j, k, m,	// Looping vars
		ncolors;	// Number of colors
  uchar		*pixel,		// Current pixel
		*indices,	// New indexed pixel array
		*indptr;	// Current index
  int		indwidth,	// Width of indexed line
		indbits;	// Bits per index
  int		max_colors;	// Max colors to use
  unsigned	colors[256],	// Colormap values
		key,		// Color key
		*match;		// Matching color value
  uchar		grays[256],	// Grayscale usage
		cmap[256][3];	// Colormap
  image_t 	*img;		// Image
  struct jpeg_compress_struct cinfo;	// JPEG compressor


 //
  * See if we can optimize the image as indexed without color loss...
 

  img      = r->data.image;
  ncolors  = 0;
  indices  = NULL;
  indwidth = 0;

  if (!img->pixels && !img->obj)
    image_load(img->filename, !OutputColor, 1);

  if (PSLevel != 1 && PDFVersion >= 1.2f && img->obj == 0)
  {
    if (img->depth == 1)
    {
     //
      * Greyscale image...
     

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
     //
      * Color image...
     

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
     //
      * Convert a grayscale image...
     

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
     //
      * Convert a color image...
     

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
   //
    * Adobe doesn't like 1 color images...
   

    ncolors   = 2;
    colors[1] = 0;
  }
}
#endif // 0


//
// End of "$Id: render-table.cxx,v 1.1 2002/03/10 03:17:24 mike Exp $".
//
