/*
 * "$Id: progress.h,v 1.2 2000/10/16 03:25:08 mike Exp $"
 *
 *   Progress class definitions for HTMLDOC, a HTML document
 *   processing program.
 *
 *   Copyright 1997-2000 by Easy Software Products.
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
 *         WWW: http: *www.easysw.com
 */

#ifndef _HD_PROGRESS_H_
#  define _HD_PROGRESS_H_

/*
 * Include necessary headers...
 */

#  include <stdarg.h>


#  ifdef __cplusplus
/*
 * Progress reporting class...
 */

class HDprogress
{
  public:

  virtual void	error(char *s);
  virtual void	hide(void);
  virtual void	show(char *s);
  virtual void	update(int percent);
};

extern "C" {
#  endif /* __cplusplus */

void	progress_error(char *format, ...);
void	progress_hide(void);
void	progress_show(char *format, ...);
void	progress_update(int percent);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HD_PROGRESS_H_ */

/*
 * End of "$Id: progress.h,v 1.2 2000/10/16 03:25:08 mike Exp $".
 */
