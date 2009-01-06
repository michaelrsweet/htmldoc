//
// "$Id: styletree.cxx,v 1.1.2.4 2005/05/06 15:28:58 mike Exp $"
//
// Document tree stylesheet routines for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2009 by Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//
// Contents:
//
//   hdStyleSheet::find_style()        - Find the default style for the given
//                                       tree node.
//   hdStyleSheet::get_private_style() - Get a private style definition.
//

//
// Include necessary headers.
//

//#define DEBUG
#include "htmldoc.h"
#include "hdstring.h"


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                tree node.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdTree *t)	// I - Tree node
{
  int			i;		// Looping var...
  int			nsels;		// Number of selectors...
  hdStyleSelector	sels[HD_SELECTOR_MAX];
					// Selectors...
  hdTree		*p;		// Tree pointer...


  // Figure out how many selectors to use...
  if (max_selectors[t->element] > HD_SELECTOR_MAX)
    nsels = HD_SELECTOR_MAX;
  else
    nsels = max_selectors[t->element];

  // Build the selectors for this node...
  for (i = 0, p = t; p && i < nsels; i ++, p = t->parent)
  {
    if (p->element < HD_ELEMENT_A)
      sels[i].element = HD_ELEMENT_P;
    else
      sels[i].element = p->element;

    sels[i].class_ = (char *)htmlGetAttr(p, "CLASS");
    sels[i].id     = (char *)htmlGetAttr(p, "ID");

    if (sels[i].element == HD_ELEMENT_A && htmlGetAttr(p, "HREF") != NULL)
      sels[i].pseudo = (char *)"link";
    else
      sels[i].pseudo = NULL;
  }

  // Do the search...
  return (find_style(i, sels));
}


//
// 'hdStyleSheet::get_private_style()' - Get a private style definition.
//

