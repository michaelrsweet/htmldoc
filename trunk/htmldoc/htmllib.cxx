/*
 * "$Id: htmllib.cxx,v 1.2 1999/11/08 22:11:35 mike Exp $"
 *
 *   HTML parsing routines for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
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
 *   get_text()          - Get all text from the given tree.
 *   htmlGetText()       - Get all text from the given tree.
 *   htmlGetVariable()   - Get a variable value from a markup entry.
 *   htmlSetVariable()   - Set a variable for a markup entry.
 *   htmlSetBaseSize()   - Set the font sizes and spacings...
 *   compare_variables() - Compare two markup variables.
 *   compare_markups()   - Compare two markup strings...
 *   parse_markup()      - Parse a markup string.
 *   parse_variable()    - Parse a markup variable string.
 *   fix_filename()      - Fix a filename to be relative to the base directory.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <ctype.h>



/*
 * Markup strings...
 */

char		*_htmlMarkups[] =
		{
		  "",		/* MARKUP_NONE */
		  "!--",	/* MARKUP_COMMENT */
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

const char	*_htmlData = "..";	/* Data directory */
float		_htmlPPI = 80.0f;	/* Image resolution */
int		_htmlGrayscale = 0;	/* Grayscale output? */
float		_htmlSizes[8] =		/* Point size for each HTML size */
		{ 6.0f, 8.0f, 9.0f, 11.0f, 14.0f, 17.0f, 20.0f, 24.0f };
float		_htmlSpacings[8] =	/* Line height for each HTML size */
		{ 7.2f, 9.6f, 10.8f, 13.2f, 16.8f, 20.4f, 24.0f, 28.8f };
typeface_t	_htmlBodyFont = TYPE_TIMES,
		_htmlHeadingFont = TYPE_HELVETICA;

int		_htmlInitialized = 0;	/* Initialized glyphs yet? */
char		_htmlCharset[256] = "";	/* Character set name */
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

static int	write_file(tree_t *t, FILE *fp, int col);
static int	compare_variables(var_t *v0, var_t *v1);
static int	compare_markups(uchar **m0, uchar **m1);
static int	parse_markup(tree_t *t, FILE *fp);
static int	parse_variable(tree_t *t, FILE *fp);
static int	compute_size(tree_t *t);
static int	compute_color(tree_t *t, uchar *color);
static int	get_alignment(tree_t *t);
static char	*fix_filename(char *path, char *base);

#define issuper(x)	((x) == MARKUP_CENTER || (x) == MARKUP_DIV ||\
			 (x) == MARKUP_BLOCKQUOTE)
#define isblock(x)	((x) == MARKUP_ADDRESS || \
			 (x) == MARKUP_P || (x) == MARKUP_PRE ||\
			 ((x) >= MARKUP_H1 && (x) <= MARKUP_H7) ||\
			 (x) == MARKUP_HR || (x) == MARKUP_TABLE)
#define islist(x)	((x) == MARKUP_DL || (x) == MARKUP_OL ||\
			 (x) == MARKUP_UL)
#define istable(x)	((x) == MARKUP_TBODY || (x) == MARKUP_THEAD ||\
			 (x) == MARKUP_TFOOT || (x) == MARKUP_TR)
#define istentry(x)	((x) == MARKUP_TD || (x) == MARKUP_TH)

#ifdef DEBUG
static uchar	indent[255] = "";
#endif /* DEBUG */


/*
 * 'htmlReadFile()' - Read a file for HTML markup codes.
 */

