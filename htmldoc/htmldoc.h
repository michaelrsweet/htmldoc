/*
 * "$Id: htmldoc.h,v 1.37 2004/03/31 20:56:56 mike Exp $"
 *
 *   Header file for HTMLDOC, a HTML document processing program.
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

/*
 * Include necessary headers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "book.h"
#include "image.h"
#include "debug.h"

#ifdef HAVE_LIBFLTK
#  include "gui.h"
#endif /* HAVE_LIBFLTK */

#ifdef WIN32	    /* Include all 8 million Windows header files... */
#  include <windows.h>
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * Macro to get rid of "unreferenced variable xyz" warnings...
 */

#define REF(x)		(void)x;




/*
 * Globals...
 */

#ifdef _HTMLDOC_CXX_
#  define VAR
#  define VALUE(x)	=x
#  define NULL3		={0,0,0}
#else
#  define VAR		extern
#  define VALUE(x)
#  define NULL3
#endif /* _HTMLDOC_CXX_ */

#ifdef HAVE_LIBFLTK
VAR GUI		*BookGUI	VALUE(NULL);	/* GUI for book files */
#  ifdef WIN32					/* Editor for HTML files */
VAR char	HTMLEditor[1024] VALUE("notepad.exe \"%s\"");
#  else
VAR char	HTMLEditor[1024] VALUE("nedit %s");
#  endif /* WIN32 */
VAR int		Tooltips	VALUE(1);	/* Show tooltips? */
VAR int		ModernSkin	VALUE(1);	/* Show modern skins? */
#endif /* HAVE_LIBFLTK */


/*
 * Prototypes...
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*
 * End of "$Id: htmldoc.h,v 1.37 2004/03/31 20:56:56 mike Exp $".
 */
