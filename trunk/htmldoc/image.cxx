/*
 * "$Id: image.cxx,v 1.2 1999/11/09 22:16:42 mike Exp $"
 *
 *   Image handling routines for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   image_load()        - Load an image file from disk...
 *   image_flush_cache() - Flush the image cache...
 *   image_copy()        - Copy image files to the destination directory...
 *   image_compare()     - Compare two image filenames...
 *   image_load_gif()    - Load a GIF image file...
 *   image_load_jpeg()   - Load a JPEG image file.
 *   image_load_png()    - Load a PNG image file.
 *   gif_read_cmap()     - Read the colormap from a GIF file...
 *   gif_get_block()     - Read a GIF data block...
 *   gif_get_code()      - Get a LZW code from the file...
 *   gif_read_lzw()      - Read a byte from the LZW stream...
 *   gif_read_image()    - Read a GIF image stream...
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"

extern "C" {		/* Workaround for JPEG header problems... */
#include <jpeglib.h>	/* JPEG/JFIF image definitions */
}

#include <png.h>	/* Portable Network Graphics (PNG) definitions */


/*
 * GIF definitions...
 */

#define GIF_INTERLACE	0x40
#define GIF_COLORMAP	0x80

typedef uchar	gif_cmap_t[256][3];


/*
 * Local globals...
 */

static int	num_images = 0;		/* Number of images in cache */
static image_t	*images[MAX_IMAGES];	/* Images in cache */
static int	gif_eof = 0;		/* Did we hit EOF? */


/*
 * Local functions...
 */

static int	image_compare(image_t **img1, image_t **img2);

static int	image_load_jpeg(image_t *img, FILE *fp, int gray);

static int	image_load_png(image_t *img, FILE *fp, int gray);

static int	image_load_gif(image_t *img, FILE *fp, int gray);
static int	gif_read_cmap(FILE *fp, int ncolors, gif_cmap_t cmap,
		              int *gray);
static int	gif_get_block(FILE *fp, uchar *buffer);
static int	gif_get_code (FILE *fp, int code_size, int first_time);
static int	gif_read_lzw(FILE *fp, int first_time, int input_code_size);
static int	gif_read_image(FILE *fp, image_t *img, gif_cmap_t cmap,
		               int interlace);


/*
 * 'image_load()' - Load an image file from disk...
 */

image_t *			/* O - Pointer to image */
image_load(char *filename,	/* I - Name of image file */
           int  gray)		/* I - 0 = color, 1 = grayscale */
{
  FILE		*fp;		/* File pointer */
  uchar		header[16];	/* First 16 bytes of file */
  image_t	*img,		/* New image buffer */
		key,		/* Search key... */
		*keyptr,	/* Pointer to search key... */
		**match;	/* Matching image */
  int		status;		/* Status of load... */


 /*
  * Range check...
  */

  if (filename == NULL)
    return (NULL);

  if (filename[0] == '\0')	/* Microsoft VC++ runtime bug workaround... */
    return (NULL);

 /*
  * See if we've already loaded it...
  */

  if (num_images > 0)
  {
    strcpy(key.filename, filename);
    keyptr = &key;

    match = (image_t **)bsearch(&keyptr, images, num_images, sizeof(image_t *),
                                (int (*)(const void *, const void *))image_compare);
    if (match != NULL)
      return (*match);
  }

 /*
  * Figure out the file type...
  */

  if ((fp = fopen(filename, "rb")) == NULL)
    return (NULL);

  if (fread(header, 1, sizeof(header), fp) == 0)
    return (NULL);

  rewind(fp);

 /*
  * Allocate memory...
  */

  img = (image_t *)calloc(sizeof(image_t), 1);

  if (img == NULL)
    return (NULL);

  images[num_images] = img;

  strcpy(img->filename, filename);

 /*
  * Load the image as appropriate...
  */

  if (memcmp(header, "GIF87a", 6) == 0 ||
           memcmp(header, "GIF89a", 6) == 0)
    status = image_load_gif(img,  fp, gray);
  else if (memcmp(header, "\211PNG", 4) == 0)
    status = image_load_png(img, fp, gray);
  else if (memcmp(header + 6, "JFIF", 4) == 0)
    status = image_load_jpeg(img, fp, gray);
  else
    status = -1;

  fclose(fp);

  if (status)
  {
    free(img);
    return (NULL);
  }

  num_images ++;
  if (num_images > 1)
    qsort(images, num_images, sizeof(image_t *),
          (int (*)(const void *, const void *))image_compare);

  return (img);
}


