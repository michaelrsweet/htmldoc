/*
 * "$Id: progress.h,v 1.4 2004/03/31 06:32:26 mike Exp $"
 *
 *   Progress function definitions for HTMLDOC, a HTML document
 *   processing program.
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
 */

#ifndef _PROGRESS_H_
#  define _PROGRESS_H_

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Error codes (in addition to the HTTP status codes...)
 */

typedef enum
{
  HD_ERROR_NONE = 0,
  HD_ERROR_NO_FILES,
  HD_ERROR_NO_PAGES,
  HD_ERROR_TOO_MANY_CHAPTERS,
  HD_ERROR_OUT_OF_MEMORY,
  HD_ERROR_FILE_NOT_FOUND,
  HD_ERROR_BAD_COMMENT,
  HD_ERROR_BAD_FORMAT,
  HD_ERROR_DELETE_ERROR,
  HD_ERROR_INTERNAL_ERROR,
  HD_ERROR_NETWORK_ERROR,
  HD_ERROR_READ_ERROR,
  HD_ERROR_WRITE_ERROR,
  HD_ERROR_HTML_ERROR,
  HD_ERROR_CONTENT_TOO_LARGE,
  HD_ERROR_UNRESOLVED_LINK,
  HD_ERROR_BAD_HF_STRING,
  HD_ERROR_HTTPBASE = 100
} HDerror;


/*
 * Prototypes...
 */

extern void	progress_error(HDerror error, const char *format, ...);
extern void	progress_hide(void);
extern void	progress_show(const char *format, ...);
extern void	progress_update(int percent);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_PROGRESS_H_ */

/*
 * End of "$Id: progress.h,v 1.4 2004/03/31 06:32:26 mike Exp $".
 */
