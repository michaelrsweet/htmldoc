//
// "$Id: htmldoc.h,v 1.18.2.21.2.4 2004/03/30 03:49:15 mike Exp $"
//
//   Common definitions for HTMLDOC, a HTML document processing program.
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

#ifndef _HTMLDOC_HTMLDOC_H_
#  define _HTMLDOC_HTMLDOC_H_


//
// Include necessary headers...
//

#  include "render.h"


//
// Error codes (in addition to the HTTP status codes...)
//

typedef enum
{
  HD_ERROR_NONE = 0,
  HD_ERROR_NO_FILES,
  HD_ERROR_NO_PAGES,
  HD_ERROR_TOO_MANY_CHAPTERS,
  HD_ERROR_OUT_OF_MEMORY,
  HD_ERROR_FILE_NOT_FOUND,
  HD_ERROR_BAD_COMMENT,
  HD_ERROR_BAD_FORMAT,
  HD_ERROR_DELETE_ERROR,
  HD_ERROR_INTERNAL_ERROR,
  HD_ERROR_NETWORK_ERROR,
  HD_ERROR_READ_ERROR,
  HD_ERROR_WRITE_ERROR,
  HD_ERROR_HTML_ERROR,
  HD_ERROR_CONTENT_TOO_LARGE,
  HD_ERROR_UNRESOLVED_LINK,
  HD_ERROR_BAD_HF_STRING,
  HD_ERROR_CSS_ERROR,
  HD_ERROR_HTTPBASE = 100
} hdError;


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
  int		verbosity;	// Verbosity
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
  char		*format_number(char *s, int slen, char format, int number);
  void		load_entities();
  void		load_sizes();

  void		progress_error(hdError error, const char *format, ...);
  void		progress_hide(void);
  void		progress_show(const char *format, ...);
  void		progress_update(int percent);
};

extern hdCommon	hdGlobal;


#endif // !_HTMLDOC_HTMLDOC_H_

//
// End of "$Id: htmldoc.h,v 1.18.2.21.2.4 2004/03/30 03:49:15 mike Exp $".
//
