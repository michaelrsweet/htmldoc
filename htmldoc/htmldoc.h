//
// "$Id: htmldoc.h,v 1.27 2002/02/23 04:03:30 mike Exp $"
//
//   Common definitions for HTMLDOC, a HTML document processing program.
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

#ifndef _HTMLDOC_HTMLDOC_H_
#  define _HTMLDOC_HTMLDOC_H_


//
// Include necessary headers...
//

#  include "image.h"
#  include "tree.h"


//
// C library comparison function type...
//

extern "C"
{
  typedef int (*hdCompareFunc)(const void *, const void *);
}


//
// Named page size information...
//

struct hdPageSize
{
  char	*name;			// Name of page size
  float	width,			// Width of page in points
	length;			// Length of page in points

  void	set(const char *n, float w, float l);
  void	clear();
};


//
// Entity information...
//

struct hdEntity
{
  char	*html;			// HTML entity name
  char	*glyph;			// PostScript glyph name

  void	set(const char *h, const char *g);
  void	clear();
};


//
// The hdCommon structure contains common global data and structures...
//
// This is a single instance of this structure, which allows us to
// free global data when the process exits.
//

struct hdCommon
{
  // Global data...
  const char	*datadir;	// Directory for data files

  int		num_sizes;	// Number of sizes in table
  hdPageSize	*sizes;		// Size table

  int		num_entities;	// Number of entities in table
  hdEntity	*entities;	// Entity table

  // Global functions...
  hdCommon();
  ~hdCommon();

  const char	*find_entity(const char *g);
  const char	*find_glyph(const char *h);
  hdPageSize	*find_size(const char *n);
  hdPageSize	*find_size(float w, float l);
  void		load_entities();
  void		load_sizes();
};

extern hdCommon	hdGlobal;


#endif // !_HTMLDOC_HTMLDOC_H_

//
// End of "$Id: htmldoc.h,v 1.27 2002/02/23 04:03:30 mike Exp $".
//
