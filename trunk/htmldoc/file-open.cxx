//
// "$Id$"
//
//   File open routines for HTMLDOC, a HTML document processing program.
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
//   hdFile::open() - Open a URI.
//   hdFile::uri()  - Set the URI for a file.
//

//
// Include necessary headers.
//

#include "file.h"
#include "hdstring.h"


//
// 'hdFile::open()' - Open a URI.
//

hdFile *				// O - New file
hdFile::open(const char *uri,		// I - URI to open
             hdMode     m,		// I - Open mode
	     const char *path)		// I - Path to search, if any
{
  hdFile	*f;			// File
  char		filename[HD_MAX_URI];	// Local filename...


  // Get the real local filename for the URI...
  switch (m)
  {
    case HD_FILE_READ :
        // Find the file...
	if (find(path, uri, filename, sizeof(filename)) == NULL)
	  return ((hdFile *)0);
        break;
    default :
        // Only allow local files
        if (scheme(uri))
	  return ((hdFile *)0);

	strncpy(filename, uri, sizeof(filename) - 1);
	filename[sizeof(filename) - 1] = '\0';
	break;
  }

  // Open the file...
  f = new hdStdFile(filename, m);

  if (f->error())
  {
    // Unable to open file...
    delete f;
    return ((hdFile *)0);
  }
  else
  {
    f->uri(uri);
    return f;
  }
}


//
// 'hdFile::uri()' - Set the URI for a file.
//

void
hdFile::uri(const char *u)		// I - URI string
{
  uri_ = strdup(u);
}


//
// End of "$Id$".
//