hdStyle	*				// O - New style
hdStyleSheet::get_private_style(
  hdTree *t,				// I - Tree node that needs style
  bool   force)				// I - Force creation?
{
  hdStyle		*style,		// New private style
			*nstyle;	// Node's style
  char			id[16];		// Selector ID
  const char		*style_attr;	// STYLE attribute, if any


  // Find the right style for this node...
  if ((nstyle = _htmlStyleSheet->find_style(t)) == NULL)
  {
    if (t->style)
      nstyle = t->style;
    else
      nstyle = &(_htmlStyleSheet->def_style);
  }

#ifdef DEBUG
  printf("%s style: %s, font-family=\"%s\", font-weight=%d, font-size=%.1f...\n",
         _htmlStyleSheet->get_element(t->element),
         _htmlStyleSheet->get_element(nstyle->selectors[0].element),
	 nstyle->font_family, nstyle->font_weight, nstyle->font_size);
#endif // DEBUG

  // Setup a private selector ID for this node...
  sprintf(id, "_HD_%08X", private_id ++);

  // Create a new style derived from this node...
  hdStyleSelector	selector(t->element, (char *)htmlGetAttr(t, "CLASS"),
                                 t->element == HD_ELEMENT_A &&
				     htmlGetAttr(t, "HREF") ? "link" : NULL,
				 id);	// Selector for private style

  DEBUG_printf(("t->style->white_space=%d, nstyle->white_space=%d\n",
        	t->style ? t->style->white_space : -1,
		nstyle ? nstyle->white_space : -1));
  DEBUG_printf(("t->style->line_height=%.1f, nstyle->line_height=%.1f\n",
        	t->style ? t->style->line_height : -1,
		nstyle ? nstyle->line_height : -1));
  DEBUG_printf(("t->style->font_size=%.1f, nstyle->font_size=%.1f\n",
        	t->style ? t->style->font_size : -1,
		nstyle ? nstyle->font_size : -1));

  style = new hdStyle(1, &selector, t->style);
  DEBUG_printf(("    BEFORE style->white_space=%d\n", style->white_space));
  DEBUG_printf(("    BEFORE style->line_height=%.1f\n", style->line_height));
  DEBUG_printf(("    BEFORE style->font_size=%.1f\n", style->font_size));

  style->inherit(nstyle);
  DEBUG_printf(("    AFTER style->white_space=%d\n", style->white_space));
  DEBUG_printf(("    AFTER style->line_height=%.1f\n", style->line_height));
  DEBUG_printf(("    AFTER style->font_size=%.1f\n", style->font_size));

  // Apply the STYLE attribute for this node, if any...
  if ((style_attr = (char *)htmlGetAttr(t, "STYLE")) != NULL)
    style->load(this, style_attr);

  style->updated = false;

  if (!force && elements[t->element] >= 0)
  {
    // See if there is already a matching style...
    style->update(this);

    int     i, j;			// Looping vars
    hdStyle *existing;			// Existing style

    for (i = elements[t->element]; i < num_styles; i ++)
    {
      existing = styles[i];

      if (existing->selectors[0].element != t->element ||
          (existing->selectors[0].class_ != NULL) != (selector.class_ != NULL) ||
          (existing->selectors[0].class_  &&
	   strcasecmp(existing->selectors[0].class_, selector.class_)) ||
          (existing->selectors[0].pseudo != NULL) != (selector.pseudo != NULL) ||
          (existing->selectors[0].pseudo  &&
	   strcasecmp(existing->selectors[0].pseudo, selector.pseudo)))
        break;

      if (memcmp(style->background_color, existing->background_color, 3) ||
          style->background_color_set != existing->background_color_set ||
	  style->background_position[0] != existing->background_position[0] ||
	  style->background_position[1] != existing->background_position[1] ||
	  style->background_repeat != existing->background_repeat ||
	  style->caption_side != existing->caption_side ||
	  style->clear != existing->clear ||
	  memcmp(style->color, existing->color, 3) ||
          style->color_set != existing->color_set ||
	  style->direction != existing->direction ||
	  style->display != existing->display ||
	  style->float_ != existing->float_ ||
	  style->font != existing->font ||
	  style->font_size != existing->font_size ||
	  style->height != existing->height ||
	  style->letter_spacing != existing->letter_spacing ||
	  style->line_height != existing->line_height ||
	  style->list_style_position != existing->list_style_position ||
	  style->list_style_type != existing->list_style_type ||
	  style->margin[0] != existing->margin[0] ||
	  style->margin[1] != existing->margin[1] ||
	  style->margin[2] != existing->margin[2] ||
	  style->margin[3] != existing->margin[3] ||
	  style->orphans != existing->orphans ||
	  style->padding[0] != existing->padding[0] ||
	  style->padding[1] != existing->padding[1] ||
	  style->padding[2] != existing->padding[2] ||
	  style->padding[3] != existing->padding[3] ||
	  style->page_break_after != existing->page_break_after ||
	  style->page_break_before != existing->page_break_before ||
	  style->page_break_inside != existing->page_break_inside ||
	  style->text_align != existing->text_align ||
	  style->text_decoration != existing->text_decoration ||
	  style->text_indent != existing->text_indent ||
	  style->text_transform != existing->text_transform ||
	  style->unicode_bidi != existing->unicode_bidi ||
	  style->vertical_align != existing->vertical_align ||
	  style->white_space != existing->white_space ||
	  style->width != existing->width ||
	  style->word_spacing != existing->word_spacing)
        continue;

      if ((style->background_image == NULL) !=
              (existing->background_image == NULL))
	continue;

      if (style->background_image &&
          strcmp(style->background_image, existing->background_image))
	continue;

      for (j = 0; j < 4; j ++)
        if (memcmp(style->border[j].color, existing->border[j].color, 3) ||
	    style->border[j].color_set != existing->border[j].color_set ||
	    style->border[j].style != existing->border[j].style ||
	    style->border[j].width != existing->border[j].width)
	  break;

      if (j < 4)
        continue;

      if ((style->list_style_image == NULL) !=
              (existing->list_style_image == NULL))
	continue;

      if (style->list_style_image &&
          strcmp(style->list_style_image, existing->list_style_image))
	continue;

      delete style;

      DEBUG_printf(("Returning existing style for %s, style->white_space=%d, "
        	    "existing->white_space=%d...\n", get_element(t->element),
		    style->white_space, existing->white_space));

      return (existing);
    }
  }

  DEBUG_printf(("Returning private style for %s...\n",
                get_element(t->element)));

  // Add the style to the stylesheet...
  add_style(style);

  // Return the new style...
  return (style);
}


//
// End of "$Id: styletree.cxx,v 1.1.2.4 2005/05/06 15:28:58 mike Exp $".
//
