//
// "$Id: link.cxx,v 1.1 2004/04/01 03:26:43 mike Exp $"
//
//   Link functions for HTMLDOC, a HTML document processing program.
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
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdBook::add_link()      - Add a named link...
//   hdBook::compare_links() - Compare two named links.
//   hdBook::find_link()     - Find a named link...
//

//
// Include necessary headers.
//

#include "htmldoc.h"


//
// 'hdBook::add_link()' - Add a named link...
//

void
hdBook::add_link(uchar *name,		// I - Name of link
                 uchar *filename,	// I - File for link
		 int   page,		// I - Page number of link
		 int   top)		// I - Y position of link on page
{
  hdLink	*temp;			// New name


  if ((temp = find_link(name, filename)) == NULL)
  {
    // See if we need to allocate memory for links...
    if (num_links >= alloc_links)
    {
      // Allocate more links...
      alloc_links += ALLOC_LINKS;

      if (num_links == 0)
        temp = (hdLink *)malloc(sizeof(hdLink) * alloc_links);
      else
        temp = (hdLink *)realloc(links, sizeof(hdLink) * alloc_links);

      if (temp == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
	               "Unable to allocate memory for %d links - %s",
	               alloc_links, strerror(errno));
        alloc_links -= ALLOC_LINKS;
	return;
      }

      links = temp;
    }

    // Add a new link...
    temp = links + num_links;
    num_links ++;

    strlcpy((char *)temp->name, (char *)name, sizeof(temp->name));
    temp->filename = filename;
    temp->page     = page;
    temp->top      = top;

    if (num_links > 1)
      qsort(links, num_links, sizeof(hdLink), (hdCompareFunc)compare_links);
  }
}


//
// 'hdBook::compare_links()' - Compare two named links.
//

int					// O - 0 = equal, -1 or 1 = not equal
hdBook::compare_links(hdLink *n1,	// I - First name
                      hdLink *n2)	// I - Second name
{
  int	diff;				// Difference...


  if (n1->filename && n2->filename)
    if ((diff = strcasecmp((char *)n1->filename, (char *)n2->filename)) != 0)
      return (diff);

  return (strcmp((char *)n1->name, (char *)n2->name));
}


//
// 'hdBook::find_link()' - Find a named link...
//

hdLink *				// O - Matching link or NULL
hdBook::find_link(uchar *name,		// I - Name to find
                  uchar *filename)	// I - Filename
{
  uchar		*target;		// Pointer to target name portion
  hdLink	key;			// Search key


  if (!name || !num_links)
    return (NULL);

  strlcpy((char *)key.name, (char *)target, sizeof(key.name));
  key.filename = filename;

  return ((hdLink *)bsearch(&key, links, num_links, sizeof(hdLink),
                            (hdCompareFunc)compare_links));
}


//
// End of "$Id: link.cxx,v 1.1 2004/04/01 03:26:43 mike Exp $".
//
