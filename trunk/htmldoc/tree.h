//
// "$Id: tree.h,v 1.2 2002/01/16 01:15:39 mike Exp $"
//
//   HTML tree definitions for HTMLDOC, a HTML document processing program.
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

#ifndef _HTMLDOC_TREE_H_
#  define _HTMLDOC_TREE_H_

//
// Include necessary headers...
//

#  include "file.h"
#  include "style.h"


//
// Tree attribute...
//

struct hdTreeAttr
{
  char		*name,		// Variable name
		*value;		// Variable value
};


//
// Parsing tree...
//

struct hdTree
{
  // Node data...
  hdTree	*parent,	// Parent tree entry
		*child,		// First child entry
		*last_child,	// Last child entry
		*prev,		// Previous entry on this level
		*next,		// Next entry on this level
		*link;		// Linked-to
  hdElement	element;	// Markup element
  char		*data;		// Text (HD_ELEMENT_NONE or HD_ELEMENT_COMMENT)
  unsigned	halignment:2,	// Horizontal alignment
		valignment:2,	// Vertical alignment
		typeface:4,	// Typeface code
		size:3,		// Size of text
		style:2,	// Style of text
		underline:1,	// Text is underlined?
		strikethrough:1,// Text is struck-through?
		subscript:1,	// Text is subscripted?
		superscript:1,	// Text is superscripted?
		preformatted:1,	// Preformatted text?
		indent:4;	// Indentation level 0-15
  char		red,		// Color of this fragment
		green,
		blue;
  float		width,		// Width of this fragment in points
		height;		// Height of this fragment in points
  int		nattrs;		// Number of attributes...
  hdTreeAttr	*attrs;		// Attributes...

  // Global data...
  static const char	*elements[];
  static const char	*datadir;
  static float		ppi;
  static int		grayscale;
  static char		text_color[];
  static float		browser_width;
  static float		font_sizes[],
			line_spacings[];
  static hdFontFace	body_font,
			heading_font;
  static char		text_charset[32];
  static float		font_widths[4][4][256];
  static const char	font_glyphs[];
  static const char	*fonts[4][4];

  // Methods...
  hdTree(hdElement e = 0, const char *d = (const char *)0);
  ~hdTree();

  void		add(hdTree *p);	// Add to end of parent
  void		insert(hdTree *p);// Insert at beginning of parent
  void		remove();	// Remove from parent

  char		*get_text();	// Create a copy of the text under this node
  const char	*get_meta(const char *name);
				// Find META data
  static hdTree	*read(hdTree *p, hdFile *fp, const char *base,
		      const char *path);

  const char	*get_attr(const char *name);
  void		set_attr(const char *name, const char *value);

  const char	*get_style(const char *name);

  static void	set_base_size(float p, float s);
  static void	set_charset(const char *cs);
  static void	set_text_color(const char *color);
} hdTree;

#endif // !_HTMLDOC_TREE_H_

//
// End of "$Id: tree.h,v 1.2 2002/01/16 01:15:39 mike Exp $".
//
