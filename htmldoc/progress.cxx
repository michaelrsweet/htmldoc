/*
 * "$Id: progress.cxx,v 1.6.2.14 2004/02/06 03:51:09 mike Exp $"
 *
 *   Progress functions for HTMLDOC, a HTML document processing program.
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
 * Local globals...
 */

static int	progress_visible = 0;


/*
 * 'progress_error()' - Display an error message.
 */

void
progress_error(HDerror    error,	/* I - Error number */
               const char *format,	/* I - Printf-style format string */
               ...)			/* I - Additional args as needed */
{
  va_list	ap;			/* Argument pointer */
  char		text[2048];		/* Formatted text string */


  if (error == HD_ERROR_HTML_ERROR && !StrictHTML)
    return;

  if (error)
    Errors ++;

  va_start(ap, format);
  vsnprintf(text, sizeof(text), format, ap);
  va_end(ap);

#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    if (error)
      BookGUI->add_error(text);

    return;
  }
#endif /* HAVE_LIBFLTK */

  if (Verbosity >= 0)
  {
    if (progress_visible)
      fprintf(stderr, "\r%-79.79s\r", "");

    if (error)
      fprintf(stderr, "ERR%03d: %s\n", error, text);
    else
      fprintf(stderr, "%s\n", text);
  }
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
    fprintf(stderr, "\r%-79.79s\r", "");
    fflush(stderr);
  }

  progress_visible = 0;
}


/*
 * 'progress_show()' - Show the current run status.
 */

void
progress_show(const char *format,	/* I - Printf-style format string */
              ...)			/* I - Additional args as needed */
{
  va_list	ap;			/* Argument pointer */
  static char	text[2048];		/* Formatted text string */


  va_start(ap, format);
  vsnprintf(text, sizeof(text), format, ap);
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
    fprintf(stderr, "\r%-79.79s", text);
    fflush(stderr);
  }

  progress_visible = 1;
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
 * End of "$Id: progress.cxx,v 1.6.2.14 2004/02/06 03:51:09 mike Exp $".
 */
