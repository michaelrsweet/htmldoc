//
// "$Id: htmllib.cxx,v 1.43 2000/11/06 19:53:04 mike Exp $"
//
//   HTML parsing routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
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

#include "htmldoc.h"
#include <ctype.h>


//
// Markup strings and other global data...
//

const char	*HDtree::markups[] =
		{
		  "",		// MARKUP_NONE
		  "!--",	// MARKUP_COMMENT
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

const char	*HDtree::datadir = HTML_DATA;	// Data directory
float		HDtree::ppi = 80.0f;		// Image resolution
int		HDtree::grayscale = 0;		// Grayscale output?
uchar		HDtree::text_color[255] =	// Default text color
		{ 0 };
uchar		HDtree::body_color[255] =	// Default body color
		{ 0 };
float		HDtree::sizes[8] =		// Point size for each HTML size
		{ 6.0f, 8.0f, 9.0f, 11.0f, 14.0f, 17.0f, 20.0f, 24.0f };
float		HDtree::spacings[8] =		// Line height for each HTML size
		{ 7.2f, 9.6f, 10.8f, 13.2f, 16.8f, 20.4f, 24.0f, 28.8f };
HDtypeface	HDtree::body_font = TYPE_TIMES,	// Default typefaces
		HDtree::heading_font = TYPE_HELVETICA;

int		HDtree::initialized = 0;	// Initialized glyphs yet?
char		HDtree::char_set[256] = "";	// Character set name
float		HDtree::widths[4][4][256];	// Character widths of fonts
const char	*HDtree::glyphs_all[65536];	// Character glyphs for Unicode
const char	*HDtree::glyphs[256];		// Character glyphs for charset
const char	*HDtree::fonts[4][4] =		// Font names
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


//
// Macros for identifying classes of markups...
//

#define issuper(x)	((x) == MARKUP_CENTER || (x) == MARKUP_DIV ||\
			 (x) == MARKUP_BLOCKQUOTE)
#define isblock(x)	((x) == MARKUP_ADDRESS || \
			 (x) == MARKUP_P || (x) == MARKUP_PRE ||\
			 ((x) >= MARKUP_H1 && (x) <= MARKUP_H6) ||\
			 (x) == MARKUP_HR || (x) == MARKUP_TABLE)
#define islist(x)	((x) == MARKUP_DL || (x) == MARKUP_OL ||\
			 (x) == MARKUP_UL || (x) == MARKUP_DIR ||\
			 (x) == MARKUP_MENU)
#define islentry(x)	((x) == MARKUP_LI || (x) == MARKUP_DD ||\
			 (x) == MARKUP_DT)
#define istable(x)	((x) == MARKUP_TBODY || (x) == MARKUP_THEAD ||\
			 (x) == MARKUP_TFOOT || (x) == MARKUP_TR)
#define istentry(x)	((x) == MARKUP_TD || (x) == MARKUP_TH)


//
// 'HDtree::init()' - Initialize attributes from the parent node.
//

void
HDtree::init(HDtree *p)			// I - Parent node
{
  // Initialize pointers and data...
  parent     = (HDtree *)0;
  child      = (HDtree *)0;
  last_child = (HDtree *)0;
  prev       = (HDtree *)0;
  next       = (HDtree *)0;
  markup     = MARKUP_NONE;
  data       = (uchar *)0;
  width      = 0.0f;
  height     = 0.0f;
  nvars      = 0;
  vars       = (HDvar *)0;

  // Set/copy font characteristics...
  if (p)
  {
    link          = p->link;

    halignment    = p->halignment;
    valignment    = p->valignment;
    typeface      = p->typeface;
    size          = p->size;
    style         = p->style;
    underline     = p->underline;
    strikethrough = p->strikethrough;
    subscript     = p->subscript;
    superscript   = p->superscript;
    preformatted  = p->preformatted;
    indent        = p->indent;

    red           = p->red;
    green         = p->green;
    blue          = p->blue;
  }
  else
  {
    link          = (HDtree *)0;

    halignment    = ALIGN_LEFT;
    valignment    = ALIGN_MIDDLE;
    typeface      = body_font;
    size          = SIZE_P;
    style         = STYLE_NORMAL;
    underline     = 0;
    strikethrough = 0;
    superscript   = 0;
    subscript     = 0;
    preformatted  = 0;
    indent        = 0;

    set_color(text_color);
  }
}


//
// 'HDtree::HDtree()' - Add a new tree entry to the parent.
//

