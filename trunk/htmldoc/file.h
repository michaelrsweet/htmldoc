/*
 * "$Id: file.h,v 1.7 2001/09/27 22:33:22 mike Exp $"
 *
 *   Filename definitions for HTMLDOC, a HTML document processing program.
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
 */

#ifndef _HD_FILE_H_
#  define _HD_FILE_H_

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

extern char	*hd_file_basename(const char *s);
extern void	hd_file_cleanup(void);
extern char	*hd_file_directory(const char *s);
extern char	*hd_file_extension(const char *s);
extern char	*hd_file_find(const char *path, const char *s);
extern char	*hd_file_localize(const char *filename, const char *newcwd);
extern char	*hd_file_method(const char *s);
extern void	hd_file_proxy(const char *url);
extern char	*hd_file_target(const char *s);
extern FILE	*hd_file_temp(char *name, int len);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HD_FILE_H_ */

/*
 * End of "$Id: file.h,v 1.7 2001/09/27 22:33:22 mike Exp $".
 */
