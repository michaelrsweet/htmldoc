//
// "$Id: tree.cxx,v 1.22.2.3 2004/03/30 03:49:16 mike Exp $"
//
//   HTML parsing routines for HTMLDOC, a HTML document processing program.
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
//   hdTree::hdTree()          - Create a new tree node.
//   hdTree::~hdTree()         - Destroy a tree node.
//   hdTree::add()             - Add a node to the end of a parent.
//   hdTree::compute_size()    - Compute the width and height of a node.
//   hdTree::copy_text()       - Copy all text nodes to a new parent.
//   hdTree::fix_url()         - Fix a URL so that the path is resolved
//                               directly.
//   hdTree::get_attr()        - Get an attribute.
//   hdTree::get_element()     - Get the element index for the named element.
//   hdTree::get_meta()        - Find meta data in the document tree.
//   hdTree::get_text()        - Get a copy of the text fragments under a node.
//   hdTree::get_title()       - Get the first title node...
//   hdTree::insert()          - Insert a node at the beginning of a parent.
//   hdTree::parse_attribute() - Parse an attribute.
//   hdTree::parse_element()   - Parse an element.
//   hdTree::parse_entity()    - Parse a HTML entity and return the text for it.
//   hdTree::read()            - Read a HTML file into a document tree.
//   hdTree::real_next()       - Find next logical node in the tree.
//   hdTree::real_next()       - Find previous logical node in the tree.
//   hdTree::remove()          - Remove a node from its parent.
//   hdTree::set_attr()        - Set an attribute value.
//   compare_elements()        - Compare two element strings...
//   compare_variables()       - Compare two element variables.
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include "hdstring.h"
#include <math.h>


//
// Class globals...
//

const char	*hdTree::elements[HD_ELEMENT_MAX] =
		{			// Element strings...
		  "",			// HD_ELEMENT_NONE
		  "",			// HD_ELEMENT_FILE
		  "",			// HD_ELEMENT_ERROR
		  "",			// HD_ELEMENT_UNKNOWN
		  "!--",		// HD_ELEMENT_COMMENT
		  "!doctype",
		  "a",
		  "acronym",
		  "address",
		  "area",
		  "b",
		  "base",
		  "basefont",
		  "big",
		  "blockquote",
		  "body",
		  "br",
		  "caption",
		  "center",
		  "cite",
		  "code",
		  "col",
		  "colgroup",
		  "dd",
		  "del",
		  "dfn",
		  "dir",
		  "div",
		  "dl",
		  "dt",
		  "em",
		  "embed",
		  "font",
		  "form",
		  "h1",
		  "h2",
		  "h3",
		  "h4",
		  "h5",
		  "h6",
		  "h7",
		  "h8",
		  "h9",
		  "h10",
		  "h11",
		  "h12",
		  "h13",
		  "h14",
		  "h15",
		  "head",
		  "hr",
		  "html",
		  "i",
		  "img",
		  "input",
		  "ins",
		  "isindex",
		  "kbd",
		  "li",
		  "link",
		  "map",
		  "menu",
		  "meta",
		  "object",
		  "ol",
		  "option",
		  "p",
		  "pre",
		  "s",
		  "samp",
		  "script",
		  "select",
		  "small",
		  "spacer",
		  "span",
		  "strike",
		  "strong",
		  "style",
		  "sub",
		  "sup",
		  "table",
		  "tbody",
		  "td",
		  "textarea",
		  "tfoot",
		  "th",
		  "thead",
		  "title",
		  "tr",
		  "tt",
		  "u",
		  "ul",
		  "var",
		  "wbr"
		};

