//
// "$Id: stylesheet.cxx,v 1.11 2004/02/03 02:55:29 mike Exp $"
//
//   CSS sheet routines for HTMLDOC, a HTML document processing program.
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
// Contents:
//
//   hdStyleSheet::hdStyleSheet()      - Create a new stylesheet.
//   hdStyleSheet::~hdStyleSheet()     - Destroy a stylesheet.
//   hdStyleSheet::add_style()         - Add a style to a stylesheet.
//   hdStyleSheet::find_font()         - Find a font for the given style.
//   hdStyleSheet::find_style()        - Find the default style for the given
//   hdStyleSheet::find_style()        - Find the default style for the given
//   hdStyleSheet::get_glyph()         - Find the index for the named glyph...
//   hdStyleSheet::get_private_style() - Get a private style definition.
//   hdStyleSheet::load()              - Load a stylesheet from the given file.
//   hdStyleSheet::pattern()           - Initialize a regex pattern buffer...
//   hdStyleSheet::read()              - Read a string from the given file.
//   hdStyleSheet::set_charset()       - Set the document character set.
//   hdStyleSheet::update_styles()     - Update all relative style data.
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
  // Initialize the stylesheet structure.  Using memset() is safe
  // on structures...

  memset(this, 0, sizeof(hdStyleSheet));

  memset(elements, -1, sizeof(elements));

  ppi = 80.0f;

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
    for (j = 0; j < HD_FONTINTERNAL_MAX; j ++)
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
  char		face[1024],		// Font face property
		*start,			// Start of current font face
		*ptr;			// End of current font face
  int		i;			// Looping var
  int		tf;			// Typeface index
  int		fs;			// Font style index
  hdStyleFont	*temp;			// New font record


  // Figure out the font style we need...
  if (s->font_weight == HD_FONTWEIGHT_BOLD ||
      s->font_weight == HD_FONTWEIGHT_BOLDER)
    fs = HD_FONTINTERNAL_BOLD;
  else
    fs = HD_FONTINTERNAL_NORMAL;

  if (s->font_style)
    fs += 2;

  // Make a copy of the font family...
  if (s->font_family)
  {
    strncpy(face, s->font_family, sizeof(face) - 1);
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
    if (strcasecmp(start, "monospace") == 0 ||
        strcasecmp(start, "Courier") == 0)
      tf = HD_FONTFACE_MONOSPACE;
    else if (strcasecmp(start, "serif") == 0 ||
             strcasecmp(start, "Times") == 0)
      tf = HD_FONTFACE_SERIF;
    else if (strcasecmp(start, "sans-serif") == 0 ||
             strcasecmp(start, "Arial") == 0 ||
             strcasecmp(start, "Helvetica") == 0)
      tf = HD_FONTFACE_SANS_SERIF;
    else if (strcasecmp(start, "symbol") == 0)
      tf = HD_FONTFACE_SYMBOL;
    else if (strcasecmp(start, "cursive") == 0 ||
             strcasecmp(start, "ZapfChancery") == 0)
      tf = HD_FONTFACE_CURSIVE;
    else
    {
      // Add a custom font...
      for (tf = HD_FONTFACE_CUSTOM; tf < HD_FONTFACE_MAX; tf ++)
        if (font_names[tf] == NULL)
	  break;
	else if (strcasecmp(start, font_names[tf]) == 0)
	  break;

      if (tf >= HD_FONTFACE_MAX)
        tf = HD_FONTFACE_SERIF;
    }

    // Return the existing font, if any...
    if (fonts[tf][fs] != NULL)
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
      return (fonts[tf][fs]);
    }
  }

  // Couldn't find font, return 0...
  return ((hdStyleFont *)0);
}


//
// 'hdStyleSheet::find_style()' - Find the default style for the given
//                                tree node.
//

