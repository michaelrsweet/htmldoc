/*
 * "$Id: debug.h,v 1.1 1999/11/07 13:33:10 mike Exp $"
 *
 *   Debugging macros for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
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
 * End of "$Id: debug.h,v 1.1 1999/11/07 13:33:10 mike Exp $".
 */
