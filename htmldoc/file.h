/*
 * "$Id: file.h,v 1.1 1999/11/08 17:12:41 mike Exp $"
 *
 *   Filename definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Easy Software Products.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING.txt".
 */

#ifndef _FILE_H_
#  define _FILE_H_

/*
 * Include necessary headers...
 */

#  include "string.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Prototypes...
 */

extern char	*file_basename(const char *s);
extern char	*file_directory(const char *s);
extern char	*file_extension(const char *s);
extern char	*file_method(const char *s);
extern char	*file_target(const char *s);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_FILE_H_ */

/*
 * End of "$Id: file.h,v 1.1 1999/11/08 17:12:41 mike Exp $".
 */
