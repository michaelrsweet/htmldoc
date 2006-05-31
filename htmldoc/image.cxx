/*
 * "$Id$"
 *
 *   Image handling routines for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2005 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   gif_read_cmap()      - Read the colormap from a GIF file...
 *   gif_get_block()      - Read a GIF data block...
 *   gif_get_code()       - Get a LZW code from the file...
 *   gif_read_image()     - Read a GIF image stream...
 *   gif_read_lzw()       - Read a byte from the LZW stream...
 *   image_compare()      - Compare two image filenames...
 *   image_copy()         - Copy image files to the destination directory...
 *   image_flush_cache()  - Flush the image cache...
 *   image_getlist()      - Get the list of images that are loaded.
 *   image_load()         - Load an image file from disk...
 *   image_load_bmp()     - Read a BMP image file.
 *   image_load_gif()     - Load a GIF image file...
 *   image_load_jpeg()    - Load a JPEG image file.
 *   image_load_png()     - Load a PNG image file.
 *   image_need_mask()    - Allocate memory for the image mask...
 *   image_set_mask()     - Set a bit in the image mask.
 *   jpeg_error_handler() - Handle JPEG errors by not exiting.
 *   read_word()          - Read a 16-bit unsigned integer.
 *   read_dword()         - Read a 32-bit unsigned integer.
 *   read_long()          - Read a 32-bit signed integer.
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
 * BMP definitions...
 */

#ifndef BI_RGB
#  define BI_RGB       0             /* No compression - straight BGR data */
#  define BI_RLE8      1             /* 8-bit run-length compression */
#  define BI_RLE4      2             /* 4-bit run-length compression */
#  define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */
#endif /* !BI_RGB */


/*
 * Local globals...
 */

static int	num_images = 0,		/* Number of images in cache */
		alloc_images = 0;	/* Allocated images */
static image_t	**images = NULL;	/* Images in cache */
static int	gif_eof = 0;		/* Did we hit EOF? */


/*
 * Local functions...
 */

static int	gif_read_cmap(FILE *fp, int ncolors, gif_cmap_t cmap,
		              int *gray);
static int	gif_get_block(FILE *fp, uchar *buffer);
static int	gif_get_code (FILE *fp, int code_size, int first_time);
static int	gif_read_image(FILE *fp, image_t *img, gif_cmap_t cmap,
		               int interlace, int transparent);
static int	gif_read_lzw(FILE *fp, int first_time, int input_code_size);

static int	image_compare(image_t **img1, image_t **img2);
static int	image_load_bmp(image_t *img, FILE *fp, int gray, int load_data);
static int	image_load_gif(image_t *img, FILE *fp, int gray, int load_data);
static int	image_load_jpeg(image_t *img, FILE *fp, int gray, int load_data);
static int	image_load_png(image_t *img, FILE *fp, int gray, int load_data);
static void	image_need_mask(image_t *img, int scaling = 1);
static void	image_set_mask(image_t *img, int x, int y, uchar alpha = 0);

static void		jpeg_error_handler(j_common_ptr);
static int		read_long(FILE *fp);
static unsigned short	read_word(FILE *fp);
static unsigned int	read_dword(FILE *fp);


/*
 * 'gif_read_cmap()' - Read the colormap from a GIF file...
 */

