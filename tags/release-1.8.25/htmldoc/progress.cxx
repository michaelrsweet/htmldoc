/*
 * "$Id$"
 *
 *   Progress functions for HTMLDOC, a HTML document processing program.
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

#ifdef WIN32
#  define getpid	GetCurrentProcessId
static FILE	*error_log = NULL;
#else
#  include <unistd.h>
#endif // WIN32


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

#ifdef WIN32
  // IIS doesn't separate stderr from stdout, so we have to send CGI errors
  // to a separate event log...
  if (CGIMode)
  {
    if (!error_log)
    {
      // Append messages to a file called "htmldoc.log"...
      char	tmppath[1024];		// Buffer for temp dir
      char	filename[1024];		// htmldoc.log filename


      GetTempPath(sizeof(tmppath), tmppath);
      snprintf(filename, sizeof(filename), "%s/htmldoc.log", tmppath);

      error_log = fopen(filename, "a");
    }

    if (error_log)
    {
      fprintf(error_log, "HTMLDOC(%d) ", (int)getpid());

      if (error)
	fprintf(error_log, "ERR%03d: %s\n", error, text);
      else
	fprintf(error_log, "%s\n", text);

      fflush(error_log);
    }

    return;
  }
#endif // WIN32

  if (Verbosity >= 0)
  {
    if (progress_visible)
      fprintf(stderr, "\r%-79.79s\r", "");

    if (CGIMode)
      fprintf(stderr, "HTMLDOC(%d) ", (int)getpid());

    if (error)
      fprintf(stderr, "ERR%03d: %s\n", error, text);
    else
      fprintf(stderr, "%s\n", text);

    fflush(stderr);
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

  if (CGIMode)
    return;

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

  if (CGIMode)
  {
    if (Verbosity > 0)
    {
      fprintf(stderr, "HTMLDOC(%d) INFO: %s\n", (int)getpid(), text);
      fflush(stderr);
    }

    return;
  }

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
 * End of "$Id$".
 */
