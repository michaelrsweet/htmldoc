//
// "$Id: image-gif.cxx,v 1.9 2004/02/03 02:55:28 mike Exp $"
//
// Image handling routines for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2004 by Easy Software Products.
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
//   hdGIFImage::hdGIFImage() - Create a new GIF image.
//   hdGIFImage::get_block()  - Read a GIF data block...
//   hdGIFImage::get_code()   - Get a LZW code from the file...
//   hdGIFImage::load()       - Load the image data...
//   hdGIFImage::read_cmap()  - Read the colormap from a GIF file...
//   hdGIFImage::read_image() - Read a GIF image stream...
//   hdGIFImage::read_lzw()   - Read a byte from the LZW stream...
//   hdGIFImage::real_load()  - Load a GIF image file...
//

//
// Include necessary headers.
//

#include "image.h"
#include "hdstring.h"


//
// GIF definitions...
//

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80


//
// 'hdGIFImage::hdGIFImage()' - Create a new GIF image.
//

hdGIFImage::hdGIFImage(const char *p,	// I - URI of image
                       int        gs)	// I - 0 = color, 1 = grayscale
{
  uri(p);

  real_load(0, gs);
}


//
// 'hdGIFImage::check()' - Try to load an image file as a GIF.
//

hdImage *
hdGIFImage::check(const char *p,	// I - URI for image file
                  int        gs,	// I - 1 = grayscale, 0 = color
		  const char *header)	// I - First 16 bytes of file
{
  if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0)
    return (new hdGIFImage(p, gs));
  else
    return (NULL);
}


//
// 'hdGIFImage::get_block()' - Read a GIF data block...
//