static int				/* O  - 0 on success, -1 on error */
gif_read_cmap(FILE       *fp,		/* I  - File to read from */
  	      int        ncolors,	/* I  - Number of colors */
	      gif_cmap_t cmap,		/* IO - Colormap array */
	      int        *gray)		/* IO - 1 = grayscale */
{
  int	i;				/* Looping var */


 /*
  * Read the colormap...
  */

  if (fread(cmap, 3, ncolors, fp) < (size_t)ncolors)
  {
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read GIF colormap: %s", strerror(errno));
    return (-1);
  }

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
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read GIF block of %d bytes: %s", count,
                   strerror(errno));
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

    curbit    = 0;
    lastbit   = 0;
    last_byte = 0;
    done      = 0;

    return (0);
  }

  if ((curbit + code_size) >= lastbit)
  {
   /*
    * Don't have enough bits to hold the code...
    */

    if (done)
    {
      progress_error(HD_ERROR_READ_ERROR,
                     "Not enough data left to read GIF compression code.");
      return (-1);	/* Sorry, no more... */
    }

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
 * 'gif_read_image()' - Read a GIF image stream...
 */

static int				/* I - 0 = success, -1 = failure */
gif_read_image(FILE       *fp,		/* I - Input file */
	       image_t    *img,		/* I - Image pointer */
	       gif_cmap_t cmap,		/* I - Colormap */
	       int        interlace,	/* I - Non-zero = interlaced image */
	       int        transparent)	/* I - Transparent color */
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
 * 'image_compare()' - Compare two image filenames...
 */

static int			/* O - Result of comparison */
image_compare(image_t **img1,	/* I - First image */
              image_t **img2)	/* I - Second image */
{
#ifdef WIN32
  return (strcasecmp((*img1)->filename, (*img2)->filename));
#else
  return (strcmp((*img1)->filename, (*img2)->filename));
#endif /* WIN32 */
}


/*
 * 'image_copy()' - Copy image files to the destination directory...
 */

void
image_copy(const char *src,		/* I - Source file */
           const char *realsrc,		/* I - Real source file */
           const char *destpath)	/* I - Destination path */
{
  char		dest[255];		/* Destination file */
  FILE		*in, *out;		/* Input/output files */
  uchar		buffer[8192];		/* Data buffer */
  int		nbytes;			/* Number of bytes in buffer */


  if (!src || !realsrc || !destpath)
    return;

 /*
  * Figure out the destination filename...
  */

  if (!strcmp(destpath, "."))
    strlcpy(dest, file_basename(src), sizeof(dest));
  else
    snprintf(dest, sizeof(dest), "%s/%s", destpath, file_basename(src));

  if (!strcmp(dest, realsrc))
    return;

 /*
  * Open files and copy...
  */

  if ((in = fopen(realsrc, "rb")) == NULL)
  {
    progress_error(HD_ERROR_READ_ERROR, "Unable to open \"%s\" - %s",
                   realsrc, strerror(errno));
    return;
  }

  if ((out = fopen(dest, "wb")) == NULL)
  {
    progress_error(HD_ERROR_READ_ERROR, "Unable to create \"%s\" - %s",
                   dest, strerror(errno));
    fclose(in);
    return;
  }

  while ((nbytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
    fwrite(buffer, 1, nbytes, out);

  progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

  fclose(in);
  fclose(out);
}


/*
 * 'image_find()' - Find an image file in memory...
 */

image_t *			/* O - Pointer to image */
image_find(const char *filename,/* I - Name of image file */
           int        load_data)/* I - 1 = load image data */
{
  image_t	key,		/* Search key... */
		*keyptr,	/* Pointer to search key... */
		**match;	/* Matching image */


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
    strlcpy(key.filename, filename, sizeof(key.filename));
    keyptr = &key;

    match = (image_t **)bsearch(&keyptr, images, num_images, sizeof(image_t *),
                                (int (*)(const void *, const void *))image_compare);
    if (match != NULL)
    {
      if (load_data && !(*match)->pixels)
        return (image_load((*match)->filename, (*match)->depth == 1, 1));
      else
        return (*match);
    }
  }

  return (NULL);
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
    if (images[i]->mask)
      free(images[i]->mask);

    if (images[i]->pixels)
      free(images[i]->pixels);

    free(images[i]);
  }

  if (alloc_images)
  {
    free(images);

    alloc_images = 0;
  }

  num_images = 0;
}


/*
 * 'image_getlist()' - Get the list of images that are loaded.
 */

int				/* O - Number of images in array */
image_getlist(image_t ***ptrs)	/* O - Pointer to images array */
{
  *ptrs = images;
  return (num_images);
}


/*
 * 'image_load()' - Load an image file from disk...
 */

image_t *			/* O - Pointer to image */
image_load(const char *filename,/* I - Name of image file */
           int        gray,	/* I - 0 = color, 1 = grayscale */
           int        load_data)/* I - 1 = load image data, 0 = just info */
{
#ifdef DEBUG
  int		i;		/* Looping var */
#endif // DEBUG
  FILE		*fp;		/* File pointer */
  uchar		header[16];	/* First 16 bytes of file */
  image_t	*img,		/* New image buffer */
		key,		/* Search key... */
		*keyptr,	/* Pointer to search key... */
		**match,	/* Matching image */
		**temp;		/* Temporary array pointer */
  int		status;		/* Status of load... */
  const char	*realname;	/* Real filename */


 /*
  * Range check...
  */

  if (filename == NULL)
    return (NULL);

  if (filename[0] == '\0')	/* Microsoft VC++ runtime bug workaround... */
    return (NULL);

  DEBUG_printf(("image_load(filename=\"%s\", gray=%d, load_data=%d)\n",
                filename, gray, load_data));
  DEBUG_printf(("Path = \"%s\"\n", Path));

 /*
  * See if we've already loaded it...
  */

  if (num_images > 0)
  {
    strlcpy(key.filename, filename, sizeof(key.filename));
    keyptr = &key;

    match = (image_t **)bsearch(&keyptr, images, num_images, sizeof(image_t *),
                                (int (*)(const void *, const void *))image_compare);
    if (match != NULL && (!load_data || (*match)->pixels))
    {
      (*match)->use ++;
      return (*match);
    }
  }
  else
    match = NULL;

 /*
  * Figure out the file type...
  */

  if ((realname = file_find(Path, filename)) == NULL)
  {
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to find image file \"%s\"!", filename);
    return (NULL);
  }

  if ((fp = fopen(realname, "rb")) == NULL)
  {
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open image file \"%s\" (%s) for reading!",
		   filename, realname);
    return (NULL);
  }

  if (fread(header, 1, sizeof(header), fp) == 0)
  {
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read image file \"%s\"!", filename);
    fclose(fp);
    return (NULL);
  }

