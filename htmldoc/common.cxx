//
// "$Id: common.cxx,v 1.1 2002/02/08 19:40:38 mike Exp $"
//
//   Common routines for HTMLDOC, a HTML document processing program.
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

#include "htmldoc.h"
#include "hdstring.h"
#include <stdlib.h>
#include <math.h>


//
// Define the global instance...
//

hdCommon	hdGlobal;


//
// 'hdCommon::hdCommon()' - Initialize global data...
//

hdCommon::hdCommon()
{
  // MRS: UPDATE THIS
  datadir   = "..";

  num_sizes = 0;
  sizes     = (hdPageSize *)0;
}


//
// 'hdCommon::~hdCommon()' - Free global data...
//

hdCommon::~hdCommon()
{
  int	i;			// Looping var


  // Free the size table, if loaded...
  if (num_sizes)
  {
    for (i = 0; i < num_sizes; i ++)
      sizes[i].clear();

    delete[] sizes;
  }

  // Cleanup the image and file caches...
  hdImage::flush();
  hdFile::cleanup();

}


//
// 'hdCommon::load_sizes()' - Load the size table.
//

void
hdCommon::load_sizes()
{
  hdFile	*fp;			// Page size file pointer
  char		filename[1024],		// Page size filename
		line[255],		// Line from file
		name[64];		// Size name
  float		width,			// Width in points
		length;			// Height in points
  hdPageSize	*temp;			// Temporary array pointer
  int		alloc_sizes;		// Number of sizes allocated


  // See if we need to load the size table...
  if (num_sizes)
    return;

  // Open the sizes.dat file...
  snprintf(filename, sizeof(filename), "%s/data/sizes.dat", datadir);
  if ((fp = hdFile::open(filename, HD_FILE_READ)) == NULL)
  {
    fprintf(stderr, "Unable to open page size file \"%s\"!\n", filename);
    return;
  }

  // Read all of the sizes from the file...
  alloc_sizes = 32;
  sizes       = new hdPageSize[32];
  num_sizes   = 0;

  while (fp->gets(line, sizeof(line)) != NULL)
  {
    // Skip blank and comment lines...
    if (line[0] == '#' || !line[0])
      continue;

    // Scan the line for the size info...
    if (sscanf(line, "%63s%f%f", name, &width, &length) != 3)
    {
      fprintf(stderr, "Bad page size line \"%s\" in \"%s\"!\n", line,
              filename);
      continue;
    }

    // Reallocate memory as needed...
    if (num_sizes >= alloc_sizes)
    {
      temp = new hdPageSize[alloc_sizes + 32];

      memcpy(temp, sizes, alloc_sizes * sizeof(hdPageSize));

      delete[] sizes;

      sizes       = temp;
      alloc_sizes += 32;
    }

    // Set the next size in the array...
    sizes[num_sizes].set(name, width, length);
    num_sizes ++;
  }

  delete fp;
}


//
// 'hdCommon::find_size()' - Get the page size by numbers.
//

hdPageSize *				// O - Matching size
hdCommon::find_size(float w,		// I - Width in points
                    float l)		// I - Length in points
{
  int		i;			// Looping var
  hdPageSize	*s;			// Current size record


  // Lookup the size in the size table...
  load_sizes();

  for (i = num_sizes, s = sizes; i > 0; i --, s ++)
    if (fabs(s->width - w) < 0.1f && fabs(s->length - l) < 0.1f)
      return (s);

  return (NULL);
}


//
// 'hdCommon::find_size()' - Set the page size by name.
//

hdPageSize *				// O - Matching page size
hdCommon::find_size(const char *name)	// I - Page size name
{
  int		i;			// Looping var
  hdPageSize	*s;			// Current size record


  // Lookup the name in the size table...
  load_sizes();

  for (i = num_sizes, s = sizes; i > 0; i --, s ++)
    if (strcasecmp(name, s->name) == 0)
      return (s);

  return (NULL);
}


//
// 'hdPageSize::set()' - Initialize a page size.
//

void
hdPageSize::set(const char *n,	// I - Name of size
                float      w,	// I - Width in points
		float      l)	// I - Length in points
{
  name   = strdup(n);
  width  = w;
  length = l;
}


//
// 'hdPageSize::clear()' - Clear a page size.
//

void
hdPageSize::clear()
{
  if (name)
  {
    free(name);
    name = NULL;
  }
}


//
// End of "$Id: common.cxx,v 1.1 2002/02/08 19:40:38 mike Exp $".
//