/*
 * 'image_flush_cache()' - Flush the image cache...
 */

void
image_flush_cache(void)
{
  int	i;			/* Looping var */


 /*
  * Free the memory used by each image...
  */

  for (i = 0; i < num_images; i ++)
  {
    free(images[i]->pixels);
    free(images[i]);
  }

  num_images = 0;
}


/*
 * 'image_copy()' - Copy image files to the destination directory...
 */

void
image_copy(char *filename,	/* I - Source file */
           char *destpath)	/* I - Destination path */
{
  char	dest[255];		/* Destination file */
  FILE	*in, *out;		/* Input/output files */
  uchar	buffer[8192];		/* Data buffer */
  int	nbytes;			/* Number of bytes in buffer */


 /*
  * Figure out the destination filename...
  */

  if (strcmp(destpath, ".") == 0)
    strcpy(dest, file_basename(filename));
  else
    sprintf(dest, "%s/%s", destpath, file_basename(filename));

  if (strcmp(dest, filename) == 0)
    return;

 /*
  * Open files and copy...
  */

  if ((in = fopen(filename, "rb")) == NULL)
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


/*
 * 'image_compare()' - Compare two image filenames...
 */

static int
image_compare(image_t **img1,
              image_t **img2)
{
#if defined(WIN32) || defined(__EMX__)
  return (strcasecmp((*img1)->filename, (*img2)->filename));
#else
  return (strcmp((*img1)->filename, (*img2)->filename));
#endif /* WIN32 || __EMX__ */
}


/*
 * 'image_load_jpeg()' - Load a JPEG image file.
 */

static int			/* O - 0 = success, -1 = fail */
image_load_jpeg(image_t *img,	/* I - Image pointer */
                FILE    *fp,	/* I - File to load from */
                int     gray)	/* I - 0 = color, 1 = grayscale */
{
  struct jpeg_decompress_struct	cinfo;		/* Decompressor info */
  struct jpeg_error_mgr		jerr;		/* Error handler info */
  JSAMPROW			row;		/* Sample row pointer */


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

  img->width  = cinfo.output_width;
  img->height = cinfo.output_height;
  img->depth  = cinfo.output_components;
  img->pixels = (uchar *)malloc(img->width * img->height * img->depth);

  if (img->pixels == NULL)
  {
    jpeg_destroy_decompress(&cinfo);
    return (-1);
  }

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(img->pixels +
                     cinfo.output_scanline * cinfo.output_width *
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return (0);
}


/*
 * 'image_load_png()' - Load a PNG image file.
 */

static int			/* O - 0 = success, -1 = fail */
image_load_png(image_t *img,	/* I - Image pointer */
               FILE    *fp,	/* I - File to read from */
               int     gray)	/* I - 0 = color, 1 = grayscale */
{
  int		i;	/* Looping var */
  png_structp	pp;	/* PNG read pointer */
  png_infop	info;	/* PNG info pointers */
  png_bytep	*rows;	/* PNG row pointers */
  uchar		*inptr,	/* Input pixels */
		*outptr;/* Output pixels */


 /*
  * Setup the PNG data structures...
  */

  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

 /*
  * Initialize the PNG read "engine"...
  */

  png_init_io(pp, fp);

 /*
  * Get the image dimensions and load the output image...
  */

  png_read_info(pp, info);

  if (info->color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (info->color_type == PNG_COLOR_TYPE_GRAY)
    img->depth = 1;
  else
    img->depth = gray ? 1 : 3;

  img->width  = info->width;
  img->height = info->height;
  img->pixels = (uchar *)malloc(img->width * img->height * 3);

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);

    if (info->valid & PNG_INFO_sBIT)
      png_set_shift(pp, &(info->sig_bit));
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

  rows = (png_bytep *)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    if (info->color_type == PNG_COLOR_TYPE_GRAY)
      rows[i] = img->pixels + i * img->width;
    else
      rows[i] = img->pixels + i * img->width * 3;

  png_read_image(pp, rows);

 /*
  * Reformat the data as necessary for the reader...
  */

  if (gray && info->color_type != PNG_COLOR_TYPE_GRAY)
  {
   /*
    * Greyscale output needed...
    */

    for (inptr = img->pixels, outptr = img->pixels, i = img->width * img->height;
         i > 0;
         inptr += 3, outptr ++, i --)
      *outptr = (31 * inptr[0] + 61 * inptr[1] + 8 * inptr[2]) / 100;
  }

 /*
  * Free memory and return...
  */

  free(rows);

  png_read_end(pp, info);
  png_read_destroy(pp, info, NULL);

  return (0);
}


/*
 * 'image_load_gif()' - Load a GIF image file...
 */

static int			/* O - 0 = success, -1 = fail */
image_load_gif(image_t *img,	/* I - Image pointer */
               FILE    *fp,	/* I - File to load from */
               int     gray)	/* I - 0 = color, 1 = grayscale */
{
  uchar		buf[1024];	/* Input buffer */
  gif_cmap_t	cmap;		/* Colormap */
  int		ncolors,	/* Bits per pixel */
		transparent;	/* Transparent color index */


 /*
  * Read the header; we already know it is a GIF file...
  */

  fread(buf, 13, 1, fp);

  img->width  = (buf[7] << 8) | buf[6];
  img->height = (buf[9] << 8) | buf[8];
  ncolors     = 2 << (buf[10] & 0x07);

  if (buf[10] & GIF_COLORMAP)
    if (gif_read_cmap(fp, ncolors, cmap, &gray))
      return (-1);

  transparent = -1;

  while (1)
  {
    switch (getc(fp))
    {
      case ';' :	/* End of image */
          return (-1);		/* Early end of file */

      case '!' :	/* Extension record */
          buf[0] = getc(fp);
          if (buf[0] == 0xf9)	/* Graphic Control Extension */
          {
            gif_get_block(fp, buf);
            if (buf[0] & 1)	/* Get transparent color index */
              transparent = buf[3];
          }

          while (gif_get_block(fp, buf) != 0);
          break;

      case ',' :	/* Image data */
          fread(buf, 9, 1, fp);

          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);

	    if (gif_read_cmap(fp, ncolors, cmap, &gray))
	      return (-1);
	  }

          if (transparent >= 0)
          {
           /*
            * Make transparent color white...
            */

            cmap[transparent][0] = 255;
            cmap[transparent][1] = 255;
            cmap[transparent][2] = 255;
          }

          img->width  = (buf[5] << 8) | buf[4];
          img->height = (buf[7] << 8) | buf[6];
          img->depth  = gray ? 1 : 3;
          img->pixels = (uchar *)malloc(img->width * img->height * img->depth);
          if (img->pixels == NULL)
            return (-1);

	  return (gif_read_image(fp, img, cmap, buf[8] & GIF_INTERLACE));
    }
  }
}


