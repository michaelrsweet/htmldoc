//
// "$Id: tree.h,v 1.10 2002/04/03 18:12:57 mike Exp $"
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
  hdTree		*build_index(hdStyleSheet *css, hdFile *wordfile);
  hdTree		*build_index(hdStyleSheet *css, int num_phrases,
			             const char **phrases);
  hdTree		*build_list(hdStyleSheet *css, const char *class_,
			            const char *prefix);
  hdTree		*build_toc(hdStyleSheet *css, int levels, int numbered);
  void			compute_size(hdStyleSheet *css);
  void			copy_text(hdTree *p);
  hdTree		*find(hdElement e);
  static char		*fix_url(const char *url, const char *base,
			         const char *path, char *s, int slen);
  const char		*get_attr(const char *name);
  static hdElement	get_element(const char *name);
  const char		*get_meta(const char *name);
  char			*get_text(int comments = 0);
  char			*get_title();
  void			insert(hdTree *p);
  int			parse_attribute(hdFile *fp);
  hdElement		parse_element(hdFile *fp);
  void			parse_entity(hdFile *fp, hdStyleSheet *css,
			             char *s, int slen);
  static hdTree		*read(hdFile *fp, const char *base,
			      const char *path, hdStyleSheet *css);
  hdTree		*real_next(int descend = 1);
  hdTree		*real_prev();
  void			remove();
  void			set_attr(const char *name, const char *value);
};

#endif // !_HTMLDOC_TREE_H_

//
// End of "$Id: tree.h,v 1.10 2002/04/03 18:12:57 mike Exp $".
//
