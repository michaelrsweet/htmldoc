//
// "$Id: image.cxx,v 1.12 2000/10/16 03:25:08 mike Exp $"
//
//   Image handling routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//

//
// Include necessary headers.
//

#include "htmldoc.h"

#include <png.h>	// Portable Network Graphics (PNG) definitions


//
// GIF definitions...
//

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80

//
// BMP definitions...
//

#  define BI_RGB       0             // No compression - straight BGR data
#  define BI_RLE8      1             // 8-bit run-length compression
#  define BI_RLE4      2             // 4-bit run-length compression
#  define BI_BITFIELDS 3             // RGB bitmap with RGB masks


//
// Static class data...
//

int	HDimage::num_images = 0;
int	HDimage::alloc_images = 0;
HDimage	**HDimage::images = NULL;


//
// 'HDimage::HDimage()' - Load an image file from disk...
//

HDimage::HDimage(const char *f,		// I - Name of image file
                 int        gray)	// I - 0 = color, 1 = grayscale
{
  FILE		*fp;			// File pointer
  uchar		header[16];		// First 16 bytes of file
  int		status;			// Status of load...
  const char	*rn;			// Real filename
  HDimage	**temp;			// Temporary pointer


  // Initialize data...
  filename[0] = '\0';
  realname[0] = '\0';
  width       = 0;
  height      = 0;
  depth       = 0;
  use         = 1;
  id          = 0;
  pixels      = NULL;

  // Range check...
  if (!f || !f[0])
    return;

  // Figure out the file type...
  if ((rn = file_find(HTMLDOC::path, f)) == NULL)
  {
    HTMLDOC::progress->error("Unable to find image file \"%s\"!", f);
    return;
  }

  if ((fp = fopen(rn, "rb")) == NULL)
  {
    HTMLDOC::progress->error("Unable to read image file \"%s\"!", f);
    return;
  }

  if (fread(header, 1, sizeof(header), fp) == 0)
  {
    HTMLDOC::progress->error("Unable to read image file \"%s\"!", f);
    fclose(fp);
    return;
  }

  rewind(fp);

  // Allocate memory...
  strncpy(filename, f, sizeof(filename) - 1);
  filename[sizeof(filename) - 1] = '\0';

  strncpy(realname, rn, sizeof(realname) - 1);
  realname[sizeof(realname) - 1] = '\0';

  // Load the image as appropriate...
  if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0)
    status = load_gif(fp, gray);
  else if (memcmp(header, "BM", 2) == 0)
    status = load_bmp(fp, gray);
  else if (memcmp(header, "\211PNG", 4) == 0)
    status = load_png(fp, gray);
  else if (memcmp(header, "\377\330\377", 3) == 0 &&	// Start-of-Image
	   header[3] >= 0xe0 && header[3] <= 0xef)	// APPn
    status = load_jpeg(fp, gray);
  else
  {
    HTMLDOC::progress->error("Unknown image file format for \"%s\"!", filename);
    fclose(fp);
    return;
  }

  fclose(fp);

  if (status)
  {
    HTMLDOC::progress->error("Unable to load image file \"%s\"!", filename);
    return;
  }

  // Add the image to the images array...
  if (num_images >= alloc_images)
  {
    alloc_images += 32;

    if (alloc_images == 32)
      temp = (HDimage **)malloc(sizeof(HDimage *) * alloc_images);
    else
      temp = (HDimage **)realloc(images, sizeof(HDimage *) * alloc_images);

    if (!temp)
      return;

    images = temp;
  }

  images[num_images] = this;
  num_images ++;

  if (num_images > 1)
    qsort(images, num_images, sizeof(HDimage *),
          (int (*)(const void *, const void *))compare);
}


//
// 'HDimage::~HDimage()' - Delete an image file...
//

HDimage::~HDimage()
{
  int	i;		// Looping var...


  // Free the memory used for the image...
  if (pixels)
    free(pixels);

  // Find the image in the array...
  num_images --;

  for (i = 0; i < num_images; i ++)
    if (images[i] == this)
      break;

  // Remove it from the list as needed...
  if (i < num_images)
    memcpy(images + i, images + i + 1, sizeof(HDimage *) * (num_images - i));
}


