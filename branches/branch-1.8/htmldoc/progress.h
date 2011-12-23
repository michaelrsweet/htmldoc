/*
 * "$Id$"
 *
 *   Progress function definitions for HTMLDOC, a HTML document
 *   processing program.
 *
 *   Copyright 2011 by Michael R Sweet.
 *   Copyright 1997-2010 by Easy Software Products.  All rights reserved.
 *
 *   This program is free software.  Distribution and use rights are outlined in
 *   the file "COPYING.txt".
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
 * End of "$Id$".
 */
