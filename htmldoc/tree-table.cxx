//
// "$Id: tree-table.cxx,v 1.1 2002/07/24 12:55:54 mike Exp $"
//
//   HTML table formatting routines for HTMLDOC, a HTML document
//   processing program.
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
// 'hdTree::format_table()' - Format a table.
//

void
hdTree::format_table(hdStyleSheet *css,		// I  - Style sheet
                     hdMargin     *m,		// IO - Margins
                     float        &x,		// IO - Current X position
                     float        &y,		// IO - Current Y position
		     int          &page)	// IO - Current page
{
}


//
// 'hdTree::get_cell_size()' - Get the sizes of a cell.
//

float
hdTree::get_cell_size(hdStyleSheet *css,
                      hdMargin     *m,
                      float        &minwidth,
                      float        &prefwidth,
		      float        &minheight)
{
  return (0.0);
}


//
// 'hdTree::get_table_size()' - Get the sizes of a table.
//

float
hdTree::get_table_size(hdStyleSheet *css,
                       hdMargin     *m,
                       float        &minwidth,
                       float        &prefwidth,
		       float        &minheight)
{
  return (0.0);
}


//
// End of "$Id: tree-table.cxx,v 1.1 2002/07/24 12:55:54 mike Exp $".
//