//
// 'HDimage::find()' - Find an image file, loading it if necessary...
//

HDimage *			// O - Image pointer
HDimage::find(const char *f,	// I - Name of image file
              int        gray)	// I - 0 = color, 1 = grayscale
{
  HDimage	*img,		// New image
		key,		// Search key...
		*keyptr,	// Pointer to search key...
		**match;	// Matching image


  // Range check...
  if (!f || !f[0])
    return (NULL);

  // See if we've already loaded it...
  if (num_images > 0)
  {
    strncpy(key.filename, f, sizeof(key.filename));
    key.filename[sizeof(key.filename) - 1] = '\0';

    keyptr = &key;

    match = (HDimage **)bsearch(&keyptr, images, num_images, sizeof(HDimage *),
                                (int (*)(const void *, const void *))compare);
    if (match)
    {
      (*match)->use ++;
      return (*match);
    }
  }

  if ((img = new HDimage(f, gray)) != NULL)
    if (!img->pixels)
    {
      delete img;
      return (NULL);
    }

 return (img);
}


//
// 'HDimage::flush()' - Flush the image cache...
//

void
HDimage::flush()
{
  // Free the memory used by each image...
  while (num_images > 0)
    delete images[0];
}


//
// 'HDimage::copy()' - Copy image files to the destination directory...
//

void
HDimage::copy(const char *destpath)// I - Destination path
{
  char	dest[1024];		// Destination file
  FILE	*in, *out;		// Input/output files
  uchar	buffer[8192];		// Data buffer
  int	nbytes;			// Number of bytes in buffer


  // Figure out the destination filename...
  if (strcmp(destpath, ".") == 0)
    strcpy(dest, file_basename(filename));
  else
    snprintf(dest, sizeof(dest), "%s/%s", destpath, file_basename(filename));

  if (strcmp(dest, filename) == 0)
    return;

  // Open files and copy...
  if ((in = fopen(realname, "rb")) == NULL)
    return;

  if ((out = fopen(dest, "wb")) == NULL)
  {
    fclose(in);
    return;
  }

  while ((nbytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
    fwrite(buffer, 1, nbytes, out);

  fclose(in);
  fclose(out);
}


//
// 'HDimage::compare()' - Compare two image filenames...
//

int					// O - Result of comparison
HDimage::compare(HDimage **img1,	// I - First image
                 HDimage **img2)	// I - Second image
{
#if defined(WIN32) || defined(__EMX__)
  return (strcasecmp((*img1)->filename, (*img2)->filename));
#else
  return (strcmp((*img1)->filename, (*img2)->filename));
#endif // WIN32 || __EMX__
}


//
// 'HDimage::load_jpeg()' - Load a JPEG image file.
//

int				// O - 0 = success, -1 = fail
HDimage::load_jpeg(FILE *fp,	// I - File to load from
                   int  gray)	// I - 0 = color, 1 = grayscale
{
  struct jpeg_decompress_struct	cinfo;		// Decompressor info
  struct jpeg_error_mgr		jerr;		// Error handler info
  JSAMPROW			row;		// Sample row pointer


  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, 1);

  cinfo.quantize_colors = 0;

  if (gray)
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

  width  = cinfo.output_width;
  height = cinfo.output_height;
  depth  = cinfo.output_components;
  pixels = (uchar *)malloc(width * height * depth);

  if (pixels == NULL)
  {
    jpeg_destroy_decompress(&cinfo);
    return (-1);
  }

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(pixels +
                     cinfo.output_scanline * cinfo.output_width *
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return (0);
}


//
// 'HDimage::load_png()' - Load a PNG image file.
//

int				// O - 0 = success, -1 = fail
HDimage::load_png(FILE *fp,	// I - File to read from
                  int  gray)	// I - 0 = color, 1 = grayscale
{
  int		i;		// Looping var
  png_structp	pp;		// PNG read pointer
  png_infop	info;		// PNG info pointers
  png_bytep	*rows;		// PNG row pointers
  uchar		*inptr,		// Input pixels
		*outptr;	// Output pixels
  png_color_16	bg;		// Background color
  float		rgb[3];		// RGB color of background


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
    depth = 1;
  else
    depth = gray ? 1 : 3;

  width  = info->width;
  height = info->height;
  pixels = (uchar *)malloc(width * height * 3);

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);

    if (info->valid & PNG_INFO_sBIT)
      png_set_shift(pp, &(info->sig_bit));
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);

  if (HDtree::body_color[0])
  {
    // User-defined color...
    HTMLDOC::get_color((uchar *)HDtree::body_color, rgb);

    bg.red   = (png_uint_16)(rgb[0] * 65535.0f + 0.5f);
    bg.green = (png_uint_16)(rgb[1] * 65535.0f + 0.5f);
    bg.blue  = (png_uint_16)(rgb[2] * 65535.0f + 0.5f);
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
  rows = (png_bytep *)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    if (info->color_type == PNG_COLOR_TYPE_GRAY)
      rows[i] = pixels + i * width;
    else
      rows[i] = pixels + i * width * 3;

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, height);

  // Reformat the data as necessary for the reader...
  if (gray && info->color_type != PNG_COLOR_TYPE_GRAY)
  {
    // Greyscale output needed...
    for (inptr = pixels, outptr = pixels, i = width * height;
         i > 0;
         inptr += 3, outptr ++, i --)
      *outptr = (31 * inptr[0] + 61 * inptr[1] + 8 * inptr[2]) / 100;

    // Return unused memory...
    pixels = (uchar *)realloc(pixels, width * height);
  }

  // Free memory and return...
  free(rows);

  png_read_end(pp, info);
  png_read_destroy(pp, info, NULL);

  return (0);
}


