//
// HTML parsing routines for HTMLDOC, a HTML document processing program.
//
// Copyright 2011-2024 by Michael R Sweet.
// Copyright 1997-2010 by Easy Software Products.  All rights reserved.
//
// This program is free software.  Distribution and use rights are outlined in
// the file "COPYING".
//

#include "htmldoc.h"
#include <cups/http.h>
#include <ctype.h>


//
// Markup strings...
//

const char	*_htmlMarkups[] =
		{
		  "",		// MARKUP_NONE
		  "!--",	// MARKUP_COMMENT
		  "!DOCTYPE",
		  "a",
		  "acronym",
		  "address",
		  "applet",
		  "area",
		  "b",
		  "base",
		  "basefont",
		  "big",
		  "blink",
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
		  "frame",
		  "frameset",
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
		  "multicol",
		  "nobr",
		  "noframes",
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

const char	*_htmlCurrentFile = "UNKNOWN";
					// Current file
const char	*_htmlData = HTML_DATA;	// Data directory
float		_htmlPPI = 80.0f;	// Image resolution
int		_htmlGrayscale = 0;	// Grayscale output?
uchar		_htmlTextColor[255] =	// Default text color
		{ 0 };
float		_htmlBrowserWidth = 680.0f;
					// Browser width for pixel scaling
float		_htmlSizes[8] =		// Point size for each HTML size
		{ 6.0f, 8.0f, 9.0f, 11.0f, 14.0f, 17.0f, 20.0f, 24.0f };
float		_htmlSpacings[8] =	// Line height for each HTML size
		{ 7.2f, 9.6f, 10.8f, 13.2f, 16.8f, 20.4f, 24.0f, 28.8f };
typeface_t	_htmlBodyFont = TYPE_TIMES,
		_htmlHeadingFont = TYPE_HELVETICA;

int		_htmlInitialized = 0;	// Initialized glyphs yet?
char		_htmlCharSet[256] = "iso-8859-1";
					// Character set name
int		_htmlWidthsLoaded[TYPE_MAX][STYLE_MAX] =
		{			// Have the widths been loaded?
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 },
		  { 0, 0, 0, 0 }
		};
short		_htmlWidths[TYPE_MAX][STYLE_MAX][256];
					// Character widths of fonts
short		_htmlWidthsAll[TYPE_MAX][STYLE_MAX][65536];
                                        // Unicode widths of fonts
int		_htmlUnicode[256];	// Character to Unicode mapping
uchar           _htmlCharacters[65536]; // Unicode to character mapping
int             _htmlUTF8 = 0;          // Doing UTF-8?
const char	*_htmlGlyphsAll[65536];	// Character glyphs for Unicode
const char	*_htmlGlyphs[256];	// Character glyphs for charset
int		_htmlNumSorted = 0;	// Number of sorted glyphs
const char	*_htmlSorted[256];	// Sorted character glyphs for charset
uchar		_htmlSortedChars[256];	// Sorted character indices
const char	*_htmlFonts[TYPE_MAX][STYLE_MAX] =
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
		    "Monospace",
		    "Monospace-Bold",
		    "Monospace-Oblique",
		    "Monospace-BoldOblique"
		  },
		  {
		    "Serif-Roman",
		    "Serif-Bold",
		    "Serif-Oblique",
		    "Serif-BoldOblique"
		  },
		  {
		    "Sans",
		    "Sans-Bold",
		    "Sans-Oblique",
		    "Sans-BoldOblique"
		  },
		  {
		    "Symbol",
		    "Symbol",
		    "Symbol",
		    "Symbol"
		  },
		  {
		    "Dingbats",
		    "Dingbats",
		    "Dingbats",
		    "Dingbats"
		  }
		};
int		_htmlStandardFonts[TYPE_MAX] =
		{
		  1,	// Courier
		  1,	// Times
		  1,	// Helvetica
		  0,	// Monospace
		  0,	// Sans
		  0,	// Serif
		  1,	// Symbol
		  1	// Dingbats
		};


//
// Local functions.
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static int	write_file(tree_t *t, FILE *fp, int col);
static int	compare_variables(var_t *v0, var_t *v1);
static int	compare_markups(uchar **m0, uchar **m1);
static void	delete_node(tree_t *t);
static void	insert_space(tree_t *parent, tree_t *t);
static int	parse_markup(tree_t *t, FILE *fp, int *linenum);
static int	parse_variable(tree_t *t, FILE *fp, int *linenum);
static int	compute_size(tree_t *t);
static int	compute_color(tree_t *t, uchar *color);
static int	get_alignment(tree_t *t);
static const char *fix_filename(char *path, char *base);
static int      utf8_getc(int ch, FILE *fp);

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

#ifdef DEBUG
static uchar	indent[255] = "";
#endif // DEBUG


//
// 'htmlReadFile()' - Read a file for HTML markup codes.
//

tree_t *				// O - Pointer to top of file tree
htmlReadFile(tree_t     *parent,	// I - Parent tree entry
             FILE       *fp,		// I - File pointer
	     const char *base)		// I - Base directory for file
{
  int		ch;			// Character from file
  uchar		*ptr,			// Pointer in string
		entity[16],		// Character entity name (&#nnn; or &name;)
		*eptr;			// Pointer in entity string
  tree_t	*tree,			// "top" of this tree
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
		*type,			// Type for EMBED tag
		*span;			// Value for SPAN tag
  int		sizeval;		// Size value from FONT tag
  int		linenum;		// Line number in file
  static uchar	s[10240];		// String from file
  static int	have_whitespace = 0;	// Non-zero if there was leading whitespace


  DEBUG_printf(("htmlReadFile(parent=%p, fp=%p, base=\"%s\")\n",
                (void *)parent, (void *)fp, base ? base : "(null)"));

#ifdef DEBUG
  indent[0] = '\0';
#endif // DEBUG

  // Start off with no previous tree entry...
  prev = NULL;
  tree = NULL;

  // Parse data until we hit end-of-file...
  linenum = 1;

  while ((ch = getc(fp)) != EOF)
  {
    // Ignore leading whitespace...
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

    // Allocate a new tree entry - use calloc() to get zeroed data...
    t = (tree_t *)calloc(sizeof(tree_t), 1);
    if (t == NULL)
    {
#ifndef DEBUG
      progress_error(HD_ERROR_OUT_OF_MEMORY,
                     "Unable to allocate memory for HTML tree node!");
#endif // !DEBUG
      break;
    }

    // Set/copy font characteristics...
    if (parent == NULL)
    {
      t->halignment   = ALIGN_LEFT;
      t->valignment   = ALIGN_BOTTOM;
      t->typeface     = _htmlBodyFont;
      t->size         = SIZE_P;

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

    // See what the character was...
    if (ch == '<')
    {
      // Markup char; grab the next char to see if this is a /...
      ch = getc(fp);

      if (isspace(ch) || ch == '=' || ch == '<')
      {
        // Sigh...  "<" followed by anything but an element name is
	// invalid HTML, but so many people have asked for this to
	// be supported that we have added this hack...
	progress_error(HD_ERROR_HTML_ERROR, "Unquoted < on line %d of %s.",
	               linenum, _htmlCurrentFile);

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

	t->markup = MARKUP_NONE;
	t->data   = (uchar *)hd_strdup((char *)s);
      }
      else
      {
        // Start of a markup...
	if (ch != '/')
          ungetc(ch, fp);

	if (parse_markup(t, fp, &linenum) == MARKUP_ERROR)
	{
#ifndef DEBUG
          progress_error(HD_ERROR_READ_ERROR,
                         "Unable to parse HTML element on line %d of %s!",
			 linenum, _htmlCurrentFile);
#endif // !DEBUG

          delete_node(t);
          break;
	}

        // Eliminate extra whitespace...
	if (issuper(t->markup) || isblock(t->markup) || islist(t->markup) || islentry(t->markup) ||
            istable(t->markup) || istentry(t->markup) || t->markup == MARKUP_TITLE)
          have_whitespace = 0;

        // If this is the matching close mark, or if we are starting the same
	// markup, or if we've completed a list, we're done!
	if (ch == '/')
	{
	  // Close markup; find matching markup...
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (temp->markup == t->markup)
            {
              break;
	    }
	    else if (temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	  }
	}
	else if (t->markup == MARKUP_BODY || t->markup == MARKUP_HEAD)
	{
	  // Make sure we aren't inside an existing HEAD or BODY...
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (temp->markup == MARKUP_BODY || temp->markup == MARKUP_HEAD)
            {
              break;
	    }
	    else if (temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	  }
	}
	else if (t->markup == MARKUP_EMBED)
	{
	  // Close any text blocks...
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (isblock(temp->markup) || islentry(temp->markup))
            {
              break;
	    }
	    else if (istentry(temp->markup) || islist(temp->markup) || issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	  }
	}
	else if (issuper(t->markup))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
	    if (istentry(temp->markup) || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	  }
	}
	else if (islist(t->markup))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (isblock(temp->markup))
	    {
	      break;
	    }
	    else if (islentry(temp->markup) || istentry(temp->markup) || issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	  }
	}
	else if (islentry(t->markup))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (islentry(temp->markup))
            {
              break;
	    }
	    else if (islist(temp->markup) || issuper(temp->markup) || istentry(temp->markup) || temp->markup == MARKUP_EMBED)
            {
	      temp = NULL;
	      break;
	    }
	  }
	}
	else if (isblock(t->markup))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (isblock(temp->markup))
            {
              break;
	    }
	    else if (istentry(temp->markup) || islist(temp->markup) || islentry(temp->markup) || issuper(temp->markup) || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
	      break;
	    }
	  }
	}
	else if (t->markup == MARKUP_THEAD || t->markup == MARKUP_TBODY || t->markup == MARKUP_TFOOT)
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
	    if (temp->markup == MARKUP_TABLE || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	  }
	}
	else if (t->markup == MARKUP_TR)
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (temp->markup == MARKUP_TR)
            {
              break;
            }
            else if (temp->markup == MARKUP_TABLE || t->markup == MARKUP_THEAD || t->markup == MARKUP_TBODY || t->markup == MARKUP_TFOOT || temp->markup == MARKUP_EMBED)
	    {
	      temp = NULL;
              break;
	    }
	  }
	}
	else if (istentry(t->markup))
	{
          for (temp = parent; temp != NULL; temp = temp->parent)
          {
            if (istentry(temp->markup))
            {
              break;
	    }
	    else if (temp->markup == MARKUP_TABLE || istable(temp->markup) || temp->markup == MARKUP_EMBED)
	    {
	      if (temp->markup != MARKUP_TR)
	      {
	        // Strictly speaking, this is an error - TD/TH can only
		// be found under TR, but web browsers automatically
		// inject a TR...
		progress_error(HD_ERROR_HTML_ERROR,
		               "No TR element before %s element on line %d of %s.",
			       _htmlMarkups[t->markup], linenum,
			       _htmlCurrentFile);

                parent = htmlAddTree(temp, MARKUP_TR, NULL);
		prev   = NULL;
		DEBUG_printf(("%str (inserted) under %s, line %d\n", indent,
		              _htmlMarkups[temp->markup], linenum));
	      }

	      temp = NULL;
              break;
	    }
	  }
	}
	else
	{
          temp = NULL;
        }

	if (temp != NULL)
	{
          DEBUG_printf(("%s>>>> Auto-ascend <<<\n", indent));

          if (ch != '/' &&
	      temp->markup != MARKUP_BODY &&
	      temp->markup != MARKUP_DD &&
	      temp->markup != MARKUP_DT &&
	      temp->markup != MARKUP_HEAD &&
	      temp->markup != MARKUP_HTML &&
	      temp->markup != MARKUP_LI &&
	      temp->markup != MARKUP_OPTION &&
	      temp->markup != MARKUP_P &&
	      temp->markup != MARKUP_TBODY &&
	      temp->markup != MARKUP_TD &&
	      temp->markup != MARKUP_TFOOT &&
	      temp->markup != MARKUP_TH &&
	      temp->markup != MARKUP_THEAD &&
	      temp->markup != MARKUP_TR)
	  {
	    // Log this condition as an error...
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "No /%s element before %s element on line %d of %s.",
	                   _htmlMarkups[temp->markup],
			   _htmlMarkups[t->markup], linenum, _htmlCurrentFile);
	    DEBUG_printf(("%sNo /%s element before %s element on line %d.\n",
	                  indent, _htmlMarkups[temp->markup],
			  _htmlMarkups[t->markup], linenum));
	  }

