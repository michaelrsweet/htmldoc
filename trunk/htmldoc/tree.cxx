//
// "$Id: tree.cxx,v 1.3 2002/02/08 19:39:52 mike Exp $"
//
//   HTML parsing routines for HTMLDOC, a HTML document processing program.
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

#include "tree.h"
#include "hdstring.h"


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
		  "ol",
		  "option",
		  "p",
		  "pre",
		  "s",
		  "samp",
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
		  HD_ELGROUP_TABLE | HD_ELGROUP_ROWCOL,
					// COL
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
		  HD_ELGROUP_LIST,	// OL
		  HD_ELGROUP_GROUP,	// OPTION
		  HD_ELGROUP_BLOCK,	// P
		  HD_ELGROUP_BLOCK,	// PRE
		  HD_ELGROUP_INLINE,	// S
		  HD_ELGROUP_INLINE,	// SAMP
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


hdElement	hdTree::elparent[HD_ELEMENT_MAX] =
		{			// "Parent" element for inheritance
		  HD_ELEMENT_NONE,	// "NONE"
		  HD_ELEMENT_NONE,	// "FILE"
		  HD_ELEMENT_NONE,	// "ERROR"
		  HD_ELEMENT_NONE,	// "UNKNOWN"
		  HD_ELEMENT_NONE,	// !--
		  HD_ELEMENT_NONE,	// !DOCTYPE
		  HD_ELEMENT_NONE,	// A
		  HD_ELEMENT_NONE,	// ACRONYM
		  HD_ELEMENT_NONE,	// ADDRESS
		  HD_ELEMENT_NONE,	// AREA
		  HD_ELEMENT_NONE,	// B
		  HD_ELEMENT_NONE,	// BASE
		  HD_ELEMENT_NONE,	// BASEFONT
		  HD_ELEMENT_NONE,	// BIG
		  HD_ELEMENT_NONE,	// BLOCKQUOTE
		  HD_ELEMENT_NONE,	// BODY
		  HD_ELEMENT_NONE,	// BR
		  HD_ELEMENT_NONE,	// CAPTION
		  HD_ELEMENT_NONE,	// CENTER
		  HD_ELEMENT_NONE,	// CITE
		  HD_ELEMENT_NONE,	// CODE
		  HD_ELEMENT_NONE,	// COL
		  HD_ELEMENT_NONE,	// COLGROUP
		  HD_ELEMENT_NONE,	// DD
		  HD_ELEMENT_NONE,	// DEL
		  HD_ELEMENT_NONE,	// DFN
		  HD_ELEMENT_NONE,	// DIR
		  HD_ELEMENT_NONE,	// DIV
		  HD_ELEMENT_NONE,	// DL
		  HD_ELEMENT_NONE,	// DT
		  HD_ELEMENT_NONE,	// EM
		  HD_ELEMENT_NONE,	// EMBED
		  HD_ELEMENT_NONE,	// FONT
		  HD_ELEMENT_NONE,	// FORM
		  HD_ELEMENT_NONE,	// H1
		  HD_ELEMENT_NONE,	// H2
		  HD_ELEMENT_NONE,	// H3
		  HD_ELEMENT_NONE,	// H4
		  HD_ELEMENT_NONE,	// H5
		  HD_ELEMENT_NONE,	// H6
		  HD_ELEMENT_NONE,	// HEAD
		  HD_ELEMENT_NONE,	// HR
		  HD_ELEMENT_NONE,	// HTML
		  HD_ELEMENT_NONE,	// I
		  HD_ELEMENT_NONE,	// IMG
		  HD_ELEMENT_NONE,	// INPUT
		  HD_ELEMENT_NONE,	// INS
		  HD_ELEMENT_NONE,	// ISINDEX
		  HD_ELEMENT_NONE,	// KBD
		  HD_ELEMENT_NONE,	// LI
		  HD_ELEMENT_NONE,	// LINK
		  HD_ELEMENT_NONE,	// MAP
		  HD_ELEMENT_NONE,	// MENU
		  HD_ELEMENT_NONE,	// META
		  HD_ELEMENT_NONE,	// OL
		  HD_ELEMENT_NONE,	// OPTION
		  HD_ELEMENT_NONE,	// P
		  HD_ELEMENT_NONE,	// PRE
		  HD_ELEMENT_NONE,	// S
		  HD_ELEMENT_NONE,	// SAMP
		  HD_ELEMENT_NONE,	// SELECT
		  HD_ELEMENT_NONE,	// SMALL
		  HD_ELEMENT_NONE,	// SPACER
		  HD_ELEMENT_NONE,	// SPAN
		  HD_ELEMENT_NONE,	// STRIKE
		  HD_ELEMENT_NONE,	// STRONG
		  HD_ELEMENT_NONE,	// STYLE
		  HD_ELEMENT_NONE,	// SUB
		  HD_ELEMENT_NONE,	// SUP
		  HD_ELEMENT_NONE,	// TABLE
		  HD_ELEMENT_NONE,	// TBODY
		  HD_ELEMENT_NONE,	// TD
		  HD_ELEMENT_NONE,	// TEXTAREA
		  HD_ELEMENT_NONE,	// TFOOT
		  HD_ELEMENT_NONE,	// TH
		  HD_ELEMENT_NONE,	// THEAD
		  HD_ELEMENT_NONE,	// TITLE
		  HD_ELEMENT_NONE,	// TR
		  HD_ELEMENT_NONE,	// TT
		  HD_ELEMENT_NONE,	// U
		  HD_ELEMENT_NONE,	// UL
		  HD_ELEMENT_NONE,	// VAR
		  HD_ELEMENT_NONE	// WBR
		};


