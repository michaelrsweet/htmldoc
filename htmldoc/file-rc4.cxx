//
// "$Id$"
//
//   RC4 filter functions for HTMLDOC.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdRC4Filter::hdRC4Filter()  - Construct an RC4 encryption filter.
//   hdRC4Filter::~hdRC4Filter() - Destroy an RC4 encryption filter.
//   hdRC4Filter::get()          - Get a character (not implemented)
//   hdRC4Filter::put()          - Put a single character to the filter.
//   hdRC4Filter::read()         - Read bytes (not implemented)
//   hdRC4Filter::seek()         - See in the file (not implemented)
//   hdRC4Filter::size()         - Return the size of the file.
//   hdRC4Filter::write()        - Write bytes.
//   hdRC4Filter::unget()        - Un-get a character (not supported)
//   hdRC4Filter::init()         - Initialize the RC4 filter with the
//                                 specified key.
//   hdRC4Filter::encrypt()      - Encrypt the given buffer.
//

//
// Include necessary headers...
//

#include "file.h"


//
// 'hdRC4Filter::hdRC4Filter()' - Construct an RC4 encryption filter.
//

hdRC4Filter::hdRC4Filter(
    hdFile       *f,			// I - File or filter
    const hdByte *key,			// I - Key
    size_t       keylen)		// I - Length of key
  : rc4_(key, keylen)
{
  chain_ = f;

//  init(key, keylen);
}


//
// 'hdRC4Filter::~hdRC4Filter()' - Destroy an RC4 encryption filter.
//

hdRC4Filter::~hdRC4Filter()
{
  // Nothing to do...
}


//
// 'hdRC4Filter::get()' - Get a character (not implemented)
//

int					// O - -1 for error/not implemented
hdRC4Filter::get()
{
  return (-1);
}


//
// 'hdRC4Filter::put()' - Put a single character to the filter.
//

int					// O - -1 on error, 0 on success
hdRC4Filter::put(int c)			// I - Character to put
{
  hdByte	in[1];			// Input array for encryption...


  in[0] = (unsigned)c;

  rc4_.encrypt(in, buffer_, 1);

  return (chain_->put(buffer_[0]));
}


//
// 'hdRC4Filter::read()' - Read bytes (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdRC4Filter::read(void *,		// I - Bytes to read
                  size_t)		// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdRC4Filter::seek()' - See in the file (not implemented)
//

ssize_t					// O - -1 for error (not implemented)
hdRC4Filter::seek(ssize_t,		// I - Position or offset
                  int)			// I - Whence to seek from
{
  return (-1);
}


//
// 'hdRC4Filter::size()' - Return the size of the file.
//

size_t					// O - Size of file in bytes
hdRC4Filter::size()
{
  return (chain_->size());
}


//
// 'hdRC4Filter::write()' - Write bytes.
//

ssize_t					// O - Number of bytes written
hdRC4Filter::write(const void *b,	// I - Buffer to write
                   size_t     len)	// I - Number of bytes to write
{
  const hdByte	*in;			// Input pointer for encryption...
  size_t	bytes,			// Bytes to encrypt/write
		total;			// Total bytes written


  // Encrypt the entire output array, breaking up into buffer-sized
  // chunks as needed...
  for (total = 0, in = (const hdByte *)b;
       total < len;
       total += (int)bytes, in += bytes)
  {
    if ((bytes = len - total) > sizeof(buffer_))
      bytes = sizeof(buffer_);

    rc4_.encrypt(in, buffer_, 1);

    if (chain_->write(buffer_, bytes) < (int)bytes)
      return (-1);
  }

  return (len);
}


//
// 'hdRC4Filter::unget()' - Un-get a character (not supported)
//

int					// O - -1 on error (not supported)
hdRC4Filter::unget(int c)		// I - Character to unget
{
  return (-1);
}


//
// End of "$Id$".
//
