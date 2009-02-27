//
// "$Id$"
//
//   Stdio file routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2008 Easy Software Products.
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
//   hdStdFile::hdStdFile()  - Open a named file.
//   hdStdFile::hdStdFile()  - Open using an existing file pointer.
//   hdStdFile::hdStdFile()  - Open using an existing file descriptor.
//   hdStdFile::~hdStdFile() - Close a file.
//   hdStdFile::get()        - Get a single character from a file.
//   hdStdFile::put()        - Put a single character to a file.
//   hdStdFile::read()       - Read data from a file.
//   hdStdFile::seek()       - Seek to a position in the file.
//   hdStdFile::size()       - Return the total size of the file.
//   hdStdFile::write()      - Write data to a file.
//   hdStdFile::unget()      - Un-get a character in a file.
//

//
// Include necessary headers.
//

#include "file.h"
#include "hdstring.h"


//
// 'hdStdFile::hdStdFile()' - Open a named file.
//

hdStdFile::hdStdFile(const char *name,	// I - Name of file to open
                     hdMode     m)	// I - Open mode
{
  mode(m);
  pos(0);
  error(0);

  size_ = 0;

  switch (m)
  {
    case HD_FILE_READ :
        fp_ = fopen(name, "rb");
	break;
    case HD_FILE_WRITE :
        fp_ = fopen(name, "wb");
	break;
    case HD_FILE_UPDATE :
        fp_ = fopen(name, "w+b");
	break;
  }

  if (!fp_)
    error(errno);
  else if (m != HD_FILE_READ)
    setbuf(fp_, NULL);
}


//
// 'hdStdFile::hdStdFile()' - Open using an existing file pointer.
//

hdStdFile::hdStdFile(FILE   *f,		// I - File pointer
                     hdMode m)		// I - Open mode
{
  mode(m);
  pos(f ? ftell(f) : 0);
  error(0);

  size_ = 0;
  fp_   = f;

  if (!fp_)
    error(errno);
}


//
// 'hdStdFile::hdStdFile()' - Open using an existing file descriptor.
//

hdStdFile::hdStdFile(int    fd,		// I - File descriptor
                     hdMode m)		// I - Open mode
{
  mode(m);
  pos(0);
  error(0);

  size_ = 0;

  switch (m)
  {
    case HD_FILE_READ :
        fp_ = fdopen(fd, "rb");
	break;
    case HD_FILE_WRITE :
        fp_ = fdopen(fd, "wb");
	break;
    case HD_FILE_UPDATE :
        fp_ = fdopen(fd, "w+b");
	break;
  }

  if (!fp_)
    error(errno);
}


//
// 'hdStdFile::~hdStdFile()' - Close a file.
//

hdStdFile::~hdStdFile()
{
  if (fp_ && fp_ != stdin && fp_ != stdout && fp_ != stderr)
    fclose(fp_);
}


//
// 'hdStdFile::get()' - Get a single character from a file.
//

int					// O - Character or -1 on error
hdStdFile::get()
{
  int c;				// Character from file


  if (!fp_)
    return (-1);

  if ((c = getc(fp_)) < 0)
  {
    error(ferror(fp_));
    return (-1);
  }

  pos(pos() + 1);

  return (c);
}


//
// 'hdStdFile::put()' - Put a single character to a file.
//

int					// O - 0 on success, -1 on error
hdStdFile::put(int c)			// I - Character to put
{
  if (!fp_)
    return (-1);

  if (putc(c, fp_) < 0)
  {
    error(ferror(fp_));
    return (-1);
  }

  pos(pos() + 1);

  if (pos() > size_)
    size_ = pos();

  return (0);
}


//
// 'hdStdFile::read()' - Read data from a file.
//

ssize_t					// O - Number of bytes read
hdStdFile::read(void    *b,		// O - Buffer to read into
                size_t  len)		// I - Number of bytes to read
{
  ssize_t	bytes;			// Number of bytes read


  if (!fp_)
    return (-1);

  if ((bytes = fread(b, 1, len, fp_)) < 1)
  {
    error(ferror(fp_));
    return (-1);
  }

  pos(pos() + bytes);

  return (bytes);
}


//
// 'hdStdFile::seek()' - Seek to a position in the file.
//

ssize_t					// O - File position on success, -1 on error
hdStdFile::seek(ssize_t p,		// I - Position
                int     w)		// I - Where to seek from
{
  if (!fp_)
    return (-1);

  if (mode() != HD_FILE_READ)
    fflush(fp_);

  if (fseek(fp_, p, w))
  {
    error(ferror(fp_));
    return (-1);
  }

  pos(ftell(fp_));

  return (pos());
}


//
// 'hdStdFile::size()' - Return the total size of the file.
//

size_t					// O - Size of file in bytes
hdStdFile::size()
{
  if (!fp_)
    return (0);
  else
    return (size_);
}


//
// 'hdStdFile::write()' - Write data to a file.
//

ssize_t					// O - Number of bytes written
hdStdFile::write(const void *b,		// I - Buffer to write
                 size_t     len)	// I - Number of bytes to write
{
  ssize_t	bytes;			// Number of bytes written


  if (!fp_)
    return (-1);

  if ((bytes = fwrite(b, 1, len, fp_)) < 1)
  {
    error(ferror(fp_));
    return (-1);
  }

  pos(pos() + bytes);

  if (pos() > size_)
    size_ = pos();

  return (bytes);
}


//
// 'hdStdFile::unget()' - Un-get a character in a file.
//

int					// O - Character pushed back or -1
hdStdFile::unget(int c)			// I - Character to push back
{
  if (!fp_)
    return (-1);

  if (ungetc(c, fp_) != c)
  {
    error(ferror(fp_));
    return (-1);
  }
  else
    return (c);
}


//
// End of "$Id$".
//
