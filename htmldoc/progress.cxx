/*
 * "$Id: progress.cxx,v 1.6.2.4 2001/08/16 21:11:52 mike Exp $"
 *
 *   Progress functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2001 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   progress_error()  - Display an error message.
 *   progress_hide()   - Hide the current run status.
 *   progress_show()   - Show the current run status.
 *   progress_update() - Update the current run status.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <stdarg.h>

#ifdef HAVE_LIBFLTK
#  include <FL/fl_ask.H>
#endif // HAVE_LIBFLTK


/*
 * 'progress_error()' - Display an error message.
 */

void
progress_error(HDerror error,	/* I - Error number */
               char    *format,	/* I - Printf-style format string */
               ...)		/* I - Additional args as needed */
{
  va_list	ap;		/* Argument pointer */
  char		text[2048];	/* Formatted text string */


  Errors ++;

  va_start(ap, format);
  vsprintf(text, format, ap);
  va_end(ap);

#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->add_error(text);
    return;
  }
#endif /* HAVE_LIBFLTK */

  if (Verbosity >= 0)
    fprintf(stderr, "\rERR%03d: %-71.71s\n", error, text);
}


/*
 * 'progress_hide()' - Hide the current run status.
 */

void
progress_hide(void)
{
#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(0, "HTMLDOC " SVERSION " Ready.");
    return;
  }
#endif /* HAVE_LIBFLTK */

  if (Verbosity > 0)
  {
    fprintf(stderr, "\r%-79s\r", "");
    fflush(stderr);
  }
}


/*
 * 'progress_show()' - Show the current run status.
 */

void
progress_show(char *format,	/* I - Printf-style format string */
              ...)		/* I - Additional args as needed */
{
  va_list	ap;		/* Argument pointer */
  static char	text[2048];	/* Formatted text string */


  va_start(ap, format);
  vsprintf(text, format, ap);
  va_end(ap);

#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(0, text);
    return;
  }
#endif /* HAVE_LIBFLTK */

  if (Verbosity > 0)
  {
    fprintf(stderr, "\r%-79s", text);
    fflush(stderr);
  }
}


/*
 * 'progress_update()' - Update the current run status.
 */

void
progress_update(int percent)	/* I - Percent complete */
{
#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(percent);
    return;
  }
#endif /* HAVE_LIBFLTK */
}


/*
 * End of "$Id: progress.cxx,v 1.6.2.4 2001/08/16 21:11:52 mike Exp $".
 */
