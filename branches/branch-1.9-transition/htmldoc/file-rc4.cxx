//
// "$Id: file-rc4.cxx,v 1.3 2004/03/08 01:01:41 mike Exp $"
//
//   RC4 filter functions for HTMLDOC.
//
//   Copyright 2001-2004 by Easy Software Products
//
//   Original code by Tim Martin
//   Copyright 1999 by Carnegie Mellon University, All Rights Reserved
//
//   Permission to use, copy, modify, and distribute this software and its
//   documentation for any purpose and without fee is hereby granted,
//   provided that the above copyright notice appear in all copies and that
//   both that copyright notice and this permission notice appear in
//   supporting documentation, and that the name of Carnegie Mellon
//   University not be used in advertising or publicity pertaining to
//   distribution of the software without specific, written prior
//   permission.
//
//   CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
//   THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
//   FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR
//   ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
//   OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

hdRC4Filter::hdRC4Filter(hdFile              *f,
					// I - File or filter
                         const unsigned char *key,
					// I - Key
			 unsigned            keylen)
					// I - Length of key
{
  chain_ = f;

  init(key, keylen);
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
  unsigned char	in[1];			// Input array for encryption...


  in[0] = (unsigned)c;

  encrypt(in, buffer_, 1);

  return (chain_->put(buffer_[0]));
}


//
// 'hdRC4Filter::read()' - Read bytes (not implemented)
//

int					// O - -1 for error (not implemented)
hdRC4Filter::read(void *,		// I - Bytes to read
                  int)			// I - Number of bytes to read
{
  return (-1);
}


//
// 'hdRC4Filter::seek()' - See in the file (not implemented)
//

int					// O - -1 for error (not implemented)
hdRC4Filter::seek(long,			// I - Position or offset
                  int)			// I - Whence to seek from
{
  return (-1);
}


//
// 'hdRC4Filter::size()' - Return the size of the file.
//

long					// O - Size of file in bytes
hdRC4Filter::size()
{
  return (chain_->size());
}


//
// 'hdRC4Filter::write()' - Write bytes.
//

int					// O - Number of bytes written
hdRC4Filter::write(const void *b,	// I - Buffer to write
                   int        len)	// I - Number of bytes to write
{
  const unsigned char	*in;		// Input pointer for encryption...
  unsigned		bytes;		// Bytes to encrypt/write
  int			total;		// Total bytes written


  // Encrypt the entire output array, breaking up into buffer-sized
  // chunks as needed...
  for (total = 0, in = (const unsigned char *)b;
       total < len;
       total += (int)bytes, in += bytes)
  {
    if ((bytes = len - total) > sizeof(buffer_))
      bytes = sizeof(buffer_);

    encrypt(in, buffer_, 1);

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
// 'hdRC4Filter::init()' - Initialize the RC4 filter with the specified key.
//

void
hdRC4Filter::init(const unsigned char *key,
					// I - Key
                  unsigned            keylen)
					// I - Length of key
{
  int		i, j;			// Looping vars
  unsigned char	tmp;			// Temporary variable


  // Fill in linearly s0=0, s1=1, ...
  for (i = 0; i < 256; i ++)
    sbox_[i] = i;

  for (i = 0, j = 0; i < 256; i ++)
  {
    // j = (j + Si + Ki) mod 256
    j = (j + sbox_[i] + key[i % keylen]) & 255;

    // Swap Si and Sj...
    tmp     = sbox_[i];
    sbox_[i] = sbox_[j];
    sbox_[j] = tmp;
  }

  // Initialized counters to 0 and return...
  si_ = 0;
  sj_ = 0;
}


//
// 'hdRC4Filter::encrypt()' - Encrypt the given buffer.
//

void
hdRC4Filter::encrypt(const unsigned char *input,
					// I - Input buffer
	             unsigned char       *output,
					// O - Output buffer
	             unsigned            len)
					// I - Size of buffers
{
  int		i, j;			// Looping vars
  unsigned char	tmp;			// Swap variable
  int		t;			// Current S box


  // Loop through the entire buffer...
  i = si_;
  j = sj_;

  while (len > 0)
  {
    // Get the next S box indices...
    i = (i + 1) & 255;
    j = (j + sbox_[i]) & 255;

    // Swap Si and Sj...
    tmp      = sbox_[i];
    sbox_[i] = sbox_[j];
    sbox_[j] = tmp;

    // Get the S box index for this byte...
    t = (sbox_[i] + sbox_[j]) & 255;

    // Encrypt using the S box...
    *output++ = *input++ ^ sbox_[t];
    len --;
  }

  // Copy current S box indices back to context...
  si_ = i;
  sj_ = j;
}


//
// End of "$Id: file-rc4.cxx,v 1.3 2004/03/08 01:01:41 mike Exp $".
//
