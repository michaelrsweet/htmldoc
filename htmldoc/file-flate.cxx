//
// "$Id: file-flate.cxx,v 1.3.2.1 2004/03/22 21:56:29 mike Exp $"
//
//   Flate filter functions for HTMLDOC.
//
//   Copyright 1997-2004 by Easy Software Products.
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
  // Make sure all compressed data is written...
  stream_.avail_in = 0;
  while (deflate(&stream_, Z_FINISH) != Z_STREAM_END)
  {
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

int					// O - -1 for error (not implemented)
hdFlateFilter::read(void *,		// I - Bytes to read
                    int)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdFlateFilter::seek()' - See in the file (not implemented)
//

int					// O - -1 for error (not implemented)
hdFlateFilter::seek(long,		// I - Position or offset
                    int)		// I - Whence to seek from
{
  return (-1);
}


//
// 'hdFlateFilter::size()' - Return the size of the file.
//

long					// O - Size of file in bytes
hdFlateFilter::size()
{
  return (chain_->size());
}


//
// 'hdFlateFilter::write()' - Write bytes.
//

int					// O - Number of bytes written
hdFlateFilter::write(const void *b,	// I - Buffer to write
                     int        len)	// I - Number of bytes to write
{
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

    deflate(&stream_, Z_NO_FLUSH);
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
// End of "$Id: file-flate.cxx,v 1.3.2.1 2004/03/22 21:56:29 mike Exp $".
//
