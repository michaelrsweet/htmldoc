//
// "$Id: file-mem.cxx,v 1.3 2004/02/03 02:55:28 mike Exp $"
//
//   Memory buffer file routines for HTMLDOC, a HTML document processing
//   program.
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
//   hdMemFile::hdMemFile()  - Open an existing memory buffer.
//   hdMemFile::hdMemFile()  - Open a new memory buffer.
//   hdMemFile::~hdMemFile() - Close a memory file.
//   hdMemFile::get()        - Get a single character from a file.
//   hdMemFile::put()        - Put a single character to a file.
//   hdMemFile::read()       - Read data from a file.
//   hdMemFile::seek()       - Seek to a position in the file.
//   hdMemFile::size()       - Return the total size of the file.
//   hdMemFile::write()      - Write data to a file.
//   hdMemFile::unget()      - Un-get a character in a file.
//

//
// Include necessary headers.
//

#include "file.h"
#include "hdstring.h"


//
// 'hdMemFile::hdMemFile()' - Open an existing memory buffer.
//

hdMemFile::hdMemFile(char   *buffer,	// I - Memory buffer
                     long   buflen,	// I - Number of bytes in buffer
                     hdMode m)		// I - Open mode
{
  mode(m);
  pos(0);
  error(0);

  if (buflen == 0)
    size_ = strlen(buffer);
  else
    size_ = buflen;

  buffer_     = buffer;
  end_        = buffer_ + size_;
  current_    = buffer_;

  alloc_size_ = 0;
  max_size_   = 0;
  incr_size_  = 0;
}


//
// 'hdMemFile::hdMemFile()' - Open a new memory buffer.
//

hdMemFile::hdMemFile(long   init_size,	// I - Initial size of buffer
                     hdMode m,		// I - Open mode
		     long   max_size,	// I - Maximum size of buffer
		     long   incr_size)	// I - Reallocation increment
{
  mode(m);
  pos(0);
  error(0);

  size_       = 0;

  buffer_     = new char[init_size];
  end_        = buffer_ + init_size;
  current_    = buffer_;

  alloc_size_ = init_size;
  max_size_   = max_size;
  incr_size_  = incr_size ? incr_size : init_size;
}


//
// 'hdMemFile::~hdMemFile()' - Close a memory file.
//

hdMemFile::~hdMemFile()
{
  if (alloc_size_)
    delete[] buffer_;
}


//
// 'hdMemFile::get()' - Get a single character from a file.
//

int					// O - Character or -1 on error
hdMemFile::get()
{
  int c;				// Character from file


  if (current_ < (buffer_ + size_))
  {
    c = *current_++;
    pos(pos() + 1);
  }
  else
    c = EOF;

  return (c);
}


//
// 'hdMemFile::put()' - Put a single character to a file.
//

int					// O - 0 on success, -1 on error
hdMemFile::put(int c)			// I - Character to put
{
  if (buffer_ >= end_)
  {
    if (alloc_size_ && (alloc_size_ < max_size_ || max_size_ == 0))
    {
      // Realloc the memory buffer...
      char	*temp;		// New buffer
      long	tempsize;	// New buffer size


      tempsize = alloc_size_ + incr_size_;
      if (tempsize > max_size_ && max_size_ > 0)
        tempsize = max_size_;

      temp = new char[tempsize];

      memcpy(temp, buffer_, alloc_size_);

      delete[] buffer_;

      alloc_size_ = tempsize;
      buffer_     = temp;
      end_        = buffer_ + alloc_size_;
      current_    = buffer_ + pos();
    }
    else
      return (-1);
  }

  *current_++ = c;

  pos(pos() + 1);

  if (pos() > size_)
    size_ = pos();

  return (0);
}


//
// 'hdMemFile::read()' - Read data from a file.
//

int					// O - Number of bytes read
hdMemFile::read(void *b,		// O - Buffer to read into
                int  len)		// I - Number of bytes to read
{
  int	bytes;				// Number of bytes read


  if (len > (size_ - pos()))
    bytes = size_ - pos();
  else
    bytes = len;

  if (len == 0)
    return (0);

  memcpy(b, current_, bytes);
  current_ += bytes;

  pos(pos() + bytes);

  return (bytes);
}


//
// 'hdMemFile::seek()' - Seek to a position in the file.
//

int					// O - 0 on success, -1 on error
hdMemFile::seek(long p,			// I - Position
                int  w)			// I - Where to seek from
{
  switch (w)
  {
    case 0 : // Seek from start
	break;

    case 1 : // Seek from current
        p += pos();
	break;

    case 2 : // Seek from end
        p = size_ - p;
	break;
  }

  if (p < 0)
    pos(0);
  else if (p > size_)
    pos(size_);
  else
    pos(p);

  current_ = buffer_ + pos();

  return (0);
}


//
// 'hdMemFile::size()' - Return the total size of the file.
//

long					// O - Size of file in bytes
hdMemFile::size()
{
  return (size_);
}


//
// 'hdMemFile::write()' - Write data to a file.
//

int					// O - Number of bytes written
hdMemFile::write(const void *b,		// I - Buffer to write
                 int        len)	// I - Number of bytes to write
{
  int	bytes;				// Number of bytes written


  if (alloc_size_)
  {
    // Figure out the number of bytes we can write to this buffer...
    bytes = alloc_size_ - pos();

    if (bytes < len && (alloc_size_ < max_size_ || max_size_ == 0))
    {
      // Realloc the memory buffer...
      char	*temp;		// New buffer
      long	tempsize;	// New buffer size


      do
      {
        tempsize = alloc_size_ + incr_size_;
        if (tempsize > max_size_ && max_size_ > 0)
          tempsize = max_size_;

	bytes = tempsize - pos();
      }
      while (bytes < len && (tempsize < max_size_ || max_size_ == 0));

      temp = new char[tempsize];

      memcpy(temp, buffer_, alloc_size_);

      delete[] buffer_;

      alloc_size_ = tempsize;
      buffer_     = temp;
      end_        = buffer_ + alloc_size_;
      current_    = buffer_ + pos();
    }
  }
  else
  {
    // Figure out the number of bytes left in the static buffer...
    bytes = size_ - pos();
  }

  if (bytes <= 0)
    return (-1);

  if (bytes > len)
    bytes = len;

  memcpy(current_, b, bytes);

  current_ += bytes;

  pos(pos() + bytes);

  if (pos() > size_)
    size_ = pos();

  return (bytes);
}


//
// 'hdMemFile::unget()' - Un-get a character in a file.
//

int					// O - Character pushed back or -1
hdMemFile::unget(int c)			// I - Character to push back
{
  if (current_ > buffer_)
  {
    current_ --;
    pos(pos() - 1);

    if (mode() != HD_FILE_READ)
      *current_ = c;

    return (c);
  }
  else
    return (-1);
}


//
// End of "$Id: file-mem.cxx,v 1.3 2004/02/03 02:55:28 mike Exp $".
//