unsigned char	hdTree::elgroup[HD_ELEMENT_MAX] =
		{			// Element group bits
		  HD_ELGROUP_NONE,	// "NONE"
		  HD_ELGROUP_NONE,	// "FILE"
		  HD_ELGROUP_NONE,	// "ERROR"
		  HD_ELGROUP_NONE,	// "UNKNOWN"
		  HD_ELGROUP_NONE,	// !--
		  HD_ELGROUP_NONE,	// !DOCTYPE
		  HD_ELGROUP_INLINE,	// A
		  HD_ELGROUP_INLINE,	// ACRONYM
		  HD_ELGROUP_INLINE,	// ADDRESS
		  HD_ELGROUP_NONE,	// AREA
		  HD_ELGROUP_INLINE,	// B
		  HD_ELGROUP_NONE,	// BASE
		  HD_ELGROUP_NONE,	// BASEFONT
		  HD_ELGROUP_INLINE,	// BIG
		  HD_ELGROUP_BLOCK,	// BLOCKQUOTE
		  HD_ELGROUP_GROUP,	// BODY
		  HD_ELGROUP_NONE,	// BR
		  HD_ELGROUP_INLINE | HD_ELGROUP_TABLE,
					// CAPTION
		  HD_ELGROUP_GROUP,	// CENTER
		  HD_ELGROUP_INLINE,	// CITE
		  HD_ELGROUP_INLINE,	// CODE
		  HD_ELGROUP_NONE,	// COL
		  HD_ELGROUP_TABLE,	// COLGROUP
		  HD_ELGROUP_ITEM,	// DD
		  HD_ELGROUP_INLINE,	// DEL
		  HD_ELGROUP_INLINE,	// DFN
		  HD_ELGROUP_INLINE,	// DIR
		  HD_ELGROUP_INLINE | HD_ELGROUP_GROUP,
					// DIV
		  HD_ELGROUP_LIST,	// DL
		  HD_ELGROUP_ITEM,	// DT
		  HD_ELGROUP_INLINE,	// EM
		  HD_ELGROUP_NONE,	// EMBED
		  HD_ELGROUP_INLINE,	// FONT
		  HD_ELGROUP_GROUP,	// FORM
		  HD_ELGROUP_BLOCK,	// H1
		  HD_ELGROUP_BLOCK,	// H2
		  HD_ELGROUP_BLOCK,	// H3
		  HD_ELGROUP_BLOCK,	// H4
		  HD_ELGROUP_BLOCK,	// H5
		  HD_ELGROUP_BLOCK,	// H6
		  HD_ELGROUP_BLOCK,	// H7
		  HD_ELGROUP_BLOCK,	// H8
		  HD_ELGROUP_BLOCK,	// H9
		  HD_ELGROUP_BLOCK,	// H10
		  HD_ELGROUP_BLOCK,	// H11
		  HD_ELGROUP_BLOCK,	// H12
		  HD_ELGROUP_BLOCK,	// H13
		  HD_ELGROUP_BLOCK,	// H14
		  HD_ELGROUP_BLOCK,	// H15
		  HD_ELGROUP_GROUP,	// HEAD
		  HD_ELGROUP_NONE,	// HR
		  HD_ELGROUP_GROUP,	// HTML
		  HD_ELGROUP_INLINE,	// I
		  HD_ELGROUP_NONE,	// IMG
		  HD_ELGROUP_NONE,	// INPUT
		  HD_ELGROUP_INLINE,	// INS
		  HD_ELGROUP_NONE,	// ISINDEX
		  HD_ELGROUP_INLINE,	// KBD
		  HD_ELGROUP_ITEM,	// LI
		  HD_ELGROUP_NONE,	// LINK
		  HD_ELGROUP_BLOCK,	// MAP
		  HD_ELGROUP_INLINE,	// MENU
		  HD_ELGROUP_NONE,	// META
		  HD_ELGROUP_NONE,	// OBJECT
		  HD_ELGROUP_LIST,	// OL
		  HD_ELGROUP_GROUP,	// OPTION
		  HD_ELGROUP_BLOCK,	// P
		  HD_ELGROUP_BLOCK,	// PRE
		  HD_ELGROUP_INLINE,	// S
		  HD_ELGROUP_INLINE,	// SAMP
		  HD_ELGROUP_INLINE,	// SCRIPT
		  HD_ELGROUP_GROUP,	// SELECT
		  HD_ELGROUP_INLINE,	// SMALL
		  HD_ELGROUP_NONE,	// SPACER
		  HD_ELGROUP_INLINE,	// SPAN
		  HD_ELGROUP_INLINE,	// STRIKE
		  HD_ELGROUP_INLINE,	// STRONG
		  HD_ELGROUP_INLINE,	// STYLE
		  HD_ELGROUP_INLINE,	// SUB
		  HD_ELGROUP_INLINE,	// SUP
		  HD_ELGROUP_INLINE | HD_ELGROUP_TABLE,
					// TABLE
		  HD_ELGROUP_TABLE,	// TBODY
		  HD_ELGROUP_CELL,	// TD
		  HD_ELGROUP_INLINE,	// TEXTAREA
		  HD_ELGROUP_TABLE,	// TFOOT
		  HD_ELGROUP_CELL,	// TH
		  HD_ELGROUP_TABLE,	// THEAD
		  HD_ELGROUP_INLINE,	// TITLE
		  HD_ELGROUP_TABLE | HD_ELGROUP_ROWCOL,
					// TR
		  HD_ELGROUP_INLINE,	// TT
		  HD_ELGROUP_INLINE,	// U
		  HD_ELGROUP_LIST,	// UL
		  HD_ELGROUP_INLINE,	// VAR
		  HD_ELGROUP_NONE	// WBR
		};


//
// Local functions.
//

static int	compare_elements(char **m0, char **m1);
static int	compare_variables(hdTreeAttr *v0, hdTreeAttr *v1);



//
// 'hdTree::hdTree()' - Create a new tree node.
//

hdTree::hdTree(hdElement  e,		// I - Element type
               const char *d,		// I - Data, if any
	       hdTree     *p)		// I - Parent node, if any
{
  // Clear the node structure...
  memset(this, 0, sizeof(hdTree));

  // Initialize the parent, element, and data, if any...
  element = e;

  if (d)
    data = strdup(d);

  if (p)
  {
    style = p->style;
    link  = p->link;

    add(p);
  }
}


//
// 'hdTree::~hdTree()' - Destroy a tree node.
//

hdTree::~hdTree()
{
  // Free any data this node has...
  if (data)
    free(data);

  // Destroy the child nodes...
  if (child)
    delete child;

  // Destroy nodes that follow...
  if (next)
    delete next;
}


//
// 'hdTree::add()' - Add a node to the end of a parent.
//

void
hdTree::add(hdTree *p)			// I - Parent node
{
  // Remove this node from the previous parent, as needed...
  if (parent)
    remove();

  // Add the node to the new parent...
  if (p->last_child)
  {
    p->last_child->next = this;
    prev                = p->last_child;
  }
  else
  {
    p->child = this;
    prev     = NULL;
  }

  parent        = p;
  p->last_child = this;
  next          = NULL;
}


//
// 'hdTree::compute_size()' - Compute the width and height of a node.
//

