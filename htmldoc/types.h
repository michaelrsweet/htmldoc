/*
 * "$Id: types.h,v 1.14 2004/03/31 10:35:07 mike Exp $"
 *
 *   Common data types for HTMLDOC, an HTML document processing program.
 *
 *   Copyright 1997-2004 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 */

#ifndef HTMLDOC_TYPES_H
#  define HTMLDOC_TYPES_H

#  include "config.h"


//
// Basic data types...
//

typedef unsigned char uchar;
typedef unsigned char hdByte;

//
// C library comparison function type...
//

extern "C"
{
  typedef int (*hdCompareFunc)(const void *, const void *);
}


#endif /* !HTMLDOC_TYPES_H */

/*
 * End of "$Id: types.h,v 1.14 2004/03/31 10:35:07 mike Exp $".
 */
