//
// "$Id: file-ascii.cxx,v 1.3 2004/03/08 01:01:41 mike Exp $"
//
//   ASCII filter functions for HTMLDOC.
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
//   hdASCII85Filter::hdASCII85Filter()    - Construct a ASCII85 filter.
//   hdASCII85Filter::~hdASCII85Filter()   - Destroy a ASCII85 filter.
//   hdASCII85Filter::get()                - Get a character (not implemented)
//   hdASCII85Filter::put()                - Put a single character to the filter.
//   hdASCII85Filter::read()               - Read bytes (not implemented)
//   hdASCII85Filter::seek()               - See in the file (not implemented)
//   hdASCII85Filter::size()               - Return the size of the file.
//   hdASCII85Filter::write()              - Write bytes.
//   hdASCII85Filter::unget()              - Un-get a character (not supported)
//   hdASCIIHexFilter::hdASCIIHexFilter()  - Construct a ASCIIHex filter.
//   hdASCIIHexFilter::~hdASCIIHexFilter() - Destroy a ASCIIHex filter.
//   hdASCIIHexFilter::get()               - Get a character (not implemented)
//   hdASCIIHexFilter::put()               - Put a single character to the filter.
//   hdASCIIHexFilter::read()              - Read bytes (not implemented)
//   hdASCIIHexFilter::seek()              - See in the file (not implemented)
//   hdASCIIHexFilter::size()              - Return the size of the file.
//   hdASCIIHexFilter::write()             - Write bytes.
//   hdASCIIHexFilter::unget()             - Un-get a character (not supported)
//

//
// Include necessary headers...
//

#include "file.h"
#include "hdstring.h"


//
// Maximum number of columns of chars; 255 is the max allowed by Adobe,
// 80 looks nice on the screen...
//

//#define HD_MAX_ASCII	255
#define HD_MAX_ASCII	80


//
// 'hdASCII85Filter::hdASCII85Filter()' - Construct a ASCII85 filter.
//

hdASCII85Filter::hdASCII85Filter(hdFile *f)
					// I - File or filter
{
  // Chain to the next file/filter...
  chain_ = f;

  column_  = 0;
  bufused_ = 0;
}


//
// 'hdASCII85Filter::~hdASCII85Filter()' - Destroy a ASCII85 filter.
//

hdASCII85Filter::~hdASCII85Filter()
{
  unsigned	val;			// 32-bit value of bytes
  char		c[5];			// Characters for output


  // Add a newline as needed...
  if (column_ >= (HD_MAX_ASCII - 1 - bufused_))
  {
    chain_->put('\n');
    column_ = 0;
  }

  // Clear remaining bytes from the buffer...
  memset(buffer_ + bufused_, 0, 4 - bufused_);

  // Get 32 bits from the buffer
  val = (((((buffer_[0] << 8) | buffer_[1]) << 8) | buffer_[2]) << 8) | buffer_[3];

  // Base-85 encode the 32-bit number...
  c[4] = (val % 85) + '!';
  val /= 85;
  c[3] = (val % 85) + '!';
  val /= 85;
  c[2] = (val % 85) + '!';
  val /= 85;
  c[1] = (val % 85) + '!';
  val /= 85;
  c[0] = val + '!';

  // Write the significant bytes of the string...
  chain_->write(c, bufused_ + 1);

  column_ += bufused_ + 1;

  // End with ~>, but make sure the line length is < 255...
  if (column_ >= 253)
    chain_->put('\n');

  chain_->write("~>\n", 3);
}


//
// 'hdASCII85Filter::get()' - Get a character (not implemented)
//

int					// O - -1 for error/not implemented
hdASCII85Filter::get()
{
  return (-1);
}


//
// 'hdASCII85Filter::put()' - Put a single character to the filter.
//

int					// O - -1 on error, 0 on success
hdASCII85Filter::put(int c)		// I - Character to put
{
  char	in[1];				// Input array for output...


  in[0] = c;

  if (write(in, 1) != 1)
    return (-1);
  else
    return (c);
}


//
// 'hdASCII85Filter::read()' - Read bytes (not implemented)
//

int					// O - -1 for error (not implemented)
hdASCII85Filter::read(void *,		// I - Bytes to read
                      int)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdASCII85Filter::seek()' - See in the file (not implemented)
//

int					// O - -1 for error (not implemented)
hdASCII85Filter::seek(long,		// I - Position or offset
                      int)		// I - Whence to seek from
{
  return (-1);
}


//
// 'hdASCII85Filter::size()' - Return the size of the file.
//

long					// O - Size of file in bytes
hdASCII85Filter::size()
{
  return (chain_->size());
}


//
// 'hdASCII85Filter::write()' - Write bytes.
//

