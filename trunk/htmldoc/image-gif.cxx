//
// "$Id: image-gif.cxx,v 1.2 2001/11/29 01:57:38 mike Exp $"
//
// Image handling routines for HTMLDOC, a HTML document processing program.
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
// GIF definitions...
//

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80


//
// 'hdGIFImage::read_cmap()' - Read the colormap from a GIF file...
//

int					// O  - 0 on success, -1 on error
hdGIFImage::read_cmap(FILE   *fp,	// I  - File to read from
  		      int    ncolors,	// I  - Number of colors
		      cmap_t cmap,	// IO - Colormap array
		      int    *gray)	// IO - 1 = grayscale
{
  int	i;				// Looping var


  // Read the colormap...
  if (fread(cmap, 3, ncolors, fp) < (size_t)ncolors)
  {
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read GIF colormap: %s", strerror(errno));
    return (-1);
  }

  // Check to see if the colormap is a grayscale ramp...
  for (i = 0; i < ncolors; i ++)
    if (cmap[i][0] != cmap[i][1] || cmap[i][1] != cmap[i][2])
      break;

  if (i == ncolors)
  {
    *gray = 1;
    return (0);
  }

  // If this needs to be a grayscale image, convert the RGB values to
  // luminance values...
  if (*gray)
  {
    for (i = 0; i < ncolors; i ++)
      cmap[i][0] = (cmap[i][0] * 31 + cmap[i][1] * 61 + cmap[i][2] * 8) / 100;
  }

  return (0);
}


//
// 'hdGIFImage::get_block()' - Read a GIF data block...
//

int					// O - Number characters read
hdGIFImage::get_block(FILE  *fp,	// I - File to read from
		      uchar *buf)	// I - Input buffer
{
  int	count;				// Number of character to read


  // Read the count byte followed by the data from the file...
  if ((count = getc(fp)) == EOF)
  {
    eof = 1;
    return (-1);
  }
  else if (count == 0)
    eof = 1;
  else if (fread(buf, 1, count, fp) < (size_t)count)
  {
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read GIF block of %d bytes: %s", count,
                   strerror(errno));
    eof = 1;
    return (-1);
  }
  else
    eof = 0;

  return (count);
}


//
// 'hdGIFImage::get_code()' - Get a LZW code from the file...
//

int					// O - LZW code
hdGIFImage::get_code(FILE *fp,		// I - File to read from
	             int  code_size,	// I - Size of code in bits
	             int  first_time)	// I - 1 = first time, 0 = not first time
{
  unsigned		i, j,		// Looping vars
			ret;		// Return value
  int			count;		// Number of bytes read
  static uchar		buf[280];	// Input buffer
  static unsigned	curbit,		// Current bit
			lastbit,	// Last bit in buffer
			done,		// Done with this buffer?
			last_byte;	// Last byte in buffer
  static unsigned	bits[8] =	// Bit masks for codes
			{
			  0x01, 0x02, 0x04, 0x08,
			  0x10, 0x20, 0x40, 0x80
			};


  if (first_time)
  {
    // Just initialize the input buffer...
    curbit  = 0;
    lastbit = 0;
    done    = 0;

    return (0);
  }

  if ((curbit + code_size) >= lastbit)
  {
    // Don't have enough bits to hold the code...
    if (done)
    {
      progress_error(HD_ERROR_READ_ERROR,
                     "Not enough data left to read GIF compression code.");
      return (-1);	// Sorry, no more...
    }

    // Move last two bytes to front of buffer...
    if (last_byte > 1)
    {
      buf[0]    = buf[last_byte - 2];
      buf[1]    = buf[last_byte - 1];
      last_byte = 2;
    }
    else if (last_byte == 1)
    {
      buf[0]    = buf[last_byte - 1];
      last_byte = 1;
    }

    // Read in another buffer...
    if ((count = get_block (fp, buf + last_byte)) <= 0)
    {
      // Whoops, no more data!
      done = 1;
      return (-1);
    }

    // Update buffer state...
    curbit    = (curbit - lastbit) + 8// last_byte;
    last_byte += count;
    lastbit   = last_byte// 8;
  }

  ret = 0;
  for (ret = 0, i = curbit + code_size - 1, j = code_size;
       j > 0;
       i --, j --)
    ret = (ret << 1) | ((buf[i / 8] & bits[i & 7]) != 0);

  curbit += code_size;

  return (ret);
}