#ifdef DEBUG
  printf("Header for \"%s\" (%s): \"", filename, realname);

  for (i = 0; i < sizeof(header); i ++)
    if (header[i] < ' ' || header[i] >= 127)
      printf("\\x%02X", header[i]);
    else
      putchar(header[i]);

  puts("\"\n");

  printf("match = %p\n", match);
#endif // DEBUG

  rewind(fp);

  // See if the images array needs to be resized...
  if (!match)
  {
    if (num_images >= alloc_images)
    {
      // Yes...
      alloc_images += ALLOC_FILES;

      if (num_images == 0)
	temp = (image_t **)malloc(sizeof(image_t *) * alloc_images);
      else
	temp = (image_t **)realloc(images, sizeof(image_t *) * alloc_images);

      if (temp == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
	               "Unable to allocate memory for %d images - %s",
                       alloc_images, strerror(errno));
	fclose(fp);
	return (NULL);
      }

      images = temp;
    }

    // Allocate memory...
    img = (image_t *)calloc(sizeof(image_t), 1);

    if (img == NULL)
    {
      progress_error(HD_ERROR_READ_ERROR, "Unable to allocate memory for \"%s\"",
                     filename);
      fclose(fp);
      return (NULL);
    }

    images[num_images] = img;

    strlcpy(img->filename, filename, sizeof(img->filename));
    img->use = 1;
  }
  else
    img = *match;

  // Load the image as appropriate...
  if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0)
    status = image_load_gif(img,  fp, gray, load_data);
  else if (memcmp(header, "BM", 2) == 0)
    status = image_load_bmp(img, fp, gray, load_data);
  else if (memcmp(header, "\211PNG", 4) == 0)
    status = image_load_png(img, fp, gray, load_data);
  else if (memcmp(header, "\377\330\377", 3) == 0)
    status = image_load_jpeg(img, fp, gray, load_data);
  else
  {
    progress_error(HD_ERROR_BAD_FORMAT, "Unknown image file format for \"%s\"!", filename);
    fclose(fp);
    free(img);
    return (NULL);
  }

  fclose(fp);

  if (status)
  {
    progress_error(HD_ERROR_READ_ERROR, "Unable to load image file \"%s\"!", filename);
    if (!match)
      free(img);
    return (NULL);
  }

  if (!match)
  {
    num_images ++;
    if (num_images > 1)
      qsort(images, num_images, sizeof(image_t *),
            (int (*)(const void *, const void *))image_compare);
  }

  return (img);
}


