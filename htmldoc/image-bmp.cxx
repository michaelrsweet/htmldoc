//
// "$Id$"
//
//   BMP image handling routines for HTMLDOC.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdBMPImage::hdBMPImage() - Create a new BMP image.
//   hdBMPImage::load()       - Read a BMP image file.
//   hdBMPImage::read_load()  - Really read a BMP image file...
//   read_word()              - Read a 16-bit unsigned integer.
//   read_dword()             - Read a 32-bit unsigned integer.
//   read_long()              - Read a 32-bit signed integer.
//

//
// Include necessary headers.
//

#include "image.h"
#include "hdstring.h"


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

static int		read_long(hdFile *fp);
static unsigned short	read_word(hdFile *fp);
static unsigned int	read_dword(hdFile *fp);


//
// 'hdBMPImage::hdBMPImage()' - Create a new BMP image.
//

hdBMPImage::hdBMPImage(const char *p,	// I - URI for image
                       bool       gs)	// I - Load as grayscale?
{
  uri(p);

  real_load(0, gs);
}


//
// 'hdBMPImage::check()' - Try to load an image file as a BMP.
//

hdImage *
hdBMPImage::check(const char *p,	// I - URI for image file
                  bool       gs,	// I - 1 = grayscale, 0 = color
		  const char *header)	// I - First 16 bytes of file
{
  if (!memcmp(header, "BM", 2))		// BMP header
    return (new hdBMPImage(p, gs));
  else
    return (NULL);
}


//
// 'hdBMPImage::load()' - Read a BMP image file.
//

int					// O - 0 = success, -1 = fail
hdBMPImage::load()
{
  return (real_load(1, depth() == 1));
}


//
// 'hdBMPImage::read_load()' - Really read a BMP image file...
//