int					// O - Number of bytes written
hdASCII85Filter::write(const void *b,	// I - Buffer to write
                       int        len)	// I - Number of bytes to write
{
  int			i;		// Looping var
  const unsigned char	*ptr,		// Pointer in buffer
			*temp;		// Pointer to values
  unsigned		val;		// 32-bit value of bytes
  char			c[5];		// Characters for output


  // Loop through the output buffer, 4 bytes at a time...
  for (i = len + bufused_, ptr = (const unsigned char *)b;
       i > 3;
       i -= 4, ptr += 4)
  {
    if (bufused_)
    {
      // Use leftover bits from last write...
      memcpy(buffer_ + bufused_, ptr, 4 - bufused_);
      temp     = buffer_;
      ptr      -= bufused_;
      bufused_ = 0;
    }
    else
    {
      // Use only what is in the incoming buffer...
      temp = ptr;
    }

    val = (((((temp[0] << 8) | temp[1]) << 8) | temp[2]) << 8) | temp[3];

    if (val == 0)
    {
      // Add a newline as needed...
      if (column_ >= (HD_MAX_ASCII - 1))
      {
	chain_->put('\n');
	column_ = 0;
      }

      chain_->put('z');
      column_ ++;
    }
    else
    {
      // Add a newline as needed...
      if (column_ >= (HD_MAX_ASCII - 5))
      {
	chain_->put('\n');
	column_ = 0;
      }

      c[4] = (val % 85) + '!';
      val /= 85;
      c[3] = (val % 85) + '!';
      val /= 85;
      c[2] = (val % 85) + '!';
      val /= 85;
      c[1] = (val % 85) + '!';
      val /= 85;
      c[0] = val + '!';

      chain_->write(c, 5);
      column_ += 5;
    }
  }

  if (i > 0)
  {
    // Save remaining bits in the local buffer for later use...
    memcpy(buffer_, ptr, i);
    bufused_ = i;
  }

  return (len);
}


//
// 'hdASCII85Filter::unget()' - Un-get a character (not supported)
//

int					// O - -1 on error (not supported)
hdASCII85Filter::unget(int c)		// I - Character to unget
{
  return (-1);
}


//
// 'hdASCIIHexFilter::hdASCIIHexFilter()' - Construct a ASCIIHex filter.
//

hdASCIIHexFilter::hdASCIIHexFilter(hdFile *f)
					// I - File or filter
{
  // Chain to the next file/filter...
  chain_  = f;
  column_ = 0;
}


//
// 'hdASCIIHexFilter::~hdASCIIHexFilter()' - Destroy a ASCIIHex filter.
//

hdASCIIHexFilter::~hdASCIIHexFilter()
{
  // Make sure the hex stream ends with a newline as needed...
  if (column_)
    chain_->put('\n');
}


//
// 'hdASCIIHexFilter::get()' - Get a character (not implemented)
//

int					// O - -1 for error/not implemented
hdASCIIHexFilter::get()
{
  return (-1);
}


//
// 'hdASCIIHexFilter::put()' - Put a single character to the filter.
//

int					// O - -1 on error, 0 on success
hdASCIIHexFilter::put(int c)		// I - Character to put
{
  // Write a single hex number, and add a newline as needed...
  if (column_ >= (HD_MAX_ASCII - 1))
  {
    chain_->put('\n');
    column_ = 0;
  }

  chain_->printf("%02X", c & 255);
  column_ += 2;

  return (c);
}


//
// 'hdASCIIHexFilter::read()' - Read bytes (not implemented)
//

int					// O - -1 for error (not implemented)
hdASCIIHexFilter::read(void *,		// I - Bytes to read
                       int)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdASCIIHexFilter::seek()' - See in the file (not implemented)
//

int					// O - -1 for error (not implemented)
hdASCIIHexFilter::seek(long,		// I - Position or offset
                       int)		// I - Whence to seek from
{
  return (-1);
}


//
// 'hdASCIIHexFilter::size()' - Return the size of the file.
//

long					// O - Size of file in bytes
hdASCIIHexFilter::size()
{
  return (chain_->size());
}


//
// 'hdASCIIHexFilter::write()' - Write bytes.
//

int					// O - Number of bytes written
hdASCIIHexFilter::write(const void *b,	// I - Buffer to write
                        int        len)	// I - Number of bytes to write
{
  int			i;		// Looping var
  const unsigned char	*ptr;		// Pointer to current byte
  static const char	*hex = "0123456789ABCDEF";
					// Hex digits...


  for (i = len, ptr = (const unsigned char *)b; i > 0; i --, ptr ++)
  {
    // Add a newline as needed to keep the line length below 255 chars
    // (Adobe DSC requirement)
    if (column_ >= (HD_MAX_ASCII - 1))
    {
      chain_->put('\n');
      column_ = 0;
    }

    // Put the hex uchars out to the file; note that we don't use printf()
    // for speed reasons...
    chain_->put(hex[*ptr >> 4]);
    chain_->put(hex[*ptr & 15]);

    column_ += 2;
  }

  return (len);
}


//
// 'hdASCIIHexFilter::unget()' - Un-get a character (not supported)
//

int					// O - -1 on error (not supported)
hdASCIIHexFilter::unget(int c)		// I - Character to unget
{
  return (-1);
}


//
// End of "$Id: file-ascii.cxx,v 1.3 2004/03/08 01:01:41 mike Exp $".
//
