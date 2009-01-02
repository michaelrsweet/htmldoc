//
// "$Id$"
//
//   Flate filter functions for HTMLDOC.
//
//   Copyright 1997-2009 Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: HTMLDOC Licensing Information
//       Easy Software Products
//       516 Rio Grand Ct
//       Morgan Hill, CA 95037 USA
//
//       http://www.htmldoc.org/
//
// Contents:
//
//   hdFlateFilter::hdFlateFilter()  - Construct a Flate filter.
//   hdFlateFilter::~hdFlateFilter() - Destroy a Flate filter.
//   hdFlateFilter::get()            - Get a character (not implemented)
//   hdFlateFilter::put()            - Put a single character to the filter.
//   hdFlateFilter::read()           - Read bytes (not implemented)
//   hdFlateFilter::seek()           - See in the file (not implemented)
//   hdFlateFilter::size()           - Return the size of the file.
//   hdFlateFilter::write()          - Write bytes.
//   hdFlateFilter::unget()          - Un-get a character (not supported)
//

//
// Include necessary headers...
//

#include "file.h"
#include "progress.h"


//
// 'hdFlateFilter::hdFlateFilter()' - Construct a Flate filter.
//

hdFlateFilter::hdFlateFilter(hdFile *f,	// I - File or filter
                             int    level)
					// I - Compression level
{
  // Chain to the next file/filter...
  chain_ = f;

  // Initialize the Flate compression stream...
  stream_.zalloc = (alloc_func)0;
  stream_.zfree  = (free_func)0;
  stream_.opaque = (voidpf)0;

  deflateInit(&stream_, level);

  // Start at the beginning of the buffer...
  stream_.next_out  = (Bytef *)buffer_;
  stream_.avail_out = sizeof(buffer_);
}


//
// 'hdFlateFilter::~hdFlateFilter()' - Destroy a Flate filter.
//

hdFlateFilter::~hdFlateFilter()
{
  int	status;				// Deflate status


  // Make sure all compressed data is written...
  stream_.avail_in = 0;
  while ((status = deflate(&stream_, Z_FINISH)) != Z_STREAM_END)
  {
    if (status < Z_OK && status != Z_BUF_ERROR)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY, "deflate() failed (%d)", status);
      break;
    }

    chain_->write(buffer_, (char *)stream_.next_out - buffer_);

    stream_.next_out  = (Bytef *)buffer_;
    stream_.avail_out = sizeof(buffer_);
  }

  if ((char *)stream_.next_out > buffer_)
    chain_->write(buffer_, (char *)stream_.next_out - buffer_);

  // Free all memory used by the compression stream...
  deflateEnd(&stream_);
}


//
// 'hdFlateFilter::get()' - Get a character (not implemented)
//

int					// O - -1 for error/not implemented
hdFlateFilter::get()
{
  return (-1);
}


//
// 'hdFlateFilter::put()' - Put a single character to the filter.
//

int					// O - -1 on error, 0 on success
hdFlateFilter::put(int c)		// I - Character to put
{
  char	in[1];				// Input array for compression...


  in[0] = c;

  if (write(in, 1) != 1)
    return (-1);
  else
    return (c);
}


//
// 'hdFlateFilter::read()' - Read bytes (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdFlateFilter::read(void *,		// I - Bytes to read
                    size_t)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdFlateFilter::seek()' - See in the file (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdFlateFilter::seek(ssize_t,		// I - Position or offset
                    int)		// I - Whence to seek from
{
  return (-1);
}


//
// 'hdFlateFilter::size()' - Return the size of the file.
//

size_t					// O - Size of file in bytes
hdFlateFilter::size()
{
  return (chain_->size());
}


//
// 'hdFlateFilter::write()' - Write bytes.
//

ssize_t					// O - Number of bytes written
hdFlateFilter::write(const void *b,	// I - Buffer to write
                     size_t     len)	// I - Number of bytes to write
{
  int	status;				// Deflate status


  // Loop until all bytes are compressed...
  stream_.next_in  = (Bytef *)b;
  stream_.avail_in = len;

  while (stream_.avail_in > 0)
  {
    if (stream_.avail_out < (int)(sizeof(buffer_) / 8))
    {
      if (chain_->write(buffer_, (char *)stream_.next_out - buffer_) < 0)
        return (-1);

      stream_.next_out  = (Bytef *)buffer_;
      stream_.avail_out = sizeof(buffer_);
    }

    status = deflate(&stream_, Z_NO_FLUSH);

    if (status < Z_OK && status != Z_BUF_ERROR)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY, "deflate() failed (%d)", status);
      return (-1);
    }
  }

  return (len);
}


//
// 'hdFlateFilter::unget()' - Un-get a character (not supported)
//

int					// O - -1 on error (not supported)
hdFlateFilter::unget(int c)		// I - Character to unget
{
  return (-1);
}


//
// End of "$Id$".
//
