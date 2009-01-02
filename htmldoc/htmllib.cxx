//
// "$Id: htmllib.cxx,v 1.41.2.80.2.9 2005/05/09 02:05:01 mike Exp $"
//
// HTML parsing routines for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2008 by Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//
// Contents:
//
//   htmlSetDebugFile()     - Set the file to send debugging information.
//   htmlReadFile()         - Read a file for HTML markup codes.
//   write_file()           - Write a tree entry to a file...
//   htmlWriteFile()        - Write an HTML markup tree to a file.
//   htmlAddTree()          - Add a tree node to the parent.
//   htmlDeleteTree()       - Free all memory associated with a tree...
//   htmlInsertTree()       - Insert a tree node under the parent.
//   htmlNewTree()          - Create a new tree node for the parent.
//   htmlGetText()          - Get all text from the given tree.
//   htmlGetMeta()          - Get document "meta" data...
//   htmlGetAttr()          - Get a variable value from a markup entry.
//   htmlSetAttr()          - Set a variable for a markup entry.
//   compare_variables()    - Compare two markup variables.
//   delete_node()          - Free all memory associated with a node...
//   insert_space()         - Insert a whitespace character before the specified
//                            node.
//   parse_markup()         - Parse a markup string.
//   parse_variable()       - Parse a markup variable string.
//   compute_size()         - Compute the width and height of a tree entry.
//   fix_filename()         - Fix a filename to be relative to the base
//                            directory.
//   html_memory_used()     - Figure out the amount of memory that was used.
//   htmlDebugStats()       - Display debug statistics for HTML tree memory use.
//   style_memory_used()    - Figure out the amount of memory that was used for
//                            styles.
//   htmlDebugStyleStats()  - Display debug statistics for stylesheet memory
//                            use.
//   htmlFindElement()      - Find an element in the document.
//   htmlFindFile()         - Find a file in the document.
//   htmlFixLinks()         - Fix the external links in the document.
//   htmlDeleteStyleSheet() - Delete all of the stylesheet data.
//   htmlInitStyleSheet()   - Initialize the stylesheet data.
//   htmlRealNext()         - Return the next logical node in the tree.
//   htmlUpdateStyle()      - Update the style data for a node.
//   utf8_getc()            - Get a UTF-8 encoded character.
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include <ctype.h>
#include <math.h>


const char	*_htmlData = HTMLDOC_DATA;	// Data directory
hdFontFace	_htmlBodyFont = HD_FONT_FACE_SERIF,
		_htmlHeadingFont = HD_FONT_FACE_SANS_SERIF;
hdStyleSheet	*_htmlStyleSheet = NULL;// Style data


//
// Local functions.
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static int	write_file(hdTree *t, hdFile *fp, int col);
static int	compare_variables(hdTreeAttr *v0, hdTreeAttr *v1);
static void	delete_node(hdTree *t);
static void	insert_space(hdTree *parent, hdTree *t);
static int	parse_markup(hdTree *t, hdFile *fp, int *linenum);
static int	parse_variable(hdTree *t, hdFile *fp, int *linenum);
static int	compute_size(hdTree *t);
static char	*fix_filename(const char *path, const char *base,
		              char *buffer, size_t bufsize);
static int	utf8_getc(int ch, hdFile *fp);

#define issuper(x)	((x) == HD_ELEMENT_CENTER || (x) == HD_ELEMENT_DIV ||\
			 (x) == HD_ELEMENT_BLOCKQUOTE)
#define isblock(x)	((x) == HD_ELEMENT_ADDRESS || \
			 (x) == HD_ELEMENT_P || (x) == HD_ELEMENT_PRE ||\
			 ((x) >= HD_ELEMENT_H1 && (x) <= HD_ELEMENT_H15) ||\
			 (x) == HD_ELEMENT_HR || (x) == HD_ELEMENT_TABLE)
#define islist(x)	((x) == HD_ELEMENT_DL || (x) == HD_ELEMENT_OL ||\
			 (x) == HD_ELEMENT_UL || (x) == HD_ELEMENT_DIR ||\
			 (x) == HD_ELEMENT_MENU)
#define islentry(x)	((x) == HD_ELEMENT_LI || (x) == HD_ELEMENT_DD ||\
			 (x) == HD_ELEMENT_DT)
#define istable(x)	((x) == HD_ELEMENT_TBODY || (x) == HD_ELEMENT_THEAD ||\
			 (x) == HD_ELEMENT_TFOOT || (x) == HD_ELEMENT_TR)
#define istentry(x)	((x) == HD_ELEMENT_TD || (x) == HD_ELEMENT_TH)

static FILE	*debug_file = NULL;
static int	debug_indent = 0;


//
// 'htmlSetDebugFile()' - Set the file to send debugging information.
//

void
htmlSetDebugFile(FILE *fp)		// I - File to send debug info or NULL
{
  debug_file   = fp;
  debug_indent = 0;
}


//
// 'htmlReadFile()' - Read a file for HTML markup codes.
//