//
// 'hdGIFImage::read_image()' - Read a GIF image stream...
//

int						// I - 0 = success, -1 = failure
hdGIFImage::read_image(FILE   *fp,		// I - Input file
		       cmap_t cmap,		// I - Colormap
		       int    interlace,	// I - Non-zero = interlaced image
		       int    transparent)	// I - Transparent color
{
  uchar		code_size,		// Code size
		*temp;			// Current pixel
  int		xpos,			// Current X position
		ypos,			// Current Y position
		pass;			// Current pass
  int		pixel;			// Current pixel
  static int	xpasses[4] = { 8, 8, 4, 2 },
		ypasses[5] = { 0, 4, 2, 1, 999999 };


  xpos      = 0;
  ypos      = 0;
  pass      = 0;
  code_size = getc(fp);

  if (read_lzw(fp, 1, code_size) < 0)
    return (-1);

  temp = img->pixels;

  while ((pixel = read_lzw(fp, 0, code_size)) >= 0)
  {
    temp[0] = cmap[pixel][0];

    if (img->depth > 1)
    {
      temp[1] = cmap[pixel][1];
      temp[2] = cmap[pixel][2];
    }

    if (pixel == transparent)
      image_set_mask(img, xpos, ypos);

    xpos ++;
    temp += img->depth;
    if (xpos == img->width)
    {
      xpos = 0;

      if (interlace)
      {
        ypos += xpasses[pass];
        temp += (xpasses[pass] - 1)// img->width// img->depth;

        if (ypos >= img->height)
	{
	  pass ++;

          ypos = ypasses[pass];
          temp = img->pixels + ypos// img->width// img->depth;
	}
      }
      else
	ypos ++;
    }

    if (ypos >= img->height)
      break;
  }

  return (0);
}


//
// 'hdGIFImage::read_lzw()' - Read a byte from the LZW stream...
//

int						// O - Byte from stream
hdGIFImage::read_lzw(FILE *fp,			// I - File to read from
		     int  first_time,		// I - 1 = first time, 0 = not first time
 		     int  input_code_size)	// I - Code size in bits
{
  int		i,			// Looping var
		code,			// Current code
		incode;			// Input code
  static short	fresh = 0,		// 1 = empty buffers
		code_size,		// Current code size
		set_code_size,		// Initial code size set
		max_code,		// Maximum code used
		max_code_size,		// Maximum code size
		firstcode,		// First code read
		oldcode,		// Last code read
		clear_code,		// Clear code for LZW input
		end_code,		// End code for LZW input
		table[2][4096],		// String table
		stack[8192],		// Output stack
		*sp;			// Current stack pointer


  if (first_time)
  {
    // Setup LZW state...
    set_code_size = input_code_size;
    code_size     = set_code_size + 1;
    clear_code    = 1 << set_code_size;
    end_code      = clear_code + 1;
    max_code_size = 2// clear_code;
    max_code      = clear_code + 2;

    // Initialize input buffers...
    get_code(fp, 0, 1);

    // Wipe the decompressor table...
    fresh = 1;

    for (i = 0; i < clear_code; i ++)
    {
      table[0][i] = 0;
      table[1][i] = i;
    }

    for (; i < 4096; i ++)
      table[0][i] = table[1][0] = 0;

    sp = stack;

    return (0);
  }
  else if (fresh)
  {
    fresh = 0;

    do
      firstcode = oldcode = get_code(fp, code_size, 0);
    while (firstcode == clear_code);

    return (firstcode);
  }

  if (sp > stack)
    return (*--sp);

  while ((code = get_code (fp, code_size, 0)) >= 0)
  {
    if (code == clear_code)
    {
      for (i = 0; i < clear_code; i ++)
      {
	table[0][i] = 0;
	table[1][i] = i;
      }

      for (; i < 4096; i ++)
	table[0][i] = table[1][i] = 0;

      code_size     = set_code_size + 1;
      max_code_size = 2// clear_code;
      max_code      = clear_code + 2;

      sp = stack;

      firstcode = oldcode = get_code(fp, code_size, 0);

      return (firstcode);
    }
    else if (code == end_code)
    {
      uchar	buf[260];


      if (!eof)
        while (get_block(fp, buf) > 0);

      return (-2);
    }

    incode = code;

    if (code >= max_code)
    {
      *sp++ = firstcode;
      code  = oldcode;
    }

    while (code >= clear_code)
    {
      *sp++ = table[1][code];
      if (code == table[0][code])
	return (255);

      code = table[0][code];
    }

    *sp++ = firstcode = table[1][code];
    code  = max_code;

    if (code < 4096)
    {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      max_code ++;

      if (max_code >= max_code_size && max_code_size < 4096)
      {
	max_code_size *= 2;
	code_size ++;
      }
    }

    oldcode = incode;

    if (sp > stack)
      return (*--sp);
  }

  return (code);
}


