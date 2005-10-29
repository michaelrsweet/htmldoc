//
// "$Id$"
//
//   Basic stylesheet routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2005 by Easy Software Products.
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
//       Hollywood, Maryland 20636 USA
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
#include "hdstring.h"
//#define DEBUG


//
// 'hdStyleSheet::hdStyleSheet()' - Create a new stylesheet.
//

hdStyleSheet::hdStyleSheet()
{
  num_styles   = 0;
  alloc_styles = 0;
  styles       = NULL;

  memset(max_selectors, 0, sizeof(max_selectors));
  memset(elements, -1, sizeof(elements));

  num_fonts = 0;
  memset(fonts, 0, sizeof(fonts));
  memset(font_names, 0, sizeof(font_names));

  charset       = NULL;
  encoding      = HD_FONT_ENCODING_8BIT;
  num_glyphs    = 0;
  glyphs        = NULL;
  grayscale     = false;
  ppi           = 80.0f;
  browser_width = 680.0f;
  private_id    = 0;

  // Set default style stuff...
  set_color("#000000");
  set_font_family("serif");
  set_font_size("11pt");
  set_line_height("1.2");

  def_style.font_style          = HD_FONT_STYLE_NORMAL;
  def_style.font_variant        = HD_FONT_VARIANT_NORMAL;
  def_style.font_weight         = HD_FONT_WEIGHT_NORMAL;
  def_style.list_style_position = HD_LIST_STYLE_POSITION_OUTSIDE;
  def_style.list_style_type     = HD_LIST_STYLE_TYPE_NONE;
  def_style.orphans             = 2;
  def_style.text_align          = HD_TEXT_ALIGN_LEFT;
  def_style.text_decoration     = HD_TEXT_DECORATION_NONE;
  def_style.text_transform      = HD_TEXT_TRANSFORM_NONE;
  def_style.widows              = 2;

  // Set the default character set to "iso-8859-1"...
  set_charset("iso-8859-1");
}


//
// 'hdStyleSheet::~hdStyleSheet()' - Destroy a stylesheet.
//

hdStyleSheet::~hdStyleSheet()
{
  int		i, j;		// Looping vars
  hdStyle	**s;		// Current style


  // Free all styles...
  if (alloc_styles)
  {
    for (i = num_styles, s = styles; i > 0; i --, s ++)
      delete *s;

    delete[] styles;
  }

  // Free all fonts...
  for (i = 0; i < num_fonts; i ++)
  {
    for (j = 0; j < HD_FONT_INTERNAL_MAX; j ++)
      if (fonts[i][j])
        delete fonts[i][j];

    if (font_names[i])
      free(font_names[i]);
  }

  // Free all glyphs...
  if (charset)
    free(charset);

  if (num_glyphs)
  {
    for (i = 0; i < num_glyphs; i ++)
      if (glyphs[i])
	free(glyphs[i]);

    delete[] glyphs;
  }
}


//
// 'hdStyleSheet::add_style()' - Add a style to a stylesheet.
//

