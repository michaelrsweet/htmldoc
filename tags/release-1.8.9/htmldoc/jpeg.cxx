//
// "$Id: jpeg.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $"
//
//   JPEG compression routines for HTMLDOC, a HTML document processing
//   program.
//
//   Just in case you didn't notice it, this file is too big; it will be
//   broken into more manageable pieces once we make all of the output
//   "drivers" into classes...
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

#ifdef MAC		// MacOS-specific header file...
#  include <Files.h>
#endif // MAC

#if defined(WIN32) || defined(__EMX__)
#  include <io.h>
#else
#  include <unistd.h>
#endif // WIN32 || __EMX__

#include <fcntl.h>


//
// JPEG library destination data manager.  These routines direct
// compressed data from libjpeg into the PDF or PostScript file.
//



//
// 'HTMLDOC::jpg_init()' - Initialize the JPEG destination.
//

void
HTMLDOC::jpg_init(j_compress_ptr cinfo)	// I - Compressor info
{
  (void)cinfo;

  jpg_dest.next_output_byte = jpg_buf;
  jpg_dest.free_in_buffer   = sizeof(jpg_buf);
}


//
// 'HTMLDOC::jpg_empty()' - Empty the JPEG output buffer.
//

static boolean			// O - True if buffer written OK
HTMLDOC::jpg_empty(j_compress_ptr cinfo)	// I - Compressor info
{
  (void)cinfo;

  if (PSLevel > 0)
    ps_ascii85(jpg_file, jpg_buf, sizeof(jpg_buf));
  else
    flate_write(jpg_file, jpg_buf, sizeof(jpg_buf));

  jpg_dest.next_output_byte = jpg_buf;
  jpg_dest.free_in_buffer   = sizeof(jpg_buf);

  return (TRUE);
}


//
// 'HTMLDOC::jpg_term()' - Write the last JPEG data to the file.
//

void
HTMLDOC::jpg_term(j_compress_ptr cinfo)	// I - Compressor info
{
  int nbytes;			// Number of bytes to write


  (void)cinfo;

  nbytes = sizeof(jpg_buf) - jpg_dest.free_in_buffer;

  if (PSLevel > 0)
    ps_ascii85(jpg_file, jpg_buf, nbytes);
  else
    flate_write(jpg_file, jpg_buf, nbytes);
}


//
// 'HTMLDOC::jpg_setup()' - Setup the JPEG compressor for writing an image.
//

void
HTMLDOC::jpg_setup(FILE           *out,	// I - Output file
          HDtree        *img,	// I - Output image
          j_compress_ptr cinfo)	// I - Compressor info
{
  int	i;			// Looping var


  jpg_file    = out;
  cinfo->err  = jpeg_std_error(&jerr);

  jpeg_create_compress(cinfo);

  cinfo->dest = &jpg_dest;
  jpg_dest.init_destination    = jpg_init;
  jpg_dest.empty_output_buffer = jpg_empty;
  jpg_dest.term_destination    = jpg_term;

  cinfo->image_width      = img->width;
  cinfo->image_height     = img->height;
  cinfo->input_components = img->depth;
  cinfo->in_color_space   = img->depth == 1 ? JCS_GRAYSCALE : JCS_RGB;

  jpeg_set_defaults(cinfo);
  jpeg_set_linear_quality(cinfo, OutputJPEG, TRUE);

  // Update things when writing to PS files...
  if (PSLevel)
  {
    // Adobe uses sampling == 1
    for (i = 0; i < img->depth; i ++)
    {
      cinfo->comp_info[i].h_samp_factor = 1;
      cinfo->comp_info[i].v_samp_factor = 1;
    }
  }

  cinfo->write_JFIF_header  = FALSE;
  cinfo->write_Adobe_marker = TRUE;

  jpeg_start_compress(cinfo, TRUE);
}


//
// End of "$Id: jpeg.cxx,v 1.1 2000/10/16 03:25:08 mike Exp $".
//