#ifdef DEBUG
          for (tree_t *p = parent;
	       p && p != temp && indent[0];
	       p = p->parent)
	    indent[strlen((char *)indent) - 4] = '\0';

          if (indent[0])
            indent[strlen((char *)indent) - 4] = '\0';
#endif // DEBUG

          // Safety check; should never happen, since MARKUP_FILE is
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
	      t->halignment   = ALIGN_LEFT;
	      t->valignment   = ALIGN_BOTTOM;
	      t->typeface     = _htmlBodyFont;
	      t->size         = SIZE_P;

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
	  if (t->markup != MARKUP_UNKNOWN &&
	      t->markup != MARKUP_COMMENT)
	  {
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "Dangling /%s element on line %d of %s.",
			   _htmlMarkups[t->markup], linenum, _htmlCurrentFile);
	    DEBUG_printf(("%sDangling /%s element on line %d.\n",
			  indent, _htmlMarkups[t->markup], linenum));
          }

	  delete_node(t);
	  continue;
	}
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
	  // Possibly a character entity...
	  eptr = entity;
	  while (eptr < (entity + sizeof(entity) - 1) &&
	         (ch = getc(fp)) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = (uchar)ch;

          if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

          *eptr = '\0';
          if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d of %s.",
	                   linenum, _htmlCurrentFile);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (size_t)(ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "Unknown character entity \"&%s;\" on line %d of %s.",
	                   entity, linenum, _htmlCurrentFile);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (size_t)(ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else
	    *ptr++ = (uchar)ch;
        }
        else if ((ch & 0x80) && _htmlUTF8)
        {
          // Collect UTF-8 value...
          ch = utf8_getc(ch, fp);

          if (ch)
            *ptr++ = (uchar)ch;
        }
	else if (ch != 0 && ch != '\r')
	{
          *ptr++ = (uchar)ch;

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

      t->markup = MARKUP_NONE;
      t->data   = (uchar *)hd_strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\", line %d\n", indent, s, linenum));
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
	         (ch = getc(fp)) != EOF)
	    if (!isalnum(ch) && ch != '#')
	      break;
	    else
	      *eptr++ = (uchar)ch;

          *eptr = '\0';

          if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

          if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d of %s.",
	                   linenum, _htmlCurrentFile);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (size_t)(ptr - s));
	    ptr += strlen((char *)ptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "Unknown character entity \"&%s;\" on line %d of %s.",
	                   entity, linenum, _htmlCurrentFile);

            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = '&';
            strlcpy((char *)ptr, (char *)entity, sizeof(s) - (size_t)(ptr - s));
	    ptr += strlen((char *)ptr);
            if (ptr < (s + sizeof(s) - 1))
	      *ptr++ = ';';
	  }
	  else
	    *ptr++ = (uchar)ch;
        }
        else
        {
          if ((ch & 0x80) && _htmlUTF8)
          {
            // Collect UTF-8 value...
            ch = utf8_getc(ch, fp);
          }

          if (ch)
            *ptr++ = (uchar)ch;
        }

	if ((_htmlUTF8 && ch == _htmlCharacters[173]) || (!_htmlUTF8 && ch == 173))
	  break;

        ch = getc(fp);
      }

      if (ch == '\n')
	linenum ++;

      if (isspace(ch))
        have_whitespace = 1;

      *ptr = '\0';

      if (ch == '<')
        ungetc(ch, fp);

      t->markup = MARKUP_NONE;
      t->data   = (uchar *)hd_strdup((char *)s);

      DEBUG_printf(("%sfragment \"%s\" (len=%d), line %d\n", indent, s,
                    (int)(ptr - s), linenum));
    }

    // If the parent tree pointer is not null and this is the first
    // entry we've read, set the child pointer...
    DEBUG_printf(("%sADDING %s node to %s parent!\n", indent,
                  _htmlMarkups[t->markup],
		  parent ? _htmlMarkups[parent->markup] : "ROOT"));
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

    switch (t->markup)
    {
      case MARKUP_BODY :
          // Update the text color as necessary...
          if ((color = htmlGetVariable(t, (uchar *)"TEXT")) != NULL)
            compute_color(t, color);
	  else
            compute_color(t, _htmlTextColor);

          if ((color = htmlGetVariable(t, (uchar *)"BGCOLOR")) != NULL &&
	      !BodyColor[0])
	    strlcpy(BodyColor, (char *)color, sizeof(BodyColor));

          // Update the background image as necessary...
          if ((filename = htmlGetVariable(t, (uchar *)"BACKGROUND")) != NULL)
	    htmlSetVariable(t, (uchar *)"BACKGROUND",
	                    (uchar *)fix_filename((char *)filename,
			                          (char *)base));

          descend = 1;
          break;

      case MARKUP_IMG :
          if (have_whitespace)
	  {
	    // Insert a space before this image...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          // Get the image alignment...
          t->valignment = ALIGN_BOTTOM;
          get_alignment(t);

          // Update the image source as necessary...
          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	    htmlSetVariable(t, (uchar *)"REALSRC",
	                    (uchar *)fix_filename((char *)filename,
			                          (char *)base));

      case MARKUP_BR :
      case MARKUP_NONE :
      case MARKUP_SPACER :
	  // Figure out the width & height of this markup...
          compute_size(t);
	  break;

      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
          get_alignment(t);

          t->typeface      = _htmlHeadingFont;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;

	  if (t->markup > MARKUP_H6)
          {
	    t->size  = SIZE_H7;
            t->style = STYLE_ITALIC;
	  }
	  else
	  {
            t->size  = (unsigned)(SIZE_H1 - t->markup + MARKUP_H1);
            t->style = STYLE_BOLD;
	  }

          descend = 1;
          break;

      case MARKUP_P :
          get_alignment(t);

          t->typeface      = _htmlBodyFont;
          t->size          = SIZE_P;
          t->style         = STYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;

          descend = 1;
          break;

      case MARKUP_PRE :
          t->typeface      = _htmlBodyFont >= TYPE_MONOSPACE ? TYPE_MONOSPACE
	                                                     : TYPE_COURIER;
          t->size          = SIZE_PRE;
          t->style         = STYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 1;

          descend = 1;
          break;

      case MARKUP_BLOCKQUOTE :
      case MARKUP_DIR :
      case MARKUP_MENU :
      case MARKUP_UL :
      case MARKUP_OL :
      case MARKUP_DL :
          t->indent ++;

          descend = 1;
          break;

      case MARKUP_DIV :
          get_alignment(t);

          descend = 1;
          break;

      case MARKUP_HR :
          t->halignment = ALIGN_CENTER;
          get_alignment(t);
          break;

      case MARKUP_DOCTYPE :
      case MARKUP_AREA :
      case MARKUP_COMMENT :
      case MARKUP_INPUT :
      case MARKUP_ISINDEX :
      case MARKUP_LINK :
      case MARKUP_META :
      case MARKUP_WBR :
      case MARKUP_COL :
          break;

      case MARKUP_EMBED :
          if ((type = htmlGetVariable(t, (uchar *)"TYPE")) != NULL &&
	      strncasecmp((const char *)type, "text/html", 9) != 0)
	    break;

          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	  {
	    const char *save_name = _htmlCurrentFile;

	    filename = (uchar *)fix_filename((char *)filename,
	                                     (char *)base);

            if ((embed = fopen((char *)filename, "r")) != NULL)
            {
	      strlcpy(newbase, file_directory((char *)filename), sizeof(newbase));

              _htmlCurrentFile = (char *)filename;
              htmlReadFile(t, embed, newbase);
              fclose(embed);
	      _htmlCurrentFile = save_name;
            }
#ifndef DEBUG
	    else
	      progress_error(HD_ERROR_FILE_NOT_FOUND,
                             "Unable to embed \"%s\" - %s", filename,
	                     strerror(errno));
#endif // !DEBUG
	  }
          break;

      case MARKUP_TH :
          if (htmlGetVariable(t->parent, (uchar *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = ALIGN_CENTER;

          if (htmlGetVariable(t->parent, (uchar *)"VALIGN") != NULL)
	    t->valignment = t->parent->valignment;
	  else
            t->valignment = ALIGN_MIDDLE;

          get_alignment(t);

          t->style = STYLE_BOLD;

          descend = 1;
          break;

      case MARKUP_TD :
          if (htmlGetVariable(t->parent, (uchar *)"ALIGN") != NULL)
	    t->halignment = t->parent->halignment;
	  else
            t->halignment = ALIGN_LEFT;

          if (htmlGetVariable(t->parent, (uchar *)"VALIGN") != NULL)
	    t->valignment = t->parent->valignment;
	  else
            t->valignment = ALIGN_MIDDLE;

	  get_alignment(t);

          t->style = STYLE_NORMAL;

          descend = 1;
          break;

      case MARKUP_SPAN :
          // Pull style data, if present...
          if (have_whitespace)
	  {
	    // Insert a space before this element...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          get_alignment(t);

          if ((color = htmlGetStyle(t, (uchar *)"color:")) != NULL)
            compute_color(t, color);

          if ((face = htmlGetStyle(t, (uchar *)"font-family:")) != NULL)
          {
	    char	font[255],	// Font name
			*fontptr;	// Pointer into font name

            for (ptr = face; *ptr;)
	    {
	      while (isspace(*ptr) || *ptr == ',')
	        ptr ++;

              if (!*ptr)
	        break;

	      for (fontptr = font; *ptr && *ptr != ',' && !isspace(*ptr); ptr ++)
	        if (fontptr < (font + sizeof(font) - 1))
		  *fontptr++ = (char)*ptr;

              *fontptr = '\0';

              if (!strcasecmp(font, "serif"))
	      {
        	t->typeface = TYPE_SERIF;
		break;
	      }
              else if (!strcasecmp(font, "sans-serif") ||
	               !strcasecmp(font, "sans"))
	      {
        	t->typeface = TYPE_SANS_SERIF;
		break;
	      }
              else if (!strcasecmp(font, "monospace"))
	      {
        	t->typeface = TYPE_MONOSPACE;
		break;
	      }
              else if (!strcasecmp(font, "arial") ||
	               !strcasecmp(font, "helvetica"))
              {
        	t->typeface = TYPE_HELVETICA;
		break;
	      }
              else if (!strcasecmp(font, "times"))
	      {
        	t->typeface = TYPE_TIMES;
		break;
	      }
              else if (!strcasecmp(font, "courier"))
	      {
        	t->typeface = TYPE_COURIER;
		break;
	      }
	      else if (!strcasecmp(font, "symbol"))
	      {
        	t->typeface = TYPE_SYMBOL;
		break;
	      }
	      else if (!strcasecmp(font, "dingbat"))
	      {
        	t->typeface = TYPE_DINGBATS;
		break;
	      }
	    }
          }

          if ((size = htmlGetStyle(t, (uchar *)"font-size:")) != NULL)
          {
            // Find the closest size to the fixed sizes...
            unsigned i;
            double fontsize = atof((char *)size);

            for (i = 0; i < 7; i ++)
              if (fontsize <= _htmlSizes[i])
                break;

	    t->size = i;
          }

          if ((span = htmlGetStyle(t, (uchar *)"font-style:")) != NULL)
          {
            if (!strcmp((char *)span, "normal"))
              t->style &= ~STYLE_ITALIC;
            else if (!strcmp((char *)span, "italic") || !strcmp((char *)span, "oblique"))
              t->style |= STYLE_ITALIC;
          }

          if ((span = htmlGetStyle(t, (uchar *)"font-weight:")) != NULL)
          {
            if (!strcmp((char *)span, "bold") || !strcmp((char *)span, "bolder") || !strcmp((char *)span, "700") || !strcmp((char *)span, "800") || !strcmp((char *)span, "900"))
              t->style |= STYLE_BOLD;
            else if (strcmp((char *)span, "inherit"))
              t->style &= ~STYLE_BOLD;
          }

          if ((span = htmlGetStyle(t, (uchar *)"text-decoration:")) != NULL)
          {
            if (!strcmp((char *)span, "underline"))
              t->underline = 1;
            else if (!strcmp((char *)span, "line-through"))
              t->strikethrough = 1;
            else if (strcmp((char *)span, "inherit"))
              t->underline = t->strikethrough = 0;
          }

          if ((span = htmlGetStyle(t, (uchar *)"vertical-align:")) != NULL)
          {
            if (!strcmp((char *)span, "sub"))
              t->subscript = 1;
            else if (!strcmp((char *)span, "super"))
              t->superscript = 1;
            else if (strcmp((char *)span, "inherit"))
              t->subscript = t->superscript = 0;
          }

          descend = 1;
          break;

      case MARKUP_FONT :
          if (have_whitespace)
	  {
	    // Insert a space before this element...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          if ((face = htmlGetVariable(t, (uchar *)"FACE")) != NULL)
          {
	    char	font[255],	// Font name
			*fontptr;	// Pointer into font name


            for (ptr = face; *ptr;)
	    {
	      while (isspace(*ptr) || *ptr == ',')
	        ptr ++;

              if (!*ptr)
	        break;

	      for (fontptr = font; *ptr && *ptr != ',' && !isspace(*ptr); ptr ++)
	        if (fontptr < (font + sizeof(font) - 1))
		  *fontptr++ = (char)*ptr;

              *fontptr = '\0';

              if (!strcasecmp(font, "serif"))
	      {
        	t->typeface = TYPE_SERIF;
		break;
	      }
              else if (!strcasecmp(font, "sans-serif") ||
	               !strcasecmp(font, "sans"))
	      {
        	t->typeface = TYPE_SANS_SERIF;
		break;
	      }
              else if (!strcasecmp(font, "mono") || !strcasecmp(font, "monospace"))
	      {
        	t->typeface = TYPE_MONOSPACE;
		break;
	      }
              else if (!strcasecmp(font, "arial") ||
	               !strcasecmp(font, "helvetica"))
              {
        	t->typeface = TYPE_HELVETICA;
		break;
	      }
              else if (!strcasecmp(font, "times"))
	      {
        	t->typeface = TYPE_TIMES;
		break;
	      }
              else if (!strcasecmp(font, "courier"))
	      {
        	t->typeface = TYPE_COURIER;
		break;
	      }
	      else if (!strcasecmp(font, "symbol"))
	      {
        	t->typeface = TYPE_SYMBOL;
		break;
	      }
	      else if (!strcasecmp(font, "dingbat"))
	      {
        	t->typeface = TYPE_DINGBATS;
		break;
	      }
	    }
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
	      sizeval = atoi((char *)size);
	    else
              sizeval = t->size + atoi((char *)size);

            if (sizeval < 0)
              t->size = 0;
            else if (sizeval > 7)
              t->size = 7;
            else
              t->size = (unsigned)sizeval;
          }

          descend = 1;
          break;

      case MARKUP_BIG :
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

          descend = 1;
          break;

      case MARKUP_SMALL :
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

          descend = 1;
          break;

      case MARKUP_SUP :
          if (have_whitespace)
	  {
	    // Insert a space before superscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->superscript = 1;

          if ((sizeval = t->size + SIZE_SUP) < 0)
	    t->size = 0;
	  else
	    t->size = (unsigned)sizeval;

          descend = 1;
          break;

      case MARKUP_SUB :
          if (have_whitespace)
	  {
	    // Insert a space before subscript text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->subscript = 1;

          if ((sizeval = t->size + SIZE_SUB) < 0)
	    t->size = 0;
	  else
	    t->size = (unsigned)sizeval;

          descend = 1;
          break;

      case MARKUP_KBD :
          t->style    = STYLE_BOLD;

      case MARKUP_TT :
      case MARKUP_CODE :
      case MARKUP_SAMP :
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

          t->typeface = _htmlBodyFont >= TYPE_MONOSPACE ? TYPE_MONOSPACE
	                                                : TYPE_COURIER;

          descend = 1;
          break;

      case MARKUP_STRONG :
      case MARKUP_B :
          t->style = (style_t)(t->style | STYLE_BOLD);

          descend = 1;
          break;

      case MARKUP_DD :
          t->indent ++;

          descend = 1;
          break;

      case MARKUP_VAR :
          t->style = (style_t)(t->style | STYLE_ITALIC);
      case MARKUP_DFN :
          t->typeface = _htmlBodyFont >= TYPE_MONOSPACE ? TYPE_SANS_SERIF
	                                                : TYPE_HELVETICA;

          descend = 1;
          break;

      case MARKUP_CITE :
      case MARKUP_EM :
      case MARKUP_I :
          t->style = (style_t)(t->style | STYLE_ITALIC);

          descend = 1;
          break;

      case MARKUP_U :
      case MARKUP_INS :
          if (have_whitespace)
	  {
	    // Insert a space before underlined text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->underline = 1;

          descend = 1;
          break;

      case MARKUP_STRIKE :
      case MARKUP_S :
      case MARKUP_DEL :
          if (have_whitespace)
	  {
	    // Insert a space before struck-through text...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          t->strikethrough = 1;

          descend = 1;
          break;

      case MARKUP_CENTER :
          t->halignment = ALIGN_CENTER;

          descend = 1;
          break;

      case MARKUP_A :
          if (have_whitespace)
	  {
	    // Insert a space before this link...
	    insert_space(parent, t);

	    have_whitespace = 0;
	  }

          descend = 1;
	  break;

      default :
          // All other markup types should be using <MARK>...</MARK>
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


//
// 'write_file()' - Write a tree entry to a file...
//

static int			// I - New column
write_file(tree_t *t,		// I - Tree entry
           FILE   *fp,		// I - File to write to
           int    col)		// I - Current column
{
  int	i;			// Looping var
  uchar	*ptr;			// Character pointer


  while (t != NULL)
  {
    if (t->markup == MARKUP_NONE)
    {
      if (t->preformatted)
      {
        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fputs((char *)iso8859(*ptr), fp);

	if (t->data[0] && t->data[strlen((char *)t->data) - 1] == '\n')
          col = 0;
	else
          col += strlen((char *)t->data);
      }
      else
      {
	if ((col + (int)strlen((char *)t->data)) > 72 && col > 0)
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
    else if (t->markup == MARKUP_COMMENT)
      fprintf(fp, "\n<!--%s-->\n", t->data);
    else if (t->markup > 0)
    {
      switch (t->markup)
      {
        case MARKUP_AREA :
        case MARKUP_BR :
        case MARKUP_CENTER :
        case MARKUP_COMMENT :
        case MARKUP_DD :
        case MARKUP_DL :
        case MARKUP_DT :
        case MARKUP_H1 :
        case MARKUP_H2 :
        case MARKUP_H3 :
        case MARKUP_H4 :
        case MARKUP_H5 :
        case MARKUP_H6 :
        case MARKUP_HEAD :
        case MARKUP_HR :
        case MARKUP_LI :
        case MARKUP_MAP :
        case MARKUP_OL :
        case MARKUP_P :
        case MARKUP_PRE :
        case MARKUP_TABLE :
        case MARKUP_TITLE :
        case MARKUP_TR :
        case MARKUP_UL :
	case MARKUP_DIR :
	case MARKUP_MENU :
            if (col > 0)
            {
              putc('\n', fp);
              col = 0;
            }
        default :
            break;
      }

      col += fprintf(fp, "<%s", _htmlMarkups[t->markup]);
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

        col += fprintf(fp, "</%s>", _htmlMarkups[t->markup]);
        switch (t->markup)
        {
          case MARKUP_AREA :
          case MARKUP_BR :
          case MARKUP_CENTER :
          case MARKUP_COMMENT :
          case MARKUP_DD :
          case MARKUP_DL :
          case MARKUP_DT :
          case MARKUP_H1 :
          case MARKUP_H2 :
          case MARKUP_H3 :
          case MARKUP_H4 :
          case MARKUP_H5 :
          case MARKUP_H6 :
          case MARKUP_HEAD :
          case MARKUP_HR :
          case MARKUP_LI :
          case MARKUP_MAP :
          case MARKUP_OL :
          case MARKUP_P :
          case MARKUP_PRE :
          case MARKUP_TABLE :
          case MARKUP_TITLE :
          case MARKUP_TR :
          case MARKUP_UL :
          case MARKUP_DIR :
          case MARKUP_MENU :
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


//
// 'htmlWriteFile()' - Write an HTML markup tree to a file.
//

int				// O - Write status: 0 = success, -1 = fail
htmlWriteFile(tree_t *parent,	// I - Parent tree entry
              FILE   *fp)	// I - File to write to
{
  if (write_file(parent, fp, 0) < 0)
    return (-1);
  else
    return (0);
}


//
// 'htmlAddTree()' - Add a tree node to the parent.
//

tree_t *			// O - New entry
htmlAddTree(tree_t   *parent,	// I - Parent entry
            markup_t markup,	// I - Markup code
            uchar    *data)	// I - Data/text
{
  tree_t	*t;		// New tree entry


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
htmlDeleteTree(tree_t *parent)	// I - Parent to delete
{
  tree_t	*next;		// Next tree entry


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

tree_t *			// O - New entry
htmlInsertTree(tree_t   *parent,// I - Parent entry
               markup_t markup,	// I - Markup code
               uchar    *data)	// I - Data/text
{
  tree_t	*t;		// New tree entry


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
// 'htmlMapUnicode()' - Map a Unicode character to the custom character set.
//

uchar				// O - Charset character
htmlMapUnicode(int ch)		// I - Unicode character
{
  uchar	newch;			// New charset character


  // If we already have a mapping for this character, return it...
  if (_htmlCharacters[ch])
    return (_htmlCharacters[ch]);

  if (_htmlUTF8 >= 0x100)
  {
    progress_error(HD_ERROR_READ_ERROR, "Too many Unicode code points.");
    return (0);
  }

  newch = _htmlUTF8++;

  _htmlCharacters[ch] = (uchar)newch;
  _htmlUnicode[newch] = ch;
  _htmlGlyphs[newch]  = _htmlGlyphsAll[ch];

  for (int i = 0; i < TYPE_MAX; i ++)
    for (int j = 0; j < STYLE_MAX; j ++)
      _htmlWidths[i][j][newch] = _htmlWidthsAll[i][j][ch];

  return (newch);
}


//
// 'htmlNewTree()' - Create a new tree node for the parent.
//

tree_t *			// O - New entry
htmlNewTree(tree_t   *parent,	// I - Parent entry
            markup_t markup,	// I - Markup code
            uchar    *data)	// I - Data/text
{
  tree_t	*t;		// New tree entry


  // Allocate a new tree entry - use calloc() to get zeroed data...
  t = (tree_t *)calloc(sizeof(tree_t), 1);
  if (t == NULL)
    return (NULL);

  // Set the markup code and copy the data if necessary...
  t->markup = markup;
  if (data != NULL)
    t->data = (uchar *)hd_strdup((char *)data);

  // Set/copy font characteristics...
  if (parent == NULL)
  {
    t->halignment = ALIGN_LEFT;
    t->valignment = ALIGN_BOTTOM;
    t->typeface   = _htmlBodyFont;
    t->size       = SIZE_P;

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

  switch (t->markup)
  {
    case MARKUP_NONE :
    case MARKUP_IMG :
        // Figure out the width & height of this fragment...
        compute_size(t);
	break;

    case MARKUP_H1 :
    case MARKUP_H2 :
    case MARKUP_H3 :
    case MARKUP_H4 :
    case MARKUP_H5 :
    case MARKUP_H6 :
        get_alignment(t);

        t->typeface      = _htmlHeadingFont;
        t->size          = (unsigned)(SIZE_H1 - t->markup + MARKUP_H1);
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        t->style         = STYLE_BOLD;
        break;

    case MARKUP_P :
        get_alignment(t);

        t->typeface      = _htmlBodyFont;
        t->size          = SIZE_P;
        t->style         = STYLE_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 0;
        break;

    case MARKUP_PRE :
        t->typeface      = _htmlBodyFont >= TYPE_MONOSPACE ? TYPE_MONOSPACE
	                                                   : TYPE_COURIER;
        t->size          = SIZE_PRE;
        t->style         = STYLE_NORMAL;
        t->subscript     = 0;
        t->superscript   = 0;
        t->strikethrough = 0;
        t->preformatted  = 1;
        break;

    case MARKUP_DIV :
        get_alignment(t);
        break;

    case MARKUP_BLOCKQUOTE :
        t->style = STYLE_ITALIC;

    case MARKUP_UL :
    case MARKUP_DIR :
    case MARKUP_MENU :
    case MARKUP_OL :
    case MARKUP_DL :
        t->indent ++;
        break;

    case MARKUP_AREA :
    case MARKUP_BR :
    case MARKUP_COMMENT :
    case MARKUP_HR :
    case MARKUP_INPUT :
    case MARKUP_ISINDEX :
    case MARKUP_META :
    case MARKUP_WBR :
        break;

    case MARKUP_TH :
        t->style = STYLE_BOLD;
    case MARKUP_TD :
        get_alignment(t);
        break;

    case MARKUP_SUP :
        t->superscript = 1;
        t->size        = SIZE_P + SIZE_SUP;
        break;

    case MARKUP_SUB :
        t->subscript = 1;
        t->size      = SIZE_P + SIZE_SUB;
        break;

    case MARKUP_B :
        t->style = (style_t)(t->style | STYLE_BOLD);
        break;

    case MARKUP_DD :
        t->indent ++;
        break;

    case MARKUP_DT :
    case MARKUP_I :
        t->style = (style_t)(t->style | STYLE_ITALIC);
        break;

    case MARKUP_U :
    case MARKUP_INS :
        t->underline = 1;
        break;

    case MARKUP_STRIKE :
    case MARKUP_DEL :
        t->strikethrough = 1;
        break;

    default :
        break;
  }

  t->parent = parent;

  return (t);
}


//
// 'htmlGetText()' - Get all text from the given tree.
//

uchar *				// O - String containing text nodes
htmlGetText(tree_t *t)		// I - Tree to pick
{
  uchar		*s,		// String
		*s2,		// New string
		*tdata = NULL,	// Temporary string data
		*talloc = NULL;	// Allocated string data
  size_t	slen,		// Length of string
		tlen;		// Length of node string


  // Loop through all of the nodes in the tree and collect text...
  slen = 0;
  s    = NULL;

  while (t != NULL)
  {
    if (t->child)
      tdata = talloc = htmlGetText(t->child);
    else
      tdata = t->data;

    tlen = tdata ? strlen((char *)tdata) : 0;

    if (tdata != NULL && tlen > 0)
    {
      // Add the text to this string...
      if (s)
        s2 = (uchar *)realloc(s, 1 + slen + tlen);
      else
        s2 = (uchar *)malloc(1 + tlen);

      if (!s2)
        break;

      s = s2;

      memcpy((char *)s + slen, (char *)tdata, tlen);

      slen += tlen;
    }

    if (talloc)
    {
      free(talloc);
      talloc = NULL;
    }

    t = t->next;
  }

  if (slen)
    s[slen] = '\0';

  if (talloc)
    free(talloc);

  return (s);
}


//
// 'htmlGetMeta()' - Get document "meta" data...
//

uchar *				// O - Content string
htmlGetMeta(tree_t *tree,	// I - Document tree
            uchar  *name)	// I - Metadata name
{
  uchar	*tname,			// Name value from tree entry
	*tcontent;		// Content value from tree entry


  while (tree != NULL)
  {
    // Check this tree entry...
    if (tree->markup == MARKUP_META &&
        (tname = htmlGetVariable(tree, (uchar *)"NAME")) != NULL &&
        (tcontent = htmlGetVariable(tree, (uchar *)"CONTENT")) != NULL)
    {
      if (strcasecmp((char *)name, (char *)tname) == 0)
        return (tcontent);
    }
    else if (tree->markup == MARKUP_HTML && !strcasecmp((char *)name, "LANG") && (tcontent = htmlGetVariable(tree, (uchar *)"LANG")) != NULL)
    {
      return (tcontent);
    }

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
// 'htmlGetStyle()' - Get a style value from a node's STYLE attribute.
//

uchar *				// O - Value or NULL
htmlGetStyle(tree_t *t,		// I - Node
             uchar  *name)	// I - Name (including ":")
{
  uchar		*ptr,		// Pointer in STYLE attribute
		*bufptr;	// Pointer in buffer
  size_t	ptrlen,		// Length of STYLE attribute
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


//
// 'htmlGetVariable()' - Get a variable value from a markup entry.
//

uchar *				// O - Value or NULL if variable does not exist
htmlGetVariable(tree_t *t,	// I - Tree entry
                uchar  *name)	// I - Variable name
{
  var_t	*v,			// Matching variable
	key;			// Search key


  if (t == NULL || name == NULL || t->nvars == 0)
    return (NULL);

  key.name = name;

  v = (var_t *)bsearch(&key, t->vars, (size_t)t->nvars, sizeof(var_t), (compare_func_t)compare_variables);
  if (v == NULL)
    return (NULL);
  else if (v->value == NULL)
    return ((uchar *)"");
  else
    return (v->value);
}


//
// 'htmlLoadFontWidths()' - Load all of the font width files.
//

void
htmlLoadFontWidths(int typeface, int style)
{
  char		filename[1024];		// Filenames
  FILE		*fp;			// Files
  int		ch;			// Character
  float		width;			// Width value
  char		glyph[64];		// Glyph name
  char		line[1024];		// Line from AFM file


  // Now read all of the font widths...
  for (ch = 0; ch < 256; ch ++)
    _htmlWidths[typeface][style][ch] = 600;

  if (_htmlUTF8)
  {
    for (ch = 0; ch < 65536; ch ++)
      _htmlWidthsAll[typeface][style][ch] = 600;
  }

  snprintf(filename, sizeof(filename), "%s/fonts/%s.afm", _htmlData, _htmlFonts[typeface][style]);
  if ((fp = fopen(filename, "r")) == NULL)
  {
#ifndef DEBUG
    progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to open font width file %s!", filename);
#endif // !DEBUG
    return;
  }

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    if (strncmp(line, "C ", 2) != 0)
      continue;

    if (typeface < TYPE_SYMBOL)
    {
      // Handle encoding of regular fonts using assigned charset...
      if (sscanf(line, "%*s%*s%*s%*s%f%*s%*s%63s", &width, glyph) != 2)
	continue;

      for (ch = 0; ch < 256; ch ++)
      {
	if (_htmlGlyphs[ch] && !strcmp(_htmlGlyphs[ch], glyph))
	{
	  _htmlWidths[typeface][style][ch] = (short)width;
	  break;
	}
      }

      if (_htmlUTF8)
      {
	for (ch = 0; ch < 65536; ch ++)
	{
	  if (_htmlGlyphsAll[ch] && !strcmp(_htmlGlyphsAll[ch], glyph))
	  {
	    _htmlWidthsAll[typeface][style][ch] = (short)width;
	    break;
	  }
	}
      }
    }
    else
    {
      // Symbol and Dingbats fonts uses their own encoding...
      if (sscanf(line, "%*s%d%*s%*s%f", &ch, &width) != 2)
	continue;

      if (ch < 256 && ch >= 0)
      {
	_htmlWidths[typeface][style][ch]    = (short)width;
	_htmlWidthsAll[typeface][style][ch] = (short)width;
      }
    }
  }

  fclose(fp);

  // Make sure that non-breaking space has the same width as a breaking space...
  _htmlWidths[typeface][style][160]    = _htmlWidths[typeface][style][32];
  _htmlWidthsAll[typeface][style][160] = _htmlWidthsAll[typeface][style][32];

  _htmlWidthsLoaded[typeface][style] = 1;
}


//
// 'htmlSetVariable()' - Set a variable for a markup entry.
//

int				// O - Set status: 0 = success, -1 = fail
htmlSetVariable(tree_t *t,	// I - Tree entry
                uchar  *name,	// I - Variable name
                uchar  *value)	// I - Variable value
{
  var_t	*v,			// Matching variable
	key;			// Search key


  DEBUG_printf(("%shtmlSetVariable(%p, \"%s\", \"%s\")\n", indent, (void *)t, name,
                value ? (const char *)value : "(null)"));

  if (t->nvars == 0)
    v = NULL;
  else
  {
    key.name = name;

    v = (var_t *)bsearch(&key, t->vars, (size_t)t->nvars, sizeof(var_t), (compare_func_t)compare_variables);
  }

  if (v == NULL)
  {
    if (t->nvars == 0)
      v = (var_t *)malloc(sizeof(var_t));
    else
      v = (var_t *)realloc(t->vars, sizeof(var_t) * (size_t)(t->nvars + 1));

    if (v == NULL)
    {
      DEBUG_printf(("%s==== MALLOC/REALLOC FAILED! ====\n", indent));

      return (-1);
    }

    t->vars  = v;
    v        += t->nvars;
    t->nvars ++;
    v->name  = (uchar *)hd_strdup((char *)name);
    if (value != NULL)
      v->value = (uchar *)hd_strdup((char *)value);
    else
      v->value = NULL;

    if (strcasecmp((char *)name, "HREF") == 0)
    {
      DEBUG_printf(("%s---- Set link to %s ----\n", indent, value));
      t->link = t;
    }

    if (t->nvars > 1)
      qsort(t->vars, (size_t)t->nvars, sizeof(var_t), (compare_func_t)compare_variables);
  }
  else if (v->value != value)
  {
    v->value = (uchar *)hd_strdup((char *)value);
  }

  return (0);
}


//
// 'htmlSetBaseSize()' - Set the font sizes and spacings...
//

void
htmlSetBaseSize(double p,	// I - Point size of paragraph font
                double s)	// I - Spacing
{
  int	i;			// Looping var


  p /= 1.2 * 1.2 * 1.2;
  for (i = 0; i < 8; i ++, p *= 1.2)
  {
    _htmlSizes[i]    = p;
    _htmlSpacings[i] = p * s;
  }
}


//
// 'htmlSetCharSet()' - Set the character set for output.
//

void
htmlSetCharSet(const char *cs)		// I - Character set file to load
{
  int		i;			// Looping var
  char		filename[1024];		// Filenames
  FILE		*fp;			// Files
  int		ch, unicode;		// Character values
  char		glyph[64];		// Glyph name
  char		line[1024];		// Line from charset file
  int		chars[256];		// Character encoding array


  strlcpy(_htmlCharSet, cs, sizeof(_htmlCharSet));

  if (!_htmlInitialized)
  {
    // Load the PostScript glyph names for all of Unicode...
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
    else
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open psglyphs data file!");
#endif // !DEBUG
  }

  memset(_htmlGlyphs, 0, sizeof(_htmlGlyphs));

  if (!strcmp(cs, "utf-8"))
  {
    // Generate a dynamic mapping of Unicode to an 8-bit charset with the
    // bottom 128 characters matching US ASCII...
    _htmlUTF8 = 0x80;

    memset(_htmlCharacters, 0, sizeof(_htmlCharacters));

    for (i = 0; i < 128; i ++)
    {
      // Add the glyph to the charset array...
      _htmlGlyphs[i]  = _htmlGlyphsAll[i];
      _htmlUnicode[i] = i;
    }

    memset(_htmlWidthsLoaded, 0, sizeof(_htmlWidthsLoaded));
    return;
  }

  if (strncmp(cs, "8859-", 5) == 0)
    snprintf(filename, sizeof(filename), "%s/data/iso-%s", _htmlData, cs);
  else
    snprintf(filename, sizeof(filename), "%s/data/%s", _htmlData, cs);

  if ((fp = fopen(filename, "r")) == NULL)
  {
    // Can't open charset file; use ISO-8859-1...
#ifndef DEBUG
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open character set file %s!", cs);
#endif // !DEBUG

    for (i = 0; i < 256; i ++)
      chars[i] = i;
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
  {
    // Add the glyph to the charset array...
    if (chars[i] == 0)
    {
      _htmlGlyphs[i] = NULL;
      continue;
    }
    else
      _htmlGlyphs[i] = _htmlGlyphsAll[chars[i]];

    if (_htmlGlyphs[i])
      _htmlUnicode[i] = chars[i];
  }

  memset(_htmlWidthsLoaded, 0, sizeof(_htmlWidthsLoaded));
}


//
// 'htmlSetTextColor()' - Set the default text color.
//

void
htmlSetTextColor(uchar *color)	// I - Text color
{
  strlcpy((char *)_htmlTextColor, (char *)color, sizeof(_htmlTextColor));
}


//
// 'compare_variables()' - Compare two markup variables.
//

static int			// O - -1 if v0 < v1, 0 if v0 == v1, 1 if v0 > v1
compare_variables(var_t *v0,	// I - First variable
                  var_t *v1)	// I - Second variable
{
  return (strcasecmp((char *)v0->name, (char *)v1->name));
}


//
// 'compare_markups()' - Compare two markup strings...
//

static int			// O - -1 if m0 < m1, 0 if m0 == m1, 1 if m0 > m1
compare_markups(uchar **m0,	// I - First markup
                uchar **m1)	// I - Second markup
{
  if (tolower((*m0)[0]) == 'h' && isdigit((*m0)[1]) &&
      tolower((*m1)[0]) == 'h' && isdigit((*m1)[1]))
    return (atoi((char *)*m0 + 1) - atoi((char *)*m1 + 1));
  else
    return (strcasecmp((char *)*m0, (char *)*m1));
}


//
// 'delete_node()' - Free all memory associated with a node...
//

static void
delete_node(tree_t *t)		// I - Node to delete
{
  if (t == NULL)
    return;

  free(t->vars);
  free(t);
}


//
// 'insert_space()' - Insert a whitespace character before the
//                    specified node.
//

static void
insert_space(tree_t *parent,	// I - Parent node
             tree_t *t)		// I - Node to insert before
{
  tree_t	*space;		// Space node


  // Allocate memory for the whitespace...
  space = (tree_t *)calloc(sizeof(tree_t), 1);
  if (space == NULL)
  {
#ifndef DEBUG
    progress_error(HD_ERROR_OUT_OF_MEMORY,
                   "Unable to allocate memory for HTML tree node!");
#endif // !DEBUG
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
    space->size     = SIZE_P;
  }

  // Initialize element data...
  space->markup = MARKUP_NONE;
  space->data   = (uchar *)" ";

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

static int			// O - -1 on error, MARKUP_nnnn otherwise
parse_markup(tree_t *t,		// I - Current tree entry
             FILE   *fp,	// I - Input file
	     int    *linenum)	// O - Current line number
{
  int	ch, ch2;		// Characters from file
  uchar	markup[255],		// Markup string...
	*mptr,			// Current character...
	comment[10240],		// Comment string
	*cptr,			// Current char...
	**temp;			// Markup variable entry


  mptr = markup;

  while ((ch = getc(fp)) != EOF && mptr < (markup + sizeof(markup) - 1))
    if (ch == '>' || isspace(ch))
      break;
    else if (ch == '/' && mptr > markup)
    {
      // Look for "/>"...
      ch = getc(fp);

      if (ch != '>')
        return (MARKUP_ERROR);

      break;
    }
    else
    {
      if ((ch & 0x80) && _htmlUTF8)
      {
        // Collect UTF-8 value...
        ch = utf8_getc(ch, fp);
      }

      if (ch)
        *mptr++ = (uchar)ch;

      // Handle comments without whitespace...
      if ((mptr - markup) == 3 && strncmp((const char *)markup, "!--", 3) == 0)
      {
        ch = getc(fp);
        break;
      }
    }

  *mptr = '\0';

  if (ch == EOF)
    return (MARKUP_ERROR);

  mptr = markup;
  temp = (uchar **)bsearch(&mptr, _htmlMarkups,
                           sizeof(_htmlMarkups) / sizeof(_htmlMarkups[0]),
                           sizeof(_htmlMarkups[0]),
                           (compare_func_t)compare_markups);

  if (temp == NULL)
  {
    // Unrecognized markup stuff...
    t->markup = MARKUP_UNKNOWN;
    strlcpy((char *)comment, (char *)markup, sizeof(comment));
    cptr = comment + strlen((char *)comment);

    DEBUG_printf(("%s%s (unrecognized!)\n", indent, markup));
  }
  else
  {
    t->markup = (markup_t)((const char **)temp - _htmlMarkups);
    cptr      = comment;

    DEBUG_printf(("%s%s, line %d\n", indent, markup, *linenum));
  }

  if (t->markup == MARKUP_COMMENT || t->markup == MARKUP_UNKNOWN)
  {
    int lastch = ch;			// Last character seen

    while (ch != EOF && cptr < (comment + sizeof(comment) - 2))
    {
      if (ch == '>' && temp == NULL)
        break;

      if (ch == '\n')
        (*linenum) ++;

      if (ch == '-' && lastch == '-')
      {
        *cptr++ = (uchar)ch;

        if ((ch2 = getc(fp)) == '>')
	{
	  // Erase trailing -->
	  cptr -= 2;

          if (cptr < comment)
            cptr = comment; // Issue #316: buffer underflow
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
	      *eptr++ = (uchar)ch;

	  if (ch != ';')
	  {
	    ungetc(ch, fp);
	    ch = 0;
	  }

	  *eptr = '\0';
	  if (!ch)
	  {
	    progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d of %s.",
	                   *linenum, _htmlCurrentFile);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity, sizeof(comment) - (size_t)(cptr - comment));
	    cptr += strlen((char *)cptr);
	  }
	  else if ((ch = iso8859(entity)) == 0)
	  {
	    progress_error(HD_ERROR_HTML_ERROR,
	                   "Unknown character entity \"&%s;\" on line %d of %s.",
	                   entity, *linenum, _htmlCurrentFile);

            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = '&';
            strlcpy((char *)cptr, (char *)entity, sizeof(comment) - (size_t)(cptr - comment));
	    cptr += strlen((char *)cptr);
            if (cptr < (comment + sizeof(comment) - 1))
	      *cptr++ = ';';
	  }
	  else
	    *cptr++ = (uchar)ch;
	}
	else
        {
          if ((ch & 0x80) && _htmlUTF8)
          {
            // Collect UTF-8 value...
            ch = utf8_getc(ch, fp);
          }

          if (ch)
            *cptr++ = (uchar)ch;
        }

        lastch = ch;
        ch     = getc(fp);
      }
    }

    *cptr = '\0';
    t->data = (uchar *)hd_strdup((char *)comment);
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
          return (MARKUP_ERROR);

	break;
      }
    }
  }

  return (t->markup);
}


//
// 'parse_variable()' - Parse a markup variable string.
//

static int				// O - -1 on error, 0 on success
parse_variable(tree_t *t,		// I - Current tree entry
               FILE   *fp,		// I - Input file
	       int    *linenum)		// I - Current line number
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
    {
      if ((ch & 0x80) && _htmlUTF8)
      {
        // Collect UTF-8 value...
        ch = utf8_getc(ch, fp);
      }

      if (ch)
        *ptr++ = (uchar)ch;
    }

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
		  *eptr++ = (uchar)ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR,
		               "Unquoted & on line %d of %s.",
	                       *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR,
		               "Unknown character entity \"&%s;\" on line %d of %s.",
	                       entity, *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = (uchar)ch;
	    }
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
            {
              if ((ch & 0x80) && _htmlUTF8)
              {
                // Collect UTF-8 value...
                ch = utf8_getc(ch, fp);
              }

              if (ch)
                *ptr++ = (uchar)ch;
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
		  *eptr++ = (uchar)ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d of %s.",
	                       *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR,
		               "Unknown character entity \"&%s;\" on line %d of %s.",
	                       entity, *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = (uchar)ch;
	    }
            else if (ptr < (value + sizeof(value) - 1) &&
	             ch != '\n' && ch != '\r')
            {
              if ((ch & 0x80) && _htmlUTF8)
              {
                // Collect UTF-8 value...
                ch = utf8_getc(ch, fp);
              }

              if (ch)
                *ptr++ = (uchar)ch;
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
          *ptr++ = (uchar)ch;
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
		  *eptr++ = (uchar)ch;

              if (ch != ';')
	      {
	        ungetc(ch, fp);
		ch = 0;
	      }

              *eptr = '\0';
              if (!ch)
	      {
		progress_error(HD_ERROR_HTML_ERROR, "Unquoted & on line %d of %s.",
	                       *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
	      }
	      else if ((ch = iso8859(entity)) == 0)
	      {
		progress_error(HD_ERROR_HTML_ERROR,
		               "Unknown character entity \"&%s;\" on line %d of %s.",
	                       entity, *linenum, _htmlCurrentFile);

        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = '&';
                strlcpy((char *)ptr, (char *)entity, sizeof(value) - (size_t)(ptr - value));
		ptr += strlen((char *)ptr);
        	if (ptr < (value + sizeof(value) - 1))
		  *ptr++ = ';';
	      }
	      else if (ptr < (value + sizeof(value) - 1))
	        *ptr++ = (uchar)ch;
	    }
            else if (ptr < (value + sizeof(value) - 1))
            {
              if ((ch & 0x80) && _htmlUTF8)
              {
                // Collect UTF-8 value...
                ch = utf8_getc(ch, fp);
              }

              if (ch)
                *ptr++ = (uchar)ch;
            }
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


//
// 'compute_size()' - Compute the width and height of a tree entry.
//

static int			// O - 0 = success, -1 = failure
compute_size(tree_t *t)		// I - Tree entry
{
  uchar		*ptr;		// Current character
  float		width;		// Current width
  int		int_width;	// Integer width
  uchar		*width_ptr,	// Pointer to width string
		*height_ptr,	// Pointer to height string
		*size_ptr,	// Pointer to size string
		*type_ptr;	// Pointer to spacer type string
  image_t	*img;		// Image
  char		number[255];	// Width or height value


  if (!_htmlInitialized)
    htmlSetCharSet("iso-8859-1");

  if (t->markup == MARKUP_IMG)
  {
    width_ptr  = htmlGetVariable(t, (uchar *)"WIDTH");
    height_ptr = htmlGetVariable(t, (uchar *)"HEIGHT");

    img = image_load((char *)htmlGetVariable(t, (uchar *)"REALSRC"),
                     _htmlGrayscale);

    if (width_ptr != NULL && height_ptr != NULL)
    {
      t->width  = (float)(atoi((char *)width_ptr) / _htmlPPI * 72.0f);
      t->height = (float)(atoi((char *)height_ptr) / _htmlPPI * 72.0f);

      return (0);
    }

    if (img == NULL)
      return (-1);

    if (width_ptr != NULL)
    {
      t->width  = (float)(atoi((char *)width_ptr) / _htmlPPI * 72.0f);
      t->height = (float)(t->width * img->height / img->width);

      snprintf(number, sizeof(number), "%d", atoi((char *)width_ptr) * img->height / img->width);
      if (strchr((char *)width_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetVariable(t, (uchar *)"HEIGHT", (uchar *)number);
    }
    else if (height_ptr != NULL)
    {
      t->height = (float)(atoi((char *)height_ptr) / _htmlPPI * 72.0f);
      t->width  = (float)(t->height * img->width / img->height);

      snprintf(number, sizeof(number), "%d", atoi((char *)height_ptr) * img->width / img->height);
      if (strchr((char *)height_ptr, '%') != NULL)
        strlcat(number, "%", sizeof(number));
      htmlSetVariable(t, (uchar *)"WIDTH", (uchar *)number);
    }
    else
    {
      t->width  = (float)(img->width / _htmlPPI * 72.0f);
      t->height = (float)(img->height / _htmlPPI * 72.0f);

      snprintf(number, sizeof(number), "%d", img->width);
      htmlSetVariable(t, (uchar *)"WIDTH", (uchar *)number);

      snprintf(number, sizeof(number), "%d", img->height);
      htmlSetVariable(t, (uchar *)"HEIGHT", (uchar *)number);
    }

    return (0);
  }
  else if (t->markup == MARKUP_SPACER)
  {
    width_ptr  = htmlGetVariable(t, (uchar *)"WIDTH");
    height_ptr = htmlGetVariable(t, (uchar *)"HEIGHT");
    size_ptr   = htmlGetVariable(t, (uchar *)"SIZE");
    type_ptr   = htmlGetVariable(t, (uchar *)"TYPE");

    if (width_ptr != NULL)
      t->width = (float)(atoi((char *)width_ptr) / _htmlPPI * 72.0f);
    else if (size_ptr != NULL)
      t->width = (float)(atoi((char *)size_ptr) / _htmlPPI * 72.0f);
    else
      t->width = 1.0f;

    if (height_ptr != NULL)
      t->height = (float)(atoi((char *)height_ptr) / _htmlPPI * 72.0f);
    else if (size_ptr != NULL)
      t->height = (float)(atoi((char *)size_ptr) / _htmlPPI * 72.0f);
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
  else if (t->markup == MARKUP_BR)
  {
    t->width  = 0.0;
    t->height = (float)_htmlSizes[t->size];

    return (0);
  }
  else if (t->preformatted && t->data)
  {
    int		max_width = 0;		// Maximum width

    for (int_width = 0, ptr = t->data; *ptr != '\0'; ptr ++)
    {
      if (*ptr == '\n')
      {
        if (int_width > max_width)
          max_width = int_width;

	int_width = 0;
      }
      else if (*ptr == '\t')
        int_width = (int_width + 7) & ~7;
      else
        int_width ++;
    }

    if (int_width > max_width)
      max_width = int_width;

    if (!_htmlWidthsLoaded[t->typeface][t->style])
      htmlLoadFontWidths(t->typeface, t->style);

    width = _htmlWidths[t->typeface][t->style][0x20] * max_width * 0.001f;
  }
  else if (t->data)
  {
    if (!_htmlWidthsLoaded[t->typeface][t->style])
      htmlLoadFontWidths(t->typeface, t->style);

    for (int_width = 0, ptr = t->data; *ptr != '\0'; ptr ++)
      int_width += _htmlWidths[t->typeface][t->style][(int)*ptr & 255];

    width = 0.001f * int_width;
  }
  else
    width = 0.0f;

  t->width  = (float)(width * _htmlSizes[t->size]);
  t->height = (float)_htmlSizes[t->size];

  DEBUG_printf(("%swidth = %.1f, height = %.1f\n", indent, t->width, t->height));

  return (0);
}


//
// 'compute_color()' - Compute the red, green, blue color from the given
//                     string.
//

static int
compute_color(tree_t *t,	// I - Tree entry
              uchar  *color)	// I - Color string
{
  float	rgb[3];			// RGB color


  get_color(color, rgb);

  t->red   = (uchar)(rgb[0] * 255.0f + 0.5f);
  t->green = (uchar)(rgb[1] * 255.0f + 0.5f);
  t->blue  = (uchar)(rgb[2] * 255.0f + 0.5f);

  return (0);
}


//
// 'get_alignment()' - Get horizontal & vertical alignment values.
//

static int			// O - 0 for success, -1 for failure
get_alignment(tree_t *t)	// I - Tree entry
{
  uchar	*align;			// Alignment string


  if ((align = htmlGetVariable(t, (uchar *)"ALIGN")) == NULL)
    align = htmlGetStyle(t, (uchar *)"text-align");

  if (align != NULL)
  {
    if (!strcasecmp((char *)align, "left"))
      t->halignment = ALIGN_LEFT;
    else if (!strcasecmp((char *)align, "center"))
      t->halignment = ALIGN_CENTER;
    else if (!strcasecmp((char *)align, "right"))
      t->halignment = ALIGN_RIGHT;
    else if (!strcasecmp((char *)align, "justify"))
      t->halignment = ALIGN_JUSTIFY;
    else if (!strcasecmp((char *)align, "top"))
      t->valignment = ALIGN_TOP;
    else if (!strcasecmp((char *)align, "middle") ||
             !strcasecmp((char *)align, "absmiddle"))
      t->valignment = ALIGN_MIDDLE;
    else if (!strcasecmp((char *)align, "bottom"))
      t->valignment = ALIGN_BOTTOM;
  }

  if ((align = htmlGetVariable(t, (uchar *)"VALIGN")) == NULL)
    align = htmlGetStyle(t, (uchar *)"vertical-align");

  if (align != NULL)
  {
    if (!strcasecmp((char *)align, "top"))
      t->valignment = ALIGN_TOP;
    else if (!strcasecmp((char *)align, "middle"))
      t->valignment = ALIGN_MIDDLE;
    else if (!strcasecmp((char *)align, "center"))
      t->valignment = ALIGN_MIDDLE;
    else if (!strcasecmp((char *)align, "bottom"))
      t->valignment = ALIGN_BOTTOM;
  }

  return (0);
}


//
// 'fix_filename()' - Fix a filename to be relative to the base directory.
//

static const char *			// O - Fixed filename
fix_filename(char *filename,		// I - Original filename
             char *base)		// I - Base directory
{
  char		*slash;			// Location of slash
  char		*tempptr;		// Pointer into filename
  static char	temp[1024];		// Temporary filename
  static char	newfilename[1024];	// New filename


//  printf("fix_filename(filename=\"%s\", base=\"%s\")\n", filename, base);

  if (filename == NULL)
    return (NULL);

#ifdef DEBUG // to silence Clang static analyzer, totally unnecessary
  memset(temp, 0, sizeof(temp));
#endif // DEBUG

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
	    *tempptr = (char)((filename[0] - '0') << 4);
	  else
	    *tempptr = (char)((tolower(filename[0]) - 'a' + 10) << 4);

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
    return (file_find(Path, filename));

  if (strncmp(filename, "./", 2) == 0 ||
      strncmp(filename, ".\\", 2) == 0)
    filename += 2;

  if (strncmp(base, "http://", 7) == 0 || strncmp(base, "https://", 8) == 0)
  {
    // Base is a URL...
    char	scheme[32],		// URI scheme
		userpass[256],		// Username:password
		host[256],		// Hostname or IP address
		resource[256];		// Resource path
    int		port;			// Port number

    httpSeparateURI(HTTP_URI_CODING_ALL, base, scheme, sizeof(scheme), userpass, sizeof(userpass), host, sizeof(host), &port, resource, sizeof(resource));

    if (filename[0] == '/')
    {
      // Absolute path, so just use the server...
      httpAssembleURI(HTTP_URI_CODING_ALL, newfilename, sizeof(newfilename), scheme, userpass, host, port, filename);
    }
    else
    {
      // Handle "../" in filename...
      while (!strncmp(filename, "../", 3))
      {
	// Strip one level of directory in the resource
	filename += 3;

	if ((slash = strrchr(resource, '/')) != NULL)
	  *slash = '\0';
      }

      // Combine the resource and remaining relative filename to make a URL...
      httpAssembleURIf(HTTP_URI_CODING_ALL, newfilename, sizeof(newfilename), scheme, userpass, host, port, "%s/%s", resource, filename);
    }
  }
  else
  {
    // Base is a filename...
    if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
	base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
    {
      // No change needed for absolute path...
      return (file_find(Path, filename));
    }

    strlcpy(newfilename, base, sizeof(newfilename));
    base = newfilename;

#if defined(WIN32) || defined(__EMX__)
    while (!strncmp(filename, "../", 3) || !strncmp(filename, "..\\", 3))
#else
    while (!strncmp(filename, "../", 3))
#endif // WIN32 || __EMX__
    {
      filename += 3;
#if defined(WIN32) || defined(__EMX__)
      if ((slash = strrchr(base, '/')) != NULL)
        *slash = '\0';
      else if ((slash = strrchr(base, '\\')) != NULL)
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
  }

//  printf("    newfilename=\"%s\"\n", newfilename);

  return (file_find(Path, newfilename));
}


//
// 'html_memory_used()' - Figure out the amount of memory that was used.
//

static int				// O - Bytes used
html_memory_used(tree_t *t)		// I - Tree node
{
  int	bytes;				// Bytes used


  if (t == NULL)
    return (0);

  bytes = 0;

  while (t != NULL)
  {
    bytes += sizeof(tree_t);
    bytes += (size_t)t->nvars * sizeof(var_t);

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
               tree_t     *t)		// I - Document root node
{
  const char	*debug;			// HTMLDOC_DEBUG env var


  if ((debug = getenv("HTMLDOC_DEBUG")) == NULL ||
      (strstr(debug, "all") == NULL && strstr(debug, "memory") == NULL))
    return;

  progress_error(HD_ERROR_NONE, "DEBUG: %s = %d kbytes", title,
                 (html_memory_used(t) + 1023) / 1024);
}


//
// 'htmlFindFile()' - Find a file in the document.
//

tree_t *				// O - Node for file
htmlFindFile(tree_t *doc,		// I - Document pointer
             uchar  *filename)		// I - Filename
{
  tree_t	*tree;			// Current node
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
htmlFixLinks(tree_t *doc,		// I - Top node
             tree_t *tree,		// I - Current node
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

    if (show_debug)
      progress_error(HD_ERROR_NONE, "DEBUG: Updating links in document.");
  }

  DEBUG_printf(("htmlFixLinks: base=\"%s\"\n", (char *)base));

  while (tree)
  {
    if (tree->markup == MARKUP_A && base && base[0] &&
        (href = htmlGetVariable(tree, (uchar *)"HREF")) != NULL)
    {
      // Check if the link needs to be localized...
      DEBUG_printf(("htmlFixLinks: href=\"%s\", file_method(href)=\"%s\", file_method(base)=\"%s\"\\n", (char *)href, file_method((char *)href), file_method((char *)base)));

      if (href[0] != '#' && file_method((char *)href) == NULL)
      {
        // Yes, localize it...
        DEBUG_puts("htmlFixLinks: Localizing");

	if (href[0] == '/')
	{
	  // Absolute URL, just copy scheme, server, etc.
	  char *ptr;			// Pointer into URL...

          DEBUG_puts("htmlFixLinks: Absolute");

	  strlcpy(full_href, (char *)base, sizeof(full_href));

          if (href[1] == '/')
	  {
	    // Just use scheme...
	    if ((ptr = strstr(full_href, "//")) != NULL)
	      *ptr ='\0';
	  }
	  else if ((ptr = strstr(full_href, "//")) != NULL && (ptr = strchr(ptr + 2, '/')) != NULL)
	    *ptr ='\0';

	  strlcat(full_href, (char *)href, sizeof(full_href));
	}
	else if (!strncmp((char *)href, "./", 2))
	{
	  // Relative URL of the form "./foo/bar", append href sans
	  // "./" to base to form full href...
          DEBUG_puts("htmlFixLinks: Current directory");

	  snprintf(full_href, sizeof(full_href), "%s/%s", base, href + 2);
	}
	else if (!strncmp((char *)href, "../", 3))
	{
	  // Relative URL of the form "../foo/bar", append href sans
	  // "../" to parent to form full href...
	  char parent[1024], *pptr;	// Parent directory

	  strlcpy(parent, (char *)base, sizeof(parent));
          while (!strncmp((char *)href, "../", 3))
          {
            href += 3;

	    if ((pptr = strrchr(parent, '/')) != NULL)
	      pptr[1] = '\0';
	    else
	      parent[0] = '\0';
	  }

          DEBUG_printf(("htmlFixLinks: Subdirectory, parent=\"%s\"\n", parent));

	  snprintf(full_href, sizeof(full_href), "%s%s", parent, href);
	}
	else
	{
	  // Relative URL, append href to base to form full href...
          DEBUG_puts("htmlFixLinks: Relative");

          if (strcmp((char *)base, "."))
	    snprintf(full_href, sizeof(full_href), "%s/%s", (char *)base, (char *)href);
	  else
	    strlcpy(full_href, (char *)href, sizeof(full_href));
	}

        if (show_debug)
          progress_error(HD_ERROR_NONE, "DEBUG: Mapping \"%s\" to \"%s\"...", href, full_href);

	htmlSetVariable(tree, (uchar *)"_HD_FULL_HREF", (uchar *)full_href);
      }
      else
      {
        // No, just mirror the link in the _HD_FULL_HREF attribute...
	htmlSetVariable(tree, (uchar *)"_HD_FULL_HREF", href);
      }
    }
    else if (tree->markup == MARKUP_FILE)
      base = htmlGetVariable(tree, (uchar *)"_HD_BASE");

    if (tree->child)
      htmlFixLinks(doc, tree->child, base);

    tree = tree->next;
  }
}


//
// 'utf8_getc()' - Get a UTF-8 encoded character.
//

static int                              // O - Unicode equivalent
utf8_getc(int  ch,                      // I - Initial character
          FILE *fp)                     // I - File to read from
{
  int  ch2 = -1, ch3 = -1;              // Temporary characters


  if ((ch & 0xe0) == 0xc0)
  {
    // Two-byte sequence for 0x80 to 0x7ff...
    ch  = (ch & 0x1f) << 6;
    ch2 = getc(fp);

    if ((ch2 & 0xc0) == 0x80)
      ch |= ch2 & 0x3f;
    else
      goto bad_sequence;
  }
  else if ((ch & 0xf0) == 0xe0)
  {
    // Three-byte sequence from 0x800 to 0xffff...
    ch  = (ch & 0x0f) << 12;
    ch2 = getc(fp);

    if ((ch2 & 0xc0) == 0x80)
      ch |= (ch2 & 0x3f) << 6;
    else
      goto bad_sequence;

    ch3 = getc(fp);

    if ((ch3 & 0xc0) == 0x80)
      ch |= ch3 & 0x3f;
    else
      goto bad_sequence;
  }
  else if (ch & 0x80)
    goto bad_sequence;

  if (ch == 0xfeff)
  {
    // BOMs are invalid in UTF-8 text, but Microsoft insists on still using
    // them...  Try reading another character...
    //
    // TODO: Emit a warning about this...
    return (utf8_getc(getc(fp), fp));
  }

  return (htmlMapUnicode(ch));

  bad_sequence:

  if (ch3 >= 0)
    progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X %02X %02X.", ch, ch2, ch3);
  else if (ch2 >= 0)
    progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X %02X.", ch, ch2);
  else
    progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X.", ch);

  return (0);
}

