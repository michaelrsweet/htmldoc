//
// "$Id: format.cxx,v 1.1 2000/10/16 03:25:06 mike Exp $"
//
//   Formatting routines for HTMLDOC, a HTML document processing
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
// 'HTMLDOC::parse_contents()' - Parse the table of contents and produce a
//                      rendering list...
//

void
HTMLDOC::parse_contents(HDtree *t,		// I - Tree to parse
               float  left,		// I - Left margin
               float  right,		// I - Printable width
               float  bottom,		// I - Bottom margin
               float  top,		// I - Printable top
               float  *y,		// IO - Y position
               int    *page,		// IO - Page #
               int    *heading)		// IO - Heading #
{
  float		x,
		width,
		numberwidth,
		height,
		rgb[3];
  uchar		number[255],
		*nptr,
		*link;
  HDtree	*flat,
		*temp,
		*next;
  HDrender	*r;
#define dot_width  (HDtree::sizes[SIZE_P] * HDtree::widths[t->typeface][t->style]['.'])


  DEBUG_printf(("parse_contents(t=%08x, y=%.1f, page=%d, heading=%d)\n",
                t, *y, *page, *heading));

  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_B :	// Top-level TOC
          if (t->prev != NULL)	// Advance one line prior to top-levels...
            *y -= HDtree::spacings[SIZE_P];

      case MARKUP_LI :	// Lower-level TOC
          DEBUG_printf(("parse_contents: heading=%d, page = %d\n", *heading,
                        heading_pages_[*heading]));

          // Put the text...
	  if (t->child)
            flat = t->child->flatten();
	  else
	    flat = NULL;

	  for (height = 0.0, temp = flat; temp != NULL; temp = temp->next)
	    if (temp->height > height)
              height = temp->height;

          height *= HDtree::spacings[SIZE_P] / HDtree::sizes[SIZE_P];

          x  = left + 36.0f * t->indent;
	  *y -= height;

	  // Get the width of the page number, leave room for three dots...
          sprintf((char *)number, "%d", heading_pages_[*heading]);
          numberwidth = get_width(number, t->typeface, t->style, t->size) +
	                3.0f * dot_width;

          for (temp = flat; temp != NULL; temp = next)
          {
	    rgb[0] = temp->red / 255.0f;
	    rgb[1] = temp->green / 255.0f;
	    rgb[2] = temp->blue / 255.0f;

	    if ((x + temp->width) >= (right - numberwidth))
	    {
	      // Too wide to fit, continue on the next line
	      *y -= HDtree::spacings[SIZE_P];
	      x  = left + 36.0f * t->indent;
	    }

            if (*y < bottom)
            {
              (*page) ++;
	      if (HTMLDOC::verbosity_)
		progress_show("Formatting page %d", *page);
              width = get_width((uchar *)toc_title_, TYPE_HELVETICA, STYLE_BOLD, SIZE_H1);
              *y = top - HDtree::spacings[SIZE_H1];
              x  = left + 0.5f * (right - left - width);
              r = new_render(*page, RENDER_TEXT, x, *y, 0, 0, toc_title_);
              r->data.text.typeface = TYPE_HELVETICA;
              r->data.text.style    = STYLE_BOLD;
              r->data.text.size     = HDtree::sizes[SIZE_H1];
	      get_color(HDtree::text_color, r->data.text.rgb);

              *y -= HDtree::spacings[SIZE_H1];
	      x  = left + 36.0f * t->indent;
            }

	    if (temp->link != NULL)
	    {
              link = temp->link->var((uchar *)"HREF");

	      // Add a page link...
	      if (file_method((char *)link) == NULL &&
	          file_target((char *)link) != NULL)
	        link = (uchar *)file_target((char *)link) - 1; // Include # sign

	      new_render(*page, RENDER_LINK, x, *y, temp->width,
	                 temp->height, link);

	      if (ps_level_ == 0)
	      {
                memcpy(rgb, link_rgb_, sizeof(rgb));

		temp->red   = (int)(link_rgb_[0] * 255.0);
		temp->green = (int)(link_rgb_[1] * 255.0);
		temp->blue  = (int)(link_rgb_[2] * 255.0);

                if (link_style_)
		  new_render(*page, RENDER_BOX, x, *y - 1, temp->width, 0,
	                     link_rgb_);
	      }
	    }

	    switch (temp->markup)
	    {
              case MARKUP_A :
        	  if ((link = temp->var((uchar *)"NAME")) != NULL)
        	  {
        	    // Add a target link...
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
        	  r->data.text.size     = HDtree::sizes[temp->size];
        	  memcpy(r->data.text.rgb, rgb, sizeof(rgb));

        	  if (temp->superscript)
        	    r->y += height - temp->height;
        	  else if (temp->subscript)
        	    r->y -= height * HDtree::sizes[0] / HDtree::spacings[0] -
		            temp->height;
		  break;

	      case MARKUP_IMG :
		  new_render(*page, RENDER_IMAGE, x, *y, temp->width, temp->height,
			     HDimage::find((char *)temp->var((uchar *)"SRC"), !output_color_));
		  break;
	    }

	    x += temp->width;
	    next = temp->next;
	    free(temp);
	  }

          // Draw dots leading up to the page number...
          sprintf((char *)number, "%d", heading_pages_[*heading]);
          width = get_width(number, t->typeface, t->style, t->size) + x;

          for (nptr = number; width < right; width += dot_width, nptr ++)
            *nptr = '.';
          nptr --;
          sprintf((char *)nptr, "%d", heading_pages_[*heading]);

          r = new_render(*page, RENDER_TEXT, right - width + x, *y, 0, 0, number);
          r->data.text.typeface = t->typeface;
          r->data.text.style    = t->style;
          r->data.text.size     = HDtree::sizes[t->size];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

          // Next heading...
          (*heading) ++;
          break;

      default :
          parse_contents(t->child, left, right, bottom, top, y, page, heading);
          break;
    }

    t = t->next;
  }
}


