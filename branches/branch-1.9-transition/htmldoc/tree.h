//
// "$Id: tree.h,v 1.20.2.1 2004/03/22 21:56:29 mike Exp $"
//
//   HTML tree definitions for HTMLDOC, a HTML document processing program.
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
#  include "margin.h"


//
// Nodebreak values...
//

enum hdNodeBreak
{
  HD_NODEBREAK_NONE = 0,
  HD_NODEBREAK_LEFT = 1,
  HD_NODEBREAK_RIGHT = 2,
  HD_NODEBREAK_LINE = 3,
  HD_NODEBREAK_PAGE = 4,
  HD_NODEBREAK_SHEET = 5
};


/**
 * The hdTreeAttr structure holds attribute information for a node.
 */

struct hdTreeAttr
{
  //* Variable name
  char		*name;
  //* Variable value
  char		*value;
};


/**
 * The hdTree structure holds a single node in an HTML document tree.
 */

struct hdTree
{
  //* Parent tree entry
  hdTree	*parent;
  //* First child entry
  hdTree	*child;
  //* Last child entry
  hdTree	*last_child;
  //* Previous entry on this level
  hdTree	*prev;
  //* Next entry on this level
  hdTree	*next;
  //* Linked-to
  hdTree	*link;
  //* Markup element
  hdElement	element;
  //* Text (HD_ELEMENT_NONE or HD_ELEMENT_COMMENT)
  char		*data;
  //* Style data
  hdStyle	*style;
  //* Width of this fragment in points
  float		width;
  //* Height of this fragment in points
  float		height;
  //* Whitespace before this node?
  bool		whitespace;
  //* Line/left/right break before this node?
  char		nodebreak;
  //* Number of attributes...
  short		nattrs;
  //* Attributes...
  hdTreeAttr	*attrs;

  //* Strings for each element...
  static const char	*elements[];
  //* Grouping information for each element...
  static unsigned char	elgroup[HD_ELEMENT_MAX];

 /**
  * The constructor creates a new tree node.
  *
  * @param e hdElement The element type for this node.
  * @param d const&nbsp;char* The data for this node, if any.
  * @param p hdTree* The parent node, if any.
  */
  hdTree(hdElement e = HD_ELEMENT_NONE, const char *d = (const char *)0,
         hdTree *p = (hdTree *)0);

 /**
  * The destructor free a tree node.
  */
  ~hdTree();

 /**
  * The <TT>add()</TT> method adds the current node as the last child of
  * the specified parent. If the node is a child of another parent, it
  * is first removed from the old parent.
  *
  * @param p hdTree* Parent node.
  */
  void			add(hdTree *p);

 /**
  * The <TT>build_index()</TT> method creates an index of words from a file.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param wordfile hdFile* The file containing the list of words.
  * @return The document tree containing the index.
  */
  hdTree		*build_index(hdStyleSheet *css, hdFile *wordfile);

 /**
  * The <TT>build_index()</TT> method creates an index of words from an array of
  * phrases.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param num_phrases int The number of word phrases to index.
  * @param phrases const&nbsp;char** The word phrases to index.
  * @return The document tree containing the index.
  */
  hdTree		*build_index(hdStyleSheet *css, int num_phrases,
			             const char **phrases);

 /**
  * The <TT>build_list()</TT> method creates a list of figures, tables, etc. based
  * on the named HTML class.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param class_ const&nbsp;char* The class to use.
  * @param prefix const&nbsp;char* The prefix to use for generated links.
  * @return The document tree containing the list.
  */
  hdTree		*build_list(hdStyleSheet *css, const char *class_,
			            const char *prefix);

 /**
  * The <TT>build_toc()</TT> method creates a table of contents with the given
  * parameters.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param levels int The number of TOC levels to show.
  * @param numbered bool If true, headings are automatically numbered.
  * @return The document tree containing the table of contents.
  */
  hdTree		*build_toc(hdStyleSheet *css, int levels, bool numbered);