//
// 'HDimage::load_gif()' - Load a GIF image file...
//

int				// O - 0 = success, -1 = fail
HDimage::load_gif(FILE *fp,	// I - File to load from
                  int  gray)	// I - 0 = color, 1 = grayscale
{
  uchar		buf[1024];	// Input buffer
  gif_cmap_t	cmap;		// Colormap
  int		ncolors,	// Bits per pixel
		transparent;	// Transparent color index


  // Read the header; we already know it is a GIF file...
  fread(buf, 13, 1, fp);

  width  = (buf[7] << 8) | buf[6];
  height = (buf[9] << 8) | buf[8];
  ncolors     = 2 << (buf[10] & 0x07);

  if (buf[10] & GIF_COLORMAP)
    if (gif_read_cmap(fp, ncolors, cmap, &gray))
      return (-1);

  transparent = -1;

  while (1)
  {
    switch (getc(fp))
    {
      case ';' :		// End of image
          return (-1);		// Early end of file

      case '!' :		// Extension record
          buf[0] = getc(fp);
          if (buf[0] == 0xf9)	// Graphic Control Extension
          {
            gif_get_block(fp, buf);
            if (buf[0] & 1)	// Get transparent color index
              transparent = buf[3];
          }

          while (gif_get_block(fp, buf) != 0);
          break;

      case ',' :		// Image data
          fread(buf, 9, 1, fp);

          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);

	    if (gif_read_cmap(fp, ncolors, cmap, &gray))
	      return (-1);
	  }

          if (transparent >= 0)
          {
            // Map transparent color to body color...
            if (HDtree::body_color[0])
	    {
	      float rgb[3];	// RGB color


	      HTMLDOC::get_color((uchar *)HDtree::body_color, rgb);

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
          }

          width  = (buf[5] << 8) | buf[4];
          height = (buf[7] << 8) | buf[6];
          depth  = gray ? 1 : 3;
          if ((pixels = (uchar *)malloc(width * height * depth)) == NULL)
            return (-1);

	  return (gif_read_image(fp, cmap, buf[8] & GIF_INTERLACE));
    }
  }
}


//
// 'HDimage::gif_read_cmap()' - Read the colormap from a GIF file...
//