void
hdStyleSheet::add_style(hdStyle *s)	// I - New style
{
  int		i, j, k;		// Looping vars
  hdStyle	**temp;			// New style pointer array
  hdElement	e, e2;			// Elements for new style...


#ifdef DEBUG
  printf("add_style(%p): %s", s, hdTree::elements[s->selectors[0].element]);
  if  (s->selectors[0].pseudo)
    printf(":%s\n", s->selectors[0].pseudo);
  else
    putchar('\n');
#endif // DEBUG

  // Allocate more memory as needed...
  if (num_styles >= alloc_styles)
  {
    temp = new hdStyle *[alloc_styles + 32];

    if (alloc_styles)
    {
      memcpy(temp, styles, alloc_styles * sizeof(hdStyle *));
      delete[] styles;
    }

    alloc_styles += 32;
    styles       = temp;
  }

  // Cache the primary selector element...
  e = s->selectors[0].element;

  // Find where to insert the style...
  if (elements[e] >= 0)
    i = elements[e];	 // Already added this element to the table...
  else if (num_styles)
  {
    // Do a binary search for a group of styles for this element...
    for (i = 0, j = num_styles - 1; i <= j;)
    {
      // Check the element against the left/right styles...
      if (e <= styles[i]->selectors[0].element)
        break; // Insert before left style...

      if (e > styles[j]->selectors[0].element)
      {
        // Insert after right style...
	i = j + 1;
	break;
      }

      // Check the midpoint...
      k  = (i + j) / 2;
      e2 = styles[k]->selectors[0].element;

      if (e < e2)
        j = k - 1;
      else if (e > e2)
        i = k + 1;
      else
      {
        // The midpoint is the right index...
        i = k;
        break;
      }
    }

    elements[e] = i;
  }
  else
    i = elements[e] = 0;

  // Now do the insert...
#ifdef DEBUG
  printf("    inserting at %d, num_styles = %d\n", i, num_styles);
#endif // DEBUG

  if (i < num_styles)
    memmove(styles + i + 1, styles + i, (num_styles - i) * sizeof(hdStyle *));

  styles[i] = s;

  num_styles ++;

  // And update any indices in the elements array...
  for (j = 0; j < HD_ELEMENT_MAX; j ++)
    if (elements[j] >= i && j != e)
      elements[j] ++;

  // Finally, update the max selectors value for this element, to make
  // lookups faster...
  if (s->num_selectors > max_selectors[e])
    max_selectors[e] = s->num_selectors;

#ifdef DEBUG
  printf("    max_selectors = %d\n", max_selectors[e]);
#endif // DEBUG
}


//
// 'hdStyleSheet::find_font()' - Find a font for the given style.
//

hdStyleFont *				// O - Font record
hdStyleSheet::find_font(hdStyle *s)	// I - Style record
{
  hdFontInternal	fs;		// Font style index


  // Figure out the font style we need...
  if (s->font_weight == HD_FONT_WEIGHT_BOLD)
  {
    if (s->font_style >= HD_FONT_STYLE_ITALIC)
      fs = HD_FONT_INTERNAL_BOLD_ITALIC;
    else
      fs = HD_FONT_INTERNAL_BOLD;
  }
  else if (s->font_style >= HD_FONT_STYLE_ITALIC)
    fs = HD_FONT_INTERNAL_ITALIC;
  else
    fs = HD_FONT_INTERNAL_NORMAL;

  return (find_font(s->font_family, fs));
}


//
// 'hdStyleSheet::find_font()' - Find a specific font.
//