/*
 * 'image_load_bmp()' - Read a BMP image file.
 */

static int			/* O - 0 = success, -1 = fail */
image_load_bmp(image_t *img,	/* I - Image to load into */
               FILE    *fp,	/* I - File to read from */
	       int     gray,	/* I - Grayscale image? */
               int     load_data)/* I - 1 = load image data, 0 = just info */
{
  int		info_size,	/* Size of info header */
		depth,		/* Depth of image (bits) */
		compression,	/* Type of compression */
		colors_used,	/* Number of colors used */
		x, y,		/* Looping vars */
		color,		/* Color of RLE pixel */
		count,		/* Number of times to repeat */
		temp,		/* Temporary color */
		align;		/* Alignment bytes */
  uchar		bit,		/* Bit in image */
		byte;		/* Byte in image */
  uchar		*ptr;		/* Pointer into pixels */
  uchar		colormap[256][4];/* Colormap */


  // Get the header...
  getc(fp);			/* Skip "BM" sync chars */
  getc(fp);
  read_dword(fp);		/* Skip size */
  read_word(fp);		/* Skip reserved stuff */
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

  // If this image is indexed and we are writing an encrypted PDF file, bump the use count so
  // we create an image object (Acrobat 6 bug workaround)
  if (depth <= 8 && Encryption)
    img->use ++;

  // Return now if we only need the dimensions...
  if (!load_data)
    return (0);

  img->pixels = (uchar *)malloc(img->width * img->height * img->depth);
  if (img->pixels == NULL)
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
  byte  = 0;
  temp  = 0;

  for (y = img->height - 1; y >= 0; y --)
  {
    ptr = img->pixels + y * img->width * img->depth;

    switch (depth)
    {
      case 1 : /* Bitmap */
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

         /*
	  * Read remaining bytes to align to 32 bits...
	  */

	  for (temp = (img->width + 7) / 8; temp & 3; temp ++)
	    getc(fp);
          break;

      case 4 : /* 16-color */
          for (x = img->width, bit = 0xf0; x > 0; x --)
	  {
	   /*
	    * Get a new count as needed...
	    */

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
		 /*
		  * End of line...
		  */

                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		 /*
		  * End of image...
		  */

		  break;
		}
		else if (count == 2)
		{
		 /*
		  * Delta...
		  */

		  count = getc(fp) * getc(fp) * img->width;
		  color = 0;
		}
		else
		{
		 /*
		  * Absolute...
		  */

		  color = -1;
		  align = ((4 - (count & 3)) / 2) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }

           /*
	    * Get a new color as needed...
	    */

	    count --;

            if (bit == 0xf0)
	    {
              if (color < 0)
		temp = getc(fp);
	      else
		temp = color;

             /*
	      * Copy the color value...
	      */

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
             /*
	      * Copy the color value...
	      */

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

      case 8 : /* 256-color */
          for (x = img->width; x > 0; x --)
	  {
	   /*
	    * Get a new count as needed...
	    */

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
		 /*
		  * End of line...
		  */

                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		 /*
		  * End of image...
		  */

		  break;
		}
		else if (count == 2)
		{
		 /*
		  * Delta...
		  */

		  count = getc(fp) * getc(fp) * img->width;
		  color = 0;
		}
		else
		{
		 /*
		  * Absolute...
		  */

		  color = -1;
		  align = (2 - (count & 1)) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }

           /*
	    * Get a new color as needed...
	    */

            if (color < 0)
	      temp = getc(fp);
	    else
	      temp = color;

            count --;

           /*
	    * Copy the color value...
	    */

            if (!gray)
	    {
	      *ptr++ = colormap[temp][2];
	      *ptr++ = colormap[temp][1];
	    }

	    *ptr++ = colormap[temp][0];
	  }
          break;

      case 24 : /* 24-bit RGB */
          if (gray)
	  {
            for (x = img->width; x > 0; x --)
	    {
	      temp = getc(fp) * 8;
	      temp += getc(fp) * 61;
	      temp += getc(fp) * 31;
	      *ptr++ = temp / 100;
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

         /*
	  * Read remaining bytes to align to 32 bits...
	  */

	  for (temp = img->width * 3; temp & 3; temp ++)
	    getc(fp);
          break;
    }
  }

  return (0);
}


/*
 * 'image_load_gif()' - Load a GIF image file...
 */

static int			/* O - 0 = success, -1 = fail */
image_load_gif(image_t *img,	/* I - Image pointer */
               FILE    *fp,	/* I - File to load from */
               int     gray,	/* I - 0 = color, 1 = grayscale */
               int     load_data)/* I - 1 = load image data, 0 = just info */
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

  // If we are writing an encrypted PDF file, bump the use count so we create
  // an image object (Acrobat 6 bug workaround)
  if (Encryption)
    img->use ++;

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
            * Map transparent color to background color...
            */

            if (BodyColor[0])
	    {
	      float rgb[3]; /* RGB color */


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

           /*
	    * Allocate a mask image...
	    */

            image_need_mask(img);
	  }

          img->width  = (buf[5] << 8) | buf[4];
          img->height = (buf[7] << 8) | buf[6];
          img->depth  = gray ? 1 : 3;
	  if (!load_data)
	    return (0);

          img->pixels = (uchar *)malloc(img->width * img->height * img->depth);
          if (img->pixels == NULL)
            return (-1);

	  return (gif_read_image(fp, img, cmap, buf[8] & GIF_INTERLACE, transparent));
    }
  }
}


