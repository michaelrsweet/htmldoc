/*
 * "$Id: progress.h,v 1.1.2.1 2001/02/02 15:10:59 mike Exp $"
 *
 *   Progress function definitions for HTMLDOC, a HTML document
 *   processing program.
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

#ifndef _PROGRESS_H_
#  define _PROGRESS_H_

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Prototypes...
 */

extern void	progress_error(char *format, ...);
extern void	progress_hide(void);
extern void	progress_show(char *format, ...);
extern void	progress_update(int percent);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_PROGRESS_H_ */

/*
 * End of "$Id: progress.h,v 1.1.2.1 2001/02/02 15:10:59 mike Exp $".
 */