void
hdTree::compute_size(hdStyleSheet *css)	// I - Stylesheet
{
  const char	*width_ptr,		// Pointer to width string
		*height_ptr,		// Pointer to height string
		*size_ptr,		// Pointer to size string
		*type_ptr;		// Pointer to spacer type string
  hdImage	*img;			// Image
  char		number[255];		// Width or height value


  // Don't even bother if we don't have a style for this node yet...
  if (style == NULL)
  {
    width  = 0.0f;
    height = 0.0f;
    return;
  }

  switch (element)
  {
    case HD_ELEMENT_IMG :
	// Load an image
	width_ptr  = get_attr("WIDTH");
	height_ptr = get_attr("HEIGHT");
	img        = hdImage::find(get_attr("_HD_SRC"), css->grayscale);

	if (width_ptr != NULL && height_ptr != NULL)
	{
	  width  = style->get_length(width_ptr, css->media.page_print_width, css);
	  height = style->get_length(height_ptr, css->media.page_print_length, css);
	}
	else if (img == NULL || img->width() == 0 || img->height() == 0)
	  return;
	else if (width_ptr != NULL)
	{
	  // Scale the height so that the aspect ratio is preserved...
	  width  = style->get_length(width_ptr, css->media.page_print_width, css);
	  height = width * img->height() / img->width();

	  // Set the HEIGHT attribute appropriately...
	  sprintf(number, "%d", 
        	  atoi(width_ptr) * img->height() / img->width());

	  if (strchr(width_ptr, '%') != NULL)
            strcat(number, "%");

	  set_attr("HEIGHT", number);
	}
	else if (height_ptr != NULL)
	{
	  // Scale the width so that the aspect ratio is preserved...
	  height = style->get_length(height_ptr, css->media.page_print_length, css);
	  width  = height * img->width() / img->height();

	  // Set the WIDTH attribute appropriately...
	  sprintf(number, "%d",
        	  atoi(height_ptr) * img->width() / img->height());

	  if (strchr(height_ptr, '%') != NULL)
            strcat(number, "%");

	  set_attr("WIDTH", number);
	}
	else
	{
	  // Use the normal image size...
	  width  = img->width() * 72.0f / css->ppi;
	  height = img->height() * 72.0f / css->ppi;

	  // Set the WIDTH and HEIGHT attributes appropriately...
	  sprintf(number, "%d", img->width());
	  set_attr("WIDTH", number);

	  sprintf(number, "%d", img->height());
	  set_attr("HEIGHT", number);
	}
	break;

    case HD_ELEMENT_SPACER :
	width_ptr  = get_attr("WIDTH");
	height_ptr = get_attr("HEIGHT");
	size_ptr   = get_attr("SIZE");
	type_ptr   = get_attr("TYPE");

	if (width_ptr != NULL)
	  width = style->get_length(width_ptr, css->media.page_print_width, css);
	else if (size_ptr != NULL)
	  width = style->get_length(size_ptr, css->media.page_print_width, css);
	else
	  width = 1.0f;

	if (height_ptr != NULL)
	  height = style->get_length(height_ptr, css->media.page_print_length, css);
	else if (size_ptr != NULL)
	  height = style->get_length(size_ptr, css->media.page_print_length, css);
	else
	  height = 1.0f;

	if (type_ptr == NULL)
	  break;

	if (strcasecmp(type_ptr, "horizontal") == 0)
	  height = 0.0f;
	else if (strcasecmp(type_ptr, "vertical") == 0)
	  width = 0.0f;
	break;

    case HD_ELEMENT_BR :
	width  = 0.0f;
	height = style->font_size;
	break;

    case HD_ELEMENT_NONE :
        if (style->font && data)
	{
	  width  = style->font->get_width(data) * style->font_size;
	  height = style->font_size;
	}
	else
	{
	  width  = 0.0f;
	  height = 0.0f;
	}
	break;

    default :
        width  = 0.0f;
	height = 0.0f;
        break;
  }
}


//
// 'hdTree::copy_text()' - Copy all text nodes to a new parent.
//

void
hdTree::copy_text(hdStyleSheet *css,	// I - Stylesheet
                  hdTree *p)		// I - New parent
{
  hdTree	*t,			// Current node
		*end,			// End node
		*copyt;			// Copy of current node


  // Collect the current node and all that follow
  end = real_next(0);

  for (t = child; t && t != end; t = t->real_next())
    if (t->element == HD_ELEMENT_NONE)
    {
      copyt = new hdTree(HD_ELEMENT_NONE, t->data, p);
      copyt->whitespace = t->whitespace;
      copyt->compute_size(css);
    }
}


//
// 'hdTree::find()' - Find an element...
//

hdTree *				// O - Matching node or NULL
hdTree::find(hdElement e)		// I - Element to find
{
  hdTree	*t,			// Current node
		*end;			// End node


  // Scan the tree for the element...
  end = real_next(0);

  for (t = child; t && t != end; t = t->real_next())
    if (t->element == e)
      return (t);

  // Return the match, if any...
  return (NULL);
}


//
// 'hdTree::fix_url()' - Fix a URL so that the path is resolved directly.
//

char *					// O - Fixed URL
hdTree::fix_url(const char *url,	// I - URL
                const char *base,	// I - Base path for URL
                const char *path,	// I - Search path for URL
                char       *s,		// O - Fixed URL buffer
		int        slen)	// I - Size of URL buffer
{
  char	*slash;				// Location of slash
  char	temp[1024];			// Temporary buffer for final find


  // Range check...
  if (url == NULL)
    return (NULL);

  // Grab a local file or an absolute http/https URL...
  if (base == NULL || strcmp(base, ".") == 0 || strstr(url, "//") != NULL)
    return (hdFile::find(path, url, s, slen));

  // Strip leading ./ from URL...
  if (strncmp(url, "./", 2) == 0 ||
      strncmp(url, ".\\", 2) == 0)
    url += 2;

  // Make sure the last char in the URL output buffer is 0...
  slen --;
  s[slen] = '\0';

  // Copy the base path to the output buffer...
  if (strncmp(base, "http://", 7) == 0)
  {
    // Base path is a URL...
    strncpy(s, base, slen - 1);
    base = s + 7;

    // Handle local or remote paths...
    if (url[0] == '/')
    {
      if ((slash = strchr(base, '/')) != NULL)
        strncpy(slash, url, slen);
      else
        strncat(s, url, slen);

      return (s);
    }
    else if ((slash = strchr(base, '/')) == NULL)
      strncat(s, "/", slen);
  }
  else
  {
    // Base path is a directory...
    if (url[0] == '/' || url[0] == '\\' || base[0] == '\0' ||
        (isalpha(url[0]) && url[1] == ':'))
      return (hdFile::find(path, url, s, slen + 1)); // No change needed for absolute path

    strncpy(s, base, slen);
    base = s;
  }

  // Now handle relative paths in the URL as needed...
  while (strncmp(url, "../", 3) == 0 ||
         strncmp(url, "..\\", 3) == 0)
  {
    url += 3;

    if ((slash = strrchr(base, '/')) != NULL)
      *slash = '\0';
    else if ((slash = strrchr(base, '\\')) != NULL)
      *slash = '\0';
    else
    {
      url -= 3;
      break;
    }
  }

  // Now construct a temporary path for searching and return the result.
  snprintf(temp, sizeof(temp), "%s/%s", s, url);

  return (hdFile::find(path, temp, s, slen + 1));
}


