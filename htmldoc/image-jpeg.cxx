//
// "$Id$"
//
//   JPEG image handling routines for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdJPEGImage::hdJPEGImage() - Create a new JPEG image.
//   hdJPEGImage::load()        - Load a JPEG image...
//   hdJPEGImage::real_load()   - Really load a JPEG image.
//   jpg_error()                - Handle JPEG errors by not exiting.
//   jpg_init()                 - Initialize the JPEG source.
//   jpg_fill()                 - Fill the JPEG source buffer.
//   jpg_skip()                 - Skip data from a JPEG source.
//   jpg_term()                 - Terminate reading from a file.
//   jpg_source::jpg_source()   - Create a JPEG source mananger.
//

//
// Include necessary headers.
//

#include "image.h"
#include "hdstring.h"

extern "C" {		// Workaround for JPEG header problems...
#include <jpeglib.h>	// JPEG/JFIF image definitions
}


//
// Source manager structure...
//

struct jpg_source
{
  jpeg_source_mgr	pub_;		// Public JPEG source manager data
  hdFile		*fp_;		// File to read from
  JOCTET		buffer_[8192];	// Buffer to read into

  jpg_source(j_decompress_ptr cinfo, hdFile *fp);
};


//
// Local functions...
//

static void		jpg_error(j_common_ptr);
static void		jpg_init(j_decompress_ptr cinfo);
static boolean		jpg_fill(j_decompress_ptr cinfo);
static void		jpg_skip(j_decompress_ptr cinfo, long num_bytes);
static void		jpg_term(j_decompress_ptr cinfo);


//
// 'hdJPEGImage::hdJPEGImage()' - Create a new JPEG image.
//

hdJPEGImage::hdJPEGImage(const char *p,	// I - URI for image file
                         bool       gs)	// I - 0 for color, 1 for grayscale
{
  uri(p);

  real_load(0, gs);
}


//
// 'hdJPEGImage::check()' - Try to load an image file as a JPEG.
//

hdImage *
hdJPEGImage::check(const char *p,	// I - URI for image file
                   bool       gs,	// I - 1 = grayscale, 0 = color
		   const char *header)	// I - First 16 bytes of file
{
  if (memcmp(header, "\377\330\377", 3) == 0)
    return (new hdJPEGImage(p, gs));
  else
    return (NULL);
}


//
// 'hdJPEGImage::load()' - Load a JPEG image...
//

int					// O - 0 on success, -1 on failure
hdJPEGImage::load()
{
  return (real_load(1, depth() == 1));
}


//
// 'hdJPEGImage::real_load()' - Really load a JPEG image.
//

int					// O - 0 = success, -1 = fail
hdJPEGImage::real_load(int  img,	// I - Load image data?
                       bool gs)		// I - 0 = color, 1 = grayscale
{
  hdFile		*fp;		// File to read from
  jpeg_decompress_struct cinfo;		// Decompressor info
  jpg_source		*src;		// Source manager
  jpeg_error_mgr	err;		// Error handler info
  JSAMPROW		row;		// Sample row pointer


  // Open the file...
  if ((fp = hdFile::open(uri(), HD_FILE_READ)) == NULL)
    return (-1);

  // Setup the JPEG decompressor...
  jpeg_create_decompress(&cinfo);

  // Setup the error manager...
  jpeg_std_error(&err);
  err.error_exit = jpg_error;
  cinfo.err = &err;

  // Setup the source manager...
  src = new jpg_source(&cinfo, fp);

  // Read the JPEG file header...
  jpeg_read_header(&cinfo, 1);

  cinfo.quantize_colors = 0;

  if (gs || cinfo.num_components == 1)
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

  set_size(cinfo.output_width, cinfo.output_height, cinfo.output_components);

  if (!img)
  {
    jpeg_destroy_decompress(&cinfo);
    delete src;
    delete fp;

    return (0);
  }

  alloc_pixels();

  jpeg_start_decompress(&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(pixels() +
                     cinfo.output_scanline * cinfo.output_width *
                     cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  delete src;
  delete fp;

  return (0);
}


//
// 'jpg_error()' - Handle JPEG errors by not exiting.
//

static void
jpg_error(j_common_ptr)			// I - JPEG data (ignored)
{
  return;
}


//
// 'jpg_init()' - Initialize the JPEG source.
//

static void
jpg_init(j_decompress_ptr)		// I - Decompressor data (ignored)
{
}


//
// 'jpg_fill()' - Fill the JPEG source buffer.
//

static boolean
jpg_fill(j_decompress_ptr cinfo)	// I - Decompressor data
{
  jpg_source	*src = (jpg_source *)(cinfo->src);
					// Source manager pointer
  int		nbytes;			// Number of bytes read...


  nbytes = src->fp_->read(src->buffer_, sizeof(src->buffer_));
  if (nbytes <= 0)
  {
    // Insert a fake EOI marker...
    src->buffer_[0] = 0xff;
    src->buffer_[1] = JPEG_EOI;
    nbytes = 2;
  }

  src->pub_.next_input_byte = src->buffer_;
  src->pub_.bytes_in_buffer = nbytes;

  return (TRUE);
}


//
// 'jpg_skip()' - Skip data from a JPEG source.
//

static void
jpg_skip(j_decompress_ptr cinfo,	// I - Decompressor data
         long             num_bytes)	// I - Number of bytes to skip
{
  jpg_source	*src = (jpg_source *)(cinfo->src);
					// Source manager pointer


  // Skip bytes in the JPEG source buffer...
  if (num_bytes > 0)
  {
    while (num_bytes > (long)src->pub_.bytes_in_buffer)
    {
      num_bytes -= (long)src->pub_.bytes_in_buffer;
      jpg_fill(cinfo);
    }

    src->pub_.next_input_byte += (size_t)num_bytes;
    src->pub_.bytes_in_buffer -= (size_t)num_bytes;
  }
}


//
// 'jpg_term()' - Terminate reading from a file.
//

static void
jpg_term(j_decompress_ptr)		// I - Decompressor data (ignored)
{
}


//
// 'jpg_source::jpg_source()' - Create a JPEG source mananger.
//

jpg_source::jpg_source(j_decompress_ptr cinfo,	// I - Decompressor data
                       hdFile           *fp)	// I - File to read from
{
  cinfo->src = (jpeg_source_mgr *)this;

  pub_.init_source       = jpg_init;
  pub_.fill_input_buffer = jpg_fill;
  pub_.skip_input_data   = jpg_skip;
  pub_.resync_to_restart = jpeg_resync_to_restart;
  pub_.term_source       = jpg_term;
  pub_.bytes_in_buffer   = 0;
  pub_.next_input_byte   = (JOCTET *)0;

  fp_                    = fp;
}


//
// End of "$Id$".
//
