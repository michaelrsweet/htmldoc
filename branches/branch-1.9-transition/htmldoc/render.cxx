//
// "$Id: render.cxx,v 1.14 2004/02/03 02:55:28 mike Exp $"
//
//   Core rendering methods for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 1997-2004 by Easy Software Products.
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
#include "debug.h"
#include "htmldoc.h"
#include "hdstring.h"


//
// 'hdRender::finish_document()' -
//

void
hdRender::finish_document(const char *author,
                          const char *creator,
		          const char *copyright,
			  const char *keywords)
{
}


#if 0
//
// 'hdRender::render_block()' - Parse a block of text.
//

void
hdRender::render_block(hdTree   *block,	// I  - Block node
                       hdMargin &m,	// I  - Current margins
		       float    &x,	// IO - Current X position
		       float    &y,	// IO - Current Y position
        	       int      &page)	// IO - Current page
{
  hdTree	*t,			// Current node
		*start,			// First node in line
		*end,			// Last node in line
		*next,			// Next node
		*prev,			// Previous node
		*bend;			// End of block
  float		tx,			// Temporary X position
		ty,			// Temporary Y position
		width,			// Width
		line_width,		// Total line width
		format_width,		// Formatted width
		above,			// Maximum height above baseline
		below,			// Maximum height below baseline
		height,			// Maximum height
		line_height,		// Line height
		letter_spacing,		// Additional letter spacing
		word_spacing,		// Additional word spacing
		temp_width,		// Temporary width value
		temp_height,		// Temporary height value
		decorationx,		// Text decoration X offset
		decorationy;		// Text decoration Y offset
  int		num_chars,		// Number of characters
		num_words;		// Number of words
  hdRenderNode	*r;			// New render node


  // Mark the last node that we care about...
  
  // First loop for any floating elements embedded in the block...
  for (t = block->child; 
  // First loop to figure out the total width and height of the line...
  width          = 0.0f;
  above          = 0.0f;
  below          = 0.0f;
  letter_spacing = t->style->letter_spacing;
  word_spacing   = t->style->word_spacing;
  num_chars      = 0;
  num_words      = 0;

  for (t = line->child; t != NULL; t = t->next)
  {
    temp_height = t->height +
                  t->style->margin[HD_POS_TOP] +
		  t->style->border[HD_POS_TOP].width +
		  t->style->padding[HD_POS_TOP] +
                  t->style->margin[HD_POS_BOTTOM] +
		  t->style->border[HD_POS_BOTTOM].width +
		  t->style->padding[HD_POS_BOTTOM];

    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_BASELINE :
	  if (temp_height > height)
	    height = temp_height;
          break;

      case HD_VERTICALALIGN_SUB :
	  if (temp_height > below)
	    below = temp_height;
          break;
      case HD_VERTICALALIGN_SUPER :
	  if (temp_height > above)
	    above = temp_height;
          break;

      default :
          break;
    }

    width += t->width;

    if (t->whitespace && t != line->child)
    {
      num_words ++;
      width += t->style->font->get_width(" ") * t->style->font_size;
    }

    if (t->element == HD_ELEMENT_NONE && t->style->font != NULL)
      num_chars += t->style->font->get_num_chars(t->data);
  }

  // Then figure out the right line height
  if (lastline)
    line_height = 0.0f;
  else
    line_height = height * t->style->line_height - height;

  if (line_height < below)
    line_height = below;

  line_height += height + above;

  // Then decide if we need more space for other types of alignment...
  for (t = line->child; t != NULL; t = t->next)
  {
    temp_height = t->height +
                  t->style->margin[HD_POS_TOP] +
		  t->style->border[HD_POS_TOP].width +
		  t->style->padding[HD_POS_TOP] +
                  t->style->margin[HD_POS_BOTTOM] +
		  t->style->border[HD_POS_BOTTOM].width +
		  t->style->padding[HD_POS_BOTTOM];

    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_TOP :
	  if (temp_height > line_height)
	    line_height = temp_height;
          break;

      case HD_VERTICALALIGN_TEXT_TOP :
	  if (temp_height > (line_height - above))
	    line_height = temp_height + above;
          break;

      case HD_VERTICALALIGN_MIDDLE :
          ty = line_height - above - height +
	       0.5f * t->style->font->x_height * t->style->font_size;

	  if ((0.5f * temp_height + ty) > line_height)
	    line_height = 0.5f * temp_height + ty;

          if ((0.5f * temp_height) > ty)
	  {
	    line_height += 0.5f * temp_height - ty;
	    below       += 0.5f * temp_height - ty;
	  }
          break;

      case HD_VERTICALALIGN_BOTTOM :
	  if (temp_height > below)
	    below = temp_height;
          break;

      case HD_VERTICALALIGN_TEXT_BOTTOM :
	  if (temp_height > (height * t->style->line_height + above))
	  {
	    line_height = temp_height + below;
	    height      = temp_height - above;
	  }
          break;

      default :
          break;
    }
  }

  // Next see if we need to skip to the next page...
  if ((*y + line_height) > m.bottom0())
  {
    // Move to the next page...
    page ++;
    *y = 0;

    m.clear(&y, page);
  }

  // Adjust the number of characters for the number of words - we don't
  // want the letter spacing after the last character in each word...
  num_chars -= num_words + 1;
  if (num_chars < 0)
    num_chars = 0;

  // Add in the word and letter spacing...
  format_width = width + num_words * word_spacing + num_chars * letter_spacing;

  // Align the line...
  line_width = m.width() -
               line->style->padding[HD_POS_LEFT] -
               line->style->padding[HD_POS_RIGHT] -
               line->style->margin[HD_POS_LEFT] -
               line->style->margin[HD_POS_RIGHT] -
               line->style->border[HD_POS_LEFT].width -
               line->style->border[HD_POS_RIGHT].width;

  switch (t->style->text_align)
  {
    case HD_TEXTALIGN_LEFT :
        tx = m.left() +
             t->style->padding[HD_POS_LEFT] +
             t->style->margin[HD_POS_LEFT] +
             t->style->border[HD_POS_LEFT].width;
	break;

    case HD_TEXTALIGN_CENTER :
        tx = m.left() + 0.5f * (m.width() - format_width);
	break;

    case HD_TEXTALIGN_RIGHT :
        tx = m.right() -
             t->style->padding[HD_POS_RIGHT] -
             t->style->margin[HD_POS_RIGHT] -
             t->style->border[HD_POS_RIGHT].width -
	     format_width;
	break;

    case HD_TEXTALIGN_JUSTIFY :
        tx = m.left() +
             t->style->padding[HD_POS_LEFT] +
             t->style->margin[HD_POS_LEFT] +
             t->style->border[HD_POS_LEFT].width;

        if (!lastline)
	{
          // Update the word and letter spacing...
	  temp_width = line_width - format_width;
	  if (num_chars < 3 && num_words > 0)
	  {
	    word_spacing += temp_width / num_words;
	  }
	  else if (num_words < 1 && num_chars > 0)
	  {
	    letter_spacing += temp_width / num_chars;
	  }
	  else if (num_words > 0 && num_chars > 0)
	  {
	    word_spacing   += 0.5f * temp_width / num_words;
	    letter_spacing += 0.5f * temp_width / num_chars;
	  }
	}
	break;
  }

  // Render any background for the paragraph...
  if (line->style->background_color_set ||
      line->style->background_image ||
      line->style->border[HD_POS_LEFT].style ||
      line->style->border[HD_POS_BOTTOM].style ||
      line->style->border[HD_POS_RIGHT].style ||
      line->style->border[HD_POS_TOP].style)
    add_render(page, HD_RENDERTYPE_BACKGROUND, tx, &y,
               m.width(), line_height, line->style);

  // Loop again to render the stuff...
  for (t = line->child; t != NULL; t = t->next)
  {
    // Figure out the width of the node...
    temp_width = t->width +
                 t->style->margin[HD_POS_LEFT] +
		 t->style->border[HD_POS_LEFT].width +
		 t->style->padding[HD_POS_LEFT] +
                 t->style->margin[HD_POS_RIGHT] +
		 t->style->border[HD_POS_RIGHT].width +
		 t->style->padding[HD_POS_RIGHT];

    if (t->element == HD_ELEMENT_NONE)
      temp_width += (t->style->font->get_num_chars(t->data) - 1) *
                    letter_spacing;
    
    if (t->whitespace && t != line->child)
      tx += t->style->font->get_width(" ") * t->style->font_size + word_spacing;

    // Render the background and border, if any...
    if (t->style->background_color_set ||
        t->style->background_image ||
	t->style->border[HD_POS_LEFT].style ||
	t->style->border[HD_POS_BOTTOM].style ||
	t->style->border[HD_POS_RIGHT].style ||
	t->style->border[HD_POS_TOP].style)
      add_render(page, HD_RENDERTYPE_BACKGROUND, tx, &y,
                 temp_width, line_height, t->style);

    // Figure out the vertical position...
    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_BASELINE :
          ty = *y + above + height;
          break;

      case HD_VERTICALALIGN_SUB :
          ty = *y + line_height - above + t->height;
          break;

      case HD_VERTICALALIGN_SUPER :
	  ty = *y + above + t->height;
          break;

      case HD_VERTICALALIGN_TOP :
          ty = *y + t->height;
          break;

      case HD_VERTICALALIGN_TEXT_TOP :
	  ty = *y + above;
          break;

      case HD_VERTICALALIGN_MIDDLE :
          ty = *y + above + height -
	       0.5f * t->style->font->x_height * t->style->font_size +
	       0.5f * t->height;
          break;

      case HD_VERTICALALIGN_BOTTOM :
          ty = *y + line_height;
          break;

      case HD_VERTICALALIGN_TEXT_BOTTOM :
          ty = *y + above + height + t->height;
          break;
    }

    // Then render it...
    switch (t->element)
    {
      case HD_ELEMENT_NONE :
          // Text element...
          r = add_render(page, HD_RENDERTYPE_TEXT,
	                 tx + t->style->margin[HD_POS_LEFT] +
			     t->style->border[HD_POS_LEFT].width +
			     t->style->padding[HD_POS_LEFT], ty,
			 t->width, t->style->font_size, t->data);

          if (r)
	  {
	    r->data.text.font = t->style->font;
	    r->data.text.font_size = t->style->font_size;
	    r->data.text.char_spacing = letter_spacing;
	    memcpy(r->data.text.rgb, t->style->color, sizeof(r->data.text.rgb));
	  }

          if (t->link)
	  {
	    // Add a hyper link for clicking...
            add_render(page, HD_RENDERTYPE_LINK,
	               tx + t->style->margin[HD_POS_LEFT] +
			   t->style->border[HD_POS_LEFT].width +
			   t->style->padding[HD_POS_LEFT], ty,
		       t->width, t->style->font_size,
		       t->link->get_attr("href"));
	  }

          if (t->style->text_decoration)
	  {
	    if (t->whitespace && t != line->child &&
	        t->prev->style->text_decoration == t->style->text_decoration)
	      decorationx = t->style->font->get_width(" ") *
	                        t->style->font_size + word_spacing;
	    else
	      decorationx = 0.0f;
            
            switch (t->style->text_decoration)
	    {
	      case HD_TEXTDECORATION_NONE :
	          decorationy = 0.0f;
	          break;

	      case HD_TEXTDECORATION_UNDERLINE :
		  decorationy = t->style->font->ul_position *
		                t->style->font_size;
	          break;

	      case HD_TEXTDECORATION_OVERLINE :
		  decorationy = t->style->font_size;
	          break;

	      case HD_TEXTDECORATION_LINE_THROUGH :
		  decorationy = 0.5f * t->style->font_size;
	          break;
	    }

            add_render(page, HD_RENDERTYPE_BOX,
	               tx - decorationx, ty - decorationy,
		       temp_width + decorationx,
		       t->style->font->ul_thickness * t->style->font_size,
		       t->style->color);
          }
	  break;

      case HD_ELEMENT_IMG :
          // Image element...
	  render_image(t, tx, ty, page);
	  break;

      default :
          break;
    }

    // Finally, update the X position...
    tx += temp_width;
  }

  // Update current position and margins...
  *x = m.left();
  *y -= line_height;

  m.clear(&y, page);
}
#endif // 0


//
// 'hdRender::render_comment()' -
//

int					// O  - Non-zero for page break
hdRender::render_comment(hdTree   *t,	// I  - Comment node
                         hdMargin &m,	// I  - Current margins
			 float    &x,	// IO - Current X position
			 float    &y,	// IO - Current Y position
                         int      &page)// IO - Current page
{
  return (0);
}


//
// 'hdRender::render_contents()' -
//

void
hdRender::render_contents(hdTree     *t,// I  - Table-of-contents node
                          hdMargin   &m,// IO - Margins
			  int        &page,
					// IO - Current page
			  const char *label)
					// I  - Table-of-contents label
{
}


//
// 'hdRender::render_doc()' -
//