//
// 'hdTree::get_attr()' - Get an attribute.
//

const char *				// O - Attribute value
hdTree::get_attr(const char *name)	// I - Attribute name
{
  hdTreeAttr	key,			// Search key
		*match;			// Matching variable


  // Check parameters and the current node first...
  if (name == NULL || !name[0] || nattrs == 0)
    return (NULL);

  // OK, now that we have a chance of returning something, create the
  // search key and see if the attribute exists...
  key.name = (char *)name;
  match    = (hdTreeAttr *)bsearch(&key, attrs, nattrs, sizeof(hdTreeAttr),
                                   (hdCompareFunc)compare_variables);

  // Return the match, if any.  If the attribute is defined but has no
  // value associated with it, then return an empty string.
  if (match == NULL)
    return (NULL);
  else if (match->value == NULL)
    return ("");
  else
    return (match->value);
}


//
// 'hdTree::get_element()' - Get the element index for the named element.
//

hdElement				// O - Element index
hdTree::get_element(const char *name)	// I - Element name
{
  char	**temp;				// Element pointer


  // Empty element names correspond to class/id "wildcard" definitions
  // in stylesheets, so always return HD_ELEMENT_NONE for these...
  if (!name[0])
    return (HD_ELEMENT_NONE);

  // Otherwise lookup the element name from the (sorted) array of
  // element names...
  temp = (char **)bsearch(&name, elements,
                          sizeof(elements) / sizeof(elements[0]),
                          sizeof(elements[0]),
                          (hdCompareFunc)compare_elements);

  if (temp)
    return ((hdElement)(temp - (char **)elements));
  else
    return (HD_ELEMENT_UNKNOWN);
}


//
// 'hdTree::get_meta()' - Find meta data in the document tree.
//

const char *				// O - Meta data or NULL
hdTree::get_meta(const char *name)	// I - Name of meta data
{
  hdTree	*t,			// Current node
		*end;			// End node
  const char	*tname,			// Name value from tree node
		*tcontent;		// Content value from tree node


  // Look here and in child nodes for the desired meta data...
  end = real_next(0);

  for (t = this; t != end; t = t->real_next())
  {
    // Check this node...
    if (t->element == HD_ELEMENT_META &&
        (tname = t->get_attr("NAME")) != NULL &&
        (tcontent = t->get_attr("CONTENT")) != NULL)
    {
      // We have meta data; is this the data we seek?
      if (strcasecmp(name, tname) == 0)
      {
        // Yes, return it...
        return (tcontent);
      }
    }
  }

  return (NULL);
}


//
// 'hdTree::get_text()' - Get a copy of the text fragments under a node.
//

char *					// O - Text under the node.
hdTree::get_text(bool comments)		// I - Include comments, too?
{
  char		*s;			// String
  int		slen,			// Length of string
		tlen;			// Length of node string
  hdTree	*t;			// Current node


  // See if we already have the text for this node...
  if (element == HD_ELEMENT_COMMENT && !comments)
    return (NULL);
  else if (data)
    return (data);
  else if (elgroup[element] == HD_ELGROUP_NONE)
    return (NULL);

  slen = 0;
  s    = NULL;

  for (t = child; t; t = t->next)
    if (t->get_text(comments))
    {
      // Add the text to this string...
      tlen = strlen(t->data);
      if (t->whitespace && slen)
        tlen ++;

      if (slen)
        s = (char *)realloc(data, 1 + slen + tlen);
      else
        s = (char *)malloc(1 + tlen);

      if (!s)
        break;

      if (t->whitespace && slen)
      {
        s[slen] = ' ';
        strcpy(s + slen + 1, t->data);
      }
      else
        strcpy(s + slen, t->data);

      slen += tlen;

      data = s;
    }

  return (data);
}


//
// 'hdTree::get_title()' - Get the first title node...
//

char *					// O - Text for the title...
hdTree::get_title()
{
  hdTree	*title;			// Title node


  if ((title = find(HD_ELEMENT_TITLE)) != NULL)
    return (title->get_text());
  else
    return (NULL);
}


//
// 'hdTree::insert()' - Insert a node at the beginning of a parent.
//

void
hdTree::insert(hdTree *p)		// I - Parent node
{
  // Remove this node from the previous parent, as needed...
  if (parent)
    remove();

  // Insert the node in the new parent...
  if (p->child)
  {
    p->child->prev = this;
    next           = p->child;
  }
  else
  {
    p->last_child = this;
    next          = NULL;
  }

  parent   = p;
  p->child = this;
  prev     = NULL;
}


//
// 'hdTree::parse_attribute()' - Parse an attribute.
//

bool					// O - True on success, false on error
hdTree::parse_attribute(hdFile *fp)	// I - File to read from
{
  char	name[1024],			// Name of variable
	value[10240],			// Value of variable
	*ptr;				// Temporary pointer
  int	ch;				// Character from file


  ptr = name;
  while ((ch = fp->get()) != EOF)
    if (isspace(ch) || ch == '=' || ch == '>' || ch == '\r')
      break;
    else if (ptr < (name + sizeof(name) - 1))
      *ptr++ = ch;

  *ptr = '\0';

  while (isspace(ch) || ch == '\r')
    ch = fp->get();

  switch (ch)
  {
    default :
        fp->unget(ch);
	set_attr(name, NULL);
        return (true);

    case EOF :
        return (false);

    case '=' :
        ptr = value;
        ch  = fp->get();

        while (isspace(ch) || ch == '\r')
          ch = fp->get();

        if (ch == EOF)
          return (-1);

        if (ch == '\'')
        {
          while ((ch = fp->get()) != EOF)
            if (ch == '\'')
              break;
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
              *ptr++ = ch;

          *ptr = '\0';
        }
        else if (ch == '\"')
        {
          while ((ch = fp->get()) != EOF)
            if (ch == '\"')
              break;
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
              *ptr++ = ch;

          *ptr = '\0';
        }
        else
        {
          *ptr++ = ch;
          while ((ch = fp->get()) != EOF)
            if (isspace(ch) || ch == '>' || ch == '\r')
              break;
            else if (ptr < (value + sizeof(value) - 1))
              *ptr++ = ch;

          *ptr = '\0';
          if (ch == '>')
            fp->unget(ch);
        }

	set_attr(name, value);
        return (true);
  }
}