/*
 * 'image_load_jpeg()' - Load a JPEG image file.
 */

static int			/* O - 0 = success, -1 = fail */
image_load_jpeg(image_t *img,	/* I - Image pointer */
                FILE    *fp,	/* I - File to load from */
                int     gray,	/* I - 0 = color, 1 = grayscale */
                int     load_data)/* I - 1 = load image data, 0 = just info */
{
  struct jpeg_decompress_struct	cinfo;		/* Decompressor info */
  struct jpeg_error_mgr		jerr;		/* Error handler info */
  JSAMPROW			row;		/* Sample row pointer */


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
               int     gray,	/* I - 0 = color, 1 = grayscale */
               int     load_data)/* I - 1 = load image data, 0 = just info */
{
  int		i, j;		/* Looping vars */
  png_structp	pp;		/* PNG read pointer */
  png_infop	info;		/* PNG info pointers */
  int		depth;		/* Input image depth */
  png_bytep	*rows;		/* PNG row pointers */
  uchar		*inptr,		/* Input pixels */
		*outptr;	/* Output pixels */


 /*
  * Setup the PNG data structures...
  */

  pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pp)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY, "Unable to allocate memory for PNG file: %s",
                   strerror(errno));
    return (-1);
  }

  info = png_create_info_struct(pp);
  if (!info)
  {
    progress_error(HD_ERROR_OUT_OF_MEMORY, "Unable to allocate memory for PNG info: %s",
                   strerror(errno));

    png_destroy_read_struct(&pp, NULL, NULL);

    return (-1);
  }

  rows = NULL;

  if (setjmp(pp->jmpbuf)) 
  {
    progress_error(HD_ERROR_BAD_FORMAT, "PNG file contains errors!");

    png_destroy_read_struct(&pp, &info, NULL);

    if (img != NULL && img->pixels != NULL)
      free(img->pixels);

    if (rows != NULL)
      free(rows);

    return (-1);
  }

 /*
  * Initialize the PNG read "engine"...
  */

  png_init_io(pp, fp);

 /*
  * Get the image dimensions and convert to grayscale or RGB...
  */

  png_read_info(pp, info);

  if (info->color_type & PNG_COLOR_MASK_PALETTE)
  {
    png_set_expand(pp);

    // If we are writing an encrypted PDF file, bump the use count so we create
    // an image object (Acrobat 6 bug workaround)
    if (Encryption)
      img->use ++;
  }
  else if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

  if (info->color_type & PNG_COLOR_MASK_COLOR)
  {
    depth      = 3;
    img->depth = gray ? 1 : 3;
  }
  else
  {
    depth      = 1;
    img->depth = 1;
  }

  img->width  = info->width;
  img->height = info->height;

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
  {
    if ((PSLevel == 0 && PDFVersion >= 14) || PSLevel == 3)
      image_need_mask(img, 8);
    else if (PSLevel == 0 && PDFVersion == 13)
      image_need_mask(img, 2);
    else
      image_need_mask(img);

    depth ++;
  }

