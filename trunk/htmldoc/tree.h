//
// "$Id: tree.h,v 1.6 2002/03/10 03:17:29 mike Exp $"
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
  hdStyle	*style;		// Style data
  int		whitespace;	// Whitespace before this node?
  float		width,		// Width of this fragment in points
		height;		// Height of this fragment in points
  short		aattrs,		// Allocated attributes...
		nattrs;		// Number of attributes...
  hdTreeAttr	*attrs;		// Attributes...

  // Global data...
  static const char	*elements[];
  static unsigned char	elgroup[HD_ELEMENT_MAX];

  // Methods...
  hdTree(hdElement e = HD_ELEMENT_NONE, const char *d = (const char *)0,
         hdTree *p = (hdTree *)0);
  ~hdTree();

  void			add(hdTree *p);
  static hdTree		*build_index(hdTree *p, hdFile *wordfile);
  static hdTree		*build_index(hdTree *p, int num_words, const char **words);
  static hdTree		*build_toc(hdTree *p, int levels);
  void			compute_size(hdStyleSheet *css);
  static char		*fix_url(const char *url, const char *base,
			         const char *path, char *s, int slen);
  const char		*get_attr(const char *name);
  static hdElement	get_element(const char *name);
  const char		*get_meta(const char *name);
  char			*get_text();
  char			*get_title();
  void			insert(hdTree *p);
  int			parse_attribute(hdFile *fp);
  hdElement		parse_element(hdFile *fp);
  void			parse_entity(hdFile *fp, hdStyleSheet *css,
			             char *s, int slen);
  static hdTree		*read(hdFile *fp, const char *base,
			      const char *path, hdStyleSheet *css);
  hdTree		*real_next();
  hdTree		*real_prev();
  void			remove();
  void			set_attr(const char *name, const char *value);
};

#endif // !_HTMLDOC_TREE_H_

//
// End of "$Id: tree.h,v 1.6 2002/03/10 03:17:29 mike Exp $".
//