 /**
  * The <TT>compute_size()</TT> method computes the width and height of the node.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  */
  void			compute_size(hdStyleSheet *css);

 /**
  * The <TT>copy_text()</TT> method copies all text nodes from the current node
  * to the new parent node.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param p hdTree* The new parent node.
  */
  void			copy_text(hdStyleSheet *css, hdTree *p);

 /**
  * The <TT>find()</TT> method finds the first child node matching the given element.
  *
  * @param e hdElement The element to find.
  * @return A pointer to the child node or NULL if none is found.
  */
  hdTree		*find(hdElement e);

 /**
  * The <TT>fix_url()</TT> method converts a relative URL into an absolute URL.
  *
  * @param url const&nbsp;char* The URL to convert.
  * @param base const&nbsp;char* The base path for the document.
  * @param path const&nbsp;char* A semicolon-separated list of directories and URLs to search.
  * @param s char* URL output buffer.
  * @param slen int The size of the URL output buffer.
  * @return The absolute URL.
  */
  static char		*fix_url(const char *url, const char *base,
			         const char *path, char *s, int slen);

 /**
  * The <TT>format_comment()</TT> method processes HTML comments.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_comment(hdStyleSheet *css, hdMargin *m,
			               float &x, float &y);

 /**
  * The <TT>format_contents()</TT> method formats a table-of-contents.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_contents(hdStyleSheet *css, hdMargin *m,
			                float &x, float &y);

 /**
  * The <TT>format_doc()</TT> method formats this node and all child nodes,
  * updating the width, height, and nodebreak members.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_doc(hdStyleSheet *css, hdMargin *m,
			           float &x, float &y);

 /**
  * The <TT>format_index()</TT> method formats an index.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_index(hdStyleSheet *css, hdMargin *m,
			             float &x, float &y);

 /**
  * The <TT>format_list()</TT> method formats a list of whatever.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_list(hdStyleSheet *css, hdMargin *m,
			            float &x, float &y);

 /**
  * The <TT>format_table()</TT> method formats a table, updating the width,
  * height, and nodebreak members.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The margins to use.
  * @param x float&amp; The current X position in points.
  * @param y float&amp; The current Y position in points.
  * @param page int&amp; The current page.
  */
  void			format_table(hdStyleSheet *css, hdMargin *m,
			             float &x, float &y);

 /**
  * The <TT>get_attr()</TT> method retrieves the value of a node attribute.
  * The attribute name is case-insensitive.
  *
  * @param name const&nbsp;char* The attribute name.
  * @return The string value or NULL if undefined. If the name is
  * defined without a value, then an empty string is returned.
  */
  const char		*get_attr(const char *name);

 /**
  * The <TT>get_cell_size()</TT> method determines the size of the table cell.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The current margins.
  * @param minwidth float&amp; The minimum width of the cell.
  * @param prefwidth float&amp; The preferred width of the cell.
  * @param minheight float&amp; The minimum height of the cell.
  * @return The required width of the cell or 0 if none is specified.
  */
  float			get_cell_size(hdStyleSheet *css, hdMargin *m,
			              float &minwidth, float &prefwidth,
				      float &minheight);

 /**
  * The <TT>get_element()</TT> method gets the hdElement constant associated with
  * the given string.
  *
  * @param name const&nbsp;char* The element name.
  * @return The hdElement constant.
  */
  static hdElement	get_element(const char *name);

 /**
  * The <TT>get_meta()</TT> method gets the META data associated with the given name.
  * If the current node is not a META element then get_meta() recursively
  * searches the tree for META elements.
  *
  * @param name const&nbsp;char* The META name.
  * @return The associated string value or NULL if none is count.
  */
  const char		*get_meta(const char *name);

