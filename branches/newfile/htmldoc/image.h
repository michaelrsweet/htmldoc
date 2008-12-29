//
// "$Id$"
//
// Image management definitions for HTMLDOC, a HTML document processing
// program.
//
// Copyright 1997-2008 Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//

#ifndef HTMLDOC_IMAGE_H
#  define HTMLDOC_IMAGE_H


//
// Include necessary headers.
//

#  include "file.h"
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
  HD_IMAGE_RASTER,
  HD_IMAGE_VECTOR
};

class hdImage;

typedef hdImage *(*hdImageCheck)(const char *uri, bool gs, const char *header);

class hdImage			//// Image class
{
  private:

  char		*uri_;		// Name of image file (for caching of images)
  int		width_,		// Width of image in pixels
		height_,	// Height of image in pixels
		depth_,		// 1 for grayscale, 3 for RGB
		use_,		// Number of times this image was used
		obj_;		// Object number
  hdImageType	type_;		// Type of image
  hdByte	*pixels_;	// 8-bit pixel data
  hdByte	*mask_;		// 1-bit mask data, if any (1 = transparent)
  int		maskwidth_;	// Byte width of mask data

  static int	num_formats_,	// Number of registered formats
  		alloc_formats_;	// Allocated formats
  static hdImageCheck *formats_;// Registered formats

  protected:

  void			alloc_mask();
  void			alloc_pixels();
  int			save_as_png(const char *path, char *d, int dlen);
  void			set_mask(int x, int y, hdByte a = 0);
  void			set_size(int w, int h, int d);
  void			uri(const char *p);
  void			type(hdImageType t) { type_ = t; }

  static int		num_images_,	// Number of images in cache
			alloc_images_;	// Allocated images
  static hdImage	**images_;	// Pointers to images

  static int		compare(hdImage **, hdImage **);

  public:

  hdImage();
  virtual ~hdImage();

  int			copy(const char *path, char *d, int dlen);
  int			depth() { return depth_; }
  void			free();
  int			height() { return height_; }
  int			hold() { return ++ use_; }
  virtual int		load();
  hdByte		*mask() { return mask_; }
  int			maskwidth() { return maskwidth_; }
  int			obj() { return obj_; }
  void			obj(int o) { obj_ = o; }
  hdByte		*pixels() { return pixels_; }
  unsigned char		*reduce(int max_colors, int &bytes_per_line,
			        int &num_colors, unsigned *colors);
  int			release() { return -- use_; }
  virtual int		save(const char *path, char *d, int dlen);
  hdImageType		type() { return type_; }
  const char		*uri() { return uri_; }
  int			use() { return use_; }
  int			width() { return width_; }

  // Global methods for caching of image files...
  static hdImage	*find(const char *uri, bool gs = false,
			      const char *path = (char *)0);
  static hdImage	**images() { return images_; }
  static int		num_images() { return num_images_; }
  static void		flush();
  static void		register_standard();
  static void		register_format(hdImageCheck check);
};


//
// Image classes for various formats...
//

class hdBMPImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdBMPImage(const char *uri, bool grayscale);

  virtual int		load();
  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdEPSImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdEPSImage(const char *uri, bool grayscale);

  virtual int		load();
  virtual int		save(char *path, char *d, int dlen);
  virtual hdImageType	type();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdGIFImage : public hdImage
{
  typedef hdByte	cmap_t[256][3];

  int	eof;		// Did we hit EOF?

  int	get_block(hdFile *fp, hdByte *buffer);
  int	get_code(hdFile *fp, int code_size, int first_time);
  int	read_cmap(hdFile *fp, int ncolors, cmap_t cmap, bool *gray);
  int	read_image(hdFile *fp, cmap_t cmap, int interlace, int transparent);
  int	read_lzw(hdFile *fp, int first_time, int input_code_size);
  int	real_load(int img, bool gs);

  public:

  hdGIFImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdJPEGImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdJPEGImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdPNGImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdPNGImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdPNMImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdPNMImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdXBMImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdXBMImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

class hdXPMImage : public hdImage
{
  int		real_load(int img, bool gs);

  public:

  hdXPMImage(const char *uri, bool grayscale);

  virtual int		load();

  static hdImage	*check(const char *uri, bool gs, const char *header);
};

#endif // !HTMLDOC_IMAGE_H

//
// End of "$Id$".
//