hdStyleFont *				// O - Font record
hdStyleSheet::find_font(
  const char     *family,		// I - Font family
  hdFontInternal fs)			// I - Internal font style
{
  char		face[1024],		// Font face property
		*start,			// Start of current font face
		*ptr;			// End of current font face
  int		i;			// Looping var
  int		tf;			// Typeface index
  hdStyleFont	*temp;			// New font record


  // Make a copy of the font family...
  if (family)
  {
    strncpy(face, family, sizeof(face) - 1);
    face[sizeof(face) - 1] = '\0';
  }
  else
    strcpy(face, "Times");

  // Loop until we find a matching font...
  for (ptr = face; *ptr;)
  {
    // Find the next face name...
    start = ptr;

    while (*ptr && !isspace(*ptr) && *ptr != ',')
      ptr ++;

    // Nul-terminate the name as needed, then do lookup...
    while (isspace(*ptr) || *ptr == ',')
      *ptr++ = '\0';

    // See if the font face exists already...
    if (!strcasecmp(start, "monospace"))
      tf = HD_FONT_FACE_MONOSPACE;
    else if (!strcasecmp(start, "Courier"))
      tf = HD_FONT_FACE_COURIER;
    else if (!strcasecmp(start, "serif"))
      tf = HD_FONT_FACE_SERIF;
    else if (!strcasecmp(start, "Times"))
      tf = HD_FONT_FACE_TIMES;
    else if (!strcasecmp(start, "sans-serif"))
      tf = HD_FONT_FACE_SANS_SERIF;
    else if (!strcasecmp(start, "Arial") ||
             !strcasecmp(start, "Helvetica"))
      tf = HD_FONT_FACE_HELVETICA;
    else if (!strcasecmp(start, "symbol"))
      tf = HD_FONT_FACE_SYMBOL;
    else if (!strcasecmp(start, "dingbats"))
      tf = HD_FONT_FACE_DINGBATS;
    else if (!strcasecmp(start, "cursive") ||
             !strcasecmp(start, "ZapfChancery"))
      tf = HD_FONT_FACE_CURSIVE;
    else
    {
      // Add a custom font...
      for (tf = HD_FONT_FACE_CUSTOM; tf < HD_FONT_FACE_MAX; tf ++)
        if (font_names[tf] == NULL)
	  break;
	else if (!strcasecmp(start, font_names[tf]))
	  break;

      if (tf >= HD_FONT_FACE_MAX)
        tf = HD_FONT_FACE_SERIF;
    }

    // Return the existing font, if any...
    if (fonts[tf][fs])
      return (fonts[tf][fs]);

    // Try loading a new font...
    for (i = 0; i < 4; i ++)
    {
      // Load the font...
      temp = new hdStyleFont(this, (hdFontFace)tf, (hdFontInternal)i, start);

      // See if we were able to load it...
      if (temp->ps_name == NULL)
      {
        delete temp;

	for (i --; i >= 0; i --)
	{
	  delete fonts[tf][i];
	  fonts[tf][i] = NULL;
	}

	break;
      }
      else
        fonts[tf][i] = temp;
    }

    if (fonts[tf][fs])
    {
      // Font was loaded, set the name and return the font...
      font_names[tf] = strdup(start);

      if (tf >= num_fonts)
        num_fonts = tf + 1;

      return (fonts[tf][fs]);
    }
  }

  // Couldn't find font, return 0...
  return ((hdStyleFont *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                selectors.
//

hdStyle *					// O - Style record
hdStyleSheet::find_style(int             nsels,	// I - Number of selectors
                         hdStyleSelector *sels,	// I - Selectors
			 int             exact)	// I - Exact match required?
{
  int		i, j;				// Looping vars
  hdStyle	*s,				// Current style
		*best;				// Best match
  int		score,				// Current score
		best_score;			// Best match score
  hdElement	e;				// Top-level element


  // Check quickly to see if we have any style info for this element...
  e = sels[0].element;

  if (elements[e] < 0)
    return ((hdStyle *)0);

  // Now loop through the styles for this element to find the best match...
  for (i = elements[e], best = NULL, best_score = 0;
       i < num_styles && styles[i]->selectors[0].element == e;
       i ++)
  {
    s = styles[i];

    if (exact && nsels != s->num_selectors)
      continue;

    // Don't use private styles...
    if (s->num_selectors == 1 && s->selectors[0].id &&
        !strncmp(s->selectors[0].id, "_HD_", 4))
      continue;

    for (j = 0, score = 0; j < nsels && j < s->num_selectors; j ++, score <<= 2)
    {
      // Check the element...
      if (sels[j].element != s->selectors[j].element &&
          s->selectors[j].element != HD_ELEMENT_NONE)
      {
        // An element mismatch is an instant no-match...
        score = 0;
	break;
      }

      // Check the class name...
      if ((sels[j].class_ != NULL) == (s->selectors[j].class_ != NULL) &&
          (sels[j].class_ == NULL ||
	   strcasecmp(sels[j].class_, s->selectors[j].class_) == 0))
	score ++;

      // Check the pseudo name...
      if ((sels[j].pseudo != NULL) == (s->selectors[j].pseudo != NULL) &&
          (sels[j].pseudo == NULL ||
	   strcasecmp(sels[j].pseudo, s->selectors[j].pseudo) == 0))
	score ++;
      else
      {
        score = 0;
	break;
      }

      // Check the id...
      if ((sels[j].id != NULL) == (s->selectors[j].id != NULL) &&
          (sels[j].id == NULL ||
	   strcasecmp(sels[j].id, s->selectors[j].id) == 0))
	score ++;

      if (exact && (score & 3) != 3)
      {
        // No exact match...
        score = 0;
	break;
      }
    }

    // Now update the best match if we get a better score...
    if (score > best_score)
    {
      best_score = score;
      best       = s;
    }
  }

#ifdef DEBUG
  if (sels[0].element == HD_ELEMENT_A)
  {
    if (best)
      printf("find_style: A:%s matched %s:%s...\n", sels[0].pseudo,
             get_element(best->selectors[0].element),
	     best->selectors[0].pseudo);
      printf("find_style: A:%s did not match...\n", sels[0].pseudo);
  }
#endif // DEBUG

  // Return the best match...
  return (best);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                element.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdElement e,	// I - Element
                         const char *c,	// I - Class name, if any
                         const char *i,	// I - ID, if any
                         const char *p)	// I - Pseudo target, if any
{
  hdStyleSelector	sel;		// Selector...


  // Build the selector for this node...
  sel.element = e;
  sel.class_  = (char *)c;
  sel.id      = (char *)i;
  sel.pseudo  = (char *)p;

  // Do the search...
  return (find_style(1, &sel));
}


static const char * const hd_elements[] =
{
  "",		/* HD_ELEMENT_NONE */
  "!--",	/* HD_ELEMENT_COMMENT */
  "!DOCTYPE",
  "a",
  "abbr",
  "acronym",
  "address",
  "applet",
  "area",
  "b",
  "base",
  "basefont",
  "bdo",
  "big",
  "blink",
  "blockquote",
  "body",
  "br",
  "button",
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
  "fieldset",
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
  "iframe",
  "img",
  "input",
  "ins",
  "isindex",
  "kbd",
  "label",
  "legend",
  "li",
  "link",
  "map",
  "menu",
  "meta",
  "multicol",
  "nobr",
  "noframes",
  "object",
  "ol",
  "optgroup",
  "option",
  "p",
  "param",
  "pre",
  "q",
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


//
// 'hd_compare_elements()' - Compare two element strings...
//

static int				// O - Result of comparison
hd_compare_elements(const char **m0,	// I - First element
                    const char **m1)	// I - Second element
{
  if (tolower((*m0)[0]) == 'h' && isdigit((*m0)[1]) &&
      tolower((*m1)[0]) == 'h' && isdigit((*m1)[1]))
    return (atoi(*m0 + 1) - atoi(*m1 + 1));
  else
    return (strcasecmp(*m0, *m1));
}


//
// 'hdStyleSheet::get_element()' - Get the enumeration for an element.
//

hdElement				// O - Element enumeration
hdStyleSheet::get_element(const char *s)// I - Element name
{
  const char	**temp;			// Element name array


  temp = (const char **)bsearch(&s, hd_elements,
                        	sizeof(hd_elements) / sizeof(hd_elements[0]),
                        	sizeof(hd_elements[0]),
                        	(hdCompareFunc)hd_compare_elements);

  if (temp == NULL)
    return (HD_ELEMENT_UNKNOWN);
  else
    return ((hdElement)(temp - hd_elements));
}


//
// 'hdStyleSheet::get_element()' - Get the element string for an enumeration.
//

const char *				// O - Element name
hdStyleSheet::get_element(hdElement e)	// I - Element enumeration
{
  return (hd_elements[e]);
}


//
// 'hdStyleSheet::get_glyph()' - Find the index for the named glyph...
//

int					// O - Glyph index or -1 if not found
hdStyleSheet::get_glyph(const char *s)	// I - Glyph name
{
  int	i;				// Looping var


  // Do a brute-force search for the glyph name...
  for (i = 0; i < num_glyphs; i ++)
    if (glyphs[i] && strcmp(glyphs[i], s) == 0)
      return (i);

  // Didn't find the glyph, so return -1...
  return (-1);
}


//
// 'hdStyleSheet::load()' - Load a stylesheet from the given file.
//

bool					// O - True on success, false on failure
hdStyleSheet::load(FILE       *f,	// I - File to read from
                   const char *path)	// I - Search path for included files
{
  int			i, j;		// Looping vars...
  bool			status;		// Load status
  int			ch;		// Character from file
  char			sel_s[1024],	// Selector string
			sel_p[256],	// Selector pattern
			*sel_class,	// Selector class
			*sel_pseudo,	// Selector pseudo-target
			*sel_id;	// Selector ID
  int			cur_fstyle,	// Current style
			num_fstyles,	// Number of styles to create
			num_selectors[HD_SELECTOR_MAX];
					// Number of selectors for each style
  hdStyleSelector	*selectors[HD_SELECTOR_MAX],
  					// Selectors for each style
			parent;		// Parent style
  hdStyle		*style;		// New style
  char			props[4096],	// Style properties
			props_p[256];	// Property pattern
  char			import[1024],	// Import string
			*import_ptr;	// Pointer into import string
  FILE			*import_f;	// Import file pointer
  char			cssmedia[256];	// Current media type


  // Initialize the read patterns.
  pattern("a-zA-Z0-9@.:#", sel_p);
  pattern("~}", props_p);

  // Loop until we can't read any more...
  cur_fstyle  = 0;
  num_fstyles = 0;
  status      = true;
  cssmedia[0] = '\0';

  while ((ch = getc(f)) != EOF)
  {
    // Skip whitespace...
    if (isspace(ch))
      continue;

    if (ch == '}')
    {
      cssmedia[0] = '\0';
      continue;
    }

    if (ch == '/')
    {
      // Check for C-style comment...
      if ((ch = getc(f)) != '*')
      {
        progress_error(HD_ERROR_CSS_ERROR,
	               "Bad sequence \"/%c\" in stylesheet!", ch);
	status = false;
	break;
      }

      // OK, now read chars until EOF or "*/"...
      while ((ch = getc(f)) != EOF)
        if (ch == '*')
	{
	  if ((ch = getc(f)) == '/')
	    break;
	  else
	    ungetc(ch, f);
	}

      if (ch != '/')
      {
        progress_error(HD_ERROR_CSS_ERROR,
	               "Unterminated comment in stylesheet!");
	status = false;
	break;
      }

      continue;
    }
    else if (ch == '{')
    {
      // Handle grouping for rendering intent...
      if (num_fstyles == 0)
        continue;

      // Read property data...
      if (read(f, props_p, props, sizeof(props)) == NULL)
      {
        progress_error(HD_ERROR_CSS_ERROR,
	               "Missing property data in stylesheet!");
	status = false;
	break;
      }

      while ((ch = getc(f)) >= 0)
        if (!isspace(ch))
	  break;

      if (ch != '}')
      {
        ungetc(ch, f);
	progress_error(HD_ERROR_CSS_ERROR,
	               "Missing } for style properties!");
      }

      // Apply properties to all styles...
#ifdef DEBUG
      printf("num_styles = %d\n", num_styles);
#endif // DEBUG

      for (i = 0; i < num_fstyles; i ++)
      {
        // Force link pseudo-selectors to element "A".
        if (selectors[i]->element == HD_ELEMENT_NONE &&
	    selectors[i]->pseudo &&
	    (!strcmp(selectors[i]->pseudo, "link") ||
	     !strcmp(selectors[i]->pseudo, "active") ||
	     !strcmp(selectors[i]->pseudo, "visited") ||
	     !strcmp(selectors[i]->pseudo, "hover")))
	  selectors[i]->element = HD_ELEMENT_A;

        if (selectors[i]->element == HD_ELEMENT_NONE)
	{
	  // Create an instance of this style for each element...
	  for (j = HD_ELEMENT_A; j < HD_ELEMENT_MAX; j ++)
	  {
	    selectors[i]->element = (hdElement)j;
            parent.element        = selectors[i]->element;

            if ((style = find_style(num_selectors[i], selectors[i], 1)) == NULL)
            {
	      style = new hdStyle(num_selectors[i], selectors[i],
	                	  find_style(1, &parent, 1));
              add_style(style);
	    }

            status = status && style->load(this, props);
	  }

	  selectors[i]->element = HD_ELEMENT_NONE;
	}
	else
	{
	  // Apply to just the selected element...
          if ((style = find_style(num_selectors[i], selectors[i], 1)) == NULL)
          {
	    if (selectors[i]->class_ || selectors[i]->pseudo ||
	        selectors[i]->id)
            {
	      parent.element = selectors[i]->element;

	      style = new hdStyle(num_selectors[i], selectors[i],
	                	  find_style(1, &parent, 1));
            }
	    else
	      style = new hdStyle(num_selectors[i], selectors[i], &def_style);

            add_style(style);
	  }

          status = status && style->load(this, props);
        }

        delete[] selectors[i];
      }

      // Reset to beginning...
      cur_fstyle  = 0;
      num_fstyles = 0;

      continue;
    }
    else if (ch == ',')
    {
      // Add another selector...
      cur_fstyle ++;

      if (cur_fstyle >= HD_SELECTOR_MAX)
      {
        progress_error(HD_ERROR_CSS_ERROR,
	               "Too many selectors (> %d) in stylesheet!",
	               HD_SELECTOR_MAX);
	status = false;
	break;
      }

      continue;
    }
    else if (!sel_p[ch])
    {
      // Not a valid selector string...
      progress_error(HD_ERROR_CSS_ERROR,
                     "Bad stylesheet character \"%c\"!", ch);
      status = false;
      break;
    }

    // Read the selector string...
    ungetc(ch, f);

    read(f, sel_p, sel_s, sizeof(sel_s));

    // OK, got a selector, see if it is @foo...
    if (sel_s[0] == '@')
    {
      // @ selector...
      if (strcmp(sel_s, "@import") == 0)
      {
        // Include another stylesheet...
	import_ptr = import;

	while ((ch = getc(f)) >= 0)
	  if (!isspace(ch))
	    break;

        ungetc(ch, f);

        // Read the URL and potentially a media selector...
	while ((ch = getc(f)) >= 0)
	{
	  // Stop at ';'...
	  if (ch == ';')
	    break;

          // Handle quotes...
	  if (import_ptr < (import + sizeof(import) - 1))
	    *import_ptr++ = ch;

	  if (ch == '\"')
	  {
	    while ((ch = getc(f)) >= 0)
	    {
	      if (import_ptr < (import + sizeof(import) - 1))
		*import_ptr++ = ch;

	      if (ch == '\"')
	        break;
            }
	  }
	}

        *import_ptr = '\0';

        // Skip the initial url(" or "...
	if (strncmp(import, "url(", 4) == 0)
	  import_ptr = import + 4;
	else
	  import_ptr = import;

	if (*import_ptr == '\"')
	  import_ptr ++;

        strcpy(import, import_ptr);
	if ((import_ptr = strchr(import, '\"')) == NULL)
	  import_ptr = strchr(import, ')');

	if (import_ptr)
	  *import_ptr++ = '\0';

        // Now see if there is a media selector...
        while (isspace(*import_ptr))
	  import_ptr ++;

        if (*import_ptr == ')')
	  import_ptr ++;

        while (isspace(*import_ptr))
	  import_ptr ++;

        if ((!*import_ptr && !cssmedia[0]) || strstr(import_ptr, "print") != NULL)
	{
	  // Import the file...
	  if ((import_f = fopen(import, "r")) != NULL)
	  {
	    load(import_f, path);

	    delete import_f;
	  }
	  else
	    progress_error(HD_ERROR_CSS_ERROR,
	                   "Unable to import \"%s\"!", import);
	}
      }
      else if (strcmp(sel_s, "@page") == 0)
      {
        // Set page parameters...
      }
      else if (strcmp(sel_s, "@media") == 0)
      {
        // Set parameters for a specific media type...
	read(f, sel_p, cssmedia, sizeof(cssmedia));

	if (strcmp(cssmedia, "print") == 0)
	  cssmedia[0] = '\0';

        // Read up to the first {...
	while ((ch = getc(f)) >= 0)
          if (!isspace(ch))
	    break;

	if (ch != '{')
	{
          ungetc(ch, f);
	  progress_error(HD_ERROR_CSS_ERROR,
	                 "Missing { for media selector!");
	}
      }
      else
      {
        // Show a warning message...
	progress_error(HD_ERROR_CSS_ERROR,
	               "Unsupported rule \"%s\"!", sel_s);

        int braces = 0;

	while ((ch = getc(f)) >= 0)
	{
	  if (ch == '\"')
	  {
	    // Skip quoted string...
	    while ((ch = getc(f)) >= 0)
	      if (ch == '\"')
	        break;
	  }
	  else if (ch == '{')
	    braces ++;
	  else if (ch == '}' && braces > 0)
	    braces --;
	  else if (ch == ';' && braces == 0)
	    break;
	}

        if (ch != ';')
	  progress_error(HD_ERROR_CSS_ERROR,
	                 "Missing terminator (;) for %s!", sel_s);
      }

      continue;
    }

    // Allocate memory for the selectors (up to HD_SELECTOR_MAX of them) as needed...
    if (cur_fstyle >= num_fstyles)
    {
      num_fstyles ++;
      num_selectors[cur_fstyle] = 0;
      selectors[cur_fstyle]     = new hdStyleSelector[HD_SELECTOR_MAX];
    }

    // Separate the selector string into its components...
    sel_class  = strchr(sel_s, '.');
    sel_pseudo = strchr(sel_s, ':');
    sel_id     = strchr(sel_s, '#');

    if (sel_class)
      *sel_class++ = '\0';

    if (sel_pseudo)
      *sel_pseudo++ = '\0';

    if (sel_id)
      *sel_id++ = '\0';

    if (get_element(sel_s) == HD_ELEMENT_UNKNOWN)
      printf("UNKNOWN ELEMENT %s!\n", sel_s);

    // Insert the new selector before any existing ones...
    if (num_selectors[cur_fstyle] > 0)
      memmove(selectors[cur_fstyle] + 1, selectors[cur_fstyle],
              num_selectors[cur_fstyle] * sizeof(hdStyleSelector));

    selectors[cur_fstyle]->set(get_element(sel_s),
                               sel_class, sel_pseudo, sel_id);

    num_selectors[cur_fstyle] ++;
  }

  // Clear any selectors still in memory...
  for (i = 0; i < num_fstyles; i ++)
  {
    for (j = 0; j < num_selectors[i]; j ++)
      selectors[i]->clear();

    delete[] selectors[i];
  }

  // Update all style data...
  update_styles();

  // Return the load status...
  return (status);
}


//
// 'hdStyleSheet::pattern()' - Initialize a regex pattern buffer...
//

void
hdStyleSheet::pattern(const char *r,	// I - Regular expression pattern
                      char       p[256])// O - Character lookup table
{
  int	s;				// Set state
  int	ch,				// Char for range
	end;				// Last char in range


  // The regex pattern string "r" can be any regex character pattern,
  // e.g.:
  //
  //    a-zA-Z      Matches all letters
  //    \-+.0-9      Matches all numbers, +, -, and .
  //    ~ \t\n      Matches anything except whitespace.
  //
  // A leading '~' inverts the logic, e.g. all characters *except*
  // those listed.  If you want to match the dash (-) then it must
  // appear be quoted (\-)...

  // Set the logic mode...
  if (*r == '~')
  {
    // Invert logic
    s = 0;
    r ++;
  }
  else
    s = 1;

  // Initialize the pattern buffer...
  memset(p, !s, 256);

  // Loop through the pattern string, updating the pattern buffer as needed.
  for (; *r; r ++)
  {
    if (*r == '\\')
    {
      // Handle quoted char...
      r ++;

      switch (*r)
      {
        case 'n' :
            ch = '\n';
	    break;

        case 'r' :
            ch = '\r';
	    break;

        case 't' :
            ch = '\t';
	    break;

        default :
            ch = *r;
	    break;
      }
    }
    else
      ch = *r;

    // Set this character...
    p[ch] = s;

    // Look ahead to see if we have a range...
    if (r[1] == '-')
    {
      // Yes, grab end character...
      r += 2;

      if (*r == '\\')
      {
	r ++;

	switch (*r)
	{
          case 'n' :
              end =  '\n';
	      break;

          case 'r' :
              end =  '\r';
	      break;

          case 't' :
              end =  '\t';
	      break;

          default :
              end =  *r;
	      break;
	}
      }
      else if (*r)
	end =  *r;
      else
        end =  255;

      // Loop through all chars until we are done...
      for (ch ++; ch <= end; ch ++)
        p[ch] = s;
    }
  }
}


//
// 'hdStyleSheet::read()' - Read a string from the given file.
//

char *					// O - String or NULL on EOF
hdStyleSheet::read(FILE     *f,	// I - File to read from
                   const char *p,	// I - Allowed chars pattern buffer
		   char       *s,	// O - String buffer
		   int        slen)	// I - Number of bytes in string buffer
{
  int	ch;
  char	*ptr,
	*end;


  // Setup pointers for the start and end of the buffer...
  ptr = s;
  end = s + slen - 1;

  // Loop until we hit EOF or a character that is not allowed...
  while (ptr < end && (ch = getc(f)) != EOF)
    if (p[ch])
      *ptr++ = ch;
    else
    {
      ungetc(ch, f);
      break;
    }

  // Nul-terminate the string...
  *ptr = '\0';

  // Return the string if it is not empty...
  if (ptr > s)
    return (s);
  else
    return (NULL);
}


//
// 'hdStyleSheet::set_charset()' - Set the document character set.
//

void
hdStyleSheet::set_charset(const char *cs)// I - Character set name
{
  int		i;			// Looping var
  char		filename[1024];		// Glyphs filename
  FILE		*fp;			// Glyphs file
  char		line[256];		// Line from file
  int		code;			// Unicode number
  char		name[255];		// Name string


  // Validate the character set name...
  if (cs == NULL || strchr(cs, '/') != NULL)
  {
    progress_error(HD_ERROR_CSS_ERROR,
                   "Bad character set \"%s\"!", cs ? cs : "(null)");
    return;
  }

  // Open the charset file...
  snprintf(filename, sizeof(filename), "%s/data/%s", _htmlData, cs);
  if ((fp = fopen(filename, "r")) == NULL)
  {
    progress_error(HD_ERROR_CSS_ERROR,
                   "Unable to open character set file \"%s\"!",
                   filename);
    return;
  }

  // Read the charset type (8bit or unicode)...
  if (file_gets(line, sizeof(line), fp) == NULL)
  {
    progress_error(HD_ERROR_CSS_ERROR,
                   "Unable to read charset type from \"%s\"!",
		   filename);
    fclose(fp);
    return;
  }

  if (strcasecmp(line, "8bit") != 0 && strcasecmp(line, "unicode") != 0)
  {
    progress_error(HD_ERROR_CSS_ERROR,
                   "Bad charset type \"%s\" in \"%s\"!",
		   line, filename);
    fclose(fp);
    return;
  }

  // Free the old charset stuff...
  if (charset)
    free(charset);

  if (num_glyphs)
  {
    for (i = 0; i < num_glyphs; i ++)
      if (glyphs[i])
        free(glyphs[i]);

    delete[] glyphs;
  }

  // Allocate the charset array...
  if (line[0] == '8')
  {
    encoding   = HD_FONT_ENCODING_8BIT;
    num_glyphs = 256;
  }
  else
  {
    encoding   = HD_FONT_ENCODING_UTF8;
    num_glyphs = 65536;
  }

  charset = strdup(cs);
  glyphs  = new char *[num_glyphs];

  memset(glyphs, 0, num_glyphs * sizeof(char *));

  // Now read all of the remaining lines from the file in the format:
  //
  //     HHHH glyph-name

  while (file_gets(line, sizeof(line), fp) != NULL)
  {
    if (sscanf(line, "%x%254s", &code, name) != 2)
    {
      progress_error(HD_ERROR_CSS_ERROR,
                     "Bad line \"%s\" in \"%s\"!", line, filename);
      break;
    }

    if (code < 0 || code >= num_glyphs)
    {
      progress_error(HD_ERROR_CSS_ERROR,
                     "Invalid code %x in \"%s\"!", code, filename);
      break;
    }

    glyphs[code] = strdup(name);
  }

  fclose(fp);
}


//
// 'hdStyleSheet::update_styles()' - Update all relative style data.
//

void
hdStyleSheet::update_styles(bool force)	// I - Force update?
{
  int		i;		// Looping var
  hdStyle	**style;	// Current style


  if (force)
  {
    // Clear the "updated" state of all styles...
    for (i = num_styles, style = styles; i > 0; i --, style ++)
      (*style)->updated = 0;
  }

  // Update all the styles...
  for (i = num_styles, style = styles; i > 0; i --, style ++)
    (*style)->update(this);
}


//
// End of "$Id$".
//
