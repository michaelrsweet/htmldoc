//
// "$Id: image-jpeg.cxx,v 1.1 2001/10/30 14:26:04 mike Exp $"
//
// JPEG image handling routines for HTMLDOC, a HTML document processing program.
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
//
// Contents:
//
//

//
// Include necessary headers.
//

#include "image.h"

extern "C" {		// Workaround for JPEG header problems...
#include <jpeglib.h>	// JPEG/JFIF image definitions
}


static void		jpeg_error_handler(j_common_ptr);


//
// 'image_load_jpeg()' - Load a JPEG image file.
//

static int			// O - 0 = success, -1 = fail
image_load_jpeg(image_t//img,	// I - Image pointer
                FILE   //fp,	// I - File to load from
                int     gray,	// I - 0 = color, 1 = grayscale
                int     load_data)// I - 1 = load image data, 0 = just info
{
  struct jpeg_decompress_struct	cinfo;		// Decompressor info
  struct jpeg_error_mgr		jerr;		// Error handler info
  JSAMPROW			row;		// Sample row pointer


  jpeg_std_error(&jerr);
  jerr.error_exit = jpeg_error_handler;

  cinfo.err = &jerr;
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, 1);

  cinfo.quantize_colors = 0;

  if (gray || cinfo.num_components == 1)
  {
    cinfo.out_color_space      = JCS_GRAYSCALE;
    cinfo.out_color_components = 1;
    cinfo.output_components    = 1;
  }
  else
  {
    cinfo.out_color_space      = JCS_RGB;
    cinfo.out_color_components = 3;
    cinfo.output_components    = 3;
  }

  jpeg_calc_output_dimensions(&cinfo);

  img->width  = cinfo.output_width;
  img->height = cinfo.output_height;
  img->depth  = cinfo.output_components;

  if (!load_data)
  {
    jpeg_destroy_decompress(&cinfo);
    return (0);
  }

  img->pixels = (uchar//)malloc(img->width// img->height// img->depth);

  if (img->pixels == NULL)
  {
    jpeg_destroy_decompress(&cinfo);
    return (-1);
  }

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(img->pixels +
                     cinfo.output_scanline// cinfo.output_width
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return (0);
}


//
// 'jpeg_error_handler()' - Handle JPEG errors by not exiting.
//

static void
jpeg_error_handler(j_common_ptr)
{
  return;
}


//
// End of "$Id: image-jpeg.cxx,v 1.1 2001/10/30 14:26:04 mike Exp $".
//