int					// O - Number characters read
hdGIFImage::get_block(hdFile *fp,	// I - File to read from
		      hdByte  *buf)	// I - Input buffer
{
  int	count;				// Number of character to read


  // Read the count byte followed by the data from the file...
  if ((count = fp->get()) == EOF)
  {
    eof = 1;
    return (-1);
  }
  else if (count == 0)
    eof = 1;
  else if (fp->read(buf, count) < count)
  {
//    progress_error(HD_ERROR_READ_ERROR,
//                   "Unable to read GIF block of %d bytes: %s", count,
//                   strerror(errno));
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
hdGIFImage::get_code(hdFile *fp,	// I - File to read from
	             int    code_size,	// I - Size of code in bits
	             int    first_time)	// I - 1 = first time, 0 = not first time
{
  unsigned		i, j,		// Looping vars
			ret;		// Return value
  int			count;		// Number of bytes read
  static hdByte		buf[280];	// Input buffer
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
//      progress_error(HD_ERROR_READ_ERROR,
//                     "Not enough data left to read GIF compression code.");
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
    if ((count = get_block(fp, buf + last_byte)) <= 0)
    {
      // Whoops, no more data!
      done = 1;
      return (-1);
    }

    // Update buffer state...
    curbit    = (curbit - lastbit) + 8 * last_byte;
    last_byte += count;
    lastbit   = last_byte * 8;
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
// 'hdGIFImage::load()' - Load the image data...
//

int					// O - 0 on success, -1 on error
hdGIFImage::load()
{
  return (real_load(1, depth() == 1));
}


//
// 'hdGIFImage::read_cmap()' - Read the colormap from a GIF file...
//

int					// O  - 0 on success, -1 on error
hdGIFImage::read_cmap(hdFile *fp,	// I  - File to read from
  		      int    ncolors,	// I  - Number of colors
		      cmap_t cmap,	// IO - Colormap array
		      int    *gray)	// IO - 1 = grayscale
{
  int	i;				// Looping var


  // Read the colormap...
  if (fp->read(cmap, 3 * ncolors) < (3 * ncolors))
  {
//    progress_error(HD_ERROR_READ_ERROR,
//                   "Unable to read GIF colormap: %s", strerror(errno));
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
// 'hdGIFImage::read_image()' - Read a GIF image stream...
//

int					// I - 0 = success, -1 = failure
hdGIFImage::read_image(hdFile *fp,	// I - Input file
		       cmap_t cmap,	// I - Colormap
		       int    interlace,// I - Non-zero = interlaced image
		       int    transparent)// I - Transparent color
{
  hdByte		code_size,		// Code size
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
  code_size = fp->get();

  if (read_lzw(fp, 1, code_size) < 0)
    return (-1);

  temp = pixels();

  while ((pixel = read_lzw(fp, 0, code_size)) >= 0)
  {
    temp[0] = cmap[pixel][0];

    if (depth() > 1)
    {
      temp[1] = cmap[pixel][1];
      temp[2] = cmap[pixel][2];
    }

    if (pixel == transparent)
      set_mask(xpos, ypos);

    xpos ++;
    temp += depth();
    if (xpos == width())
    {
      xpos = 0;

      if (interlace)
      {
        ypos += xpasses[pass];
        temp += (xpasses[pass] - 1) * width() * depth();

        if (ypos >= height())
	{
	  pass ++;

          ypos = ypasses[pass];
          temp = pixels() + ypos * width() * depth();
	}
      }
      else
	ypos ++;
    }

    if (ypos >= height())
      break;
  }

  return (0);
}


//
// 'hdGIFImage::read_lzw()' - Read a byte from the LZW stream...
//

int					// O - Byte from stream
hdGIFImage::read_lzw(hdFile *fp,	// I - File to read from
		     int    first_time,	// I - 1 = first time, 0 = not first time
 		     int    input_code_size)// I - Code size in bits
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
    max_code_size = 2 * clear_code;
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
      max_code_size = 2 * clear_code;
      max_code      = clear_code + 2;

      sp = stack;

      firstcode = oldcode = get_code(fp, code_size, 0);

      return (firstcode);
    }
    else if (code == end_code)
    {
      hdByte	buf[260];


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
// 'hdGIFImage::real_load()' - Load a GIF image file...
//

int				// O - 0 = success, -1 = fail
hdGIFImage::real_load(int img,	// I - 1 = load image data, 0 = just info
                      int gs)	// I - 0 = color, 1 = grayscale
{
  hdFile	*fp;		// File to load from
  hdByte	buf[1024];	// Input buffer
  cmap_t	cmap;		// Colormap
  int		ncolors,	// Bits per pixel
		transparent;	// Transparent color index
  int		w, h;		// Width and height of image
  int		status;		// Status of load...


  // Open the image file...
  if ((fp = hdFile::open(uri(), HD_FILE_READ)) == NULL)
    return (-1);

  // Read the header; we already know it is a GIF file...
  fp->read(buf, 13);

  w       = (buf[7] << 8) | buf[6];
  h       = (buf[9] << 8) | buf[8];
  ncolors = 2 << (buf[10] & 0x07);

  if (buf[10] & GIF_COLORMAP)
    if (read_cmap(fp, ncolors, cmap, &gs))
    {
      delete fp;
      return (-1);
    }

  transparent = -1;

  while (1)
  {
    switch (fp->get())
    {
      case ';' :	// End of image
          delete fp;
          return (-1);	// Early end of file

      case '!' :	// Extension record
          buf[0] = fp->get();
          if (buf[0] == 0xf9)	// Graphic Control Extension
          {
            get_block(fp, buf);
            if (buf[0] & 1)	// Get transparent color index
              transparent = buf[3];
          }

          while (get_block(fp, buf) != 0);
          break;

      case ',' :	// Image data
          fp->read(buf, 9);

          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);

	    if (read_cmap(fp, ncolors, cmap, &gs))
	    {
	      delete fp;
	      return (-1);
	    }
	  }

          w = (buf[5] << 8) | buf[4];
          h = (buf[7] << 8) | buf[6];

          set_size(w, h, gs ? 1 : 3);

	  if (!img)
	  {
	    delete fp;
	    return (0);
	  }

          alloc_pixels();

          if (transparent >= 0)
          {
	    // Allocate a mask image...
            alloc_mask();
	  }

	  status = read_image(fp, cmap, buf[8] & GIF_INTERLACE, transparent);
	  delete fp;

	  return (status);
    }
  }
}


//
// End of "$Id: image-gif.cxx,v 1.9 2004/02/03 02:55:28 mike Exp $".
//