//
// Local functions.
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

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
hdTree::compute_size()
{
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
  return ((char *)url);
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
                                   (compare_func_t)compare_variables);

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
                          (compare_func_t)compare_elements);

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
  return (NULL);
}


//
// 'hdTree::get_text()' - Get a copy of the text fragments under a node.
//

char *					// O - Text under the node.
hdTree::get_text()
{
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

int					// O - 0 on success, -1 on error
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
        return (0);

    case EOF :
        return (-1);

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
        return (0);
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
// 'hdTree::read()' - Read a HTML file into a document tree.
//

hdTree *				// O - New document tree
hdTree::read(hdFile       *fp,		// I - File to read from
	     const char   *base,	// I - Base path for file
             const char   *path,	// I - Additional search paths
             hdStyleSheet *css)		// I - Stylesheet
{
  int		ch;			// Character from file
  char		*ptr,			// Pointer in string
		glyph[16],		// Glyph name (&#nnn; or &name;)
		*glyphptr;		// Pointer in glyph string
  hdTree	*tree,			// "top" of this tree
		*t,			// New tree node
		*p,			// Parent tree node
		*temp;			// Temporary looping var
  hdStyle	*style;			// Current style
  hdSelector	selector;		// Style selector...
  int		pos;			// Current file position
  hdFile	*embed;			// File pointer for EMBED
  char		newbase[1024];		// New base directory for EMBED
  const char	*filename,		// Filename for EMBED tag
		*align,			// Horizontal/vertical alignment
		*color,			// Color for FONT tag
		*face,			// Typeface for FONT tag
		*size;			// Size for FONT tag
  int		sizeval;		// Size value from FONT tag
  char		s[10240];		// String from file
  int		whitespace;		// Leading whitespace?


  // The initial parent node is the FILE pseudo-element...
  p = new hdTree(HD_ELEMENT_FILE);

  // Parse data until we hit end-of-file...
  while ((ch = fp->get()) != EOF)
  {
    // Ignore leading whitespace...
    if (p->style == NULL || p->style->white_space != HD_WHITESPACE_PRE)
    {
      while (isspace(ch))
      {
        whitespace = 1;
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
	whitespace    = 0;

	ptr = s;
        *ptr++ = '<';
	if (ch == '=')
	  *ptr++ = '=';
	else if (ch == '<')
	  fp->unget(ch);
	else
	  whitespace = 1;

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
          whitespace = 0;

        t->whitespace = whitespace;
	whitespace    = 0;

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
            if (hdElIsBlock(temp->element) || hdElIsItem(temp->element))
              break;
	    else if (hdElIsCell(temp->element) || hdElIsList(temp->element) ||
	             hdElIsGroup(temp->element))
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (hdElIsTable(t->element))
	{
          for (temp = p; temp != NULL; temp = temp->parent)
            if (hdElIsTable(temp->element))
	      break;
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
#if 0
        if (ch == '&')
        {
          for (glyphptr = glyph;
               (ch = fp->get()) != EOF && (glyphptr - glyph) < 15;
               glyphptr ++)
            if (ch == ';' || isspace(ch))
              break;
            else
              *glyphptr = ch;

          *glyphptr = '\0';
          ch = iso8859(glyph);
        }
#endif // 0

        if (ch != 0 && ch != '\r')
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
      whitespace    = 0;

      while (!isspace(ch) && ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
#if 0
        if (ch == '&')
        {
          for (glyphptr = glyph;
               (ch = fp->get()) != EOF && (glyphptr - glyph) < 15;
               glyphptr ++)
            if (ch == ';' || isspace(ch))
              break;
            else
              *glyphptr = ch;

          *glyphptr = '\0';
          ch = iso8859(glyph);
        }
#endif // 0

        if (ch != 0)
          *ptr++ = ch;

        ch = fp->get();
      }

      if (isspace(ch))
        whitespace = 1;

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
	      t->set_attr("_HD_SRC", strdup(filename));
          }

 	  // Figure out the width & height of this element...
          t->compute_size();
	  break;

      case HD_ELEMENT_BR :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
 	  // Figure out the width & height of this element...
          t->compute_size();
	  break;

#if 0
      case HD_ELEMENT_EMBED :
          if ((filename = t->get_attr("SRC")) != NULL)
	  {
	    filename = (char *)fix_filename((char *)filename,
	                                     (char *)base);

            if ((embed = fopen((char *)filename, "r")) != NULL)
            {
	      strcpy(newbase, file_directory((char *)filename));

              htmlReadFile(t, embed, newbase);
              fclose(embed);
            }
#ifndef DEBUG
	    else
	      progress_error(HD_ERROR_FILE_NOT_FOUND,
                             "Unable to embed \"%s\" - %s", filename,
	                     strerror(errno));
#endif // !DEBUG
	  }
          break;

      case HD_ELEMENT_FONT :
          if ((face = t->get_attr("FACE")) != NULL)
          {
            for (ptr = face; *ptr != '\0'; ptr ++)
              *ptr = tolower(*ptr);

            if (strstr((char *)face, "arial") != NULL ||
                strstr((char *)face, "helvetica") != NULL)
              t->typeface = HD_FONTFACE_HELVETICA;
            else if (strstr((char *)face, "times") != NULL)
              t->typeface = HD_FONTFACE_TIMES;
            else if (strstr((char *)face, "courier") != NULL)
	    {
              t->typeface = HD_FONTFACE_COURIER;

              if (whitespace)
	      {
		// Insert a space before monospaced text...
		insert_space(parent, t);

		whitespace = 0;
	      }
            }
	    else if (strstr((char *)face, "symbol") != NULL)
              t->typeface = HD_FONTFACE_SYMBOL;
          }

          if ((color = t->get_attr("COLOR")) != NULL)
            compute_color(t, color);

          if ((size = t->get_attr("SIZE")) != NULL)
          {
            if (whitespace)
	    {
	      // Insert a space before sized text...
	      insert_space(parent, t);

	      whitespace = 0;
	    }

	    if (isdigit(size[0]))
	      sizeval = atoi((char *)size);
	    else
              sizeval = t->size + atoi((char *)size);

            if (sizeval < 0)
              t->size = 0;
            else if (sizeval > 7)
              t->size = 7;
            else
              t->size = sizeval;
          }

          htmlReadFile(t, fp, base);
          break;
#endif // 0

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
hdTree::real_next()
{
  // Start at the current node and find the next logical node in
  // the tree...
  if (next != NULL)
    return (next);

  if (parent != NULL)
    return (parent->real_next());
  else
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
        	                     (compare_func_t)compare_variables);
  }

  if (match == NULL)
  {
    if (nattrs >= aattrs)
    {
      // Allocate more memory...
      match = new hdTreeAttr[aattrs + 5];

      if (aattrs)
      {
        memcpy(match, attrs, aattrs * sizeof(hdTreeAttr));
	delete[] attrs;
      }

      attrs  = match;
      aattrs += 5;
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
            (compare_func_t)compare_variables);
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





#if 0
static void	delete_node(hdTree *t);
static void	insert_space(hdTree *parent, hdTree *t);
static int	parse_element(hdTree *t, FILE *fp);
static int	parse_variable(hdTree *t, FILE *fp);
static int	compute_size(hdTree *t);
static int	compute_color(hdTree *t, char *color);
static int	get_alignment(hdTree *t);
static const char *fix_filename(char *path, char *base);


/*
 * 'htmlReadFile()' - Read a file for HTML element codes.
 */

hdTree *			/* O - Pointer to top of file tree */
htmlReadFile(hdTree     *parent,/* I - Parent tree node */
             FILE       *fp,	/* I - File pointer */
	     const char *base)	/* I - Base directory for file */
{
}


/*
 * 'htmlAddTree()' - Add a tree node to the parent.
 */

hdTree *			/* O - New entry */
htmlAddTree(hdTree   *parent,	/* I - Parent entry */
            hdElement element,	/* I - Markup code */
            char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree node */


  if ((t = htmlNewTree(parent, element, data)) == NULL)
    return (NULL);

 /*
  * Add the tree node to the end of the chain of children...
  */

  if (parent != NULL)
  {
    if (parent->last_child != NULL)
    {
      parent->last_child->next = t;
      t->prev                  = parent->last_child;
    }
    else
      parent->child = t;

    parent->last_child = t;
  }

  return (t);
}


/*
 * 'htmlDeleteTree()' - Free all memory associated with a tree...
 */

int				/* O - 0 for success, -1 for failure */
htmlDeleteTree(hdTree *parent)	/* I - Parent to delete */
{
  hdTree	*next;		/* Next tree node */


  if (parent == NULL)
    return (-1);

  while (parent != NULL)
  {
    next = parent->next;

    if (parent->child != NULL)
      if (htmlDeleteTree(parent->child))
        return (-1);

    delete_node(parent);

    parent = next;
  }

  return (0);
}


/*
 * 'htmlInsertTree()' - Insert a tree node under the parent.
 */

hdTree *			/* O - New entry */
htmlInsertTree(hdTree   *parent,/* I - Parent entry */
               hdElement element,	/* I - Markup code */
               char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree node */


  if ((t = htmlNewTree(parent, element, data)) == NULL)
    return (NULL);

 /*
  * Insert the tree node at the beginning of the chain of children...
  */

  if (parent != NULL)
  {
    if (parent->child != NULL)
    {
      parent->child->prev = t;
      t->next             = parent->child;
    }
    else
      parent->last_child = t;

    parent->child = t;
  }

  return (t);
}


/*
 * 'htmlNewTree()' - Create a new tree node for the parent.
 */

hdTree *			/* O - New entry */
htmlNewTree(hdTree   *parent,	/* I - Parent entry */
            hdElement element,	/* I - Markup code */
            char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree node */


 /*
  * Allocate a new tree node - use calloc() to get zeroed data...
  */

  t = (hdTree *)calloc(sizeof(hdTree), 1);
  if (t == NULL)
    return (NULL);

 /*
  * Set the element code and copy the data if necessary...
  */

  t->element = element;
  if (data != NULL)
    t->data = (char *)strdup((char *)data);

 /*
  * Set/copy font characteristics...
  */

  if (parent == NULL)
  {
    t->halignment = HD_ALIGN_LEFT;
    t->valignment = HD_ALIGN_MIDDLE;
    t->typeface   = body_font;
    t->size       = HD_FONTSIZE_P;

    compute_color(t, text_color);
  }
  else
  {
    t->link          = parent->link;
    t->halignment    = parent->halignment;
    t->valignment    = parent->valignment;
    t->typeface      = parent->typeface;
    t->size          = parent->size;
    t->style         = parent->style;
    t->preformatted  = parent->preformatted;
    t->indent        = parent->indent;
    t->red           = parent->red;
    t->green         = parent->green;
    t->blue          = parent->blue;
    t->underline     = parent->underline;
    t->strikethrough = parent->strikethrough;
  }

  switch (t->element)
  {
    case HD_ELEMENT_NONE :
    case HD_ELEMENT_IMG :
       /*
	* Figure out the width & height of this fragment...
	*/

        compute_size(t);
	break;

    case HD_ELEMENT_H1 :
    case HD_ELEMENT_H2 :
    case HD_ELEMENT_H3 :
    case HD_ELEMENT_H4 :
    case HD_ELEMENT_H5 :
    case HD_ELEMENT_H6 :
        get_alignment(t);

        t->typeface      = heading_font;
        t->size          = HD_FONTSIZE_H1 - t->element + HD_ELEMENT_H1;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        t->style         = HD_FONTSTYLE_BOLD;
        break;

    case HD_ELEMENT_P :
        get_alignment(t);

        t->typeface      = body_font;
        t->size          = HD_FONTSIZE_P;
        t->style         = HD_FONTSTYLE_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        break;

    case HD_ELEMENT_PRE :
        t->typeface      = HD_FONTFACE_COURIER;
        t->size          = HD_FONTSIZE_PRE;
        t->style         = HD_FONTSTYLE_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 1;
        break;

    case HD_ELEMENT_DIV :
        get_alignment(t);
        break;

    case HD_ELEMENT_BLOCKQUOTE :
        t->style = HD_FONTSTYLE_ITALIC;

    case HD_ELEMENT_UL :
    case HD_ELEMENT_DIR :
    case HD_ELEMENT_MENU :
    case HD_ELEMENT_OL :
    case HD_ELEMENT_DL :
        t->indent ++;
        break;

    case HD_ELEMENT_AREA :
    case HD_ELEMENT_BR :
    case HD_ELEMENT_COMMENT :
    case HD_ELEMENT_HR :
    case HD_ELEMENT_INPUT :
    case HD_ELEMENT_ISINDEX :
    case HD_ELEMENT_META :
    case HD_ELEMENT_WBR :
        break;

    case HD_ELEMENT_TH :
        t->style = HD_FONTSTYLE_BOLD;
    case HD_ELEMENT_TD :
        get_alignment(t);
        break;

    case HD_ELEMENT_SUP :
        t->superscript = 1;
        t->size        = HD_FONTSIZE_P + HD_FONTSIZE_SUP;
        break;

    case HD_ELEMENT_SUB :
        t->subscript = 1;
        t->size      = HD_FONTSIZE_P + HD_FONTSIZE_SUB;
        break;

    case HD_ELEMENT_B :
        t->style = (style_t)(t->style | HD_FONTSTYLE_BOLD);
        break;

    case HD_ELEMENT_DD :
        t->indent ++;
        break;

    case HD_ELEMENT_DT :
    case HD_ELEMENT_I :
        t->style = (style_t)(t->style | HD_FONTSTYLE_ITALIC);
        break;

    case HD_ELEMENT_U :
    case HD_ELEMENT_INS :
        t->underline = 1;
        break;

    case HD_ELEMENT_STRIKE :
    case HD_ELEMENT_DEL :
        t->strikethrough = 1;
        break;

    default :
        break;
  }

  t->parent = parent;

  return (t);
}


/*
 * 'get_text()' - Get all text from the given tree.
 */

static char *		/* O - Pointer to last char set */
get_text(hdTree *tree,	/* I - Tree to pick */
         char  *buf)	/* I - Buffer to store text in */
{
  while (tree != NULL)
  {
    if (tree->child != NULL)
      buf = get_text(tree->child, buf);
    else if (tree->element == HD_ELEMENT_NONE && tree->data != NULL)
    {
      strcpy((char *)buf, (char *)tree->data);
      buf += strlen((char *)buf);
    }
    else if (tree->element == HD_ELEMENT_BR)
    {
      strcat((char *)buf, " ");
      buf ++;
    }

    tree = tree->next;
  }

  return (buf);
}


/*
 * 'htmlGetText()' - Get all text from the given tree.
 */

char *				/* O - String containing text nodes */
htmlGetText(hdTree *tree)	/* I - Tree to pick */
{
  char	buf[10240];		/* String buffer */


  buf[0] = '\0';
  get_text(tree, buf);

  return ((char *)strdup((char *)buf));
}


/*
 * 'htmlGetMeta()' - Get document "meta" data...
 */

char *				/* O - Content string */
htmlGetMeta(hdTree *tree,	/* I - Document tree */
            char  *name)	/* I - Metadata name */
{
  char	*tname,			/* Name value from tree node */
	*tcontent;		/* Content value from tree node */


  while (tree != NULL)
  {
   /*
    * Check this tree node...
    */

    if (tree->element == HD_ELEMENT_META &&
        (tname = htmlGetVariable(tree, "NAME")) != NULL &&
        (tcontent = htmlGetVariable(tree, "CONTENT")) != NULL)
      if (strcasecmp(name, (char *)tname) == 0)
        return (tcontent);

   /*
    * Check child entries...
    */

    if (tree->child != NULL)
      if ((tcontent = htmlGetMeta(tree->child, name)) != NULL)
        return (tcontent);

   /*
    * Next tree node...
    */

    tree = tree->next;
  }

  return (NULL);
}


/*
 * 'htmlGetVariable()' - Get a variable value from a element entry.
 */

char *				/* O - Value or NULL if variable does not exist */
htmlGetVariable(hdTree *t,	/* I - Tree entry */
                char  *name)	/* I - Variable name */
{
}


/*
 * 'htmlSetVariable()' - Set a variable for a element entry.
 */

int				/* O - Set status: 0 = success, -1 = fail */
htmlSetVariable(hdTree *t,	/* I - Tree entry */
                char  *name,	/* I - Variable name */
                char  *value)	/* I - Variable value */
{
}


/*
 * 'htmlSetBaseSize()' - Set the font sizes and spacings...
 */

void
htmlSetBaseSize(float p,	/* I - Point size of paragraph font */
                float s)	/* I - Spacing */
{
  int	i;			/* Looping var */


  p /= 1.2 * 1.2 * 1.2;
  for (i = 0; i < 8; i ++, p *= 1.2)
  {
    font_sizes[i]    = p;
    line_spacings[i] = p * s;
  }
}


/*
 * 'htmlSetCharSet()' - Set the character set for output.
 */

void
htmlSetCharSet(const char *cs)	/* I - Character set file to load */
{
  int		i, j;		/* Looping vars */
  char		filename[1024];	/* Filenames */
  FILE		*fp;		/* Files */
  int		ch, unicode;	/* Character values */
  float		width;		/* Width value */
  char		glyph[64];	/* Glyph name */
  char		line[1024];	/* Line from AFM file */
  int		chars[256];	/* Character encoding array */


  strcpy(font_charset, cs);

  if (!initialized)
  {
   /*
    * Load the PostScript glyph names for all of Unicode...
    */

    memset(font_glyphs_unicode, 0, sizeof(font_glyphs_unicode));

    snprintf(line, sizeof(line), "%s/data/psglyphs", data_dir);
    if ((fp = fopen(line, "r")) != NULL)
    {
      while (fscanf(fp, "%x%63s", &unicode, glyph) == 2)
        font_glyphs_unicode[unicode] = strdup(glyph);

      fclose(fp);

      initialized = 1;
    }
#ifndef DEBUG
    else
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open psglyphs data file!");
#endif /* !DEBUG */
  }

  memset(font_glyphs, 0, sizeof(font_glyphs));

  if (strncmp(cs, "8859-", 5) == 0)
    snprintf(filename, sizeof(filename), "%s/data/iso-%s", data_dir, cs);
  else
    snprintf(filename, sizeof(filename), "%s/data/%s", data_dir, cs);

  if ((fp = fopen(filename, "r")) == NULL)
  {
   /*
    * Can't open charset file; use ISO-8859-1...
    */

#ifndef DEBUG
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open character set file %s!", cs);
#endif /* !DEBUG */

    for (i = 0; i < 256; i ++)
      chars[i] = i;

   /*
    * Hardcode characters 128 to 159 for Microsoft's version of ISO-8859-1...
    */

    chars[0x80] = 0x20ac; /* Euro */
    chars[0x82] = 0x201a;
    chars[0x83] = 0x0192;
    chars[0x84] = 0x201e;
    chars[0x85] = 0x2026;
    chars[0x86] = 0x2020;
    chars[0x87] = 0x2021;
    chars[0x88] = 0x02c6;
    chars[0x89] = 0x2030;
    chars[0x8a] = 0x0160;
    chars[0x8b] = 0x2039;
    chars[0x8c] = 0x0152;
    chars[0x91] = 0x2018;
    chars[0x92] = 0x2019;
    chars[0x93] = 0x201c;
    chars[0x94] = 0x201d;
    chars[0x95] = 0x2022;
    chars[0x96] = 0x2013;
    chars[0x97] = 0x2014;
    chars[0x98] = 0x02dc;
    chars[0x99] = 0x2122;
    chars[0x9a] = 0x0161;
    chars[0x9b] = 0x203a;
    chars[0x9c] = 0x0153;
    chars[0x9f] = 0x0178;
  }
  else
  {
   /*
    * Read the <char> <unicode> lines from the file...
    */

    memset(chars, 0, sizeof(chars));

    while (fscanf(fp, "%x%x", &ch, &unicode) == 2)
      chars[ch] = unicode;

    fclose(fp);
  }

 /*
  * Build the glyph array...
  */

  for (i = 0; i < 256; i ++)
    if (chars[i] == 0)
      font_glyphs[i] = NULL;
    else
      font_glyphs[i] = font_glyphs_unicode[chars[i]];

 /*
  * Now read all of the font widths...
  */

  for (i = 0; i < 4; i ++)
    for (j = 0; j < 4; j ++)
    {
      for (ch = 0; ch < 256; ch ++)
        font_widths[i][j][ch] = 0.6f;

      snprintf(filename, sizeof(filename), "%s/afm/%s", data_dir,
               fonts[i][j]);
      if ((fp = fopen(filename, "r")) == NULL)
      {
#ifndef DEBUG
        progress_error(HD_ERROR_FILE_NOT_FOUND,
                       "Unable to open font width file %s!", fonts[i][j]);
#endif /* !DEBUG */
        continue;
      }

      while (fgets(line, sizeof(line), fp) != NULL)
      {
        if (strncmp(line, "C ", 2) != 0)
	  continue;

        if (i < 3)
	{
	 /*
	  * Handle encoding of Courier, Times, and Helvetica using
	  * assigned charset...
	  */

          if (sscanf(line, "%*s%*s%*s%*s%f%*s%*s%s", &width, glyph) != 2)
	    continue;

          for (ch = 0; ch < 256; ch ++)
	    if (font_glyphs[ch] && strcmp(font_glyphs[ch], glyph) == 0)
	      break;

          if (ch < 256)
	    font_widths[i][j][ch] = width * 0.001f;
	}
	else
	{
	 /*
	  * Symbol font uses its own encoding...
	  */

          if (sscanf(line, "%*s%d%*s%*s%f", &ch, &width) != 2)
	    continue;

          if (ch < 256)
	    font_widths[i][j][ch] = width * 0.001f;
	}
      }

      fclose(fp);
    }
}


/*
 * 'htmlSetTextColor()' - Set the default text color.
 */

void
htmlSetTextColor(char *color)	/* I - Text color */
{
  strncpy((char *)text_color, (char *)color, sizeof(text_color));
  text_color[sizeof(text_color) - 1] = '\0';
}


/*
 * 'delete_node()' - Free all memory associated with a node...
 */

static void
delete_node(hdTree *t)		/* I - Node to delete */
{
  int		i;		/* Looping var */
  hdTreeAttr		*var;		/* Current variable */


  if (t == NULL)
    return;

  if (t->data != NULL)
    free(t->data);

  for (i = nattrs, var = attrs; i > 0; i --, var ++)
  {
    free(var->name);
    if (var->value != NULL)
      free(var->value);
  }

  if (attrs != NULL)
    free(attrs);

  free(t);
}


//
// 'insert_space()' - Insert a whitespace character before the
//                    specified node.
//

static void
insert_space(hdTree *parent,	// I - Parent node
             hdTree *t)		// I - Node to insert before
{
  hdTree	*space;		// Space node


  // Allocate memory for the whitespace...
  space = (hdTree *)calloc(sizeof(hdTree), 1);
  if (space == NULL)
  {
#ifndef DEBUG
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for HTML tree node!");
#endif /* !DEBUG */
    return;
  }

  // Set/copy font characteristics...
  if (parent)
  {
    space->typeface = parent->typeface;
    space->size     = parent->size;
    space->style    = parent->style;
  }
  else
  {
    space->typeface = body_font;
    space->size     = HD_FONTSIZE_P;
  }

  // Initialize element data...
  space->element = HD_ELEMENT_NONE;
  space->data   = (char *)strdup(" ");

  // Set tree pointers...
  space->parent = parent;
  space->prev   = t->prev;
  space->next   = t;

  if (space->prev)
    space->prev->next = space;
  else if (parent)
    parent->child = space;

  t->prev = space;

  compute_size(space);
}


/*
 * 'parse_element()' - Parse a element string.
 */

static int			/* O - -1 on error, HD_ELEMENT_nnnn otherwise */
parse_element(hdTree *t,		/* I - Current tree node */
             FILE   *fp)	/* I - Input file */
{
  int	ch, ch2;		/* Characters from file */
  char	element[255],		/* Markup string... */
	*mptr,			/* Current character... */
	comment[10240],		/* Comment string */
	*cptr,			/* Current char... */
	**temp;			/* Markup variable entry */


  mptr = element;

  while ((ch = getc(fp)) != EOF && mptr < (element + sizeof(element) - 1))
    if (ch == '>' || isspace(ch))
      break;
    else
    {
      *mptr++ = ch;

      // Handle comments without whitespace...
      if ((mptr - element) == 3 && strncmp((const char *)element, "!--", 3) == 0)
      {
        ch = getc(fp);
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (HD_ELEMENT_ERROR);

  mptr = element;
  temp = (char **)bsearch(&mptr, elements,
                           sizeof(elements) / sizeof(elements[0]),
                           sizeof(elements[0]),
                           (compare_func_t)compare_elements);

  if (temp == NULL)
  {
   /*
    * Unrecognized element stuff...
    */

    t->element = HD_ELEMENT_COMMENT;
    strcpy((char *)comment, (char *)element);
    cptr = comment + strlen((char *)comment);

    DEBUG_printf(("%s%s (unrecognized!)\n", indent, element));
  }
  else
  {
    t->element = (hdElement)((const char **)temp - elements);
    cptr      = comment;

    DEBUG_printf(("%s%s\n", indent, element));
  }

  if (t->element == HD_ELEMENT_COMMENT)
  {
    while (ch != EOF && cptr < (comment + sizeof(comment) - 1))
    {
      if (ch == '>' && temp == NULL)
        break;

      *cptr++ = ch;

      if (ch == '-')
      {
        if ((ch2 = getc(fp)) == '>')
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
        ch = getc(fp);
    }

    *cptr = '\0';
    t->data = (char *)strdup((char *)comment);
  }
  else
  {
    while (ch != EOF && ch != '>')
    {
      if (!isspace(ch))
      {
        ungetc(ch, fp);
        parse_variable(t, fp);
      }

      ch = getc(fp);
    }
  }

  return (t->element);
}


/*
 * 'parse_variable()' - Parse a element variable string.
 */

static int			/* O - -1 on error, 0 on success */
parse_variable(hdTree *t,	/* I - Current tree node */
               FILE   *fp)	/* I - Input file */
{
}


/*
 * 'compute_color()' - Compute the red, green, blue color from the given
 *                     string.
 */

static int
compute_color(hdTree *t,	/* I - Tree entry */
              char  *color)	/* I - Color string */
{
  float	rgb[3];			/* RGB color */


  get_color(color, rgb);

  t->red   = (char)(rgb[0] * 255.0f + 0.5f);
  t->green = (char)(rgb[1] * 255.0f + 0.5f);
  t->blue  = (char)(rgb[2] * 255.0f + 0.5f);

  return (0);
}


/*
 * 'compute_size()' - Compute the width and height of a tree node.
 */

static int			/* O - 0 = success, -1 = failure */
compute_size(hdTree *t)		/* I - Tree entry */
{
  char		*ptr;		/* Current character */
  float		width,		/* Current width */
		max_width;	/* Maximum width */
  char		*width_ptr,	/* Pointer to width string */
		*height_ptr,	/* Pointer to height string */
		*size_ptr,	/* Pointer to size string */
		*type_ptr;	/* Pointer to spacer type string */
  image_t	*img;		/* Image */
  char		number[255];	/* Width or height value */


  if (!initialized)
    htmlSetCharSet("8859-1");

  if (t->element == HD_ELEMENT_IMG)
  {
    width_ptr  = t->get_attr("WIDTH");
    height_ptr = t->get_attr("HEIGHT");

    img = image_load((char *)t->get_attr("_HD_SRC"),
                     grayscale);

    if (width_ptr != NULL && height_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / ppi * 72.0f;
      t->height = atoi((char *)height_ptr) / ppi * 72.0f;

      return (0);
    }

    if (img == NULL)
      return (-1);

    if (width_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / ppi * 72.0f;
      t->height = t->width * img->height / img->width;

      sprintf(number, "%d",
              atoi((char *)width_ptr) * img->height / img->width);
      if (strchr((char *)width_ptr, '%') != NULL)
        strcat(number, "%");
      htmlSetVariable(t, "HEIGHT", (char *)number);
    }
    else if (height_ptr != NULL)
    {
      t->height = atoi((char *)height_ptr) / ppi * 72.0f;
      t->width  = t->height * img->width / img->height;

      sprintf(number, "%d",
              atoi((char *)height_ptr) * img->width / img->height);
      if (strchr((char *)height_ptr, '%') != NULL)
        strcat(number, "%");
      htmlSetVariable(t, "WIDTH", (char *)number);
    }
    else
    {
      t->width  = img->width / ppi * 72.0f;
      t->height = img->height / ppi * 72.0f;

      sprintf(number, "%d", img->width);
      htmlSetVariable(t, "WIDTH", (char *)number);

      sprintf(number, "%d", img->height);
      htmlSetVariable(t, "HEIGHT", (char *)number);
    }

    return (0);
  }
  else if (t->element == HD_ELEMENT_SPACER)
  {
    width_ptr  = t->get_attr("WIDTH");
    height_ptr = t->get_attr("HEIGHT");
    size_ptr   = t->get_attr("SIZE");
    type_ptr   = t->get_attr("TYPE");

    if (width_ptr != NULL)
      t->width = atoi((char *)width_ptr) / ppi * 72.0f;
    else if (size_ptr != NULL)
      t->width = atoi((char *)size_ptr) / ppi * 72.0f;
    else
      t->width = 1.0f;

    if (height_ptr != NULL)
      t->height = atoi((char *)height_ptr) / ppi * 72.0f;
    else if (size_ptr != NULL)
      t->height = atoi((char *)size_ptr) / ppi * 72.0f;
    else
      t->height = 1.0f;

    if (type_ptr == NULL)
      return (0);

    if (strcasecmp(type_ptr, "horizontal") == 0)
      t->height = 0.0;
    else if (strcasecmp(type_ptr, "vertical") == 0)
      t->width = 0.0;

    return (0);
  }
  else if (t->element == HD_ELEMENT_BR)
  {
    t->width  = 0.0;
    t->height = font_sizes[t->size];

    return (0);
  }
  else if (t->preformatted && t->data)
  {
    for (max_width = 0.0, width = 0.0, ptr = t->data; *ptr != '\0'; ptr ++)
      if (*ptr == '\n')
      {
        if (width > max_width)
          max_width = width;
      }
      else if (*ptr == '\t')
        width = (float)(((int)width + 7) & ~7);
      else
        width += font_widths[t->typeface][t->style][*ptr];

   if (width < max_width)
     width = max_width;
  }
  else if (t->data)
    for (width = 0.0, ptr = t->data; *ptr != '\0'; ptr ++)
      width += font_widths[t->typeface][t->style][*ptr];
  else
    width = 0.0f;

  t->width  = width * font_sizes[t->size];
  t->height = font_sizes[t->size];

  DEBUG_printf(("%swidth = %.1f, height = %.1f\n",
                indent, t->width, t->height));

  return (0);
}


/*
 * 'get_alignment()' - Get horizontal & vertical alignment values.
 */

static int			/* O - 0 for success, -1 for failure */
get_alignment(hdTree *t)	/* I - Tree entry */
{
  char	*align;			/* Alignment string */


  if ((align = t->get_attr("ALIGN")) == NULL)
    align = htmlGetStyle(t, "text-align");

  if (align != NULL)
  {
    if (strcasecmp(align, "left") == 0)
      t->halignment = HD_ALIGN_LEFT;
    else if (strcasecmp(align, "center") == 0)
      t->halignment = HD_ALIGN_CENTER;
    else if (strcasecmp(align, "right") == 0)
      t->halignment = HD_ALIGN_RIGHT;
    else if (strcasecmp(align, "justify") == 0)
      t->halignment = HD_ALIGN_JUSTIFY;
    else if (strcasecmp(align, "top") == 0)
      t->valignment = HD_ALIGN_TOP;
    else if (strcasecmp(align, "middle") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp(align, "bottom") == 0)
      t->valignment = HD_ALIGN_BOTTOM;
  }

  if ((align = t->get_attr("VALIGN")) == NULL)
    align = htmlGetStyle(t, "vertical-align");

  if (align != NULL)
  {
    if (strcasecmp(align, "top") == 0)
      t->valignment = HD_ALIGN_TOP;
    else if (strcasecmp(align, "middle") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp(align, "center") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp(align, "bottom") == 0)
      t->valignment = HD_ALIGN_BOTTOM;
  }

  return (0);
}


/*
 * 'fix_filename()' - Fix a filename to be relative to the base directory.
 */

static const char *				/* O - Fixed filename */
fix_filename(char *filename,		/* I - Original filename */
             char *base)		/* I - Base directory */
{
  char		*slash;			/* Location of slash */
  static char	newfilename[1024];	/* New filename */


  if (filename == NULL)
    return (NULL);

  if (strcmp(base, ".") == 0 || strstr(filename, "//") != NULL)
    return (file_find(Path, filename));

#ifdef MAC
  //
  // Convert UNIX/DOS/WINDOWS slashes to colons for MacOS...
  //
  // Question: WHY doesn't the Mac standard C library do this for
  // you???
  //

  for (slash = strchr(filename, '/'); slash != NULL; slash = strchr(slash + 1, '/'))
    *slash = ':';

  for (slash = strchr(filename, '\\'); slash != NULL; slash = strchr(slash + 1, '\\'))
    *slash = ':';
#endif // MAC

  if (strncmp(filename, "./", 2) == 0 ||
      strncmp(filename, ".\\", 2) == 0)
    filename += 2;

  if (strncmp(base, "http://", 7) == 0)
  {
    strcpy(newfilename, base);
    base = newfilename + 7;

    if (filename[0] == '/')
    {
      if ((slash = strchr(base, '/')) != NULL)
        strcpy(slash, filename);
      else
        strcat(newfilename, filename);

      return (newfilename);
    }
    else if ((slash = strchr(base, '/')) == NULL)
      strcat(newfilename, "/");
  }
  else
  {
    if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
	base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
      return (file_find(Path, filename)); /* No change needed for absolute path */

    strcpy(newfilename, base);
    base = newfilename;
  }

#ifdef MAC
  //
  // Convert UNIX/DOS/WINDOWS slashes to colons for MacOS...
  //
  // Question: WHY doesn't the Mac standard C library do this for
  // you???
  //

  for (slash = strchr(newfilename, '/'); slash != NULL; slash = strchr(slash + 1, '/'))
    *slash = ':';

  for (slash = strchr(newfilename, '\\'); slash != NULL; slash = strchr(slash + 1, '\\'))
    *slash = ':';
#endif // MAC

#if defined(WIN32) || defined(__EMX__)
  while (strncmp(filename, "../", 3) == 0 ||
         strncmp(filename, "..\\", 3) == 0)
#elif defined(MAC)
  while (strncmp(filename, "..:", 3) == 0)
#else
  while (strncmp(filename, "../", 3) == 0)
#endif // WIN32 || __EMX__
  {
    filename += 3;
#if defined(WIN32) || defined(__EMX__)
    if ((slash = strrchr(base, '/')) != NULL)
      *slash = '\0';
    else if ((slash = strrchr(base, '\\')) != NULL)
      *slash = '\0';
#elif defined(MAC)
    if ((slash = strrchr(base, ':')) != NULL)
      *slash = '\0';
#else
    if ((slash = strrchr(base, '/')) != NULL)
      *slash = '\0';
#endif // WIN32 || __EMX__
    else
    {
      filename -= 3;
      break;
    }
  }

#ifdef MAC
  strcat(newfilename, ":");
#else
  strcat(newfilename, "/");
#endif // MAC
  strcat(newfilename, filename);

  return (file_find(Path, newfilename));
}
#endif // 0


//
// End of "$Id: tree.cxx,v 1.3 2002/02/08 19:39:52 mike Exp $".
//
