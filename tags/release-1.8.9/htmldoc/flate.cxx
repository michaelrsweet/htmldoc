//
// "$Id: flate.cxx,v 1.1 2000/10/16 03:25:06 mike Exp $"
//
//   Flate compression routines for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 1997-2000 by Easy Software Products.
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
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "htmldoc.h"


//
// 'HTMLDOC::flate_open()' - Open a deflated output stream.
//

void
HTMLDOC::flate_open()
{
  if (encryption_ && !ps_level_)
    encrypt_init();

  if (!compression_)
    return;

  compressor_.zalloc = (alloc_func)0;
  compressor_.zfree  = (free_func)0;
  compressor_.opaque = (voidpf)0;

  deflateInit(&compressor_, compression_);

  compressor_.next_out  = (Bytef *)comp_buffer_;
  compressor_.avail_out = sizeof(comp_buffer_);
}


//
// 'HTMLDOC::flate_close()' - Close a deflated output stream.
//

void
HTMLDOC::flate_close()
{
  if (!compression_)
    return;

  while (deflate(&compressor_, Z_FINISH) != Z_STREAM_END)
  {
    if (ps_level_)
      ps_ascii85(comp_buffer_,
                 (uchar *)compressor_.next_out - (uchar *)comp_buffer_);
    else
    {
      if (encryption_)
        rc4_encrypt(&encrypt_state_, comp_buffer_, comp_buffer_,
	            (uchar *)compressor_.next_out - (uchar *)comp_buffer_);

      fwrite(comp_buffer_,
             (uchar *)compressor_.next_out - (uchar *)comp_buffer_, 1, out_);
    }

    compressor_.next_out  = (Bytef *)comp_buffer_;
    compressor_.avail_out = sizeof(comp_buffer_);
  }

  if ((uchar *)compressor_.next_out > (uchar *)comp_buffer_)
  {
    if (ps_level_)
      ps_ascii85(comp_buffer_,
                 (uchar *)compressor_.next_out - (uchar *)comp_buffer_);
    else
    {
      if (encryption_)
        rc4_encrypt(&encrypt_state_, comp_buffer_, comp_buffer_,
	            (uchar *)compressor_.next_out - (uchar *)comp_buffer_);

      fwrite(comp_buffer_,
             (uchar *)compressor_.next_out - (uchar *)comp_buffer_, 1, out_);
    }

  }

  deflateEnd(&compressor_);

  if (ps_level_)
    fputs("~>\n", out_);
}


//
// 'HTMLDOC::flate_puts()' - Write a character string to a compressed stream.
//

void
HTMLDOC::flate_puts(char *s)	// I - String to write
{
  flate_write((uchar *)s, strlen(s));
}


//
// 'HTMLDOC::flate_printf()' - Write a formatted character string to a compressed stream.
//

void
HTMLDOC::flate_printf(char *format,	// I - Format string
                      ...)		// I - Additional args as necessary
{
  int		length;		// Length of output string
  char		buf[10240];	// Output buffer
  va_list	ap;		// Argument pointer


  va_start(ap, format);
  length = vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  flate_write((uchar *)buf, length);
}


//
// 'HTMLDOC::flate_write()' - Write data to a compressed stream.
//

void
HTMLDOC::flate_write(uchar *buf,	// I - Buffer
        	     int   length,	// I - Number of bytes to write
		     int   flush)	// I - Flush when writing data?
{
  if (compression_)
  {
    compressor_.next_in  = buf;
    compressor_.avail_in = length;

    while (compressor_.avail_in > 0)
    {
      if (compressor_.avail_out < (sizeof(comp_buffer_) / 8))
      {
	if (ps_level_)
	  ps_ascii85(comp_buffer_,
                     (uchar *)compressor_.next_out - (uchar *)comp_buffer_);
	else
	{
	  if (encryption_)
            rc4_encrypt(&encrypt_state_, comp_buffer_, comp_buffer_,
	        	(uchar *)compressor_.next_out - (uchar *)comp_buffer_);

	  fwrite(comp_buffer_,
	         (uchar *)compressor_.next_out - (uchar *)comp_buffer_, 1, out_);
	}

	compressor_.next_out  = (Bytef *)comp_buffer_;
	compressor_.avail_out = sizeof(comp_buffer_);
      }

      deflate(&compressor_, flush ? Z_FULL_FLUSH : Z_NO_FLUSH);
      flush = 0;
    }
  }
  else if (encryption_ && !ps_level_)
  {
    int		i,		// Looping var
		bytes;		// Number of bytes to encrypt/write
    uchar	newbuf[1024];	// New encrypted data buffer


    for (i = 0; i < length; i += sizeof(newbuf))
    {
      if ((bytes = length - i) > sizeof(newbuf))
	bytes = sizeof(newbuf);

      rc4_encrypt(&encrypt_state_, buf + i, newbuf, bytes);
      fwrite(newbuf, bytes, 1, out_);
    }
  }
  else
    fwrite(buf, length, 1, out_);
}


//
// End of "$Id: flate.cxx,v 1.1 2000/10/16 03:25:06 mike Exp $".
//
