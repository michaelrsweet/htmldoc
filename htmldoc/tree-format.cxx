//
// "$Id: tree-format.cxx,v 1.4 2003/01/02 04:36:07 mike Exp $"
//
//   HTML formatting routines for HTMLDOC, a HTML document processing program.
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

#include "htmldoc.h"
#include "hdstring.h"
#include <math.h>


//
// 'hdTree::format()' - Format a document.
//

void
hdTree::format(hdStyleSheet *css,		// I  - Style sheet
               hdMargin     *m,			// IO - Margins
               float        &x,			// IO - Current X position
               float        &y,			// IO - Current Y position
	       int          &page)		// IO - Current page
{
  float		linewidth,			// Current line width
		fragwidth,			// Fragment width
		linespacing,			// Line spacing
		ascender,			// Ascender
		descender;			// Descender
  hdTree	*frag,				// Start of fragment
		*block,				// Start of block
		*temp;				// Current child
  float		tempx,				// Temporary X position
		tempy;				// Temporary Y position
  int		temppage;			// Temporary page


  // Don't do anything if there are no child nodes...
  if (child == NULL)
  {
    compute_size(css);
    nodebreak = HD_NODEBREAK_NONE;

    return;
  }

  x          = 0.0;
  width      = 0.0;
  height     = 0.0;
  temp       = child;
  frag       = child;
  fragwidth  = 0.0;
  ascender   = 0.0;
  descender  = 0.0;
  block      = NULL;

  if (style)
    linewidth = m->width() -
                style->get_format_spacing(HD_POS_LEFT) -
                style->get_format_spacing(HD_POS_RIGHT);
  else
    linewidth = m->width();

  while (temp != NULL)
  {
    printf("temp = %p\n", temp);

    // If there is whitespace here, process the fragment...
    temp->nodebreak = HD_NODEBREAK_NONE;

    if ((temp->whitespace || hdElIsGroup(temp->element)) && frag != temp)
    {
      if ((fragwidth + x) > linewidth)
      {
        frag->nodebreak = HD_NODEBREAK_LINE;
	x               = 0.0;
	y               += ascender + descender;
	ascender        = 0.0;
	descender       = 0.0;

        if (y >= m->length())
	{
	  y = 0.0;
	  page ++;
	}

	m->clear(y, page);

	if (style)
	  linewidth = m->width() -
                      style->get_format_spacing(HD_POS_LEFT) -
                      style->get_format_spacing(HD_POS_RIGHT);
	else
	  linewidth = m->width();

        if (frag->whitespace)
	  fragwidth -= frag->style->font->get_width(" ") *
	               frag->style->font_size;
      }

      x         += fragwidth;
      fragwidth = 0.0;
      frag      = temp;
    }

    // Get the size of this node, if applicable...
    switch (temp->element)
    {
      case HD_ELEMENT_NONE :
          fragwidth += temp->width;

	  if (temp->whitespace && x > 0.0)
	    fragwidth += temp->style->font->get_width(" ") *
	                 temp->style->font_size;

          switch (temp->style->vertical_align)
	  {
	    case HD_VERTICALALIGN_BASELINE :
	    case HD_VERTICALALIGN_TOP :
	    case HD_VERTICALALIGN_TEXT_TOP :
	    case HD_VERTICALALIGN_SUPER :
		if (temp->height > ascender)
		  ascender = temp->height;

        	linespacing = temp->style->line_height - temp->height;

		if (linespacing > descender)
		  descender = linespacing;
	        break;

	    case HD_VERTICALALIGN_MIDDLE :
		if ((temp->height * 0.5) > ascender)
		  ascender = temp->height * 0.5;

		if ((temp->height * 0.5) > descender)
		  descender = temp->height * 0.5;
	        break;

	    case HD_VERTICALALIGN_SUB :
	    case HD_VERTICALALIGN_BOTTOM :
	    case HD_VERTICALALIGN_TEXT_BOTTOM :
		if (temp->height > descender)
		  descender = temp->height;
	        break;
	  }
          break;

      case HD_ELEMENT_IMG :
          switch (temp->style->float_)
	  {
	    case HD_FLOAT_NONE :
		fragwidth += temp->width;

		if (temp->whitespace && x > 0.0)
		  fragwidth += temp->style->font->get_width(" ") *
	                       temp->style->font_size;

                switch (temp->style->vertical_align)
		{
		  case HD_VERTICALALIGN_BASELINE :
		  case HD_VERTICALALIGN_TOP :
		  case HD_VERTICALALIGN_TEXT_TOP :
		  case HD_VERTICALALIGN_SUPER :
		      if (temp->height > ascender)
		        ascender = temp->height;
	              break;

		  case HD_VERTICALALIGN_MIDDLE :
		      if ((temp->height * 0.5) > ascender)
		        ascender = temp->height * 0.5;

		      if ((temp->height * 0.5) > descender)
		        descender = temp->height * 0.5;
	              break;

		  case HD_VERTICALALIGN_SUB :
		  case HD_VERTICALALIGN_BOTTOM :
		  case HD_VERTICALALIGN_TEXT_BOTTOM :
		      if (temp->height > descender)
		        descender = temp->height;
	              break;
		}
		break;

	    case HD_FLOAT_LEFT :
	        temp->nodebreak = HD_NODEBREAK_LEFT;

		m->push(m->left() + temp->width, m->right(), y + temp->height);
		break;

	    case HD_FLOAT_RIGHT :
	        temp->nodebreak = HD_NODEBREAK_RIGHT;

		m->push(m->left(), m->right() + temp->width, y + temp->height);
		break;
	  }
          break;

      case HD_ELEMENT_SPACER :
          fragwidth += temp->width;

	  if (temp->whitespace && x > 0.0)
	    fragwidth += temp->style->font->get_width(" ") *
	                 temp->style->font_size;

	  if (temp->height > ascender)
	    ascender = temp->height;
          break;

      case HD_ELEMENT_TABLE :
          switch (temp->style->float_)
	  {
	    case HD_FLOAT_NONE :
        	temp->format_table(css, m, x, y, page);

        	fragwidth += temp->width;

		if (temp->height > ascender)
		  ascender = temp->height;
		break;

	    case HD_FLOAT_LEFT :
        	tempx    = x;
		tempy    = y;
		temppage = page;

        	temp->format_table(css, m, tempx, tempy, temppage);
	        temp->nodebreak = HD_NODEBREAK_LEFT;

		m->push(m->left() + temp->width, m->right(), tempy, temppage);
		break;

	    case HD_FLOAT_RIGHT :
        	tempx    = x;
		tempy    = y;
		temppage = page;

        	temp->format_table(css, m, tempx, tempy, temppage);
	        temp->nodebreak = HD_NODEBREAK_LEFT;

		m->push(m->left(), m->right() + temp->width, tempy, temppage);
		break;
          }
	  break;

      default :
          if (hdElIsGroup(temp->element))
	  {
	    x = 0.0;

	    if (block)
	      y += block->style->get_format_spacing(HD_POS_BOTTOM);

            block = temp;

	    y += block->style->get_format_spacing(HD_POS_TOP);

            if (y >= m->length())
	    {
	      y = 0.0;
	      page ++;
	    }

	    m->clear(y, page);

	    if (style)
	      linewidth = m->width() -
                	  style->get_format_spacing(HD_POS_LEFT) -
                	  style->get_format_spacing(HD_POS_RIGHT);
	    else
	      linewidth = m->width();
	  }
          break;
    }

    // Find the next logical node...
    if (temp->child != NULL)
      temp = temp->child;
    else
    {
      while (temp != NULL && temp->next == NULL)
      {
        temp = temp->parent;
        if (temp == this)
          temp = NULL;
      }

      if (temp)
        temp = temp->next;
    }
  }

  // Handle any remaining text fragment...
  if ((fragwidth + x) > linewidth)
  {
    frag->nodebreak = HD_NODEBREAK_LINE;
    x               = 0.0;
    y               += ascender + descender;

    if (y >= m->length())
    {
      y = 0.0;
      page ++;
    }

    m->clear(y, page);
  }
}


