//
// "$Id: render.cxx,v 1.1 2000/10/16 03:25:09 mike Exp $"
//
//   Render routines for HTMLDOC, a HTML document processing program.
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
// 'HTMLDOC::new_render()' - Allocate memory for a new rendering structure.
//

HDrender *		// O - New render structure
HTMLDOC::new_render(int   page,		// I - Page number (0-n)
           int   type,		// I - Type of render primitive
           float x,		// I - Horizontal position
           float y,		// I - Vertical position
           float width,		// I - Width
           float height,	// I - Height
           void  *data,		// I - Data
	   int   insert)	// I - Insert instead of append?
{
  HDrender		*r;	// New render primitive
  static HDrender	dummy;	// Dummy var for errors...


  if (page < 0 || page >= MAX_PAGES)
  {
    progress_error("Page number (%d) out of range (1...%d)\n", page + 1, MAX_PAGES);
    memset(&dummy, 0, sizeof(dummy));
    return (&dummy);
  }

  if (type == RENDER_IMAGE || data == NULL)
    r = (HDrender *)calloc(sizeof(HDrender), 1);
  else
    r = (HDrender *)calloc(sizeof(HDrender) + strlen((char *)data), 1);

  if (r == NULL)
  {
    progress_error("Unable to allocate memory on page %s\n", page + 1);
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
        r->data.image = (HDtree *)data;
        break;
    case RENDER_BOX :
        memcpy(r->data.box, data, sizeof(r->data.box));
        break;
    case RENDER_FBOX :
        memcpy(r->data.fbox, data, sizeof(r->data.fbox));
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
    r->next     = pages[page];
    pages[page] = r;
    if (r->next == NULL)
      endpages[page] = r;
  }
  else
  {
    if (endpages[page] != NULL)
      endpages[page]->next = r;
    else
      pages[page] = r;

    r->next        = NULL;
    endpages[page] = r;
  }

  if (page >= num_pages)
    num_pages = page + 1;

  return (r);
}


//
// End of "$Id: render.cxx,v 1.1 2000/10/16 03:25:09 mike Exp $".
//
