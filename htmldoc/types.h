/*
 * "$Id$"
 *
 *   Common data types for HTMLDOC, an HTML document processing program.
 *
 *   Copyright 1997-2005 by Easy Software Products.
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

#ifndef _TYPES_H_
#  define _TYPES_H_

#  include "config.h"

typedef unsigned char hdByte;
typedef unsigned char hdChar;
typedef hdChar *hdUTF8;


//
// C library comparison function type...
//

extern "C"
{
  typedef int (*hdCompareFunc)(const void *, const void *);
}


#endif /* !_TYPES_H_ */

/*
 * End of "$Id$".
 */