void
hdRender::render_doc(hdTree   *t,	// I  - Document node
                     hdMargin &m,	// IO - Margins
		     float    &x,	// IO - Image X position
		     float    &y,	// IO - Image Y position
		     int      &page)	// IO - Current page
{
}


//
// 'hdRender::render_image()' - Render an image...
//

void
hdRender::render_image(hdTree   *t,	// I  - Image node
                       hdMargin &m,	// IO - Margins
                       float    &x,	// IO - Image X position
		       float    &y,	// IO - Image Y position
		       int      &page,	// IO - Current page
		       float    ascent,	// I  - Ascent of line
		       float    descent)// I  - Descent of line
{
  float		tx,			// Temporary X position
		ty,			// Temporary Y position
		temp_width,		// Temporary width
		temp_height;		// Temporary height
  hdImage	*img;			// Image
  const char	*imgmapname;		// Image map name
  hdTree	*imgmap,		// Image map
		*imgarea;		// Image map area
  const char	*imgareacoords;		// Image area coordinates
  float		imgareax,		// Image area X
		imgareay,		// Image area Y
		imgareaw,		// Image area width
		imgareah;		// Image area height


  // Figure out the position and size of the image box...
  temp_width  = t->width +
                t->style->margin[HD_POS_LEFT] +
		t->style->border[HD_POS_LEFT].width +
		t->style->padding[HD_POS_LEFT] +
		t->style->margin[HD_POS_RIGHT] +
		t->style->border[HD_POS_RIGHT].width +
		t->style->padding[HD_POS_RIGHT];
  temp_height = t->height +
                t->style->margin[HD_POS_TOP] +
		t->style->border[HD_POS_TOP].width +
		t->style->padding[HD_POS_TOP] +
		t->style->margin[HD_POS_BOTTOM] +
		t->style->border[HD_POS_BOTTOM].width +
		t->style->padding[HD_POS_BOTTOM];
  tx          = x;
  ty          = y + temp_height;

  // Handle floating images...
  if (t->style->float_ != HD_FLOAT_NONE)
  {
    if ((ty + temp_height) > m.bottom0())
    {
      page ++;
      ty = temp_height;
      y  = 0.0f;
    }

    if (t->style->float_ == HD_FLOAT_LEFT)
    {
      tx = m.left();

      m.push(m.left() + temp_width, m.right(), ty, page);
    }
    else
    {
      tx = m.right() - temp_width;

      m.push(m.left(), m.right() - temp_width, ty, page);
    }
  }

  // Render the background...
  if (t->style->background_color_set ||
      t->style->background_image ||
      t->style->border[HD_POS_LEFT].style ||
      t->style->border[HD_POS_BOTTOM].style ||
      t->style->border[HD_POS_RIGHT].style ||
      t->style->border[HD_POS_TOP].style)
    add_render(page, HD_RENDERTYPE_BACKGROUND, tx, ty, temp_width,
               temp_height, t->style);

  // Lookup the image...
  img = hdImage::find(t->get_attr("_HD_SRC"), css->grayscale);
  if (!img)
    return;

  // Render the image...
  add_render(page, HD_RENDERTYPE_IMAGE,
	     tx + t->style->margin[HD_POS_LEFT] +
		 t->style->border[HD_POS_LEFT].width +
		 t->style->padding[HD_POS_LEFT], ty,
	     t->width, t->height, img);

  // Render any links...
  if (t->link)
  {
    // Add a hyper link for clicking...
    add_render(page, HD_RENDERTYPE_LINK,
	       tx + t->style->margin[HD_POS_LEFT] +
		   t->style->border[HD_POS_LEFT].width +
		   t->style->padding[HD_POS_LEFT], ty,
	       t->width, t->height,
	       t->link->get_attr("href"));
  }

  if ((imgmapname = t->get_attr("usemap")) != NULL &&
      (imgmap = find_imgmap(imgmapname)) != NULL)
  {
    // Add links from the image map...
    for (imgarea = imgmap->child; imgarea; imgarea = imgarea->next)
    {
      if (imgarea->element != HD_ELEMENT_AREA)
	continue;

      if ((imgareacoords = imgarea->get_attr("coords")) == NULL)
	continue;

      if (sscanf(imgareacoords, "%f,%f,%f,%f", &imgareax, &imgareay,
	         &imgareaw, &imgareah) != 4)
	continue;

      imgareax *= t->width / img->width();
      imgareay *= t->height / img->height();
      imgareaw *= t->width / img->width();
      imgareah *= t->height / img->height();

      imgareaw -= imgareax;
      imgareah -= imgareay;

      add_render(page, HD_RENDERTYPE_LINK,
	         tx + t->style->margin[HD_POS_LEFT] +
		     t->style->border[HD_POS_LEFT].width +
		     t->style->padding[HD_POS_LEFT] + imgareax,
		 ty + imgareay, imgareaw, imgareah,
		 imgarea->get_attr("href"));
    }
  }
}


//
// 'hdRender::render_index()' -
//

void
hdRender::render_index(hdTree     *t,
                       hdMargin   &m,
		       int        &page,
		       const char *label)
{
}


#if 0
//
// 'hdRender::render_line()' - Render a single line of text.
//

void
hdRender::render_line(hdTree   *line,	// I  - Line tree
                      hdMargin &m,	// I  - Current margins
		      float    &x,	// IO - Current X position
		      float    &y,	// IO - Current Y position
		      int      &page,	// IO - Current page
		      int      lastline)// I  - 1 = last line
{
  hdTree	*t;			// Current node
  float		tx,			// Temporary X position
		ty,			// Temporary Y position
		width,			// Width
		line_width,		// Total line width
		format_width,		// Formatted width
		above,			// Maximum height above baseline
		below,			// Maximum height below baseline
		height,			// Maximum height
		line_height,		// Line height
		letter_spacing,		// Additional letter spacing
		word_spacing,		// Additional word spacing
		temp_width,		// Temporary width value
		temp_height,		// Temporary height value
		decorationx,		// Text decoration X offset
		decorationy;		// Text decoration Y offset
  int		num_chars,		// Number of characters
		num_words;		// Number of words
  hdRenderNode	*r;			// New render node


  // First loop to figure out the total width and height of the line...
  width          = 0.0f;
  above          = 0.0f;
  below          = 0.0f;
  letter_spacing = t->style->letter_spacing;
  word_spacing   = t->style->word_spacing;
  num_chars      = 0;
  num_words      = 0;

  for (t = line->child; t != NULL; t = t->next)
  {
    temp_height = t->height +
                  t->style->margin[HD_POS_TOP] +
		  t->style->border[HD_POS_TOP].width +
		  t->style->padding[HD_POS_TOP] +
                  t->style->margin[HD_POS_BOTTOM] +
		  t->style->border[HD_POS_BOTTOM].width +
		  t->style->padding[HD_POS_BOTTOM];

    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_BASELINE :
	  if (temp_height > height)
	    height = temp_height;
          break;

      case HD_VERTICALALIGN_SUB :
	  if (temp_height > below)
	    below = temp_height;
          break;
      case HD_VERTICALALIGN_SUPER :
	  if (temp_height > above)
	    above = temp_height;
          break;

      default :
          break;
    }

    width += t->width;

    if (t->whitespace && t != line->child)
    {
      num_words ++;
      width += t->style->font->get_width(" ") * t->style->font_size;
    }

    if (t->element == HD_ELEMENT_NONE && t->style->font != NULL)
      num_chars += t->style->font->get_num_chars(t->data);
  }

  // Then figure out the right line height
  if (lastline)
    line_height = 0.0f;
  else
    line_height = height * t->style->line_height - height;

  if (line_height < below)
    line_height = below;

  line_height += height + above;

  // Then decide if we need more space for other types of alignment...
  for (t = line->child; t != NULL; t = t->next)
  {
    temp_height = t->height +
                  t->style->margin[HD_POS_TOP] +
		  t->style->border[HD_POS_TOP].width +
		  t->style->padding[HD_POS_TOP] +
                  t->style->margin[HD_POS_BOTTOM] +
		  t->style->border[HD_POS_BOTTOM].width +
		  t->style->padding[HD_POS_BOTTOM];

    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_TOP :
	  if (temp_height > line_height)
	    line_height = temp_height;
          break;

      case HD_VERTICALALIGN_TEXT_TOP :
	  if (temp_height > (line_height - above))
	    line_height = temp_height + above;
          break;

      case HD_VERTICALALIGN_MIDDLE :
          ty = line_height - above - height +
	       0.5f * t->style->font->x_height * t->style->font_size;

	  if ((0.5f * temp_height + ty) > line_height)
	    line_height = 0.5f * temp_height + ty;

          if ((0.5f * temp_height) > ty)
	  {
	    line_height += 0.5f * temp_height - ty;
	    below       += 0.5f * temp_height - ty;
	  }
          break;

      case HD_VERTICALALIGN_BOTTOM :
	  if (temp_height > below)
	    below = temp_height;
          break;

      case HD_VERTICALALIGN_TEXT_BOTTOM :
	  if (temp_height > (height * t->style->line_height + above))
	  {
	    line_height = temp_height + below;
	    height      = temp_height - above;
	  }
          break;

      default :
          break;
    }
  }

  // Next see if we need to skip to the next page...
  if ((*y + line_height) > m.bottom0())
  {
    // Move to the next page...
    page ++;
    *y = 0;

    m.clear(&y, page);
  }

  // Adjust the number of characters for the number of words - we don't
  // want the letter spacing after the last character in each word...
  num_chars -= num_words + 1;
  if (num_chars < 0)
    num_chars = 0;

  // Add in the word and letter spacing...
  format_width = width + num_words * word_spacing + num_chars * letter_spacing;

  // Align the line...
  line_width = m.width() -
               line->style->padding[HD_POS_LEFT] -
               line->style->padding[HD_POS_RIGHT] -
               line->style->margin[HD_POS_LEFT] -
               line->style->margin[HD_POS_RIGHT] -
               line->style->border[HD_POS_LEFT].width -
               line->style->border[HD_POS_RIGHT].width;

  switch (t->style->text_align)
  {
    case HD_TEXTALIGN_LEFT :
        tx = m.left() +
             t->style->padding[HD_POS_LEFT] +
             t->style->margin[HD_POS_LEFT] +
             t->style->border[HD_POS_LEFT].width;
	break;

    case HD_TEXTALIGN_CENTER :
        tx = m.left() + 0.5f * (m.width() - format_width);
	break;

    case HD_TEXTALIGN_RIGHT :
        tx = m.right() -
             t->style->padding[HD_POS_RIGHT] -
             t->style->margin[HD_POS_RIGHT] -
             t->style->border[HD_POS_RIGHT].width -
	     format_width;
	break;

    case HD_TEXTALIGN_JUSTIFY :
        tx = m.left() +
             t->style->padding[HD_POS_LEFT] +
             t->style->margin[HD_POS_LEFT] +
             t->style->border[HD_POS_LEFT].width;

        if (!lastline)
	{
          // Update the word and letter spacing...
	  temp_width = line_width - format_width;
	  if (num_chars < 3 && num_words > 0)
	  {
	    word_spacing += temp_width / num_words;
	  }
	  else if (num_words < 1 && num_chars > 0)
	  {
	    letter_spacing += temp_width / num_chars;
	  }
	  else if (num_words > 0 && num_chars > 0)
	  {
	    word_spacing   += 0.5f * temp_width / num_words;
	    letter_spacing += 0.5f * temp_width / num_chars;
	  }
	}
	break;
  }

  // Render any background for the paragraph...
  if (line->style->background_color_set ||
      line->style->background_image ||
      line->style->border[HD_POS_LEFT].style ||
      line->style->border[HD_POS_BOTTOM].style ||
      line->style->border[HD_POS_RIGHT].style ||
      line->style->border[HD_POS_TOP].style)
    add_render(page, HD_RENDERTYPE_BACKGROUND, tx, &y,
               m.width(), line_height, line->style);

  // Loop again to render the stuff...
  for (t = line->child; t != NULL; t = t->next)
  {
    // Figure out the width of the node...
    temp_width = t->width +
                 t->style->margin[HD_POS_LEFT] +
		 t->style->border[HD_POS_LEFT].width +
		 t->style->padding[HD_POS_LEFT] +
                 t->style->margin[HD_POS_RIGHT] +
		 t->style->border[HD_POS_RIGHT].width +
		 t->style->padding[HD_POS_RIGHT];

    if (t->element == HD_ELEMENT_NONE)
      temp_width += (t->style->font->get_num_chars(t->data) - 1) *
                    letter_spacing;
    
    if (t->whitespace && t != line->child)
      tx += t->style->font->get_width(" ") * t->style->font_size + word_spacing;

    // Figure out the vertical position...
    switch (t->style->vertical_align)
    {
      case HD_VERTICALALIGN_BASELINE :
          ty = *y + above + height;
          break;

      case HD_VERTICALALIGN_SUB :
          ty = *y + line_height - above + t->height;
          break;

      case HD_VERTICALALIGN_SUPER :
	  ty = *y + above + t->height;
          break;

      case HD_VERTICALALIGN_TOP :
          ty = *y + t->height;
          break;

      case HD_VERTICALALIGN_TEXT_TOP :
	  ty = *y + above;
          break;

      case HD_VERTICALALIGN_MIDDLE :
          ty = *y + above + height -
	       0.5f * t->style->font->x_height * t->style->font_size +
	       0.5f * t->height;
          break;

      case HD_VERTICALALIGN_BOTTOM :
          ty = *y + line_height;
          break;

      case HD_VERTICALALIGN_TEXT_BOTTOM :
          ty = *y + above + height + t->height;
          break;
    }

    // Then render it...
    switch (t->element)
    {
      case HD_ELEMENT_NONE :
	  // Render the background and border, if any...
	  if (t->style->background_color_set ||
              t->style->background_image ||
	      t->style->border[HD_POS_LEFT].style ||
	      t->style->border[HD_POS_BOTTOM].style ||
	      t->style->border[HD_POS_RIGHT].style ||
	      t->style->border[HD_POS_TOP].style)
	    add_render(page, HD_RENDERTYPE_BACKGROUND, tx, &y,
                       temp_width, line_height, t->style);

          // Text element...
          r = add_render(page, HD_RENDERTYPE_TEXT,
	                 tx + t->style->margin[HD_POS_LEFT] +
			     t->style->border[HD_POS_LEFT].width +
			     t->style->padding[HD_POS_LEFT], ty,
			 t->width, t->style->font_size, t->data);

          if (r)
	  {
	    r->data.text.font = t->style->font;
	    r->data.text.font_size = t->style->font_size;
	    r->data.text.char_spacing = letter_spacing;
	    memcpy(r->data.text.rgb, t->style->color, sizeof(r->data.text.rgb));
	  }

          if (t->link)
	  {
	    // Add a hyper link for clicking...
            add_render(page, HD_RENDERTYPE_LINK,
	               tx + t->style->margin[HD_POS_LEFT] +
			   t->style->border[HD_POS_LEFT].width +
			   t->style->padding[HD_POS_LEFT], ty,
		       t->width, t->style->font_size,
		       t->link->get_attr("href"));
	  }

          if (t->style->text_decoration)
	  {
	    if (t->whitespace && t != line->child &&
	        t->prev->style->text_decoration == t->style->text_decoration)
	      decorationx = t->style->font->get_width(" ") *
	                        t->style->font_size + word_spacing;
	    else
	      decorationx = 0.0f;
            
            switch (t->style->text_decoration)
	    {
	      case HD_TEXTDECORATION_NONE :
	          decorationy = 0.0f;
	          break;

	      case HD_TEXTDECORATION_UNDERLINE :
		  decorationy = t->style->font->ul_position *
		                t->style->font_size;
	          break;

	      case HD_TEXTDECORATION_OVERLINE :
		  decorationy = t->style->font_size;
	          break;

	      case HD_TEXTDECORATION_LINE_THROUGH :
		  decorationy = 0.5f * t->style->font_size;
	          break;
	    }

            add_render(page, HD_RENDERTYPE_BOX,
	               tx - decorationx, ty - decorationy,
		       temp_width + decorationx,
		       t->style->font->ul_thickness * t->style->font_size,
		       t->style->color);
          }
	  break;

      case HD_ELEMENT_IMG :
          // Image element...
	  render_image(t, m, &tx, &ty, page);
	  break;

      default :
          break;
    }

    // Finally, update the X position...
    tx += temp_width;
  }

  // Update current position and margins...
  *x = m.left();
  *y -= line_height;

  m.clear(&y, page);
}
#endif // 0