 /**
  * The <TT>get_table_size()</TT> method determines the size of a table.
  *
  * @param css hdStyleSheet* The stylesheet to use.
  * @param m hdMargin* The current margins.
  * @param minwidth float&amp; The minimum width of the table.
  * @param prefwidth float&amp; The preferred width of the table.
  * @param minheight float&amp; The minimum height of the table.
  * @return The required width of the table or 0 if none is specified.
  */
  float			get_table_size(hdStyleSheet *css, hdMargin *m,
			               float &minwidth, float &prefwidth,
				       float &minheight);

 /**
  * The <TT>get_text()</TT> method returns the text associated with the current
  * node. If the current node is not HD_ELEMENT_NONE or
  * HD_ELEMENT_COMMENT, then get_text() recursively combines text
  * fragments of child nodes as needed.
  *
  * @param comments bool Whether to include comment text.
  * @return The text associated with the current node or NULL if none
  * is found.
  */
  char			*get_text(bool comments = false);

 /**
  * The <TT>get_title()</TT> method returns the text associated with the title
  * node in the current tree. If the current node is not a
  * HD_ELEMENT_TITLE element, then get_title() recursively searches the
  * tree.
  *
  * @return The title text or NULL if none is found.
  */
  char			*get_title();

 /**
  * The <TT>insert()</TT> method inserts the current node as the first child of
  * the specified parent. If the node is a child of another parent, it
  * is first removed from the old parent.
  *
  * @param p hdTree* Parent node.
  */
  void			insert(hdTree *p);

 /**
  * The <TT>parse_attribute()</TT> method parses a single attribute from a file
  * and adds it to the current node.
  *
  * @param fp hdFile* File to read from.
  * @return Returns true if the attribute was read correctly, false
  * otherwise.
  */
  bool			parse_attribute(hdFile *fp);

 /**
  * The <TT>parse_element()</TT> method parses an element from a file and
  * initializes the current node.
  *
  * @param fp hdFile* File to read from.
  * @return The HTML element or HD_ELEMENT_ERROR if the element could
  * not be parsed.
  */
  hdElement		parse_element(hdFile *fp);

 /**
  * The <TT>parse_entity()</TT> method parses a HTML entity (&amp;lt;, etc.)
  * from a file and puts the corresponding string representation in
  * the supplied string buffer.  If the entity is invalid, the
  * original text is returned.
  *
  * @param fp hdFile* File to read from.
  * @param css hdStyleSheet* The stylesheet to use.
  * @param s char* The string buffer.
  * @param slen int The size of the string buffer in bytes.
  */
  void			parse_entity(hdFile *fp, hdStyleSheet *css,
			             char *s, int slen);

 /**
  * The <TT>read()</TT> method reads a HTML document tree from the specified
  * file.
  *
  * @param fp hdFile* File to read from.
  * @param base const&nbsp;char* The base path for the file.
  * @param path const&nbsp;char* The search path to use.
  * @param css hdStyleSheet* The stylesheet to use.
  * @return The document tree.
  */
  static hdTree		*read(hdFile *fp, const char *base,
			      const char *path, hdStyleSheet *css);

 /**
  * The <TT>real_next()</TT> method returns the next logical node in the
  * tree.
  *
  * @param descend bool Whether to descend into child nodes.
  * @return The next logical node.
  */
  hdTree		*real_next(bool descend = true);

 /**
  * The <TT>real_prev()</TT> method returns the previous logical node in
  * the tree.
  *
  * @return The previous logical node.
  */
  hdTree		*real_prev();

 /**
  * The <TT>remove()</TT> method removes the current node from its parent.
  */
  void			remove();

 /**
  * The <TT>set_attr()</TT> method adds or changes a node attribute.
  * The attribute name is case-insensitive.
  *
  * @param name const&nbsp;char* The name of the attribute.
  * @param value const&nbsp;char* The value of the attribute.
  */
  void			set_attr(const char *name, const char *value);
};

#endif // !_HTMLDOC_TREE_H_

//
// End of "$Id: tree.h,v 1.20.2.1 2004/03/22 21:56:29 mike Exp $".
//
