/*
 * "$Id: image.h,v 1.1 1999/11/07 13:33:10 mike Exp $"
 *
 *   Image management definitions for HTMLDOC, a HTML document processing
 *   program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 */

#ifndef _IMAGE_H_
#  define _IMAGE_H_

/*
 * Include necessary headers.
 */

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>

#  include "types.h"


/*
 * Image structure...
 */

typedef struct			/**** Image structure ****/
{
  char		filename[1024];	/* Name of image file (for caching of images */
  int		width,		/* Width of image in pixels */
		height,		/* Height of image in pixels */
		depth;		/* 1 for grayscale, 3 for RGB */
  uchar		*pixels;	/* 8-bit pixel data */
} image_t;

/*
 * Prototypes...
 */

extern image_t	*image_load(char *filename, int gray);
extern void	image_flush_cache(void);
extern void	image_copy(char *filename, char *destpath);

#endif /* !_IMAGE_H_ */

/*
 * End of "$Id: image.h,v 1.1 1999/11/07 13:33:10 mike Exp $".
 */