//
// 'hdRender::render_list()' -
//

void
hdRender::render_list(hdTree   *t,
                      hdMargin &m,
		      float    &x,
		      float    &y,
                      int      &page,
		      float    ascent,
		      float    descent)
{
}


//
// 'hdRender::render_text()' -
//

void
hdRender::render_text(hdTree   *t,
                      hdMargin &m,
		      float    &x,
		      float    &y,
                      int      &page,
		      float    ascent,
		      float    descent)
{
}


//
// 'hdRender::prepare_page()' -
//

void
hdRender::prepare_page(int page)
{
}


//
// 'hdRender::prepare_heading()' -
//

void
hdRender::prepare_heading(int  page,
                	  int  print_page,
			  char **format,
			  int  y,
			  char *page_text,
			  int  page_len)
{
}


#if 0

//
 * 'pspdf_export()' - Export PostScript/PDF file(s)...


int
pspdf_export(hdTree *document,	// I - Document to export
             hdTree *toc)	// I - Table of contents for document
{
  const char	*title_file;	// Location of title image/file
  uchar		*author,	// Author of document
		*creator,	// HTML file creator (Netscape, etc)
		*copyright,	// File copyright
		*docnumber,	// Document number
		*keywords;	// Search keywords
  hdTree	*t;		// Title page document tree
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
  hdRenderNode	*r;		// Rendering structure...
  float		rgb[3];		// Text color
  int		needspace;	// Need whitespace


 //
  * Figure out the printable area of the output page...
 

  if (css->orientation)
  {
    css->print_width  = css->page_length - css->page_left - css->page_right;
    css->print_length = css->page_width - css-> - css->bottom;
  }
  else
  {
    css->print_width  = css->page_width - css->page_left - css->page_right;
    css->print_length = css->page_length - css-> - css->bottom;
  }

  toc_width     = css->page_width;
  toc_length    = css->page_length;
  toc_left      = css->page_left;
  toc_right     = css->page_right;
  toc_bottom    = css->bottom;
  toc_top       = css->;
  toc_landscape = css->orientation;
  toc_duplex    = css->sides;

 //
  * Get the document title, author, etc...
 

  doc_title  = get_title(document);
  author     = htmlGetMeta(document, "author");
  creator    = htmlGetMeta(document, "generator");
  copyright  = htmlGetMeta(document, "copyright");
  docnumber  = htmlGetMeta(document, "docnumber");
  keywords   = htmlGetMeta(document, "keywords");
  logo_image = image_load(LogoImage, !OutputColor);

  if (logo_image != NULL)
  {
    logo_width  = logo_image->width * css->print_width / _htmlBrowserWidth;
    logo_height = logo_width * logo_image->height / logo_image->width;
  }
  else
    logo_width = logo_height = 0.0f;

  find_background(document);
  get_color(LinkColor, link_color, 0);

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
        stricmp(hdFile::extension(TitleImage), "bmp") != 0 &&
	stricmp(hdFile::extension(TitleImage), "gif") != 0 &&
	stricmp(hdFile::extension(TitleImage), "jpg") != 0 &&
	stricmp(hdFile::extension(TitleImage), "png") != 0)
#else
    if (TitleImage[0] &&
        strcmp(hdFile::extension(TitleImage), "bmp") != 0 &&
	strcmp(hdFile::extension(TitleImage), "gif") != 0 &&
	strcmp(hdFile::extension(TitleImage), "jpg") != 0 &&
	strcmp(hdFile::extension(TitleImage), "png") != 0)
