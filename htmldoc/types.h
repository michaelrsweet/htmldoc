/*
 * "$Id$"
 *
 *   Common data types for HTMLDOC, an HTML document processing program.
 *
 *   Copyright 2011 by Michael R Sweet.
 *   Copyright 1997-2010 by Easy Software Products.
 *
 *   This program is free software.  Distribution and use rights are outlined in
 *   the file "COPYING.txt".
 */

#ifndef _TYPES_H_
#  define _TYPES_H_

#  include "config.h"
#  include <sys/types.h>

typedef unsigned char hdByte;
typedef unsigned char hdChar;
typedef hdChar *hdUTF8;
typedef unsigned int hdWord;


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
