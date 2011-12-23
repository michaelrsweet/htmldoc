/*
 * "$Id$"
 *
 *   Debugging macros for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 2011 by Michael R Sweet.
 *   Copyright 1997-2010 by Easy Software Products.
 *
 *   This program is free software.  Distribution and use rights are outlined in
 *   the file "COPYING.txt".
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