//
// 'HTMLDOC::parse_doc()' - Parse a document tree and produce rendering list output.
//

void
HTMLDOC::parse_doc(HDtree *t,		// I - Tree to parse
          float  left,		// I - Left margin
          float  right,		// I - Printable width
          float  bottom,	// I - Bottom margin
          float  top,		// I - Printable top
          float  *x,		// IO - X position
          float  *y,		// IO - Y position
          int    *page,		// IO - Page #
	  HDtree *cpara,	// I - Current paragraph
	  int    *needspace)	// I - Need whitespace before this element
{
  int		i;		// Looping var
  HDtree	*para,		// Phoney paragraph tree entry
		*temp;		// Paragraph entry
  HDvar		*v;		// Variable entry
  uchar		*name;		// ID name
  float		width,		// Width of horizontal rule
		height,		// Height of rule
		rgb[3];		// RGB color of rule


  DEBUG_printf(("parse_doc(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d, cpara = %08x\n",
                t, left, right, *x, *y, *page, cpara));

  if (cpara == NULL)
    para = new HDtree(NULL, MARKUP_P, NULL);
  else
    para = cpara;

  while (t != NULL)
  {
    if (((t->markup == MARKUP_H1 && output_book_) ||
         (t->markup == MARKUP_FILE && !output_book_)) && !in_title_page_)
    {
      // New page on H1 in book mode or file in webpage mode...
      if (para->child != NULL)
      {
        parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
        delete para->child;
      }

      if ((chapter_ > 0 && output_book_) ||
          ((*page > 1 || *y < top) && !output_book_))
      {
        (*page) ++;
        if (page_duplex_ && (*page & 1))
          (*page) ++;

        if (HTMLDOC::verbosity_)
          progress_show("Formatting page %d", *page);

        if (output_book_)
          chapter_ends_[chapter_] = *page - 1;
      }

      if (output_book_)
      {
        chapter_ ++;
	if (chapter_ >= MAX_CHAPTERS)
	{
	  progress_error("Too many chapters in document (%d > %d)!",
	                 chapter_, MAX_CHAPTERS);
          chapter_ = MAX_CHAPTERS - 1;
	}
	else
          chapter_starts_[chapter_] = *page;

	if (chapter_ > num_chapters_)
	  num_chapters_ = chapter_;
      }

      *y         = top;
      *x         = left;
      *needspace = 0;
    }

    if ((name = t->var((uchar *)"ID")) != NULL)
    {
      // Add a link target using the ID=name variable...
      add_link(name, *page, (int)(*y + 3 * t->height));
    }
    else if (t->markup == MARKUP_FILE)
    {
      // Add a file link...
      add_link(t->var((uchar *)"FILENAME"), *page + output_book_,
               (int)top);
    }

    if (chapter_ == 0 && !in_title_page_)
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
      case MARKUP_NONE :
      case MARKUP_BR :
          if (para->child == NULL)
          {
            para->halignment = t->halignment;
            para->indent     = t->indent;
          }

          if ((temp = t->dup()) != NULL)
          {
	    para->add(temp);
	    if (t->child)
	      t->child->copy(temp);
          }
          break;

      case MARKUP_TABLE :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            delete para->child;
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
            delete para->child;
          }

          parse_heading(t, left, right, bottom, top, x, y, page, 1);
	  *needspace = 1;
          break;

      case MARKUP_BLOCKQUOTE :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            delete para->child;
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
            delete para->child;
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
            delete para->child;
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
            delete para->child;
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
            delete para->child;
          }

          *x = left + 36.0f;

          parse_doc(t->child, left + 36, right, bottom, top, x, y, page, para,
	            needspace);
          break;

      case MARKUP_LI :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            delete para->child;
          }

          parse_list(t, left, right, bottom, top, x, y, page, *needspace);

          *x         = left;
          *needspace = 0;
          break;

      case MARKUP_DT :
          if (para->child != NULL)
          {
            parse_paragraph(para, left, right, bottom, top, x, y, page, *needspace);
            delete para->child;
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
            delete para->child;
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
            delete para->child;
          }

          if (t->var((uchar *)"BREAK") == NULL)
	  {
	    // Generate a horizontal rule...
            if ((name = t->var((uchar *)"WIDTH")) == NULL)
	      width = right - left;
	    else
	    {
	      if (strchr((char *)name, '%') != NULL)
	        width = atoi((char *)name) * (right - left) / 100;
	      else
                width = atoi((char *)name) * page_print_width_ / browser_width_;
            }

            if ((name = t->var((uchar *)"SIZE")) == NULL)
	      height = 2;
	    else
	      height = atoi((char *)name) * page_print_width_ / browser_width_;

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

            if (*y < (bottom + height + HDtree::spacings[SIZE_P]))
	    {
	      // Won't fit on this page...
              (*page) ++;
	      if (HTMLDOC::verbosity_)
	        progress_show("Formatting page %d", *page);
              *y = top;
            }

            (*y)   -= height + HDtree::spacings[SIZE_P];
            rgb[0] = t->red / 255.0f;
            rgb[1] = t->green / 255.0f;
            rgb[2] = t->blue / 255.0f;

            new_render(*page, RENDER_FBOX, *x, *y + HDtree::spacings[SIZE_P] * 0.5,
	               width, height, rgb);
	  }
	  else
	  {
	    // <HR BREAK> generates a page break...
            (*page) ++;
	    if (HTMLDOC::verbosity_)
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
          if (t->var((uchar *)"NAME") != NULL)
	  {
	    // Add this named destination to the paragraph tree...
            if (para->child == NULL)
            {
              para->halignment = t->halignment;
              para->indent     = t->indent;
            }

            if ((temp = t->dup()) != NULL)
	      para->add(temp);
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
    delete para->child;
    *needspace  = 1;
  }

  if (cpara != para)
    delete para;
}


