//
// "$Id: testsuite.cxx,v 1.1 2002/01/06 20:04:48 mike Exp $"
//
//   Test program for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2002 by Easy Software Products.
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

#include "image.h"
#include "hdstring.h"


//
// 'main()' - Main entry.
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  hdImage	*img;		// Image
  char		filename[1024];	// Remote file from local cache


  setbuf(stdout, NULL);

  hdImage::register_standard();

  if ((img = hdImage::find("testimg.jpg", 0, ".;../jpeg")) == NULL)
  {
    puts("Unable to load testimg.jpg using path \".;../jpeg\"...");
    return (1);
  }

  printf("testimg.jpg: %dx%d pixels\n", img->width(), img->height());

  if (hdFile::find(0, "http://www.easysw.com/images/title-htmldoc.gif",
                   filename, sizeof(filename)))
  {
    puts("Copied title-htmldoc.gif from www.easysw.com, trying to load...");

    if ((img = hdImage::find(filename, 0)) == NULL)
    {
      puts("Unable to load title-htmldoc.gif from cache file...");
      return (1);
    }
  }
  else
  {
    puts("Unable to load title-htmldoc.gif from www.easysw.com...");
    return (1);
  }

  printf("title-htmldoc.gif: %dx%d pixels\n", img->width(), img->height());

  hdImage::flush();
  hdFile::cleanup();

  return (0);
}


//
// End of "$Id: testsuite.cxx,v 1.1 2002/01/06 20:04:48 mike Exp $".
//