#ifdef DEBUG
  printf("color_type=0x%04x, depth=%d, img->width=%d, img->height=%d, img->depth=%d\n",
         info->color_type, depth, img->width, img->height, img->depth);
  if (info->color_type & PNG_COLOR_MASK_COLOR)
    puts("    COLOR");
  else
    puts("    GRAYSCALE");
  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
    puts("    ALPHA");
  if (info->color_type & PNG_COLOR_MASK_PALETTE)
    puts("    PALETTE");
#endif // DEBUG

  if (!load_data)
  {
    png_destroy_read_struct(&pp, &info, NULL);
    return (0);
  }

  img->pixels = (uchar *)malloc(img->width * img->height * depth);

 /*
  * Allocate pointers...
  */

  rows = (png_bytep *)calloc(info->height, sizeof(png_bytep));

  for (i = 0; i < (int)info->height; i ++)
    rows[i] = img->pixels + i * img->width * depth;

 /*
  * Read the image, handling interlacing as needed...
  */

  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, img->height);

 /*
  * Generate the alpha mask as necessary...
  */

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
  {
#ifdef DEBUG
    for (inptr = img->pixels, i = 0; i < img->height; i ++)
    {
      for (j = 0; j < img->width; j ++, inptr += depth)
        switch (depth)
	{
	  case 2 :
	      printf(" %02X%02X", inptr[0], inptr[1]);
	      break;
	  case 4 :
	      printf(" %02X%02X%02X%02X", inptr[0], inptr[1], inptr[2], inptr[3]);
	      break;
	}

      putchar('\n');
    }
#endif // DEBUG

    for (inptr = img->pixels + depth - 1, i = 0; i < img->height; i ++)
      for (j = 0; j < img->width; j ++, inptr += depth)
        image_set_mask(img, j, i, *inptr);
  }

 /*
  * Reformat the data as necessary for the reader...
  */

  if (gray && info->color_type & PNG_COLOR_MASK_COLOR)
  {
   /*
    * Greyscale output needed...
    */

    for (inptr = img->pixels, outptr = img->pixels, i = img->width * img->height;
         i > 0;
         inptr += depth, outptr ++, i --)
      *outptr = (31 * inptr[0] + 61 * inptr[1] + 8 * inptr[2]) / 100;
  }
  else if (img->depth != depth)
  {
   /*
    * Remove alpha from final array...
    */

    if (depth == 4)
    {
      for (inptr = img->pixels, outptr = img->pixels, i = img->width * img->height;
           i > 0;
           inptr ++, i --)
      {
        *outptr++ = *inptr++;
        *outptr++ = *inptr++;
        *outptr++ = *inptr++;
      }
    }
    else
    {
      for (inptr = img->pixels, outptr = img->pixels, i = img->width * img->height;
           i > 0;
           inptr ++, i --)
        *outptr++ = *inptr++;
    }
  }

 /*
  * Free memory and return...
  */

  free(rows);

  png_read_end(pp, info);
  png_destroy_read_struct(&pp, &info, NULL);

  return (0);
}