hdTree *				// O - Pointer to top of file tree
htmlReadFile(hdTree     *parent,	// I - Parent tree entry
             hdFile     *fp,		// I - File pointer
	     const char *base)		// I - Base directory for file
{
  int		ch;			// Character from file
  hdChar	*ptr,			// Pointer in string
		entity[16],		// Character entity name (&#nnn; or &name;)
		*eptr;			// Pointer in entity string
  hdTree	*tree,			// "top" of this tree
		*t,			// New tree entry
		*prev,			// Previous tree entry
		*temp;			// Temporary looping var
  int		descend;		// Descend into node?
  hdFile	*embed;			// File pointer for EMBED
  char		newbase[1024],		// New base directory for EMBED
		fixname[1024];		// Fixed filename
  hdChar	*filename,		// Filename for EMBED tag
		*type;			// Type for EMBED tag
  int		linenum;		// Line number in file
  static hdChar	s[10240];		// String from file
  static int	have_whitespace = 0;	// Non-zero if there was leading whitespace


  DEBUG_printf(("htmlReadFile(parent=%p, fp=%p, base=\"%s\")\n",
                parent, fp, base ? base : "(null)"));

  // Start off with no previous tree entry...
  prev = NULL;
  tree = NULL;

  // Parse data until we hit end-of-file...
  linenum = 1;

  while ((ch = fp->get()) != EOF)
  {
    // Ignore leading whitespace...
    if (parent == NULL || parent->style == NULL ||
        parent->style->white_space != HD_WHITE_SPACE_PRE)
    {
      while (isspace(ch))
      {
	if (ch == '\n')
	  linenum ++;

        have_whitespace = 1;
        ch              = fp->get();
      }

      if (ch == EOF)
        break;
    }

    // Allocate a new tree entry - use calloc() to get zeroed data...
    t = (hdTree *)calloc(sizeof(hdTree), 1);
    if (t == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for HTML tree node!");
      break;
    }

    // Set/copy style characteristics...
    if (parent)
    {
      t->link  = parent->link;
      t->style = parent->style ? parent->style :
                                 _htmlStyleSheet->find_style(parent);
    }

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
	progress_error(HD_ERROR_HTML_ERROR, "Unquoted < on line %d.", linenum);

	if (ch == '\n')
	  linenum ++;

	ptr = s;
	if (have_whitespace)
	{
          *ptr++ = ' ';
	  have_whitespace = 0;
	}

        *ptr++ = '<';
	if (ch == '=')
	  *ptr++ = '=';
	else if (ch == '<')
	  fp->unget(ch);
	else
	  have_whitespace = 1;

	*ptr++ = '\0';

	t->element = HD_ELEMENT_NONE;
	t->data   = (hdChar *)strdup((char *)s);
      }
      else
      {
        // Start of a markup...
	if (ch != '/')
          fp->unget(ch);

	if (parse_markup(t, fp, &linenum) == HD_ELEMENT_ERROR)
	{
          progress_error(HD_ERROR_READ_ERROR,
                         "Unable to parse HTML element on line %d!", linenum);

          delete_node(t);
          break;
	}

        htmlUpdateStyle(t, base);

	// Eliminate extra whitespace...
	if (issuper(t->element) || isblock(t->element) ||
            islist(t->element) || islentry(t->element) ||
            istable(t->element) || istentry(t->element) ||
	    t->element == HD_ELEMENT_TITLE)
          have_whitespace = 0;

	// If this is the matching close mark, or if we are starting the same
	// markup, or if we've completed a list, we're done!
	if (ch == '/')
	{
	  // Close markup; find matching markup...
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (temp->element == t->element)
              break;
	    else if (temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	}
	else if (t->element == HD_ELEMENT_BODY || t->element == HD_ELEMENT_HEAD)
	{
	  // Make sure we aren't inside an existing HEAD or BODY...
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (temp->element == HD_ELEMENT_BODY || temp->element == HD_ELEMENT_HEAD)
              break;
	    else if (temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (t->element == HD_ELEMENT_EMBED)
	{
	  // Close any text blocks...
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (isblock(temp->element) || islentry(temp->element))
              break;
	    else if (istentry(temp->element) || islist(temp->element) ||
	             issuper(temp->element) || temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (issuper(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
	    if (istentry(temp->element) || temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	}
	else if (islist(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (isblock(temp->element))
	      break;
	    else if (islentry(temp->element) || istentry(temp->element) ||
	             issuper(temp->element) || temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	}
	else if (islentry(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (islentry(temp->element))
              break;
	    else if (islist(temp->element) || issuper(temp->element) ||
	             istentry(temp->element) || temp->element == HD_ELEMENT_EMBED)
            {
	      temp = NULL;
	      break;
	    }
	}
	else if (isblock(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (isblock(temp->element))
              break;
	    else if (istentry(temp->element) || islist(temp->element) ||
	             islentry(temp->element) ||
	             issuper(temp->element) || temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	}
	else if (istable(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (istable(temp->element))
	      break;
	    else if (temp->element == HD_ELEMENT_TABLE || temp->element == HD_ELEMENT_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	}
	else if (istentry(t->element))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
            if (istentry(temp->element))
              break;
	    else if (temp->element == HD_ELEMENT_TABLE || istable(temp->element) ||
	             temp->element == HD_ELEMENT_EMBED)
	    {
	      if (temp->element != HD_ELEMENT_TR)
	      {
	        // Strictly speaking, this is an error - TD/TH can only
		// be found under TR, but web browsers automatically
		// inject a TR...
		progress_error(HD_ERROR_HTML_ERROR,
		               "No TR element before %s element on line %d.",
			       _htmlStyleSheet->get_element(t->element),
			       linenum);

                parent = htmlAddTree(temp, HD_ELEMENT_TR, NULL);
		prev   = NULL;
		if (debug_file)
		  fprintf(debug_file, "%*str (inserted) under %s, line %d\n",
		          debug_indent, "",
		          _htmlStyleSheet->get_element(temp->element),
			  linenum);
	      }

	      temp = NULL;
              break;
	    }
	}
	else
          temp = NULL;

	if (temp != NULL)
	{
          if (debug_file)
	    fprintf(debug_file, "%*s>>>> Auto-ascend <<<\n", debug_indent, "");

          if (temp && ch != '/' &&
	      temp->element != HD_ELEMENT_BODY &&
	      temp->element != HD_ELEMENT_DD &&
	      temp->element != HD_ELEMENT_DT &&
	      temp->element != HD_ELEMENT_HEAD &&
	      temp->element != HD_ELEMENT_HTML &&
	      temp->element != HD_ELEMENT_LI &&
	      temp->element != HD_ELEMENT_OPTION &&
	      temp->element != HD_ELEMENT_P &&
	      temp->element != HD_ELEMENT_TBODY &&
	      temp->element != HD_ELEMENT_TD &&
	      temp->element != HD_ELEMENT_TFOOT &&
	      temp->element != HD_ELEMENT_TH &&
	      temp->element != HD_ELEMENT_THEAD &&
	      temp->element != HD_ELEMENT_TR)
	  {
	    // Log this condition as an error...
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "No /%s element before %s element on line %d.",
	                   _htmlStyleSheet->get_element(temp->element),
			   _htmlStyleSheet->get_element(t->element), linenum);
	    if (debug_file)
	      fprintf(debug_file,
	              "%*sNo /%s element before %s element on line %d.\n",
	              debug_indent, "",
		      _htmlStyleSheet->get_element(temp->element),
		      _htmlStyleSheet->get_element(t->element), linenum);
	  }

          if (debug_file && debug_indent > 0)
	  {
	    hdTree *p = parent;

            for (debug_indent -= 4;
		 p && p != temp && debug_indent > 0;
		 p = p->parent, debug_indent -= 4);
          }

          // Safety check; should never happen, since HD_ELEMENT_FILE is
	  // the root node created by the caller...
          if (temp->parent)
	  {
	    parent = temp->parent;
            prev   = parent->last_child;
	  }
	  else
	  {
	    for (prev = temp; prev->next; prev = prev->next);
	    parent = NULL;
	  }

          if (ch == '/')
	  {
	    // Closing element, so delete this node...
            delete_node(t);
	    continue;
	  }
	  else
	  {
	    // Reparent the node...
	    if (parent)
	    {
	      t->link  = parent->link;
              t->style = parent->style ? parent->style :
	                     _htmlStyleSheet->find_style(parent);

              htmlUpdateStyle(t, base);
	    }
          }
	}
	else if (ch == '/')
	{
	  // Log this condition as an error...
	  if (t->element != HD_ELEMENT_UNKNOWN &&
	      t->element != HD_ELEMENT_COMMENT)
	  {
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "Dangling /%s element on line %d.",
			   _htmlStyleSheet->get_element(t->element), linenum);
	    if (debug_file)
	      fprintf(debug_file, "%*sDangling /%s element on line %d.\n",
		      debug_indent, "",
		      _htmlStyleSheet->get_element(t->element), linenum);
          }

	  delete_node(t);
	  continue;
	}
      }
    }
    else if (t->style->white_space == HD_WHITE_SPACE_PRE)
    {
      // Read a pre-formatted string into the current tree entry...
      ptr = s;
      while (ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
        if (ch == '&')
        {
	  // Possibly a character entity...
	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
	         (ch = fp->get()) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

          *eptr = '\0';

          if (ch != ';')
	  {
	    fp->unget(ch);
	    ch = 0;
	  }

          if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                   linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                   entity, linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else
	    *ptr++ = ch;
        }
	else if ((ch & 0x80) &&
	         _htmlStyleSheet->encoding == HD_FONT_ENCODING_UTF8)
        {
	  // Collect UTF-8 value...
	  ch = utf8_getc(ch, fp);

	  if (ch)
	    *ptr++ = ch;
	}
	else if (ch != 0 && ch != '\r')
	{
          *ptr++ = ch;

          if (ch == '\n')
	  {
	    linenum ++;
            break;
	  }
	}

        ch = fp->get();
      }

      *ptr = '\0';

      if (ch == '<')
        fp->unget(ch);

      t->element = HD_ELEMENT_NONE;
      t->data   = (hdChar *)strdup((char *)s);

      if (debug_file)
	fprintf(debug_file, "%*sfragment \"%s\" (len=%d), line %d\n",
	        debug_indent, "", s, (int)(ptr - s), linenum);
    }
    else
    {
      // Read the next string fragment...
      ptr = s;
      if (have_whitespace)
      {
        *ptr++ = ' ';
	have_whitespace = 0;
      }

      while (!isspace(ch) && ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
        if (ch == '&')
        {
	  // Possibly a character entity...
	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
	         (ch = fp->get()) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

          *eptr = '\0';

          if (ch != ';')
	  {
	    fp->unget(ch);
	    ch = 0;
	  }

          if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                   linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                   entity, linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else
	    *ptr++ = ch;
        }
	else if ((ch & 0x80) &&
	         _htmlStyleSheet->encoding == HD_FONT_ENCODING_UTF8)
        {
	  // Collect UTF-8 value...
	  ch = utf8_getc(ch, fp);

	  if (ch)
	    *ptr++ = ch;
	}
	else if (ch)
          *ptr++ = ch;

        ch = fp->get();
      }

      if (ch == '\n')
	linenum ++;

      if (isspace(ch))
        have_whitespace = 1;

      *ptr = '\0';

      if (ch == '<')
        fp->unget(ch);

      t->element = HD_ELEMENT_NONE;
      t->data   = (hdChar *)strdup((char *)s);

      if (debug_file)
	fprintf(debug_file, "%*sfragment \"%s\" (len=%d), line %d\n",
	        debug_indent, "", s, (int)(ptr - s), linenum);
    }

    // If the parent tree pointer is not null and this is the first
    // entry we've read, set the child pointer...
    if (debug_file)
      fprintf(debug_file, "%*sADDING %s node to %s parent, white_space=%d!\n",
              debug_indent, "", _htmlStyleSheet->get_element(t->element),
	      parent ? _htmlStyleSheet->get_element(parent->element) : "ROOT",
	      t->style ? t->style->white_space : -1);

    if (parent != NULL && prev == NULL)
      parent->child = t;

    if (parent != NULL)
      parent->last_child = t;

    // Do the prev/next links...
    t->parent = parent;
    t->prev   = prev;
    if (prev != NULL)
      prev->next = t;

    if (tree == NULL)
      tree = t;

    prev = t;

    // Do markup-specific stuff...
    descend = 0;

    switch (t->element)
    {
      case HD_ELEMENT_STYLE :
          // Read embedded stylesheet...
	  descend = 1;

	  _htmlStyleSheet->load(fp, base);
	  break;

      case HD_ELEMENT_LINK :
          // See if we have a stylesheet link...
	  {
	    hdChar *rel = htmlGetAttr(t, "REL");
	    hdChar *href = htmlGetAttr(t, "HREF");

	    if (rel && !strcasecmp((char *)rel, "stylesheet") && href)
	    {
	      filename = (hdChar *)fix_filename((char *)href, (char *)base,
	                                        fixname, sizeof(fixname));

              if ((embed = hdFile::open((char *)filename, HD_FILE_READ)) != NULL)
              {
		hdFile::dirname((char *)filename, newbase, sizeof(newbase));
        	_htmlStyleSheet->load(embed, newbase);
        	delete embed;
              }
	      else
		progress_error(HD_ERROR_FILE_NOT_FOUND,
                               "Unable to read stylesheet \"%s\" - %s",
			       rel, strerror(errno));
	    }
	  }
	  break;

      case HD_ELEMENT_BODY :
          // Update the background image as necessary...
          if ((filename = htmlGetAttr(t, "BACKGROUND")) != NULL)
	    htmlSetAttr(t, "BACKGROUND",
	                (hdChar *)fix_filename((char *)filename, base, fixname,
			                       sizeof(fixname)));

          descend = 1;
          break;

      case HD_ELEMENT_IMG :
          if (have_whitespace)
	  {
	    // Insert a space before this image...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          // Update the image source as necessary...
          if ((filename = htmlGetAttr(t, "SRC")) != NULL)
	    htmlSetAttr(t, "_HD_SRC",
	                (hdChar *)fix_filename((char *)filename, base, fixname,
			                       sizeof(fixname)));

      case HD_ELEMENT_BR :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
	  // Figure out the width & height of this markup...
          compute_size(t);
	  break;

      case HD_ELEMENT_HR :
      case HD_ELEMENT_DOCTYPE :
      case HD_ELEMENT_AREA :
      case HD_ELEMENT_COMMENT :
      case HD_ELEMENT_INPUT :
      case HD_ELEMENT_ISINDEX :
      case HD_ELEMENT_META :
      case HD_ELEMENT_WBR :
      case HD_ELEMENT_COL :
          break;

      case HD_ELEMENT_EMBED :
          if ((type = htmlGetAttr(t, "TYPE")) != NULL &&
	      strncasecmp((const char *)type, "text/html", 9) != 0)
	    break;

          if ((filename = htmlGetAttr(t, "SRC")) != NULL)
	  {
	    filename = (hdChar *)fix_filename((char *)filename, (char *)base,
	                                      fixname, sizeof(fixname));

            if ((embed = hdFile::open((char *)filename, HD_FILE_READ)) != NULL)
            {
	      hdFile::dirname((char *)filename, newbase, sizeof(newbase));
              htmlReadFile(t, embed, newbase);
              delete embed;
            }
	    else
	      progress_error(HD_ERROR_FILE_NOT_FOUND,
                             "Unable to embed \"%s\" - %s", filename,
	                     strerror(errno));
	  }
          break;

      case HD_ELEMENT_KBD :
      case HD_ELEMENT_TT :
      case HD_ELEMENT_CODE :
      case HD_ELEMENT_SAMP :
          if (isspace(ch = fp->get()))
	    have_whitespace = 1;
	  else
	    fp->unget(ch);

      case HD_ELEMENT_BIG :
      case HD_ELEMENT_SMALL :
      case HD_ELEMENT_SUP :
      case HD_ELEMENT_SUB :
      case HD_ELEMENT_U :
      case HD_ELEMENT_INS :
      case HD_ELEMENT_STRIKE :
      case HD_ELEMENT_S :
      case HD_ELEMENT_DEL :
          if (have_whitespace)
	  {
	    // Insert a space before subscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          descend = 1;
          break;

      case HD_ELEMENT_A :
          if (have_whitespace)
	  {
	    // Insert a space before this link...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          descend = 1;
	  break;

      default :
          descend = 1;
          break;
    }

    if (descend)
    {
      debug_indent += 4;

      parent = t;
      prev   = NULL;
    }
  }  

  return (tree);
}


//
// 'write_file()' - Write a tree entry to a file...
//

static int				// I - New column
write_file(hdTree *t,			// I - Tree entry
           hdFile *fp,			// I - File to write to
           int    col)			// I - Current column
{
  int		i;			// Looping var
  hdChar	*ptr;			// Character pointer


  while (t != NULL)
  {
    if (t->element == HD_ELEMENT_NONE)
    {
      if (t->style->white_space == HD_WHITE_SPACE_PRE)
      {
        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fp->puts(_htmlStyleSheet->get_entity(*ptr));

	if (t->data[strlen((char *)t->data) - 1] == '\n')
          col = 0;
	else
          col += strlen((char *)t->data);
      }
      else
      {
	if ((col + strlen((char *)t->data)) > 72 && col > 0)
	{
          fp->put('\n');
          col = 0;
	}

        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fp->puts(_htmlStyleSheet->get_entity(*ptr));

	col += strlen((char *)t->data);

	if (col > 72)
	{
          fp->put('\n');
          col = 0;
	}
      }
    }
    else if (t->element == HD_ELEMENT_COMMENT)
      fp->printf("\n<!--%s-->\n", t->data);
    else if (t->element > 0)
    {
      switch (t->element)
      {
        case HD_ELEMENT_AREA :
        case HD_ELEMENT_BR :
        case HD_ELEMENT_CENTER :
        case HD_ELEMENT_COMMENT :
        case HD_ELEMENT_DD :
        case HD_ELEMENT_DL :
        case HD_ELEMENT_DT :
        case HD_ELEMENT_H1 :
        case HD_ELEMENT_H2 :
        case HD_ELEMENT_H3 :
        case HD_ELEMENT_H4 :
        case HD_ELEMENT_H5 :
        case HD_ELEMENT_H6 :
        case HD_ELEMENT_HEAD :
        case HD_ELEMENT_HR :
        case HD_ELEMENT_LI :
        case HD_ELEMENT_MAP :
        case HD_ELEMENT_OL :
        case HD_ELEMENT_P :
        case HD_ELEMENT_PRE :
        case HD_ELEMENT_TABLE :
        case HD_ELEMENT_TITLE :
        case HD_ELEMENT_TR :
        case HD_ELEMENT_UL :
	case HD_ELEMENT_DIR :
	case HD_ELEMENT_MENU :
            if (col > 0)
            {
              fp->put('\n');
              col = 0;
            }
        default :
            break;
      }

      col += fp->printf("<%s", _htmlStyleSheet->get_element(t->element));
      for (i = 0; i < t->nattrs; i ++)
      {
	if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE)
	{
          fp->put('\n');
          col = 0;
	}

        if (col > 0)
        {
          fp->put(' ');
          col ++;
        }

	if (t->attrs[i].value == NULL)
          col += fp->printf("%s", t->attrs[i].name);
	else if (strchr((char *)t->attrs[i].value, '\"') != NULL)
          col += fp->printf("%s=\'%s\'", t->attrs[i].name, t->attrs[i].value);
	else
          col += fp->printf("%s=\"%s\"", t->attrs[i].name, t->attrs[i].value);
      }

      fp->put('>');
      col ++;

      if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE)
      {
	fp->put('\n');
	col = 0;
      }

      if (t->child != NULL)
      {
	col = write_file(t->child, fp, col);

	if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE)
	{
	  fp->put('\n');
	  col = 0;
	}
	
        col += fp->printf("</%s>", _htmlStyleSheet->get_element(t->element));
        switch (t->element)
        {
          case HD_ELEMENT_AREA :
          case HD_ELEMENT_BR :
          case HD_ELEMENT_CENTER :
          case HD_ELEMENT_COMMENT :
          case HD_ELEMENT_DD :
          case HD_ELEMENT_DL :
          case HD_ELEMENT_DT :
          case HD_ELEMENT_H1 :
          case HD_ELEMENT_H2 :
          case HD_ELEMENT_H3 :
          case HD_ELEMENT_H4 :
          case HD_ELEMENT_H5 :
          case HD_ELEMENT_H6 :
          case HD_ELEMENT_HEAD :
          case HD_ELEMENT_HR :
          case HD_ELEMENT_LI :
          case HD_ELEMENT_MAP :
          case HD_ELEMENT_OL :
          case HD_ELEMENT_P :
          case HD_ELEMENT_PRE :
          case HD_ELEMENT_TABLE :
          case HD_ELEMENT_TITLE :
          case HD_ELEMENT_TR :
          case HD_ELEMENT_UL :
          case HD_ELEMENT_DIR :
          case HD_ELEMENT_MENU :
              fp->put('\n');
              col = 0;
          default :
	      break;
        }
      }
    }

    t = t->next;
  }

  return (col);
}
        
  
//
// 'htmlWriteFile()' - Write an HTML markup tree to a file.
//

int				// O - Write status: 0 = success, -1 = fail
htmlWriteFile(hdTree *parent,	// I - Parent tree entry
              hdFile *fp)	// I - File to write to
{
  if (write_file(parent, fp, 0) < 0)
    return (-1);
  else
    return (0);
}


//
// 'htmlAddTree()' - Add a tree node to the parent.
//

hdTree *			// O - New entry
htmlAddTree(hdTree   *parent,	// I - Parent entry
            hdElement markup,	// I - Markup code
            hdChar    *data)	// I - Data/text
{
  hdTree	*t;		// New tree entry


  if ((t = htmlNewTree(parent, markup, data)) == NULL)
    return (NULL);

  // Add the tree entry to the end of the chain of children...
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


//
// 'htmlDeleteTree()' - Free all memory associated with a tree...
//

int				// O - 0 for success, -1 for failure
htmlDeleteTree(hdTree *parent)	// I - Parent to delete
{
  hdTree	*next;		// Next tree entry


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


//
// 'htmlInsertTree()' - Insert a tree node under the parent.
//

hdTree *			// O - New entry
htmlInsertTree(hdTree   *parent,// I - Parent entry
               hdElement markup,	// I - Markup code
               hdChar    *data)	// I - Data/text
{
  hdTree	*t;		// New tree entry


  if ((t = htmlNewTree(parent, markup, data)) == NULL)
    return (NULL);

  // Insert the tree entry at the beginning of the chain of children...
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


//
// 'htmlNewTree()' - Create a new tree node for the parent.
//

hdTree *				/// O - New entry
htmlNewTree(hdTree    *parent,		/// I - Parent entry
            hdElement element,		/// I - Element code
            hdChar    *data)		/// I - Data/text
{
  hdTree	*t;			/// New tree entry


  // Allocate a new tree entry - use calloc() to get zeroed data...
  t = (hdTree *)calloc(sizeof(hdTree), 1);
  if (t == NULL)
    return (NULL);

  // Inherit characteristics as needed...
  if (parent)
  {
    t->link  = parent->link;
    t->style = parent->style;
  }

  if (!t->style)
    t->style = _htmlStyleSheet->find_style(HD_ELEMENT_BODY);

  // Set the element code and copy the data if necessary...
  t->element = element;
  t->parent  = parent;

  if (data)
  {
    t->data = (hdChar *)strdup((char *)data);
    compute_size(t);
  }

  // Return the new node...
  return (t);
}


//
// 'htmlGetText()' - Get all text from the given tree.
//

hdChar *				// O - String containing text nodes
htmlGetText(hdTree *t)			// I - Tree to pick
{
  hdChar	*s,			// String
		*s2,			// New string
		*tdata;			// Temporary string data
  int		slen,			// Length of string
		tlen;			// Length of node string


  // Loop through all of the nodes in the tree and collect text...
  slen = 0;
  s    = NULL;

  while (t != NULL)
  {
    if (t->child)
      tdata = htmlGetText(t->child);
    else
      tdata = t->data;

    if (tdata != NULL)
    {
      // Add the text to this string...
      tlen = strlen((char *)tdata);

      if (slen)
        s2 = (hdChar *)realloc(s, 1 + slen + tlen);
      else
        s2 = (hdChar *)malloc(1 + tlen);

      if (!s2)
        break;

      s = s2;

      memcpy((char *)s + slen, (char *)tdata, tlen);

      slen += tlen;

      if (t->child)
	free(tdata);
    }

    t = t->next;
  }

  if (slen)
    s[slen] = '\0';

  return (s);
}


//
// 'htmlGetMeta()' - Get document "meta" data...
//

hdChar *				// O - Content string
htmlGetMeta(hdTree     *tree,		// I - Document tree
            const char *name)		// I - Metadata name
{
  hdChar	*tname,			// Name value from tree entry
		*tcontent;		// Content value from tree entry


  while (tree != NULL)
  {
    // Check this tree entry...
    if (tree->element == HD_ELEMENT_META &&
        (tname = htmlGetAttr(tree, "NAME")) != NULL &&
        (tcontent = htmlGetAttr(tree, "CONTENT")) != NULL)
      if (strcasecmp((char *)name, (char *)tname) == 0)
        return (tcontent);

    // Check child entries...
    if (tree->child != NULL)
      if ((tcontent = htmlGetMeta(tree->child, name)) != NULL)
        return (tcontent);

    // Next tree entry...
    tree = tree->next;
  }

  return (NULL);
}


//
// 'htmlGetAttr()' - Get a variable value from a markup entry.
//

hdChar *				// O - Value or NULL if variable does not exist
htmlGetAttr(hdTree     *t,		// I - Tree entry
            const char *name)		// I - Variable name
{
  hdTreeAttr	*v,			// Matching variable
		key;			// Search key


  if (t == NULL || name == NULL || t->nattrs == 0)
    return (NULL);

  key.name = (char *)name;

  v = (hdTreeAttr *)bsearch(&key, t->attrs, t->nattrs, sizeof(hdTreeAttr),
                            (compare_func_t)compare_variables);
  if (v == NULL)
    return (NULL);
  else if (v->value == NULL)
    return ((hdChar *)"");
  else
    return (v->value);
}


//
// 'htmlSetAttr()' - Set a variable for a markup entry.
//

int					// O - Set status: 0 = success, -1 = fail
htmlSetAttr(hdTree     *t,		// I - Tree entry
            const char *name,		// I - Variable name
            hdChar     *value)		// I - Variable value
{
  hdTreeAttr	*v,			// Matching variable
		key;			// Search key


  DEBUG_printf(("%shtmlSetAttr(%p, \"%s\", \"%s\")\n", indent, t, name,
                value ? (const char *)value : "(null)"));

  if (t->nattrs == 0)
    v = NULL;
  else
  {
    key.name = (char *)name;

    v = (hdTreeAttr *)bsearch(&key, t->attrs, t->nattrs, sizeof(hdTreeAttr),
        	    	      (compare_func_t)compare_variables);
  }

  if (v == NULL)
  {
    if (t->nattrs == 0)
      v = (hdTreeAttr *)malloc(sizeof(hdTreeAttr));
    else
      v = (hdTreeAttr *)realloc(t->attrs, sizeof(hdTreeAttr) * (t->nattrs + 1));

    if (v == NULL)
    {
      DEBUG_printf(("%s==== MALLOC/REALLOC FAILED! ====\n", indent));

      return (-1);
    }

    t->attrs  = v;
    v        += t->nattrs;
    t->nattrs ++;
    v->name  = strdup(name);
    if (value != NULL)
      v->value = (hdChar *)strdup((char *)value);
    else
      v->value = NULL;

    if (strcasecmp(name, "HREF") == 0)
    {
      DEBUG_printf(("%s---- Set link to %s ----\n", indent, value));
      t->link = t;
    }

    if (t->nattrs > 1)
      qsort(t->attrs, t->nattrs, sizeof(hdTreeAttr),
            (compare_func_t)compare_variables);
  }
  else if (v->value != value)
  {
    if (v->value != NULL)
      free(v->value);
    if (value != NULL)
      v->value = (hdChar *)strdup((char *)value);
    else
      v->value = NULL;
  }

  return (0);
}


//
// 'compare_variables()' - Compare two markup variables.
//

static int			// O - -1 if v0 < v1, 0 if v0 == v1, 1 if v0 > v1
compare_variables(hdTreeAttr *v0,	// I - First variable
                  hdTreeAttr *v1)	// I - Second variable
{
  return (strcasecmp((char *)v0->name, (char *)v1->name));
}


//
// 'delete_node()' - Free all memory associated with a node...
//

static void
delete_node(hdTree *t)		// I - Node to delete
{
  int		i;		// Looping var
  hdTreeAttr		*var;		// Current variable


  if (t == NULL)
    return;

  if (t->data != NULL)
    free(t->data);

  for (i = t->nattrs, var = t->attrs; i > 0; i --, var ++)
  {
    free(var->name);
    if (var->value != NULL)
      free(var->value);
  }

  if (t->attrs != NULL)
    free(t->attrs);

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
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for HTML tree node!");
    return;
  }

  // Set/copy font characteristics...
  if (parent)
    space->style = parent->style;

  // Initialize element data...
  space->element = HD_ELEMENT_NONE;
  space->data   = (hdChar *)strdup(" ");

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


//
// 'parse_markup()' - Parse a markup string.
//

static int				// O - -1 on error, HD_ELEMENT_nnnn otherwise
parse_markup(hdTree *t,			// I - Current tree entry
             hdFile *fp,		// I - Input file
	     int    *linenum)		// O - Current line number
{
  int		ch, ch2;		// Characters from file
  char		markup[255],		// Markup string...
		*mptr,			// Current character...
		comment[10240],		// Comment string
		*cptr;			// Current char...


  mptr = markup;

  while ((ch = fp->get()) != EOF && mptr < (markup + sizeof(markup) - 1))
    if (ch == '>' || isspace(ch))
      break;
    else if (ch == '/' && mptr > markup)
    {
      // Look for "/>"...
      ch = fp->get();

      if (ch != '>')
        return (HD_ELEMENT_ERROR);

      break;
    }
    else
    {
      *mptr++ = ch;

      // Handle comments without whitespace...
      if ((mptr - markup) == 3 && strncmp((const char *)markup, "!--", 3) == 0)
      {
        ch = fp->get();
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (HD_ELEMENT_ERROR);

  t->element = _htmlStyleSheet->get_element(markup);

  if (t->element == HD_ELEMENT_UNKNOWN)
  {
    // Unrecognized markup stuff...
    strlcpy((char *)comment, (char *)markup, sizeof(comment));
    cptr = comment + strlen((char *)comment);

    DEBUG_printf(("%s%s (unrecognized!)\n", indent, markup));
  }
  else
  {
    cptr = comment;

    DEBUG_printf(("%s%s, line %d\n", indent, markup, *linenum));
  }

  if (t->element == HD_ELEMENT_COMMENT || t->element == HD_ELEMENT_UNKNOWN)
  {
    int lastch = ch;			// Last character seen


    while (ch != EOF && cptr < (comment + sizeof(comment) - 2))
    {
      if (ch == '>' && t->element == HD_ELEMENT_UNKNOWN)
        break;

      if (ch == '\n')
        (*linenum) ++;

      if (ch == '-' && lastch == '-')
      {
        *cptr++ = ch;

        if ((ch2 = fp->get()) == '>')
	{
	  // Erase trailing -->
	  cptr -= 2;
	  break;
        }
	else
	  ch = ch2;
      }
      else
      {
        if (ch == '&')
	{
          // Handle character entities...
	  hdChar	entity[16],	// Character entity name
			*eptr;		// Pointer into name


	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
		 (ch = fp->get()) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

	  if (ch != ';')
	  {
	    fp->unget(ch);
	    ch = 0;
	  }

	  *eptr = '\0';
	  if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                   *linenum);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity,
	            sizeof(comment) - (cptr - comment));
	    cptr += strlen((char *)cptr);
	  }
	  else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                   entity, *linenum);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity,
	            sizeof(comment) - (cptr - comment));
	    cptr += strlen((char *)cptr);
            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = ';';
	  }
	  else
	    *cptr++ = ch;
	}
	else
	  *cptr++ = ch;

        lastch = ch;
        ch     = fp->get();
      }
    }

    *cptr = '\0';
    t->data = (hdChar *)strdup((char *)comment);
  }
  else
  {
    while (ch != EOF && ch != '>')
    {
      if (ch == '\n')
        (*linenum) ++;

      if (!isspace(ch))
      {
        fp->unget(ch);
        parse_variable(t, fp, linenum);
      }

      ch = fp->get();

      if (ch == '/')
      {
	// Look for "/>"...
	ch = fp->get();

	if (ch != '>')
          return (HD_ELEMENT_ERROR);

	break;
      }
    }
  }

  return (t->element);
}


//
// 'parse_variable()' - Parse a markup variable string.
//

static int				// O - -1 on error, 0 on success
parse_variable(hdTree *t,		// I - Current tree entry
               hdFile *fp,		// I - Input file
	       int    *linenum)		// I - Current line number
{
  hdChar	name[1024],		// Name of variable
		value[10240],		// Value of variable
		*ptr,			// Temporary pointer
		entity[16],		// Character entity name
		*eptr;			// Pointer into name
  int		ch;			// Character from file


  ptr = name;
  while ((ch = fp->get()) != EOF)
    if (isspace(ch) || ch == '=' || ch == '>' || ch == '\r')
      break;
    else if (ch == '/' && ptr == name)
      break;
    else if (ptr < (name + sizeof(name) - 1))
      *ptr++ = ch;

  *ptr = '\0';

  if (ch == '\n')
    (*linenum) ++;

  while (isspace(ch) || ch == '\r')
  {
    ch = fp->get();

    if (ch == '\n')
      (*linenum) ++;
  }

  switch (ch)
  {
    default :
        fp->unget(ch);
        return (htmlSetAttr(t, (char *)name, NULL));
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
	  {
            if (ch == '\'')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = fp->get()) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        fp->unget(ch);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                       entity, *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	      {
		if ((ch & 0x80) &&
	            _htmlStyleSheet->encoding == HD_FONT_ENCODING_UTF8)
	        {
		  // Collect UTF-8 value...
		  ch = utf8_getc(ch, fp);
		}

	        if (ch)
		  *ptr++ = ch;
              }
	    }
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
              *ptr++ = ch;
	    else if (ch == '\n')
	    {
	      if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = ' ';

	      (*linenum) ++;
	    }
	  }

          *ptr = '\0';
        }
        else if (ch == '\"')
        {
          while ((ch = fp->get()) != EOF)
	  {
            if (ch == '\"')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = fp->get()) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        fp->unget(ch);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                       entity, *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = ch;
	    }
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
            {
	      if ((ch & 0x80) &&
		  _htmlStyleSheet->encoding == HD_FONT_ENCODING_UTF8)
	      {
		// Collect UTF-8 value...
		ch = utf8_getc(ch, fp);
	      }

	      if (ch)
		*ptr++ = ch;
	    }
	    else if (ch == '\n')
	    {
	      if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = ' ';

	      (*linenum) ++;
	    }
	  }

          *ptr = '\0';
        }
        else
        {
          *ptr++ = ch;
          while ((ch = fp->get()) != EOF)
	  {
            if (isspace(ch) || ch == '>' || ch == '\r')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = fp->get()) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        fp->unget(ch);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = _htmlStyleSheet->get_entity((char *)entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
	                       entity, *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = ch;
	    }
            else if (ptr < (value + sizeof(value) - 1))
	    {
	      if ((ch & 0x80) &&
		  _htmlStyleSheet->encoding == HD_FONT_ENCODING_UTF8)
	      {
		// Collect UTF-8 value...
		ch = utf8_getc(ch, fp);
	      }

	      if (ch)
		*ptr++ = ch;
            }
	  }

	  if (ch == '\n')
	    (*linenum) ++;

          *ptr = '\0';
          if (ch == '>')
            fp->unget(ch);
        }

        return (htmlSetAttr(t, (char *)name, value));
  }
}


//
// 'compute_size()' - Compute the width and height of a tree entry.
//

static int			// O - 0 = success, -1 = failure
compute_size(hdTree *t)		// I - Tree entry
{
  hdChar	*width_ptr,	// Pointer to width string
		*height_ptr,	// Pointer to height string
		*size_ptr,	// Pointer to size string
		*type_ptr;	// Pointer to spacer type string
  hdImage	*img;		// Image
  char		number[255];	// Width or height value


  if (t->element == HD_ELEMENT_IMG)
  {
    width_ptr  = htmlGetAttr(t, "WIDTH");
    height_ptr = htmlGetAttr(t, "HEIGHT");

    if ((img = hdImage::find((char *)htmlGetAttr(t, "_HD_SRC"),
                             _htmlStyleSheet->grayscale)) == NULL)
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open image file \"%s\"",
		     (char *)htmlGetAttr(t, "SRC"));

    if (width_ptr != NULL && height_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlStyleSheet->ppi * 72.0f;
      t->height = atoi((char *)height_ptr) / _htmlStyleSheet->ppi * 72.0f;

      return (0);
    }

    if (img == NULL)
      return (-1);

    if (width_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlStyleSheet->ppi * 72.0f;
      t->height = t->width * img->height() / img->width();

      sprintf(number, "%d",
              atoi((char *)width_ptr) * img->height() / img->width());
      if (strchr((char *)width_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetAttr(t, "HEIGHT", (hdChar *)number);
    }
    else if (height_ptr != NULL)
    {
      t->height = atoi((char *)height_ptr) / _htmlStyleSheet->ppi * 72.0f;
      t->width  = t->height * img->width() / img->height();

      sprintf(number, "%d",
              atoi((char *)height_ptr) * img->width() / img->height());
      if (strchr((char *)height_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetAttr(t, "WIDTH", (hdChar *)number);
    }
    else
    {
      t->width  = img->width() / _htmlStyleSheet->ppi * 72.0f;
      t->height = img->height() / _htmlStyleSheet->ppi * 72.0f;

      sprintf(number, "%d", img->width());
      htmlSetAttr(t, "WIDTH", (hdChar *)number);

      sprintf(number, "%d", img->height());
      htmlSetAttr(t, "HEIGHT", (hdChar *)number);
    }

    return (0);
  }
  else if (t->element == HD_ELEMENT_SPACER)
  {
    width_ptr  = htmlGetAttr(t, "WIDTH");
    height_ptr = htmlGetAttr(t, "HEIGHT");
    size_ptr   = htmlGetAttr(t, "SIZE");
    type_ptr   = htmlGetAttr(t, "TYPE");

    if (width_ptr != NULL)
      t->width = atoi((char *)width_ptr) / _htmlStyleSheet->ppi * 72.0f;
    else if (size_ptr != NULL)
      t->width = atoi((char *)size_ptr) / _htmlStyleSheet->ppi * 72.0f;
    else
      t->width = 1.0f;

    if (height_ptr != NULL)
      t->height = atoi((char *)height_ptr) / _htmlStyleSheet->ppi * 72.0f;
    else if (size_ptr != NULL)
      t->height = atoi((char *)size_ptr) / _htmlStyleSheet->ppi * 72.0f;
    else
      t->height = 1.0f;

    if (type_ptr == NULL)
      return (0);

    if (strcasecmp((char *)type_ptr, "horizontal") == 0)
      t->height = 0.0;
    else if (strcasecmp((char *)type_ptr, "vertical") == 0)
      t->width = 0.0;

    return (0);
  }
  else if (t->data)
    t->width = t->style->get_width(t->data);
  else
    t->width = 0.0f;

  t->height = t->style->font_size;

  DEBUG_printf(("%swidth = %.1f, height = %.1f\n",
                indent, t->width, t->height));

  return (0);
}


//
// 'fix_filename()' - Fix a filename to be relative to the base directory.
//

static char *				// O - Fixed filename
fix_filename(const char *filename,	// I - Original filename
             const char *base,		// I - Base directory
	     char       *buffer,	// I - Filename buffer
	     size_t     bufsize)	// I - Size of filename buffer
{
  char		*slash;			// Location of slash
  char		temp[1024],		// Temporary filename
		*tempptr,		// Pointer into filename
		newfilename[1024];	// New filename


//  printf("fix_filename(filename=\"%s\", base=\"%s\")\n", filename, base);

  if (filename == NULL)
    return (NULL);

  // Unescape filenames as needed...
  if (strchr(filename, '%') && !strstr(filename, "//"))
  {
    for (tempptr = temp; *filename && tempptr < (temp + sizeof(temp) - 1);)
    {
      if (*filename == '%')
      {
        // Decode hex-escaped filename character...
	filename ++;
	if (isxdigit(filename[0] & 255) && isxdigit(filename[1] & 255))
	{
	  if (isdigit(filename[0] & 255))
	    *tempptr = (filename[0] - '0') << 4;
	  else
	    *tempptr = (tolower(filename[0]) - 'a' + 10) << 4;

	  if (isdigit(filename[1] & 255))
	    *tempptr |= filename[1] - '0';
	  else
	    *tempptr |= tolower(filename[0]) - 'a' + 10;

          tempptr ++;
	  filename += 2;
	}
	else
	  *tempptr++ = '%';
      }
      else
        *tempptr++ = *filename++;
    }

    *tempptr = '\0';
    filename = temp;
  }

  if (strcmp(base, ".") == 0 || strstr(filename, "//") != NULL)
    return (hdFile::find(Path, filename, buffer, bufsize));

  if (strncmp(filename, "./", 2) == 0 ||
      strncmp(filename, ".\\", 2) == 0)
    filename += 2;

  if (strncmp(base, "http://", 7) == 0 || strncmp(base, "https://", 8) == 0)
  {
    strlcpy(newfilename, base, sizeof(newfilename));
    base = strchr(newfilename, ':') + 3;

    if (filename[0] == '/')
    {
      if ((slash = (char *)strchr(base, '/')) != NULL)
        strlcpy(slash, filename, sizeof(newfilename) - (slash - newfilename));
      else
        strlcat(newfilename, filename, sizeof(newfilename));

      strlcpy(buffer, newfilename, bufsize);
      return (buffer);
    }
    else if ((slash = (char *)strchr(base, '/')) == NULL)
      strlcat(newfilename, "/", sizeof(newfilename));
  }
  else
  {
    if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
	base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
      // No change needed for absolute path
      return (hdFile::find(Path, filename, buffer, bufsize));

    strlcpy(newfilename, base, sizeof(newfilename));
    base = newfilename;
  }

#if defined(WIN32) || defined(__EMX__)
  while (strncmp(filename, "../", 3) == 0 ||
         strncmp(filename, "..\\", 3) == 0)
#else
  while (strncmp(filename, "../", 3) == 0)
#endif // WIN32 || __EMX__
  {
    filename += 3;
#if defined(WIN32) || defined(__EMX__)
    if ((slash = (char *)strrchr(base, '/')) != NULL)
      *slash = '\0';
    else if ((slash = (char *)strrchr(base, '\\')) != NULL)
      *slash = '\0';
#else
    if ((slash = (char *)strrchr(base, '/')) != NULL)
      *slash = '\0';
#endif // WIN32 || __EMX__
    else
    {
      filename -= 3;
      break;
    }
  }

  if (filename[0] != '/' && *base && base[strlen(base) - 1] != '/')
    strlcat(newfilename, "/", sizeof(newfilename));

  strlcat(newfilename, filename, sizeof(newfilename));

//  printf("    newfilename=\"%s\"\n", newfilename);

  return (hdFile::find(Path, newfilename, buffer, bufsize));
}


//
// 'html_memory_used()' - Figure out the amount of memory that was used.
//

static int				// O - Bytes used
html_memory_used(hdTree *t,		// I - Tree node
                 int    *nodes)		// O - Number of nodes
{
  int	i;				// Looping var
  int	bytes;				// Bytes used


  if (t == NULL)
    return (0);

  bytes = 0;

  while (t != NULL)
  {
    (*nodes) ++;

    bytes += sizeof(hdTree);
    bytes += t->nattrs * sizeof(hdTreeAttr);

    for (i = 0; i < t->nattrs; i ++)
    {
      bytes += (strlen((char *)t->attrs[i].name) + 8) & ~7;

      if (t->attrs[i].value != NULL)
        bytes += (strlen((char *)t->attrs[i].value) + 8) & ~7;
    }

    if (t->data != NULL)
      bytes += (strlen((char *)t->data) + 8) & ~7;

    bytes += html_memory_used(t->child, nodes);

    t = t->next;
  }

  return (bytes);
}


//
// 'htmlDebugStats()' - Display debug statistics for HTML tree memory use.
//

void
htmlDebugStats(const char *title,	// I - Title
               hdTree     *t)		// I - Document root node
{
  const char	*debug;			// HTMLDOC_DEBUG env var
  int		nodes,			// Number of nodes
		bytes;			// Number of bytes


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

  nodes = 0;
  bytes = html_memory_used(t, &nodes);

  progress_error(HD_ERROR_NONE, "DEBUG: %s = %dk, %d nodes", title,
                 (bytes + 1023) / 1024, nodes);
}


//
// 'style_memory_used()' - Figure out the amount of memory that was used for styles.
//

static int				// O - Bytes used
style_memory_used(hdStyle *s)		// I - Style
{
  int			i;		// Looping var
  int			bytes;		// Bytes used
  hdStyleSelector	*sel;		// Current selector


  bytes = sizeof(hdStyle);
  bytes += s->num_selectors * sizeof(hdStyleSelector);

  for (i = 0, sel = s->selectors; i < s->num_selectors; i ++, sel ++)
  {
    if (sel->class_)
      bytes += (strlen(sel->class_) + 8) & ~7;
    if (sel->id)
      bytes += (strlen(sel->id) + 8) & ~7;
    if (sel->pseudo)
      bytes += (strlen(sel->pseudo) + 8) & ~7;
  }

  if (s->background_image)
    bytes += (strlen(s->background_image) + 8) & ~7;

  for (i = 0; i < 2; i ++)
    if (s->background_position_rel[i])
      bytes += (strlen(s->background_position_rel[i]) + 8) & ~7;

  for (i = 0; i < 4; i ++)
    if (s->border[i].width_rel)
      bytes += (strlen(s->border[i].width_rel) + 8) & ~7;

  if (s->font_family)
    bytes += (strlen(s->font_family) + 8) & ~7;

  if (s->height_rel)
    bytes += (strlen(s->height_rel) + 8) & ~7;

  if (s->line_height_rel)
    bytes += (strlen(s->line_height_rel) + 8) & ~7;

  for (i = 0; i < 4; i ++)
    if (s->margin_rel[i])
      bytes += (strlen(s->margin_rel[i]) + 8) & ~7;

  for (i = 0; i < 4; i ++)
    if (s->padding_rel[i])
      bytes += (strlen(s->padding_rel[i]) + 8) & ~7;

  if (s->text_indent_rel)
    bytes += (strlen(s->text_indent_rel) + 8) & ~7;

  if (s->width_rel)
    bytes += (strlen(s->width_rel) + 8) & ~7;

  return (bytes);
}


//
// 'htmlDebugStyleStats()' - Display debug statistics for stylesheet memory use.
//

void
htmlDebugStyleStats(void)
{
  const char	*debug;			// HTMLDOC_DEBUG env var
  int		i, j;			// Looping vars
  int		sbytes;			// Stylesheet bytes
  hdStyleFont	*f;			// Current font


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

  for (sbytes = sizeof(hdStyleSheet), i = 0;
       i < _htmlStyleSheet->num_styles;
       i ++)
    sbytes += style_memory_used(_htmlStyleSheet->styles[i]);

  sbytes += style_memory_used(&(_htmlStyleSheet->def_style)) - sizeof(hdStyle);

  for (i = 0; i < _htmlStyleSheet->num_fonts; i ++)
  {
    if (_htmlStyleSheet->font_names[i])
      sbytes += (strlen(_htmlStyleSheet->font_names[i]) + 8) & ~7;

    for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
      if ((f = _htmlStyleSheet->fonts[i][j]) != NULL)
      {
        sbytes += sizeof(hdStyleFont);
	sbytes += (strlen(f->ps_name) + 8) & ~7;
	sbytes += (strlen(f->full_name) + 8) & ~7;
	sbytes += (strlen(f->font_file) + 8) & ~7;
	sbytes += f->num_widths * sizeof(float);
	sbytes += f->num_kerns * sizeof(hdFontKernPair);
      }
  }

  sbytes += (strlen(_htmlStyleSheet->charset) + 8) & ~7;
  sbytes += sizeof(_htmlStyleSheet->glyphs);
  for (i = 0; i < 256; i ++)
    if (_htmlStyleSheet->glyphs[i])
      sbytes += (strlen(_htmlStyleSheet->glyphs[i]) + 8) & ~7;

  progress_error(HD_ERROR_NONE, "DEBUG: Stylesheet = %dk, %d styles, %d fonts",
		 (sbytes + 1023) / 1024, _htmlStyleSheet->num_styles,
		 _htmlStyleSheet->num_fonts);
}


//
// 'htmlFindElement()' - Find an element in the document.
//

hdTree *				// O - Matching element node or NULL
htmlFindElement(hdTree    *doc,		// I - Document
                hdElement element)	// I - Element to find
{
  hdTree	*tree,			// Current node
		*match;			// Marching node


  if (!doc)
    return (NULL);

  for (tree = doc; tree; tree = tree->next)
    if (tree->element == element)
      return (tree);
    else if (tree->child &&
             (match = htmlFindElement(tree->child, element)) != NULL)
      return (match);

  return (NULL);
}


//
// 'htmlFindFile()' - Find a file in the document.
//

hdTree *				// O - Node for file
htmlFindFile(hdTree     *doc,		// I - Document pointer
             const char *filename)	// I - Filename
{
  hdTree	*tree;			// Current node
  hdChar	*treename;		// Filename from node


  if (!filename || !doc)
    return (NULL);

  for (tree = doc; tree; tree = tree->next)
    if ((treename = htmlGetAttr(tree, "_HD_hdFileNAME")) != NULL &&
        !strcmp((char *)treename, filename))
      return (tree);

  return (NULL);
}


//
// 'htmlFixLinks()' - Fix the external links in the document.
//

void
htmlFixLinks(hdTree     *doc,		// I - Top node
             hdTree     *tree,		// I - Current node
	     const char *base)		// I - Base directory/path
{
  hdChar	*href;			// HREF attribute
  char		full_href[1024],	// Full HREF value
		base_href[1024];	// Basename of HREF value
  const char	*debug;			// HTMLDOC_DEBUG environment variable
  static int	show_debug = -1;	// Show debug messages?


  if (show_debug < 0)
  {
    if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
	(strstr(debug, "all") == NULL && strstr(debug, "links") == NULL))
      show_debug = 0;
    else
      show_debug = 1;
  }

  while (tree)
  {
    if (tree->element == HD_ELEMENT_A && base && base[0] &&
        (href = htmlGetAttr(tree, "HREF")) != NULL)
    {
      // Check if the link needs to be localized...
      if (href[0] != '#' && !hdFile::scheme((char *)href) &&
          hdFile::scheme((char *)base) &&
	  htmlFindFile(doc, hdFile::basename((char *)href, base_href,
	                                     sizeof(base_href))) == NULL)
      {
        // Yes, localize it...
	if (href[0] == '/')
	{
	  // Absolute URL, just copy scheme, server, etc.
	  char *ptr;			// Pointer into URL...

	  strlcpy(full_href, (char *)base, sizeof(full_href));

          if (href[1] == '/')
	  {
	    // Just use scheme...
	    if ((ptr = strstr(full_href, "//")) != NULL)
	      *ptr ='\0';
	  }
	  else if ((ptr = strstr(full_href, "//")) != NULL  &&
	           (ptr = strchr(ptr + 2, '/')) != NULL)
	    *ptr ='\0';

	  strlcat(full_href, (char *)href, sizeof(full_href));
	}
	else if (!strncmp((char *)href, "./", 2))
	{
	  // Relative URL of the form "./foo/bar", append href sans
	  // "./" to base to form full href...
	  snprintf(full_href, sizeof(full_href), "%s/%s", base, href + 2);
	}
	else
	{
	  // Relative URL, append href to base to form full href...
	  snprintf(full_href, sizeof(full_href), "%s/%s", base, href);
	}

        if (show_debug)
          progress_error(HD_ERROR_NONE, "DEBUG: Mapping \"%s\" to \"%s\"...\n",
	        	 href, full_href);

	htmlSetAttr(tree, "_HD_FULL_HREF", (hdChar *)full_href);
      }
      else
      {
        // No, just mirror the link in the _HD_FULL_HREF attribute...
	htmlSetAttr(tree, "_HD_FULL_HREF", href);
      }
    }
    else if (tree->element == HD_ELEMENT_FILE)
      base = (char *)htmlGetAttr(tree, "_HD_BASE");

    if (tree->child)
      htmlFixLinks(doc, tree->child, base);

    tree = tree->next;
  }
}


//
// 'htmlDeleteStyleSheet()' - Delete all of the stylesheet data.
//

void
htmlDeleteStyleSheet(void)
{
  if (_htmlStyleSheet)
  {
    delete _htmlStyleSheet;
    _htmlStyleSheet = NULL;
  }
}


//
// 'htmlInitStyleSheet()' - Initialize the stylesheet data.
//

void
htmlInitStyleSheet(void)
{
  char		filename[1024];		// Stylesheet filename
  hdFile	*fp;			// Standard stylesheet


  // Check if we have already been called...
  if (_htmlStyleSheet)
    return;

  // Create a new stylesheet and load the standard stylesheet file...
  _htmlStyleSheet = new hdStyleSheet();

  snprintf(filename, sizeof(filename), "%s/data/standard.css", _htmlData);
  if ((fp = hdFile::open(filename, HD_FILE_READ)) != NULL)
  {
    _htmlStyleSheet->load(fp, _htmlData);
    delete fp;
  }
  else
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open standard stylesheet \"%s\" - %s",
		   filename, strerror(errno));

  _htmlStyleSheet->update_styles();
}


//
// 'htmlRealNext()' - Return the next logical node in the tree.
//

hdTree *				// O - Next logical node or NULL
htmlRealNext(hdTree *tree,		// I - Current node
             bool   descend)		// I - Descend into child node?
{
  hdTree	*t;			// Current node


  // Start at the current node and find the next logical node in
  // the tree...
  if (tree->child && descend)
    return (tree->child);

  if (tree->next)
    return (tree->next);

  for (t = tree->parent; t; t = t->parent)
    if (t->next)
      return (t->next);

  return (NULL);
}


//
// 'htmlUpdateStyle()' - Update the style data for a node.
//

void
htmlUpdateStyle(hdTree     *t,		// I - Node to update
                const char *base)	// I - Base directory
{
  hdChar	*align,			// Attributes that affect style
		*background,
		*bgcolor,
		*border,
		*bordercolor,
		*cellpadding,
		*cellspacing,
		*color,
		*face,
		*hspace,
		*size,
		*valign,
		*vspace;
  int		pos;			// Position for margins, borders, etc.
  float		val;			// Measurement value
  bool		center;			// Center table/block?
  char		buffer[1024];		// String buffer


  // Get element-specific attributes...
  if (t->element == HD_ELEMENT_BODY)
  {
    align       = NULL;
    background  = htmlGetAttr(t, "BACKGROUND");
    bgcolor     = htmlGetAttr(t, "BGCOLOR");
    border      = htmlGetAttr(t, "BORDER");
    bordercolor = NULL;
    cellpadding = NULL;
    cellspacing = NULL;
    color       = htmlGetAttr(t, "TEXT");
    face        = NULL;
    hspace      = NULL;
    size        = NULL;
    valign      = NULL;
    vspace      = NULL;
  }
  else if (issuper(t->element) || isblock(t->element))
  {
    align       = htmlGetAttr(t, "ALIGN");
    background  = NULL;
    bgcolor     = NULL;
    border      = NULL;
    bordercolor = NULL;
    cellpadding = NULL;
    cellspacing = NULL;
    color       = NULL;
    face        = NULL;
    hspace      = NULL;
    size        = NULL;
    valign      = NULL;
    vspace      = NULL;
  }
  else if (t->element == HD_ELEMENT_IMG)
  {
    align       = htmlGetAttr(t, "ALIGN");
    background  = NULL;
    bgcolor     = NULL;
    border      = htmlGetAttr(t, "BORDER");
    bordercolor = NULL;
    cellpadding = NULL;
    cellspacing = NULL;
    color       = NULL;
    face        = NULL;
    size        = NULL;
    hspace      = htmlGetAttr(t, "HSPACE");
    valign      = htmlGetAttr(t, "VALIGN");
    vspace      = htmlGetAttr(t, "VSPACE");
  }
  else if (istable(t->element) || istentry(t->element) ||
           t->element == HD_ELEMENT_BODY)
  {
    align       = htmlGetAttr(t, "ALIGN");
    background  = htmlGetAttr(t, "BACKGROUND");
    bgcolor     = htmlGetAttr(t, "BGCOLOR");
    border      = htmlGetAttr(t, "BORDER");
    bordercolor = NULL;
    cellpadding = htmlGetAttr(t, "CELLPADDING");
    cellspacing = htmlGetAttr(t, "CELLSPACING");
    color       = NULL;
    face        = NULL;
    hspace      = NULL;
    size        = NULL;
    valign      = htmlGetAttr(t, "VALIGN");
    vspace      = NULL;
  }
  else if (t->element == HD_ELEMENT_FONT)
  {
    align       = NULL;
    background  = NULL;
    bgcolor     = NULL;
    border      = NULL;
    bordercolor = NULL;
    cellpadding = NULL;
    cellspacing = NULL;
    color       = htmlGetAttr(t, "COLOR");
    face        = htmlGetAttr(t, "FACE");
    hspace      = NULL;
    size        = htmlGetAttr(t, "SIZE");
    valign      = NULL;
    vspace      = NULL;
  }
  else
  {
    align       = NULL;
    background  = NULL;
    bgcolor     = NULL;
    border      = NULL;
    bordercolor = NULL;
    cellpadding = NULL;
    cellspacing = NULL;
    color       = NULL;
    face        = NULL;
    hspace      = NULL;
    size        = NULL;
    valign      = NULL;
    vspace      = NULL;
  }

  center = t->style && t->style->text_align == HD_TEXT_ALIGN_CENTER &&
           t->element == HD_ELEMENT_TABLE;

  // Create a private style and make changes as needed...
  t->style = _htmlStyleSheet->get_private_style(t,
                 center || align != NULL || background != NULL ||
		 bgcolor != NULL || border != NULL || bordercolor != NULL ||
		 cellpadding != NULL || cellspacing != NULL || color != NULL ||
		 face != NULL || hspace != NULL || size != NULL ||
		 valign != NULL || vspace != NULL);

  // Special case: TABLEs inherit ALIGN="CENTER" as left/right margins with
  // the "auto" value...
  if (center)
  {
    t->style->margin[HD_POS_LEFT] = HD_MARGIN_AUTO;
    t->style->set_string("auto", t->style->margin_rel[HD_POS_LEFT]);
    t->style->margin[HD_POS_RIGHT] = HD_MARGIN_AUTO;
    t->style->set_string("auto", t->style->margin_rel[HD_POS_RIGHT]);
  }

  // Apply attributes...
  if (align)
  {
    if (t->element == HD_ELEMENT_IMG || t->element == HD_ELEMENT_TABLE ||
        t->element == HD_ELEMENT_CAPTION)
    {
      // ALIGN attribute maps to float or vertical-align
      if (!strcasecmp((char *)align, "left"))
        t->style->float_ = HD_FLOAT_LEFT;
      else if (!strcasecmp((char *)align, "right"))
        t->style->float_ = HD_FLOAT_RIGHT;
      else if (!strcasecmp((char *)align, "top"))
        t->style->vertical_align = HD_VERTICAL_ALIGN_TOP;
      else if (!strcasecmp((char *)align, "middle"))
        t->style->vertical_align = HD_VERTICAL_ALIGN_MIDDLE;
      else if (!strcasecmp((char *)align, "bottom"))
        t->style->vertical_align = HD_VERTICAL_ALIGN_BOTTOM;
    }
    else if (!strcasecmp((char *)align, "left"))
      t->style->text_align = HD_TEXT_ALIGN_LEFT;
    else if (!strcasecmp((char *)align, "center"))
      t->style->text_align = HD_TEXT_ALIGN_CENTER;
    else if (!strcasecmp((char *)align, "right"))
      t->style->text_align = HD_TEXT_ALIGN_RIGHT;
    else if (!strcasecmp((char *)align, "justify"))
      t->style->text_align = HD_TEXT_ALIGN_JUSTIFY;
  }

  if (background)
    t->style->set_string(fix_filename((char *)background, base, buffer,
                                      sizeof(buffer)),
                         t->style->background_image);

  if (bgcolor)
    t->style->get_color((char *)bgcolor, t->style->background_color,
                        &(t->style->background_color_set));

  if (border)
  {
    val = t->style->get_length((char *)border, 1, 72.0f / _htmlStyleSheet->ppi,
                               _htmlStyleSheet);

    for (pos = 0; pos < 4; pos ++)
    {
      t->style->set_string(NULL, t->style->border[pos].width_rel);
      t->style->border[pos].width = val;
    }
  }

  if (bordercolor)
  {
    for (pos = 0; pos < 4; pos ++)
      t->style->get_color((char *)bordercolor, t->style->border[pos].color,
                          &(t->style->border[pos].color_set));
  }

  if (cellpadding)
  {
    val = t->style->get_length((char *)cellpadding, 1,
                               72.0f / _htmlStyleSheet->ppi, _htmlStyleSheet);

    for (pos = 0; pos < 4; pos ++)
    {
      t->style->set_string(NULL, t->style->padding_rel[pos]);
      t->style->padding[pos] = val;
    }
  }

  if (cellspacing)
  {
    val = t->style->get_length((char *)cellspacing, 1,
                               72.0f / _htmlStyleSheet->ppi, _htmlStyleSheet);

    for (pos = 0; pos < 4; pos ++)
    {
      t->style->set_string(NULL, t->style->margin_rel[pos]);
      t->style->margin[pos] = val;
    }
  }

  if (color)
    t->style->get_color((char *)color, t->style->color,
                        &(t->style->color_set));

  if (face)
    t->style->set_string((char *)face, t->style->font_family);

  if (hspace)
  {
    val = t->style->get_length((char *)hspace, 1,
                               72.0f / _htmlStyleSheet->ppi, _htmlStyleSheet);

    t->style->set_string(NULL, t->style->margin_rel[HD_POS_LEFT]);
    t->style->margin[HD_POS_LEFT] = val;

    t->style->set_string(NULL, t->style->margin_rel[HD_POS_RIGHT]);
    t->style->margin[HD_POS_RIGHT] = val;
  }

  if (size)
  {
    if (isdigit(size[0]))
      t->style->font_size = _htmlStyleSheet->def_style.font_size *
                            pow(1.2, atof((char *)size) - 3.0);
    else
      t->style->font_size *= pow(1.2, atof((char *)size));

    t->style->set_string(NULL, t->style->font_size_rel);
  }

  if (valign)
  {
    if (!strcasecmp((char *)valign, "top"))
      t->style->vertical_align = HD_VERTICAL_ALIGN_TOP;
    else if (!strcasecmp((char *)valign, "middle"))
      t->style->vertical_align = HD_VERTICAL_ALIGN_MIDDLE;
    else if (!strcasecmp((char *)valign, "bottom"))
      t->style->vertical_align = HD_VERTICAL_ALIGN_BOTTOM;
  }

  if (vspace)
  {
    val = t->style->get_length((char *)vspace, 1,
                               72.0f / _htmlStyleSheet->ppi, _htmlStyleSheet);

    t->style->set_string(NULL, t->style->margin_rel[HD_POS_TOP]);
    t->style->margin[HD_POS_TOP] = val;

    t->style->set_string(NULL, t->style->margin_rel[HD_POS_BOTTOM]);
    t->style->margin[HD_POS_BOTTOM] = val;
  }

  t->style->update(_htmlStyleSheet);

  if (debug_file)
    fprintf(debug_file,
            "%*sstyle->font_size=%.1f, line_height=%.1f, white_space=%d...\n",
            debug_indent, "", t->style->font_size, t->style->line_height,
	    t->style->white_space);

  // Force PRE to be preformatted...
  if (t->element == HD_ELEMENT_PRE)
    t->style->white_space = HD_WHITE_SPACE_PRE;

#ifdef DEBUG
  if (t->element == HD_ELEMENT_H1)
    printf("H1 font-family=\"%s\", font=%p...\n", t->style->font_family,
           t->style->font);
#endif // DEBUG
}


//
// 'utf8_getc()' - Get a UTF-8 encoded character.
//

static int				// O - Unicode equivalent
utf8_getc(int    ch,			// I - Initial character
          hdFile *fp)			// I - File to read from
{
  int	ch2 = -1, ch3 = -1;		// Temporary characters
  int	unicode;			// Unicode character


  if ((ch & 0xe0) == 0xc0)
  {
   /*
    * Two-byte sequence for 0x80 to 0x7ff...
    */

    ch  = (ch & 0x1f) << 6;
    ch2 = fp->get();

    if ((ch2 & 0xc0) == 0x80)
      ch |= ch2 & 0x3f;
    else
      goto bad_sequence;
  }
  else if ((ch & 0xf0) == 0xe0)
  {
   /*
    * Three-byte sequence from 0x800 to 0xffff...
    */

    ch  = (ch & 0x0f) << 12;
    ch2 = fp->get();

    if ((ch2 & 0xc0) == 0x80)
      ch |= (ch2 & 0x3f) << 6;
    else
      goto bad_sequence;

    ch3 = fp->get();

    if ((ch3 & 0xc0) == 0x80)
      ch |= ch3 & 0x3f;
    else
      goto bad_sequence;
  }
  else
    goto bad_sequence;

  // Now that we have the Unicode value, return the mapped character...
  if ((unicode = _htmlStyleSheet->unichars[ch]) != 0)
    return (unicode);
  else
    return (_htmlStyleSheet->get_entity(NULL, ch));

  bad_sequence:

  if (ch3 >= 0)
    progress_error(HD_ERROR_READ_ERROR,
                   "Bad UTF-8 character sequence %02X %02X %02X!",
		   ch, ch2, ch3);
  else if (ch2 >= 0)
    progress_error(HD_ERROR_READ_ERROR,
                   "Bad UTF-8 character sequence %02X %02X!", ch, ch2);
  else
    progress_error(HD_ERROR_READ_ERROR,
                   "Bad UTF-8 character sequence %02X!", ch);

  return (0);
}


//
// End of "$Id: htmllib.cxx,v 1.41.2.80.2.9 2005/05/09 02:05:01 mike Exp $".
//
