/*
 * "$Id: progress.cxx,v 1.12 2004/10/23 07:06:19 mike Exp $"
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   hdBook::progress_debug()  - Display a debug message.
 *   hdBook::progress_error()  - Display an error message.
 *   hdBook::progress_hide()   - Hide the current run status.
 *   hdBook::progress_show()   - Show the current run status.
 *   hdBook::progress_update() - Update the current run status.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <stdarg.h>

#ifdef HAVE_LIBFLTK
#  include <FL/fl_ask.H>
#endif // HAVE_LIBFLTK

#ifdef WIN32
#  define getpid	GetCurrentProcessId
#else
#  include <unistd.h>
#endif // WIN32


/*
 * 'hdBook::progress_debug()' - Display a debug message.
 */

void
hdBook::progress_debug(const char *format,
					// I - Printf-style format string
                       ...)		// I - Additional args as needed
{
  va_list	ap;			// Argument pointer
  char		text[2048];		// Formatted text string


  va_start(ap, format);
  vsnprintf(text, sizeof(text), format, ap);
  va_end(ap);

#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
    return;
#endif /* HAVE_LIBFLTK */

  if (verbosity > 0)
  {
    if (progress_visible)
      fprintf(stderr, "\r%-79.79s\r", "");

    fprintf(stderr, "DEBUG: %s\n", text);
  }
}


/*
 * 'hdBook::progress_error()' - Display an error message.
 */

void
hdBook::progress_error(hdError    error,// I - Error number
                       const char *format,
					// I - Printf-style format string
                       ...)		// I - Additional args as needed
{
  va_list	ap;			// Argument pointer
  char		text[2048];		// Formatted text string


  if (error == HD_ERROR_HTML_ERROR && !strict_html)
    return;

  if (error)
    error_count ++;

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

#ifdef WIN32
  // IIS doesn't separate stderr from stdout, so we cannot output any CGI error messages
  // on Windows...
  if (CGIMode)
    return;
#endif // WIN32

  if (verbosity >= 0)
  {
    if (progress_visible)
      fprintf(stderr, "\r%-79.79s\r", "");

    if (CGIMode)
      fprintf(stderr, "HTMLDOC(%d) ", getpid());

    if (error)
      fprintf(stderr, "ERR%03d: %s\n", error, text);
    else
      fprintf(stderr, "%s\n", text);

    fflush(stderr);
  }
}


/*
 * 'hdBook::progress_hide()' - Hide the current run status.
 */

void
hdBook::progress_hide(void)
{
#ifdef HAVE_LIBFLTK
  if (BookGUI != NULL)
  {
    BookGUI->progress(0, "HTMLDOC " SVERSION " Ready.");
    return;
  }
#endif /* HAVE_LIBFLTK */

  if (CGIMode)
    return;

  if (verbosity > 0)
  {
    fprintf(stderr, "\r%-79.79s\r", "");
    fflush(stderr);
  }

  progress_visible = 0;
}


/*
 * 'hdBook::progress_show()' - Show the current run status.
 */

void
hdBook::progress_show(const char *format,
					/* I - Printf-style format string */
                      ...)		/* I - Additional args as needed */
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

  if (CGIMode)
  {
    if (verbosity > 0)
    {
      fprintf(stderr, "HTMLDOC(%d) INFO: %s\n", getpid(), text);
      fflush(stderr);
    }

    return;
  }

  if (verbosity > 0)
  {
    fprintf(stderr, "\r%-79.79s", text);
    fflush(stderr);
  }

  progress_visible = 1;
}


/*
 * 'hdBook::progress_update()' - Update the current run status.
 */

void
hdBook::progress_update(int percent)	/* I - Percent complete */
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
 * End of "$Id: progress.cxx,v 1.12 2004/10/23 07:06:19 mike Exp $".
 */