int
HDimage::gif_read_cmap(FILE       *fp,
  	               int        ncolors,
	               gif_cmap_t cmap,
	               int        *gray)
{
  int	i;


  // Read the colormap...
  if (fread(cmap, 3, ncolors, fp) < (size_t)ncolors)
    return (-1);

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
    for (i = 0; i < ncolors; i ++)
      cmap[i][0] = (cmap[i][0] * 31 + cmap[i][1] * 61 + cmap[i][2] * 8) / 100;

  return (0);
}


//
// 'HDimage::gif_get_block()' - Read a GIF data block...
//

int					// O - Number characters read
HDimage::gif_get_block(FILE  *fp,	// I - File to read from
	               uchar *buf)	// I - Input buffer
{
  int	count;				// Number of character to read


  // Read the count byte followed by the data from the file...
  if ((count = getc(fp)) == EOF)
  {
    gif_eof = 1;
    return (-1);
  }
  else if (count == 0)
    gif_eof = 1;
  else if (fread(buf, 1, count, fp) < (size_t)count)
  {
    gif_eof = 1;
    return (-1);
  }
  else
    gif_eof = 0;

  return (count);
}


//
// 'HDimage::gif_get_code()' - Get a LZW code from the file...
//.

int					// O - LZW code
HDimage::gif_get_code(FILE *fp,		// I - File to read from
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
      return (-1);	// Sorry, no more...

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
    if ((count = gif_get_block (fp, buf + last_byte)) <= 0)
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

  return ret;
}


//
// 'HDimage::gif_read_lzw()' - Read a byte from the LZW stream...
//