HDtree::HDtree(HDtree   *p,	// I - Parent node
               HDmarkup m,	// I - Markup
	       uchar    *d,	// I - Data for markup
	       int      ins)	// I - Insert or add?
{
  // Copy data from the parent...
  init(p);

  // Set the markup and data...
  markup = m;

  if (d)
    data = (uchar *)strdup((char *)d);

  // Add this node to the parent...
  if (ins)
    insert(p);
  else
    add(p);

  // Set default values for node attributes based on the markup...
  switch (markup)
  {
    case MARKUP_NONE :
    case MARKUP_IMG :
        // Figure out the width & height of this fragment...
        get_size();
	break;

    case MARKUP_H1 :
    case MARKUP_H2 :
    case MARKUP_H3 :
    case MARKUP_H4 :
    case MARKUP_H5 :
    case MARKUP_H6 :
        typeface      = heading_font;
        size          = SIZE_H1 - markup + MARKUP_H1;
        subscript     = 0;
        superscript   = 0;
        strikethrough = 0;
        preformatted  = 0;
        style         = STYLE_BOLD;
        break;

    case MARKUP_P :
        typeface      = body_font;
        size          = SIZE_P;
        style         = STYLE_NORMAL;
        subscript     = 0;
        superscript   = 0;
        strikethrough = 0;
        preformatted  = 0;
        break;

    case MARKUP_PRE :
        typeface      = TYPE_COURIER;
        size          = SIZE_PRE;
        style         = STYLE_NORMAL;
        subscript     = 0;
        superscript   = 0;
        strikethrough = 0;
        preformatted  = 1;
        break;

    case MARKUP_BLOCKQUOTE :
        style = STYLE_ITALIC;

    case MARKUP_UL :
    case MARKUP_DIR :
    case MARKUP_MENU :
    case MARKUP_OL :
    case MARKUP_DL :
        indent ++;
        break;

    case MARKUP_TH :
        style      = STYLE_BOLD;
	halignment = ALIGN_CENTER;
        break;

    case MARKUP_SUP :
        superscript = 1;
        size        = SIZE_SUP;
        break;

    case MARKUP_SUB :
        subscript = 1;
        size      = SIZE_SUB;
        break;

    case MARKUP_B :
        style |= STYLE_BOLD;
        break;

    case MARKUP_DD :
        indent ++;
        break;

    case MARKUP_DT :
    case MARKUP_I :
        style |= STYLE_ITALIC;
        break;

    case MARKUP_U :
    case MARKUP_INS :
        underline = 1;
        break;

    case MARKUP_STRIKE :
    case MARKUP_DEL :
        strikethrough = 1;
        break;
  }
}


//
// 'HDtree:add()' - Add an existing tree node to the parent.
//

void
HDtree::add(HDtree *p)		// I - Parent node
{
  // Add the tree entry to the end of the chain of children...
  if (p)
  {
    if (p->last_child)
    {
      p->last_child->next = this;
      prev                = p->last_child;
    }
    else
      p->child = this;

    p->last_child = this;
  }
}


//
// 'HDtree::~HDtree()' - Delete a tree node and its children...
//

HDtree::~HDtree()
{
  int		i;		// Looping var
  HDvar		*var;		// Current variable
  HDtree	*t,		// Current tree entry
		*n;		// Next tree entry


  if (!is_copy)
  {
    // Free data and variables (attributes)...
    if (data)
      free(data);

    for (i = nvars, var = vars; i > 0; i --, var ++)
    {
      free(var->name);
      if (var->value != NULL)
	free(var->value);
    }

    if (vars)
      free(vars);

    // Delete child nodes...
    for (t = child; t; t = n)
    {
      n = t->next;
      delete t;
    }

    // Remove this node from the list and parent, if this node has a parent...
    if (prev)
      prev->next = next;
    else if (parent)
      parent->child = next;

    if (next)
      next->prev = prev;
    else if (parent)
      parent->last_child = prev;
  }
}


//
// 'HDtree:insert()' - Insert an existing tree node to the parent.
//

void
HDtree::insert(HDtree *p)	// I - Parent entry
{
  // Insert the node at the beginning of the chain of children...
  if (p)
  {
    if (p->child)
    {
      p->child->prev = this;
      next           = p->child;
    }
    else
      p->last_child = this;

    p->child = this;
  }
}


//
// 'HDtree:read()' - Read a HTML file.
//