/*
 * 'gif_read_cmap()' - Read the colormap from a GIF file...
 */

static int
gif_read_cmap(FILE       *fp,
  	      int        ncolors,
	      gif_cmap_t cmap,
	      int        *gray)
{
  int	i;


 /*
  * Read the colormap...
  */

  if (fread(cmap, 3, ncolors, fp) < (size_t)ncolors)
    return (-1);

 /*
  * Check to see if the colormap is a grayscale ramp...
  */

  for (i = 0; i < ncolors; i ++)
    if (cmap[i][0] != cmap[i][1] || cmap[i][1] != cmap[i][2])
      break;

  if (i == ncolors)
  {
    *gray = 1;
    return (0);
  }

 /*
  * If this needs to be a grayscale image, convert the RGB values to
  * luminance values...
  */

  if (*gray)
    for (i = 0; i < ncolors; i ++)
      cmap[i][0] = (cmap[i][0] * 31 + cmap[i][1] * 61 + cmap[i][2] * 8) / 100;

  return (0);
}


/*
 * 'gif_get_block()' - Read a GIF data block...
 */

static int			/* O - Number characters read */
gif_get_block(FILE  *fp,	/* I - File to read from */
	      uchar *buf)	/* I - Input buffer */
{
  int	count;			/* Number of character to read */


 /*
  * Read the count byte followed by the data from the file...
  */

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


/*
 * 'gif_get_code()' - Get a LZW code from the file...
 */

static int			/* O - LZW code */
gif_get_code(FILE *fp,		/* I - File to read from */
	     int  code_size,	/* I - Size of code in bits */
	     int  first_time)	/* I - 1 = first time, 0 = not first time */
{
  unsigned		i, j,		/* Looping vars */
			ret;		/* Return value */
  int			count;		/* Number of bytes read */
  static uchar		buf[280];	/* Input buffer */
  static unsigned	curbit,		/* Current bit */
			lastbit,	/* Last bit in buffer */
			done,		/* Done with this buffer? */
			last_byte;	/* Last byte in buffer */
  static unsigned	bits[8] =	/* Bit masks for codes */
			{
			  0x01, 0x02, 0x04, 0x08,
			  0x10, 0x20, 0x40, 0x80
			};


  if (first_time)
  {
   /*
    * Just initialize the input buffer...
    */

    curbit  = 0;
    lastbit = 0;
    done    = 0;

    return (0);
  }


  if ((curbit + code_size) >= lastbit)
  {
   /*
    * Don't have enough bits to hold the code...
    */

    if (done)
      return (-1);	/* Sorry, no more... */

   /*
    * Move last two bytes to front of buffer...
    */

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

   /*
    * Read in another buffer...
    */

    if ((count = gif_get_block (fp, buf + last_byte)) <= 0)
    {
     /*
      * Whoops, no more data!
      */

      done = 1;
      return (-1);
    }

   /*
    * Update buffer state...
    */

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


/*
 * 'gif_read_lzw()' - Read a byte from the LZW stream...
 */

static int				/* I - Byte from stream */
gif_read_lzw(FILE *fp,			/* I - File to read from */
	     int  first_time,		/* I - 1 = first time, 0 = not first time */
 	     int  input_code_size)	/* I - Code size in bits */
{
  int		i,			/* Looping var */
		code,			/* Current code */
		incode;			/* Input code */
  static short	fresh = 0,		/* 1 = empty buffers */
		code_size,		/* Current code size */
		set_code_size,		/* Initial code size set */
		max_code,		/* Maximum code used */
		max_code_size,		/* Maximum code size */
		firstcode,		/* First code read */
		oldcode,		/* Last code read */
		clear_code,		/* Clear code for LZW input */
		end_code,		/* End code for LZW input */
		table[2][4096],		/* String table */
		stack[8192],		/* Output stack */
		*sp;			/* Current stack pointer */


  if (first_time)
  {
   /*
    * Setup LZW state...
    */

    set_code_size = input_code_size;
    code_size     = set_code_size + 1;
    clear_code    = 1 << set_code_size;
    end_code      = clear_code + 1;
    max_code_size = 2 * clear_code;
    max_code      = clear_code + 2;

   /*
    * Initialize input buffers...
    */

    gif_get_code(fp, 0, 1);

   /*
    * Wipe the decompressor table...
    */

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


/*
 * 'gif_read_image()' - Read a GIF image stream...
 */

static int				/* I - 0 = success, -1 = failure */
gif_read_image(FILE       *fp,		/* I - Input file */
	       image_t    *img,		/* I - Image pointer */
	       gif_cmap_t cmap,		/* I - Colormap */
	       int        interlace)	/* I - Non-zero = interlaced image */
{
  uchar		code_size,		/* Code size */
		*temp;			/* Current pixel */
  int		xpos,			/* Current X position */
		ypos,			/* Current Y position */
		pass;			/* Current pass */
  int		pixel;			/* Current pixel */
  static int	xpasses[4] = { 8, 8, 4, 2 },
		ypasses[5] = { 0, 4, 2, 1, 999999 };


  xpos      = 0;
  ypos      = 0;
  pass      = 0;
  code_size = getc(fp);

  if (gif_read_lzw(fp, 1, code_size) < 0)
    return (-1);

  temp = img->pixels;

  while ((pixel = gif_read_lzw(fp, 0, code_size)) >= 0)
  {
    temp[0] = cmap[pixel][0];

    if (img->depth > 1)
    {
      temp[1] = cmap[pixel][1];
      temp[2] = cmap[pixel][2];
    }

    xpos ++;
    temp += img->depth;
    if (xpos == img->width)
    {
      xpos = 0;

      if (interlace)
      {
        ypos += xpasses[pass];
        temp += (xpasses[pass] - 1) * img->width * img->depth;

        if (ypos >= img->height)
	{
	  pass ++;

          ypos = ypasses[pass];
          temp = img->pixels + ypos * img->width * img->depth;
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


/*
 * End of "$Id: image.cxx,v 1.2 1999/11/09 22:16:42 mike Exp $".
 */