//
// 'hdGIFImage::load()' - Load a GIF image file...
//

int				// O - 0 = success, -1 = fail
hdGIFImage::load(FILE *fp,	// I - File to load from
                 int  gray,	// I - 0 = color, 1 = grayscale
                 int  load_data)// I - 1 = load image data, 0 = just info
{
  uchar		buf[1024];	// Input buffer
  cmap_t	cmap;		// Colormap
  int		ncolors,	// Bits per pixel
		transparent;	// Transparent color index


  // Read the header; we already know it is a GIF file...
  fread(buf, 13, 1, fp);

  img->width  = (buf[7] << 8) | buf[6];
  img->height = (buf[9] << 8) | buf[8];
  ncolors     = 2 << (buf[10] & 0x07);

  if (buf[10] & GIF_COLORMAP)
    if (read_cmap(fp, ncolors, cmap, &gray))
      return (-1);

  transparent = -1;

  while (1)
  {
    switch (getc(fp))
    {
      case ';' :	// End of image
          return (-1);		// Early end of file

      case '!' :	// Extension record
          buf[0] = getc(fp);
          if (buf[0] == 0xf9)	// Graphic Control Extension
          {
            get_block(fp, buf);
            if (buf[0] & 1)	// Get transparent color index
              transparent = buf[3];
          }

          while (get_block(fp, buf) != 0);
          break;

      case ',' :	// Image data
          fread(buf, 9, 1, fp);

          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);

	    if (read_cmap(fp, ncolors, cmap, &gray))
	      return (-1);
	  }

          if (transparent >= 0)
          {
            // Map transparent color to background color...
            if (BodyColor[0])
	    {
	      float rgb[3]; // RGB color


	      get_color((uchar *)BodyColor, rgb);

	      cmap[transparent][0] = (uchar)(rgb[0] * 255.0f + 0.5f);
	      cmap[transparent][1] = (uchar)(rgb[1] * 255.0f + 0.5f);
	      cmap[transparent][2] = (uchar)(rgb[2] * 255.0f + 0.5f);
	    }
	    else
	    {
	      cmap[transparent][0] = 255;
              cmap[transparent][1] = 255;
              cmap[transparent][2] = 255;
	    }

	    // Allocate a mask image...
            alloc_mask();
	  }

          img->width  = (buf[5] << 8) | buf[4];
          img->height = (buf[7] << 8) | buf[6];
          img->depth  = gray ? 1 : 3;
	  if (!load_data)
	    return (0);

          img->pixels = (uchar *)malloc(img->width * img->height * img->depth);
          if (img->pixels == NULL)
            return (-1);

	  return (read_image(fp, img, cmap, buf[8] & GIF_INTERLACE, transparent));
    }
  }
}


//
// End of "$Id: image-gif.cxx,v 1.2 2001/11/29 01:57:38 mike Exp $".
//
