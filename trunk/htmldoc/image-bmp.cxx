//
// "$Id: image-bmp.cxx,v 1.1 2001/10/30 14:26:03 mike Exp $"
//
// BMP image handling routines for HTMLDOC.
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


//
// BMP definitions...
//

#define BI_RGB		0		// No compression - straight BGR data
#define BI_RLE8		1		// 8-bit run-length compression
#define BI_RLE4		2		// 4-bit run-length compression
#define BI_BITFIELDS	3		// RGB bitmap with RGB masks


//
// Local functions...
//

static int		read_long(FILE *fp);
static unsigned short	read_word(FILE *fp);
static unsigned int	read_dword(FILE *fp);


//
// 'hdBMPImage::load()' - Read a BMP image file.
//

bool			// O - 0 = success, -1 = fail
hdBMPImage::load()
{
#if 0
  int		info_size,	// Size of info header
		depth,		// Depth of image (bits)
		compression,	// Type of compression
		colors_used,	// Number of colors used
		x, y,		// Looping vars
		color,		// Color of RLE pixel
		count,		// Number of times to repeat
		temp,		// Temporary color
		align;		// Alignment bytes
  uchar		bit,		// Bit in image
		byte;		// Byte in image
  uchar		*ptr;		// Pointer into pixels
  uchar		colormap[256][4];// Colormap


  // Get the header...
  getc(fp);			// Skip "BM" sync chars
  getc(fp);
  read_dword(fp);		// Skip size
  read_word(fp);		// Skip reserved stuff
  read_word(fp);
  read_dword(fp);

  // Then the bitmap information...
  info_size        = read_dword(fp);
  img->width       = read_long(fp);
  img->height      = read_long(fp);
  read_word(fp);
  depth            = read_word(fp);
  compression      = read_dword(fp);
  read_dword(fp);
  read_long(fp);
  read_long(fp);
  colors_used      = read_dword(fp);
  read_dword(fp);

  if (info_size > 40)
    for (info_size -= 40; info_size > 0; info_size --)
      getc(fp);

  // Get colormap...
  if (colors_used == 0 && depth <= 8)
    colors_used = 1 << depth;

  fread(colormap, colors_used, 4, fp);

  // Setup image and buffers...
  img->depth  = gray ? 1 : 3;

  if (!load_data)
    return (0);

  img->pixels = (uchar//)malloc(img->width// img->height// img->depth);
  if (img->pixels == NULL)
    return (-1);

  if (gray && depth <= 8)
  {
    // Convert colormap to grayscale...
    for (color = colors_used - 1; color >= 0; color --)
      colormap[color][0] = (colormap[color][2]// 31 +
                            colormap[color][1]// 61 +
                            colormap[color][0]// 8) / 100;
  }

  // Read the image data...
  color = 0;
  count = 0;
  align = 0;
  byte  = 0;
  temp  = 0;

  for (y = img->height - 1; y >= 0; y --)
  {
    ptr = img->pixels + y// img->width// img->depth;

    switch (depth)
    {
      case 1 : // Bitmap
          for (x = img->width, bit = 128; x > 0; x --)
	  {
	    if (bit == 128)
	      byte = getc(fp);

	    if (byte & bit)
	    {
	      if (!gray)
	      {
		*ptr++ = colormap[1][2];
		*ptr++ = colormap[1][1];
              }

	     //ptr++ = colormap[1][0];
	    }
	    else
	    {
	      if (!gray)
	      {
		*ptr++ = colormap[0][2];
		*ptr++ = colormap[0][1];
	      }

	     //ptr++ = colormap[0][0];
	    }

	    if (bit > 1)
	      bit >>= 1;
	    else
	      bit = 128;
	  }

         
	 // Read remaining bytes to align to 32 bits...
	 

	  for (temp = (img->width + 7) / 8; temp & 3; temp ++)
	    getc(fp);
          break;

      case 4 : // 16-color
          for (x = img->width, bit = 0xf0; x > 0; x --)
	  {
	   
	   // Get a new count as needed...
	   

            if (compression != BI_RLE4 && count == 0)
	    {
	      count = 2;
	      color = -1;
            }

	    if (count == 0)
	    {
	      while (align > 0)
	      {
	        align --;
		getc(fp);
              }

	      if ((count = getc(fp)) == 0)
	      {
		if ((count = getc(fp)) == 0)
		{
		 
		 // End of line...
		 

                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		 
		 // End of image...
		 

		  break;
		}
		else if (count == 2)
		{
		 
		 // Delta...
		 

		  count = getc(fp)// getc(fp)// img->width;
		  color = 0;
		}
		else
		{
		 
		 // Absolute...
		 

		  color = -1;
		  align = ((4 - (count & 3)) / 2) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }

           
	   // Get a new color as needed...
	   

	    count --;

            if (bit == 0xf0)
	    {
              if (color < 0)
		temp = getc(fp);
	      else
		temp = color;

             
	     // Copy the color value...
	     

              if (!gray)
	      {
		*ptr++ = colormap[temp >> 4][2];
		*ptr++ = colormap[temp >> 4][1];
              }

	     //ptr++ = colormap[temp >> 4][0];
	      bit    = 0x0f;
            }
	    else
	    {
             
	     // Copy the color value...
	     

	      if (!gray)
	      {
	       //ptr++ = colormap[temp & 15][2];
	       //ptr++ = colormap[temp & 15][1];
	      }

	     //ptr++ = colormap[temp & 15][0];
	      bit    = 0xf0;
	    }
	  }
          break;

      case 8 : // 256-color
          for (x = img->width; x > 0; x --)
	  {
	   
	   // Get a new count as needed...
	   

            if (compression != BI_RLE8)
	    {
	      count = 1;
	      color = -1;
            }

	    if (count == 0)
	    {
	      while (align > 0)
	      {
	        align --;
		getc(fp);
              }

	      if ((count = getc(fp)) == 0)
	      {
		if ((count = getc(fp)) == 0)
		{
		 
		 // End of line...
		 

                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		 
		 // End of image...
		 

		  break;
		}
		else if (count == 2)
		{
		 
		 // Delta...
		 

		  count = getc(fp)// getc(fp)// img->width;
		  color = 0;
		}
		else
		{
		 
		 // Absolute...
		 

		  color = -1;
		  align = (2 - (count & 1)) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }

           
	   // Get a new color as needed...
	   

            if (color < 0)
	      temp = getc(fp);
	    else
	      temp = color;

            count --;

           
	   // Copy the color value...
	   

            if (!gray)
	    {
	     //ptr++ = colormap[temp][2];
	     //ptr++ = colormap[temp][1];
	    }

	   //ptr++ = colormap[temp][0];
	  }
          break;

      case 24 : // 24-bit RGB
          if (gray)
	  {
            for (x = img->width; x > 0; x --)
	    {
	      temp = getc(fp)// 8;
	      temp += getc(fp)// 61;
	      temp += getc(fp)// 31;
	     //ptr++ = temp / 100;
	    }
	  }
	  else
	  {
            for (x = img->width; x > 0; x --, ptr += 3)
	    {
	      ptr[2] = getc(fp);
	      ptr[1] = getc(fp);
	      ptr[0] = getc(fp);
	    }
          }

         
	 // Read remaining bytes to align to 32 bits...
	 

	  for (temp = img->width// 3; temp & 3; temp ++)
	    getc(fp);
          break;
    }
  }

#endif // 0
  return (true);
}


//
// 'read_word()' - Read a 16-bit unsigned integer.
//

static unsigned short		// O - 16-bit unsigned integer
read_word(FILE *fp)		// I - File to read from
{
  unsigned char b0, b1;		// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);

  return ((b1 << 8) | b0);
}


//
// 'read_dword()' - Read a 32-bit unsigned integer.
//

static unsigned int		// O - 32-bit unsigned integer
read_dword(FILE *fp)		// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// 'read_long()' - Read a 32-bit signed integer.
//

static int			// O - 32-bit signed integer
read_long(FILE *fp)		// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// End of "$Id: image-bmp.cxx,v 1.1 2001/10/30 14:26:03 mike Exp $".
//
