//
// "$Id: image.h,v 1.8 2001/10/30 14:26:04 mike Exp $"
//
// Image management definitions for HTMLDOC, a HTML document processing
// program.
//
// Copyright 1997-2001 by Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: ESP Licensing Information
//     Easy Software Products
//     44141 Airport View Drive, Suite 204
//     Hollywood, Maryland 20636-3111 USA
//
//     Voice: (301) 373-9600
//     EMail: info@easysw.com
//       WWW: http://www.easysw.com
///

#ifndef HTMLDOC_IMAGE_H
#  define HTMLDOC_IMAGE_H


//
// Include necessary headers.
//

#  include "types.h"


//
// Base image class...
//
// The base image class is incomplete - the load() method is not
// implemented.  Instead all child image classes must at a minimum
// implement a constructor and the load() method.
//
// The save() method should be used for exporting image formats for
// HTML output.  The default implementation uses the copy() method
// to copy the image file to the destination directory.  The
// hdEPSImageFile class uses the save_as_png() method to provide
// a web-readable format and modify the destination filename with
// a ".png" extension...
// 

enum hdImageType
{
  RASTER,
  VECTOR
};

class hdImage			//// Image class
{
  private:

  char		*path_;		// Name of image file (for caching of images)
  int		width_,		// Width of image in pixels
		height_,	// Height of image in pixels
		depth_,		// 1 for grayscale, 3 for RGB
		use_,		// Number of times this image was used
		obj_;		// Object number
  hdImageType	type_;		// Type of image
  uchar		*pixels_;	// 8-bit pixel data
  uchar		*mask_;		// 1-bit mask data, if any
  int		maskwidth_;	// Byte width of mask data

  protected:

  void			alloc_mask();
  void			alloc_pixels();
  bool			copy(char *dest, int destlen);
  void			path(const char *p);
  bool			save_as_png(char *dest, int destlen);
  void			set_mask(int x, int y, uchar a = 0);
  void			set_size(int w, int h, int d);

  public:

  ~hdImage();

  int			depth() { return depth_; }
  void			free();
  int			height() { return height_; }
  int			hold() { return ++ use_; }
  virtual bool		load();
  uchar			*mask() { return mask_; }
  int			maskwidth() { return maskwidth_; }
  int			obj() { return obj; }
  void			obj(int o) { obj_ = o; }
  const char		*path() { return path_; }
  uchar			*pixels() { return pixels_; }
  int			release() { return -- use_; }
  virtual bool		save(char *dest, int destlen);
  virtual hdImageType	type();
  int			width() { return width_; }
};


//
// Image classes for various formats...
//

class hdBMPImage public hdImage
{
  public:

  hdBMPImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdEPSImage public hdImage
{
  public:

  hdEPSImage(const char *path, bool grayscale);

  virtual bool		load();
  virtual bool		save(const char *dest);
  virtual hdImageType	type();
}

class hdGIFImage public hdImage
{
  private:

  typedef uchar	cmap_t[256][3];

  int	eof;		// Did we hit EOF?

  int	read_cmap(FILE *fp, int ncolors, cmap_t cmap, int *gray);
  int	get_block(FILE *fp, uchar *buffer);
  int	get_code(FILE *fp, int code_size, int first_time);
  int	read_image(FILE *fp, cmap_t cmap, int interlace, int transparent);
  int	read_lzw(FILE *fp, int first_time, int input_code_size);

  public:

  hdGIFImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdJPEGImage public hdImage
{
  public:

  hdJPEGImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdPNGImage public hdImage
{
  public:

  hdPNGImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdPNMImage public hdImage
{
  public:

  hdPNMImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdXBMImage public hdImage
{
  public:

  hdXBMImage(const char *path, bool grayscale);

  virtual bool	load();
}

class hdXPMImage public hdImage
{
  public:

  hdXPMImage(const char *path, bool grayscale);

  virtual bool	load();
}


#endif // !HTMLDOC_IMAGE_H

//
// End of "$Id: image.h,v 1.8 2001/10/30 14:26:04 mike Exp $".
//