//
// 'hdTree::format_block()' - Format a block.
//

void
hdTree::format_block(hdStyleSheet *css,		// I  - Style sheet
                     hdMargin     *m,		// IO - Margins
                     float        &x,		// IO - Current X position
                     float        &y,		// IO - Current Y position
		     int          &page)	// IO - Current page
{
}


//
// 'hdTree::format_comment()' - Format a comment.
//

void
hdTree::format_comment(hdStyleSheet *css,	// I  - Style sheet
                       hdMargin     *m,		// IO - Margins
                       float        &x,		// IO - Current X position
                       float        &y,		// IO - Current Y position
		       int          &page)	// IO - Current page
{
}


//
// 'hdTree::format_contents()' - Format a contents.
//

void
hdTree::format_contents(hdStyleSheet *css,	// I  - Style sheet
                	hdMargin     *m,	// IO - Margins
                	float        &x,	// IO - Current X position
                	float        &y,	// IO - Current Y position
			int          &page)	// IO - Current page
{
}


//
// 'hdTree::format_image()' - Format an image.
//

void
hdTree::format_image(hdStyleSheet *css,		// I  - Style sheet
                     hdMargin     *m,		// IO - Margins
                     float        &x,		// IO - Current X position
                     float        &y,		// IO - Current Y position
		     int          &page)	// IO - Current page
{
}


//
// 'hdTree::format_index()' - Format an index.
//

void
hdTree::format_index(hdStyleSheet *css,		// I  - Style sheet
                     hdMargin     *m,		// IO - Margins
                     float        &x,		// IO - Current X position
                     float        &y,		// IO - Current Y position
		     int          &page)	// IO - Current page
{
}


//
// 'hdTree::format_list()' - Format a list.
//

void
hdTree::format_list(hdStyleSheet *css,		// I  - Style sheet
                    hdMargin     *m,		// IO - Margins
                    float        &x,		// IO - Current X position
                    float        &y,		// IO - Current Y position
		    int          &page)		// IO - Current page
{
}


//
// End of "$Id: tree-format.cxx,v 1.4 2003/01/02 04:36:07 mike Exp $".
//
