//
// "$Id: image.h,v 1.6 2000/10/16 03:25:08 mike Exp $"
//
//   Image management definitions for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 1997-2000 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//

#ifndef _HD_IMAGE_H_
#  define _HD_IMAGE_H_

//
// Include necessary headers.
//

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>

#  include "types.h"


//
// Image class...
//

class HDimage
{
  typedef uchar	gif_cmap_t[256][3];

  static int	compare(HDimage **img1, HDimage **img2);

  int		gif_eof;		// Did we hit EOF?

  int		gif_read_cmap(FILE *fp, int ncolors, gif_cmap_t cmap,
		              int *gray);
  int		gif_get_block(FILE *fp, uchar *buffer);
  int		gif_get_code (FILE *fp, int code_size, int first_time);
  int		gif_read_lzw(FILE *fp, int first_time, int input_code_size);
  int		gif_read_image(FILE *fp, gif_cmap_t cmap,
		               int interlace);
  int		load_bmp(FILE *fp, int gray);
  int		load_gif(FILE *fp, int gray);
  int		load_jpeg(FILE *fp, int gray);
  int		load_png(FILE *fp, int gray);
  unsigned short read_word(FILE *fp);
  unsigned int	read_dword(FILE *fp);
  int		read_long(FILE *fp);

  public:

  static int	num_images;	// Number of images
  static int	alloc_images;	// Allocated images
  static HDimage **images;	// Images

  char		filename[1024],	// Name of image file (for caching of images)
		realname[1024];	// Real name of image file
  int		width,		// Width of image in pixels
		height,		// Height of image in pixels
		depth,		// 1 for grayscale, 3 for RGB
		use,		// Number of times this image was used
		id;		// ID number for image
  uchar		*pixels;	// 8-bit pixel data

  HDimage() { pixels = (uchar *)0; }
  HDimage(const char *f, int gray);
  ~HDimage();

  void copy(const char *destpath);

  static HDimage *find(const char *f, int gray);
  static void	flush();
};

#endif // !_HD_IMAGE_H_

//
// End of "$Id: image.h,v 1.6 2000/10/16 03:25:08 mike Exp $".
//
