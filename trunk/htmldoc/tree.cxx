//
// "$Id: tree.cxx,v 1.2 2002/02/05 17:47:59 mike Exp $"
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
#include <ctype.h>


//
// Class globals...
//

const char	*hdTree::elements[HD_ELEMENT_MAX] =
		{				// Element strings...
		  "",				// HD_ELEMENT_NONE
		  "",				// HD_ELEMENT_FILE
		  "",				// HD_ELEMENT_ERROR
		  "",				// HD_ELEMENT_UNKNOWN
		  "!--",			// HD_ELEMENT_COMMENT
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
		{				// Element group bits
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
			HD_ELGROUP_NONE		// WBR
		};


hdElement	hdTree::elparent[HD_ELEMENT_MAX] =
		{				// "Parent" element for inheritance
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
			HD_ELEMENT_NONE		// WBR
		};







#if 0
const char	*data_dir = HTML_DATA;	// Data directory
float		ppi = 80.0f;	// Image resolution
int		grayscale = 0;	// Grayscale output?
char		text_color[255] =	// Default text color
		{ 0 };
float		browser_width = 680.0f;
					// Browser width for pixel scaling
float		font_sizes[8] =		// Point size for each HTML size
		{ 6.0f, 8.0f, 9.0f, 11.0f, 14.0f, 17.0f, 20.0f, 24.0f };
float		line_spacings[8] =	// Line height for each HTML size
		{ 7.2f, 9.6f, 10.8f, 13.2f, 16.8f, 20.4f, 24.0f, 28.8f };
typeface_t	body_font = HD_FONTFACE_TIMES,
		heading_font = HD_FONTFACE_HELVETICA;

int		initialized = 0;	// Initialized glyphs yet?
char		font_charset[256] = "";	// Character set name
float		font_widths[4][4][256];	// Character widths of fonts
const char	*font_glyphs_unicode[65536];	// Character glyphs for Unicode
const char	*font_glyphs[256];	// Character glyphs for charset
const char	*fonts[16][4] =
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
static int	compare_variables(var_t *v0, var_t *v1);
static int	compare_elements(char **m0, char **m1);
static void	delete_node(hdTree *t);
static void	insert_space(hdTree *parent, hdTree *t);
static int	parse_element(hdTree *t, FILE *fp);
static int	parse_variable(hdTree *t, FILE *fp);
static int	compute_size(hdTree *t);
static int	compute_color(hdTree *t, char *color);
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
static char	indent[255] = "";
#endif /* DEBUG */


/*
 * 'htmlReadFile()' - Read a file for HTML element codes.
 */

hdTree *			/* O - Pointer to top of file tree */
htmlReadFile(hdTree     *parent,/* I - Parent tree entry */
             FILE       *fp,	/* I - File pointer */
	     const char *base)	/* I - Base directory for file */
{
  int		ch;		/* Character from file */
  char		*ptr,		/* Pointer in string */
		glyph[16],	/* Glyph name (&#nnn; or &name;) */
		*glyphptr;	/* Pointer in glyph string */
  hdTree	*tree,		/* "top" of this tree */
		*t,		/* New tree entry */
		*prev,		/* Previous tree entry */
		*temp;		/* Temporary looping var */
  int		pos;		/* Current file position */
  FILE		*embed;		/* File pointer for EMBED */
  char		newbase[1024];	/* New base directory for EMBED */
  char		*filename,	/* Filename for EMBED tag */
		*face,		/* Typeface for FONT tag */
		*color,		/* Color for FONT tag */
		*size;		/* Size for FONT tag */
  int		sizeval;	/* Size value from FONT tag */
  static char	s[10240];	/* String from file */
  static int	have_whitespace = 0;
  				/* Non-zero if there was leading whitespace */


#ifdef DEBUG
  strcat((char *)indent, "    ");
#endif /* DEBUG */

 /*
  * Start off with no previous tree entry...
  */

  prev = NULL;
  tree = NULL;

 /*
  * Parse data until we hit end-of-file...
  */

  while ((ch = getc(fp)) != EOF)
  {
   /*
    * Ignore leading whitespace...
    */

    if (parent == NULL || !parent->preformatted)
    {
      while (isspace(ch))
      {
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
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for HTML tree node!");
#endif /* !DEBUG */
      break;
    }

   /*
    * Set/copy font characteristics...
    */

    if (parent == NULL)
    {
      t->halignment   = HD_ALIGN_LEFT;
      t->valignment   = HD_ALIGN_MIDDLE;
      t->typeface     = body_font;
      t->size         = HD_FONTSIZE_P;

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

      pos = ftell(fp) - 1;

      ch = getc(fp);
      if (isspace(ch) || ch == '=' || ch == '<')
      {
       /*
        * Sigh...  "<" followed by anything but an element name is
	* invalid HTML, but so many people have asked for this to
	* be supported that we have added this hack...
	*/

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
	t->data   = (char *)strdup((char *)s);
      }
      else
      {
       /*
        * Start of a element...
	*/

	if (ch != '/')
          ungetc(ch, fp);

	if (parse_element(t, fp) == HD_ELEMENT_ERROR)
	{
  #ifndef DEBUG
          progress_error(HD_ERROR_READ_ERROR,
                         "Unable to parse HTML element at %d!", pos);
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
	* element, or if we've completed a list, we're done!
	*/

	if (ch == '/')
	{
	 /*
          * Close element; find matching element...
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
            if (islentry(temp->element) || isblock(temp->element))
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
            if (isblock(temp->element) || islentry(temp->element))
              break;
	    else if (istentry(temp->element) || islist(temp->element) ||
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

          delete_node(t);

	 /*
          * If the element doesn't start with a slash, or if it does but
          * doesn't match up with the parent (i.e. <UL><LI>...<LI>...</UL>)
          * then seek back so the parent entry gets a copy...
          */

          if (ch != '/' || temp != parent)
            fseek(fp, pos, SEEK_SET);	/* Make sure parent gets this element... */
          break;
	}
	else if (ch == '/')
	{
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
          for (glyphptr = glyph;
               (ch = getc(fp)) != EOF && (glyphptr - glyph) < 15;
               glyphptr ++)
            if (ch == ';' || isspace(ch))
              break;
            else
              *glyphptr = ch;

          *glyphptr = '\0';
          ch = iso8859(glyph);
        }

        if (ch != 0 && ch != '\r')
          *ptr++ = ch;

        if (ch == '\n')
          break;

        ch = getc(fp);
      }

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      t->element = HD_ELEMENT_NONE;
      t->data   = (char *)strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\"\n", indent, s));
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
          for (glyphptr = glyph;
               (ch = getc(fp)) != EOF && (glyphptr - glyph) < 15;
               glyphptr ++)
            if (ch == ';' || isspace(ch))
              break;
            else
              *glyphptr = ch;

          *glyphptr = '\0';
          ch = iso8859(glyph);
        }

        if (ch != 0)
          *ptr++ = ch;

        ch = getc(fp);
      }

      if (isspace(ch))
        have_whitespace = 1;

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      t->element = HD_ELEMENT_NONE;
      t->data   = (char *)strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\"\n", indent, s));
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
    else
      tree = t;

    prev = t;

   /*
    * Do element-specific stuff...
    */

    switch (t->element)
    {
      case HD_ELEMENT_BODY :
         /*
	  * Update the text color as necessary...
	  */

          if ((color = htmlGetVariable(t, (char *)"TEXT")) != NULL)
            compute_color(t, color);
	  else
            compute_color(t, text_color);

          if ((color = htmlGetVariable(t, (char *)"BGCOLOR")) != NULL &&
	      !BodyColor[0])
	    strcpy(BodyColor, (char *)color);

          // Update the background image as necessary...
          if ((filename = htmlGetVariable(t, (char *)"BACKGROUND")) != NULL)
	    htmlSetVariable(t, (char *)"BACKGROUND",
	                    (char *)fix_filename((char *)filename,
			                          (char *)base));

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_IMG :
          if (have_whitespace)
	  {
	    // Insert a space before this image...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          // Get the image alignment...
          t->valignment = HD_ALIGN_TOP;
          get_alignment(t);

          // Update the image source as necessary...
          if ((filename = htmlGetVariable(t, (char *)"SRC")) != NULL)
	    htmlSetVariable(t, (char *)"REALSRC",
	                    (char *)fix_filename((char *)filename,
			                          (char *)base));

      case HD_ELEMENT_BR :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
	 /*
	  * Figure out the width & height of this element...
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

          htmlReadFile(t, fp, base);
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

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_PRE :
          t->typeface      = HD_FONTFACE_COURIER;
          t->size          = HD_FONTSIZE_PRE;
          t->style         = HD_FONTSTYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 1;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_BLOCKQUOTE :
      case HD_ELEMENT_DIR :
      case HD_ELEMENT_MENU :
      case HD_ELEMENT_UL :
      case HD_ELEMENT_OL :
      case HD_ELEMENT_DL :
          t->indent ++;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_DIV :
          get_alignment(t);

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_HR :
          t->halignment = HD_ALIGN_CENTER;
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
          break;

      case HD_ELEMENT_EMBED :
          if ((filename = htmlGetVariable(t, (char *)"SRC")) != NULL)
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
#endif /* !DEBUG */
	  }
          break;

      case HD_ELEMENT_TH :
          if (htmlGetVariable(t->parent, (char *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = HD_ALIGN_CENTER;

          get_alignment(t);

          t->style = HD_FONTSTYLE_BOLD;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_TD :
          if (htmlGetVariable(t->parent, (char *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = HD_ALIGN_LEFT;

	  get_alignment(t);

          t->style = HD_FONTSTYLE_NORMAL;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_FONT :
          if ((face = htmlGetVariable(t, (char *)"FACE")) != NULL)
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

              if (have_whitespace)
	      {
		// Insert a space before monospaced text...
		insert_space(parent, t);

		have_whitespace = 0;
	      }
            }
	    else if (strstr((char *)face, "symbol") != NULL)
              t->typeface = HD_FONTFACE_SYMBOL;
          }

          if ((color = htmlGetVariable(t, (char *)"COLOR")) != NULL)
            compute_color(t, color);

          if ((size = htmlGetVariable(t, (char *)"SIZE")) != NULL)
          {
            if (have_whitespace)
	    {
	      // Insert a space before sized text...
	      insert_space(parent, t);

	      have_whitespace = 0;
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

      case HD_ELEMENT_BIG :
          if (have_whitespace)
	  {
	    // Insert a space before big text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          if (t->size < 6)
            t->size += 2;
          else
            t->size = 7;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_SMALL :
          if (have_whitespace)
	  {
	    // Insert a space before small text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          if (t->size > 2)
            t->size -= 2;
          else
            t->size = 0;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_SUP :
          if (have_whitespace)
	  {
	    // Insert a space before superscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->superscript = 1;

          if ((sizeval = t->size + HD_FONTSIZE_SUP) < 0)
	    t->size = 0;
	  else
	    t->size = sizeval;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_SUB :
          if (have_whitespace)
	  {
	    // Insert a space before subscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->subscript = 1;

          if ((sizeval = t->size + HD_FONTSIZE_SUB) < 0)
	    t->size = 0;
	  else
	    t->size = sizeval;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_KBD :
          t->style    = HD_FONTSTYLE_BOLD;

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

          t->typeface = HD_FONTFACE_COURIER;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_B :
          t->style = (style_t)(t->style | HD_FONTSTYLE_BOLD);

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_DD :
          t->indent ++;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_VAR :
          t->style = (style_t)(t->style | HD_FONTSTYLE_ITALIC);
      case HD_ELEMENT_DFN :
          t->typeface = HD_FONTFACE_HELVETICA;

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_STRONG :
          t->style = (style_t)(t->style | HD_FONTSTYLE_BOLD);
      case HD_ELEMENT_CITE :
      case HD_ELEMENT_DT :
      case HD_ELEMENT_EM :
      case HD_ELEMENT_I :
          t->style = (style_t)(t->style | HD_FONTSTYLE_ITALIC);

          htmlReadFile(t, fp, base);
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

          htmlReadFile(t, fp, base);
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

          htmlReadFile(t, fp, base);
          break;

      case HD_ELEMENT_CENTER :
          t->halignment = HD_ALIGN_CENTER;

          htmlReadFile(t, fp, base);
          break;

      default :
         /*
          * All other element types should be using <MARK>...</MARK>
          */

          get_alignment(t);

          htmlReadFile(t, fp, base);
          break;
    }
  }  

#ifdef DEBUG
  indent[strlen((char *)indent) - 4] = '\0';
#endif /* DEBUG */

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
  char	*ptr;			/* Character pointer */


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

      col += fprintf(fp, "<%s", elements[t->element]);
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
	
        col += fprintf(fp, "</%s>", elements[t->element]);
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
 * 'htmlWriteFile()' - Write an HTML element tree to a file.
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
            hdElement element,	/* I - Markup code */
            char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


  if ((t = htmlNewTree(parent, element, data)) == NULL)
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
               hdElement element,	/* I - Markup code */
               char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


  if ((t = htmlNewTree(parent, element, data)) == NULL)
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
            hdElement element,	/* I - Markup code */
            char    *data)	/* I - Data/text */
{
  hdTree	*t;		/* New tree entry */


 /*
  * Allocate a new tree entry - use calloc() to get zeroed data...
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
  char	*tname,			/* Name value from tree entry */
	*tcontent;		/* Content value from tree entry */


  while (tree != NULL)
  {
   /*
    * Check this tree entry...
    */

    if (tree->element == HD_ELEMENT_META &&
        (tname = htmlGetVariable(tree, (char *)"NAME")) != NULL &&
        (tcontent = htmlGetVariable(tree, (char *)"CONTENT")) != NULL)
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

char *				// O - Value or NULL
htmlGetStyle(hdTree *t,		// I - Node
             char  *name)	// I - Name (including ":")
{
  char		*ptr,		// Pointer in STYLE attribute
		*bufptr;	// Pointer in buffer
  int		ptrlen,		// Length of STYLE attribute
		namelen;	// Length of name
  static char	buffer[1024];	// Buffer for value


  // See if we have a STYLE attribute...
  if ((ptr = htmlGetVariable(t, (char *)"STYLE")) == NULL)
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
 * 'htmlGetVariable()' - Get a variable value from a element entry.
 */

char *				/* O - Value or NULL if variable does not exist */
htmlGetVariable(hdTree *t,	/* I - Tree entry */
                char  *name)	/* I - Variable name */
{
  var_t	*v,			/* Matching variable */
	key;			/* Search key */


  if (t == NULL || name == NULL || t->nvars == 0)
    return (NULL);

  key.name = name;

  v = (var_t *)bsearch(&key, t->vars, t->nvars, sizeof(var_t),
                       (compare_func_t)compare_variables);
  if (v == NULL)
    return (NULL);
  else if (v->value == NULL)
    return ((char *)"");
  else
    return (v->value);
}


/*
 * 'htmlSetVariable()' - Set a variable for a element entry.
 */

int				/* O - Set status: 0 = success, -1 = fail */
htmlSetVariable(hdTree *t,	/* I - Tree entry */
                char  *name,	/* I - Variable name */
                char  *value)	/* I - Variable value */
{
  var_t	*v,			/* Matching variable */
	key;			/* Search key */


  DEBUG_printf(("%shtmlSetVariable(%08x, \"%s\", \"%s\")\n", indent, t, name,
                value ? (const char *)value : "(null)"));

  if (t->nvars == 0)
    v = NULL;
  else
  {
    key.name = name;

    v = (var_t *)bsearch(&key, t->vars, t->nvars, sizeof(var_t),
        	         (compare_func_t)compare_variables);
  }

  if (v == NULL)
  {
    if (t->nvars == 0)
      v = (var_t *)malloc(sizeof(var_t));
    else
      v = (var_t *)realloc(t->vars, sizeof(var_t) * (t->nvars + 1));

    if (v == NULL)
    {
      DEBUG_printf(("%s==== MALLOC/REALLOC FAILED! ====\n", indent));

      return (-1);
    }

    t->vars  = v;
    v        += t->nvars;
    t->nvars ++;
    v->name  = (char *)strdup((char *)name);
    if (value != NULL)
      v->value = (char *)strdup((char *)value);
    else
      v->value = NULL;

    if (strcasecmp((char *)name, "HREF") == 0)
    {
      DEBUG_printf(("%s---- Set link to %s ----\n", indent, value));
      t->link = t;
    }

    if (t->nvars > 1)
      qsort(t->vars, t->nvars, sizeof(var_t),
            (compare_func_t)compare_variables);
  }
  else if (v->value != value)
  {
    if (v->value != NULL)
      free(v->value);
    if (value != NULL)
      v->value = (char *)strdup((char *)value);
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
 * 'compare_variables()' - Compare two element variables.
 */

static int			/* O - -1 if v0 < v1, 0 if v0 == v1, 1 if v0 > v1 */
compare_variables(var_t *v0,	/* I - First variable */
                  var_t *v1)	/* I - Second variable */
{
  return (strcasecmp((char *)v0->name, (char *)v1->name));
}


/*
 * 'compare_elements()' - Compare two element strings...
 */

static int			/* O - -1 if m0 < m1, 0 if m0 == m1, 1 if m0 > m1 */
compare_elements(char **m0,	/* I - First element */
                char **m1)	/* I - Second element */
{
  return (strcasecmp((char *)*m0, (char *)*m1));
}


/*
 * 'delete_node()' - Free all memory associated with a node...
 */

static void
delete_node(hdTree *t)		/* I - Node to delete */
{
  int		i;		/* Looping var */
  var_t		*var;		/* Current variable */


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
parse_element(hdTree *t,		/* I - Current tree entry */
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
parse_variable(hdTree *t,	/* I - Current tree entry */
               FILE   *fp)	/* I - Input file */
{
  char	name[1024],		/* Name of variable */
	value[10240],		/* Value of variable */
	*ptr;			/* Temporary pointer */
  int	ch;			/* Character from file */


  ptr = name;
  while ((ch = getc(fp)) != EOF)
    if (isspace(ch) || ch == '=' || ch == '>' || ch == '\r')
      break;
    else if (ptr < (name + sizeof(name) - 1))
      *ptr++ = ch;

  *ptr = '\0';

  while (isspace(ch) || ch == '\r')
    ch = getc(fp);

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
            if (ch == '\'')
              break;
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
              *ptr++ = ch;

          *ptr = '\0';
        }
        else if (ch == '\"')
        {
          while ((ch = getc(fp)) != EOF)
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
          while ((ch = getc(fp)) != EOF)
            if (isspace(ch) || ch == '>' || ch == '\r')
              break;
            else if (ptr < (value + sizeof(value) - 1))
              *ptr++ = ch;

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
    width_ptr  = htmlGetVariable(t, (char *)"WIDTH");
    height_ptr = htmlGetVariable(t, (char *)"HEIGHT");

    img = image_load((char *)htmlGetVariable(t, (char *)"REALSRC"),
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
      htmlSetVariable(t, (char *)"HEIGHT", (char *)number);
    }
    else if (height_ptr != NULL)
    {
      t->height = atoi((char *)height_ptr) / ppi * 72.0f;
      t->width  = t->height * img->width / img->height;

      sprintf(number, "%d",
              atoi((char *)height_ptr) * img->width / img->height);
      if (strchr((char *)height_ptr, '%') != NULL)
        strcat(number, "%");
      htmlSetVariable(t, (char *)"WIDTH", (char *)number);
    }
    else
    {
      t->width  = img->width / ppi * 72.0f;
      t->height = img->height / ppi * 72.0f;

      sprintf(number, "%d", img->width);
      htmlSetVariable(t, (char *)"WIDTH", (char *)number);

      sprintf(number, "%d", img->height);
      htmlSetVariable(t, (char *)"HEIGHT", (char *)number);
    }

    return (0);
  }
  else if (t->element == HD_ELEMENT_SPACER)
  {
    width_ptr  = htmlGetVariable(t, (char *)"WIDTH");
    height_ptr = htmlGetVariable(t, (char *)"HEIGHT");
    size_ptr   = htmlGetVariable(t, (char *)"SIZE");
    type_ptr   = htmlGetVariable(t, (char *)"TYPE");

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

    if (strcasecmp((char *)type_ptr, "horizontal") == 0)
      t->height = 0.0;
    else if (strcasecmp((char *)type_ptr, "vertical") == 0)
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
 * 'get_alignment()' - Get horizontal & vertical alignment values.
 */

static int			/* O - 0 for success, -1 for failure */
get_alignment(hdTree *t)	/* I - Tree entry */
{
  char	*align;			/* Alignment string */


  if ((align = htmlGetVariable(t, (char *)"ALIGN")) == NULL)
    align = htmlGetStyle(t, (char *)"text-align");

  if (align != NULL)
  {
    if (strcasecmp((char *)align, "left") == 0)
      t->halignment = HD_ALIGN_LEFT;
    else if (strcasecmp((char *)align, "center") == 0)
      t->halignment = HD_ALIGN_CENTER;
    else if (strcasecmp((char *)align, "right") == 0)
      t->halignment = HD_ALIGN_RIGHT;
    else if (strcasecmp((char *)align, "justify") == 0)
      t->halignment = HD_ALIGN_JUSTIFY;
    else if (strcasecmp((char *)align, "top") == 0)
      t->valignment = HD_ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
      t->valignment = HD_ALIGN_BOTTOM;
  }

  if ((align = htmlGetVariable(t, (char *)"VALIGN")) == NULL)
    align = htmlGetStyle(t, (char *)"vertical-align");

  if (align != NULL)
  {
    if (strcasecmp((char *)align, "top") == 0)
      t->valignment = HD_ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "center") == 0)
      t->valignment = HD_ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
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
// End of "$Id: tree.cxx,v 1.2 2002/02/05 17:47:59 mike Exp $".
//