/*
 * 'image_need_mask()' - Allocate memory for the image mask...
 */

static void
image_need_mask(image_t *img,	/* I - Image to add mask to */
                int     scaling)/* I - Scaling for mask image */
{
  int	size;			/* Byte size of mask image */


  if (img == NULL || img->mask != NULL)
    return;

 /* 
  * Figure out the size of the mask image, and then allocate and set all the
  * bits needed...
  */

  img->maskscale = scaling;

  if (scaling == 8)
  {
    // Alpha image
    img->maskwidth = img->width;
    size           = img->width * img->height;
  }
  else
  {
    // Alpha mask
    img->maskwidth = (img->width * scaling + 7) / 8;
    size           = img->maskwidth * img->height * scaling;
  }

  img->mask = (uchar *)calloc(size, 1);
}


/*
 * 'image_set_mask()' - Set a bit in the image mask.
 */

static void
image_set_mask(image_t *img,	/* I - Image to operate on */
               int     x,	/* I - X coordinate */
               int     y,	/* I - Y coordinate */
	       uchar   alpha)	/* I - Alpha value */
{
  int		i, j;		/* Looping vars */
  uchar		*maskptr;	/* Pointer into mask image */
  static uchar	masks[8] =	/* Masks for each bit */
		{
		  0x80, 0x40, 0x20, 0x10,
		  0x08, 0x04, 0x02, 0x01
		};
  static uchar	dither[4][4] = // Simple 4x4 clustered-dot dither
		{
		  { 0,  2,  15, 6 },
		  { 4,  12, 9,  11 },
		  { 14, 7,  1,  3 },
		  { 8,  10, 5,  13 }
	        };


  if (img == NULL || img->mask == NULL || x < 0 || x >= img->width ||
      y < 0 || y > img->height)
    return;

  if (img->maskscale == 8)
  {
    // Store the alpha value directly...
    if (PSLevel)
      img->mask[y * img->maskwidth + x] = 255 - alpha;
    else
      img->mask[y * img->maskwidth + x] = alpha;
  }
  else
  {
    // Store an alpha mask...
    x *= img->maskscale;
    y *= img->maskscale;
    alpha >>= 4;

    for (i = 0; i < img->maskscale; i ++, y ++, x -= img->maskscale)
      for (j = 0; j < img->maskscale; j ++, x ++)
      {
	maskptr  = img->mask + y * img->maskwidth + x / 8;
	if (alpha <= dither[x & 3][y & 3])
	  *maskptr |= masks[x & 7];
      }
  }
}


/*
 * 'image_unload()' - Unload an image from memory.
 */

void
image_unload(image_t *img)	// I - Image
{
  if (!img)
    return;

  if (!img->use || !img->pixels)
    return;

  if (img->obj)
    img->use = 0;
  else
    img->use --;

  if (img->use)
    return;

  free(img->pixels);
  img->pixels = NULL;
}


/*
 * 'jpeg_error_handler()' - Handle JPEG errors by not exiting.
 */

static void
jpeg_error_handler(j_common_ptr)
{
  return;
}


/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */

static unsigned short     /* O - 16-bit unsigned integer */
read_word(FILE *fp)       /* I - File to read from */
{
  unsigned char b0, b1; /* Bytes from file */

  b0 = getc(fp);
  b1 = getc(fp);

  return ((b1 << 8) | b0);
}


/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */

static unsigned int               /* O - 32-bit unsigned integer */
read_dword(FILE *fp)              /* I - File to read from */
{
  unsigned char b0, b1, b2, b3; /* Bytes from file */

  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


/*
 * 'read_long()' - Read a 32-bit signed integer.
 */

static int                        /* O - 32-bit signed integer */
read_long(FILE *fp)               /* I - File to read from */
{
  unsigned char b0, b1, b2, b3; /* Bytes from file */

  b0 = getc(fp);
  b1 = getc(fp);
  b2 = getc(fp);
  b3 = getc(fp);

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


/*
 * End of "$Id$".
 */
