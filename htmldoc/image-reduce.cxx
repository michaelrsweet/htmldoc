//
// "$Id$"
//
// Image color reduction methods for HTMLDOC, a HTML document processing
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
// Contents:
//
//   compare_rgb()     - Compare two RGB colors...
//   hdImage::reduce() - Attempt to reduce the number of colors used
//                       by an image...
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include "hdstring.h"


//
// 'compare_rgb()' - Compare two RGB colors...
//

static int			// O - -1 if rgb1<rgb2, etc.
compare_rgb(unsigned *rgb1,	// I - First color
            unsigned *rgb2)	// I - Second color
{
  return (*rgb1 - *rgb2);
}


//
// 'hdImage::reduce()' - Attempt to reduce the number of colors used by an image...
//

unsigned char *					// O - Image data pointer
hdImage::reduce(int          max_colors,	// I - Maximum number of colors to use
		int          &bytes_per_line,	// O - Bytes per line
		int          &num_colors,	// O - Number of colors
		unsigned     *colors)		// O - Colormap
{
  int		i, j, k, m;			// Looping vars
  unsigned char	*pixel,				// Current pixel
		*data,				// New indexed pixel array
		*ptr;				// Current index
  int		bits;				// Bits per index
  unsigned	key,				// Color key
		*match;				// Matching color value
  hdByte	grays[256];			// Grayscale usage


  // Load the image as needed...
  if (!pixels_)
    load();

  // See if we can optimize the image as indexed without color loss...
  num_colors  = 0;
  data        = NULL;

  if (depth_ == 1)
  {
    // Greyscale image...
    memset(grays, 0, sizeof(grays));

    // Scan the image, stopping if we have more than 16 grayshades...
    for (i = width_ * height_, pixel = pixels_;
	 i > 0;
	 i --, pixel ++)
      if (!grays[*pixel])
      {
        if (num_colors >= 16)
	  break;

	grays[*pixel] = 1;
	num_colors ++;
      }

    if (i == 0)
    {
      // Have <= 16 shades, map them to the colormap...
      for (i = 0, j = 0; i < 256; i ++)
	if (grays[i])
	{
	  colors[j] = (((i << 8) | i) << 8) | i;
	  grays[i]  = j;
	  j ++;
	}
    }
    else
    {
      // Too many shades of gray; return NULL...
      num_colors = 0;

      return (NULL);
    }
  }
  else
  {
    // Color image...
    for (i = width_ * height_, pixel = pixels_, match = NULL;
	 i > 0;
	 i --, pixel += 3)
    {
      // Grab the current color...
      key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

      // See if this is the same as the last color...
      if (!match || *match != key)
      {
        // Lookup the color...
        if (num_colors > 0)
          match = (unsigned *)bsearch(&key, colors, num_colors, sizeof(unsigned),
                                      (hdCompareFunc)compare_rgb);
        else
          match = NULL;
      }

      if (match == NULL)
      {
        // New color; make sure we don't have too many...
        if (num_colors >= max_colors)
          break;

        // Add the color to the colormap...
        colors[num_colors] = key;
        num_colors ++;

        if (num_colors > 1)
          qsort(colors, num_colors, sizeof(unsigned),
                (hdCompareFunc)compare_rgb);
      }
    }

    if (i > 0)
    {
      // Too many colors; return NULL...
      num_colors = 0;

      return (NULL);
    }
  }

  // Figure out how many bits are needed...
  if (num_colors <= 2)
    bits = 1;
  else if (num_colors <= 4)
    bits = 2;
  else if (num_colors <= 16)
    bits = 4;
  else
    bits = 8;

  // Allocate memory...
  bytes_per_line = (width_ * bits + 7) / 8;
  data           = new unsigned char[bytes_per_line * height_];

  if (depth_ == 1)
  {
    // Convert a grayscale image...
    switch (bits)
    {
      case 1 :
	  for (i = height_, pixel = pixels_, ptr = data;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 7; j > 0; j --, k = (k + 7) & 7, pixel ++)
	      switch (k)
	      {
		case 7 :
	            *ptr = grays[*pixel] << 7;
		    break;
		default :
	            *ptr |= grays[*pixel] << k;
		    break;
		case 0 :
	            *ptr++ |= grays[*pixel];
		    break;
              }

	    if (k != 7)
	      ptr ++;
	  }
	  break;

      case 2 :
	  for (i = height_, pixel = pixels_, ptr = data;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 0; j > 0; j --, k = (k + 1) & 3, pixel ++)
	      switch (k)
	      {
		case 0 :
	            *ptr = grays[*pixel] << 6;
		    break;
		case 1 :
	            *ptr |= grays[*pixel] << 4;
		    break;
		case 2 :
	            *ptr |= grays[*pixel] << 2;
		    break;
		case 3 :
	            *ptr++ |= grays[*pixel];
		    break;
              }

	    if (k)
	      ptr ++;
	  }
	  break;

      case 4 :
	  for (i = height_, pixel = pixels_, ptr = data;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 0; j > 0; j --, k ^= 1, pixel ++)
	      if (k)
		*ptr++ |= grays[*pixel];
	      else
		*ptr = grays[*pixel] << 4;

	    if (k)
	      ptr ++;
	  }
	  break;
    }
  }
  else
  {
    // Convert a color image...
    switch (bits)
    {
      case 1 :
	  for (i = height_, pixel = pixels_, ptr = data,
	           match = colors;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 7;
	         j > 0;
		 j --, k = (k + 7) & 7, pixel += 3)
	    {
              key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      if (*match != key)
        	match = (unsigned *)bsearch(&key, colors, num_colors,
		                            sizeof(unsigned),
                            	            (hdCompareFunc)compare_rgb);
	      m = match - colors;

	      switch (k)
	      {
		case 7 :
	            *ptr = m << 7;
		    break;
		default :
	            *ptr |= m << k;
		    break;
		case 0 :
	            *ptr++ |= m;
		    break;
              }
	    }

	    if (k != 7)
	      ptr ++;
	  }
	  break;

      case 2 :
	  for (i = height_, pixel = pixels_, ptr = data,
	           match = colors;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 0;
	         j > 0;
		 j --, k = (k + 1) & 3, pixel += 3)
	    {
              key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      if (*match != key)
        	match = (unsigned *)bsearch(&key, colors, num_colors,
		                            sizeof(unsigned),
                            	            (hdCompareFunc)compare_rgb);
	      m = match - colors;

	      switch (k)
	      {
		case 0 :
	            *ptr = m << 6;
		    break;
		case 1 :
	            *ptr |= m << 4;
		    break;
		case 2 :
	            *ptr |= m << 2;
		    break;
		case 3 :
	            *ptr++ |= m;
		    break;
              }
	    }

	    if (k)
	      ptr ++;
	  }
	  break;

      case 4 :
	  for (i = height_, pixel = pixels_, ptr = data,
	           match = colors;
	       i > 0;
	       i --)
	  {
	    for (j = width_, k = 0; j > 0; j --, k ^= 1, pixel += 3)
	    {
              key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      if (*match != key)
        	match = (unsigned *)bsearch(&key, colors, num_colors,
		                            sizeof(unsigned),
                            	            (hdCompareFunc)compare_rgb);
	      m = match - colors;

	      if (k)
		*ptr++ |= m;
	      else
		*ptr = m << 4;
	    }

	    if (k)
	      ptr ++;
	  }
	  break;

      case 8 :
	  for (i = height_, pixel = pixels_, ptr = data,
	           match = colors;
	       i > 0;
	       i --)
	  {
	    for (j = width_; j > 0; j --, pixel += 3, ptr ++)
	    {
              key = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      if (*match != key)
        	match = (unsigned *)bsearch(&key, colors, num_colors,
		                            sizeof(unsigned),
                            	            (hdCompareFunc)compare_rgb);
	      *ptr = match - colors;
	    }
	  }
	  break;
    }
  }

  if (num_colors == 1)
  {
    // Adobe doesn't like 1 color images...
    num_colors = 2;
    colors[1]  = 0;
  }

  return (data);
}


//
// End of "$Id$".
//