//
// 'hdTree::parse_element()' - Parse an element.
//

hdElement				// O - Element number
hdTree::parse_element(hdFile *fp)	// I - File to read from
{
  int	ch, ch2;			// Characters from file
  char	m[255],				// Element string...
	*mptr,				// Current char...
	comment[10240],			// Comment string
	*cptr;				// Current char...


  // Read the element name...
  for (mptr = m; (ch = fp->get()) != EOF;)
    if (ch == '>' || isspace(ch))
      break;
    else if (mptr < (m + sizeof(m) - 1))
    {
      // Add the character to the element name...
      *mptr++ = ch;

      // Handle comments without whitespace...
      if ((mptr - m) == 3 && strncmp(m, "!--", 3) == 0)
      {
        ch = fp->get();
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (HD_ELEMENT_ERROR);

  // Now lookup the element...
  element = get_element(m);

  if (element == HD_ELEMENT_UNKNOWN)
  {
    // Unrecognized element, store in data...
    strcpy(comment, m);
    cptr = comment + strlen(m);
  }
  else
    cptr = comment;

  if (element == HD_ELEMENT_COMMENT || element == HD_ELEMENT_UNKNOWN)
  {
    // Read comments and unknown elements...
    while (ch != EOF && cptr < (comment + sizeof(comment) - 1))
    {
      if (ch == '>' && element == HD_ELEMENT_UNKNOWN)
        break;

      *cptr++ = ch;

      if (ch == '-')
      {
        if ((ch2 = fp->get()) == '>')
	{
	  // Erase trailing -->
	  cptr --;
	  if (*cptr == '-' && cptr > comment)
	    cptr --;

	  break;
        }
	else
	  ch = ch2;
      }
      else
        ch = fp->get();
    }

    // Store the comment/element text in the data field...
    *cptr = '\0';
    data  = strdup(comment);
  }
  else
  {
    // Parse the attributes that go along with the element...
    while (ch != EOF && ch != '>')
    {
      if (!isspace(ch))
      {
        fp->unget(ch);
        parse_attribute(fp);
      }

      ch = fp->get();
    }
  }

  return (element);
}


//
// 'hdTree::parse_entity()' - Parse a HTML entity and return the text for it.
//

void
hdTree::parse_entity(hdFile       *fp,	// I - File to read from
                     hdStyleSheet *css,	// I - Stylesheet
	             char         *s,	// O - Glyph string buffer
		     int          slen)	// I - Size of glyph string buffer
{
  int		ch,		// Character from file...
		code;		// Code for entity...
  char		html[16],	// Entity name (&#nnn; or &name;)
		*ptr;		// Pointer in entity string
  const char	*glyph;		// PostScript glyph...


  // Make sure the string buffer is nul-terminated...
  slen --;
  s[slen] = '\0';

  // Now read all of the entity text; if the first char is invalid
  // (not a letter or #), then unget the one character and put an
  // ampersand in the output string.  Files that don't quote the
  // ampersands are not conformant, but unfortunately exist...
  ch = fp->get();

  if (!isalpha(ch) && ch != '#')
  {
    // Bad ampersand in file...
    fp->unget(ch);

    // Put an ampersand in the string buffer...
    s[0] = '&';
    s[1] = '\0';

    fprintf(stderr, "Bad character sequence \"&%c\"!\n", ch);
    return;
  }

  ptr = html;

  if (ch == '#')
  {
    *ptr++ = ch;
    ch     = fp->get();

    while (isdigit(ch))
    {
      if (ptr < (html + sizeof(html) - 1))
	*ptr++ = ch;

      ch = fp->get();
    }
  }
  else
  {    
    while (isalnum(ch))
    {
      if (ptr < (html + sizeof(html) - 1))
	*ptr++ = ch;

      ch = fp->get();
    }
  }

  *ptr = '\0';

  if (ch != ';')
  {
    fp->unget(ch);

    snprintf(s, slen + 1, "&%s", html);
    return;
  }

  if (html[0] == '#')
  {
    // &#NNNN; quotes a raw character value in the current charset...
    code = atoi(html + 1);
  }
  else
  {
    // Start by lookup up the PostScript glyph...
    glyph = hdGlobal.find_glyph(html);

    // Then look at the current charset for the code associated
    // with the glyph...
    for (code = 0; code < css->num_glyphs; code ++)
      if (css->glyphs[code] && strcmp(glyph, css->glyphs[code]) == 0)
        break;
  }

  if (code >= css->num_glyphs)
  {
    // The code can't be represented by the current character set,
    // so just return the text...
    snprintf(s, slen + 1, "&%s;", html);
    return;
  }

  // For all 8-bit charsets, we just put a single character in the
  // buffer.  Otherwise we have to decompose the character code
  // into the correct UTF-8 stream...
  if (css->encoding == HD_FONTENCODING_8BIT || code < 128)
  {
    // 7-bit ASCII value
    s[0] = code;
    s[1] = '\0';
  }
  else if (code < 2048)
  {
    // 11-bit Unicode value
    s[0] = 0xc0 | (code >> 6);
    s[1] = 0x80 | (code & 63);
    s[2] = '\0';
  }
  else if (code < 65536)
  {
    // 16-bit Unicode value
    s[0] = 0xe0 | (code >> 12);
    s[1] = 0x80 | ((code >> 6) & 63);
    s[2] = 0x80 | (code & 63);
    s[3] = '\0';
  }
  else
  {
    // 21-bit Unicode value
    s[0] = 0xf0 | (code >> 18);
    s[1] = 0x80 | ((code >> 12) & 63);
    s[2] = 0x80 | ((code >> 6) & 63);
    s[3] = 0x80 | (code & 63);
    s[4] = '\0';
  }
}


//
// 'hdTree::read()' - Read a HTML file into a document tree.
//

hdTree *				// O - New document tree
hdTree::read(hdFile       *fp,		// I - File to read from
	     const char   *base,	// I - Base path for file
             const char   *path,	// I - Additional search paths
             hdStyleSheet *css)		// I - Stylesheet
{
  int			ch;		// Character from file
  char			*ptr;		// Pointer in string
  hdTree		*t,		// New tree node
			*p,		// Parent tree node
			*temp;		// Temporary tree node
  hdStyle		*style;		// Current style
  hdStyleSelector	selector;	// Style selector...
  hdFile		*embed;		// File pointer for EMBED
  char			newbase[1024];	// New base directory for EMBED
  const char		*filename,	// Filename for EMBED tag
			*align,		// Horizontal/vertical alignment
			*color,		// Color for FONT tag
			*face,		// Typeface for FONT tag
			*size;		// Size for FONT tag
  int			sizeval;	// Size value from FONT tag
  char			s[10240];	// String from file
  bool			whitespace;	// Leading whitespace?


  // The initial parent node is the FILE pseudo-element...
  p = new hdTree(HD_ELEMENT_FILE);

  // Parse data until we hit end-of-file...
  whitespace = false;

  while ((ch = fp->get()) != EOF)
  {
    // Ignore leading whitespace...
    if (p->style == NULL || p->style->white_space != HD_WHITESPACE_PRE)
    {
      while (isspace(ch))
      {
        whitespace = true;
        ch         = fp->get();
      }

      if (ch == EOF)
        break;
    }

    // Allocate a new empty tree node...
    t = new hdTree(HD_ELEMENT_NONE, NULL, p);

    // See what the character was...
    if (ch == '<')
    {
      // Markup char; grab the next char to see if this is a /...
      ch = fp->get();

      if (isspace(ch) || ch == '=' || ch == '<')
      {
        // Sigh...  "<" followed by anything but an element name is
	// invalid HTML, but so many people have asked for this to
	// be supported that we have added this hack...
        t->whitespace = whitespace;
	whitespace    = false;

	ptr = s;
        *ptr++ = '<';
	if (ch == '=')
	  *ptr++ = '=';
	else if (ch == '<')
	  fp->unget(ch);
	else
	  whitespace = true;

	*ptr++ = '\0';

	t->data = strdup(s);
      }
      else
      {
        // Start of a element...
	if (ch != '/')
          fp->unget(ch);

	if (t->parse_element(fp) == HD_ELEMENT_ERROR)
	{
	  t->remove();
	  delete t;
          break;
	}

        // Eliminate extra whitespace...
	if (hdElIsGroup(t->element) || hdElIsBlock(t->element) ||
            hdElIsList(t->element) || hdElIsItem(t->element) ||
            hdElIsTable(t->element) || hdElIsCell(t->element) ||
	    t->element == HD_ELEMENT_TITLE)
          whitespace = false;

        // If this is the matching close mark, or if we are starting the
	// same element, or if we've completed a list, we're done!
	if (ch == '/')
	{
	  // Close element; find matching element...
          for (temp = p; temp != NULL; temp = temp->parent)
            if (temp->element == t->element)
              break;

	  // Then delete the closing element...
          t->remove();
	  delete t;

          // If this is a STYLE element, load the style data...
	  if (temp != NULL && temp->element == HD_ELEMENT_STYLE)
	  {
	    // Get the text and create a memory file...
	    hdMemFile *mf = new hdMemFile(temp->get_text(1));
	    css->load(mf);
	    delete mf;
	  }
	}
	else if (hdElIsGroup(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
	    if (hdElIsCell(temp->element))
	    {
	      temp = NULL;
              break;
	    }
	}
	else if (hdElIsList(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsBlock(temp->element))
	      break;
	    else if (hdElIsItem(temp->element) || hdElIsCell(temp->element) ||
	             hdElIsGroup(temp->element))
	    {
	      temp = NULL;
              break;
	    }

	}
	else if (hdElIsItem(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsItem(temp->element) || hdElIsBlock(temp->element))
              break;
	    else if (hdElIsList(temp->element) || hdElIsGroup(temp->element) ||
	             hdElIsCell(temp->element))
            {
	      temp = NULL;
	      break;
	    }
	}
	else if (hdElIsBlock(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsBlock(temp->element))
              break;
	    else if (hdElIsCell(temp->element) || hdElIsList(temp->element) ||
	             hdElIsGroup(temp->element) || hdElIsItem(temp->element))
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (hdElIsRowCol(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsRowCol(temp->element))
	      break;
	    else if (temp->element == HD_ELEMENT_TABLE)
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (hdElIsCell(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsCell(temp->element))
              break;
	    else if (hdElIsTable(temp->element))
	    {
	      temp = NULL;
              break;
	    }
	}
	else
          temp = NULL;

	if (temp != NULL)
	{
	  // Set the current parent node to the parent of the 
	  // original (opening) element...
          p = temp->parent;

          if (ch == '/')
	    continue;

	  // Reset the parent of this node to the parent of the
	  // opening element...

	  t->add(p);
	  t->style = p->style;
	}
	else if (ch == '/')
	  continue;
      }
    }
    else if (p->style != NULL && p->style->white_space == HD_WHITESPACE_PRE)
    {
      // Read a pre-formatted string into the current tree node...
      ptr = s;
      while (ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
        if (ch == '&')
	{
	  t->parse_entity(fp, css, ptr, sizeof(s) - (ptr - s));
	  ptr += strlen(ptr);
	}
	else if (ch != '\r')
          *ptr++ = ch;

        if (ch == '\n')
          break;

        ch = fp->get();
      }

      *ptr = '\0';

      if (ch == '<')
        fp->unget(ch);

      t->data = strdup(s);
    }
    else
    {
      // Read the next string fragment...
      ptr           = s;
      t->whitespace = whitespace;
      whitespace    = false;

      while (!isspace(ch) && ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
        if (ch == '&')
	{
	  t->parse_entity(fp, css, ptr, sizeof(s) - (ptr - s));
	  ptr += strlen(ptr);
	}
	else
          *ptr++ = ch;

        ch = fp->get();
      }

      if (isspace(ch))
        whitespace = true;

      *ptr = '\0';

      if (ch == '<')
        fp->unget(ch);

      t->data = strdup(s);
    }

    // Create a private copy of the style data as needed...
    if (t->element >= HD_ELEMENT_A)
      t->style = css->get_private_style(t);

    // Do element-specific stuff...
    switch (t->element)
    {
      case HD_ELEMENT_BODY :
          // Update the text color as necessary...
          if ((color = t->get_attr("TEXT")) != NULL)
          {
	    t->style->color_set = 1;
	    t->style->get_color(color, t->style->color);
	  }

          if ((color = t->get_attr("LINK")) != NULL)
          {
	    selector.element = HD_ELEMENT_A;
	    selector.class_  = NULL;
	    selector.pseudo  = "link";
	    selector.id      = NULL;

	    style = css->find_style(1, &selector, 1);
	    if (style)
	    {
	      style->border[HD_POS_BOTTOM].color_set = 1;
	      style->get_color(color, style->border[HD_POS_BOTTOM].color);
	    }
	  }

          if ((color = t->get_attr("ALINK")) != NULL)
          {
	    selector.element = HD_ELEMENT_A;
	    selector.class_  = NULL;
	    selector.pseudo  = "active";
	    selector.id      = NULL;

	    style = css->find_style(1, &selector, 1);
	    if (style)
	    {
	      style->border[HD_POS_BOTTOM].color_set = 1;
	      style->get_color(color, style->border[HD_POS_BOTTOM].color);
	    }
	  }

          if ((color = t->get_attr("VLINK")) != NULL)
          {
	    selector.element = HD_ELEMENT_A;
	    selector.class_  = NULL;
	    selector.pseudo  = "visited";
	    selector.id      = NULL;

	    style = css->find_style(1, &selector, 1);
	    if (style)
	    {
	      style->border[HD_POS_BOTTOM].color_set = 1;
	      style->get_color(color, style->border[HD_POS_BOTTOM].color);
	    }
	  }

          // Update the background attributes as necessary...
          if ((color = t->get_attr("BGCOLOR")) != NULL)
          {
	    t->style->background_color_set = 1;
	    t->style->get_color(color, t->style->background_color);
	  }

          if ((filename = t->get_attr("BACKGROUND")) != NULL)
	  {
	    if ((filename = fix_url(filename, base, path, s, sizeof(s))) != NULL)
	      t->style->background_image = strdup(filename);
	  }
          break;

      case HD_ELEMENT_IMG :
          // Get the image alignment...
	  if ((align = t->get_attr("ALIGN")) != NULL)
	  {
	    if (strcasecmp(align, "left") == 0)
	      t->style->float_ = HD_FLOAT_LEFT;
	    else if (strcasecmp(align, "right") == 0)
	      t->style->float_ = HD_FLOAT_RIGHT;
	    else if (strcasecmp(align, "top") == 0)
	    {
	      t->style->float_         = HD_FLOAT_NONE;
	      t->style->vertical_align = HD_VERTICALALIGN_TOP;
	    }
	    else if (strcasecmp(align, "middle") == 0)
	    {
	      t->style->float_         = HD_FLOAT_NONE;
	      t->style->vertical_align = HD_VERTICALALIGN_MIDDLE;
	    }
	    else if (strcasecmp(align, "bottom") == 0)
	    {
	      t->style->float_         = HD_FLOAT_NONE;
	      t->style->vertical_align = HD_VERTICALALIGN_BOTTOM;
	    }
	  }

          // Update the image source as necessary...
          if ((filename = t->get_attr("SRC")) != NULL)
	  {
	    if ((filename = fix_url(filename, base, path, s, sizeof(s))) != NULL)
	      t->set_attr("_HD_SRC", filename);
          }

 	  // Figure out the width & height of this element...
          t->compute_size(css);
	  break;

      case HD_ELEMENT_BR :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
 	  // Figure out the width & height of this element...
          t->compute_size(css);
	  break;

      case HD_ELEMENT_EMBED :
          if ((filename = t->get_attr("SRC")) != NULL)
	  {
	    fix_url(filename, base, path, s, sizeof(s));

            if ((embed = hdFile::open(s, HD_FILE_READ)) != NULL)
            {
	      hdFile::directory(s, newbase, sizeof(newbase));

	      if ((temp = read(embed, newbase, path, css)) != NULL)
	        temp->add(t);

	      delete embed;
            }
	    else
	      hdGlobal.progress_error(HD_ERROR_FILE_NOT_FOUND,
                                      "Unable to embed \"%s\" - %s", filename,
	                              strerror(errno));
	  }
          break;

      case HD_ELEMENT_FONT :
          if (t->style == NULL)
	    break;

          if ((face = t->get_attr("FACE")) != NULL)
          {
	    // Reset the font_family attribute...
	    if (t->style->font_family)
	      free(t->style->font_family);

	    t->style->font_family = strdup(face);
	    t->style->updated     = 0;

            // Update the style data...
	    t->style->update(css);
          }

          if ((color = t->get_attr("COLOR")) != NULL)
	  {
	    t->style->color_set = 1;
	    t->style->get_color(color, t->style->color);
	  }

          if ((size = t->get_attr("SIZE")) != NULL)
          {
	    if (isdigit(size[0]))
	    {
	      // Absolute size change...
	      sizeval = atoi(size) - 3;

	      // Get the base font size from the BODY element...
	      selector.element = HD_ELEMENT_BODY;
	      selector.class_  = NULL;
	      selector.pseudo  = NULL;
	      selector.id      = NULL;

	      if ((style = css->find_style(1, &selector, 1)) != NULL)
	        t->style->font_size = style->font_size * pow(1.2f, sizeval);
              else
	        t->style->font_size = 11.0f * pow(1.2f, sizeval);
	    }
	    else
	    {
	      // Relative size change...
              sizeval = atoi(size);

              t->style->font_size *= pow(1.2f, sizeval);
	    }
          }
          break;

      case HD_ELEMENT_TABLE :
          // Get the table alignment...
	  if ((align = t->get_attr("ALIGN")) != NULL)
	  {
	    if (strcasecmp(align, "left") == 0)
	      t->style->float_ = HD_FLOAT_LEFT;
	    else if (strcasecmp(align, "right") == 0)
	      t->style->float_ = HD_FLOAT_RIGHT;
	  }
	  break;

      case HD_ELEMENT_TD :
      case HD_ELEMENT_TH :
      case HD_ELEMENT_TR :
          // Handle vertical alignment and background attributes...
	  if ((align = t->get_attr("VALIGN")) != NULL)
	  {
	    if (strcasecmp(align, "top") == 0)
	      t->style->vertical_align = HD_VERTICALALIGN_TOP;
	    else if (strcasecmp(align, "middle") == 0)
	      t->style->vertical_align = HD_VERTICALALIGN_MIDDLE;
	    else if (strcasecmp(align, "bottom") == 0)
	      t->style->vertical_align = HD_VERTICALALIGN_BOTTOM;
	  }

          if ((color = t->get_attr("BGCOLOR")) != NULL)
          {
	    t->style->background_color_set = 1;
	    t->style->get_color(color, t->style->background_color);
	  }

          if ((filename = t->get_attr("BACKGROUND")) != NULL)
	  {
	    if ((filename = fix_url(filename, base, path, s, sizeof(s))) != NULL)
	      t->style->background_image = strdup(filename);
	  }

      default :
          // Handle alignment...
	  if ((align = t->get_attr("ALIGN")) != NULL)
	  {
	    if (strcasecmp(align, "left") == 0)
	      t->style->text_align = HD_TEXTALIGN_LEFT;
	    else if (strcasecmp(align, "right") == 0)
	      t->style->text_align = HD_TEXTALIGN_RIGHT;
	    else if (strcasecmp(align, "center") == 0)
	      t->style->text_align = HD_TEXTALIGN_CENTER;
	    else if (strcasecmp(align, "justify") == 0)
	      t->style->text_align = HD_TEXTALIGN_JUSTIFY;
	  }
	  break;
    }

    // Descend for all but leaf elements...
    if (!hdElIsNone(t->element))
      p = t;
  }  

  // End of file, find the parent node for non-compliant HTML files...
  while (p->parent != NULL)
    p = p->parent;

  // Return the HTML 
  return (p);
}


//
// 'hdTree::real_next()' - Find next logical node in the tree.
//

hdTree *				// O - Next logical node
hdTree::real_next(bool descend)		// I - Descend into children?
{
  hdTree	*t;			// Current node


  // Start at the current node and find the next logical node in
  // the tree...
  if (child != NULL && descend)
    return (child);

  if (next != NULL)
    return (next);

  for (t = parent; t; t = t->parent)
    if (t->next)
      return (t->next);

  return (NULL);
}


//
// 'hdTree::real_next()' - Find previous logical node in the tree.
//

hdTree *				// O - Previous logical node
hdTree::real_prev()
{
  // Start at the current node and find the previous logical node in
  // the tree...
  if (prev != NULL)
    return (prev);
  else
    return (parent);
}


//
// 'hdTree::remove()' - Remove a node from its parent.
//

void
hdTree::remove()
{
  if (!parent)
    return;

  if (prev)
    prev->next = next;
  else
    parent->child = next;

  if (next)
    next->prev = prev;
  else
    parent->last_child = prev;

  parent = NULL;
}


//
// 'hdTree::set_attr()' - Set an attribute value.
//

void
hdTree::set_attr(const char *name,	// I - Name of attribute
                 const char *value)	// I - Value of attribute
{
  hdTreeAttr	*match,			// Matching attribute
		key;			// Search key


  // Don't set anything if we don't need to...
  if (name == NULL || !name[0])
    return;

  // See if the attribute is already defined...
  if (nattrs == 0)
    match = NULL;
  else
  {
    // Search for it...
    key.name = (char *)name;
    match    = (hdTreeAttr *)bsearch(&key, attrs, nattrs, sizeof(hdTreeAttr),
        	                     (hdCompareFunc)compare_variables);
  }

  if (match == NULL)
  {
    if ((nattrs & 3) == 0)
    {
      // Allocate more memory...
      match = new hdTreeAttr[nattrs + 4];

      if (nattrs)
      {
        memcpy(match, attrs, nattrs * sizeof(hdTreeAttr));
	delete[] attrs;
      }

      attrs = match;
    }

    // Add a new attribute...
    match = attrs + nattrs;
    nattrs ++;

    match->name  = strdup(name);

    if (value != NULL)
      match->value = strdup(value);
    else
      match->value = NULL;

    // Set the "link" value if we have a link value...
    if (element == HD_ELEMENT_A && strcasecmp(name, "HREF") == 0)
      link = this;

    // Sort the attribute array as needed...
    if (nattrs > 1)
      qsort(attrs, nattrs, sizeof(hdTreeAttr),
            (hdCompareFunc)compare_variables);
  }
  else if (match->value != value)
  {
    // Replace the existing value...
    if (match->value != NULL)
      free(match->value);

    if (value != NULL)
      match->value = strdup(value);
    else
      match->value = NULL;
  }
}


//
// 'compare_elements()' - Compare two element strings...
//

static int				// O - Result of comparison
compare_elements(char **m0,		// I - First element
                 char **m1)		// I - Second element
{
  return (strcasecmp(*m0, *m1));
}


//
// 'compare_variables()' - Compare two element variables.
//

static int				// O - Result of comparison
compare_variables(hdTreeAttr *v0,	// I - First variable
                  hdTreeAttr *v1)	// I - Second variable
{
  return (strcasecmp(v0->name, v1->name));
}


//
// End of "$Id: tree.cxx,v 1.22.2.3 2004/03/30 03:49:16 mike Exp $".
//
