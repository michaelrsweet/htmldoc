/*
 * "$Id: tree.cxx,v 1.30 2004/10/25 14:30:03 mike Exp $"
 *
 *   HTML parsing routines for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2004 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   htmlReadFile()      - Read a file for HTML markup codes.
 *   write_file()        - Write a tree entry to a file...
 *   htmlWriteFile()     - Write an HTML markup tree to a file.
 *   htmlAddTree()       - Add a tree node to the parent.
 *   htmlDeleteTree()    - Free all memory associated with a tree...
 *   htmlInsertTree()    - Insert a tree node to the parent.
 *   htmlNewTree()       - Create a new tree node for the parent.
 *   htmlGetText()       - Get all text from the given tree.
 *   htmlGetMeta()       - Get document "meta" data...
 *   htmlGetStyle()      - Get a style value from a node's STYLE attribute.
 *   htmlGetVariable()   - Get a variable value from a markup entry.
 *   htmlSetVariable()   - Set a variable for a markup entry.
 *   htmlSetBaseSize()   - Set the font sizes and spacings...
 *   htmlSetCharSet()    - Set the character set for output.
 *   htmlSetTextColor()  - Set the default text color.
 *   compare_variables() - Compare two markup variables.
 *   compare_markups()   - Compare two markup strings...
 *   delete_node()       - Free all memory associated with a node...
 *   insert_space()      - Insert a whitespace character before the
 *                         specified node.
 *   parse_markup()      - Parse a markup string.
 *   parse_variable()    - Parse a markup variable string.
 *   compute_size()      - Compute the width and height of a tree entry.
 *   compute_color()     - Compute the red, green, blue color from the given
 *   get_alignment()     - Get horizontal & vertical alignment values.
 *   fix_filename()      - Fix a filename to be relative to the base directory.
 *   html_memory_used()  - Figure out the amount of memory that was used.
 *   htmlDebugStats()    - Display debug statistics for HTML tree memory use.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <math.h>


/*
 * Markup strings...
 */

const char	*_htmlMarkups[] =
		{
		  "",		/* HD_ELEMENT_NONE */
		  "!--",	/* HD_ELEMENT_COMMENT */
		  "!DOCTYPE",
		  "A",
		  "ACRONYM",
		  "ADDRESS",
		  "APPLET",
		  "AREA",
		  "B",
		  "BASE",
		  "BASEFONT",
		  "BIG",
		  "BLINK",
		  "BLOCKQUOTE",
		  "BODY",
		  "BR",
		  "CAPTION",
		  "CENTER",
		  "CITE",
		  "CODE",
		  "COL",
		  "COLGROUP",
		  "DD",
		  "DEL",
		  "DFN",
		  "DIR",
		  "DIV",
		  "DL",
		  "DT",
		  "EM",
		  "EMBED",
		  "FONT",
		  "FORM",
		  "FRAME",
		  "FRAMESET",
		  "H1",
		  "H2",
		  "H3",
		  "H4",
		  "H5",
		  "H6",
		  "H7",
		  "H8",
		  "H9",
		  "H10",
		  "H11",
		  "H12",
		  "H13",
		  "H14",
		  "H15",
		  "HEAD",
		  "HR",
		  "HTML",
		  "I",
		  "IMG",
		  "INPUT",
		  "INS",
		  "ISINDEX",
		  "KBD",
		  "LI",
		  "LINK",
		  "MAP",
		  "MENU",
		  "META",
		  "MULTICOL",
		  "NOBR",
		  "NOFRAMES",
		  "OL",
		  "OPTION",
		  "P",
		  "PRE",
		  "S",
		  "SAMP",
		  "SCRIPT",
		  "SELECT",
		  "SMALL",
		  "SPACER",
		  "STRIKE",
		  "STRONG",
		  "STYLE",
		  "SUB",
		  "SUP",
		  "TABLE",
		  "TBODY",
		  "TD",
		  "TEXTAREA",
		  "TFOOT",
		  "TH",
		  "THEAD",
		  "TITLE",
		  "TR",
		  "TT",
		  "U",
		  "UL",
		  "VAR",
		  "WBR"
		};

const char	*_htmlData = HTML_DATA;	/* Data directory */
float		_htmlPPI = 80.0f;	/* Image resolution */
int		_htmlGrayscale = 0;	/* Grayscale output? */
uchar		_htmlTextColor[255] =	/* Default text color */
		{ 0 };
float		_htmlBrowserWidth = 680.0f;
					/* Browser width for pixel scaling */
float		_htmlSizes[8] =		/* Point size for each HTML size */
		{ 6.0f, 8.0f, 9.0f, 11.0f, 14.0f, 17.0f, 20.0f, 24.0f };
float		_htmlSpacings[8] =	/* Line height for each HTML size */
		{ 7.2f, 9.6f, 10.8f, 13.2f, 16.8f, 20.4f, 24.0f, 28.8f };
hdFontFace	_htmlBodyFont = HD_FONTFACE_SERIF,
		_htmlHeadingFont = HD_FONTFACE_SANS_SERIF;

int		_htmlInitialized = 0;	/* Initialized glyphs yet? */
char		_htmlCharSet[256] = "iso-8859-1";
					/* Character set name */
float		_htmlWidths[4][4][256];	/* Character widths of fonts */
const char	*_htmlGlyphsAll[65536];	/* Character glyphs for Unicode */
const char	*_htmlGlyphs[256];	/* Character glyphs for charset */
const char	*_htmlFonts[4][4] =
		{
		  {
		    "Courier",
		    "Courier-Bold",
		    "Courier-Oblique",
		    "Courier-BoldOblique"
		  },
		  {
		    "Times-Roman",
		    "Times-Bold",
		    "Times-Italic",
		    "Times-BoldItalic"
		  },
		  {
		    "Helvetica",
		    "Helvetica-Bold",
		    "Helvetica-Oblique",
		    "Helvetica-BoldOblique"
		  },
		  {
		    "Symbol",
		    "Symbol",
		    "Symbol",
		    "Symbol"
		  }
		};


/*
 * Local functions.
 */

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static int	write_file(hdTree *t, FILE *fp, int col);
static int	compare_variables(hdAttr *v0, hdAttr *v1);
static int	compare_markups(uchar **m0, uchar **m1);
static void	delete_node(hdTree *t);
static void	insert_space(hdTree *parent, hdTree *t);
static int	parse_markup(hdTree *t, FILE *fp, int *linenum);
static int	parse_variable(hdTree *t, FILE *fp, int *linenum);
static int	compute_size(hdTree *t);
static int	compute_color(hdTree *t, uchar *color);
static int	get_alignment(hdTree *t);
static const char *fix_filename(char *path, char *base);

#define issuper(x)	((x) == HD_ELEMENT_CENTER || (x) == HD_ELEMENT_DIV ||\
			 (x) == HD_ELEMENT_BLOCKQUOTE)
#define isblock(x)	((x) == HD_ELEMENT_ADDRESS || \
			 (x) == HD_ELEMENT_P || (x) == HD_ELEMENT_PRE ||\
			 ((x) >= HD_ELEMENT_H1 && (x) <= HD_ELEMENT_H6) ||\
			 (x) == HD_ELEMENT_HR || (x) == HD_ELEMENT_TABLE)
#define islist(x)	((x) == HD_ELEMENT_DL || (x) == HD_ELEMENT_OL ||\
			 (x) == HD_ELEMENT_UL || (x) == HD_ELEMENT_DIR ||\
			 (x) == HD_ELEMENT_MENU)
#define islentry(x)	((x) == HD_ELEMENT_LI || (x) == HD_ELEMENT_DD ||\
			 (x) == HD_ELEMENT_DT)
