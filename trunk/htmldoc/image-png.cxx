//
// "$Id$"
//
// PNG image handling routines for HTMLDOC, a HTML document processing program.
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
// Contents:
//
//   hdPNGImage::hdPNGImage() - Create a new PNG image.
//   hdPNGImage::check()      - Try to load an image file as a PNG.
//   hdPNGImage::load()       - Load a PNG image...
//   hdPNGImage::real_load()  - Load a PNG image file.
//   png_read()               - Read data from a PNG image file...
//

//
// Include necessary headers.
//

#include "image.h"

#include <png.h>	// Portable Network Graphics (PNG) definitions


//
// Local functions...
//

static void	png_read(png_structp pp, png_bytep data, png_uint_32 length);


//
// 'hdPNGImage::hdPNGImage()' - Create a new PNG image.
//

hdPNGImage::hdPNGImage(const char *p,	// I - URI for image file
                       bool       gs)	// I - 0 for color, 1 for grayscale
{
  uri(p);

  real_load(0, gs);
}


//
// 'hdPNGImage::check()' - Try to load an image file as a PNG.
//

hdImage *
hdPNGImage::check(const char *p,	// I - URI for image file
                  bool       gs,	// I - 1 = grayscale, 0 = color
		  const char *header)	// I - First 16 bytes of file
{
  if (memcmp(header, "\211PNG", 4) == 0)
    return (new hdPNGImage(p, gs));
  else
    return (NULL);
}


//
// 'hdPNGImage::load()' - Load a PNG image...
//

int					// O - 0 on success, -1 on failure
hdPNGImage::load()
{
  return (real_load(1, depth() == 1));
}


//
// 'hdPNGImage::real_load()' - Load a PNG image file.
//

int					// O - 0 = success, -1 = fail
hdPNGImage::real_load(int  img,		// I - 1 = load image data, 0 = just info
                      bool gs)		// I - 0 = color, 1 = grayscale
{
  hdFile	*fp;			// File pointer
  int		i, j;			// Looping vars
  png_structp	pp;			// PNG read pointer
  png_infop	info;			// PNG info pointers
  int		d;			// Depth of image
  png_bytep	*rows;			// PNG row pointers
  png_bytep	local;			// Local image data...
  png_bytep	localptr;		// Pointer to local image data...
  hdByte	*pixelptr;		// Pointer to final image data...


  // Open the file...
  if ((fp = hdFile::open(uri(), HD_FILE_READ)) == NULL)
    return (-1);

  // Setup the PNG data structures...
  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

  // Initialize the PNG read "engine"...
  png_set_read_fn(pp, fp, (png_rw_ptr)png_read);

  // Get the image dimensions and convert to grayscale or RGB...
  png_read_info(pp, info);

  if (info->color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (info->color_type & PNG_COLOR_MASK_COLOR)
    d = 3;
  else
    d = 1;

  set_size(info->width, info->height, gs ? 1 : d);

  if (!img)
  {
    png_destroy_read_struct(&pp, &info, NULL);
    delete fp;
    return (0);
  }

  alloc_pixels();

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
    d ++;

  if (!(d & 1))
    alloc_mask();

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);

  // Allocate a local copy of the image as needed...
  if (depth() != d)
    local = new png_byte[info->width * info->height * d];
  else
    local = (png_bytep)pixels();

  // Allocate memory and setup the pointers for the whole image...
  rows = new png_bytep[info->height];

  for (i = 0; i < (int)info->height; i ++)
    rows[i] = local + i * info->width * d;

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, info->height);

  // Reformat the data as necessary for the reader...
  if (local != pixels())
  {
    if (d == 3)
    {
      // Convert to grayscale...
      for (i = (int)info->height, pixelptr = pixels(), localptr = local;
           i > 0;
	   i --)
	for (j = (int)info->width; j > 0; j --, localptr += 3)
          *pixelptr++ = (31 * localptr[0] + 61 * localptr[1] +
	                 8 * localptr[2]) / 100;
    }
    else
    {
      // Handle transparency and possibly convert to grayscale...
      for (i = 0, pixelptr = pixels(), localptr = local;
           i < (int)info->height;
	   i ++)
        if (d == 2)
	{
	  for (j = 0; j < (int)info->width; j ++, localptr += 2)
	  {
            *pixelptr++ = localptr[0];
	    set_mask(j, i, localptr[1]);
	  }
        }
	else if (gs)
	{
	  for (j = 0; j < (int)info->width; j ++, localptr += 4)
	  {
            *pixelptr++ = (31 * localptr[0] + 61 * localptr[1] +
	                   8 * localptr[2]) / 100;
	    set_mask(j, i, localptr[3]);
	  }
	}
	else
	{
	  for (j = 0; j < (int)info->width; j ++)
	  {
            *pixelptr++ = *localptr++;
            *pixelptr++ = *localptr++;
            *pixelptr++ = *localptr++;
	    set_mask(j, i, *localptr++);
	  }
	}
    }
  }

  // Free memory and return...
  delete[] rows;

  if (local != (png_bytep)pixels())
    delete[] local;

  png_read_end(pp, info);
  png_destroy_read_struct(&pp, &info, NULL);

  delete fp;

  return (0);
}


//
// 'png_read()' - Read data from a PNG image file...
//

static void
png_read(png_structp pp,		// I - PNG image
         png_bytep   data,		// I - Data buffer
	 png_uint_32 length)		// I - Number of bytes to read
{
  hdFile	*fp;			// File pointer


  fp = (hdFile *)png_get_io_ptr(pp);

  if (fp->read(data, length) != (int)length)
    png_error(pp, "Read Error");
}


//
// End of "$Id$".
//