tree_t *			/* O - Pointer to top of file tree */
htmlReadFile(tree_t *parent,	/* I - Parent tree entry */
             FILE   *fp,	/* I - File pointer */
	     char   *base)	/* I - Base directory for file */
{
  int		ch,		/* Character from file */
		have_whitespace;/* Non-zero if there was leading whitespace */
  static uchar	s[10240];	/* String from file */
  uchar		*ptr,		/* Pointer in string */
		glyph[16],	/* Glyph name (&#nnn; or &name;) */
		*glyphptr;	/* Pointer in glyph string */
  tree_t	*tree,		/* "top" of this tree */
		*t,		/* New tree entry */
		*prev,		/* Previous tree entry */
		*temp;		/* Temporary looping var */
  int		pos;		/* Current file position */
  FILE		*embed;		/* File pointer for EMBED */
  char		newbase[1024];	/* New base directory for EMBED */
  uchar		*filename,	/* Filename for EMBED tag */
		*face,		/* Typeface for FONT tag */
		*color,		/* Color for FONT tag */
		*size;		/* Size for FONT tag */
  int		sizeval;	/* Size value from FONT tag */
  unsigned	halignment;	/* Saved horizontal alignment for tables. */


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

    have_whitespace = 0;

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

    t = (tree_t *)calloc(sizeof(tree_t), 1);
    if (t == NULL)
      break;

   /*
    * Set/copy font characteristics...
    */

    if (parent == NULL)
    {
      t->halignment   = ALIGN_LEFT;
      t->valignment   = ALIGN_MIDDLE;
      t->typeface     = _htmlBodyFont;
      t->size         = SIZE_P;
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
      if (ch == ' ')
      {
       /*
        * Illegal lone "<"!  Ignore it...
	*/

	free(t);
	continue;
      }
      
      if (ch != '/')
        ungetc(ch, fp);

      if (parse_markup(t, fp) == MARKUP_ERROR)
      {
        free(t);
        break;
      }

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
          if (temp->markup == t->markup)
            break;
      }
      else if (t->markup == MARKUP_DT ||
               t->markup == MARKUP_DD)
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (temp->markup == MARKUP_DT || temp->markup == MARKUP_DD)
            break;
      }
      else if (issuper(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (issuper(temp->markup))
            break;
	  else if (istentry(temp->markup))
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
	  else if (istentry(temp->markup) || issuper(temp->markup))
	  {
	    temp = NULL;
            break;
	  }

      }
      else if (isblock(t->markup))
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (isblock(temp->markup))
            break;
	  else if (istentry(temp->markup) || islist(temp->markup) ||
	           issuper(temp->markup))
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
	  else if (temp->markup == MARKUP_TABLE)
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
	  else if (temp->markup == MARKUP_TABLE || istable(temp->markup))
	  {
	    temp = NULL;
            break;
	  }
      }
      else if (t->markup == MARKUP_LI)
      {
        for (temp = parent; temp != NULL; temp = temp->parent)
          if (temp->markup == MARKUP_LI)
	    break;
	  else if (islist(temp->markup) || issuper(temp->markup))
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
        free(t);

       /*
        * If the markup doesn't start with a slash, or if it does but
        * doesn't match up with the parent (i.e. <UL><LI>...<LI>...</UL>)
        * then seek back so the parent entry gets a copy...
        */

        if (ch != '/' || temp != parent)
          fseek(fp, pos, SEEK_SET);	/* Make sure parent gets this markup... */
        break;
      }
      else if (ch == '/')
      {
        free(t);
	continue;
      }
    }
    else if (t->preformatted)
    {
     /*
      * Read a pre-formatted string into the current tree entry...
      */

      ptr = s;
      while (ch != '<' && ch != EOF)
      {
        if (ch == '&')
        {
          for (glyphptr = glyph;
               (ch = getc(fp)) != EOF && (glyphptr - glyph) < 15;
               glyphptr ++)
            if (!isalnum(ch))
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

      t->markup = MARKUP_NONE;
      t->data   = (uchar *)strdup((char *)s);

      DEBUG_printf(("%sfragment %s\n", indent, s));
    }
    else
    {
     /*
      * Read the next string fragment...
      */

      ptr = s;
      if (have_whitespace && prev != NULL)
        *ptr++ = ' ';

      while (!isspace(ch) && ch != '<' && ch != EOF)
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

      t->markup = MARKUP_NONE;
      t->data   = (uchar *)strdup((char *)s);

      DEBUG_printf(("%sfragment %s\n", indent, s));
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
    * Do markup-specific stuff...
    */

    switch (t->markup)
    {
      case MARKUP_IMG :
         /*
	  * Update the image source as necessary...
	  */

          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	    htmlSetVariable(t, (uchar *)"SRC",
	                    (uchar *)fix_filename((char *)filename, base));

      case MARKUP_NONE :
	 /*
	  * Figure out the width & height of this markup...
	  */

          compute_size(t);
	  break;

      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
          get_alignment(t);

          t->typeface      = _htmlHeadingFont;
          t->size          = SIZE_H1 - t->markup + MARKUP_H1;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 0;
          t->style         = STYLE_BOLD;
          t->child         = htmlReadFile(t, fp, base);
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
          t->child         = htmlReadFile(t, fp, base);
          break;

      case MARKUP_PRE :
          t->typeface      = TYPE_COURIER;
          t->size          = SIZE_PRE;
          t->style         = STYLE_NORMAL;
          t->subscript     = 0;
          t->superscript   = 0;
          t->strikethrough = 0;
          t->preformatted  = 1;
          t->child         = htmlReadFile(t, fp, base);
          break;

      case MARKUP_BLOCKQUOTE :
          t->style = STYLE_ITALIC;

      case MARKUP_UL :
      case MARKUP_OL :
      case MARKUP_DL :
          t->indent ++;

          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_DIV :
          get_alignment(t);
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_HR :
          t->halignment = ALIGN_CENTER;
          get_alignment(t);
          break;

      case MARKUP_DOCTYPE :
      case MARKUP_AREA :
      case MARKUP_BR :
      case MARKUP_COMMENT :
      case MARKUP_INPUT :
      case MARKUP_ISINDEX :
      case MARKUP_LINK :
      case MARKUP_META :
      case MARKUP_WBR :
          break;

      case MARKUP_EMBED :
          if ((filename = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
	  {
	    filename = (uchar *)fix_filename((char *)filename, base);

            if ((embed = fopen((char *)filename, "r")) != NULL)
            {
	      strcpy(newbase, file_directory((char *)filename));

              t->child = htmlReadFile(t, embed, newbase);
              fclose(embed);
            }
	  }
          break;

      case MARKUP_TH :
          get_alignment(t);
          t->style = STYLE_BOLD;
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_TD :
          get_alignment(t);
          t->style = STYLE_NORMAL;
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_FONT :
          if ((face = htmlGetVariable(t, (uchar *)"FACE")) != NULL)
          {
            for (ptr = face; *ptr != '\0'; ptr ++)
              *ptr = tolower(*ptr);

            if (strstr((char *)face, "arial") != NULL ||
                strstr((char *)face, "helvetica") != NULL)
              t->typeface = TYPE_HELVETICA;
            else if (strstr((char *)face, "times") != NULL)
              t->typeface = TYPE_TIMES;
            else if (strstr((char *)face, "courier") != NULL)
              t->typeface = TYPE_COURIER;
            else if (strstr((char *)face, "symbol") != NULL)
              t->typeface = TYPE_SYMBOL;
          }

          if ((color = htmlGetVariable(t, (uchar *)"COLOR")) != NULL)
            compute_color(t, color);

          if ((size = htmlGetVariable(t, (uchar *)"SIZE")) != NULL)
          {
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

          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_BIG :
          if (t->size < 6)
            t->size += 2;
          else
            t->size = 7;

          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_SMALL :
          if (t->size > 2)
            t->size -= 2;
          else
            t->size = 0;

          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_SUP :
          t->superscript = 1;
          t->size        = SIZE_SUP;
          t->child       = htmlReadFile(t, fp, base);
          break;

      case MARKUP_SUB :
          t->subscript = 1;
          t->size      = SIZE_SUB;
          t->child     = htmlReadFile(t, fp, base);
          break;

      case MARKUP_KBD :
          t->style    = STYLE_BOLD;

      case MARKUP_TT :
      case MARKUP_CODE :
      case MARKUP_SAMP :
          t->size --;
          t->typeface = TYPE_COURIER;
          t->child    = htmlReadFile(t, fp, base);
          break;

      case MARKUP_B :
          t->style |= STYLE_BOLD;
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_DD :
          t->indent ++;
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_VAR :
          t->style    |= STYLE_ITALIC;
      case MARKUP_DFN :
          t->typeface = TYPE_HELVETICA;
          t->child    = htmlReadFile(t, fp, base);
          break;

      case MARKUP_STRONG :
          t->style |= STYLE_BOLD;
      case MARKUP_CITE :
      case MARKUP_DT :
      case MARKUP_EM :
      case MARKUP_I :
          t->style |= STYLE_ITALIC;
          t->child = htmlReadFile(t, fp, base);
          break;

      case MARKUP_U :
      case MARKUP_INS :
          t->underline = 1;
          t->child     = htmlReadFile(t, fp, base);
          break;

      case MARKUP_STRIKE :
      case MARKUP_DEL :
          t->strikethrough = 1;
          t->child         = htmlReadFile(t, fp, base);
          break;

      case MARKUP_CENTER :
          t->halignment = ALIGN_CENTER;
          t->child      = htmlReadFile(t, fp, base);
          break;

      case MARKUP_TABLE :
         /*
          * Tables have the overall table alignment and the cell alignment...
          */

          halignment    = t->halignment;
	  t->halignment = ALIGN_LEFT;
          get_alignment(t);

          t->child      = htmlReadFile(t, fp, base);
	  t->halignment = halignment;
          break;

      default :
         /*
          * All other markup types should be using <MARK>...</MARK>
          */

          get_alignment(t);

          t->child = htmlReadFile(t, fp, base);
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
write_file(tree_t *t,		/* I - Tree entry */
           FILE   *fp,		/* I - File to write to */
           int    col)		/* I - Current column */
{
  int	i;			/* Looping var */
  uchar	*ptr;			/* Character pointer */


  while (t != NULL)
  {
    if (t->markup == MARKUP_NONE)
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
        case MARKUP_H7 :
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
            if (col > 0)
            {
              putc('\n', fp);
              col = 0;
            }
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
	else if (strchr((char *)t->vars[i].value, ' ') != NULL ||
        	 strchr((char *)t->vars[i].value, '\t') != NULL ||
        	 strchr((char *)t->vars[i].value, '\n') != NULL ||
        	 strchr((char *)t->vars[i].value, '\r') != NULL)
          col += fprintf(fp, "%s=\"%s\"", t->vars[i].name, t->vars[i].value);
	else
          col += fprintf(fp, "%s=%s", t->vars[i].name, t->vars[i].value);
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
          case MARKUP_H7 :
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
              putc('\n', fp);
              col = 0;
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
htmlWriteFile(tree_t *parent,	/* I - Parent tree entry */
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

tree_t *			/* O - New entry */
htmlAddTree(tree_t   *parent,	/* I - Parent entry */
            markup_t markup,	/* I - Markup code */
            uchar    *data)	/* I - Data/text */
{
  tree_t	*t;		/* New tree entry */


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
htmlDeleteTree(tree_t *parent)	/* I - Parent to delete */
{
  int		i;		/* Looping var */
  var_t		*var;		/* Current variable */
  tree_t	*next;		/* Next tree entry */


  if (parent == NULL)
    return (-1);

  while (parent != NULL)
  {
    next = parent->next;

    if (parent->child != NULL)
      if (htmlDeleteTree(parent->child))
        return (-1);

    if (parent->data != NULL)
      free(parent->data);

    for (i = 0, var = parent->vars; i < parent->nvars; i ++, var ++)
    {
      free(var->name);
      if (var->value != NULL)
        free(var->value);
    }

    if (parent->vars != NULL)
      free(parent->vars);

    free(parent);

    parent = next;
  }

  return (0);
}


/*
 * 'htmlInsertTree()' - Insert a tree node to the parent.
 */

tree_t *			/* O - New entry */
htmlInsertTree(tree_t   *parent,/* I - Parent entry */
               markup_t markup,	/* I - Markup code */
               uchar    *data)	/* I - Data/text */
{
  tree_t	*t;		/* New tree entry */


  if ((t = htmlNewTree(parent, markup, data)) == NULL)
    return (NULL);

 /*
  * Insert the tree entry to the end of the chain of children...
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

tree_t *			/* O - New entry */
htmlNewTree(tree_t   *parent,	/* I - Parent entry */
            markup_t markup,	/* I - Markup code */
            uchar    *data)	/* I - Data/text */
{
  tree_t	*t;		/* New tree entry */


 /*
  * Allocate a new tree entry - use calloc() to get zeroed data...
  */

  t = (tree_t *)calloc(sizeof(tree_t), 1);
  if (t == NULL)
    return (NULL);

 /*
  * Set the markup code and copy the data if necessary...
  */

  t->markup = markup;
  if (data != NULL)
    t->data = (uchar *)strdup((char *)data);

 /*
  * Set/copy font characteristics...
  */

  if (parent == NULL)
  {
    t->halignment = ALIGN_LEFT;
    t->valignment = ALIGN_MIDDLE;
    t->typeface   = _htmlBodyFont;
    t->size       = SIZE_P;
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
       /*
	* Figure out the width & height of this fragment...
	*/

        compute_size(t);
	break;

    case MARKUP_H1 :
    case MARKUP_H2 :
    case MARKUP_H3 :
    case MARKUP_H4 :
    case MARKUP_H5 :
    case MARKUP_H6 :
    case MARKUP_H7 :
        get_alignment(t);

        t->typeface      = _htmlHeadingFont;
        t->size          = SIZE_H1 - t->markup + MARKUP_H1;
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
        t->typeface      = TYPE_COURIER;
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
        t->size        = SIZE_SUP;
        break;

    case MARKUP_SUB :
        t->subscript = 1;
        t->size      = SIZE_SUB;
        break;

    case MARKUP_B :
        t->style |= STYLE_BOLD;
        break;

    case MARKUP_DD :
        t->indent ++;
        break;

    case MARKUP_DT :
    case MARKUP_I :
        t->style |= STYLE_ITALIC;
        break;

    case MARKUP_U :
    case MARKUP_INS :
        t->underline = 1;
        break;

    case MARKUP_STRIKE :
    case MARKUP_DEL :
        t->strikethrough = 1;
        break;
  }

  t->parent = parent;

  return (t);
}


/*
 * 'get_text()' - Get all text from the given tree.
 */

static uchar *		/* O - Pointer to last char set */
get_text(tree_t *tree,	/* I - Tree to pick */
         uchar  *buf)	/* I - Buffer to store text in */
{
  while (tree != NULL)
  {
    if (tree->child != NULL)
      buf = get_text(tree->child, buf);
    else if (tree->markup == MARKUP_NONE && tree->data != NULL)
    {
      strcpy((char *)buf, (char *)tree->data);
      buf += strlen((char *)buf);
    }
    else if (tree->markup == MARKUP_BR)
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

uchar *				/* O - String containing text nodes */
htmlGetText(tree_t *tree)	/* I - Tree to pick */
{
  uchar	buf[10240];		/* String buffer */


  buf[0] = '\0';
  get_text(tree, buf);

  return ((uchar *)strdup((char *)buf));
}


/*
 * 'htmlGetMeta()' - Get document "meta" data...
 */

uchar *				/* O - Content string */
htmlGetMeta(tree_t *tree,	/* I - Document tree */
            uchar  *name)	/* I - Metadata name */
{
  uchar	*tname,			/* Name value from tree entry */
	*tcontent;		/* Content value from tree entry */


  while (tree != NULL)
  {
   /*
    * Check this tree entry...
    */

    if (tree->markup == MARKUP_META &&
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
 * 'htmlGetVariable()' - Get a variable value from a markup entry.
 */

uchar *				/* O - Value or NULL if variable does not exist */
htmlGetVariable(tree_t *t,	/* I - Tree entry */
                uchar  *name)	/* I - Variable name */
{
  var_t	*v,			/* Matching variable */
	key;			/* Search key */


  if (t->nvars == 0)
    return (NULL);

  key.name = name;

  v = (var_t *)bsearch(&key, t->vars, t->nvars, sizeof(var_t),
                       (int (*)(const void *, const void *))compare_variables);
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
htmlSetVariable(tree_t *t,	/* I - Tree entry */
                uchar  *name,	/* I - Variable name */
                uchar  *value)	/* I - Variable value */
{
  var_t	*v,			/* Matching variable */
	key;			/* Search key */


  DEBUG_printf(("%shtmlSetVariable(%08x, \"%s\", \"%s\")\n", indent, t, name, value));

  if (t->nvars == 0)
    v = NULL;
  else
  {
    key.name = name;

    v = (var_t *)bsearch(&key, t->vars, t->nvars, sizeof(var_t),
        	         (int (*)(const void *, const void *))compare_variables);
  }

  if (v == NULL)
  {
    if (t->nvars == 0)
      t->vars = (var_t *)malloc(sizeof(var_t));
    else
      t->vars = (var_t *)realloc(t->vars, sizeof(var_t) * (t->nvars + 1));

    if (t->vars == NULL)
    {
      DEBUG_printf(("%s==== MALLOC/REALLOC FAILED! ====\n", indent));

      t->nvars = 0;
      return (-1);
    }

    v        = t->vars + t->nvars;
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
      qsort(t->vars, t->nvars, sizeof(var_t),
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


  if (strcmp(cs, _htmlCharset) == 0)
    return;

  strcpy(_htmlCharset, cs);

  if (!_htmlInitialized)
  {
   /*
    * Load the PostScript glyph names for all of Unicode...
    */

    memset(_htmlGlyphsAll, 0, sizeof(_htmlGlyphsAll));

    sprintf(line, "%s/data/psglyphs", _htmlData);
    if ((fp = fopen(line, "r")) != NULL)
    {
      while (fscanf(fp, "%x%63s", &unicode, glyph) == 2)
        _htmlGlyphsAll[unicode] = strdup(glyph);

      fclose(fp);
    }

    _htmlInitialized = 1;
  }

  memset(_htmlGlyphs, 0, sizeof(_htmlGlyphs));

  sprintf(filename, "%s/data/%s", _htmlData, cs);
  if ((fp = fopen(filename, "r")) == NULL)
  {
   /*
    * Can't open charset file; use ISO-8859-1...
    */

    for (i = 0; i < 256; i ++)
      chars[i] = i;
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

      sprintf(filename, "%s/afm/%s", _htmlData, _htmlFonts[i][j]);
      if ((fp = fopen(filename, "r")) == NULL)
        continue;

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
	      break;

          if (ch < 256)
	    _htmlWidths[i][j][ch] = width * 0.001f;
	}
	else
	{
	 /*
	  * Symbol font uses its own encoding...
	  */

          if (sscanf(line, "%*s%d%*s%*s%f", &ch, &width) != 2)
	    continue;

          if (ch < 256)
	    _htmlWidths[i][j][ch] = width * 0.001f;
	}
      }

      fclose(fp);
    }
}


/*
 * 'compare_variables()' - Compare two markup variables.
 */

static int			/* O - -1 if v0 < v1, 0 if v0 == v1, 1 if v0 > v1 */
compare_variables(var_t *v0,	/* I - First variable */
                  var_t *v1)	/* I - Second variable */
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
  return (strcasecmp((char *)*m0, (char *)*m1));
}


/*
 * 'parse_markup()' - Parse a markup string.
 */

static int			/* O - -1 on error, MARKUP_nnnn otherwise */
parse_markup(tree_t *t,		/* I - Current tree entry */
             FILE   *fp)	/* I - Input file */
{
  int	ch;			/* Character from file */
  uchar	markup[255],		/* Markup string... */
	*mptr,			/* Current character... */
	comment[10240],		/* Comment string */
	*cptr,			/* Current char... */
	**temp;			/* Markup variable entry */


  mptr = markup;

  while ((ch = getc(fp)) != EOF)
    if (ch == '>' || isspace(ch))
      break;
    else
      *mptr++ = ch;

  *mptr = '\0';

  if (ch == EOF)
    return (MARKUP_ERROR);

  mptr = markup;
  temp = (uchar **)bsearch(&mptr, _htmlMarkups,
                           sizeof(_htmlMarkups) / sizeof(_htmlMarkups[0]),
                           sizeof(_htmlMarkups[0]),
                           (int (*)(const void *, const void *))compare_markups);

  if (temp == NULL)
  {
   /*
    * Unrecognized markup stuff...
    */

    t->markup = MARKUP_COMMENT;
    strcpy((char *)comment, (char *)markup);
    cptr = comment + strlen((char *)comment);

    DEBUG_printf(("%s%s (unrecognized!)\n", indent, markup));
  }
  else
  {
    t->markup = (markup_t)((char **)temp - _htmlMarkups);
    cptr      = comment;

    DEBUG_printf(("%s%s\n", indent, markup));
  }

  if (t->markup == MARKUP_COMMENT)
  {
    while (ch != EOF && ch != '>')
    {
      *cptr++ = ch;
      ch = getc(fp);
    }

    *cptr = '\0';
    t->data = (uchar *)strdup((char *)comment);
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

  return (t->markup);
}


/*
 * 'parse_variable()' - Parse a markup variable string.
 */

static int			/* O - -1 on error, 0 on success */
parse_variable(tree_t *t,	/* I - Current tree entry */
               FILE   *fp)	/* I - Input file */
{
  uchar	name[1024],		/* Name of variable */
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
    case '>' :
        ungetc(ch, fp);
        return (htmlSetVariable(t, name, NULL));
    case EOF :
        return (-1);
    default : /* '=' */
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

        return (htmlSetVariable(t, name, value));
  }
}


/*
 * 'compute_size()' - Compute the width and height of a tree entry.
 */

static int			/* O - 0 = success, -1 = failure */
compute_size(tree_t *t)		/* I - Tree entry */
{
  uchar		*ptr;		/* Current character */
  float		width,		/* Current width */
		max_width;	/* Maximum width */
  uchar		*width_ptr,	/* Pointer to width string */
		*height_ptr;	/* Pointer to height string */
  image_t	*img;		/* Image */


  if (!_htmlInitialized)
    htmlSetCharSet("8859-1");

  if (t->markup == MARKUP_IMG)
  {
    width_ptr  = htmlGetVariable(t, (uchar *)"WIDTH");
    height_ptr = htmlGetVariable(t, (uchar *)"HEIGHT");

    if (width_ptr != NULL && height_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlPPI * 72.0f;
      t->height = atoi((char *)height_ptr) / _htmlPPI * 72.0f;

      return (0);
    }

    img = image_load((char *)htmlGetVariable(t, (uchar *)"SRC"),
                     _htmlGrayscale);

    if (img == NULL)
      return (-1);

    if (width_ptr != NULL)
    {
      t->width  = atoi((char *)width_ptr) / _htmlPPI * 72.0f;
      t->height = t->width * img->height / img->width;
    }
    else if (height_ptr != NULL)
    {
      t->height = atoi((char *)height_ptr) / _htmlPPI * 72.0f;
      t->width  = t->height * img->width / img->height;
    }
    else
    {
      t->width  = img->width / _htmlPPI * 72.0f;
      t->height = img->height / _htmlPPI * 72.0f;
    }

    return (0);
  }
  else if (t->preformatted)
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
        width += _htmlWidths[t->typeface][t->style][*ptr];

   if (width < max_width)
     width = max_width;
  }
  else
    for (width = 0.0, ptr = t->data; *ptr != '\0'; ptr ++)
      width += _htmlWidths[t->typeface][t->style][*ptr];

  t->width  = width * _htmlSizes[t->size];
  t->height = _htmlSizes[t->size];

  return (0);
}


/*
 * 'compute_color()' - Compute the red, green, blue color from the given
 *                     string.
 */

static int
compute_color(tree_t *t,	/* I - Tree entry */
              uchar  *color)	/* I - Color string */
{
  int		i;		/* Looping vars */
  static struct
  {
    char		*name;	/* Color name */
    uchar		red,	/* Red value */
			green,	/* Green value */
			blue;	/* Blue value */
  }		colors[] =	/* Color "database" */
  {
    { "black",		0,   0,   0 },
    { "red",		255, 0,   0 },
    { "green",		0,   255, 0 },
    { "yellow",		255, 255, 0 },
    { "blue",		0,   0,   255 },
    { "magenta",	255, 0,   255 },
    { "cyan",		0,   255, 255 },
    { "white",		255, 255, 255 }
  };


  if (color[0] == '#')
  {
   /*
    * RGB value in hex...
    */

    i = strtol((char *)color + 1, NULL, 16);

    t->red   = i >> 16;
    t->green = (i >> 8) & 255;
    t->blue  = i & 255;

    return (0);
  }

  for (i = 0; i < (sizeof(colors) / sizeof(colors[0])); i ++)
    if (strcasecmp(colors[i].name, (char *)color) == 0)
    {
      t->red   = colors[i].red;
      t->green = colors[i].green;
      t->blue  = colors[i].blue;
      return (0);
    }

  return (-1);
}


/*
 * 'get_alignment()' - Get horizontal & vertical alignment values.
 */

static int			/* O - 0 for success, -1 for failure */
get_alignment(tree_t *t)	/* I - Tree entry */
{
  uchar	*align;			/* Alignment string */


  if ((align = htmlGetVariable(t, (uchar *)"ALIGN")) != NULL)
  {
    if (strcasecmp((char *)align, "left") == 0)
      t->halignment = ALIGN_LEFT;
    else if (strcasecmp((char *)align, "center") == 0)
      t->halignment = ALIGN_CENTER;
    else if (strcasecmp((char *)align, "right") == 0)
      t->halignment = ALIGN_RIGHT;
  }

  if ((align = htmlGetVariable(t, (uchar *)"VALIGN")) != NULL)
  {
    if (strcasecmp((char *)align, "top") == 0)
      t->valignment = ALIGN_TOP;
    else if (strcasecmp((char *)align, "middle") == 0)
      t->valignment = ALIGN_MIDDLE;
    else if (strcasecmp((char *)align, "bottom") == 0)
      t->valignment = ALIGN_BOTTOM;
  }

  return (0);
}


/*
 * 'fix_filename()' - Fix a filename to be relative to the base directory.
 */

static char *				/* O - Fixed filename */
fix_filename(char *filename,		/* I - Original filename */
             char *base)		/* I - Base directory */
{
  char		*slash;			/* Location of slash */
  static char	newfilename[1024];	/* New filename */


  if (filename == NULL)
    return (NULL);

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

  if (filename[0] == '/' || filename[0] == '\\' || base == NULL ||
      base[0] == '\0' || (isalpha(filename[0]) && filename[1] == ':'))
    return (filename);		/* No change needed for absolute path */

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
  }

#ifdef MAC
  strcat(newfilename, ":");
#else
  strcat(newfilename, "/");
#endif // MAC
  strcat(newfilename, filename);

  return (newfilename);
}


/*
 * End of "$Id: htmllib.cxx,v 1.2 1999/11/08 22:11:35 mike Exp $".
 */