//
// 'HTMLDOC::parse_heading()' - Parse a heading tree and produce rendering list output.
//

void
HTMLDOC::parse_heading(HDtree *t,	// I - Tree to parse
              float  left,	// I - Left margin
              float  right,	// I - Printable width
              float  bottom,	// I - Bottom margin
              float  top,	// I - Printable top
              float  *x,	// IO - X position
              float  *y,	// IO - Y position
              int    *page,	// IO - Page #
              int    needspace)	// I - Need whitespace?
{
  DEBUG_printf(("parse_heading(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (((t->markup - MARKUP_H1) < toc_levels_ || toc_levels_ == 0) && !in_title_page_)
    current_heading = t->child;

  if (*y < (5 * HDtree::spacings[SIZE_P] + bottom))
  {
    (*page) ++;
    *y = top;
    if (HTMLDOC::verbosity_)
      progress_show("Formatting page %d", *page);
  }

  if (t->markup == MARKUP_H1 && !in_title_page_)
    page_chapters_[*page] = current_heading->get_text();

  if ((page_headings_[*page] == NULL || t->markup == MARKUP_H1) && !in_title_page_)
    page_headings_[*page] = current_heading->get_text();

  if ((t->markup - MARKUP_H1) < toc_levels_ && !in_title_page_)
  {
    DEBUG_printf(("H%d: heading_pages_[%d] = %d\n", t->markup - MARKUP_H1 + 1,
                  num_headings_, *page - 1));
    heading_pages_[num_headings_] = *page - chapter_starts_[1] + 1;
    heading_tops_[num_headings_]  = (int)(*y + 2 * HDtree::spacings[SIZE_P]);
    num_headings_ ++;
  }

  parse_paragraph(t, left, right, bottom, top, x, y, page, needspace);

  if (t->halignment == ALIGN_RIGHT && t->markup == MARKUP_H1 && output_book_ &&
      !in_title_page_)
  {
    // Special case - chapter heading for users manual...
    *y = bottom + 0.5f * (top - bottom);
  }
}


//
// 'HTMLDOC::parse_paragraph()' - Parse a paragraph tree and produce rendering list
//                       output.
//

void
HTMLDOC::parse_paragraph(HDtree *t,	// I - Tree to parse
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
  HDtree	*flat,
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
  HDrender	*r;
  uchar		*align,
		*link;
  float		rgb[3];
  uchar		line[10240],
		*lineptr;
  HDtree	*linetype;
  float		linex,
		linewidth;
  int		firstline;


  if (*page > MAX_PAGES)
    return;

  DEBUG_printf(("parse_paragraph(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child)
    flat = t->child->flatten();
  else
    flat = NULL;

  image_left  = left;
  image_right = right;
  image_y     = 0;

  if (flat == NULL)
    DEBUG_puts("parse_paragraph: flat == NULL!");

  if (*y < top && needspace)
    *y -= HDtree::spacings[SIZE_P];

  // First scan for images with left/right alignment tags...
  for (temp = flat, prev = NULL; temp != NULL;)
  {
    if (temp->markup == MARKUP_IMG &&
        (align = temp->var((uchar *)"ALIGN")))
    {
      if (strcasecmp((char *)align, "LEFT") == 0)
      {
        if (*y < (bottom + temp->height))
        {
	  (*page) ++;
	  *y = top;

	  if (HTMLDOC::verbosity_)
	    progress_show("Formatting page %d", *page);
        }

        new_render(*page, RENDER_IMAGE, image_left, *y - temp->height,
	           temp->width, temp->height,
		   HDimage::find((char *)temp->var((uchar *)"SRC"),
		                 !output_color_));

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

	  if (HTMLDOC::verbosity_)
	    progress_show("Formatting page %d", *page);
        }

        image_right -= temp->width;

        new_render(*page, RENDER_IMAGE, image_right, *y - temp->height,
                   temp->width, temp->height,
		   HDimage::find((char *)temp->var((uchar *)"SRC"),
		                 !output_color_));

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

  // Then format the text and inline images...
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
          temp_width -= HDtree::widths[temp->typeface][temp->style][' '] *
                        HDtree::sizes[temp->size];

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
        temp_height = temp->height * HDtree::spacings[0] / HDtree::sizes[0];
      else
      {
        switch (temp->valignment)
	{
	  case ALIGN_TOP :
              temp_height = temp->height;
	      break;
	  case ALIGN_MIDDLE :
	      if ((0.5f * temp->height) > HDtree::sizes[t->size])
	        temp_height = temp->height;
	      else
	        temp_height = 0.5f * temp->height + HDtree::sizes[t->size];
              break;
	  case ALIGN_BOTTOM :
	      temp_height = temp->height + HDtree::sizes[t->size];
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

      if (HTMLDOC::verbosity_)
        progress_show("Formatting page %d", *page);
    }

    firstline = 0;

    if (height == 0.0)
      height = spacing;

    if (start != NULL && start->markup == MARKUP_NONE && start->data[0] == ' ')
    {
      // Remove leading space...
      strcpy((char *)start->data, (char *)start->data + 1);
      temp_width = HDtree::widths[start->typeface][start->style][' '] *
                   HDtree::sizes[start->size];
      start->width -= temp_width;
      width        -= temp_width;
    }

    if (prev != NULL && prev->markup == MARKUP_NONE &&
        prev->data[strlen((char *)prev->data) - 1] == ' ')
    {
      // Remove trailing space...
      prev->data[strlen((char *)prev->data) - 1] = '\0';
      temp_width = HDtree::widths[prev->typeface][prev->style][' '] *
                   HDtree::sizes[prev->size];
      prev->width -= temp_width;
      width       -= temp_width;
    }

    if (*y < (height + bottom))
    {
      (*page) ++;
      *y = top;

      if (HTMLDOC::verbosity_)
        progress_show("Formatting page %d", *page);
    }

    *y -= height;

    if (HTMLDOC::verbosity_)
      progress_update(100 - (int)(100 * (*y) / page_print_length_));

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
        link = temp->link->var((uchar *)"HREF");

        // Add a page link...
	if (file_method((char *)link) == NULL)
	{
	  if (file_target((char *)link) != NULL)
	    link = (uchar *)file_target((char *)link) - 1; // Include # sign
	  else
	    link = (uchar *)file_basename((char *)link);
	}

	new_render(*page, RENDER_LINK, *x, *y, temp->width,
	           temp->height, link);

	if (ps_level_ == 0)
	{
	  temp->red   = (int)(link_rgb_[0] * 255.0);
	  temp->green = (int)(link_rgb_[1] * 255.0);
	  temp->blue  = (int)(link_rgb_[2] * 255.0);

          if (link_style_)
	    new_render(*page, RENDER_BOX, *x, *y - 1, temp->width, 0,
	               link_rgb_);
	}
      }

      // See if we are doing a run of characters in a line and need to
      // output this run...
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
	r->data.text.size     = HDtree::sizes[linetype->size];
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
            if ((link = temp->var((uchar *)"NAME")) != NULL)
            {
              // Add a target link...
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
		       HDimage::find((char *)temp->var((uchar *)"SRC"), !output_color_));
            whitespace = 0;
	    break;
      }

      *x += temp->width;
      prev = temp;
      temp = temp->next;
      if (prev != linetype)
        free(prev);
    }

    // See if we have a run of characters that hasn't been output...
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
      r->data.text.size     = HDtree::sizes[linetype->size];
      memcpy(r->data.text.rgb, rgb, sizeof(rgb));

      if (linetype->superscript)
        r->y += height - linetype->height;
      else if (linetype->subscript)
        r->y -= height - linetype->height;

      free(linetype);
    }

    // Update the margins after we pass below the images...
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
// 'HTMLDOC::parse_pre()' - Parse preformatted text and produce rendering list output.
//

void
HTMLDOC::parse_pre(HDtree *t,		// I - Tree to parse
          float  left,		// I - Left margin
          float  right,		// I - Printable width
          float  bottom,	// I - Bottom margin
          float  top,		// I - Printable top
          float  *x,		// IO - X position
          float  *y,		// IO - Y position
          int    *page,		// IO - Page #
          int    needspace)	// I - Need whitespace?
{
  HDtree	*flat, *next;
  uchar		*link,
		line[10240],
		*lineptr,
		*dataptr;
  int		col;
  float		width,
		rgb[3];
  HDrender	*r;


  REF(right);

  DEBUG_printf(("parse_pre(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;

  if (*y < top && needspace)
    *y -= HDtree::spacings[SIZE_P];

  col  = 0;
  flat = t->child->flatten();

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
      if (*y < (HDtree::spacings[t->size] + bottom))
      {
        (*page) ++;
        *y = top;

	if (HTMLDOC::verbosity_)
	  progress_show("Formatting page %d", *page);
      }

      *x = left;
      *y -= HDtree::sizes[t->size];

      if (HTMLDOC::verbosity_)
        progress_update(100 - (int)(100 * (*y) / page_print_length_));
    }

    if (flat->link != NULL)
    {
      link = flat->link->var((uchar *)"HREF");

      // Add a page link...
      if (file_method((char *)link) == NULL)
      {
	if (file_target((char *)link) != NULL)
	  link = (uchar *)file_target((char *)link) - 1; // Include # sign
	else
	  link = (uchar *)file_basename((char *)link);
      }

      new_render(*page, RENDER_LINK, *x, *y, flat->width,
	         flat->height, link);

      if (ps_level_ == 0)
      {
        memcpy(rgb, link_rgb_, sizeof(rgb));

	flat->red   = (int)(link_rgb_[0] * 255.0);
	flat->green = (int)(link_rgb_[1] * 255.0);
	flat->blue  = (int)(link_rgb_[2] * 255.0);

        if (link_style_)
	  new_render(*page, RENDER_BOX, *x, *y - 1, flat->width, 0,
	             link_rgb_);
      }
    }

    switch (flat->markup)
    {
      case MARKUP_A :
          if ((link = flat->var((uchar *)"NAME")) != NULL)
          {
            // Add a target link...
            add_link(link, *page, (int)(*y + 6 * t->height));
          }
          break;

      case MARKUP_BR :
          col = 0;
          *y  -= HDtree::spacings[t->size] - HDtree::sizes[t->size];
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
          r->data.text.size     = HDtree::sizes[flat->size];
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
            *y  -= HDtree::spacings[t->size] - HDtree::sizes[t->size];
          }
          break;

      case MARKUP_IMG :
	  new_render(*page, RENDER_IMAGE, *x, *y, flat->width, flat->height,
		     HDimage::find((char *)flat->var((uchar *)"SRC"),
		                   !output_color_));

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
//
// 'HTMLDOC::parse_table()' - Parse a table and produce rendering output.
//

void
HTMLDOC::parse_table(HDtree *t,		// I - Tree to parse
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
  uchar		*v;
  HDtree	*temprow,
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
  HDrender	*cell_start[MAX_COLUMNS];	// Start of the content for a cell in the row
  HDrender	*cell_end[MAX_COLUMNS];		// End of the content for a cell in a row
  uchar		*bgcolor;
  float		rgb[3],
		bgrgb[3];


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("parse_table(t=%08x, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (t->child == NULL)
    return;   // Empty table...

  rgb[0] = t->red / 255.0f;
  rgb[1] = t->green / 255.0f;
  rgb[2] = t->blue / 255.0f;

  // Figure out the # of rows, columns, and the desired widths...
  memset(col_spans, 0, sizeof(col_spans));
  memset(col_widths, 0, sizeof(col_widths));
  memset(col_swidths, 0, sizeof(col_swidths));
  memset(col_mins, 0, sizeof(col_mins));
  memset(col_smins, 0, sizeof(col_smins));
  memset(col_prefs, 0, sizeof(col_prefs));

  if ((v = t->var((uchar *)"WIDTH")) != NULL)
  {
    if (v[strlen((char *)v) - 1] == '%')
      table_width = atof((char *)v) * (right - left) / 100.0f;
    else
      table_width = atoi((char *)v) * page_print_width_ / browser_width_;
  }
  else
    table_width = right - left;

  if ((v = t->var((uchar *)"CELLPADDING")) != NULL)
    cellpadding = atoi((char *)v);
  else
    cellpadding = 1.0f;

  if ((v = t->var((uchar *)"CELLSPACING")) != NULL)
    cellspacing = atoi((char *)v);
  else
    cellspacing = 0.0f;

  if ((v = t->var((uchar *)"BORDER")) != NULL)
  {
    if ((border = atof((char *)v)) == 0.0 && v[0] != '0')
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
	  cells = (HDtree ***)malloc(sizeof(HDtree **) * alloc_rows);
	else
	  cells = (HDtree ***)realloc(cells, sizeof(HDtree **) * alloc_rows);

        if (cells == (HDtree ***)0)
	{
	  progress_error("Unable to allocate memory for table!");
	  return;
	}
      }	

      if ((cells[num_rows] = (HDtree **)calloc(sizeof(HDtree *), MAX_COLUMNS)) == NULL)
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
          if ((v = tempcol->var((uchar *)"COLSPAN")) != NULL)
            colspan = atoi((char *)v);
          else
            colspan = 1;

          if ((v = tempcol->var((uchar *)"ROWSPAN")) != NULL)
            row_spans[col] = atoi((char *)v);

          DEBUG_printf(("num_rows = %d, col = %d, colspan = %d (%s)\n",
	                num_rows, col, colspan, v));

          if ((v = tempcol->var((uchar *)"WIDTH")) != NULL &&
	      colspan == 1)
	  {
            if (v[strlen((char *)v) - 1] == '%')
              col_width = atof((char *)v) * table_width / 100.0f -
	                  2.0 * (cellpadding + cellspacing + border);
            else
              col_width = atoi((char *)v) * page_print_width_ / browser_width_;

            col_min  = col_width;
	    col_pref = col_width;
	  }
	  else
	  {
	    if (tempcol->child)
	      flat = tempcol->child->flatten();
	    else
	      flat = NULL;

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

	    if (tempcol->var((uchar *)"NOWRAP") != NULL &&
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


  // Now figure out the width of the table...
  if ((v = t->var((uchar *)"WIDTH")) != NULL)
  {
    if (v[strlen((char *)v) - 1] == '%')
      width = atof((char *)v) * (right - left) / 100.0f;
    else
      width = atoi((char *)v) * page_print_width_ / browser_width_;
  }
  else
  {
    for (col = 0, width = 0.0; col < num_cols; col ++)
      width += col_prefs[col];

    width += 2 * (border + cellpadding + cellspacing) * num_cols;

    if (width > (right - left))
      width = right - left;
  }

  // Compute the width of each column based on the printable width.
  actual_width  = 2 * (border + cellpadding + cellspacing) * num_cols;
  regular_width = (width - actual_width) / num_cols;

  // The first pass just handles columns with a specified width...
  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (col_widths[col] > 0.0f)
    {
      if (col_mins[col] > col_widths[col])
        col_widths[col] = col_mins[col];

      actual_width += col_widths[col];
    }
    else
      regular_cols ++;

  // Pass two uses the "preferred" width whenever possible, and the
  // minimum otherwise...
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

  // Pass three enforces any hard or minimum widths for COLSPAN'd
  // columns...
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

  // The final pass divides up the remaining space amongst the columns...
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

  // Now render the whole table...
  if (*y < top && needspace)
    *y -= HDtree::spacings[SIZE_P];

  memset(row_spans, 0, sizeof(row_spans));
  memset(cell_start, 0, sizeof(cell_start));
  memset(cell_end, 0, sizeof(cell_end));
  memset(cell_height, 0, sizeof(cell_height));

  for (row = 0; row < num_rows; row ++)
  {
    if (cells[row][0] != NULL)
    {
      // Do page comments...
      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->markup == MARKUP_COMMENT)
        parse_comment(cells[row][0]->parent->prev,
                      left, right, bottom + border + cellpadding,
                      top - border - cellpadding, x, y, page, NULL, 0);

      // Get height...
      if ((v = t->var((uchar *)"HEIGHT")) == NULL)
	if ((v = cells[row][0]->parent->var((uchar *)"HEIGHT")) == NULL)
	  for (col = 0; col < num_cols; col ++)
	    if ((v = cells[row][col]->var((uchar *)"HEIGHT")) != NULL)
	      break;
    }

    if (cells[row][0] != NULL && v != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (v[strlen((char *)v) - 1] == '%')
	temp_height = atof((char *)v) * 0.01f * page_print_length_;
      else
        temp_height = atof((char *)v) * page_print_width_ / browser_width_;

      if (t->var((uchar *)"HEIGHT") != NULL)
        temp_height /= num_rows;

      temp_height -= 2 * (border + cellpadding);
    }
    else
      temp_height = HDtree::spacings[SIZE_P];

    if (*y < (bottom + 2 * (border + cellpadding) + temp_height) &&
        temp_height < (top - bottom - 2 * (border + cellpadding)))
    {
      *y = top;
      (*page) ++;

      if (HTMLDOC::verbosity_)
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
        if ((v = cells[row][col]->var((uchar *)"ROWSPAN")) != NULL)
          row_spans[col] = atoi((char *)v);

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
	cell_start[col] = endpages_[*page];
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
        cell_end[col]     = endpages_[*page];

        if (cell_start[col] == NULL)
	  cell_start[col] = pages_[*page];

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

    // Do the vertical alignment
    if (do_valign)
    {
      if (v != NULL)
      {
        // Hardcode the row height...
        if (v[strlen((char *)v) - 1] == '%')
	  temp_height = atof((char *)v) * 0.01f * page_print_length_;
	else
          temp_height = atof((char *)v) * page_print_width_ / browser_width_;

	if (t->var((uchar *)"HEIGHT") != NULL)
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
        HDrender	*p;
        float		delta_y;


        for (colspan = 1; (col + colspan) < num_cols; colspan ++)
          if (cells[row][col] != cells[row][col + colspan])
            break;

        colspan --;

        if (cell_start[col] == NULL || cell_start[col] == cell_end[col] ||
	    row_spans[col] > 1)
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

      if ((bgcolor = cells[row][col]->var((uchar *)"BGCOLOR")) == NULL)
        if ((bgcolor = cells[row][col]->parent->var((uchar *)"BGCOLOR")) == NULL)
	  bgcolor = t->var((uchar *)"BGCOLOR");

      if (bgcolor != NULL)
        get_color(bgcolor, bgrgb);

      if (cell_page[col] != cell_endpage[col])
      {
        // Crossing a page boundary...
        if (border > 0)
	{
	  // +---+---+---+
	  // |   |   |   |
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
	  // |   |   |   |
	  // |   |   |   |
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
	  // |   |   |   |
	  // +---+---+---+
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

  // Free memory for the table...
  if (num_rows > 0)
  {
    for (row = 0; row < num_rows; row ++)
      free(cells[row]);

    free(cells);
  }
}
#undef DEBUG_printf
#define DEBUG_printf(x)


//
// 'HTMLDOC::parse_list()' - Parse a list entry and produce rendering output.
//

void
HTMLDOC::parse_list(HDtree *t,		// I - Tree to parse
           float  left,		// I - Left margin
           float  right,	// I - Printable width
           float  bottom,	// I - Bottom margin
           float  top,		// I - Printable top
           float  *x,		// IO - X position
           float  *y,		// IO - Y position
           int    *page,	// IO - Page #
           int    needspace)	// I - Need whitespace?
{
  uchar		number[255];	// List number (for numbered types)
  uchar		*value;		// VALUE= variable
  int		typeface;	// Typeface of list number
  float		width;		// Width of list number
  HDrender	*r;		// Render primitive
  int		oldpage;	// Old page value
  float		oldy;		// Old Y value
  float		tempx;		// Temporary X value


  DEBUG_printf(("parse_list(t=%08x, left=%d, right=%d, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, *x, *y, *page));

  if (needspace && *y < top)
  {
    *y        -= HDtree::spacings[t->size];
    needspace = 0;
  }

  oldy    = *y;
  oldpage = *page;
  r       = endpages_[*page];
  tempx   = *x;

  if (t->indent == 0)
  {
    // Adjust left margin when no UL/OL/DL is being used...
    left  += HDtree::sizes[t->size];
    tempx += HDtree::sizes[t->size];
  }

  parse_doc(t->child, left, right, bottom, top, &tempx, y, page, NULL,
            &needspace);

  // Handle when paragraph wrapped to new page...
  if (*page != oldpage)
  {
    // First see if anything was added to the old page...
    if ((r != NULL && r->next == NULL) || endpages_[oldpage] == NULL)
    {
      // No, put the symbol on the next page...
      oldpage = *page;
      oldy    = top;
    }
  }
    
  if ((value = t->var((uchar *)"VALUE")) != NULL)
  {
    if (isdigit(value[0]))
      list_values_[t->indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values_[t->indent] = value[0] - 'A' + 1;
    else
      list_values_[t->indent] = value[0] - 'a' + 1;
  }

  switch (list_types_[t->indent])
  {
    case 'a' :
    case 'A' :
    case '1' :
    case 'i' :
    case 'I' :
        strcpy((char *)number, format_number(list_values_[t->indent],
	                                     list_types_[t->indent]));
        strcat((char *)number, ". ");
        typeface = t->typeface;
        break;

    default :
        sprintf((char *)number, "%c ", list_types_[t->indent]);
        typeface = TYPE_SYMBOL;
        break;
  }

  width = get_width(number, typeface, t->style, t->size);

  r = new_render(oldpage, RENDER_TEXT, left - width, oldy - HDtree::sizes[t->size],
                 width, HDtree::spacings[t->size], number);
  r->data.text.typeface = typeface;
  r->data.text.style    = t->style;
  r->data.text.size     = HDtree::sizes[t->size];
  r->data.text.rgb[0]   = t->red / 255.0f;
  r->data.text.rgb[1]   = t->green / 255.0f;
  r->data.text.rgb[2]   = t->blue / 255.0f;

  list_values_[t->indent] ++;
}


//
// 'HTMLDOC::init_list()' - Initialize the list type and value as necessary.
//

void
HTMLDOC::init_list(HDtree *t)		// I - List entry
{
  uchar		*type,		// TYPE= variable
		*value;		// VALUE= variable
  static uchar	*symbols = (uchar *)"\327\267\250\340";


  if ((type = t->var((uchar *)"TYPE")) != NULL)
  {
    if (strlen((char *)type) == 1)
      list_types_[t->indent] = type[0];
    else if (strcasecmp((char *)type, "disc") == 0 ||
             strcasecmp((char *)type, "circle") == 0)
      list_types_[t->indent] = symbols[1];
    else
      list_types_[t->indent] = symbols[2];
  }
  else if (t->markup == MARKUP_UL)
    list_types_[t->indent] = symbols[t->indent & 3];
  else if (t->markup == MARKUP_OL)
    list_types_[t->indent] = '1';

  if ((value = t->var((uchar *)"VALUE")) == NULL)
    value = t->var((uchar *)"START");

  if (value != NULL)
  {
    if (isdigit(value[0]))
      list_values_[t->indent] = atoi((char *)value);
    else if (isupper(value[0]))
      list_values_[t->indent] = value[0] - 'A' + 1;
    else
      list_values_[t->indent] = value[0] - 'a' + 1;
  }
  else if (t->markup == MARKUP_OL)
    list_values_[t->indent] = 1;
}


//
// 'HTMLDOC::parse_comment()' - Parse a comment for HTMLDOC comments.
//

void
HTMLDOC::parse_comment(HDtree *t,	// I - Tree to parse
              float  left,	// I - Left margin
              float  right,	// I - Printable width
              float  bottom,	// I - Bottom margin
              float  top,	// I - Printable top
              float  *x,	// IO - X position
              float  *y,	// IO - Y position
              int    *page,	// IO - Page #
	      HDtree *para,	// I - Current paragraph
	      int    needspace)	// I - Need whitespace?
{
  const char	*comment;	// Comment text


  if (t->data == NULL)
    return;

  for (comment = (const char *)t->data;
       *comment && isspace(*comment);
       comment ++);		// Skip leading whitespace...

  if (strncasecmp(comment, "PAGE BREAK", 10) == 0 ||
      strncasecmp(comment, "NEW PAGE", 8) == 0)
  {
    // <!-- PAGE BREAK --> and <!-- NEW PAGE --> generate a page
    // break...
    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      delete para->child;
    }

    (*page) ++;
    if (HTMLDOC::verbosity_)
      progress_show("Formatting page %d", *page);
    *x = left;
    *y = top;
  }
  else if (strncasecmp(comment, "NEW SHEET", 9) == 0)
  {
    // <!-- NEW SHEET --> generate a page break to a new sheet...
    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      delete para->child;
    }

    (*page) ++;
    if (page_duplex_ && ((*page) & 1))
      (*page) ++;

    if (HTMLDOC::verbosity_)
      progress_show("Formatting page %d", *page);
    *x = left;
    *y = top;
  }
  else if (strncasecmp(comment, "HALF PAGE", 9) == 0)
  {
    // <!-- HALF PAGE --> Go to the next half page.  If in the
    // top half of a page, go to the bottom half.  If in the
    // bottom half, go to the next page.
    float halfway;


    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      delete para->child;
    }

    halfway = 0.5f * (top + bottom);

    if (*y <= halfway)
    {
      (*page) ++;
      if (HTMLDOC::verbosity_)
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
    // <!-- NEED amount --> generate a page break if there isn't
    // enough remaining space...
    if (para != NULL && para->child != NULL)
    {
      parse_paragraph(para, left, right, bottom, top, x, y, page, needspace);
      delete para->child;
    }

    if ((*y - get_measurement(comment + 5)) < bottom)
    {
      (*page) ++;

      if (HTMLDOC::verbosity_)
	progress_show("Formatting page %d", *page);
      *y = top;
    }

    *x = left;
  }
}


//
// 'HTMLDOC::find_background()' - Find the background image/color for the given document.
//

void
HTMLDOC::find_background(HDtree *t)	// I - Document to search
{
  uchar		*v;		// BGCOLOR/BACKGROUND variable


  // First see if the --bodycolor or --bodyimage options have been
  // specified...
  if (body_file_[0] != '\0')
  {
    background_image_ = HDimage::find(body_file_, !output_color_);
    return;
  }
  else if (body_color_[0] != '\0')
  {
    get_color((uchar *)body_color_, background_rgb_);
    return;
  }

  // If not, search the document tree...
  while (t != NULL && background_image_ == NULL &&
         background_rgb_[0] == 1.0 && background_rgb_[1] == 1.0 &&
	 background_rgb_[2] == 1.0)
  {
    if (t->markup == MARKUP_BODY)
    {
      if ((v = t->var((uchar *)"BACKGROUND")) != NULL)
        background_image_ = HDimage::find((char *)v, !output_color_);

      if ((v = t->var((uchar *)"BGCOLOR")) != NULL)
        get_color(v, background_rgb_);
    }

    if (t->child != NULL)
      find_background(t->child);

    t = t->next;
  }
}


//
// 'HTMLDOC::add_link()' - Add a named link...
//

void
HTMLDOC::add_link(uchar *name,	// I - Name of link
         int   page,	// I - Page #
         int   top)	// I - Y position
{
  HDlink	*temp;	// New name


  if (name == NULL)
    return;

  if ((temp = find_link(name)) != NULL)
  {
    temp->page = page - 1;
    temp->top  = top;
  }
  else if (num_links_ < MAX_LINKS)
  {
    temp = links_ + num_links_;
    num_links_ ++;

    strncpy((char *)temp->name, (char *)name, sizeof(temp->name) - 1);
    temp->name[sizeof(temp->name) - 1] = '\0';
    temp->page = page - 1;
    temp->top  = top;

    if (num_links_ > 1)
      qsort(links_, num_links_, sizeof(HDlink),
            (int (*)(const void *, const void *))compare_links);
  }
}


//
// 'HTMLDOC::find_link()' - Find a named link...
//

HDlink *
HTMLDOC::find_link(uchar *name)	// I - Name to find
{
  HDlink	key,	// Search key
		*match;	// Matching name entry


  if (name == NULL || num_links_ == 0)
    return (NULL);

  if (name[0] == '#')
    name ++;

  strncpy((char *)key.name, (char *)name, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';
  match = (HDlink *)bsearch(&key, links_, num_links_, sizeof(HDlink),
                            (int (*)(const void *, const void *))compare_links);

  return (match);
}


//
// 'HTMLDOC::compare_links()' - Compare two named links_.
//

int			// O - 0 = equal, -1 or 1 = not equal
HTMLDOC::compare_links(HDlink *n1,	// I - First name
              HDlink *n2)	// I - Second name
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


//
// 'HTMLDOC::get_width()' - Get the width of a string in points.
//

float			// O - Width in points
HTMLDOC::get_width(uchar *s,		// I - String to scan
          int   typeface,	// I - Typeface code
          int   style,		// I - Style code
          int   size)		// I - Size
{
  uchar	*ptr;			// Current character
  float	width;			// Current width


  DEBUG_printf(("get_width(\"%s\", %d, %d, %d)\n", s == NULL ? "(null)" : s,
                typeface, style, size));

  if (s == NULL)
    return (0.0);

  for (width = 0.0, ptr = s; *ptr != '\0'; ptr ++)
    width += HDtree::widths[typeface][style][*ptr];

  return (width * HDtree::sizes[size]);
}


//
// 'HTMLDOC::get_title()' - Get the title string for a document.
//

uchar *				// O - Title string
HTMLDOC::get_title(HDtree *doc)	// I - Document
{
  uchar	*temp;


  while (doc != NULL)
  {
    if (doc->markup == MARKUP_TITLE)
      return (doc->child->get_text());
    else if (doc->child != NULL)
      if ((temp = doc->child->get_text()) != NULL)
        return (temp);
    doc = doc->next;
  }

  return (NULL);
}


//
// End of "$Id: format.cxx,v 1.1 2000/10/16 03:25:06 mike Exp $".
//
