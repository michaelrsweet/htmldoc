/*
 * "$Id$"
 *
 *   Debugging macros for HTMLDOC, a HTML document processing program.
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
 *       Attn: HTMLDOC Licensing Information
 *       Easy Software Products
 *       516 Rio Grand Ct
 *       Morgan Hill, CA 95037 USA
 *
 *       http://www.htmldoc.org/
 */

#ifndef _DEBUG_H_
#  define _DEBUG_H_

/*
 * Include necessary headers.
 */

#  include <stdio.h>

#  ifdef DEBUG
#    define DEBUG_printf(x) printf x
#    define DEBUG_puts(x)   puts(x)
#  else
#    define DEBUG_printf(x)
#    define DEBUG_puts(x)
#  endif /* DEBUG */

#endif /* !_DEBUG_H_ */

/*
 * End of "$Id$".
 */