int				// O - 0 = success, -1 = fail
hdBMPImage::real_load(int  img,	// I - Load image data?
                      bool gs)	// I - Load as grayscale?
{
  hdFile	*fp;		// File pointer
  int		info_size,	// Size of info header
		w, h, d,	// Size of image (pixels/bits)
		compression,	// Type of compression
		colors_used,	// Number of colors used
		x, y,		// Looping vars
		color,		// Color of RLE pixel
		count,		// Number of times to repeat
		temp,		// Temporary color
		align;		// Alignment bytes
  hdByte	bit,		// Bit in image
		byte;		// Byte in image
  hdByte	*ptr;		// Pointer into pixels
  hdByte	colormap[256][4];// Colormap


  // Open the file if possible...
  if ((fp = hdFile::open(uri(), HD_FILE_READ)) == NULL)
    return (-1);

  // Get the header...
  fp->get();			// Skip "BM" sync chars
  fp->get();
  read_dword(fp);		// Skip size
  read_word(fp);		// Skip reserved stuff
  read_word(fp);
  read_dword(fp);

  // Then the bitmap information...
  info_size   = read_dword(fp);
  w           = read_long(fp);
  h           = read_long(fp);
  /* skip */    read_word(fp);
  d           = read_word(fp);
  compression = read_dword(fp);
  /* skip */    read_dword(fp);
  /* skip */    read_long(fp);
  /* skip */    read_long(fp);
  colors_used = read_dword(fp);
  /* skip */    read_dword(fp);

  if (info_size > 40)
    for (info_size -= 40; info_size > 0; info_size --)
      fp->get();

  // Get colormap...
  if (colors_used == 0 && d <= 8)
    colors_used = 1 << d;

  fp->read(colormap, colors_used * 4);

  // Setup image and buffers...
  set_size(w, h, gs ? 1 : 3);

  if (!img)
  {
    delete fp;
    return (0);
  }

  alloc_pixels();

  if (gs && d <= 8)
  {
    // Convert colormap to grayscale...
    for (color = colors_used - 1; color >= 0; color --)
      colormap[color][0] = (colormap[color][2] * 31 +
                            colormap[color][1] * 61 +
                            colormap[color][0] * 8) / 100;
  }

  // Read the image data...
  color = 0;
  count = 0;
  align = 0;
  byte  = 0;
  temp  = 0;

  for (y = height() - 1; y >= 0; y --)
  {
    ptr = pixels() + y * width() * depth();

    switch (d)
    {
      case 1 : // Bitmap
          for (x = width(), bit = 128; x > 0; x --)
	  {
	    if (bit == 128)
	      byte = fp->get();

	    if (byte & bit)
	    {
	      if (!gs)
	      {
		*ptr++ = colormap[1][2];
		*ptr++ = colormap[1][1];
              }

	      *ptr++ = colormap[1][0];
	    }
	    else
	    {
	      if (!gs)
	      {
		*ptr++ = colormap[0][2];
		*ptr++ = colormap[0][1];
	      }

	      *ptr++ = colormap[0][0];
	    }

	    if (bit > 1)
	      bit >>= 1;
	    else
	      bit = 128;
	  }

          // Read remaining bytes to align to 32 bits...
	  for (temp = (width() + 7) / 8; temp & 3; temp ++)
	    fp->get();
          break;

      case 4 : // 16-color
          for (x = width(), bit = 0xf0; x > 0; x --)
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
		fp->get();
              }

	      if ((count = fp->get()) == 0)
	      {
		if ((count = fp->get()) == 0)
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
		  count = fp->get() * fp->get() * width();
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
	        color = fp->get();
            }

	    // Get a new color as needed...
	    count --;

            if (bit == 0xf0)
	    {
              if (color < 0)
		temp = fp->get();
	      else
		temp = color;

	      // Copy the color value...
              if (!gs)
	      {
		*ptr++ = colormap[temp >> 4][2];
		*ptr++ = colormap[temp >> 4][1];
              }

	      *ptr++ = colormap[temp >> 4][0];
	      bit    = 0x0f;
            }
	    else
	    {
	      // Copy the color value...
	      if (!gs)
	      {
	        *ptr++ = colormap[temp & 15][2];
	        *ptr++ = colormap[temp & 15][1];
	      }

	      *ptr++ = colormap[temp & 15][0];
	      bit    = 0xf0;
	    }
	  }

	  if (!compression) {
            // Read remaining bytes to align to 32 bits...
	    for (temp = (width() + 1) / 2; temp & 3; temp ++) {
	      fp->get();
	    }
	  }
          break;

      case 8 : // 256-color
          for (x = width(); x > 0; x --)
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
		fp->get();
              }

	      if ((count = fp->get()) == 0)
	      {
		if ((count = fp->get()) == 0)
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
		  count = fp->get() * fp->get() * width();
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
	        color = fp->get();
            }

	    // Get a new color as needed...
            if (color < 0)
	      temp = fp->get();
	    else
	      temp = color;

            count --;

	    // Copy the color value...
            if (!gs)
	    {
	      *ptr++ = colormap[temp][2];
	      *ptr++ = colormap[temp][1];
	    }

	    *ptr++ = colormap[temp][0];
	  }

	  if (!compression) {
            // Read remaining bytes to align to 32 bits...
	    for (temp = width(); temp & 3; temp ++) {
	      fp->get();
	    }
	  }
          break;

      case 24 : // 24-bit RGB
          if (gs)
	  {
            for (x = width(); x > 0; x --)
	    {
	      temp = fp->get() * 8;
	      temp += fp->get() * 61;
	      temp += fp->get() * 31;
	      *ptr++ = temp / 100;
	    }
	  }
	  else
	  {
            for (x = width(); x > 0; x --, ptr += 3)
	    {
	      ptr[2] = fp->get();
	      ptr[1] = fp->get();
	      ptr[0] = fp->get();
	    }
          }

	  // Read remaining bytes to align to 32 bits...
	  for (temp = width() * 3; temp & 3; temp ++)
	    fp->get();
          break;
    }
  }

  // Close the file and return...
  delete fp;

  return (0);
}


//
// 'read_word()' - Read a 16-bit unsigned integer.
//

static unsigned short		// O - 16-bit unsigned integer
read_word(hdFile *fp)		// I - File to read from
{
  unsigned char b0, b1;		// Bytes from file


  b0 = fp->get();
  b1 = fp->get();

  return ((b1 << 8) | b0);
}


//
// 'read_dword()' - Read a 32-bit unsigned integer.
//

static unsigned int		// O - 32-bit unsigned integer
read_dword(hdFile *fp)		// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = fp->get();
  b1 = fp->get();
  b2 = fp->get();
  b3 = fp->get();

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// 'read_long()' - Read a 32-bit signed integer.
//

static int			// O - 32-bit signed integer
read_long(hdFile *fp)		// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = fp->get();
  b1 = fp->get();
  b2 = fp->get();
  b3 = fp->get();

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// End of "$Id$".
//
