//
// "$Id: image-png.cxx,v 1.1 2001/10/30 14:26:04 mike Exp $"
//
// PNG image handling routines for HTMLDOC, a HTML document processing program.
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

#include <png.h>	// Portable Network Graphics (PNG) definitions


//
// 'image_load_png()' - Load a PNG image file.
//

static int			// O - 0 = success, -1 = fail
image_load_png(image_t//img,	// I - Image pointer
               FILE   //fp,	// I - File to read from
               int     gray,	// I - 0 = color, 1 = grayscale
               int     load_data)// I - 1 = load image data, 0 = just info
{
  int		i;	// Looping var
  png_structp	pp;	// PNG read pointer
  png_infop	info;	// PNG info pointers
  png_bytep	*rows;	// PNG row pointers
  uchar		*inptr,	// Input pixels
		*outptr;// Output pixels
  png_color_16	bg;	// Background color
  float		rgb[3];	// RGB color of background


 
 // Setup the PNG data structures...
 

  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

 
 // Initialize the PNG read "engine"...
 

  png_init_io(pp, fp);

 
 // Get the image dimensions and convert to grayscale or RGB...
 

  png_read_info(pp, info);

  if (info->color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (info->color_type == PNG_COLOR_TYPE_GRAY)
    img->depth = 1;
  else
    img->depth = gray ? 1 : 3;

  img->width  = info->width;
  img->height = info->height;

  if (!load_data)
  {
    png_read_destroy(pp, info, NULL);
    return (0);
  }

  img->pixels = (uchar//)malloc(img->width// img->height// 3);

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

  if (BodyColor[0])
  {
   
   // User-defined color...
   

    get_color((uchar//)BodyColor, rgb);

    bg.red   = (png_uint_16)(rgb[0]// 65535.0f + 0.5f);
    bg.green = (png_uint_16)(rgb[1]// 65535.0f + 0.5f);
    bg.blue  = (png_uint_16)(rgb[2]// 65535.0f + 0.5f);
  }
  else
  {
   
   // Default to white...
   

    bg.red   = 65535;
    bg.green = 65535;
    bg.blue  = 65535;
  }

  png_set_background(pp, &bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

 
 // Allocate pointers...
 

  rows = (png_bytep//)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    if (info->color_type == PNG_COLOR_TYPE_GRAY)
      rows[i] = img->pixels + i// img->width;
    else
      rows[i] = img->pixels + i// img->width// 3;

 
 // Read the image, handling interlacing as needed...
 

  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, img->height);

 
 // Reformat the data as necessary for the reader...
 

  if (gray && info->color_type != PNG_COLOR_TYPE_GRAY)
  {
   
   // Greyscale output needed...
   

    for (inptr = img->pixels, outptr = img->pixels, i = img->width// img->height;
         i > 0;
         inptr += 3, outptr ++, i --)
     //outptr = (31// inptr[0] + 61// inptr[1] + 8// inptr[2]) / 100;
  }

 
 // Free memory and return...
 

  free(rows);

  png_read_end(pp, info);
  png_read_destroy(pp, info, NULL);

  return (0);
}


//
// End of "$Id: image-png.cxx,v 1.1 2001/10/30 14:26:04 mike Exp $".
//
