//
// "$Id$"
//
//   JPEG filter functions for HTMLDOC.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdJPEGFilter::hdJPEGFilter()  - Construct a JPEG filter.
//   hdJPEGFilter::~hdJPEGFilter() - Destroy a JPEG filter.
//   hdJPEGFilter::get()           - Get a character (not implemented)
//   hdJPEGFilter::put()           - Put a single character (not implemented)
//   hdJPEGFilter::read()          - Read bytes (not implemented)
//   hdJPEGFilter::seek()          - See in the file (not implemented)
//   hdJPEGFilter::size()          - Return the size of the file.
//   hdJPEGFilter::write()         - Write bytes.
//   hdJPEGFilter::unget()         - Un-get a character (not supported)
//   hdJPEGFilter::init()          - Initialize the JPEG destination.
//   hdJPEGFilter::empty()         - Empty the JPEG output buffer.
//   hdJPEGFilter::term()          - Write the last JPEG data to the file.
//

//
// Include necessary headers...
//

#include "file.h"


//
// 'hdJPEGFilter::hdJPEGFilter()' - Construct a JPEG filter.
//

hdJPEGFilter::hdJPEGFilter(hdFile *f,	// I - File or filter
                           int    width,// I - Width of data
			   int    height,
					// I - Height of data
			   int    depth,// I - Color depth of data
			   int    quality)
					// I - Output quality
{
  int	i;				// Looping var...


  // Chain to the next file/filter...
  chain_ = f;

  // The destination manager handles writing...
  dest_mgr_.pub_.init_destination    = init;
  dest_mgr_.pub_.empty_output_buffer = empty;
  dest_mgr_.pub_.term_destination    = term;
  dest_mgr_.jpeg_filter_             = this;

  // Initialize the compressor...
  cinfo_.err = jpeg_std_error(&error_mgr_);

  jpeg_create_compress(&cinfo_);

  cinfo_.dest             = (jpeg_destination_mgr *)&dest_mgr_;
  cinfo_.image_width      = width;
  cinfo_.image_height     = height;
  cinfo_.input_components = depth;
  cinfo_.in_color_space   = depth == 1 ? JCS_GRAYSCALE : JCS_RGB;

  jpeg_set_defaults(&cinfo_);
  jpeg_set_quality(&cinfo_, quality, TRUE);

  // Adobe uses sampling == 1
  for (i = 0; i < depth; i ++)
  {
    cinfo_.comp_info[i].h_samp_factor = 1;
    cinfo_.comp_info[i].v_samp_factor = 1;
  }

  // We're writing JPEG data for Adobe formats...
  cinfo_.write_JFIF_header  = FALSE;
  cinfo_.write_Adobe_marker = TRUE;

  // Start compression
  jpeg_start_compress(&cinfo_, TRUE);
}


//
// 'hdJPEGFilter::~hdJPEGFilter()' - Destroy a JPEG filter.
//

hdJPEGFilter::~hdJPEGFilter()
{
  jpeg_finish_compress(&cinfo_);
  jpeg_destroy_compress(&cinfo_);
}


//
// 'hdJPEGFilter::get()' - Get a character (not implemented)
//

int					// O - -1 for error/not implemented
hdJPEGFilter::get()
{
  return (-1);
}


//
// 'hdJPEGFilter::put()' - Put a single character (not implemented)
//

int					// O - -1 on error (not implemented)
hdJPEGFilter::put(int c)		// I - Character to put
{
  return (-1);
}


//
// 'hdJPEGFilter::read()' - Read bytes (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdJPEGFilter::read(void *,		// I - Bytes to read
                   size_t)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdJPEGFilter::seek()' - See in the file (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdJPEGFilter::seek(ssize_t,		// I - Position or offset
                   int)			// I - Whence to seek from
{
  return (-1);
}


//
// 'hdJPEGFilter::size()' - Return the size of the file.
//

size_t					// O - Size of file in bytes
hdJPEGFilter::size()
{
  return (chain_->size());
}


//
// 'hdJPEGFilter::write()' - Write bytes.
//

ssize_t					// O - Number of bytes written
hdJPEGFilter::write(const void *b,	// I - Buffer to write
                    size_t     len)	// I - Number of bytes to write
{
  int		i;			// Looping var
  int		line_size;		// Size of a line
  JSAMPLE	*pixel;			// Pointer to current pixel


  // Check that the number of bytes is a multiple of 1 line...
  line_size = cinfo_.image_width * cinfo_.input_components;
  i         = len / line_size;

  if (len != (size_t)(i * line_size))
    return (-1);

  // Loop through the array, writing 1 line at a time...
  for (pixel = (JSAMPLE *)b; i > 0; i --, pixel += line_size)
    jpeg_write_scanlines(&cinfo_, &pixel, 1);

  return (len);
}


//
// 'hdJPEGFilter::unget()' - Un-get a character (not supported)
//

int					// O - -1 on error (not supported)
hdJPEGFilter::unget(int c)		// I - Character to unget
{
  return (-1);
}


//
// 'hdJPEGFilter::init()' - Initialize the JPEG destination.
//

void
hdJPEGFilter::init(j_compress_ptr cinfo)// I - Compressor info
{
  hdJPEGFilter	*jf;			// JPEG filter


  // Grab the pointer to the real JPEG filter...
  jf = ((hdJPEGDest *)cinfo->dest)->jpeg_filter_;

  // Initialize things..
  cinfo->dest->next_output_byte = jf->buffer_;
  cinfo->dest->free_in_buffer   = sizeof(jf->buffer_);
}


//
// 'hdJPEGFilter::empty()' - Empty the JPEG output buffer.
//

boolean					// O - TRUE if buffer written OK
hdJPEGFilter::empty(j_compress_ptr cinfo)
					// I - Compressor info
{
  hdJPEGFilter	*jf;			// JPEG filter


  // Grab the pointer to the real JPEG filter...
  jf = ((hdJPEGDest *)cinfo->dest)->jpeg_filter_;

  if (jf->chain_->write(jf->buffer_, sizeof(jf->buffer_)) <
          (int)sizeof(jf->buffer_))
   return (FALSE);

  cinfo->dest->next_output_byte = jf->buffer_;
  cinfo->dest->free_in_buffer   = sizeof(jf->buffer_);

  return (TRUE);
}


//
// 'hdJPEGFilter::term()' - Write the last JPEG data to the file.
//

void
hdJPEGFilter::term(j_compress_ptr cinfo)// I - Compressor info
{
  int		nbytes;			// Number of bytes to write
  hdJPEGFilter	*jf;			// JPEG filter


  // Grab the pointer to the real JPEG filter...
  jf = ((hdJPEGDest *)cinfo->dest)->jpeg_filter_;

  // Figure out how many bytes to write...
  nbytes = sizeof(jf->buffer_) - cinfo->dest->free_in_buffer;

  // Write the bytes...
  jf->chain_->write(jf->buffer_, nbytes);
}


//
// End of "$Id$".
//