hdStyle *				// O - Style record
hdStyleSheet::find_style(hdTree *t)	// I - Tree node
{
  int			i;		// Looping var...
  int			nsels;		// Number of selectors...
  hdStyleSelector	sels[HD_SELECTOR_MAX];
					// Selectors...
  hdTree		*p;		// Tree pointer...


  // Figure out how many selectors to use...
  if (max_selectors[t->element] > HD_SELECTOR_MAX)
    nsels = HD_SELECTOR_MAX;
  else
    nsels = max_selectors[t->element];

  // Build the selectors for this node...
  for (i = 0, p = t; i < nsels; i ++, p = t->parent)
  {
    sels[i].element = p->element;
    sels[i].class_  = (char *)p->get_attr("CLASS");
    sels[i].id      = (char *)p->get_attr("ID");
    if (sels[i].element == HD_ELEMENT_A && p->get_attr("HREF") != NULL)
      sels[i].pseudo = (char *)"link";
    else
      sels[i].pseudo = NULL;
  }

  // Do the search...
  return (find_style(i, sels));
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

  // Return the best match...
  return (best);
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
// 'hdStyleSheet::get_private_style()' - Get a private style definition.
//

hdStyle	*				// O - New style
hdStyleSheet::get_private_style(hdTree *t)
					// I - Tree node that needs style
{
  hdStyle		*parent,	// Parent style
			*style;		// New private style
  hdStyleSelector	selector;	// Selector for private style
  char			id[16];		// Selector ID
  const char		*style_attr;	// STYLE attribute, if any


  // Find the parent style...
  parent = find_style(t);

  // Setup a private selector ID for this node...
  sprintf(id, "_HD_%08X", private_id ++);

  // Create a new style derived from this node...
  selector.set(t->element, NULL, NULL, id);

  style = new hdStyle(1, &selector, t->style);

  style->inherit(parent);

  // Apply the STYLE attribute for this node, if any...
  if ((style_attr = t->get_attr("STYLE")) != NULL)
    style->load(this, style_attr);

  // Add the style to the stylesheet...
  add_style(style);

  // Return the new style...
  return (style);
}


//
// 'hdStyleSheet::load()' - Load a stylesheet from the given file.
//

int					// O - 0 on success, -1 on failure
hdStyleSheet::load(hdFile     *f,	// I - File to read from
                   const char *path)	// I - Search path for included files
{
  int			i, j;		// Looping vars...
  int			status;		// Load status
  int			ch;		// Character from file
  char			sel_s[1024],	// Selector string
			sel_p[256],	// Selector pattern
			*sel_class,	// Selector class
			*sel_pseudo,	// Selector pseudo-target
			*sel_id;	// Selector ID
  int			cur_style,	// Current style
			num_styles,	// Number of styles to create
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
  hdFile		*import_f;	// Import file pointer
  char			media[256];	// Current media type


  // Initialize the read patterns.
  pattern("a-zA-Z0-9@.:#", sel_p);
  pattern("~}", props_p);

  // Loop until we can't read any more...
  cur_style  = 0;
  num_styles = 0;
  status     = 0;
  media[0]   = '\0';

  while ((ch = f->get()) != EOF)
  {
    // Skip whitespace...
    if (isspace(ch))
      continue;

    if (ch == '}')
    {
      media[0] = '\0';
      continue;
    }

    if (ch == '/')
    {
      // Check for C-style comment...
      if ((ch = f->get()) != '*')
      {
        hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Bad sequence \"/%c\" in stylesheet!", ch);
	status = -1;
	break;
      }

      // OK, now read chars until EOF or "*/"...
      while ((ch = f->get()) != EOF)
        if (ch == '*')
	{
	  if ((ch = f->get()) == '/')
	    break;
	  else
	    f->unget(ch);
	}

      if (ch != '/')
      {
        hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Unterminated comment in stylesheet!");
	status = -1;
	break;
      }

      continue;
    }
    else if (ch == '{')
    {
      // Handle grouping for rendering intent...
      if (num_styles == 0)
        continue;

      // Read property data...
      if (read(f, props_p, props, sizeof(props)) == NULL)
      {
        hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Missing property data in stylesheet!");
	status = -1;
	break;
      }

      while ((ch = f->get()) >= 0)
        if (!isspace(ch))
	  break;

      if (ch != '}')
      {
        f->unget(ch);
	hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Missing } for style properties!");
      }

      // Apply properties to all styles...
#ifdef DEBUG
      printf("num_styles = %d\n", num_styles);
#endif // DEBUG

      for (i = 0; i < num_styles; i ++)
      {
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

            style->load(this, props);
	  }

	  selectors[i]->element = HD_ELEMENT_NONE;
	}
	else
	{
	  // Apply to just the selected element...
          parent.element = selectors[i]->element;

          if ((style = find_style(num_selectors[i], selectors[i], 1)) == NULL)
          {
	    style = new hdStyle(num_selectors[i], selectors[i],
	                	find_style(1, &parent, 1));
            add_style(style);
	  }

          style->load(this, props);
        }

        delete[] selectors[i];
      }

      // Reset to beginning...
      cur_style  = 0;
      num_styles = 0;

      continue;
    }
    else if (ch == ',')
    {
      // Add another selector...
      cur_style ++;

      if (cur_style >= HD_SELECTOR_MAX)
      {
        hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Too many selectors (> %d) in stylesheet!",
	                        HD_SELECTOR_MAX);
	status = -1;
	break;
      }

      continue;
    }
    else if (!sel_p[ch])
    {
      // Not a valid selector string...
      hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                              "Bad stylesheet character \"%c\"!", ch);
      status = -1;
      break;
    }

    // Read the selector string...
    f->unget(ch);

    read(f, sel_p, sel_s, sizeof(sel_s));

    // OK, got a selector, see if it is @foo...
    if (sel_s[0] == '@')
    {
      // @ selector...
      if (strcmp(sel_s, "@import") == 0)
      {
        // Include another stylesheet...
	import_ptr = import;

	while ((ch = f->get()) >= 0)
	  if (!isspace(ch))
	    break;

        f->unget(ch);

        // Read the URL and potentially a media selector...
	while ((ch = f->get()) >= 0)
	{
	  // Stop at ';'...
	  if (ch == ';')
	    break;

          // Handle quotes...
	  if (import_ptr < (import + sizeof(import) - 1))
	    *import_ptr++ = ch;

	  if (ch == '\"')
	  {
	    while ((ch = f->get()) >= 0)
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

        if ((!*import_ptr && !media[0]) || strstr(import_ptr, "print") != NULL)
	{
	  // Import the file...
	  if ((import_f = hdFile::open(import, HD_FILE_READ)) != NULL)
	  {
	    load(import_f, path);

	    delete import_f;
	  }
	  else
	    hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
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
	read(f, sel_p, media, sizeof(media));

	if (strcmp(media, "print") == 0)
	  media[0] = '\0';

        // Read up to the first {...
	while ((ch = f->get()) >= 0)
          if (!isspace(ch))
	    break;

	if (ch != '{')
	{
          f->unget(ch);
	  hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                          "Missing { for media selector!");
	}
      }
      else
      {
        // Show a warning message...
	hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                        "Unsupported rule \"%s\"!", sel_s);

        int braces = 0;

	while ((ch = f->get()) >= 0)
	{
	  if (ch == '\"')
	  {
	    // Skip quoted string...
	    while ((ch = f->get()) >= 0)
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
	  hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
	                          "Missing terminator (;) for %s!", sel_s);
      }

      continue;
    }

    // Allocate memory for the selectors (up to HD_SELECTOR_MAX of them) as needed...
    if (cur_style >= num_styles)
    {
      num_styles ++;
      num_selectors[cur_style] = 0;
      selectors[cur_style]     = new hdStyleSelector[HD_SELECTOR_MAX];
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

    if (hdTree::get_element(sel_s) == HD_ELEMENT_UNKNOWN)
      printf("UNKNOWN ELEMENT %s!\n", sel_s);

    // Insert the new selector before any existing ones...
    if (num_selectors[cur_style] > 0)
      memmove(selectors[cur_style] + 1, selectors[cur_style],
              num_selectors[cur_style] * sizeof(hdStyleSelector));

    selectors[cur_style]->set(hdTree::get_element(sel_s),
                              sel_class, sel_pseudo, sel_id);

    num_selectors[cur_style] ++;
  }

  // Clear any selectors still in memory...
  for (i = 0; i < num_styles; i ++)
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
hdStyleSheet::read(hdFile     *f,	// I - File to read from
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
  while (ptr < end && (ch = f->get()) != EOF)
    if (p[ch])
      *ptr++ = ch;
    else
    {
      f->unget(ch);
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
  hdFile	*fp;			// Glyphs file
  char		line[256];		// Line from file
  int		code;			// Unicode number
  char		name[255];		// Name string


  // Validate the character set name...
  if (cs == NULL || strchr(cs, '/') != NULL)
  {
    hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                            "Bad character set \"%s\"!", cs ? cs : "(null)");
    return;
  }

  // Open the charset file...
  snprintf(filename, sizeof(filename), "%s/data/%s.charset",
           hdGlobal.datadir, cs);
  if ((fp = hdFile::open(filename, HD_FILE_READ)) == NULL)
  {
    hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                            "Unable to open character set file \"%s\"!",
			    filename);
    return;
  }

  // Read the charset type (8bit or unicode)...
  if (fp->gets(line, sizeof(line)) == NULL)
  {
    hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                            "Unable to read charset type from \"%s\"!",
			    filename);
    delete fp;
    return;
  }

  if (strcasecmp(line, "8bit") != 0 && strcasecmp(line, "unicode") != 0)
  {
    hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                            "Bad charset type \"%s\" in \"%s\"!",
			    line, filename);
    delete fp;
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
    encoding   = HD_FONTENCODING_8BIT;
    num_glyphs = 256;
  }
  else
  {
    encoding   = HD_FONTENCODING_UTF8;
    num_glyphs = 65536;
  }

  charset = strdup(cs);
  glyphs  = new char *[num_glyphs];

  memset(glyphs, 0, num_glyphs * sizeof(char *));

  // Now read all of the remaining lines from the file in the format:
  //
  //     HHHH glyph-name

  while (fp->gets(line, sizeof(line)) != NULL)
  {
    if (sscanf(line, "%x%254s", &code, name) != 2)
    {
      hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                              "Bad line \"%s\" in \"%s\"!", line, filename);
      break;
    }

    if (code < 0 || code >= num_glyphs)
    {
      hdGlobal.progress_error(HD_ERROR_CSS_ERROR,
                              "Invalid code %x in \"%s\"!", code, filename);
      break;
    }

    glyphs[code] = strdup(name);
  }

  delete fp;
}


//
// 'hdStyleSheet::update_styles()' - Update all relative style data.
//

void
hdStyleSheet::update_styles()
{
  int		i;		// Looping var
  hdStyle	**style;	// Current style


  // First clear the "updated" state of all styles...
  for (i = num_styles, style = styles; i > 0; i --, style ++)
    (*style)->updated = 0;

  // Then update all the styles...
  for (i = num_styles, style = styles; i > 0; i --, style ++)
    (*style)->update(this);
}


//
// End of "$Id: stylesheet.cxx,v 1.11 2004/02/03 02:55:29 mike Exp $".
//