#define istable(x)	((x) == HD_ELEMENT_TBODY || (x) == HD_ELEMENT_THEAD ||\
			 (x) == HD_ELEMENT_TFOOT || (x) == HD_ELEMENT_TR)
#define istentry(x)	((x) == HD_ELEMENT_TD || (x) == HD_ELEMENT_TH)

#ifdef DEBUG
static uchar	indent[255] = "";
#endif /* DEBUG */


/*
 * 'htmlReadFile()' - Read a file for HTML markup codes.
 */

hdTree *				// O - Pointer to top of file tree
htmlReadFile(hdTree     *parent,	// I - Parent tree entry
             FILE       *fp,		// I - File pointer
	     const char *base,		// I - Base directory for file
             hdStyleSheet *stylesheet)	// I - Stylesheet
{
  int		ch;			// Character from file
  uchar		*ptr,			// Pointer in string
		entity[16],		// Character entity name (&#nnn; or &name;)
		*eptr;			// Pointer in entity string
  hdTree	*tree,			// "top" of this tree
		*t,			// New tree entry
		*prev,			// Previous tree entry
		*temp;			// Temporary looping var
  int		descend;		// Descend into node?
  FILE		*embed;			// File pointer for EMBED
  char		newbase[1024];		// New base directory for EMBED
  uchar		*filename,		// Filename for EMBED tag
		*face,			// Typeface for FONT tag
		*color,			// Color for FONT tag
		*size,			// Size for FONT tag
		*type;			// Type for EMBED tag
  int		sizeval;		// Size value from FONT tag
  int		linenum;		// Line number in file
  static uchar	s[10240];		// String from file
  static int	have_whitespace = 0;	// Non-zero if there was leading whitespace


  DEBUG_printf(("htmlReadFile(parent=%p, fp=%p, base=\"%s\")\n",
                parent, fp, base ? base : "(null)"));

#ifdef DEBUG
  indent[0] = '\0';
#endif // DEBUG

 /*
  * Start off with no previous tree entry...
  */

  prev = NULL;
  tree = NULL;

 /*
  * Parse data until we hit end-of-file...
  */

  linenum = 1;

  while ((ch = getc(fp)) != EOF)
  {
   /*
    * Ignore leading whitespace...
    */

    if (parent == NULL || !parent->preformatted)
    {
      while (isspace(ch))
      {
	if (ch == '\n')
	  linenum ++;

        have_whitespace = 1;
        ch              = getc(fp);
      }

      if (ch == EOF)
        break;
    }

   /*
    * Allocate a new tree entry - use calloc() to get zeroed data...
    */

    t = (hdTree *)calloc(sizeof(hdTree), 1);
    if (t == NULL)
    {
#ifndef DEBUG
//      progress_error(HD_ERROR_OUT_OF_MEMORY,
//                     "Unable to allocate memory for HTML tree node!");
#endif /* !DEBUG */
      break;
    }

   /*
    * Set/copy font characteristics...
    */

    if (parent == NULL)
    {
      t->halignment   = HD_TEXT_ALIGN_LEFT;
      t->valignment   = HD_VERTICAL_ALIGN_BOTTOM;
      t->typeface     = _htmlBodyFont;
      t->size         = _htmlSizes[SIZE_P];

      compute_color(t, _htmlTextColor);
    }
    else
    {
      t->link          = parent->link;
      t->halignment    = parent->halignment;
      t->valignment    = parent->valignment;
      t->typeface      = parent->typeface;
      t->size          = parent->size;
      t->style         = parent->style;
      t->superscript   = parent->superscript;
      t->subscript     = parent->subscript;
      t->preformatted  = parent->preformatted;
      t->indent        = parent->indent;
      t->red           = parent->red;
      t->green         = parent->green;
      t->blue          = parent->blue;
      t->underline     = parent->underline;
      t->strikethrough = parent->strikethrough;
    }

   /*
    * See what the character was...
    */

    if (ch == '<')
    {
     /*
      * Markup char; grab the next char to see if this is a /...
      */

      ch = getc(fp);

      if (isspace(ch) || ch == '=' || ch == '<')
      {
       /*
        * Sigh...  "<" followed by anything but an element name is
	* invalid HTML, but so many people have asked for this to
	* be supported that we have added this hack...
	*/

//	progress_error(HD_ERROR_HTML_ERROR, "Unquoted < on line %d.", linenum);

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
	  ungetc(ch, fp);
	else
	  have_whitespace = 1;

	*ptr++ = '\0';

	t->element = HD_ELEMENT_NONE;
	t->data   = (uchar *)strdup((char *)s);
      }
      else
      {
       /*
        * Start of a markup...
	*/

	if (ch != '/')
          ungetc(ch, fp);

	if (parse_markup(t, fp, &linenum) == HD_ELEMENT_ERROR)
	{
#ifndef DEBUG
//          progress_error(HD_ERROR_READ_ERROR,
//                         "Unable to parse HTML element on line %d!", linenum);
#endif /* !DEBUG */

          delete_node(t);
          break;
	}

       /*
	* Eliminate extra whitespace...
	*/

	if (issuper(t->element) || isblock(t->element) ||
            islist(t->element) || islentry(t->element) ||
            istable(t->element) || istentry(t->element) ||
	    t->element == HD_ELEMENT_TITLE)
          have_whitespace = 0;

       /*
	* If this is the matching close mark, or if we are starting the same
	* markup, or if we've completed a list, we're done!
	*/

	if (ch == '/')
	{
	 /*
          * Close markup; find matching markup...
          */

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
	 /*
          * Make sure we aren't inside an existing HEAD or BODY...
	  */

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
	 /*
          * Close any text blocks...
	  */

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
	      temp = NULL;
              break;
	    }
	}
	else
          temp = NULL;

	if (temp != NULL)
	{
          DEBUG_printf(("%s>>>> Auto-ascend <<<\n", indent));

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
//	    progress_error(HD_ERROR_HTML_ERROR,
//	                   "No /%s element before %s element on line %d.",
//	                   _htmlMarkups[temp->element],
//			   _htmlMarkups[t->element], linenum);
	    DEBUG_printf(("%sNo /%s element before %s element on line %d.\n",
	                  indent, _htmlMarkups[temp->element],
			  _htmlMarkups[t->element], linenum));
	  }

#ifdef DEBUG
          for (hdTree *p = parent;
	       p && p != temp && indent[0];
	       p = p->parent)
	    indent[strlen((char *)indent) - 4] = '\0';

          if (indent[0])
            indent[strlen((char *)indent) - 4] = '\0';
#endif // DEBUG


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
	    if (parent == NULL)
	    {
	      t->halignment   = HD_TEXT_ALIGN_LEFT;
	      t->valignment   = HD_VERTICAL_ALIGN_BOTTOM;
	      t->typeface     = _htmlBodyFont;
	      t->size         = _htmlSizes[SIZE_P];

	      compute_color(t, _htmlTextColor);
	    }
	    else
	    {
	      t->link          = parent->link;
	      t->halignment    = parent->halignment;
	      t->valignment    = parent->valignment;
	      t->typeface      = parent->typeface;
	      t->size          = parent->size;
	      t->style         = parent->style;
	      t->superscript   = parent->superscript;
	      t->subscript     = parent->subscript;
	      t->preformatted  = parent->preformatted;
	      t->indent        = parent->indent;
	      t->red           = parent->red;
	      t->green         = parent->green;
	      t->blue          = parent->blue;
	      t->underline     = parent->underline;
	      t->strikethrough = parent->strikethrough;
	    }
	    
          }
	}
	else if (ch == '/')
	{
	  // Log this condition as an error...
	  if (t->element != HD_ELEMENT_UNKNOWN &&
	      t->element != HD_ELEMENT_COMMENT)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR,
//	                   "Dangling /%s element on line %d.",
//			   _htmlMarkups[t->element], linenum);
	    DEBUG_printf(("%sDangling /%s element on line %d.\n",
			  indent, _htmlMarkups[t->element], linenum));
          }

	  delete_node(t);
	  continue;
	}
      }
    }
    else if (t->preformatted)
    {
     /*
      * Read a pre-formatted string into the current tree entry...
      */

      ptr = s;
      while (ch != '<' && ch != EOF && ptr < (s + sizeof(s) - 1))
      {
        if (ch == '&')
        {
	  // Possibly a character entity...
	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
	         (ch = getc(fp)) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

          if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

          *eptr = '\0';
          if (!ch)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                   linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                   entity, linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else if (ptr < (s + sizeof(s) - 1))
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

        ch = getc(fp);
      }

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      t->element = HD_ELEMENT_NONE;
      t->data   = (uchar *)strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\", line %d\n", indent, s, linenum));
    }
    else
    {
     /*
      * Read the next string fragment...
      */

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
	         (ch = getc(fp)) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

          if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

          *eptr = '\0';
          if (!ch)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                   linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                   entity, linenum);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else if (ptr < (s + sizeof(s) - 1))
	    *ptr++ = ch;
        }
	else if (ch)
          *ptr++ = ch;

        ch = getc(fp);
      }

      if (ch == '\n')
	linenum ++;

      if (isspace(ch))
        have_whitespace = 1;

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      t->element = HD_ELEMENT_NONE;
      t->data   = (uchar *)strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\", line %d\n", indent, s, linenum));
    }

   /*
    * If the parent tree pointer is not null and this is the first
    * entry we've read, set the child pointer...
    */

    if (parent != NULL && prev == NULL)
      parent->child = t;

    if (parent != NULL)
      parent->last_child = t;

   /*
    * Do the prev/next links...
    */

    t->parent = parent;
    t->prev   = prev;
    if (prev != NULL)
      prev->next = t;

    if (tree == NULL)
      tree = t;

    prev = t;

   /*
    * Do markup-specific stuff...
    */

    descend = 0;

    switch (t->element)
    {
      case HD_ELEMENT_BODY :
         /*
	  * Update the text color as necessary...
	  */

          if ((color = htmlGetVariable(t, (uchar *)"TEXT")) != NULL)
            compute_color(t, color);
	  else
            compute_color(t, _htmlTextColor);

//          if ((color = htmlGetVariable(t, (uchar *)"BGCOLOR")) != NULL &&
//	      !BodyColor[0])
//	    strcpy(BodyColor, (char *)color);

          // Update the background image as necessary...
          if ((filename = htmlGetVariable(t, (uchar *)"BACKGROUND")) != NULL)
	    htmlSetVariable(t, (uchar *)"BACKGROUND",
	                    (uchar *)fix_filename((char *)filename,
			                          (char *)base));

          descend = 1;
          break;

      case HD_ELEMENT_IMG :
          if (have_whitespace)
	  {
	    // Insert a space before this image...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          // Get the image alignment...
          t->valignment = HD_VERTICAL_ALIGN_BOTTOM;
          get_alignment(t);

          // Update the image source as necessary...
          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	    htmlSetVariable(t, (uchar *)"REALSRC",
	                    (uchar *)fix_filename((char *)filename,
			                          (char *)base));

      case HD_ELEMENT_BR :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
	 /*
	  * Figure out the width & height of this markup...
	  */

          compute_size(t);
	  break;

      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_H7 :
      case HD_ELEMENT_H8 :
      case HD_ELEMENT_H9 :
      case HD_ELEMENT_H10 :
      case HD_ELEMENT_H11 :
      case HD_ELEMENT_H12 :
      case HD_ELEMENT_H13 :
      case HD_ELEMENT_H14 :
      case HD_ELEMENT_H15 :
          get_alignment(t);

          t->typeface      = _htmlHeadingFont;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;

	  if (t->element > HD_ELEMENT_H6)
          {
	    t->size  = _htmlSizes[SIZE_H7];
            t->style = HD_FONTINTERNAL_ITALIC;
	  }
	  else
	  {
            t->size  = _htmlSizes[SIZE_H1 - t->element + HD_ELEMENT_H1];
            t->style = HD_FONTINTERNAL_BOLD;
	  }

          descend = 1;
          break;

      case HD_ELEMENT_P :
          get_alignment(t);

          t->typeface      = _htmlBodyFont;
          t->size          = _htmlSizes[SIZE_P];
          t->style         = HD_FONTINTERNAL_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;

          descend = 1;
          break;

      case HD_ELEMENT_PRE :
          t->typeface      = HD_FONTFACE_MONOSPACE;
          t->size          = _htmlSizes[SIZE_PRE];
          t->style         = HD_FONTINTERNAL_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 1;

          descend = 1;
          break;

      case HD_ELEMENT_BLOCKQUOTE :
      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_OL :
      case HD_ELEMENT_DL :
          t->indent ++;

          descend = 1;
          break;

      case HD_ELEMENT_DIV :
          get_alignment(t);

          descend = 1;
          break;

      case HD_ELEMENT_HR :
          t->halignment = HD_TEXT_ALIGN_CENTER;
          get_alignment(t);
          break;

      case HD_ELEMENT_DOCTYPE :
      case HD_ELEMENT_AREA :
      case HD_ELEMENT_COMMENT :
      case HD_ELEMENT_INPUT :
      case HD_ELEMENT_ISINDEX :
      case HD_ELEMENT_LINK :
      case HD_ELEMENT_META :
      case HD_ELEMENT_WBR :
      case HD_ELEMENT_COL :
          break;

      case HD_ELEMENT_EMBED :
          if ((type = htmlGetVariable(t, (uchar *)"TYPE")) != NULL &&
	      strncasecmp((const char *)type, "text/html", 9) != 0)
	    break;

          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	  {
	    filename = (uchar *)fix_filename((char *)filename,
	                                     (char *)base);

            if ((embed = fopen((char *)filename, "r")) != NULL)
            {
	      strcpy(newbase, hdBook::file_directory((char *)filename));

              htmlReadFile(t, embed, newbase, stylesheet);
              fclose(embed);
            }
#ifndef DEBUG
//	    else
//	      progress_error(HD_ERROR_FILE_NOT_FOUND,
//                             "Unable to embed \"%s\" - %s", filename,
//	                     strerror(errno));
#endif /* !DEBUG */
	  }
          break;

      case HD_ELEMENT_TH :
          if (htmlGetVariable(t->parent, (uchar *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = HD_TEXT_ALIGN_CENTER;

          if (htmlGetVariable(t->parent, (uchar *)"VALIGN") != NULL)
	    t->valignment = t->parent->valignment;
	  else
            t->valignment = HD_VERTICAL_ALIGN_MIDDLE;

          get_alignment(t);

          t->style = HD_FONTINTERNAL_BOLD;

          descend = 1;
          break;

      case HD_ELEMENT_TD :
          if (htmlGetVariable(t->parent, (uchar *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = HD_TEXT_ALIGN_LEFT;

          if (htmlGetVariable(t->parent, (uchar *)"VALIGN") != NULL)
	    t->valignment = t->parent->valignment;
	  else
            t->valignment = HD_VERTICAL_ALIGN_MIDDLE;

	  get_alignment(t);

          t->style = HD_FONTINTERNAL_NORMAL;

          descend = 1;
          break;

      case HD_ELEMENT_FONT :
          if (have_whitespace)
	  {
	    // Insert a space before this element...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          if ((face = htmlGetVariable(t, (uchar *)"FACE")) != NULL)
          {
            for (ptr = face; *ptr != '\0'; ptr ++)
              *ptr = tolower(*ptr);

            if (strstr((char *)face, "arial") != NULL ||
                strstr((char *)face, "helvetica") != NULL)
              t->typeface = HD_FONTFACE_SANS_SERIF;
            else if (strstr((char *)face, "times") != NULL)
              t->typeface = HD_FONTFACE_SERIF;
            else if (strstr((char *)face, "courier") != NULL)
              t->typeface = HD_FONTFACE_MONOSPACE;
	    else if (strstr((char *)face, "symbol") != NULL)
              t->typeface = HD_FONTFACE_SYMBOL;
          }

          if ((color = htmlGetVariable(t, (uchar *)"COLOR")) != NULL)
            compute_color(t, color);

          if ((size = htmlGetVariable(t, (uchar *)"SIZE")) != NULL)
          {
            if (have_whitespace)
	    {
	      // Insert a space before sized text...
	      insert_space(parent, t);

	      have_whitespace = 0;
	    }

	    if (isdigit(size[0]))
	    {
	      t->size = _htmlSizes[atoi((char *)size)];
	    }
	    else
	    {
              sizeval = atoi((char *)size);

	      if (sizeval > 0)
	        t->size *= pow(1.2, sizeval);
	      else if (sizeval < 0)
	        t->size /= pow(1.2, -sizeval);
	    }
          }

          descend = 1;
          break;

      case HD_ELEMENT_BIG :
          if (have_whitespace)
	  {
	    // Insert a space before big text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->size *= 1.44;
          descend = 1;
          break;

      case HD_ELEMENT_SMALL :
          if (have_whitespace)
	  {
	    // Insert a space before small text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->size /= 1.44;
          descend = 1;
          break;

      case HD_ELEMENT_SUP :
          if (have_whitespace)
	  {
	    // Insert a space before superscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->superscript = 1;

          t->size /= 1.44;
          descend = 1;
          break;

      case HD_ELEMENT_SUB :
          if (have_whitespace)
	  {
	    // Insert a space before subscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->subscript = 1;

          t->size /= 1.44;
          descend = 1;
          break;

      case HD_ELEMENT_KBD :
          t->style    = HD_FONTINTERNAL_BOLD;

      case HD_ELEMENT_TT :
      case HD_ELEMENT_CODE :
      case HD_ELEMENT_SAMP :
          if (isspace(ch = getc(fp)))
	    have_whitespace = 1;
	  else
	    ungetc(ch, fp);

          if (have_whitespace)
	  {
	    // Insert a space before monospaced text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->typeface = HD_FONTFACE_MONOSPACE;

          descend = 1;
          break;

      case HD_ELEMENT_B :
          t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_BOLD);

          descend = 1;
          break;

      case HD_ELEMENT_DD :
          t->indent ++;

          descend = 1;
          break;

      case HD_ELEMENT_VAR :
          t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_ITALIC);
      case HD_ELEMENT_DFN :
          t->typeface = HD_FONTFACE_SANS_SERIF;

          descend = 1;
          break;

      case HD_ELEMENT_STRONG :
          t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_BOLD);
      case HD_ELEMENT_CITE :
      case HD_ELEMENT_EM :
      case HD_ELEMENT_I :
          t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_ITALIC);

          descend = 1;
          break;

      case HD_ELEMENT_U :
      case HD_ELEMENT_INS :
          if (have_whitespace)
	  {
	    // Insert a space before underlined text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->underline = 1;

          descend = 1;
          break;

      case HD_ELEMENT_STRIKE :
      case HD_ELEMENT_S :
      case HD_ELEMENT_DEL :
          if (have_whitespace)
	  {
	    // Insert a space before struck-through text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->strikethrough = 1;

          descend = 1;
          break;

      case HD_ELEMENT_CENTER :
          t->halignment = HD_TEXT_ALIGN_CENTER;

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
         /*
          * All other markup types should be using <MARK>...</MARK>
          */

          get_alignment(t);

          descend = 1;
          break;
    }

    if (descend)
    {
#ifdef DEBUG
      strlcat((char *)indent, "    ", sizeof(indent));
#endif // DEBUG

      parent = t;
      prev   = NULL;
    }
  }  

  return (tree);
}


/*
 * 'write_file()' - Write a tree entry to a file...
 */

static int			/* I - New column */
write_file(hdTree *t,		/* I - Tree entry */
           FILE   *fp,		/* I - File to write to */
           int    col)		/* I - Current column */
{
  int	i;			/* Looping var */
  uchar	*ptr;			/* Character pointer */


  while (t != NULL)
  {
    if (t->element == HD_ELEMENT_NONE)
    {
      if (t->preformatted)
      {
        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fputs((char *)iso8859(*ptr), fp);

	if (t->data[strlen((char *)t->data) - 1] == '\n')
          col = 0;
	else
          col += strlen((char *)t->data);
      }
      else
      {
	if ((col + strlen((char *)t->data)) > 72 && col > 0)
	{
          putc('\n', fp);
          col = 0;
	}

        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fputs((char *)iso8859(*ptr), fp);

	col += strlen((char *)t->data);

	if (col > 72)
	{
          putc('\n', fp);
          col = 0;
	}
      }
    }
    else if (t->element == HD_ELEMENT_COMMENT)
      fprintf(fp, "\n<!--%s-->\n", t->data);
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
              putc('\n', fp);
              col = 0;
            }
        default :
            break;
      }

      col += fprintf(fp, "<%s", _htmlMarkups[t->element]);
      for (i = 0; i < t->nvars; i ++)
      {
	if (col > 72 && !t->preformatted)
	{
          putc('\n', fp);
          col = 0;
	}

        if (col > 0)
        {
          putc(' ', fp);
          col ++;
        }

	if (t->vars[i].value == NULL)
          col += fprintf(fp, "%s", t->vars[i].name);
	else if (strchr((char *)t->vars[i].value, '\"') != NULL)
          col += fprintf(fp, "%s=\'%s\'", t->vars[i].name, t->vars[i].value);
	else
          col += fprintf(fp, "%s=\"%s\"", t->vars[i].name, t->vars[i].value);
      }

      putc('>', fp);
      col ++;

      if (col > 72 && !t->preformatted)
      {
	putc('\n', fp);
	col = 0;
      }

      if (t->child != NULL)
      {
	col = write_file(t->child, fp, col);

	if (col > 72 && !t->preformatted)
	{
	  putc('\n', fp);
	  col = 0;
	}
	
        col += fprintf(fp, "</%s>", _htmlMarkups[t->element]);
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
              putc('\n', fp);
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
        
  
/*
 * 'htmlWriteFile()' - Write an HTML markup tree to a file.
 */

int				/* O - Write status: 0 = success, -1 = fail */
htmlWriteFile(hdTree *parent,	/* I - Parent tree entry */
              FILE   *fp)	/* I - File to write to */
{
  if (write_file(parent, fp, 0) < 0)
    return (-1);
  else
    return (0);
}


/*
 * 'htmlAddTree()' - Add a tree node to the parent.
 */

hdTree *			/* O - New entry */
htmlAddTree(hdTree   *parent,	/* I - Parent entry */
            hdElement markup,	/* I - Markup code */
            uchar    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


  if ((t = htmlNewTree(parent, markup, data)) == NULL)
    return (NULL);

 /*
  * Add the tree entry to the end of the chain of children...
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
  hdTree	*next;		/* Next tree entry */


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
               hdElement markup,	/* I - Markup code */
               uchar    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


  if ((t = htmlNewTree(parent, markup, data)) == NULL)
    return (NULL);

 /*
  * Insert the tree entry at the beginning of the chain of children...
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
            hdElement markup,	/* I - Markup code */
            uchar    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


 /*
  * Allocate a new tree entry - use calloc() to get zeroed data...
  */

  t = (hdTree *)calloc(sizeof(hdTree), 1);
  if (t == NULL)
    return (NULL);

 /*
  * Set the markup code and copy the data if necessary...
  */

  t->element = markup;
  if (data != NULL)
    t->data = (uchar *)strdup((char *)data);

 /*
  * Set/copy font characteristics...
  */

  if (parent == NULL)
  {
    t->halignment = HD_TEXT_ALIGN_LEFT;
    t->valignment = HD_VERTICAL_ALIGN_BOTTOM;
    t->typeface   = _htmlBodyFont;
    t->size       = _htmlSizes[SIZE_P];

    compute_color(t, _htmlTextColor);
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

        t->typeface      = _htmlHeadingFont;
        t->size          = _htmlSizes[SIZE_H1 - t->element + HD_ELEMENT_H1];
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        t->style         = HD_FONTINTERNAL_BOLD;
        break;

    case HD_ELEMENT_P :
        get_alignment(t);

        t->typeface      = _htmlBodyFont;
        t->size          = _htmlSizes[SIZE_P];
        t->style         = HD_FONTINTERNAL_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        break;

    case HD_ELEMENT_PRE :
        t->typeface      = HD_FONTFACE_MONOSPACE;
        t->size          = _htmlSizes[SIZE_PRE];
        t->style         = HD_FONTINTERNAL_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 1;
        break;

    case HD_ELEMENT_DIV :
        get_alignment(t);
        break;

    case HD_ELEMENT_BLOCKQUOTE :
        t->style = HD_FONTINTERNAL_ITALIC;

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
        t->style = HD_FONTINTERNAL_BOLD;
    case HD_ELEMENT_TD :
        get_alignment(t);
        break;

    case HD_ELEMENT_SUP :
        t->superscript = 1;
        t->size        = _htmlSizes[SIZE_P + SIZE_SUP];
        break;

    case HD_ELEMENT_SUB :
        t->subscript = 1;
        t->size      = _htmlSizes[SIZE_P + SIZE_SUB];
        break;

    case HD_ELEMENT_B :
        t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_BOLD);
        break;

    case HD_ELEMENT_DD :
        t->indent ++;
        break;

    case HD_ELEMENT_DT :
    case HD_ELEMENT_I :
        t->style = (hdFontInternal)(t->style | HD_FONTINTERNAL_ITALIC);
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
 * 'htmlGetText()' - Get all text from the given tree.
 */

uchar *				/* O - String containing text nodes */
htmlGetText(hdTree *t)		/* I - Tree to pick */
{
  uchar		*s,		// String
		*s2,		// New string
		*tdata;		// Temporary string data
  int		slen,		// Length of string
		tlen;		// Length of node string


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
        s2 = (uchar *)realloc(s, 1 + slen + tlen);
      else
        s2 = (uchar *)malloc(1 + tlen);

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


/*
 * 'htmlGetMeta()' - Get document "meta" data...
 */

uchar *				/* O - Content string */
htmlGetMeta(hdTree *tree,	/* I - Document tree */
            uchar  *name)	/* I - Metadata name */
{
  uchar	*tname,			/* Name value from tree entry */
	*tcontent;		/* Content value from tree entry */


  while (tree != NULL)
  {
   /*
    * Check this tree entry...
    */

    if (tree->element == HD_ELEMENT_META &&
        (tname = htmlGetVariable(tree, (uchar *)"NAME")) != NULL &&
        (tcontent = htmlGetVariable(tree, (uchar *)"CONTENT")) != NULL)
      if (strcasecmp((char *)name, (char *)tname) == 0)
        return (tcontent);

   /*
    * Check child entries...
    */

    if (tree->child != NULL)
      if ((tcontent = htmlGetMeta(tree->child, name)) != NULL)
        return (tcontent);

   /*
    * Next tree entry...
    */

    tree = tree->next;
  }

  return (NULL);
}


/*
 * 'htmlGetStyle()' - Get a style value from a node's STYLE attribute.
 */

uchar *				// O - Value or NULL
htmlGetStyle(hdTree *t,		// I - Node
             uchar  *name)	// I - Name (including ":")
{
  uchar		*ptr,		// Pointer in STYLE attribute
		*bufptr;	// Pointer in buffer
  int		ptrlen,		// Length of STYLE attribute
		namelen;	// Length of name
  static uchar	buffer[1024];	// Buffer for value


  // See if we have a STYLE attribute...
  if ((ptr = htmlGetVariable(t, (uchar *)"STYLE")) == NULL)
    return (NULL);

  // Loop through the STYLE attribute looking for the name...
  for (namelen = strlen((char *)name), ptrlen = strlen((char *)ptr);
       ptrlen > namelen;
       ptr ++, ptrlen --)
    if (strncasecmp((char *)name, (char *)ptr, namelen) == 0)
    {
      for (ptr += namelen; isspace(*ptr); ptr ++);

      for (bufptr = buffer;
           *ptr && *ptr != ';' && bufptr < (buffer + sizeof(buffer) - 1);
	   *bufptr++ = *ptr++);

      *bufptr = '\0';

      return (buffer);
    }

  return (NULL);
}


/*
 * 'htmlGetVariable()' - Get a variable value from a markup entry.
 */

uchar *				/* O - Value or NULL if variable does not exist */
htmlGetVariable(hdTree *t,	/* I - Tree entry */
                uchar  *name)	/* I - Variable name */
{
  hdAttr	*v,			/* Matching variable */
	key;			/* Search key */


  if (t == NULL || name == NULL || t->nvars == 0)
    return (NULL);

  key.name = name;

  v = (hdAttr *)bsearch(&key, t->vars, t->nvars, sizeof(hdAttr),
                       (compare_func_t)compare_variables);
  if (v == NULL)
    return (NULL);
  else if (v->value == NULL)
    return ((uchar *)"");
  else
    return (v->value);
}


/*
 * 'htmlSetVariable()' - Set a variable for a markup entry.
 */

int				/* O - Set status: 0 = success, -1 = fail */
htmlSetVariable(hdTree *t,	/* I - Tree entry */
                uchar  *name,	/* I - Variable name */
                uchar  *value)	/* I - Variable value */
{
  hdAttr	*v,			/* Matching variable */
	key;			/* Search key */


  DEBUG_printf(("%shtmlSetVariable(%p, \"%s\", \"%s\")\n", indent, t, name,
                value ? (const char *)value : "(null)"));

  if (t->nvars == 0)
    v = NULL;
  else
  {
    key.name = name;

    v = (hdAttr *)bsearch(&key, t->vars, t->nvars, sizeof(hdAttr),
        	         (compare_func_t)compare_variables);
  }

  if (v == NULL)
  {
    if (t->nvars == 0)
      v = (hdAttr *)malloc(sizeof(hdAttr));
    else
      v = (hdAttr *)realloc(t->vars, sizeof(hdAttr) * (t->nvars + 1));

    if (v == NULL)
    {
      DEBUG_printf(("%s==== MALLOC/REALLOC FAILED! ====\n", indent));

      return (-1);
    }

    t->vars  = v;
    v        += t->nvars;
    t->nvars ++;
    v->name  = (uchar *)strdup((char *)name);
    if (value != NULL)
      v->value = (uchar *)strdup((char *)value);
    else
      v->value = NULL;

    if (strcasecmp((char *)name, "HREF") == 0)
    {
      DEBUG_printf(("%s---- Set link to %s ----\n", indent, value));
      t->link = t;
    }

    if (t->nvars > 1)
      qsort(t->vars, t->nvars, sizeof(hdAttr),
            (compare_func_t)compare_variables);
  }
  else if (v->value != value)
  {
    if (v->value != NULL)
      free(v->value);
    if (value != NULL)
      v->value = (uchar *)strdup((char *)value);
    else
      v->value = NULL;
  }

  return (0);
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
    _htmlSizes[i]    = p;
    _htmlSpacings[i] = p * s;
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


  strlcpy(_htmlCharSet, cs, sizeof(_htmlCharSet));

  if (!_htmlInitialized)
  {
   /*
    * Load the PostScript glyph names for all of Unicode...
    */

    memset(_htmlGlyphsAll, 0, sizeof(_htmlGlyphsAll));

    snprintf(line, sizeof(line), "%s/data/psglyphs", _htmlData);
    if ((fp = fopen(line, "r")) != NULL)
    {
      while (fscanf(fp, "%x%63s", &unicode, glyph) == 2)
        _htmlGlyphsAll[unicode] = strdup(glyph);

      fclose(fp);

      _htmlInitialized = 1;
    }
#ifndef DEBUG
//    else
//      progress_error(HD_ERROR_FILE_NOT_FOUND,
//                     "Unable to open psglyphs data file!");
#endif /* !DEBUG */
  }

  memset(_htmlGlyphs, 0, sizeof(_htmlGlyphs));

  if (strncmp(cs, "8859-", 5) == 0)
    snprintf(filename, sizeof(filename), "%s/data/iso-%s", _htmlData, cs);
  else
    snprintf(filename, sizeof(filename), "%s/data/%s", _htmlData, cs);

  if ((fp = fopen(filename, "r")) == NULL)
  {
   /*
    * Can't open charset file; use ISO-8859-1...
    */

#ifndef DEBUG
//    progress_error(HD_ERROR_FILE_NOT_FOUND,
//                   "Unable to open character set file %s!", cs);
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
      _htmlGlyphs[i] = NULL;
    else
      _htmlGlyphs[i] = _htmlGlyphsAll[chars[i]];

 /*
  * Now read all of the font widths...
  */

  for (i = 0; i < 4; i ++)
    for (j = 0; j < 4; j ++)
    {
      for (ch = 0; ch < 256; ch ++)
        _htmlWidths[i][j][ch] = 0.6f;

      snprintf(filename, sizeof(filename), "%s/fonts/%s.afm", _htmlData,
               _htmlFonts[i][j]);
      if ((fp = fopen(filename, "r")) == NULL)
      {
#ifndef DEBUG
//        progress_error(HD_ERROR_FILE_NOT_FOUND,
//                       "Unable to open font width file %s!", filename);
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
	    if (_htmlGlyphs[ch] && strcmp(_htmlGlyphs[ch], glyph) == 0)
	      _htmlWidths[i][j][ch] = width * 0.001f;
	}
	else
	{
	 /*
	  * Symbol font uses its own encoding...
	  */

          if (sscanf(line, "%*s%d%*s%*s%f", &ch, &width) != 2)
	    continue;

          if (ch < 256 && ch >= 0)
	    _htmlWidths[i][j][ch] = width * 0.001f;
	}
      }

      fclose(fp);

      // Make sure that non-breaking space has the same width as
      // a breaking space...
      _htmlWidths[i][j][160] = _htmlWidths[i][j][32];
    }
}


/*
 * 'htmlSetTextColor()' - Set the default text color.
 */

void
htmlSetTextColor(uchar *color)	/* I - Text color */
{
  strlcpy((char *)_htmlTextColor, (char *)color, sizeof(_htmlTextColor));
}


/*
 * 'compare_variables()' - Compare two markup variables.
 */

static int			/* O - -1 if v0 < v1, 0 if v0 == v1, 1 if v0 > v1 */
compare_variables(hdAttr *v0,	/* I - First variable */
                  hdAttr *v1)	/* I - Second variable */
{
  return (strcasecmp((char *)v0->name, (char *)v1->name));
}


/*
 * 'compare_markups()' - Compare two markup strings...
 */

static int			/* O - -1 if m0 < m1, 0 if m0 == m1, 1 if m0 > m1 */
compare_markups(uchar **m0,	/* I - First markup */
                uchar **m1)	/* I - Second markup */
{
  if (tolower((*m0)[0]) == 'h' && isdigit((*m0)[1]) &&
      tolower((*m1)[0]) == 'h' && isdigit((*m1)[1]))
    return (atoi((char *)*m0 + 1) - atoi((char *)*m1 + 1));
  else
    return (strcasecmp((char *)*m0, (char *)*m1));
}


/*
 * 'delete_node()' - Free all memory associated with a node...
 */

static void
delete_node(hdTree *t)		/* I - Node to delete */
{
  int		i;		/* Looping var */
  hdAttr		*var;		/* Current variable */


  if (t == NULL)
    return;

  if (t->data != NULL)
    free(t->data);

  for (i = t->nvars, var = t->vars; i > 0; i --, var ++)
  {
    free(var->name);
    if (var->value != NULL)
      free(var->value);
  }

  if (t->vars != NULL)
    free(t->vars);

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
//    progress_error(HD_ERROR_OUT_OF_MEMORY,
//                   "Unable to allocate memory for HTML tree node!");
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
    space->typeface = _htmlBodyFont;
    space->size     = _htmlSizes[SIZE_P];
  }

  // Initialize element data...
  space->element = HD_ELEMENT_NONE;
  space->data   = (uchar *)strdup(" ");

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
// 'hdTree:get_element()' - Get the element constant corresponding to a string.
//

hdElement				// O - Element constant
hdTree::get_element(const char *n)	// I - Name of element
{
  const char	**temp;			// Element entry


  temp = (const char **)bsearch(&n, _htmlMarkups,
                                sizeof(_htmlMarkups) / sizeof(_htmlMarkups[0]),
                                sizeof(_htmlMarkups[0]),
                                (hdCompareFunc)compare_markups);

  if (temp == NULL)
    return (HD_ELEMENT_UNKNOWN);
  else
    return ((hdElement)(temp - _htmlMarkups));
}


/*
 * 'parse_markup()' - Parse a markup string.
 */

static int			/* O - -1 on error, HD_ELEMENT_nnnn otherwise */
parse_markup(hdTree *t,		/* I - Current tree entry */
             FILE   *fp,	/* I - Input file */
	     int    *linenum)	/* O - Current line number */
{
  int	ch, ch2;		/* Characters from file */
  uchar	markup[255],		/* Markup string... */
	*mptr,			/* Current character... */
	comment[10240],		/* Comment string */
	*cptr,			/* Current char... */
	**temp;			/* Markup variable entry */


  mptr = markup;

  while ((ch = getc(fp)) != EOF && mptr < (markup + sizeof(markup) - 1))
    if (ch == '>' || isspace(ch))
      break;
    else if (ch == '/' && mptr > markup)
    {
      // Look for "/>"...
      ch = getc(fp);

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
        ch = getc(fp);
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (HD_ELEMENT_ERROR);

  mptr = markup;
  temp = (uchar **)bsearch(&mptr, _htmlMarkups,
                           sizeof(_htmlMarkups) / sizeof(_htmlMarkups[0]),
                           sizeof(_htmlMarkups[0]),
                           (compare_func_t)compare_markups);

  if (temp == NULL)
  {
   /*
    * Unrecognized markup stuff...
    */

    t->element = HD_ELEMENT_UNKNOWN;
    strlcpy((char *)comment, (char *)markup, sizeof(comment));
    cptr = comment + strlen((char *)comment);

    DEBUG_printf(("%s%s (unrecognized!)\n", indent, markup));
  }
  else
  {
    t->element = (hdElement)((const char **)temp - _htmlMarkups);
    cptr      = comment;

    DEBUG_printf(("%s%s, line %d\n", indent, markup, *linenum));
  }

  if (t->element == HD_ELEMENT_COMMENT || t->element == HD_ELEMENT_UNKNOWN)
  {
    while (ch != EOF && cptr < (comment + sizeof(comment) - 1))
    {
      if (ch == '>' && temp == NULL)
        break;

      if (ch == '\n')
        (*linenum) ++;

      if (ch == '-')
      {
        *cptr++ = ch;

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
      {
        if (ch == '&')
	{
          // Handle character entities...
	  uchar	entity[16],		// Character entity name
		*eptr;			// Pointer into name


	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
		 (ch = getc(fp)) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = ch;

	  if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

	  *eptr = '\0';
	  if (!ch)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                   *linenum);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity,
	            sizeof(comment) - (cptr - comment));
	    cptr += strlen((char *)cptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
//	    progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                   entity, *linenum);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity,
	            sizeof(comment) - (cptr - comment));
	    cptr += strlen((char *)cptr);
            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = ';';
	  }
	  else if (cptr < (comment + sizeof(comment) - 1))
	    *cptr++ = ch;
	}
	else
	  *cptr++ = ch;

        ch = getc(fp);
      }
    }

    *cptr = '\0';
    t->data = (uchar *)strdup((char *)comment);
  }
  else
  {
    while (ch != EOF && ch != '>')
    {
      if (ch == '\n')
        (*linenum) ++;

      if (!isspace(ch))
      {
        ungetc(ch, fp);
        parse_variable(t, fp, linenum);
      }

      ch = getc(fp);

      if (ch == '/')
      {
	// Look for "/>"...
	ch = getc(fp);

	if (ch != '>')
          return (HD_ELEMENT_ERROR);

	break;
      }
    }
  }

  return (t->element);
}


/*
 * 'parse_variable()' - Parse a markup variable string.
 */

static int			/* O - -1 on error, 0 on success */
parse_variable(hdTree *t,	/* I - Current tree entry */
               FILE   *fp,	/* I - Input file */
	       int    *linenum)	/* I - Current line number */
{
  uchar	name[1024],			// Name of variable
	value[10240],			// Value of variable
	*ptr,				// Temporary pointer
	entity[16],			// Character entity name
	*eptr;				// Pointer into name
  int	ch;				// Character from file


  ptr = name;
  while ((ch = getc(fp)) != EOF)
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
    ch = getc(fp);

    if (ch == '\n')
      (*linenum) ++;
  }

  switch (ch)
  {
    default :
        ungetc(ch, fp);
        return (htmlSetVariable(t, name, NULL));
    case EOF :
        return (-1);
    case '=' :
        ptr = value;
        ch  = getc(fp);

        while (isspace(ch) || ch == '\r')
          ch = getc(fp);

        if (ch == EOF)
          return (-1);

        if (ch == '\'')
        {
          while ((ch = getc(fp)) != EOF)
	  {
            if (ch == '\'')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = getc(fp)) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                       entity, *linenum);

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
          while ((ch = getc(fp)) != EOF)
	  {
            if (ch == '\"')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = getc(fp)) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                       entity, *linenum);

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
        else
        {
          *ptr++ = ch;
          while ((ch = getc(fp)) != EOF)
	  {
            if (isspace(ch) || ch == '>' || ch == '\r')
              break;
	    else if (ch == '&')
	    {
	      // Possibly a character entity...
	      eptr = entity;
	      while (eptr < (entity + sizeof(entity) - 1) &&
	             (ch = getc(fp)) != EOF)
	        if (!isalnum(ch) && ch != '#')
		  break;
		else
		  *eptr++ = ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d.",
//	                       *linenum);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
//		progress_error(HD_ERROR_HTML_ERROR, "Unknown character entity \"&%s;\" on line %d.",
//	                       entity, *linenum);

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
              *ptr++ = ch;
	  }

	  if (ch == '\n')
	    (*linenum) ++;

          *ptr = '\0';
          if (ch == '>')
            ungetc(ch, fp);
        }

        return (htmlSetVariable(t, name, value));
  }
}


/*
 * 'compute_size()' - Compute the width and height of a tree entry.
 */

static int			/* O - 0 = success, -1 = failure */
compute_size(hdTree *t)		/* I - Tree entry */
{
  uchar		*ptr;		/* Current character */
  float		width,		/* Current width */
		max_width;	/* Maximum width */
  uchar		*width_ptr,	/* Pointer to width string */
		*height_ptr,	/* Pointer to height string */
		*size_ptr,	/* Pointer to size string */
		*type_ptr;	/* Pointer to spacer type string */
  image_t	*img;		/* Image */
  char		number[255];	/* Width or height value */


  if (!_htmlInitialized)
    htmlSetCharSet("8859-1");

  if (t->element == HD_ELEMENT_IMG)
  {
    width_ptr  = htmlGetVariable(t, (uchar *)"WIDTH");
    height_ptr = htmlGetVariable(t, (uchar *)"HEIGHT");

#if 0 // NEED TO FIX THIS
    img = image_load((char *)htmlGetVariable(t, (uchar *)"REALSRC"),
                     _htmlGrayscale);
#else
    img = NULL;
#endif // 0

    if (width_ptr != NULL && height_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlPPI * 72.0f;
      t->height = atoi((char *)height_ptr) / _htmlPPI * 72.0f;

      return (0);
    }

    if (img == NULL)
      return (-1);

    if (width_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlPPI * 72.0f;
      t->height = t->width * img->height / img->width;

      sprintf(number, "%d",
              atoi((char *)width_ptr) * img->height / img->width);
      if (strchr((char *)width_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetVariable(t, (uchar *)"HEIGHT", (uchar *)number);
    }
    else if (height_ptr != NULL)
    {
      t->height = atoi((char *)height_ptr) / _htmlPPI * 72.0f;
      t->width  = t->height * img->width / img->height;

      sprintf(number, "%d",
              atoi((char *)height_ptr) * img->width / img->height);
      if (strchr((char *)height_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetVariable(t, (uchar *)"WIDTH", (uchar *)number);
    }
    else
    {
      t->width  = img->width / _htmlPPI * 72.0f;
      t->height = img->height / _htmlPPI * 72.0f;

      sprintf(number, "%d", img->width);
      htmlSetVariable(t, (uchar *)"WIDTH", (uchar *)number);

      sprintf(number, "%d", img->height);
      htmlSetVariable(t, (uchar *)"HEIGHT", (uchar *)number);
    }

    return (0);
  }
  else if (t->element == HD_ELEMENT_SPACER)
  {
    width_ptr  = htmlGetVariable(t, (uchar *)"WIDTH");
    height_ptr = htmlGetVariable(t, (uchar *)"HEIGHT");
    size_ptr   = htmlGetVariable(t, (uchar *)"SIZE");
    type_ptr   = htmlGetVariable(t, (uchar *)"TYPE");

    if (width_ptr != NULL)
      t->width = atoi((char *)width_ptr) / _htmlPPI * 72.0f;
    else if (size_ptr != NULL)
      t->width = atoi((char *)size_ptr) / _htmlPPI * 72.0f;
    else
      t->width = 1.0f;

    if (height_ptr != NULL)
      t->height = atoi((char *)height_ptr) / _htmlPPI * 72.0f;
    else if (size_ptr != NULL)
      t->height = atoi((char *)size_ptr) / _htmlPPI * 72.0f;
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
  else if (t->element == HD_ELEMENT_BR)
  {
    t->width  = 0.0;
    t->height = t->size;

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
        width += _htmlWidths[t->typeface][t->style][(int)*ptr & 255];

   if (width < max_width)
     width = max_width;
  }
  else if (t->data)
    for (width = 0.0, ptr = t->data; *ptr != '\0'; ptr ++)
      width += _htmlWidths[t->typeface][t->style][(int)*ptr & 255];
  else
    width = 0.0f;

  t->width  = width * t->size;
  t->height = t->size;

  DEBUG_printf(("%swidth = %.1f, height = %.1f\n",
                indent, t->width, t->height));

  return (0);
}


/*
 * 'compute_color()' - Compute the red, green, blue color from the given
 *                     string.
 */

static int
compute_color(hdTree *t,	/* I - Tree entry */
              uchar  *color)	/* I - Color string */
{
  float	rgb[3];			/* RGB color */


//  get_color(color, rgb);

  t->red   = (uchar)(rgb[0] * 255.0f + 0.5f);
  t->green = (uchar)(rgb[1] * 255.0f + 0.5f);
  t->blue  = (uchar)(rgb[2] * 255.0f + 0.5f);

  return (0);
}


/*
 * 'get_alignment()' - Get horizontal & vertical alignment values.
 */

static int			/* O - 0 for success, -1 for failure */
get_alignment(hdTree *t)	/* I - Tree entry */
{
  uchar	*align;			/* Alignment string */


  if ((align = htmlGetVariable(t, (uchar *)"ALIGN")) == NULL)
    align = htmlGetStyle(t, (uchar *)"text-align");

  if (align != NULL)
  {
    if (strcasecmp((char *)align, "left") == 0)
      t->halignment = HD_TEXT_ALIGN_LEFT;
    else if (strcasecmp((char *)align, "center") == 0)
      t->halignment = HD_TEXT_ALIGN_CENTER;
    else if (strcasecmp((char *)align, "right") == 0)
      t->halignment = HD_TEXT_ALIGN_RIGHT;
    else if (strcasecmp((char *)align, "justify") == 0)
      t->halignment = HD_TEXT_ALIGN_JUSTIFY;
    else if (strcasecmp((char *)align, "top") == 0)
      t->valignment = HD_VERTICAL_ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      t->valignment = HD_VERTICAL_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
      t->valignment = HD_VERTICAL_ALIGN_BOTTOM;
  }

  if ((align = htmlGetVariable(t, (uchar *)"VALIGN")) == NULL)
    align = htmlGetStyle(t, (uchar *)"vertical-align");

  if (align != NULL)
  {
    if (strcasecmp((char *)align, "top") == 0)
      t->valignment = HD_VERTICAL_ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      t->valignment = HD_VERTICAL_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "center") == 0)
      t->valignment = HD_VERTICAL_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
      t->valignment = HD_VERTICAL_ALIGN_BOTTOM;
  }

  return (0);
}


/*
 * 'fix_filename()' - Fix a filename to be relative to the base directory.
 */

static const char *			/* O - Fixed filename */
fix_filename(char *filename,		/* I - Original filename */
             char *base)		/* I - Base directory */
{
  char		*slash;			/* Location of slash */
  static char	newfilename[1024];	/* New filename */


  if (filename == NULL)
    return (NULL);

//  if (strcmp(base, ".") == 0 || strstr(filename, "//") != NULL)
//    return (hdBook::file_find(Path, filename));

  if (strncmp(filename, "./", 2) == 0 ||
      strncmp(filename, ".\\", 2) == 0)
    filename += 2;

  if (strncmp(base, "http://", 7) == 0 || strncmp(base, "https://", 8) == 0)
  {
    strlcpy(newfilename, base, sizeof(newfilename));
    base = strchr(newfilename, ':') + 3;

    if (filename[0] == '/')
    {
      if ((slash = strchr(base, '/')) != NULL)
        strlcpy(slash, filename, sizeof(newfilename) - (slash - newfilename));
      else
        strlcat(newfilename, filename, sizeof(newfilename));

      return (newfilename);
    }
    else if ((slash = strchr(base, '/')) == NULL)
      strlcat(newfilename, "/", sizeof(newfilename));
  }
  else
  {
//    if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
//	base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
//      return (hdBook::file_find(Path, filename)); /* No change needed for absolute path */

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

  if (filename[0] != '/' && *base && base[strlen(base) - 1] != '/')
    strlcat(newfilename, "/", sizeof(newfilename));

  strlcat(newfilename, filename, sizeof(newfilename));

  return (newfilename);
//  return (hdBook::file_find(Path, newfilename));
}


//
// 'html_memory_used()' - Figure out the amount of memory that was used.
//

static int				// O - Bytes used
html_memory_used(hdTree *t)		// I - Tree node
{
  int	i;				// Looping var
  int	bytes;				// Bytes used


  if (t == NULL)
    return (0);

  bytes = 0;

  while (t != NULL)
  {
    bytes += sizeof(hdTree);
    bytes += t->nvars * sizeof(hdAttr);

    for (i = 0; i < t->nvars; i ++)
    {
      bytes += (strlen((char *)t->vars[i].name) + 8) & ~7;

      if (t->vars[i].value != NULL)
        bytes += (strlen((char *)t->vars[i].value) + 8) & ~7;
    }

    if (t->data != NULL)
      bytes += (strlen((char *)t->data) + 8) & ~7;

    bytes += html_memory_used(t->child);

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
  const char	*debug;			/* HTMLDOC_DEBUG env var */


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

//  progress_error(HD_ERROR_NONE, "DEBUG: %s = %d kbytes", title,
//                 (html_memory_used(t) + 1023) / 1024);
}


//
// 'htmlFindFile()' - Find a file in the document.
//

hdTree *				// O - Node for file
htmlFindFile(hdTree *doc,		// I - Document pointer
             uchar  *filename)		// I - Filename
{
  hdTree	*tree;			// Current node
  uchar		*treename;		// Filename from node


  if (!filename || !doc)
    return (NULL);

  for (tree = doc; tree; tree = tree->next)
    if ((treename = htmlGetVariable(tree, (uchar *)"_HD_FILENAME")) != NULL &&
        !strcmp((char *)treename, (char *)filename))
      return (tree);

  return (NULL);
}


//
// 'htmlFixLinks()' - Fix the external links in the document.
//

void
htmlFixLinks(hdTree *doc,		// I - Top node
             hdTree *tree,		// I - Current node
	     uchar  *base)		// I - Base directory/path
{
  uchar		*href;			// HREF attribute
  char		full_href[1024];	// Full HREF value
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
        (href = htmlGetVariable(tree, (uchar *)"HREF")) != NULL)
    {
      // Check if the link needs to be localized...
      if (href[0] != '#' && hdBook::file_method((char *)href) == NULL &&
          hdBook::file_method((char *)base) != NULL &&
	  htmlFindFile(doc, (uchar *)hdBook::file_basename((char *)href)) == NULL)
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

//        if (show_debug)
//          progress_error(HD_ERROR_NONE, "DEBUG: Mapping \"%s\" to \"%s\"...\n",
//	        	 href, full_href);

	htmlSetVariable(tree, (uchar *)"_HD_FULL_HREF", (uchar *)full_href);
      }
      else
      {
        // No, just mirror the link in the _HD_FULL_HREF attribute...
	htmlSetVariable(tree, (uchar *)"_HD_FULL_HREF", href);
      }
    }
    else if (tree->element == HD_ELEMENT_FILE)
      base = htmlGetVariable(tree, (uchar *)"_HD_BASE");

    if (tree->child)
      htmlFixLinks(doc, tree->child, base);

    tree = tree->next;
  }
}


/*
 * End of "$Id: tree.cxx,v 1.30 2004/10/25 14:30:03 mike Exp $".
 */
