//
// "$Id: stylemedia.cxx,v 1.4 2004/03/31 09:35:38 mike Exp $"
//
//   CSS media routines for HTMLDOC, a HTML document processing program.
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
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdStyleMedia::hdStyleMedia()      - Initialize an hdStyleMedia structure.
//   hdStyleMedia::set_margins()       - Set the page margins.
//   hdStyleMedia::set_orientation()   - Set the page orientation.
//   hdStyleMedia::set_size()          - Set the page size by numbers.
//   hdStyleMedia::set_size()          - Set the page size by name.
//   hdStyleMedia::update_printable()  - Update the printable page area.
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include "hdstring.h"
//#define DEBUG


//
// 'hdStyleMedia::hdStyleMedia()' - Initialize an hdStyleMedia structure.
//

hdStyleMedia::hdStyleMedia()
{
  memset(this, 0, sizeof(hdStyleMedia));

  // Set the default page to "Universal" with half-inch margins all the
  // way around...

  set_orientation(HD_ORIENTATION_PORTRAIT);
  set_size(595.0f, 792.0f);
  set_margins(36.0f, 36.0f, 36.0f, 36.0f);
  update_printable();
}


//
// 'hdStyleMedia::set_margins()' - Set the page margins.
//

void
hdStyleMedia::set_margins(float l,	// I - Left margin in points
                          float b,	// I - Bottom margin in points
			  float r,	// I - Right margin in points
			  float t)	// I - Top margin in points
{
  page_left   = l;
  page_bottom = b;
  page_right  = r;
  page_top    = t;

  update_printable();
}


//
// 'hdStyleMedia::set_orientation()' - Set the page orientation.
//

void
hdStyleMedia::set_orientation(hdOrientation o)	// I - Orientation
{
  orientation = o;

  update_printable();
}


//
// 'hdStyleMedia::set_size()' - Set the page size by numbers.
//

void
hdStyleMedia::set_size(float w,		// I - Width in points
                       float l)		// I - Length in points
{
  hdPageSize	*s;			// Current size record


  // Lookup the size in the size table...
  if ((s = hdGlobal.find_size(w, l)) != NULL)
  {
    // Use the standard size name...
    strncpy(size_name, s->name, sizeof(size_name) - 1);
    size_name[sizeof(size_name) - 1] = '\0';
  }
  else
  {
    // If the size wasn't found, use wNNNhNNN...
    sprintf(size_name, "w%dh%d", (int)w, (int)l);
  }

  // Now set the page size and update the printable area...
  page_width  = w;
  page_length = l;

  update_printable();
}


//
// 'hdStyleMedia::set_size()' - Set the page size by name.
//

void
hdStyleMedia::set_size(const char *name)// I - Page size name
{
  hdPageSize	*s;			// Current size record
  int		w, l;			// Width and length in points


  // Lookup the size in the size table...
  if ((s = hdGlobal.find_size(name)) != NULL)
  {
    // Use the standard size...
    strncpy(size_name, s->name, sizeof(size_name) - 1);
    size_name[sizeof(size_name) - 1] = '\0';

    page_width  = s->width;
    page_length = s->length;

    update_printable();
  }
  else
  {
    // OK, that didn't work; see if the name is of the form "wNNNhNNN"...
    if (sscanf(name, "w%dh%d", &w, &l) == 2)
    {
      // Yes, it is a custom page size; set it...
      strncpy(size_name, name, sizeof(size_name) - 1);
      size_name[sizeof(size_name) - 1] = '\0';

      page_width  = w;
      page_length = l;

      update_printable();
    }
  }
}


//
// 'hdStyleMedia::update_printable()' - Update the printable page area.
//

void
hdStyleMedia::update_printable()
{
  switch (orientation)
  {
    case HD_ORIENTATION_PORTRAIT :
    case HD_ORIENTATION_REVERSE_PORTRAIT :
        page_print_width  = page_width - page_left - page_right;
	page_print_length = page_length - page_top - page_bottom;
	break;

    case HD_ORIENTATION_LANDSCAPE :
    case HD_ORIENTATION_REVERSE_LANDSCAPE :
	page_print_width  = page_length - page_left - page_right;
        page_print_length = page_width - page_top - page_bottom;
	break;
  }
}


//
// End of "$Id: stylemedia.cxx,v 1.4 2004/03/31 09:35:38 mike Exp $".
//