int						// I - Byte from stream
HDimage::gif_read_lzw(FILE *fp,			// I - File to read from
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
    max_code_size = 2 * clear_code;
    max_code      = clear_code + 2;

    // Initialize input buffers...
    gif_get_code(fp, 0, 1);

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
      firstcode = oldcode = gif_get_code(fp, code_size, 0);
    while (firstcode == clear_code);

    return (firstcode);
  }

  if (sp > stack)
    return (*--sp);

  while ((code = gif_get_code (fp, code_size, 0)) >= 0)
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

      firstcode = oldcode = gif_get_code(fp, code_size, 0);

      return (firstcode);
    }
    else if (code == end_code)
    {
      uchar	buf[260];


      if (!gif_eof)
        while (gif_get_block(fp, buf) > 0);

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
// 'HDimage::gif_read_image()' - Read a GIF image stream...
//

int						// I - 0 = success, -1 = failure
HDimage::gif_read_image(FILE       *fp,		// I - Input file
                        gif_cmap_t cmap,	// I - Colormap
	                int        interlace)	// I - Non-zero = interlaced image
{
  uchar		code_size,		// Code size
		*temp;			// Current pixel
  int		xpos,			// Current X position
		ypos,			// Current Y position
		pass;			// Current pass
  int		pixel;			// Current pixel
  int		xpasses[4] = { 8, 8, 4, 2 },
		ypasses[5] = { 0, 4, 2, 1, 999999 };


  xpos      = 0;
  ypos      = 0;
  pass      = 0;
  code_size = getc(fp);

  if (gif_read_lzw(fp, 1, code_size) < 0)
    return (-1);

  temp = pixels;

  while ((pixel = gif_read_lzw(fp, 0, code_size)) >= 0)
  {
    temp[0] = cmap[pixel][0];

    if (depth > 1)
    {
      temp[1] = cmap[pixel][1];
      temp[2] = cmap[pixel][2];
    }

    xpos ++;
    temp += depth;
    if (xpos == width)
    {
      xpos = 0;

      if (interlace)
      {
        ypos += xpasses[pass];
        temp += (xpasses[pass] - 1) * width * depth;

        if (ypos >= height)
	{
	  pass ++;

          ypos = ypasses[pass];
          temp = pixels + ypos * width * depth;
	}
      }
      else
	ypos ++;
    }

    if (ypos >= height)
      break;
  }

  return (0);
}


//
// 'HDimage::load_bmp()' - Read a BMP image file.
//

int				// O - 0 = success, -1 = fail
HDimage::load_bmp(FILE    *fp,	// I - File to read from
	          int     gray)	// I - Grayscale image?
{
  int		offset,		// Offset to bitmap data
		info_size,	// Size of info header
		planes,		// Number of planes (always 1)
		depth,		// Depth of image (bits)
		compression,	// Type of compression
		image_size,	// Size of image in bytes
		colors_used,	// Number of colors used
		colors_important,// Number of important colors
		bpp,		// Bytes per pixel
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
  offset = read_dword(fp);

  // Then the bitmap information...
  info_size        = read_dword(fp);
  width            = read_long(fp);
  height           = read_long(fp);
  planes           = read_word(fp);
  depth            = read_word(fp);
  compression      = read_dword(fp);
  image_size       = read_dword(fp);
  read_long(fp);
  read_long(fp);
  colors_used      = read_dword(fp);
  colors_important = read_dword(fp);

  if (info_size > 40)
    for (info_size -= 40; info_size > 0; info_size --)
      getc(fp);

  // Get colormap...
  if (colors_used == 0 && depth <= 8)
    colors_used = 1 << depth;

  fread(colormap, colors_used, 4, fp);

  // Setup image and buffers...
  depth  = gray ? 1 : 3;
  pixels = (uchar *)malloc(width * height * depth);
  if (pixels == NULL)
    return (-1);

  if (gray && depth <= 8)
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

  for (y = height - 1; y >= 0; y --)
  {
    ptr = pixels + y * width * depth;

    switch (depth)
    {
      case 1 : // Bitmap
          for (x = width, bit = 128; x > 0; x --)
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

	      *ptr++ = colormap[1][0];
	    }
	    else
	    {
	      if (!gray)
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
	  for (temp = (width + 7) / 8; temp & 3; temp ++)
	    getc(fp);
          break;

      case 4 : // 16-color
          for (x = width, bit = 0xf0; x > 0; x --)
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
		  count = getc(fp) * getc(fp) * width;
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

	      *ptr++ = colormap[temp >> 4][0];
	      bit    = 0x0f;
            }
	    else
	    {
              // Copy the color value...
	      if (!gray)
	      {
	        *ptr++ = colormap[temp & 15][2];
	        *ptr++ = colormap[temp & 15][1];
	      }

	      *ptr++ = colormap[temp & 15][0];
	      bit    = 0xf0;
	    }
	  }
          break;

      case 8 : // 256-color
          for (x = width; x > 0; x --)
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
		  count = getc(fp) * getc(fp) * width;
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
	      *ptr++ = colormap[temp][2];
	      *ptr++ = colormap[temp][1];
	    }

	    *ptr++ = colormap[temp][0];
	  }
          break;

      case 24 : // 24-bit RGB
          if (gray)
	  {
            for (x = width; x > 0; x --)
	    {
	      temp = getc(fp) * 8;
	      temp += getc(fp) * 61;
	      temp += getc(fp) * 31;
	      *ptr++ = temp / 100;
	    }
	  }
	  else
	  {
            for (x = width; x > 0; x --, ptr += 3)
	    {
	      ptr[2] = getc(fp);
	      ptr[1] = getc(fp);
	      ptr[0] = getc(fp);
	    }
          }

          // Read remaining bytes to align to 32 bits...
	  for (temp = width * 3; temp & 3; temp ++)
	    getc(fp);
          break;
    }
  }

  return (0);
}


//
// 'HDimage::read_word()' - Read a 16-bit unsigned integer.
//

unsigned short			// O - 16-bit unsigned integer
HDimage::read_word(FILE *fp)	// I - File to read from
{
  unsigned char	b0, b1;		// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);

  return ((b1 << 8) | b0);
}


//
// 'HDimage::read_dword()' - Read a 32-bit unsigned integer.
//

unsigned int			// O - 32-bit unsigned integer
HDimage::read_dword(FILE *fp)	// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// 'HDimage::read_long()' - Read a 32-bit signed integer.
//

int				// O - 32-bit signed integer
HDimage::read_long(FILE *fp)	// I - File to read from
{
  unsigned char b0, b1, b2, b3;	// Bytes from file


  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


//
// End of "$Id: image.cxx,v 1.12 2000/10/16 03:25:08 mike Exp $".
//
