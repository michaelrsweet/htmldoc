//
// "$Id$"
//
//   RC4 code for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
//   Original code by Rob Earhart
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
//   hdRC4::encrypt() - Encrypt the given buffer.
//   hdRC4::init()    - Initialize the RC4 context with the specified key.
//

//
// Include necessary headers...
//

#include "rc4.h"


//
// 'hdRC4::encrypt()' - Encrypt the given buffer.
//

void
hdRC4::encrypt(const hdByte *input,	// I - Input buffer
	       hdByte       *output,	// O - Output buffer
	       size_t       len)	// I - Size of buffers
{
  int		i, j;			// Looping vars
  hdByte	tmp;			// Swap variable
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
// 'hdRC4::init()' - Initialize the RC4 context with the specified key.
//

void
hdRC4::init(const hdByte *key,		// I - Key
    	    size_t       keylen)	// I - Length of key
{
  int		i, j;			// Looping vars
  hdByte	tmp;			// Temporary variable


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
// End of "$Id$".
//