void
HDtree::read(FILE       *fp,	// I - File pointer
             const char *base)	// I - Base directory for file
{
  int		ch;		// Character from file
  uchar		*ptr,		// Pointer in string
		glyph[16],	// Glyph name (&#nnn; or &name;)
		*glyphptr;	// Pointer in glyph string
  HDtree	*t,		// New tree entry
		*temp;		// Temporary looping var
  int		pos;		// Current file position
  FILE		*embed;		// File pointer for EMBED
  char		newbase[1024];	// New base directory for EMBED
  uchar		*filename,	// Filename for EMBED tag
		*value;		// Typeface for FONT tag
  int		sizeval;	// Size value from FONT tag
  static uchar	s[10240];	// String from file
  static int	have_whitespace = 0;
  				// Non-zero if there was leading whitespace


  // Parse data until we hit end-of-file...
  while ((ch = getc(fp)) != EOF)
  {
    // Ignore leading whitespace...
    if (!parent || !parent->preformatted)
    {
      while (isspace(ch))
      {
        have_whitespace = 1;
        ch              = getc(fp);
      }

      if (ch == EOF)
        break;
    }

    t = new HDtree(this);

    // See what the character was...
    if (ch == '<')
    {
      // Markup char; grab the next char to see if this is a /...
      pos = ftell(fp) - 1;

      ch = getc(fp);
      if (ch == ' ')
      {
        // Illegal lone "<"!  Ignore it...
	delete t;
	continue;
      }
      
      if (ch != '/')
        ungetc(ch, fp);

      if (t->parse_markup(fp) == MARKUP_ERROR)
      {
        HTMLDOC::progress->error("Unable to parse HTML element at %d!", pos);
        delete t;
        break;
      }

      // Eliminate extra whitespace...
      if (issuper(t->markup) || isblock(t->markup) ||
          islist(t->markup) || islentry(t->markup) ||
          istable(t->markup) || istentry(t->markup))
        have_whitespace = 0;

      // If this is the matching close mark, or if we are starting the same
      // markup, or if we've completed a list, we're done!
      if (ch == '/')
      {
        // Close markup; find matching markup...
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (temp->markup == markup)
            break;
	  else if (temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
            break;
	  }
      }
      else if (t->markup == MARKUP_EMBED)
      {
        // Close any text blocks...
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (isblock(temp->markup) || islentry(temp->markup))
            break;
	  else if (istentry(temp->markup) || islist(temp->markup) ||
	           issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
	    break;
	  }
      }
      else if (issuper(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (issuper(temp->markup))
            break;
	  else if (istentry(temp->markup) || temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
            break;
	  }
      }
      else if (islist(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (isblock(temp->markup))
	    break;
	  else if (islentry(temp->markup) || istentry(temp->markup) ||
	           issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
            break;
	  }

      }
      else if (islentry(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (islentry(temp->markup) || isblock(temp->markup))
            break;
	  else if (islist(temp->markup) || issuper(temp->markup) ||
	           istentry(temp->markup) || temp->markup == MARKUP_EMBED)
          {
	    temp = NULL;
	    break;
	  }
      }
      else if (isblock(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (isblock(temp->markup) || islentry(temp->markup))
            break;
	  else if (istentry(temp->markup) || islist(temp->markup) ||
	           issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
	    break;
	  }
      }
      else if (istable(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (istable(temp->markup))
	    break;
	  else if (temp->markup == MARKUP_TABLE || temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
            break;
	  }
      }
      else if (istentry(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (istentry(temp->markup))
            break;
	  else if (temp->markup == MARKUP_TABLE || istable(temp->markup) ||
	           temp->markup == MARKUP_EMBED)
	  {
	    temp = NULL;
            break;
	  }
      }
      else
        temp = NULL;

      if (temp != NULL)
      {
        // If the markup doesn't start with a slash, or if it does but
        // doesn't match up with the parent (i.e. <UL><LI>...<LI>...</UL>)
        // then seek back so the parent entry gets a copy...
        delete t;

        if (ch != '/' || temp != parent)
          fseek(fp, pos, SEEK_SET);	// Make sure parent gets this markup...
        break;
      }
      else if (ch == '/')
      {
        delete t;
	continue;
      }
    }
    else if (t->preformatted)
    {
      // Read a pre-formatted string into the current tree entry...
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

        if (ch != 0)
          *ptr++ = ch;

        if (ch == '\n')
          break;

        ch = getc(fp);
      }

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      markup = MARKUP_NONE;
      data   = (uchar *)strdup((char *)s);
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
        *ptr++ = ' ';

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      markup = MARKUP_NONE;
      data   = (uchar *)strdup((char *)s);
    }

    // Add the new node to this node...
    t->add(this);

    // Do markup-specific stuff...
    switch (t->markup)
    {
      case MARKUP_BODY :
          // Update the text color as necessary...
          if ((value = t->var((uchar *)"TEXT")) != NULL)
            t->set_color(value);
	  else
            t->set_color(text_color);

          if ((value = t->var((uchar *)"BGCOLOR")) != NULL &&
	      !body_color[0])
	  {
	    strncpy((char *)body_color, (char *)value, sizeof(body_color) - 1);
	    body_color[sizeof(body_color) - 1] = '\0';
	  }
          break;

      case MARKUP_IMG :
          // Update the image source as necessary...
          if ((filename = var((uchar *)"SRC")) != NULL)
	    t->var((uchar *)"SRC",
	           (uchar *)fix_filename((char *)filename, base));

      case MARKUP_BR :
      case MARKUP_NONE :
      case MARKUP_SPACER :
	  // Figure out the width & height of this markup...
          t->get_size();
	  break;

      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
          t->get_alignment();

          t->typeface      = heading_font;
          t->size          = SIZE_H1 - markup + MARKUP_H1;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;
          t->style         = STYLE_BOLD;

          t->read(fp, base);
          break;

      case MARKUP_P :
          t->get_alignment();

          t->typeface      = body_font;
          t->size          = SIZE_P;
          t->style         = STYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;

          t->read(fp, base);
          break;

      case MARKUP_PRE :
          t->typeface      = TYPE_COURIER;
          t->size          = SIZE_PRE;
          t->style         = STYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 1;

          t->read(fp, base);
          break;

      case MARKUP_BLOCKQUOTE :
      case MARKUP_DIR :
      case MARKUP_MENU :
      case MARKUP_UL :
      case MARKUP_OL :
      case MARKUP_DL :
          t->indent ++;

          t->read(fp, base);
          break;

      case MARKUP_DIV :
          t->get_alignment();

          t->read(fp, base);
          break;

      case MARKUP_HR :
          halignment = ALIGN_CENTER;
          t->get_alignment();
          break;

      case MARKUP_DOCTYPE :
      case MARKUP_AREA :
      case MARKUP_COMMENT :
      case MARKUP_INPUT :
      case MARKUP_ISINDEX :
      case MARKUP_LINK :
      case MARKUP_META :
      case MARKUP_WBR :
          break;

      case MARKUP_EMBED :
          if ((filename = t->var((uchar *)"SRC")) != NULL)
	  {
	    filename = (uchar *)fix_filename((char *)filename, base);

            if ((embed = fopen((char *)filename, "r")) != NULL)
            {
	      strcpy(newbase, file_directory((char *)filename));

              t->read(embed, newbase);
              fclose(embed);
            }
	    else
	      HTMLDOC::progress->error("Unable to embed \"%s\" - %s", filename,
	                     strerror(errno));
	  }
          break;

      case MARKUP_TH :
          t->halignment = ALIGN_CENTER;
          t->style      = STYLE_BOLD;
          t->get_alignment();

          t->read(fp, base);
          break;

      case MARKUP_TD :
          t->halignment = ALIGN_LEFT;
          t->style      = STYLE_NORMAL;
	  t->get_alignment();

          t->read(fp, base);
          break;

      case MARKUP_FONT :
          if ((value = t->var((uchar *)"FACE")) != NULL)
          {
            for (ptr = value; *ptr != '\0'; ptr ++)
              *ptr = tolower(*ptr);

            if (strstr((char *)value, "arial") != NULL ||
                strstr((char *)value, "helvetica") != NULL)
              t->typeface = TYPE_HELVETICA;
            else if (strstr((char *)value, "times") != NULL)
              t->typeface = TYPE_TIMES;
            else if (strstr((char *)value, "courier") != NULL)
              t->typeface = TYPE_COURIER;
            else if (strstr((char *)value, "symbol") != NULL)
              t->typeface = TYPE_SYMBOL;
          }

          if ((value = var((uchar *)"COLOR")) != NULL)
            t->set_color(value);

          if ((value = var((uchar *)"SIZE")) != NULL)
          {
	    if (isdigit(value[0]))
	      sizeval = atoi((char *)value);
	    else
              sizeval = t->size + atoi((char *)value);

            if (sizeval < 0)
              t->size = 0;
            else if (sizeval > 7)
              t->size = 7;
            else
              t->size = sizeval;
          }

          t->read(fp, base);
          break;

      case MARKUP_BIG :
          if (t->size < 6)
            t->size += 2;
          else
            t->size = 7;

          t->read(fp, base);
          break;

      case MARKUP_SMALL :
          if (t->size > 2)
            t->size -= 2;
          else
            t->size = 0;

          t->read(fp, base);
          break;

      case MARKUP_SUP :
          t->superscript = 1;
          t->size        = SIZE_SUP;
          t->read(fp, base);
          break;

      case MARKUP_SUB :
          t->subscript = 1;
          t->size      = SIZE_SUB;
          t->read(fp, base);
          break;

      case MARKUP_KBD :
          t->style = STYLE_BOLD;

      case MARKUP_TT :
      case MARKUP_CODE :
      case MARKUP_SAMP :
          t->typeface = TYPE_COURIER;
          t->read(fp, base);
          break;

      case MARKUP_B :
          t->style |= STYLE_BOLD;
          t->read(fp, base);
          break;

      case MARKUP_DD :
          t->indent ++;
          t->read(fp, base);
          break;

      case MARKUP_VAR :
          t->style |= STYLE_ITALIC;
      case MARKUP_DFN :
          t->typeface = TYPE_HELVETICA;
          t->read(fp, base);
          break;

      case MARKUP_STRONG :
          t->style |= STYLE_BOLD;
      case MARKUP_CITE :
      case MARKUP_DT :
      case MARKUP_EM :
      case MARKUP_I :
          t->style |= STYLE_ITALIC;
          t->read(fp, base);
          break;

      case MARKUP_U :
      case MARKUP_INS :
          t->underline = 1;
          t->read(fp, base);
          break;

      case MARKUP_STRIKE :
      case MARKUP_S :
      case MARKUP_DEL :
          t->strikethrough = 1;
          t->read(fp, base);
          break;

      case MARKUP_CENTER :
          t->halignment = ALIGN_CENTER;
          t->read(fp, base);
          break;

      default :
          // All other markup types should be using <MARK>...</MARK>
          t->get_alignment();

          t->read(fp, base);
          break;
    }
  }  
}


//
// 'HDtree::get_text()' - Get all text from the tree (private).
//

void
HDtree::get_text(uchar *buf,	// I - Buffer to store text in
                 int   buflen)	// I - Length of buffer
{
  HDtree	*t;		// Current tree node
  int		len;		// Length of text


  for (t = this; t && buflen > 1; t = t->next)
  {
    if (t->child != NULL)
      t->child->get_text(buf, buflen);
    else if (t->markup == MARKUP_NONE && t->data != NULL)
    {
      strncpy((char *)buf, (char *)t->data, buflen - 1);
      buf[buflen - 1] = '\0';
    }
    else if (t->markup == MARKUP_BR)
      strcat((char *)buf, " ");

    len    = strlen((char *)buf);
    buflen -= len;
    buf    += len;
  }
}


//
// 'HDtree::get_text()' - Get all text from the tree.
//

uchar *				// O - String containing text nodes
HDtree::get_text()
{
  static uchar	buf[10240];	// String buffer


  buf[0] = '\0';
  get_text(buf, sizeof(buf));

  return ((uchar *)strdup((char *)buf));
}


//
// 'HDtree::get_meta()' - Get document "meta" data...
//

uchar *				// O - Content string
HDtree::get_meta(uchar  *name)	// I - Metadata name
{
  HDtree	*t;		// Current node in tree
  uchar		*tname,		// Name value from tree entry
		*tcontent;	// Content value from tree entry


  for (t = this; t; t = t->next)
  {
    // Check this tree entry...
    if (t->markup == MARKUP_META &&
        (tname = t->var((uchar *)"NAME")) != NULL &&
        (tcontent = t->var((uchar *)"CONTENT")) != NULL)
      if (strcasecmp((char *)name, (char *)tname) == 0)
        return (tcontent);

    // Check child entries...
    if (t->child != NULL)
      if ((tcontent = t->child->get_meta(name)) != NULL)
        return (tcontent);
  }

  return (NULL);
}


//
// 'HDtree::var()' - Get a variable value from a node.
//

uchar *				// O - Value or NULL if variable does not exist
HDtree::var(uchar  *name)	// I - Variable name
{
  HDvar	*v,			// Matching variable
	key;			// Search key


  if (name == NULL || nvars == 0)
    return (NULL);

  key.name = name;

  v = (HDvar *)bsearch(&key, vars, nvars, sizeof(HDvar),
                       (int (*)(const void *, const void *))compare_variables);
  if (v == NULL)
    return (NULL);
  else if (v->value == NULL)
    return ((uchar *)"");
  else
    return (v->value);
}


//
// 'HDtree::var()' - Set a variable for a node.


int				// O - Set status: 0 = success, -1 = fail
HDtree::var(uchar  *name,	// I - Variable name
            uchar  *value)	// I - Variable value
{
  HDvar	*v,			// Matching variable
	key;			// Search key


  if (nvars == 0)
    v = NULL;
  else
  {
    key.name = name;

    v = (HDvar *)bsearch(&key, vars, nvars, sizeof(HDvar),
        	         (int (*)(const void *, const void *))compare_variables);
  }

  if (v == NULL)
  {
    if (nvars == 0)
      vars = (HDvar *)malloc(sizeof(HDvar));
    else
      vars = (HDvar *)realloc(vars, sizeof(HDvar) * (nvars + 1));

    if (vars == NULL)
    {
      nvars = 0;
      return (-1);
    }

    v = vars + nvars;
    nvars ++;
    v->name  = (uchar *)strdup((char *)name);
    if (value != NULL)
      v->value = (uchar *)strdup((char *)value);
    else
      v->value = NULL;

    if (strcasecmp((char *)name, "HREF") == 0)
      link = this;

    if (nvars > 1)
      qsort(vars, nvars, sizeof(HDvar),
            (int (*)(const void *, const void *))compare_variables);
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


//
// 'HDtree::set_base_size()' - Set the font sizes and spacings...
//

void
HDtree::set_base_size(float p,	// I - Point size of paragraph font
                      float s)	// I - Spacing
{
  int	i;			// Looping var


  p /= 1.2 * 1.2 * 1.2;
  for (i = 0; i < 8; i ++, p *= 1.2)
  {
    sizes[i]    = p;
    spacings[i] = p * s;
  }
}


//
// 'HDtree::setCharSet()' - Set the character set for output.
//

void
HDtree::set_char_set(const char *cs)	// I - Character set file to load
{
  int		i, j;		// Looping vars
  char		filename[1024];	// Filenames
  FILE		*fp;		// Files
  int		ch, unicode;	// Character values
  float		width;		// Width value
  char		glyph[64];	// Glyph name
  char		line[1024];	// Line from AFM file
  int		chars[256];	// Character encoding array


  if (strcmp(cs, char_set) == 0)
    return;

  strncpy(char_set, cs, sizeof(char_set) - 1);
  char_set[sizeof(char_set) - 1] = '\0';

  if (!initialized)
  {
    // Load the PostScript glyph names for all of Unicode...
    memset(glyphs_all, 0, sizeof(glyphs_all));

    sprintf(line, "%s/data/psglyphs", datadir);
    if ((fp = fopen(line, "r")) != NULL)
    {
      while (fscanf(fp, "%x%63s", &unicode, glyph) == 2)
        glyphs_all[unicode] = strdup(glyph);

      fclose(fp);

      initialized = 1;
    }
    else
      HTMLDOC::progress->error("Unable to open psglyphs data file!");
  }

  memset(glyphs, 0, sizeof(glyphs));

  sprintf(filename, "%s/data/%s", datadir, cs);
  if ((fp = fopen(filename, "r")) == NULL)
  {
    // Can't open charset file; use ISO-8859-1...
    HTMLDOC::progress->error("Unable to open character set file %s!", cs);

    for (i = 0; i < 256; i ++)
      chars[i] = i;

    // Hardcode characters 128 to 159 for Microsoft's version of ISO-8859-1...
    chars[0x80] = 0x20ac; // Euro
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
    // Read the <char> <unicode> lines from the file...
    memset(chars, 0, sizeof(chars));

    while (fscanf(fp, "%x%x", &ch, &unicode) == 2)
      chars[ch] = unicode;

    fclose(fp);
  }

  // Build the glyph array...
  for (i = 0; i < 256; i ++)
    if (chars[i] == 0)
      glyphs[i] = NULL;
    else
      glyphs[i] = glyphs_all[chars[i]];

  // Now read all of the font widths...
  for (i = 0; i < 4; i ++)
    for (j = 0; j < 4; j ++)
    {
      for (ch = 0; ch < 256; ch ++)
        widths[i][j][ch] = 0.6f;

      sprintf(filename, "%s/afm/%s", datadir, fonts[i][j]);
      if ((fp = fopen(filename, "r")) == NULL)
      {
        HTMLDOC::progress->error("Unable to open font width file %s!", fonts[i][j]);
        continue;
      }

      while (fgets(line, sizeof(line), fp) != NULL)
      {
        if (strncmp(line, "C ", 2) != 0)
	  continue;

        if (i < 3)
	{
	  // Handle encoding of Courier, Times, and Helvetica using
	  // assigned charset...
          if (sscanf(line, "%*s%*s%*s%*s%f%*s%*s%s", &width, glyph) != 2)
	    continue;

          for (ch = 0; ch < 256; ch ++)
	    if (glyphs[ch] && strcmp(glyphs[ch], glyph) == 0)
	      break;

          if (ch < 256)
	    widths[i][j][ch] = width * 0.001f;
	}
	else
	{
	  // Symbol font uses its own encoding...
          if (sscanf(line, "%*s%d%*s%*s%f", &ch, &width) != 2)
	    continue;

          if (ch < 256)
	    widths[i][j][ch] = width * 0.001f;
	}
      }

      fclose(fp);
    }
}


//
// 'HDtree::set_text_color()' - Set the default text color.
//

void
HDtree::set_text_color(uchar *color)	// I - Text color
{
  strncpy((char *)text_color, (char *)color, sizeof(text_color));
  text_color[sizeof(text_color) - 1] = '\0';
}


//
// 'HDtree::compare_variables()' - Compare two markup variables.
//

int					// O - Result of comparison
HDtree::compare_variables(HDvar *v0,	// I - First variable
                          HDvar *v1)	// I - Second variable
{
  return (strcasecmp((char *)v0->name, (char *)v1->name));
}


//
// 'HDtree::compare_markups()' - Compare two markup strings...
//

int					// O - Result of comparison
HDtree::compare_markups(uchar **m0,	// I - First markup
                        uchar **m1)	// I - Second markup
{
  return (strcasecmp((char *)*m0, (char *)*m1));
}


//
// 'HDtree::parse_markup()' - Parse a markup string.
//

int				// O - -1 on error, MARKUP_nnnn otherwise
HDtree::parse_markup(FILE *fp)	// I - Input file
{
  int		ch, ch2;	// Characters from file
  uchar		m[255],		// Markup string...
		*mptr,		// Current character...
		*cptr,		// Current char...
		**temp;		// Markup variable entry
  static uchar	comment[10240];	// Comment string


  mptr = m;

  while ((ch = getc(fp)) != EOF && mptr < (m + sizeof(m) - 1))
    if (ch == '>' || isspace(ch))
      break;
    else
    {
      *mptr++ = ch;

      // Handle comments without whitespace...
      if ((mptr - m) == 3 && memcmp((const char *)m, "!--", 3) == 0)
      {
        ch = getc(fp);
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (MARKUP_ERROR);

  mptr = m;
  temp = (uchar **)bsearch(&mptr, markups,
                           sizeof(markups) / sizeof(markups[0]),
                           sizeof(markups[0]),
                           (int (*)(const void *, const void *))compare_markups);

  if (temp == NULL)
  {
    // Unrecognized markup stuff...
    markup = MARKUP_COMMENT;
    strcpy((char *)comment, (char *)markup);
    cptr = comment + strlen((char *)comment);
  }
  else
  {
    markup = (HDmarkup)((const char **)temp - markups);
    cptr   = comment;
  }

  if (markup == MARKUP_COMMENT)
  {
    while (ch != EOF && cptr < (comment + sizeof(comment) - 1))
    {
      if (ch == '>' && temp == NULL)
        break;

      *cptr++ = ch;

      if (ch == '-')
      {
        if ((ch2 = getc(fp)) == '>')
	  break;
	else
	  ch = ch2;
      }
      else
        ch = getc(fp);
    }

    *cptr = '\0';
    data  = (uchar *)strdup((char *)comment);
  }
  else
  {
    while (ch != EOF && ch != '>')
    {
      if (!isspace(ch))
      {
        ungetc(ch, fp);
        parse_variable(fp);
      }

      ch = getc(fp);
    }
  }

  return (markup);
}


//
// 'HDtree::parse_variable()' - Parse a markup variable string.
//

int					// O - -1 on error, 0 on success
HDtree::parse_variable(FILE   *fp)	// I - Input file
{
  uchar	name[1024],			// Name of variable
	value[10240],			// Value of variable
	*ptr;				// Temporary pointer
  int	ch;				// Character from file


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
        return (var(name, NULL));
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
            else if (ptr < (value + sizeof(value) - 1))
              *ptr++ = ch;

          *ptr = '\0';
        }
        else if (ch == '\"')
        {
          while ((ch = getc(fp)) != EOF)
            if (ch == '\"')
              break;
            else if (ptr < (value + sizeof(value) - 1))
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

        return (var(name, value));
  }
}


//
// 'HDtree::get_size()' - Compute the width and height of a tree entry.
//

int				// O - 0 = success, -1 = failure
HDtree::get_size()
{
  uchar		*ptr;		// Current character
  float		width,		// Current width
		max_width;	// Maximum width
  uchar		*width_ptr,	// Pointer to width string
		*height_ptr,	// Pointer to height string
		*size_ptr,	// Pointer to size string
		*type_ptr;	// Pointer to spacer type string
  HDimage	*img;		// Image
  char		number[255];	// Width or height value


  if (!initialized)
    set_char_set("8859-1");

  switch (markup)
  {
    case MARKUP_IMG :
	width_ptr  = var((uchar *)"WIDTH");
	height_ptr = var((uchar *)"HEIGHT");

	img = HDimage::find((char *)var((uchar *)"SRC"), grayscale);

	if (width_ptr != NULL && height_ptr != NULL)
	{
	  width  = atoi((char *)width_ptr) / ppi * 72.0f;
	  height = atoi((char *)height_ptr) / ppi * 72.0f;
	  break;
	}

	if (img == NULL)
	  return (-1);

	if (width_ptr != NULL)
	{
	  width  = atoi((char *)width_ptr) / ppi * 72.0f;
	  height = width * img->height / img->width;

	  sprintf(number, "%d", atoi((char *)width_ptr) * img->height / img->width);
	  if (strchr((char *)width_ptr, '%') != NULL)
            strcat(number, "%");
	  var((uchar *)"HEIGHT", (uchar *)number);
	}
	else if (height_ptr != NULL)
	{
	  height = atoi((char *)height_ptr) / ppi * 72.0f;
	  width  = height * img->width / img->height;

	  sprintf(number, "%d", atoi((char *)height_ptr) * img->width / img->height);
	  if (strchr((char *)height_ptr, '%') != NULL)
            strcat(number, "%");
	  var((uchar *)"WIDTH", (uchar *)number);
	}
	else
	{
	  width  = img->width / ppi * 72.0f;
	  height = img->height / ppi * 72.0f;

	  sprintf(number, "%d", img->width);
	  var((uchar *)"WIDTH", (uchar *)number);

	  sprintf(number, "%d", img->height);
	  var((uchar *)"HEIGHT", (uchar *)number);
	}
	break;

    case MARKUP_SPACER :
	width_ptr  = var((uchar *)"WIDTH");
	height_ptr = var((uchar *)"HEIGHT");
	size_ptr   = var((uchar *)"SIZE");
	type_ptr   = var((uchar *)"TYPE");

	if (width_ptr != NULL)
	  width = atoi((char *)width_ptr) / ppi * 72.0f;
	else if (size_ptr != NULL)
	  width = atoi((char *)size_ptr) / ppi * 72.0f;
	else
	  width = 1.0f;

	if (height_ptr != NULL)
	  height = atoi((char *)height_ptr) / ppi * 72.0f;
	else if (size_ptr != NULL)
	  height = atoi((char *)size_ptr) / ppi * 72.0f;
	else
	  height = 1.0f;

	if (type_ptr == NULL)
	  break;

	if (strcasecmp((char *)type_ptr, "horizontal") == 0)
	  height = 0.0;
	else if (strcasecmp((char *)type_ptr, "vertical") == 0)
	  width = 0.0;
	break;

    case MARKUP_BR :
	width  = 0.0;
	height = sizes[size];
	break;

    default :
	if (preformatted && data)
	{
	  for (max_width = 0.0, width = 0.0, ptr = data; *ptr != '\0'; ptr ++)
	    if (*ptr == '\n')
	    {
              if (width > max_width)
        	max_width = width;
	    }
	    else if (*ptr == '\t')
              width = (float)(((int)width + 7) & ~7);
	    else
              width += widths[typeface][style][*ptr];

	 if (width < max_width)
	   width = max_width;
	}
	else if (data)
	  for (width = 0.0, ptr = data; *ptr != '\0'; ptr ++)
	    width += widths[typeface][style][*ptr];
	else
	  width = 0.0f;

	width  = width * sizes[size];
	height = sizes[size];
	break;
  }

  return (0);
}


//
// 'HDtree::set_color()' - Get the color value for the node.
//

int				// O - 0 = success, -1 = failure
HDtree::set_color(uchar  *color)// I - Color string
{
  float	rgb[3];			// RGB color


  HTMLDOC::get_color(color, rgb);

  red   = (uchar)(rgb[0] * 255.0f + 0.5f);
  green = (uchar)(rgb[1] * 255.0f + 0.5f);
  blue  = (uchar)(rgb[2] * 255.0f + 0.5f);

  return (0);
}


//
// 'get_alignment()' - Get horizontal & vertical alignment values.
//

int				// O - 0 for success, -1 for failure
HDtree::get_alignment()
{
  uchar	*align;			// Alignment string


  if ((align = var((uchar *)"ALIGN")) != NULL)
  {
    if (strcasecmp((char *)align, "left") == 0)
      halignment = ALIGN_LEFT;
    else if (strcasecmp((char *)align, "center") == 0)
      halignment = ALIGN_CENTER;
    else if (strcasecmp((char *)align, "right") == 0)
      halignment = ALIGN_RIGHT;
  }

  if ((align = var((uchar *)"VALIGN")) != NULL)
  {
    if (strcasecmp((char *)align, "top") == 0)
      valignment = ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      valignment = ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "center") == 0)
      valignment = ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
      valignment = ALIGN_BOTTOM;
  }

  return (0);
}


//
// 'HDtree::fix_filename()' - Fix a filename to be relative to the base
//                            directory.
//

char *						// O - Fixed filename
HDtree::fix_filename(char       *filename,	// I - Original filename
                     const char *base)		// I - Base directory
{
  char		*slash;				// Location of slash
  static char	newfilename[1024];		// New filename


  if (filename == NULL)
    return (NULL);

  if (strcmp(base, ".") == 0 || strncmp(filename, "http://", 7) == 0)
    return (filename);

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

  if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
      base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
    return (filename);		// No change needed for absolute path

  strcpy(newfilename, base);

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
    if ((slash = strrchr(newfilename, '/')) != NULL)
      *slash = '\0';
    else if ((slash = strrchr(newfilename, '\\')) != NULL)
      *slash = '\0';
#elif defined(MAC)
    if ((slash = strrchr(newfilename, ':')) != NULL)
      *slash = '\0';
#else
    if ((slash = strrchr(newfilename, '/')) != NULL)
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

  return (newfilename);
}


//
// 'HDtree::real_prev()' - Return the previous non-link markup in the tree.
//

HDtree *		// O - Pointer to previous markup
HDtree::real_prev()
{
  HDtree	*t;	// Current tree node


  if (prev && (prev->markup == MARKUP_A || prev->markup == MARKUP_COMMENT))
    t = prev;
  else
    t = this;

  if (t->prev)
    return (t->prev);

  t = t->parent;
  if (!t)
    return ((HDtree *)0);

  if (t->markup != MARKUP_A && t->markup != MARKUP_EMBED &&
      t->markup != MARKUP_COMMENT)
    return (t);
  else
    return (t->real_prev());
}


//
// 'HDtree::real_next()' - Return the next non-link markup in the tree.
//

HDtree *		// O - Pointer to next markup
HDtree::real_next()
{
  HDtree	*t;	// Current tree node


  if (next && (next->markup == MARKUP_A || next->markup == MARKUP_COMMENT))
    t = next;
  else
    t = this;

  if (t->next)
    return (t->next);
  else if (t->parent)
    return (t->parent->real_next());
  else
    return ((HDtree *)0);
}


//
// 'HDtree::copy()' - Copy a markup tree...
//

void
HDtree::copy(HDtree *p)		// I - Destination tree
{
  int		i;		// Looping var
  HDtree	*t,		// Current tree node
		*temp;		// New tree node
  HDvar		*v;		// Current markup variable


  for (t = this; t; t = t->next)
    if ((temp = new HDtree(p, t->markup, t->data)) != NULL)
    {
      temp->link          = t->link;
      temp->halignment    = t->halignment;
      temp->valignment    = t->valignment;
      temp->typeface      = t->typeface;
      temp->style         = t->style;
      temp->size          = t->size;
      temp->underline     = t->underline;
      temp->strikethrough = t->strikethrough;
      temp->superscript   = t->superscript;
      temp->subscript     = t->subscript;
      temp->preformatted  = t->preformatted;
      temp->indent        = t->indent;
      temp->red           = t->red;
      temp->green         = t->green;
      temp->blue          = t->blue;
      temp->width         = t->width;
      temp->height        = t->height;

      for (i = 0, v = t->vars; i < t->nvars; i ++, v ++)
        temp->var(v->name, v->value);

      if (t->child)
        t->child->copy(temp);
    }
}


//
// 'HDtree::dup()' - Duplicate a tree node.
//

HDtree *			// O - New node
HDtree::dup()
{
  HDtree	*t;		// New node


  if ((t = new HDtree()) == (HDtree *)0)
    return ((HDtree *)0);

  t->is_copy       = 1;
  t->markup        = markup;
  t->data          = data;
  t->link          = link;
  t->halignment    = halignment;
  t->valignment    = valignment;
  t->typeface      = typeface;
  t->style         = style;
  t->size          = size;
  t->underline     = underline;
  t->strikethrough = strikethrough;
  t->superscript   = superscript;
  t->subscript     = subscript;
  t->preformatted  = preformatted;
  t->indent        = indent;
  t->red           = red;
  t->green         = green;
  t->blue          = blue;
  t->width         = width;
  t->height        = height;
  t->nvars         = nvars;
  t->vars          = vars;
}


//
// 'HDtree::flatten()' - Flatten an HTML tree to only include the text, image,
//                       link, and break markups.

HDtree *			// O - Flattened markup tree
HDtree::flatten(float  padding)	// I - Padding for table cells
{
  HDtree	*t,		// Current tree node
		*temp,		// New tree node
		*flat;		// Flattened tree
  float		newpadding;	// New padding for table cells
  uchar		*v;		// Attribute value


  for (t = this, flat = (HDtree *)0; t; t = t->next)
  {
    newpadding = padding;

    switch (t->markup)
    {
      case MARKUP_NONE :
          if (!t->data)
	  {
	    temp = NULL;
	    break;
	  }
      case MARKUP_BR :
      case MARKUP_SPACER :
      case MARKUP_IMG :
	  temp = t->dup();
          break;

      case MARKUP_A :
          if (t->var((uchar *)"NAME"))
	    temp = t->dup();
          else
	    temp = NULL;
	  break;

      case MARKUP_TD :
      case MARKUP_TH :
	  temp         = new HDtree();
	  temp->markup = MARKUP_SPACER;
	  temp->width  = padding;
          break;

      case MARKUP_TABLE :
	  temp         = new HDtree();
	  temp->markup = MARKUP_SPACER;

	  if ((v = t->var((uchar *)"CELLPADDING")) != NULL)
	    newpadding = 2.0f * atof((char *)v);
	  else
	    newpadding = 2.0f;

	  if ((v = t->var((uchar *)"CELLSPACING")) != NULL)
	    newpadding += 2.0f * atof((char *)v);

	  if ((v = t->var((uchar *)"BORDER")) != NULL)
	  {
	    if (!isdigit(v[0]))
	      temp->width = 2.0f;
	    else
	      temp->width = 2.0f * atof((char *)v);
	  }
	  break;

      case MARKUP_P :
      case MARKUP_PRE :
      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_UL :
      case MARKUP_DIR :
      case MARKUP_MENU :
      case MARKUP_OL :
      case MARKUP_DL :
      case MARKUP_LI :
      case MARKUP_DD :
      case MARKUP_DT :
      case MARKUP_TR :
      case MARKUP_CAPTION :
	  temp         = new HDtree();
	  temp->markup = MARKUP_BR;
          break;

      default :
          temp = (HDtree *)0;
	  break;
    }

    if (temp)
    {
      temp->prev = flat;

      if (flat)
        flat->next = temp;

      flat = temp;
    }

    if (t->child != NULL)
    {
      temp = t->child->flatten(newpadding);

      if (temp)
        temp->prev = flat;

      if (flat)
        flat->next = temp;
      else
        flat = temp;
    }

    if (flat)
      while (flat->next)
        flat = flat->next;
  }

  if (flat)
    while (flat->prev)
      flat = flat->prev;

  return (flat);
}


//
// End of "$Id: htmllib.cxx,v 1.43 2000/11/06 19:53:04 mike Exp $".
//
