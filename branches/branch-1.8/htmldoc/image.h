/*
 * "$Id: image.h,v 1.5.2.8 2003/01/06 22:09:33 mike Exp $"
 *
 *   Image management definitions for HTMLDOC, a HTML document processing
 *   program.
 *
 *   Copyright 1997-2003 by Easy Software Products.
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

#ifndef _IMAGE_H_
#  define _IMAGE_H_

/*
 * Include necessary headers.
 */

#  include <stdio.h>
#  include <stdlib.h>
#  include "hdstring.h"

#  include "types.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Image structure...
 */

typedef struct			/**** Image structure ****/
{
  char		filename[1024];	/* Name of image file (for caching of images */
  int		width,		/* Width of image in pixels */
		height,		/* Height of image in pixels */
		depth,		/* 1 for grayscale, 3 for RGB */
		use,		/* Number of times this image was used */
		obj;		/* Object number */
  uchar		*pixels;	/* 8-bit pixel data */
  uchar		*mask;		/* 1-bit mask data, if any */
  int		maskwidth,	/* Byte width of mask data */
		maskscale;	/* Scaling of mask data */
} image_t;


/*
 * Prototypes...
 */

extern void	image_copy(const char *filename, const char *destpath);
extern image_t	*image_find(const char *filename, int load_data = 0);
extern void	image_flush_cache(void);
extern int	image_getlist(image_t ***ptrs);
extern image_t	*image_load(const char *filename, int gray, int load_data = 0);
extern void	image_unload(image_t *img);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_IMAGE_H_ */

/*
 * End of "$Id: image.h,v 1.5.2.8 2003/01/06 22:09:33 mike Exp $".
 */
