//
// "$Id: pdf.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $"
//
//   PDF output routines for HTMLDOC, a HTML document processing program.
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
// 'HTMLDOC::pdf_write_document()' - Write all render entities to a PDF file.
//

void
HTMLDOC::pdf_write_document(uchar  *title,	// I - Title for all pages
        	   uchar  *author,	// I - Author of document
        	   uchar  *creator,	// I - Application that generated the HTML file
        	   uchar  *copyright,	// I - Copyright (if any) on the document
                   uchar  *keywords,	// I - Search keywords
                   HDtree *toc)		// I - Table of contents tree
{
  uchar		*page_chapter,	// Current chapter text
		*page_heading;	// Current heading text
  FILE		*out;		// Output file
  int		page,		// Current page #
		heading;	// Current heading #
  float		title_width;	// Width of title string
  int		bytes;		// Number of bytes
  char		buffer[8192];	// Copy buffer


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

    // Write the outline tree...
    heading = 0;
    pdf_write_contents(out, toc, 0, 0, 0, &heading);
  }
  else
    outline_object = 0;

  // Write the trailer and close the output file...
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


//
// 'HTMLDOC::pdf_write_resources()' - Write the resources dictionary for a page.
//

void
HTMLDOC::pdf_write_resources(FILE *out,	// I - Output file
                    int  page)	// I - Page for resources
{
  int		i;		// Looping var
  HDrender	*r;		// Render pointer
  int		fonts_used[16];	// Non-zero if the page uses a font
  int		images_used;	// Non-zero if the page uses an image
  int		text_used;	// Non-zero if the page uses text
  static char	*effects[] =	// Effects and their commands
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


//
// 'HTMLDOC::pdf_write_page()' - Write a page to a PDF file.
//

void
HTMLDOC::pdf_write_page(FILE  *out,		// I - Output file
               int   page,		// I - Page number
               uchar  *title,		// I - Title string
               float title_width,	// I - Width of title string
               uchar  **page_heading,	// IO - Page heading string
	       uchar  **page_chapter)	// IO - Page chapter string
{
  int		file_page,	// Current page # in file
		length,		// Stream length
		last_render;	// Last type of render
  HDrender	*r,		// Render pointer
		*next;		// Next render


  if (page < 0 || page >= MAX_PAGES)
    return;

  // Add headers/footers as needed...
  pspdf_prepare_page(page, &file_page, title, title_width, page_heading, page_chapter);

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
  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fputs("/Type/Page", out);
  fprintf(out, "/Parent %d 0 R", pages_object);
  fprintf(out, "/Contents %d 0 R", num_objects + 1);

  pdf_write_resources(out, page);

  // Actions (links)...
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

  // Render all page elements, freeing used memory as we go...
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

  // Output the page trailer...
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


//
// 'HTMLDOC::pdf_write_contents()' - Write the table of contents as outline records to
//                          a PDF file.
//

void
HTMLDOC::pdf_write_contents(FILE   *out,			// I - Output file
                   HDtree *toc,			// I - Table of contents tree
                   int    parent,		// I - Parent outline object
                   int    prev,			// I - Previous outline object
                   int    next,			// I - Next outline object
                   int    *heading)		// IO - Current heading #
{
  int		i,				// Looping var
		thisobj,			// This object
		entry,				// TOC entry object
		count;				// Number of entries at this level
  uchar		*text;				// Entry text
  HDtree	*temp;				// Looping var
  int		entry_counts[MAX_HEADINGS],	// Number of sub-entries for this entry
		entry_objects[MAX_HEADINGS];	// Objects for each entry
  HDtree	*entries[MAX_HEADINGS];		// Pointers to each entry


  // Make an object for this entry...
  num_objects ++;
  thisobj = num_objects;

  if (toc == NULL)
  {
    // This is for the Table of Contents page...
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
    // Find and count the children (entries)...
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
      // Add the table of contents to the top-level contents...
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

    // Output the top-level object...
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


//
// 'HTMLDOC::pdf_count_headings()' - Count the number of headings under this TOC
//                          entry.
//

int			// O - Number of headings found
HTMLDOC::pdf_count_headings(HDtree *toc)	// I - TOC entry
{
  int	headings;		// Number of headings


  for (headings = 0; toc != NULL; toc = toc->next)
    if (toc->markup == MARKUP_B || toc->markup == MARKUP_LI)
      headings ++;
    else if (toc->markup == MARKUP_UL && toc->child != NULL)
      headings += pdf_count_headings(toc->child);

  return (headings);
}


//
// 'HTMLDOC::pdf_write_background()' - Write a background image...
//

void
HTMLDOC::pdf_write_background(FILE *out)		// I - Output file
{
  int	length;				// Length of image


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


//
// 'HTMLDOC::pdf_write_links()' - Write annotation link objects for each page in the
//                       document.
//

void
HTMLDOC::pdf_write_links(FILE *out)		// I - Output file
{
  int		page,			// Current page
		lobj,			// Current link
		num_lobjs,		// Number of links on this page
		lobjs[2 * MAX_LINKS];	// Link objects
  float		x, y;			// Position of last link
  HDrender	*r,			// Current render primitive
		*rlast,			// Last render link primitive
		*rprev;			// Previous render primitive
  HDlink	*link;			// Local link


  // First combine adjacent, identical links...
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

  // Figure out how many link objects we'll have...
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

  // Add space for named links for PDF 1.2 output...
  if (PDFVersion >= 1.2)
    pages_object += num_links + 3;

  // Then generate annotation objects for all the links...
  memset(annots_objects, 0, sizeof(annots_objects));

  for (page = 0; page < num_pages; page ++)
  {
    num_lobjs = 0;

    for (r = pages[page]; r != NULL; r = r->next)
      if (r->type == RENDER_LINK)
      {
        if ((link = find_link(r->data.link)) != NULL)
	{
	  // Local link...
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
	  // Remote link...
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
#endif // WIN32 || __EMX__
	  {
	    // Link to external PDF file...
            fputs("/S/GoToR", out);
            fputs("/D[0/XYZ null null 0]", out);
            fputs("/F", out);
	    write_string(out, r->data.link, 0);
	  }
	  else
	  {
	    // Link to web file...
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


//
// 'HTMLDOC::pdf_write_names()' - Write named destinations for each link.
//

void
HTMLDOC::pdf_write_names(FILE *out)		// I - Output file
{
  int		i;			// Looping var
  uchar		*s;			// Current character in name
  HDlink	*link;			// Local link


  // Convert all link names to lowercase...
  for (i = num_links, link = links; i > 0; i --, link ++)
    for (s = link->name; *s != '\0'; s ++)
      *s = tolower(*s);

  // Write the root name tree entry...
  num_objects ++;
  names_object = num_objects;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fprintf(out, "/Dests %d 0 R", num_objects + 1);
  fputs(">>", out);
  fputs("endobj\n", out);

  // Write the name tree child list...
  num_objects ++;
  objects[num_objects] = ftell(out);

  fprintf(out, "%d 0 obj", num_objects);
  fputs("<<", out);
  fprintf(out, "/Kids[%d 0 R]", num_objects + 1);
  fputs(">>", out);
  fputs("endobj\n", out);

  // Write the leaf node for the name tree...
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
// 'HTMLDOC::write_prolog()' - Write the file prolog...
//

void
HTMLDOC::write_prolog(FILE *out,		// I - Output file
             int  page_count,	// I - Number of pages (0 if not known)
             uchar *title,	// I - Title of document
             uchar *author,	// I - Author of document
             uchar *creator,	// I - Application that generated the HTML file
             uchar *copyright,	// I - Copyright (if any) on the document
             uchar *keywords)	// I - Search keywords
{
  int		i, j,		// Looping vars
		encoding_object;// Font encoding object
  time_t	curtime;	// Current time
  struct tm	*curdate;	// Current date
  int		page;		// Current page
  HDrender	*r;		// Current render data
  int		fonts_used[4][4];// Whether or not a font is used
  char		temp[255];	// Temporary string
  md5_state_t	md5;		// MD5 state
  md5_byte_t	digest[16];	// MD5 digest value
  rc4_context_t	rc4;		// RC4 context
  uchar		owner_pad[32],	// Padded owner password
		owner_key[32],	// Owner key
		user_pad[32],	// Padded user password
		user_key[32];	// User key
  uchar		perm_bytes[4];	// Permission bytes
  unsigned	perm_value;	// Permission value, unsigned
  static unsigned char pad[32] =
		{		// Padding for passwords
		  0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41,
		  0x64, 0x00, 0x4e, 0x56, 0xff, 0xfa, 0x01, 0x08,
		  0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68, 0x3e, 0x80,
		  0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a
		};


  // Get the current time and date (ZULU).
  curtime = time(NULL);
  curdate = gmtime(&curtime);

  // See what fonts are used...
  memset(fonts_used, 0, sizeof(fonts_used));
  fonts_used[HeadFootType][HeadFootStyle] = 1;

  for (page = 0; page < num_pages; page ++)
    for (r = pages[page]; r != NULL; r = r->next)
      if (r->type == RENDER_TEXT)
	fonts_used[r->data.text.typeface][r->data.text.style] = 1;

  // Generate the heading...
  if (PSLevel > 0)
  {
    // Write PostScript prolog stuff...
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

    // Output the font encoding for the current character set...  For now we
    // just support 8-bit fonts since true Unicode support needs a very large
    // number of extra fonts that aren't normally available on a PS printer.
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

    // Fonts...
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

    // Now for the macros...
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
    // Write PDF prolog stuff...
    fprintf(out, "%%PDF-%.1f\n", PDFVersion);
    fputs("%\342\343\317\323\n", out);
    num_objects = 0;

    // Compute the file ID...
    md5_init(&md5);
    md5_append(&md5, (md5_byte_t *)OutputPath, sizeof(OutputPath));
    md5_append(&md5, (md5_byte_t *)&curtime, sizeof(curtime));
    md5_finish(&md5, file_id);

    // Setup encryption stuff as necessary...
    if (Encryption)
    {
      // Copy and pad the user password...
      strncpy((char *)user_pad, UserPassword, sizeof(user_pad));

      if ((i = strlen(UserPassword)) < 32)
	memcpy(user_pad + i, pad, 32 - i);

      if (OwnerPassword[0])
      {
        // Copy and pad the owner password...
        strncpy((char *)owner_pad, OwnerPassword, sizeof(owner_pad));

	if ((i = strlen(OwnerPassword)) < 32)
	  memcpy(owner_pad + i, pad, 32 - i);
      }
      else
      {
        // Generate a pseudo-random owner password...
	srand(curtime);

	for (i = 0; i < 32; i ++)
	  owner_pad[i] = rand();
      }

      // Compute the owner key...
      md5_init(&md5);
      md5_append(&md5, owner_pad, 32);
      md5_finish(&md5, digest);

      rc4_init(&rc4, digest, 5);
      rc4_encrypt(&rc4, user_pad, owner_key, 32);

      // Compute the encryption key...
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

      // Write the encryption dictionary...
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

    // Write info object...
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

    // Write the font encoding for the selected character set.  Note that
    // we *should* be able to use the WinAnsiEncoding value for ISO-8859-1
    // to make smaller files, however Acrobat Exchange does not like it
    // despite the fact that it is defined in the PDF specification...
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
        // Output a character index if we had blank ones...
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
	  if (i < 3) // Use native encoding for symbols
	    fprintf(out, "/Encoding %d 0 R", encoding_object);
	  fputs(">>", out);
	  fputs("endobj\n", out);
        }

    if (background_image != NULL)
      pdf_write_background(out);
  }
}


//
// 'HTMLDOC::write_trailer()' - Write the file trailer.
//

void
HTMLDOC::write_trailer(FILE *out,	// I - Output file
              int  pages)	// I - Number of pages in file
{
  int		i, j,		// Looping vars
		type,		// Type of number
		offset;		// Offset to xref table in PDF file
  static char	*modes[] =	// Page modes
		{
		  "UseNone",
		  "UseOutlines",
		  "FullScreen"
		};
  static char	*layouts[] =	// Page layouts
		{
		  "SinglePage",
		  "OneColumn",
		  "TwoColumnLeft",
		  "TwoColumnRight"
		};


  if (PSLevel > 0)
  {
    // PostScript...
    fputs("%%Trailer\n", out);
    if (pages > 0)
      fprintf(out, "%%%%Pages: %d\n", pages);

    fputs("%%EOF\n", out);
  }
  else
  {
    // PDF...
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


//
// 'HTMLDOC::encrypt_init()' - Initialize the RC4 encryption context for the current
//                    object.
//

void
HTMLDOC::encrypt_init(void)
{
  uchar		data[10];	// Key data
  md5_state_t	md5;		// MD5 state
  md5_byte_t	digest[16];	// MD5 digest value


  // Compute the key data for the MD5 hash.
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

  // Hash it...
  md5_init(&md5);
  md5_append(&md5, data, 10);
  md5_finish(&md5, digest);

  // Initialize the RC4 context using the first 10 bytes of the digest...
  rc4_init(&encrypt_state, digest, 10);
}


//
// End of "$Id: pdf.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $".
//
