//
// "$Id: tree-format.cxx,v 1.1 2002/07/24 12:55:54 mike Exp $"
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
// End of "$Id: tree-format.cxx,v 1.1 2002/07/24 12:55:54 mike Exp $".
//