#endif // WIN32
    {
      // Find the title file...
      if ((title_file = hdFile::find(Path, TitleImage)) == NULL)
        return (1);

      // Write a title page from HTML source...
      if ((fp = hdFile::open(title_file, HD_FILE_READ)) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
	               "Unable to open title file \"%s\" - %s!",
                       TitleImage, strerror(errno));
	return (1);
      }

      t = htmlReadFile(NULL, fp, hdFile::directory(TitleImage));
      fclose(fp);

      page            = 0;
      title_page      = 1;
      current_heading = NULL;
      x               = 0.0f;
      bottom          = 0.0f;
      top             = css->print_length;
      y               = top;
      needspace       = 0;
      left            = 0.0f;
      right           = css->print_width;

      render_doc(t, &left, &right, &bottom, &top, &x, &y, page, NULL,
                &needspace);

      if (css->sides && (num_pages & 1))
	check_pages(num_pages);

      htmlDeleteTree(t);
    }
    else
    {
     //
      * Create a standard title page...
     

      if ((timage = image_load(TitleImage, !OutputColor)) != NULL)
      {
	timage_width  = timage->width * css->print_width / _htmlBrowserWidth;
	timage_height = timage_width * timage->height / timage->width;
      }
      else
        timage_width = timage_height = 0.0f;

      check_pages(0);
      if (css->sides)
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

      y = 0.5f * (css->print_length + height);

      if (timage != NULL)
      {
	add_render(0, RENDER_IMAGE, 0.5f * (css->print_width - timage_width),
                   y - timage_height, timage_width, timage_height, timage);
	y -= timage_height + _htmlSpacings[SIZE_P];
      }

      get_color(_htmlTextColor, rgb);

      if (doc_title != NULL)
      {
	width = get_width(doc_title, _htmlHeadingFont, STYLE_BOLD, SIZE_H1);
	r     = add_render(0, RENDER_TEXT, (css->print_width - width) * 0.5f,
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
	  r     = add_render(0, RENDER_TEXT, (css->print_width - width) * 0.5f,
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
	r     = add_render(0, RENDER_TEXT, (css->print_width - width) * 0.5f,
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
	r     = add_render(0, RENDER_TEXT, (css->print_width - width) * 0.5f,
                	   y - _htmlSpacings[SIZE_P], width, _htmlSizes[SIZE_P],
			   copyright);

	r->data.text.typeface = _htmlBodyFont;
	r->data.text.style    = STYLE_NORMAL;
	r->data.text.size     = _htmlSizes[SIZE_P];
	memcpy(r->data.text.rgb, rgb, sizeof(rgb));
      }
    }

    for (page = 0; page < num_pages; page ++)
      strcpy(pages[page].hdRenderPageext, (page & 1) ? "eltit" : "title");
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
  right           = css->print_width;

  // Adjust top margin as needed...
  for (pos = 0; pos < 3; pos ++)
    if (doc_header[pos])
      break;

  if (pos == 3)
    top = css->print_length;
  else if (logo_height > HeadFootSize)
    top = css->print_length - logo_height - HeadFootSize;
  else
    top = css->print_length - 2 * HeadFootSize;

  // Adjust bottom margin as needed...
  for (pos = 0; pos < 3; pos ++)
    if (doc_footer[pos])
      break;

  if (pos == 3)
    bottom = 0.0f;
  else if (logo_height > HeadFootSize)
    bottom = logo_height + HeadFootSize;
  else
    bottom = 2 * HeadFootSize;

  y = top;

  render_doc(document, &left, &right, &bottom, &top, &x, &y, page, NULL,
            &needspace);

  if (css->sides && (num_pages & 1))
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
    css->page_width  = toc_width;
    css->page_length = toc_length;
    css->page_left   = toc_left;
    css->page_right  = toc_right;
    css->bottom = toc_bottom;
    css->    = toc_top;
    css->orientation  = toc_landscape;
    css->sides = toc_duplex;

    if (css->orientation)
    {
      css->print_width  = css->page_length - css->page_left - css->page_right;
      css->print_length = css->page_width - css-> - css->bottom;
    }
    else
    {
      css->print_width  = css->page_width - css->page_left - css->page_right;
      css->print_length = css->page_length - css-> - css->bottom;
    }

    // Adjust top margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (toc_header[pos])
	break;

    if (pos == 3)
      top = css->print_length;
    else if (logo_height > HeadFootSize)
      top = css->print_length - logo_height - HeadFootSize;
    else
      top = css->print_length - 2 * HeadFootSize;

    // Adjust bottom margin as needed...
    for (pos = 0; pos < 3; pos ++)
      if (toc_footer[pos])
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

    render_contents(toc, 0, css->print_width, bottom, top, &y, page, &heading, 0);
    if (css->sides && (num_pages & 1))
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
  char	hdRenderPageext[64];			// Page number text


  DEBUG_printf(("pspdf_prepare_page(%d)\n", page));

 //
  * Make a page number; use roman numerals for the table of contents
  * and arabic numbers for all others...
 

  if (chapter == 0 && OutputType == OUTPUT_BOOK)
  {
    print_page = page - chapter_starts[0] + 1;
    strncpy(hdRenderPageext, format_number(print_page, 'i'), sizeof(hdRenderPageext) - 1);
    hdRenderPageext[sizeof(hdRenderPageext) - 1] = '\0';
  }
  else if (chapter < 0)
  {
    print_page = 0;
    strcpy(hdRenderPageext, (page & 1) ? "eltit" : "title");
  }
  else
  {
    print_page = page - chapter_starts[1] + 1;
    strncpy(hdRenderPageext, format_number(print_page, '1'), sizeof(hdRenderPageext) - 1);
    hdRenderPageext[sizeof(hdRenderPageext) - 1] = '\0';
  }

 //
  * Add page headings...
 

  if (pages[page].landscape)
  {
    css->print_width  = pages[page].length - pages[page].right - pages[page].left;
    css->print_length = pages[page].width - pages[page].top - pages[page].bottom;
  }
  else
  {
    css->print_width  = pages[page].width - pages[page].right - pages[page].left;
    css->print_length = pages[page].length - pages[page].top - pages[page].bottom;
  }

  if (chapter == 0)
  {
   //
    * Add table-of-contents header & footer...
   

    pspdf_prepare_heading(page, print_page, pages[page].header,
                          css->print_length, hdRenderPageext, sizeof(hdRenderPageext));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          hdRenderPageext, sizeof(hdRenderPageext));
  }
  else if (chapter > 0 && !title_page)
  {
   //
    * Add chapter header & footer...
   

    if (page > chapter_starts[chapter] || OutputType != OUTPUT_BOOK)
      pspdf_prepare_heading(page, print_page, pages[page].header,
                            css->print_length, hdRenderPageext, sizeof(hdRenderPageext));
    pspdf_prepare_heading(page, print_page, pages[page].footer, 0,
                          hdRenderPageext, sizeof(hdRenderPageext));
  }

 //
  * Copy the page number for the TOC...
 

  strncpy(pages[page].hdRenderPageext, hdRenderPageext, sizeof(pages[page].hdRenderPageext) - 1);
  pages[page].hdRenderPageext[sizeof(pages[page].hdRenderPageext) - 1] = '\0';
}


//
 * 'pspdf_prepare_heading()' - Add headers/footers to page before writing...


static void
pspdf_prepare_heading(int   page,		// I - Page number
                      int   print_page,         // I - Printed page number
		      uchar **format,		// I - Page headings
		      int   y,			// I - Baseline of heading
		      char  *hdRenderPageext,		// O - Page number text
		      int   page_len)		// I - Size of page text
{
  int		pos,		// Position in heading
		dir;		// Direction of page
  char		*number;	// Page number
  char		buffer[1024],	// String buffer
		*bufptr,	// Pointer into buffer
		*formatptr;	// Pointer into format string
  int		formatlen;	// Length of format command string
  hdRenderNode	*temp;		// Render structure for titles, etc.


  DEBUG_printf(("pspdf_prepare_heading(%d, %d, %p, %d, %p, %d)\n",
                page, print_page, format, y, hdRenderPageext, page_len));

 //
  * Add page headings...
 

  if (css->sides && (page & 1))
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

    if (strncasecmp(*format, "$LOGOIMAGE", 10) == 0 && logo_image)
    {
      // Insert the logo image...
      if (y < (css->print_length / 2))
	temp = add_render(page, RENDER_IMAGE, 0, y, logo_width,
	                  logo_height, logo_image);
      else // Offset from top
	temp = add_render(page, RENDER_IMAGE, 0,
	                  y + HeadFootSize - logo_height,
	                  logo_width, logo_height, logo_image);
    }
    else
    {
      // Otherwise format the text...
      buffer[sizeof(buffer) - 1] = '\0';

      for (bufptr = buffer, formatptr = *format; *formatptr;)
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
              strncpy(bufptr, doc_title,
	              sizeof(buffer) - 1 - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "CHAPTER", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].chapter)
	    {
              strncpy(bufptr, (pages[page].chapter),
	              sizeof(buffer) - 1 - (bufptr - buffer));
	      bufptr += strlen(bufptr);
	    }
	  }
	  else if (formatlen == 7 && strncasecmp(formatptr, "HEADING", 7) == 0)
	  {
            formatptr += 7;
	    if (pages[page].heading)
	    {
              strncpy(bufptr, (pages[page].heading),
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

      temp = add_render(page, RENDER_TEXT, 0, y,
                	get_width(buffer, HeadFootType,
			          HeadFootStyle, SIZE_P),
	        	HeadFootSize, buffer);

      if (strstr(*format, "$PAGE") ||
          strstr(*format, "$CHAPTERPAGE"))
      {
        strncpy(hdRenderPageext, buffer, page_len - 1);
	hdRenderPageext[page_len - 1] = '\0';
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
          temp->x = (css->print_width - temp->width) * 0.5;
          break;
      case 2 : // Right justified
          temp->x = css->print_width - temp->width;
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
render_contents(hdTree *t,		// I - Tree to parse
                float  left,		// I - Left margin
                float  right,		// I - Printable width
                float  bottom,		// I - Bottom margin
                float  top,		// I - Printable top
                float  &y,		// IO - Y position
                int    &page,		// IO - Page #
	        int    heading,		// I - Heading #
	        hdTree *chap)		// I - Chapter heading
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
  hdTree	*flat,
		*temp,
		*next;
  hdRenderNode	*r;
#define dot_width  (_htmlSizes[SIZE_P] * _htmlWidths[t->typeface][t->style]['.'])


  DEBUG_printf(("render_contents(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, left, right, bottom, top, &y, page, heading, chap));

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
    numberwidth = get_width(pages[hpage].hdRenderPageext,
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
      page ++;
      if (Verbosity)
	progress_show("Formatting page %d", page);

      width = get_width(TocTitle, TYPE_HELVETICA, STYLE_BOLD, SIZE_H1);
      *y = top - _htmlSpacings[SIZE_H1];
      x  = left + 0.5f * (right - left - width);
      r = add_render(page, RENDER_TEXT, x, &y, 0, 0, TocTitle);
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
      link = htmlGetVariable(temp->link, "HREF");

     //
      * Add a page link...
     

      if (file_method(link) == NULL &&
	  file_target(link) != NULL)
	link = file_target(link) - 1; // Include # sign

      add_render(page, RENDER_LINK, x, &y, temp->width,
	         temp->height, link);

      if (PSLevel == 0 && Links)
      {
        memcpy(rgb, link_color, sizeof(rgb));

	temp->red   = (int)(link_color[0] * 255.0);
	temp->green = (int)(link_color[1] * 255.0);
	temp->blue  = (int)(link_color[2] * 255.0);

        if (LinkStyle)
	  add_render(page, RENDER_BOX, x, *y - 1, temp->width, 0,
	             link_color);
      }
    }

    switch (temp->element)
    {
      case HD_ELEMENT_A :
          if ((link = htmlGetVariable(temp, "NAME")) != NULL)
          {
           //
            * Add a target link...
           

            add_link(link, page, (int)(*y + 6 * height));
          }
          break;

      case HD_ELEMENT_NONE :
          if (temp->data == NULL)
            break;

	  if (temp->underline)
	    add_render(page, RENDER_BOX, x, *y - 1, temp->width, 0, rgb);

	  if (temp->strikethrough)
	    add_render(page, RENDER_BOX, x, *y + temp->height * 0.25f,
		       temp->width, 0, rgb);

          r = add_render(page, RENDER_TEXT, x, &y, 0, 0, temp->data);
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

      case HD_ELEMENT_IMG :
	  update_image_size(temp);
	  add_render(page, RENDER_IMAGE, x, &y, temp->width, temp->height,
		     image_find(htmlGetVariable(temp, "REALSRC")));
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

    strncpy(nptr, pages[hpage].hdRenderPageext,
            sizeof(number) - (nptr - number) - 1);
    number[sizeof(number) - 1] = '\0';

    r = add_render(page, RENDER_TEXT, right - width + x, &y, 0, 0, number);
    r->data.text.typeface = t->typeface;
    r->data.text.style    = t->style;
    r->data.text.size     = _htmlSizes[t->size];
    memcpy(r->data.text.rgb, rgb, sizeof(rgb));
  }
}


//
 * 'render_contents()' - Parse the table of contents and produce a
 *                      rendering list...


static void
render_contents(hdTree *t,		// I - Tree to parse
               float  left,		// I - Left margin
               float  right,		// I - Printable width
               float  bottom,		// I - Bottom margin
               float  top,		// I - Printable top
               float  &y,		// IO - Y position
               int    &page,		// IO - Page #
               int    *heading,		// IO - Heading #
	       hdTree *chap)		// I - Chapter heading
{
  DEBUG_printf(("render_contents(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, y=%.1f, page=%d, heading=%d, chap=%p)\n",
                t, left, right, bottom, top, &y, page, *heading, chap));

  while (t != NULL)
  {
    switch (t->element)
    {
      case HD_ELEMENT_B :	// Top-level TOC
          if (t->prev != NULL)	// Advance one line prior to top-levels...
            *y -= _htmlSpacings[SIZE_P];

          if (*y < (bottom + _htmlSpacings[SIZE_P] * 3))
	    *y = 0; // Force page break

          chap = t;

      case HD_ELEMENT_LI :	// Lower-level TOC
          DEBUG_printf(("render_contents: heading=%d, page = %d\n", *heading,
                        heading_pages[*heading]));

         //
          * Put the text...
         

          render_contents(t, left, right, bottom, top, y, page, *heading, chap);

         //
	  * Update current headings for header/footer strings in TOC.
	 

	  check_pages(page);

	  if (t->element == HD_ELEMENT_B &&
	      pages[page].chapter == pages[&page - 1].chapter)
	    pages[page].chapter = htmlGetText(t->child);

	  if (pages[page].heading == pages[&page - 1].heading)
	    pages[page].heading = htmlGetText(t->child);

         //
          * Next heading...
         

          (*heading) ++;
          break;

      default :
          render_contents(t->child, left, right, bottom, top, y, page, heading,
	                 chap);
          break;
    }

    t = t->next;
  }
}


//
 * 'render_doc()' - Parse a document tree and produce rendering list output.


static void
render_doc(hdTree *t,		// I - Tree to parse
          float  *left,		// I - Left margin
          float  *right,	// I - Printable width
          float  *bottom,	// I - Bottom margin
          float  *top,		// I - Printable top
          float  &x,		// IO - X position
          float  &y,		// IO - Y position
          int    &page,		// IO - Page #
	  hdTree *cpara,	// I - Current paragraph
	  int    *needspace)	// I - Need whitespace before this element
{
  int		i;		// Looping var
  hdTree	*para,		// Phoney paragraph tree entry
		*temp;		// Paragraph entry
  var_t		*var;		// Variable entry
  uchar		*name;		// ID name
  uchar		*style;		// STYLE attribute
  float		width,		// Width of horizontal rule
		height,		// Height of rule
		rgb[3];		// RGB color of rule


  DEBUG_printf(("render_doc(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, cpara=%p, needspace=%d\n",
                t, *left, *right, *bottom, *top, &x, &y, page, cpara,
	        *needspace));

  if (cpara == NULL)
    para = htmlNewTree(NULL, HD_ELEMENT_P, NULL);
  else
    para = cpara;

  while (t != NULL)
  {
    if (((t->element == HD_ELEMENT_H1 && OutputType == OUTPUT_BOOK) ||
         (t->element == HD_ELEMENT_FILE && OutputType == OUTPUT_WEBPAGES)) &&
	!title_page)
    {
      // New page on H1 in book mode or file in webpage mode...
      if (para->child != NULL)
      {
        render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
        htmlDeleteTree(para->child);
        para->child = para->last_child = NULL;
      }

      if ((chapter > 0 && OutputType == OUTPUT_BOOK) ||
          ((&page > 1 || *y < *top) && OutputType == OUTPUT_WEBPAGES))
      {
        if (*y < *top)
          page ++;

        if (css->sides && (&page & 1))
          page ++;

        if (Verbosity)
          progress_show("Formatting page %d", page);

        chapter_ends[chapter] = &page - 1;
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
        chapter_starts[chapter] = &page;

      if (chapter > TocDocCount)
	TocDocCount = chapter;

      *y         = *top;
      *x         = *left;
      *needspace = 0;
    }

    if ((name = htmlGetVariable(t, "ID")) != NULL)
    {
     //
      * Add a link target using the ID=name variable...
     

      add_link(name, page, (int)(*y + 3 * t->height));
    }
    else if (t->element == HD_ELEMENT_FILE)
    {
     //
      * Add a file link...
     

      uchar	name[256],	// New filename
		*sep;		// "?" separator in links


      // Strip any trailing HTTP GET data stuff...
      strncpy(name, htmlGetVariable(t, "FILENAME"),
              sizeof(name) - 1);
      name[sizeof(name) - 1] = '\0';

      if ((sep = strchr(name, '?')) != NULL)
        *sep = '\0';

      // Add the link
      add_link(name, page + (OutputType == OUTPUT_BOOK), (int)top);
    }

    if (chapter == 0 && !title_page)
    {
      // Need to handle page comments before the first heading...
      if (t->element == HD_ELEMENT_COMMENT)
        render_comment(t, left, right, bottom, top, x, y, page, para,
	              *needspace);

      if (t->child != NULL)
        render_doc(t->child, left, right, bottom, top, x, y, page, para,
	          needspace);

      t = t->next;
      continue;
    }

    // Check for some basic stylesheet stuff...
    if ((style = htmlGetStyle(t, "page-break-before:")) != NULL &&
	strcasecmp(style, "avoid") != 0)
    {
      // Advance to the next page...
      page ++;
      *x         = *left;
      *y         = *top;
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (css->sides && (page & 1) &&
          strcasecmp(style, "right") == 0)
	page ++;
      else if (css->sides && !(page & 1) &&
               strcasecmp(style, "left") == 0)
	page ++;

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", page);
    }

    // Process the markup...
    switch (t->element)
    {
      case HD_ELEMENT_IMG :
          update_image_size(t);
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_BR :
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
          if (para->child == NULL && t->element == HD_ELEMENT_NONE &&
	      t->data != NULL && strcmp(t->data, " ") == 0)
	    break;

          if ((temp = htmlAddTree(para, t->element, t->data)) != NULL)
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

      case HD_ELEMENT_TABLE :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          render_table(t, *left, *right, *bottom, *top, x, y, page, *needspace);
	  *needspace = 0;
          break;

      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          render_heading(t, *left, *right, *bottom, *top, x, y, page, *needspace);
	  *needspace = 1;
          break;

      case HD_ELEMENT_BLOCKQUOTE :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          *left  += 36;
	  *right -= 36;

          render_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *left  -= 36;
	  *right += 36;

          *x         = *left;
          *needspace = 1;
          break;

      case HD_ELEMENT_CENTER :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

            *needspace = 1;
          }

          render_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case HD_ELEMENT_P :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          render_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case HD_ELEMENT_DIV :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          render_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }
          break;

      case HD_ELEMENT_PRE :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 1;
          }

          render_pre(t, *left, *right, *bottom, *top, x, y, page, *needspace);

          *x         = *left;
          *needspace = 1;
          break;

      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_OL :
          init_list(t);
      case HD_ELEMENT_DL :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          if (t->indent == 1)
	    *needspace = 1;

          *x    = *left + 36.0f;
	  *left += 36.0f;

          render_doc(t->child, left, right, bottom, top, x, y, page, para,
	            needspace);

          *left -= 36.0f;

          if (t->indent == 1)
	    *needspace = 1;
          break;

      case HD_ELEMENT_LI :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          render_list(t, left, right, bottom, top, x, y, page, *needspace);

          *x         = *left;
          *needspace = t->next && t->next->element != HD_ELEMENT_LI;
          break;

      case HD_ELEMENT_DT :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          *x    = *left - 36.0f;
	  *left -= 36.0f;

          render_doc(t->child, left, right, bottom, top, x, y, page,
	            NULL, needspace);

	  *left      += 36.0f;
          *x         = *left;
          *needspace = 0;
          break;

      case HD_ELEMENT_DD :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;

	    *needspace = 0;
          }

          render_doc(t->child, left, right, bottom, top, x, y, page, NULL,
	            needspace);

          *x         = *left;
          *needspace = 0;
          break;

      case HD_ELEMENT_HR :
          if (para->child != NULL)
          {
            render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
            htmlDeleteTree(para->child);
            para->child = para->last_child = NULL;
          }

          if (htmlGetVariable(t, "BREAK") == NULL)
	  {
	   //
	    * Generate a horizontal rule...
	   

            if ((name = htmlGetVariable(t, "WIDTH")) == NULL)
	      width = *right - *left;
	    else
	    {
	      if (strchr(name, '%') != NULL)
	        width = atoi(name) * (*right - *left) / 100;
	      else
                width = atoi(name) * css->print_width / _htmlBrowserWidth;
            }

            if ((name = htmlGetVariable(t, "SIZE")) == NULL)
	      height = 2;
	    else
	      height = atoi(name) * css->print_width / _htmlBrowserWidth;

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
	     // Won't fit on this page...
	     

              page ++;
	      if (Verbosity)
	        progress_show("Formatting page %d", page);
              *y = *top;
            }

            (*y)   -= height + _htmlSpacings[SIZE_P];
            rgb[0] = t->red / 255.0f;
            rgb[1] = t->green / 255.0f;
            rgb[2] = t->blue / 255.0f;

            add_render(page, RENDER_BOX, &x, *y + _htmlSpacings[SIZE_P] * 0.5,
	               width, height, rgb);
	  }
	  else
	  {
	   //
	    * <HR BREAK> generates a page break...
	   

            page ++;
	    if (Verbosity)
	      progress_show("Formatting page %d", page);
            *y = *top;
	  }

          *x         = *left;
          *needspace = 0;
          break;

      case HD_ELEMENT_COMMENT :
          // Check comments for commands...
          render_comment(t, left, right, bottom, top, x, y, page, para,
	                *needspace);
          break;

      case HD_ELEMENT_HEAD : // Ignore document HEAD section
      case HD_ELEMENT_TITLE : // Ignore title and meta stuff
      case HD_ELEMENT_META :
      case HD_ELEMENT_SCRIPT : // Ignore script stuff
      case HD_ELEMENT_INPUT : // Ignore form stuff
      case HD_ELEMENT_SELECT :
      case HD_ELEMENT_OPTION :
      case HD_ELEMENT_TEXTAREA :
          break;

      case HD_ELEMENT_A :
          if (htmlGetVariable(t, "NAME") != NULL)
	  {
	   //
	    * Add this named destination to the paragraph tree...
	   

            if (para->child == NULL)
            {
              para->halignment = t->halignment;
              para->indent     = t->indent;
            }

            if ((temp = htmlAddTree(para, t->element, t->data)) != NULL)
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
            render_doc(t->child, left, right, bottom, top, x, y, page, para,
	              needspace);
          break;
    }


    // Check for some basic stylesheet stuff...
    if ((style = htmlGetStyle(t, "page-break-after:")) != NULL &&
	strcasecmp(style, "avoid") != 0)
    {
      // Advance to the next page...
      page ++;
      *x         = *left;
      *y         = *top;
      *needspace = 0;

      // See if we need to go to the next left/righthand page...
      if (css->sides && (page & 1) &&
          strcasecmp(style, "right") == 0)
	page ++;
      else if (css->sides && !(page & 1) &&
               strcasecmp(style, "left") == 0)
	page ++;

      // Update the progress as necessary...
      if (Verbosity)
	progress_show("Formatting page %d", page);
    }

    // Move to the next node...
    t = t->next;
  }

  if (para->child != NULL && cpara != para)
  {
    render_paragraph(para, *left, *right, *bottom, *top, x, y, page, *needspace);
    htmlDeleteTree(para->child);
    para->child = para->last_child = NULL;
    *needspace  = 1;
  }

  if (cpara != para)
    htmlDeleteTree(para);
}


//
 * 'render_heading()' - Parse a heading tree and produce rendering list output.


static void
render_heading(hdTree *t,	// I - Tree to parse
              float  left,	// I - Left margin
              float  right,	// I - Printable width
              float  bottom,	// I - Bottom margin
              float  top,	// I - Printable top
              float  &x,	// IO - X position
              float  &y,	// IO - Y position
              int    &page,	// IO - Page #
              int    needspace)	// I - Need whitespace?
{
  int	*temp;			// Temporary integer array pointer


  DEBUG_printf(("render_heading(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                t, left, right, bottom, top, &x, &y, page, needspace));

  if (((t->element - HD_ELEMENT_H1) < TocLevels || TocLevels == 0) && !title_page)
    current_heading = t->child;

  if (*y < (5 * _htmlSpacings[SIZE_P] + bottom))
  {
    page ++;
    *y = top;
    if (Verbosity)
      progress_show("Formatting page %d", page);
  }

  check_pages(page);

  if (t->element == HD_ELEMENT_H1 && !title_page)
    pages[page].chapter = htmlGetText(current_heading);

  if ((pages[page].heading == NULL || t->element == HD_ELEMENT_H1 ||
      (&page > 0 && pages[page].heading == pages[&page - 1].heading)) &&
      !title_page)
    pages[page].heading = htmlGetText(current_heading);

  if ((t->element - HD_ELEMENT_H1) < TocLevels && !title_page)
  {
    DEBUG_printf(("H%d: heading_pages[%d] = %d\n", t->element - HD_ELEMENT_H1 + 1,
                  num_headings, page - 1));

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

    heading_pages[num_headings] = &page - chapter_starts[1] + 1;
    heading_tops[num_headings]  = (int)(*y + 4 * _htmlSpacings[SIZE_P]);
    num_headings ++;
  }

  render_paragraph(t, left, right, bottom, top, x, y, page, needspace);

  if (t->halignment == ALIGN_RIGHT && t->element == HD_ELEMENT_H1 &&
      OutputType == OUTPUT_BOOK && !title_page)
  {
   //
    * Special case - chapter heading for users manual...
   

    *y = bottom + 0.5f * (top - bottom);
  }
}


//
 * 'render_paragraph()' - Parse a paragraph tree and produce rendering list
 *                       output.


static void
render_paragraph(hdTree *t,	// I - Tree to parse
        	float  left,	// I - Left margin
        	float  right,	// I - Printable width
        	float  bottom,	// I - Bottom margin
        	float  top,	// I - Printable top
        	float  &x,	// IO - X position
        	float  &y,	// IO - Y position
        	int    &page,	// IO - Page #
        	int    needspace)// I - Need whitespace?
{
  int		whitespace;	// Non-zero if a fragment ends in whitespace
  hdTree	*flat,
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
  hdRenderNode	*r;
  uchar		*align,
		*hspace,
		*vspace,
		*link,
		*border;
  float		rgb[3];
  uchar		line[10240],
		*lineptr;
  hdTree	*linetype;
  float		linex,
		linewidth;
  int		firstline;


  DEBUG_printf(("render_paragraph(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                t, left, right, bottom, top, &x, &y, page, needspace));

  flat        = flatten_tree(t->child);
  image_left  = left;
  image_right = right;
  image_y     = 0;

  if (flat == NULL)
    DEBUG_puts("render_paragraph: flat == NULL!");

  // Add leading whitespace...
  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

 //
  * First scan for images with left/right alignment tags...
 

  for (temp = flat, prev = NULL; temp != NULL;)
  {
    if (temp->element == HD_ELEMENT_IMG)
      update_image_size(temp);

    if (temp->element == HD_ELEMENT_IMG &&
        (align = htmlGetVariable(temp, "ALIGN")))
    {
      if ((border = htmlGetVariable(temp, "BORDER")) != NULL)
	borderspace = atof(border);
      else if (temp->link)
	borderspace = 1;
      else
	borderspace = 0;

      borderspace *= css->print_width / _htmlBrowserWidth;

      if (strcasecmp(align, "LEFT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, "VSPACE")) != NULL)
	  *y -= atoi(vspace);

        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  page ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", page);
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
          add_render(page, RENDER_BOX, image_left, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          add_render(page, RENDER_BOX, image_left,
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          add_render(page, RENDER_BOX, image_left + temp->width + borderspace,
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          add_render(page, RENDER_BOX, image_left,
	             *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        *y -= borderspace;

        add_render(page, RENDER_IMAGE, image_left + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find(htmlGetVariable(temp, "REALSRC")));

        *y -= borderspace;

        if (vspace != NULL)
	  *y -= atoi(vspace);

        image_left += temp->width + 2 * borderspace;
	temp_y     = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

        if ((hspace = htmlGetVariable(temp, "HSPACE")) != NULL)
	  image_left += atoi(hspace);

        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;

        free(temp);
        temp = prev;
      }
      else if (strcasecmp(align, "RIGHT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, "VSPACE")) != NULL)
	  *y -= atoi(vspace);

        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  page ++;
	  *y = top;

	  if (Verbosity)
	    progress_show("Formatting page %d", page);
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
          add_render(page, RENDER_BOX, image_right, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
	  // Left
          add_render(page, RENDER_BOX, image_right,
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Right
          add_render(page, RENDER_BOX, image_right + temp->width + borderspace,
	             *y - temp->height - borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
	  // Bottom
          add_render(page, RENDER_BOX, image_right, *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}

        *y -= borderspace;

        add_render(page, RENDER_IMAGE, image_right + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find(htmlGetVariable(temp, "REALSRC")));

        *y -= borderspace;

        if (vspace != NULL)
	  *y -= atoi(vspace);

	temp_y = *y - temp->height;

	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;

        if ((hspace = htmlGetVariable(temp, "HSPACE")) != NULL)
	  image_right -= atoi(hspace);

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
        if (temp->element == HD_ELEMENT_NONE && temp->data[0] == ' ')
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

        if (temp->element == HD_ELEMENT_IMG)
	{
	  if ((border = htmlGetVariable(temp, "BORDER")) != NULL)
	    borderspace = atof(border);
	  else if (temp->link)
	    borderspace = 1;
	  else
	    borderspace = 0;

          borderspace *= css->print_width / _htmlBrowserWidth;

          temp_width += 2 * borderspace;
	}

        prev       = temp;
        temp       = temp->next;
        temp_width += prev->width;

        
        if (prev->element == HD_ELEMENT_BR)
	  break;
      }

      if ((width + temp_width) <= format_width)
      {
        width += temp_width;
        end  = temp;
        flat = temp;

        if (prev->element == HD_ELEMENT_BR)
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

      if (temp->element == HD_ELEMENT_NONE)
        num_chars += strlen(temp->data);

      if (temp->height > height && temp->element != HD_ELEMENT_IMG)
        height = temp->height;
      else if ((0.5 * temp->height) > height && temp->element == HD_ELEMENT_IMG &&
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

      if (temp->element != HD_ELEMENT_IMG)
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

	if ((border = htmlGetVariable(temp, "BORDER")) != NULL)
	  borderspace = atof(border);
	else if (temp->link)
	  borderspace = 1;
	else
	  borderspace = 0;

        borderspace *= css->print_width / _htmlBrowserWidth;

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
      page ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", page);
    }

    firstline = 0;

    if (height == 0.0f)
      height = spacing;

    for (temp = start; temp != end; temp = temp->next)
      if (temp->element != HD_ELEMENT_A)
        break;

    if (temp != NULL && temp->element == HD_ELEMENT_NONE && temp->data[0] == ' ')
    {
      strcpy(temp->data, temp->data + 1);
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
      page ++;
      *y = top;

      if (Verbosity)
        progress_show("Formatting page %d", page);
    }

    *y -= height;

    DEBUG_printf(("    y = %.1f, width = %.1f, height = %.1f\n", &y, width,
                  height));

    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / css->print_length));

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
	  if (flat != NULL && flat->prev->element != HD_ELEMENT_BR && num_chars > 1)
	    char_spacing = (format_width - width) / (num_chars - 1);
	  break;
    }

    while (temp != end)
    {
      if (temp->link != NULL && PSLevel == 0 && Links &&
          temp->element == HD_ELEMENT_NONE)
      {
	temp->red   = (int)(link_color[0] * 255.0);
	temp->green = (int)(link_color[1] * 255.0);
	temp->blue  = (int)(link_color[2] * 255.0);
      }

     //
      * See if we are doing a run of characters in a line and need to
      * output this run...
     

      if (linetype != NULL &&
	  (temp->element != HD_ELEMENT_NONE ||
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

        r = add_render(page, RENDER_TEXT, linex - linewidth, *y + offset,
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

      switch (temp->element)
      {
        case HD_ELEMENT_A :
            if ((link = htmlGetVariable(temp, "NAME")) != NULL)
            {
             //
              * Add a target link...
             

              add_link(link, page, (int)(*y + 6 * height));
            }

	default :
	    temp_width = temp->width;
            break;

        case HD_ELEMENT_NONE :
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

            strcpy(lineptr, temp->data);

            temp_width = temp->width + char_spacing * strlen(lineptr);

	    if (temp->underline || (temp->link && LinkStyle && PSLevel == 0))
	      add_render(page, RENDER_BOX, linex, *y + offset - 1, temp_width, 0, rgb);

	    if (temp->strikethrough)
	      add_render(page, RENDER_BOX, linex, *y + offset + temp->height * 0.25f,
	                 temp_width, 0, rgb);

            linewidth  += temp_width;
            lineptr    += strlen(lineptr);

            if (lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;

	case HD_ELEMENT_IMG :
	    if ((border = htmlGetVariable(temp, "BORDER")) != NULL)
	      borderspace = atof(border);
	    else if (temp->link)
	      borderspace = 1;
	    else
	      borderspace = 0;

            borderspace *= css->print_width / _htmlBrowserWidth;

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
              add_render(page, RENDER_BOX, linex,
	                 *y + offset + temp->height + borderspace,
			 temp->width + 2 * borderspace, borderspace, rgb);
	      // Left
              add_render(page, RENDER_BOX, linex, *y + offset,
                	 borderspace, temp->height + 2 * borderspace, rgb);
	      // Right
              add_render(page, RENDER_BOX, linex + temp->width + borderspace,
	                 *y + offset, borderspace,
			 temp->height + 2 * borderspace, rgb);
	      // Bottom
              add_render(page, RENDER_BOX, linex, *y + offset,
                	 temp->width + 2 * borderspace, borderspace, rgb);
	    }

	    add_render(page, RENDER_IMAGE, linex + borderspace,
	               *y + offset + borderspace, temp->width, temp->height,
		       image_find(htmlGetVariable(temp, "REALSRC")));
            whitespace = 0;
	    temp_width = temp->width + 2 * borderspace;
	    break;
      }

      if (temp->link != NULL)
      {
        link = htmlGetVariable(temp->link, "HREF");

       //
	* Add a page link...
	*/

	if (file_method(link) == NULL)
	{
	  if (file_target(link) != NULL)
	    link = file_target(link) - 1; // Include # sign
	  else
	    link = file_basename(link);
	}

	add_render(page, RENDER_LINK, linex, *y + offset, temp->width,
	           temp->height, link);
      }

      linex += temp_width;
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

      r = add_render(page, RENDER_TEXT, linex - linewidth, *y + offset,
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
 * 'render_pre()' - Parse preformatted text and produce rendering list output.


static void
render_pre(hdTree *t,		// I - Tree to parse
          float  left,		// I - Left margin
          float  right,		// I - Printable width
          float  bottom,	// I - Bottom margin
          float  top,		// I - Printable top
          float  &x,		// IO - X position
          float  &y,		// IO - Y position
          int    &page,		// IO - Page #
          int    needspace)	// I - Need whitespace?
{
  hdTree	*flat, *next;
  uchar		*link,
		line[10240],
		*lineptr,
		*dataptr;
  int		col;
  float		width,
		rgb[3];
  hdRenderNode	*r;


  REF(right);

  DEBUG_printf(("render_pre(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, &x, &y, page));

  if (t->child == NULL)
    return;

  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];

  col  = 0;
  flat = flatten_tree(t->child);

  if (flat->element == HD_ELEMENT_NONE && flat->data != NULL)
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
        page ++;
        *y = top;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
      }

      *x = left;
      *y -= _htmlSizes[t->size];

      if (Verbosity)
        progress_update(100 - (int)(100 * (*y) / css->print_length));
    }

    if (flat->link != NULL)
    {
      link = htmlGetVariable(flat->link, "HREF");

     //
      * Add a page link...
     

      if (file_method(link) == NULL)
      {
	if (file_target(link) != NULL)
	  link = file_target(link) - 1; // Include # sign
	else
	  link = file_basename(link);
      }

      add_render(page, RENDER_LINK, &x, &y, flat->width,
	         flat->height, link);

      if (PSLevel == 0 && Links)
      {
        memcpy(rgb, link_color, sizeof(rgb));

	flat->red   = (int)(link_color[0] * 255.0);
	flat->green = (int)(link_color[1] * 255.0);
	flat->blue  = (int)(link_color[2] * 255.0);

        if (LinkStyle)
	  add_render(page, RENDER_BOX, &x, *y - 1, flat->width, 0,
	             link_color);
      }
    }

    switch (flat->element)
    {
      case HD_ELEMENT_A :
          if ((link = htmlGetVariable(flat, "NAME")) != NULL)
          {
           //
            * Add a target link...
           

            add_link(link, page, (int)(*y + 6 * t->height));
          }
          break;

      case HD_ELEMENT_BR :
          col = 0;
          *y  -= _htmlSpacings[t->size] - _htmlSizes[t->size];
          break;

      case HD_ELEMENT_NONE :
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
          r = add_render(page, RENDER_TEXT, &x, &y, width, 0, line);
          r->data.text.typeface = flat->typeface;
          r->data.text.style    = flat->style;
          r->data.text.size     = _htmlSizes[flat->size];
          memcpy(r->data.text.rgb, rgb, sizeof(rgb));

	  if (flat->underline)
	    add_render(page, RENDER_BOX, &x, *y - 1, flat->width, 0, rgb);

	  if (flat->strikethrough)
	    add_render(page, RENDER_BOX, &x, *y + flat->height * 0.25f,
	               flat->width, 0, rgb);

          *x += flat->width;

          if (*dataptr == '\n')
          {
            col = 0;
            *y  -= _htmlSpacings[t->size] - _htmlSizes[t->size];
          }
          break;

      case HD_ELEMENT_IMG :
	  add_render(page, RENDER_IMAGE, &x, &y, flat->width, flat->height,
		     image_find(htmlGetVariable(flat, "REALSRC")));

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
 * 'render_table()' - Parse a table and produce rendering output.


static void
render_table(hdTree *t,		// I - Tree to parse
            float  left,	// I - Left margin
            float  right,	// I - Printable width
            float  bottom,	// I - Bottom margin
            float  top,		// I - Printable top
            float  &x,		// IO - X position
            float  &y,		// IO - Y position
            int    &page,	// IO - Page #
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
  hdTree	*temprow,
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
  hdRenderNode	*cell_bg[MAX_COLUMNS];		// Background rectangles
  hdRenderNode	*cell_start[MAX_COLUMNS];	// Start of the content for a cell in the row
  hdRenderNode	*cell_end[MAX_COLUMNS];		// End of the content for a cell in a row
  uchar		*bgcolor;
  float		rgb[3],
		bgrgb[3];


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("render_table(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, &x, &y, page));

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

  if ((var = htmlGetVariable(t, "WIDTH")) != NULL)
  {
    if (var[strlen(var) - 1] == '%')
      table_width = atof(var) * (right - left) / 100.0f;
    else
      table_width = atoi(var) * css->print_width / _htmlBrowserWidth;
  }
  else
    table_width = right - left;

  DEBUG_printf(("table_width = %.1f\n", table_width));

  if ((var = htmlGetVariable(t, "CELLPADDING")) != NULL)
    cellpadding = atoi(var);
  else
    cellpadding = 1.0f;

  if ((var = htmlGetVariable(t, "CELLSPACING")) != NULL)
    cellspacing = atoi(var);
  else
    cellspacing = 0.0f;

  if ((var = htmlGetVariable(t, "BORDER")) != NULL)
  {
    if ((border = atof(var)) == 0.0 && var[0] != '0')
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

  cellspacing *= css->print_width / _htmlBrowserWidth;
  cellpadding *= css->print_width / _htmlBrowserWidth;
  border      *= css->print_width / _htmlBrowserWidth;
  border_size *= css->print_width / _htmlBrowserWidth;

  temp_bottom = bottom - cellpadding;
  temp_top    = top + cellpadding;

  memset(row_spans, 0, sizeof(row_spans));
  memset(span_heights, 0, sizeof(span_heights));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;

    if (temprow->element == HD_ELEMENT_CAPTION)
    {
      render_paragraph(temprow, left, right, bottom, top, x, y, page, needspace);
      needspace = 1;
    }
    else if (temprow->element == HD_ELEMENT_TR ||
             ((temprow->element == HD_ELEMENT_TBODY || temprow->element == HD_ELEMENT_THEAD ||
               temprow->element == HD_ELEMENT_TFOOT) && temprow->child != NULL))
    {
      // Descend into table body as needed...
      if (temprow->element == HD_ELEMENT_TBODY || temprow->element == HD_ELEMENT_THEAD ||
          temprow->element == HD_ELEMENT_TFOOT)
        temprow = temprow->child;

      // Figure out the next row...
      if ((tempnext = temprow->next) == NULL)
        if (temprow->parent->element == HD_ELEMENT_TBODY ||
            temprow->parent->element == HD_ELEMENT_THEAD ||
            temprow->parent->element == HD_ELEMENT_TFOOT)
          tempnext = temprow->parent->next;

      // Allocate memory for the table as needed...
      if (num_rows >= alloc_rows)
      {
        alloc_rows += ALLOC_ROWS;

        if (alloc_rows == ALLOC_ROWS)
	  cells = (hdTree ***)malloc(sizeof(hdTree **) * alloc_rows);
	else
	  cells = (hdTree ***)realloc(cells, sizeof(hdTree **) * alloc_rows);

        if (cells == (hdTree ***)0)
	{
	  progress_error(HD_ERROR_OUT_OF_MEMORY,
                         "Unable to allocate memory for table!");
	  return;
	}
      }	

      if ((cells[num_rows] = (hdTree **)calloc(sizeof(hdTree *), MAX_COLUMNS)) == NULL)
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
        if (tempcol->element == HD_ELEMENT_TD || tempcol->element == HD_ELEMENT_TH)
        {
	  // Handle colspan and rowspan stuff...
          if ((var = htmlGetVariable(tempcol, "COLSPAN")) != NULL)
            colspan = atoi(var);
          else
            colspan = 1;

          if ((var = htmlGetVariable(tempcol, "ROWSPAN")) != NULL)
	  {
            row_spans[col] = atoi(var);

	    for (tcol = 1; tcol < colspan; tcol ++)
              row_spans[col + tcol] = row_spans[col];
          }

          // Compute the cell size...
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min,
	                            &col_pref, &col_height);
          if ((var = htmlGetVariable(tempcol, "WIDTH")) != NULL)
	  {
	    if (var[strlen(var) - 1] == '%')
              col_width -= 2.0 * cellpadding - cellspacing;
	  }
	  else if (htmlGetVariable(tempcol, "NOWRAP") != NULL)
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
 

  if ((var = htmlGetVariable(t, "WIDTH")) != NULL)
  {
    if (var[strlen(var) - 1] == '%')
      width = atof(var) * (right - left) / 100.0f;
    else
      width = atoi(var) * css->print_width / _htmlBrowserWidth;
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

  // Pass three enforces any hard or minimum widths for COLSPAN'd
  // columns...
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
          cells[row][0]->parent->prev->element == HD_ELEMENT_COMMENT)
        render_comment(cells[row][0]->parent->prev,
                      &left, &right, &temp_bottom, &temp_top, x, y,
		      page, NULL, 0);

     //
      * Get height...
     

      if ((height_var = htmlGetVariable(t, "HEIGHT")) == NULL)
	if ((height_var = htmlGetVariable(cells[row][0]->parent,
                           	          "HEIGHT")) == NULL)
	  for (col = 0; col < num_cols; col ++)
	    if (htmlGetVariable(cells[row][col], "ROWSPAN") == NULL)
	      if ((height_var = htmlGetVariable(cells[row][col],
                                                "HEIGHT")) != NULL)
	        break;
    }

    if (cells[row][0] != NULL && height_var != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (height_var[strlen(height_var) - 1] == '%')
	temp_height = atof(height_var) * 0.01f *
	              (css->print_length - 2 * cellpadding);
      else
        temp_height = atof(height_var) * css->print_width / _htmlBrowserWidth;

      if (htmlGetVariable(t, "HEIGHT") != NULL)
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

      if (temp_height > (css->page_length / 8) && height_var == NULL)
	temp_height = css->page_length / 8;
    }

    DEBUG_printf(("BEFORE row = %d, temp_height = %.1f, *y = %.1f\n",
                  row, temp_height, *y));

    if (*y < (bottom + 2 * cellpadding + temp_height) &&
        temp_height <= (top - bottom - 2 * cellpadding))
    {
      DEBUG_puts("NEW PAGE");

      *y = top;
      page ++;

      if (Verbosity)
        progress_show("Formatting page %d", page);
    }

    do_valign  = 1;
    row_y      = *y - cellpadding;
    row_page   = &page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row_y = %.1f, *y = %.1f\n", row_y, *y));

    for (col = 0, rowspan = 9999; col < num_cols; col += colspan)
    {
      if (row_spans[col] == 0)
      {
        if ((var = htmlGetVariable(cells[row][col], "ROWSPAN")) != NULL)
          row_spans[col] = atoi(var);

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
      temp_page = &page;
      tempspace = 0;

      if (row == 0 || cells[row][col] != cells[row - 1][col])
      {
        check_pages(page);

        if (cells[row][col] == NULL)
	  bgcolor = NULL;
	else if ((bgcolor = htmlGetVariable(cells[row][col], "BGCOLOR")) == NULL)
          if ((bgcolor = htmlGetVariable(cells[row][col]->parent, "BGCOLOR")) == NULL)
	    bgcolor = htmlGetVariable(t, "BGCOLOR");

	if (bgcolor != NULL)
	{
          get_color(bgcolor, bgrgb, 0);

	  width       = col_rights[col + colspan] - col_lefts[col] +
        	        2 * cellpadding;
	  border_left = col_lefts[col] - cellpadding;

          cell_bg[col] = add_render(page, RENDER_BOX, border_left, row_y,
                                    width + border, 0.0, bgrgb);
	}
	else
	  cell_bg[col] = NULL;

	cell_start[col] = pages[page].last;
	cell_page[col]  = temp_page;
	cell_y[col]     = temp_y;

        if (cells[row][col] != NULL && cells[row][col]->child != NULL)
	{
	  DEBUG_printf(("    parsing cell %d,%d; width = %.1f\n", row, col,
	                col_rights[col + colspan] - col_lefts[col]));

          bottom += cellpadding;
	  top    -= cellpadding;

          render_doc(cells[row][col]->child,
                    col_lefts + col, col_rights + col + colspan,
                    &bottom, &top,
                    x, &temp_y, &temp_page, NULL, &tempspace);

          bottom -= cellpadding;
	  top    += cellpadding;
        }

        cell_endpage[col] = temp_page;
        cell_endy[col]    = temp_y;
        cell_height[col]  = *y - cellpadding - temp_y;
        cell_end[col]     = pages[page].last;

        if (cell_start[col] == NULL)
	  cell_start[col] = pages[page].first;

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
                  row, row_y, row_height, &y, do_valign));

   //
    * Do the vertical alignment
   

    if (do_valign)
    {
      if (height_var != NULL)
      {
        // Hardcode the row height...
        if (height_var[strlen(height_var) - 1] == '%')
	  temp_height = atof(height_var) * 0.01f * css->print_length;
	else
          temp_height = atof(height_var) * css->print_width / _htmlBrowserWidth;

        if (htmlGetVariable(t, "HEIGHT") != NULL)
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
        hdRenderNode	*p;
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

      if ((bgcolor = htmlGetVariable(cells[row][col], "BGCOLOR")) == NULL)
        if ((bgcolor = htmlGetVariable(cells[row][col]->parent, "BGCOLOR")) == NULL)
	  bgcolor = htmlGetVariable(t, "BGCOLOR");

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
          add_render(cell_page[col], RENDER_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          add_render(cell_page[col], RENDER_BOX, border_left, bottom,
                     border, cell_y[col] - bottom + cellpadding + border, rgb);
	  // Right
          add_render(cell_page[col], RENDER_BOX,
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
            add_render(temp_page, RENDER_BOX, border_left, bottom,
                       border, top - bottom, rgb);
	    // Right
            add_render(temp_page, RENDER_BOX,
	               border_left + width, bottom,
		       border, top - bottom, rgb);
          }

	  if (bgcolor != NULL)
            add_render(temp_page, RENDER_BOX, border_left, bottom,
                       width + border, top - bottom, bgrgb,
		       pages[temp_page].first);
        }

        if (border > 0.0f)
	{
	 //
	  * |   |   |   |
	  * +---+---+---+
	 

	  // Left
          add_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
                     border, top - row_y, rgb);
	  // Right
          add_render(cell_endpage[col], RENDER_BOX,
	             border_left + width, row_y,
                     border, top - row_y, rgb);
	  // Bottom
          add_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
        }

        if (bgcolor != NULL)
          add_render(cell_endpage[col], RENDER_BOX, border_left, row_y,
	             width + border, top - row_y, bgrgb,
		     pages[cell_endpage[col]].first);
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
          add_render(cell_page[col], RENDER_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          add_render(cell_page[col], RENDER_BOX, border_left, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Right
          add_render(cell_page[col], RENDER_BOX,
	             border_left + width, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Bottom
          add_render(cell_page[col], RENDER_BOX, border_left, row_y,
                     width + border, border, rgb);
	}

        if (bgcolor != NULL)
	{
	  cell_bg[col]->y      = row_y;
	  cell_bg[col]->height = cell_y[col] - row_y + cellpadding + border;
	}
      }
    }

    &page = row_page;
    *y    = row_y;

    if (row < (num_rows - 1))
      (*y) -= cellspacing;

    DEBUG_printf(("END row = %d, *y = %.1f, page = %d\n", row, &y, page));
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
 * 'render_list()' - Parse a list entry and produce rendering output.


static void
render_list(hdTree *t,		// I - Tree to parse
           float  *left,	// I - Left margin
           float  *right,	// I - Printable width
           float  *bottom,	// I - Bottom margin
           float  *top,		// I - Printable top
           float  &x,		// IO - X position
           float  &y,		// IO - Y position
           int    &page,	// IO - Page #
           int    needspace)	// I - Need whitespace?
{
  uchar		number[255];	// List number (for numbered types)
  uchar		*value;		// VALUE= variable
  int		typeface;	// Typeface of list number
  float		width;		// Width of list number
  hdRenderNode	*r;		// Render primitive
  int		oldpage;	// Old page value
  float		oldy;		// Old Y value
  float		tempx;		// Temporary X value


  DEBUG_printf(("render_list(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, *left, *right, &x, &y, page));

  if (needspace && *y < *top)
  {
    *y        -= _htmlSpacings[t->size];
    needspace = 0;
  }

  check_pages(page);

  oldy    = *y;
  oldpage = &page;
  r       = pages[page].last;
  tempx   = *x;

  if (t->indent == 0)
  {
    // Adjust left margin when no UL/OL/DL is being used...
    *left += _htmlSizes[t->size];
    tempx += _htmlSizes[t->size];
  }

  render_doc(t->child, left, right, bottom, top, &tempx, y, page, NULL,
            &needspace);

  // Handle when paragraph wrapped to new page...
  if (&page != oldpage)
  {
    // First see if anything was added to the old page...
    if ((r != NULL && r->next == NULL) || pages[oldpage].last == NULL)
    {
      // No, put the symbol on the next page...
      oldpage = &page;
      oldy    = *top;
    }
  }
    
  if ((value = htmlGetVariable(t, "VALUE")) != NULL)
  {
    if (isdigit(value[0]))
      list_values[t->indent] = atoi(value);
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
        strcpy(number, format_number(list_values[t->indent],
	                                     list_types[t->indent]));
        strcat(number, ". ");
        typeface = t->typeface;
        break;

    default :
        sprintf(number, "%c ", list_types[t->indent]);
        typeface = TYPE_SYMBOL;
        break;
  }

  width = get_width(number, typeface, t->style, t->size);

  r = add_render(oldpage, RENDER_TEXT, *left - width, oldy - _htmlSizes[t->size],
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
init_list(hdTree *t)		// I - List entry
{
  uchar		*type,		// TYPE= variable
		*value;		// VALUE= variable
  static uchar	*symbols = "\327\267\250\340";


  if ((type = htmlGetVariable(t, "TYPE")) != NULL)
  {
    if (strlen(type) == 1)
      list_types[t->indent] = type[0];
    else if (strcasecmp(type, "disc") == 0 ||
             strcasecmp(type, "circle") == 0)
      list_types[t->indent] = symbols[1];
    else
      list_types[t->indent] = symbols[2];
  }
  else if (t->element == HD_ELEMENT_UL)
    list_types[t->indent] = symbols[t->indent & 3];
  else if (t->element == HD_ELEMENT_OL)
    list_types[t->indent] = '1';

  if ((value = htmlGetVariable(t, "VALUE")) == NULL)
    value = htmlGetVariable(t, "START");

  if (value != NULL)
  {
    if (isdigit(value[0]))
      list_values[t->indent] = atoi(value);
    else if (isupper(value[0]))
      list_values[t->indent] = value[0] - 'A' + 1;
    else
      list_values[t->indent] = value[0] - 'a' + 1;
  }
  else if (t->element == HD_ELEMENT_OL)
    list_values[t->indent] = 1;
}


//
 * 'render_comment()' - Parse a comment for HTMLDOC comments.


static void
render_comment(hdTree *t,	// I - Tree to parse
              float  *left,	// I - Left margin
              float  *right,	// I - Printable width
              float  *bottom,	// I - Bottom margin
              float  *top,	// I - Printable top
              float  &x,	// IO - X position
              float  &y,	// IO - Y position
              int    &page,	// IO - Page #
	      hdTree *para,	// I - Current paragraph
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
      para->child->child == NULL && para->child->element == HD_ELEMENT_NONE &&
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      page ++;
      if (Verbosity)
	progress_show("Formatting page %d", page);
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      page ++;
      if (Verbosity)
	progress_show("Formatting page %d", page);
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      page ++;
      if (css->sides && (page & 1))
	page ++;

      if (Verbosity)
	progress_show("Formatting page %d", page);
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      halfway = 0.5f * (*top + *bottom);

      if (*y <= halfway)
      {
	page ++;
	if (Verbosity)
	  progress_show("Formatting page %d", page);
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
      // <!-- NEED amount --> generate a page break if there isn't
      // enough remaining space...
      comment += 5;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if ((*y - get_measurement(comment, _htmlSpacings[SIZE_P])) < *bottom)
      {
	page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (css->sides && (page & 1))
	  page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(page);
      
      // Get color...
      if (*comment == '\"')
      {
	for (ptr = pages[page].media_color, comment ++;
             *comment && *comment != '\"';
	     comment ++)
          if (ptr < (pages[page].media_color +
	             sizeof(pages[page].media_color) - 1))
	    *ptr++ = *comment;

        if (*comment == '\"')
	  comment ++;
      }
      else
      {
	for (ptr = pages[page].media_color;
             *comment && !isspace(*comment);
	     comment ++)
          if (ptr < (pages[page].media_color +
	             sizeof(pages[page].media_color) - 1))
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (css->sides && (page & 1))
	  page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(page);

      pages[page].media_position = atoi(comment);

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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (css->sides && (page & 1))
	  page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(page);
      
      // Get type...
      if (*comment == '\"')
      {
	for (ptr = pages[page].media_type, comment ++;
             *comment && *comment != '\"';
	     comment ++)
          if (ptr < (pages[page].media_type +
	             sizeof(pages[page].media_type) - 1))
	    *ptr++ = *comment;

        if (*comment == '\"')
	  comment ++;
      }
      else
      {
	for (ptr = pages[page].media_type;
             *comment && !isspace(*comment);
	     comment ++)
          if (ptr < (pages[page].media_type +
	             sizeof(pages[page].media_type) - 1))
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

        tof = 1;
      }

      if (css->sides && (page & 1))
	page ++;

      if (Verbosity)
	progress_show("Formatting page %d", page);

      check_pages(page);

      *right = css->print_width - *right;
      *top   = css->print_length - *top;

      set_page_size(comment);

      if (css->orientation)
      {
	css->print_width  = css->page_length - css->page_left - css->page_right;
	css->print_length = css->page_width - css-> - css->bottom;
      }
      else
      {
	css->print_width  = css->page_width - css->page_left - css->page_right;
	css->print_length = css->page_length - css-> - css->bottom;
      }

      *right = css->print_width - *right;
      *top   = css->print_length - *top;

      *x = *left;
      *y = *top;

      pages[page].width  = css->page_width;
      pages[page].length = css->page_length;
      css->print_width      = css->page_width - css->page_right - css->page_left;
      css->print_length     = css->page_length - css-> - css->bottom;

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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(page);

      *right         = css->print_width - *right;
      css->page_left       = pages[page].left = get_measurement(comment);
      css->print_width = css->page_width - css->page_right - css->page_left;
      *right         = css->print_width - *right;

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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
	*y = *top;
        tof = 1;
      }

      *x = *left;

      check_pages(page);

      *right         = css->print_width - *right;
      css->page_right      = pages[page].right = get_measurement(comment);
      css->print_width = css->page_width - css->page_right - css->page_left;
      *right         = css->print_width - *right;

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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);
        tof = 1;
      }

      *x = *left;

      check_pages(page);

      *top            = css->print_length - *top;
      css->bottom      = pages[page].bottom = get_measurement(comment);
      css->print_length = css->page_length - css-> - css->bottom;
      *top            = css->print_length - *top;
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	if (Verbosity)
	  progress_show("Formatting page %d", page);

        tof = 1;
      }

      *x = *left;

      check_pages(page);

      *top            = css->print_length - *top;
      css->         = pages[page].top = get_measurement(comment);
      css->print_length = css->page_length - css-> - css->bottom;
      *top            = css->print_length - *top;
      *y              = *top;

      // Skip top...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "MEDIA LANDSCAPE ", 16) == 0)
    {
      // css->orientation on/off...
      comment += 16;

      while (isspace(*comment))
	comment ++;

      if (!*comment)
	break;

      if (para != NULL && para->child != NULL)
      {
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

        tof = 1;
      }

      if (css->sides && (page & 1))
	page ++;

      if (Verbosity)
	progress_show("Formatting page %d", page);

      *x = *left;

      check_pages(page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
      {
        if (css->orientation)
	{
	  *right         = css->page_length - css->page_right - *right;
	  css->print_width = css->page_width - css->page_right - css->page_left;
	  *right         = css->page_width - css->page_right - *right;

	  *top            = css->page_width - css-> - *top;
	  css->print_length = css->page_length - css-> - css->bottom;
	  *top            = css->page_length - css-> - *top;
        }

        css->orientation = pages[page].landscape = 0;
      }
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
        if (!css->orientation)
	{
	  *top           = css->page_length - css-> - *top;
	  css->print_width = css->page_width - css-> - css->page_left;
	  *top           = css->page_width - css-> - *top;

	  *right          = css->page_width - css->page_right - *right;
	  css->print_length = css->page_length - css->page_right - css->bottom;
	  *right          = css->page_length - css->page_right - *right;
        }

        css->orientation = pages[page].landscape = 1;
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
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
	htmlDeleteTree(para->child);
	para->child = para->last_child = NULL;
      }

      if (!tof)
      {
	page ++;

	*y = *top;
        tof = 1;
      }

      if (css->sides && (page & 1))
	page ++;

      if (Verbosity)
	progress_show("Formatting page %d", page);

      *x = *left;

      check_pages(page);

      if (strncasecmp(comment, "OFF", 3) == 0 || tolower(comment[0]) == 'n')
        css->sides = pages[page].duplex = 0;
      else if (strncasecmp(comment, "ON", 2) == 0 || tolower(comment[0]) == 'y')
      {
	if (page & 1)
	{
	  page ++;

          check_pages(page);

	  if (Verbosity)
	    progress_show("Formatting page %d", page);
	}

        css->sides = pages[page].duplex = 1;
      }

      // Skip duplex...
      while (*comment && !isspace(*comment))
        comment ++;
    }
    else if (strncasecmp(comment, "HEADER ", 7) == 0)
    {
      // doc_header string...
      comment += 7;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
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
        doc_header[pos] = strdup(buffer);
      else
        doc_header[pos] = NULL;

      if (tof)
      {
	check_pages(page);

	pages[page].header[pos] = doc_header[pos];
      }

      // Adjust top margin as needed...
      for (pos = 0; pos < 3; pos ++)
        if (doc_header[pos])
	  break;

      if (pos < 3)
      {
	if (logo_height > HeadFootSize)
          css-> = (int)(logo_height + HeadFootSize);
	else
          css-> = (int)(2 * HeadFootSize);
      }

      if (tof)
        *y = *top = css->print_length - css->;
    }
    else if (strncasecmp(comment, "FOOTER ", 7) == 0)
    {
      // doc_header string...
      comment += 7;

      while (isspace(*comment))
	comment ++;

      if (para != NULL && para->child != NULL)
      {
	render_paragraph(para, *left, *right, *bottom, *top, x, y, page, needspace);
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
        doc_footer[pos] = strdup(buffer);
      else
        doc_footer[pos] = NULL;

      if (tof)
      {
	check_pages(page);

	pages[page].footer[pos] = doc_footer[pos];
      }

      // Adjust bottom margin as needed...
      for (pos = 0; pos < 3; pos ++)
        if (doc_footer[pos])
	  break;

      if (pos == 3)
        css->bottom = 0;
      else if (logo_height > HeadFootSize)
        css->bottom = (int)(logo_height + HeadFootSize);
      else
        css->bottom = (int)(2 * HeadFootSize);

      if (tof)
        *bottom = css->bottom;
    }
    else
      break;
  }
}



//
 * 'check_pages()' - Allocate memory for more pages as needed...


static void
check_pages(int page)	// I - Current page
{
  hdRenderPage	*temp;	// Temporary page pointer


  DEBUG_printf(("check_pages(%d)\n", page));

  // See if we need to allocate memory for the page...
  if (page >= alloc_pages)
  {
    // Yes, allocate enough for ALLOC_PAGES more pages...
    alloc_pages += ALLOC_PAGES;

    // Do the pages pointers...
    if (num_pages == 0)
      temp = (hdRenderPage *)malloc(sizeof(hdRenderPage) * alloc_pages);
    else
      temp = (hdRenderPage *)realloc(pages, sizeof(hdRenderPage) * alloc_pages);

    if (temp == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for %d pages - %s",
	             alloc_pages, strerror(errno));
      alloc_pages -= ALLOC_PAGES;
      return;
    }

    memset(temp + alloc_pages - ALLOC_PAGES, 0, sizeof(hdRenderPage) * ALLOC_PAGES);

    pages = temp;
  }

  // Initialize the page data as needed...
  for (temp = pages + num_pages; num_pages <= page; num_pages ++, temp ++)
    if (!temp->width)
    {
      if (num_pages == 0 || !temp[-1].width || !temp[-1].length)
      {
	temp->width     = page_width;
	temp->length    = page_length;
	temp->left      = page_left;
	temp->right     = page_right;
	temp->top       = page_top;
	temp->bottom    = page_bottom;
	temp->duplex    = sides;
	temp->landscape = orientation;
      }
      else
      {
	memcpy(temp, temp - 1, sizeof(hdRenderPage));
	temp->start = NULL;
	temp->end   = NULL;
      }

      if (chapter == 0)
      {
	memcpy(temp->header, toc_header, sizeof(temp->header));
	memcpy(temp->footer, toc_footer, sizeof(temp->footer));
      }
      else
      {
	memcpy(temp->header, doc_header, sizeof(temp->header));
	memcpy(temp->footer, doc_footer, sizeof(temp->footer));
      }

      memcpy(temp->background_color, background_color,
             sizeof(temp->background_color));
      temp->background_image = background_image;
    }
}


//
 * 'copy_tree()' - Copy a markup tree...


static void
copy_tree(hdTree *parent,	// I - Source tree
          hdTree *t)		// I - Destination tree
{
  int		i;		// I - Looping var
  hdTree	*temp;		// I - New tree entry
  var_t		*var;		// I - Current markup variable


  while (t != NULL)
  {
    if ((temp = htmlAddTree(parent, t->element, t->data)) != NULL)
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


//
 * 'flatten_tree()' - Flatten an HTML tree to only include the text, image,
 *                    link, and break markups.


static hdTree *			// O - Flattened markup tree
flatten_tree(hdTree *t)		// I - Markup tree to flatten
{
  hdTree	*temp,		// New tree node
		*flat;		// Flattened tree


  flat = NULL;

  while (t != NULL)
  {
    switch (t->element)
    {
      case HD_ELEMENT_NONE :
          if (t->data == NULL)
	    break;
      case HD_ELEMENT_BR :
      case HD_ELEMENT_SPACER :
      case HD_ELEMENT_IMG :
	  temp = (hdTree *)calloc(sizeof(hdTree), 1);
	  memcpy(temp, t, sizeof(hdTree));
	  temp->parent = NULL;
	  temp->child  = NULL;
	  temp->prev   = flat;
	  temp->next   = NULL;
	  if (flat != NULL)
            flat->next = temp;
          flat = temp;

          if (temp->element == HD_ELEMENT_IMG)
            update_image_size(temp);
          break;

      case HD_ELEMENT_A :
          if (htmlGetVariable(t, "NAME") != NULL)
          {
	    temp = (hdTree *)calloc(sizeof(hdTree), 1);
	    memcpy(temp, t, sizeof(hdTree));
	    temp->parent = NULL;
	    temp->child  = NULL;
	    temp->prev   = flat;
	    temp->next   = NULL;
	    if (flat != NULL)
              flat->next = temp;
            flat = temp;
          }
	  break;

      case HD_ELEMENT_P :
      case HD_ELEMENT_PRE :
      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_OL :
      case HD_ELEMENT_DL :
      case HD_ELEMENT_LI :
      case HD_ELEMENT_DD :
      case HD_ELEMENT_DT :
      case HD_ELEMENT_TR :
      case HD_ELEMENT_CAPTION :
	  temp = (hdTree *)calloc(sizeof(hdTree), 1);
	  temp->element = HD_ELEMENT_BR;
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
update_image_size(hdTree *t)	// I - Tree entry
{
  image_t	*img;		// Image file
  uchar		*width,		// Width string
		*height;	// Height string


  width  = htmlGetVariable(t, "WIDTH");
  height = htmlGetVariable(t, "HEIGHT");

  if (width != NULL && height != NULL)
  {
    if (width[strlen(width) - 1] == '%')
      t->width = atof(width) * css->print_width / 100.0f;
    else
      t->width = atoi(width) * css->print_width / _htmlBrowserWidth;

    if (height[strlen(height) - 1] == '%')
      t->height = atof(height) * css->print_width / 100.0f;
    else
      t->height = atoi(height) * css->print_width / _htmlBrowserWidth;

    return;
  }

  img = image_find(htmlGetVariable(t, "REALSRC"));

  if (img == NULL)
    return;

  if (width != NULL)
  {
    if (width[strlen(width) - 1] == '%')
      t->width = atof(width) * css->print_width / 100.0f;
    else
      t->width = atoi(width) * css->print_width / _htmlBrowserWidth;

    t->height = t->width * img->height / img->width;
  }
  else if (height != NULL)
  {
    if (height[strlen(height) - 1] == '%')
      t->height = atof(height) * css->print_width / 100.0f;
    else
      t->height = atoi(height) * css->print_width / _htmlBrowserWidth;

    t->width = t->height * img->width / img->height;
  }
  else
  {
    t->width  = img->width * css->print_width / _htmlBrowserWidth;
    t->height = img->height * css->print_width / _htmlBrowserWidth;
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
get_title(hdTree *doc)	// I - Document
{
  uchar	*temp;


  while (doc != NULL)
  {
    if (doc->element == HD_ELEMENT_TITLE)
      return (htmlGetText(doc->child));
    else if (doc->child != NULL)
      if ((temp = get_title(doc->child)) != NULL)
        return (temp);
    doc = doc->next;
  }

  return (NULL);
}
#endif // 0


//
// End of "$Id: render.cxx,v 1.14 2004/02/03 02:55:28 mike Exp $".
//
