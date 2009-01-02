//
// "$Id$"
//
// HTML exporting functions for HTMLDOC, a HTML document processing program.
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
//   html_export()     - Export to HTML.
//   htmlsep_export()  - Export to separated HTML files.
//   add_heading()     - Add a heading to the list of headings.
//   add_link()        - Add a named link.
//   compare_links()   - Compare two named links.
//   do_export()       - Export to HTML.
//   find_link()       - Find a named link.
//   get_title()       - Get the title string for the given document.
//   scan_links()      - Scan a document for link targets, and keep track of the
//                       files they are in.
//   update_links()    - Update links as needed.
//   write_header()    - Output the standard "header" for a HTML file.
//   write_footer()    - Output the standard "footer" for a HTML file.
//   write_title()     - Write a title page.
//   write_all()       - Write all markup text for the given tree.
//   write_doc()       - Write the entire document.
//   write_node()      - Write a single tree node.
//   write_nodeclose() - Close a single tree node.
//   write_toc()       - Write all markup text for the given table-of-contents.
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include <ctype.h>


//
// Named link structure...
//

typedef struct
{
  hdChar		*filename;	// File for link
  hdChar		name[124];	// Reference name
} hdLink;


//
// Local globals...
//

static int	num_headings = 0,	// Number of headings
		alloc_headings = 0;	// Allocated headings
static hdChar	**headings;		// Heading strings

static int	num_links = 0,
		alloc_links = 0;
static hdLink	*links;


//
// Local functions...
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static int	do_export(hdTree *document, hdTree *toc, hdTree *ind,
		          bool use_headings);
static void	write_header(hdFile *&out, hdChar *filename, hdChar *title,
		             hdChar *author, hdChar *copyright,
			     hdChar *docnumber, hdTree *t, int heading);
static void	write_footer(hdFile *&out, hdTree *t, int heading);
static void	write_title(hdFile *out, hdChar *title, hdChar *author,
		            hdChar *copyright, hdChar *docnumber);
static int	write_all(hdFile *out, hdTree *t, int col);
static int	write_doc(hdFile *&out, hdTree *t, int col, int *heading,
		          hdChar *title, hdChar *author, hdChar *copyright,
			  hdChar *docnumber);
static int	write_node(hdFile *out, hdTree *t, int col);
static int	write_nodeclose(hdFile *out, hdTree *t, int col);
static int	write_toc(hdFile *out, hdTree *t, int col);
static hdChar	*get_title(hdTree *doc);

static void	add_heading(hdTree *t);
static void	add_link(hdChar *name, hdChar *filename);
static int	compare_links(hdLink *n1, hdLink *n2);
static hdLink	*find_link(hdChar *name);
static void	scan_links(hdTree *t, hdChar *filename, bool use_headings);
static void	update_links(hdTree *t, hdChar *filename, int *heading);


//
// 'html_export()' - Export to HTML.
//

int					// O - 0 = success, -1 = failure
html_export(hdTree *document,		// I - Document to export
            hdTree *toc,		// I - Table of contents for document
	    hdTree *ind)		// I - Index of document
	    
{
  return (do_export(document, toc, ind, false));
}


//
// 'htmlsep_export()' - Export to separated HTML files.
//

int					// O - 0 = success, -1 = failure
htmlsep_export(hdTree *document,	// I - Document to export
               hdTree *toc,		// I - Table of contents for document
	       hdTree *ind)		// I - Index of document
{
  return (do_export(document, toc, ind, true));
}


//
// 'add_heading()' - Add a heading to the list of headings.
//

static void
add_heading(hdTree *t)			// I - Heading node
{
  int		i,			// Looping var
		count;			// Count of headings with this name
  hdChar	*heading,		// Heading text for this node
		*ptr,			// Pointer into text
		*ptr2,			// Second pointer into text
		filename[1024],		// Filename to use
		**temp;			// New heading array pointer


  // Start by getting the heading text...
  heading = htmlGetText(t->child);
  if (!heading || !*heading)
    return;				// Nothing to do!

  // Sanitize the text...
  for (ptr = heading; *ptr;)
    if (!isalnum(*ptr & 255))
    {
      // Remove anything but letters and numbers from the filename
      for (ptr2 = ptr; *ptr2; ptr2 ++)
        *ptr2 = ptr2[1];

      *ptr2 = '\0';
    }
    else
      ptr ++;

  snprintf((char *)filename, sizeof(filename), "%s.html", heading);

  // Now loop through the existing headings and check for dups...
  for (i = 0, count = 0; i < num_headings; i ++)
    if (!strcmp((char *)headings[i], (char *)filename))
    {
      // Create a new instance of the heading...
      count ++;
      snprintf((char *)filename, sizeof(filename), "%s%d.html", heading, count);
    }

  // Now add the heading...
  if (num_headings >= alloc_headings)
  {
    // Allocate more headings...
    alloc_headings += ALLOC_HEADINGS;

    if (num_headings == 0)
      temp = (hdChar **)malloc(sizeof(hdChar *) * alloc_headings);
    else
      temp = (hdChar **)realloc(headings, sizeof(hdChar *) * alloc_headings);

    if (temp == NULL)
    {
      progress_error(HD_ERROR_OUT_OF_MEMORY,
	             "Unable to allocate memory for %d headings - %s",
	             alloc_headings, strerror(errno));
      alloc_headings -= ALLOC_HEADINGS;
      return;
    }

    headings = temp;
  }

  // Make a copy of the string "s" and free the old heading string...
  headings[num_headings] = (hdChar *)strdup((char *)filename);
  free(heading);

  num_headings ++;
}


//
// 'add_link()' - Add a named link.
//

static void
add_link(hdChar *name,			// I - Name of link
         hdChar *filename)		// I - File for link
{
  hdLink	*temp;			// New name


  if (!filename)
    return;

  if ((temp = find_link(name)) != NULL)
    temp->filename = filename;
  else
  {
    // See if we need to allocate memory for links...
    if (num_links >= alloc_links)
    {
      // Allocate more links...
      alloc_links += ALLOC_LINKS;

      if (num_links == 0)
        temp = (hdLink *)malloc(sizeof(hdLink) * alloc_links);
      else
        temp = (hdLink *)realloc(links, sizeof(hdLink) * alloc_links);

      if (temp == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
	               "Unable to allocate memory for %d links - %s",
	               alloc_links, strerror(errno));
        alloc_links -= ALLOC_LINKS;
	return;
      }

      links = temp;
    }

    // Add a new link...
    temp = links + num_links;
    num_links ++;

    strlcpy((char *)temp->name, (char *)name, sizeof(temp->name));
    temp->filename = filename;

    if (num_links > 1)
      qsort(links, num_links, sizeof(hdLink), (compare_func_t)compare_links);
  }
}


//
// 'compare_links()' - Compare two named links.
//

static int				// O - 0 = equal, -1 or 1 = not equal
compare_links(hdLink *n1,		// I - First name
              hdLink *n2)		// I - Second name
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


//
// 'do_export()' - Export to HTML.
//

static int				// O - 0 = success, -1 = failure
do_export(hdTree *document,		// I - Document to export
	  hdTree *toc,			// I - Table of contents for document
	  hdTree *ind,			// I - Index of document
	  bool   use_headings)		// I - Use headings for filenames
{
  int		i;			// Looping var
  int		heading;		// Current heading number
  hdChar	*title,			// Title text
		*author,		// Author name
		*copyright,		// Copyright text
		*docnumber;		// Document number
  hdFile	*out;			// Output file
  char		temp[1024];		// Temporary buffer for find


  // We only support use_headings when writing to a directory...
  if (use_headings && !OutputFiles)
  {
    progress_error(HD_ERROR_INTERNAL_ERROR,
                   "Unable to generate separated HTML to a single file!");
    return (-1);
  }

  // Copy logo and title images...
  if (OutputFiles)
  {
    if (LogoImage)
      LogoImage->copy(OutputPath, temp, sizeof(temp));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (HFImage[hfi])
        HFImage[hfi]->copy(OutputPath, temp, sizeof(temp));
  }

  if (OutputFiles && TitleImage && TitlePage)
    TitleImage->copy(OutputPath, temp, sizeof(temp));

  // Get document strings...
  title     = get_title(document);
  author    = htmlGetMeta(document, "author");
  copyright = htmlGetMeta(document, "copyright");
  docnumber = htmlGetMeta(document, "docnumber");

  // Scan for all links in the document, and then update them...
  num_links   = 0;
  alloc_links = 0;
  links       = NULL;

  scan_links(document, NULL, use_headings);
  if (ind)
    scan_links(ind, OutputFiles ? (hdChar *)"idx.html" : NULL, false);

  heading = -1;
  update_links(document, NULL, use_headings ? &heading : NULL);
  update_links(toc, NULL, NULL);
  if (ind)
    update_links(ind, NULL, NULL);

  // Generate title pages and a table of contents...
  out = NULL;
  if (TitlePage)
  {
    write_header(out, (hdChar *)"index.html", title, author, copyright,
                 docnumber, NULL, -1);
    write_title(out, title, author, copyright, docnumber);
  }
  else
    write_header(out, (hdChar *)"index.html", title, author, copyright,
                 docnumber, NULL, -1);

  write_toc(out, toc, 0);
  write_footer(out, NULL, -1);

  // Then write each output file...
  if (use_headings)
  {
    heading = -1;
    write_doc(out, document, 0, &heading, title, author, copyright, docnumber);
    write_footer(out, document, heading);
  }
  else
  {
    while (document != NULL)
    {
      write_header(out, htmlGetAttr(document, "_HD_FILENAME"),
		   title, author, copyright, docnumber, document, -1);
      write_all(out, document->child, 0);
      write_footer(out, document, -1);

      document = document->next;
    }
  }

  if (ind)
  {
    write_header(out, (hdChar *)"idx.html", title, author, copyright,
        	 docnumber, ind, -1);
    write_all(out, ind->child, 0);
    write_footer(out, ind, -1);
  }

  if (!OutputFiles && out)
  {
    out->puts("</BODY>\n");
    out->puts("</HTML>\n");

    progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)out->size());

    delete out;
  }

  if (title != NULL)
    free(title);

  if (alloc_links)
  {
    free(links);

    num_links   = 0;
    alloc_links = 0;
    links       = NULL;
  }

  if (alloc_headings)
  {
    for (i = 0; i < num_headings; i ++)
      free(headings[i]);

    free(headings);

    num_headings   = 0;
    alloc_headings = 0;
    headings       = NULL;
  }

  return (out == NULL ? -1 : 0);
}


//
// 'find_link()' - Find a named link.
//

static hdLink *				// O - Link or NULL
find_link(hdChar *name)			// I - Name to find
{
  hdChar	*target;		// Pointer to target name portion
  hdLink	key,			// Search key
		*match;			// Matching name entry


  if (name == NULL || num_links == 0)
    return (NULL);

  if ((target = (hdChar *)hdFile::target((char *)name)) == NULL)
    return (NULL);

  strlcpy((char *)key.name, (char *)target, sizeof(key.name));
  match = (hdLink *)bsearch(&key, links, num_links, sizeof(hdLink),
                            (compare_func_t)compare_links);

  return (match);
}


//
// 'get_title()' - Get the title string for the given document.
//

static hdChar *				// O - Title string
get_title(hdTree *doc)			// I - Document tree
{
  hdChar	*temp;			// Temporary pointer to title


  while (doc != NULL)
  {
    if (doc->element == HD_ELEMENT_TITLE)
      return (htmlGetText(doc->child));
    else if (doc->child != NULL)
      if ((temp = get_title(doc->child)) != NULL)
        return (temp);

    doc = doc->next;
  }

  return (NULL);
}


//
// 'scan_links()' - Scan a document for link targets, and keep track of
//                  the files they are in.
//

static void
scan_links(hdTree *t,			// I - Document tree
           hdChar *filename,		// I - Filename
	   bool   use_headings)		// I - Use heading-based filenames?
{
  hdChar	*name;			// Name of link


  while (t != NULL)
  {
    if (!use_headings && t->element == HD_ELEMENT_FILE)
      filename = htmlGetAttr(t, "_HD_FILENAME");
    else if (use_headings && t->element == HD_ELEMENT_H1 &&
             !htmlGetAttr(t, "_HD_OMIT_TOC"))
    {
      add_heading(t);
      filename = headings[num_headings - 1];
    }
    else if (t->element == HD_ELEMENT_A &&
	     (name = htmlGetAttr(t, "NAME")) != NULL)
      add_link(name, filename);

    if (t->child)
      scan_links(t->child, filename, use_headings);

    t = t->next;
  }
}


//
// 'update_links()' - Update links as needed.
//

static void
update_links(hdTree *t,			// I  - Document tree
             hdChar *filename,		// I  - Current filename
	     int    *heading)		// IO - Current heading
{
  hdLink	*link;			// Link
  hdChar	*href;			// Reference name
  hdChar	newhref[1024];		// New reference name
  char		base[1024];		// Base filename


  if (OutputFiles)
  {
    // Scan the document, rewriting HREF's as needed...
    while (t != NULL)
    {
      if (heading && t->element == HD_ELEMENT_H1 &&
          !htmlGetAttr(t, "_HD_OMIT_TOC"))
      {
	// Figure out the current filename based upon the current heading
	// number...
	(*heading) ++;

	if (*heading < 0 || *heading >= num_headings)
	  filename = (hdChar *)"noheading";
	else
	  filename = headings[*heading];
      }

      if (t->element == HD_ELEMENT_A &&
          (href = htmlGetAttr(t, "HREF")) != NULL)
      {
        // Update this link as needed...
        if (href[0] == '#' &&
	    (link = find_link(href)) != NULL)
	{
#ifdef WIN32
	  if (!filename || strcasecmp((char *)filename, (char *)link->filename))
#else
          if (!filename || strcmp((char *)filename, (char *)link->filename))
#endif // WIN32
	  {
	    snprintf((char *)newhref, sizeof(newhref), "%s%s",
	             link->filename, href);
	    htmlSetAttr(t, "HREF", newhref);
	  }
	}
      }

      if (t->child != NULL)
      {
        if (!heading && t->element == HD_ELEMENT_FILE)
	{
          hdFile::basename((char *)htmlGetAttr(t, "_HD_FILENAME"), base,
	                   sizeof(base));

          update_links(t->child, (hdChar *)base, heading);
	}
	else
          update_links(t->child, filename, heading);
      }

      t = t->next;
    }
  }
  else
  {
    // Need to strip filenames.
    while (t != NULL)
    {
      if (t->element == HD_ELEMENT_A &&
          (href = htmlGetAttr(t, "HREF")) != NULL)
      {
        // Update this link as needed...
        if (href[0] != '#' && hdFile::scheme((char *)href) == NULL &&
	    (link = find_link(href)) != NULL)
	{
	  snprintf((char *)newhref, sizeof(newhref), "#%s", link->name);
	  htmlSetAttr(t, "HREF", newhref);
	}
      }

      if (t->child != NULL)
        update_links(t->child, filename, heading);

      t = t->next;
    }
  }
}


//
// 'write_all()' - Write all markup text for the given tree.
//

static int				// O - Current column
write_all(hdFile *out,			// I - Output file
          hdTree *t,			// I - Document tree
          int    col)			// I - Current column
{
  if (!out)
    return (0);

  while (t != NULL)
  {
    col = write_node(out, t, col);

    if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
      col = write_all(out, t->child, col);

    col = write_nodeclose(out, t, col);

    t = t->next;
  }

  return (col);
}


//
// 'write_doc()' - Write the entire document.
//

static int				// O  - Current column
write_doc(hdFile  *&out,		// IO - Output file
          hdTree  *t,			// I  - Document tree
          int     col,			// I  - Current column
          int     *heading,		// IO - Current heading
	  hdChar  *title,		// I  - Title
          hdChar  *author,		// I  - Author
	  hdChar  *copyright,		// I  - Copyright
	  hdChar  *docnumber)		// I  - Document number
{
  while (t != NULL)
  {
    if (t->element == HD_ELEMENT_H1 && !htmlGetAttr(t, "_HD_OMIT_TOC"))
    {
      if (*heading >= 0)
        write_footer(out, NULL, *heading);

      (*heading) ++;

      if (*heading >= 0)
	write_header(out, headings[*heading], title, author, copyright,
	             docnumber, NULL, *heading);
    }

    col = write_node(out, t, col);

    if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
      col = write_doc(out, t->child, col, heading,
                      title, author, copyright, docnumber);

    col = write_nodeclose(out, t, col);

    t = t->next;
  }

  return (col);
}


//
// 'write_footer()' - Output the standard "footer" for a HTML file.
//

static void
write_footer(hdFile *&out,		// IO - Output file pointer
	     hdTree *t,			// I  - Current document file
	     int    heading)		// I  - Current heading
{
  char	base[1024];			// Base path for images


  if (!out)
    return;

  if (OutputFiles && ((t && (t->prev || t->next)) || heading >= 0))
  {
    out->puts("<DIV CLASS=\"HD_NAV\">\n");

    if (LogoImage)
      out->printf("<IMG SRC=\"%s\">\n",
                  hdFile::basename(LogoImage->uri(), base, sizeof(base)));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi])
        out->printf("<IMG SRC=\"%s\">\n",
	        hdFile::basename(HFImage[hfi]->uri(), base, sizeof(base)));

    out->puts("<A HREF=\"index.html#HD_CONTENTS\">Contents</A>\n");

    if (heading >= 0)
    {
      if (heading > 0)
	out->printf("<A HREF=\"%s\">Previous</A>\n", headings[heading - 1]);

      if (heading < (num_headings - 1))
	out->printf("<A HREF=\"%s\">Next</A>\n", headings[heading + 1]);
    }
    else if (t)
    {
      if (t->prev)
	out->printf("<A HREF=\"%s\">Previous</A>\n",
		    htmlGetAttr(t->prev, "_HD_FILENAME"));

      if (t->next)
	out->printf("<A HREF=\"%s\">Next</A>\n",
	            htmlGetAttr(t->next, "_HD_FILENAME"));
    }

    out->puts("</DIV>\n");
  }

  if (OutputFiles)
  {
    out->puts("</BODY>\n");
    out->puts("</HTML>\n");

    progress_error(HD_ERROR_NONE, "BYTES: %ld", out->size());

    delete out;
    out = NULL;
  }
}


//
// 'write_header()' - Output the standard "header" for a HTML file.
//

static void
write_header(hdFile *&out,		// IO - Output file
             hdChar *filename,		// I - Output filename
	     hdChar *title,		// I - Title for document
             hdChar *author,		// I - Author for document
             hdChar *copyright,		// I - Copyright for document
             hdChar *docnumber,		// I - ID number for document
	     hdTree *t,			// I - Current document file
	     int    heading)		// I - Current heading
{
  char		base[1024],		// Base path for images
		realname[1024];		// Real filename
  int		newfile;		// Non-zero if this is a new file
  static const char *families[] =	// Typeface names
		{
		  "monospace",
		  "serif",
		  "sans-serif",
		  "monospace",
		  "serif",
		  "sans-serif",
		  "symbol",
		  "dingbats"
		};


  if (OutputFiles)
  {
    newfile  = 1;

    snprintf(realname, sizeof(realname), "%s/%s", OutputPath, filename);

    out = hdFile::open(realname, HD_FILE_WRITE);
  }
  else if (OutputPath[0])
  {
    if (!out)
    {
      out     = hdFile::open(OutputPath, HD_FILE_WRITE);
      newfile = 1;
    }
    else
      newfile = 0;
  }
  else
  {
    if (!out)
    {
      out     = new hdStdFile(stdout, HD_FILE_WRITE);
      newfile = 1;
    }
    else
      newfile = 0;
  }

  if (!out)
  {
    progress_error(HD_ERROR_WRITE_ERROR,
                   "Unable to create output file \"%s\" - %s.\n",
                   OutputFiles ? realname : OutputPath,
		   strerror(errno));
    return;
  }

  if (newfile)
  {
    out->puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
              "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n"
	      "<HTML>\n"
	      "<HEAD>\n");
    if (title)
      out->printf("<TITLE>%s</TITLE>\n", title);
    if (author)
      out->printf("<META NAME=\"author\" CONTENT=\"%s\">\n", author);
    if (copyright)
      out->printf("<META NAME=\"copyright\" CONTENT=\"%s\">\n", copyright);
    if (docnumber)
      out->printf("<META NAME=\"docnumber\" CONTENT=\"%s\">\n", docnumber);
    out->printf("<META HTTP-EQUIV=\"Content-Type\" "
		"CONTENT=\"text/html; CHARSET=%s\">\n",
                _htmlStyleSheet->charset);

    if (OutputFiles)
    {
      out->puts("<LINK REL=\"Start\" HREF=\"index.html\">\n"
                "<LINK REL=\"Contents\" HREF=\"index.html#HD_CONTENTS\">\n");

      if (heading >= 0)
      {
	if (heading > 0)
	  out->printf("<LINK REL=\"Prev\" HREF=\"%s\">\n",
	              headings[heading - 1]);

	if (heading < (num_headings - 1))
	  out->printf("<LINK REL=\"Next\" HREF=\"%s\">\n",
	              headings[heading + 1]);
      }
      else if (t)
      {
	if (t->prev)
	  out->printf("<LINK REL=\"Prev\" HREF=\"%s\">\n",
        	      htmlGetAttr(t->prev, "_HD_FILENAME"));

	if (t->next)
	  out->printf("<LINK REL=\"Next\" HREF=\"%s\">\n",
		      htmlGetAttr(t->next, "_HD_FILENAME"));
      }
    }

    out->puts("<STYLE TYPE=\"text/css\"><!--\n");
    out->printf("BODY { font-family: %s }\n", families[_htmlBodyFont]);
    out->printf("H1 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->printf("H2 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->printf("H3 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->printf("H4 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->printf("H5 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->printf("H6 { font-family: %s }\n", families[_htmlHeadingFont]);
    out->puts("SUB { font-size: smaller }\n");
    out->puts("SUP { font-size: smaller }\n");
    out->puts("PRE { font-family: monospace }\n");

    if (!LinkStyle)
      out->puts("A { text-decoration: none }\n");

    out->puts("DIV.HD_NAV { background: #eeeeee; border: dotted thin black; "
              "font-size: 90%; padding: 5px; }\n");

    out->puts("--></STYLE>\n");
    out->puts("</HEAD>\n");
    out->puts("<BODY");

    if (BodyImage)
      out->printf(" BACKGROUND=\"%s\"", BodyImage->uri());
    if (BodyColor[0])
      out->printf(" BGCOLOR=\"%s\"", BodyColor);

    out->printf(" TEXT=\"#%02X%02X%02X\"",
            _htmlStyleSheet->def_style.color[0],
            _htmlStyleSheet->def_style.color[1],
            _htmlStyleSheet->def_style.color[2]);

    if (LinkColor[0])
      out->printf(" LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\"", LinkColor,
              LinkColor, LinkColor);

    out->puts(">\n");
  }
  else
    out->puts("<HR NOSHADE>\n");

  if (OutputFiles && ((t && (t->prev || t->next)) || heading >= 0))
  {
    out->puts("<DIV CLASS=\"HD_NAV\">\n");

    if (LogoImage)
      out->printf("<IMG SRC=\"%s\">\n",
                  hdFile::basename(LogoImage->uri(), base, sizeof(base)));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi])
        out->printf("<IMG SRC=\"%s\">\n",
	        hdFile::basename(HFImage[hfi]->uri(), base, sizeof(base)));

    out->puts("<A HREF=\"index.html#HD_CONTENTS\">Contents</A>\n");

    if (heading >= 0)
    {
      if (heading > 0)
	out->printf("<A HREF=\"%s\">Previous</A>\n", headings[heading - 1]);

      if (heading < (num_headings - 1))
	out->printf("<A HREF=\"%s\">Next</A>\n", headings[heading + 1]);
    }
    else if (t)
    {
      if (t->prev)
	out->printf("<A HREF=\"%s\">Previous</A>\n",
		    htmlGetAttr(t->prev, "_HD_FILENAME"));

      if (t->next)
	out->printf("<A HREF=\"%s\">Next</A>\n",
		    htmlGetAttr(t->next, "_HD_FILENAME"));
    }

    out->puts("</DIV>\n");
  }
}


//
// 'write_node()' - Write a single tree node.
//

static int				// O - Current column
write_node(hdFile *out,			// I - Output file
           hdTree *t,			// I - Document tree node
           int    col)			// I - Current column
{
  int		i;			// Looping var
  const hdChar	*ptr,			// Pointer to output string
		*src,			// Source image
		*realsrc;		// Real source image
  hdChar	newsrc[1024];		// New source image filename
  const char	*entity;		// Entity string


  if (!out)
    return (0);

  switch (t->element)
  {
    case HD_ELEMENT_NONE :
        if (t->data == NULL)
	  break;

	if (t->style->white_space == HD_WHITE_SPACE_PRE)
	{
          for (ptr = t->data; *ptr; ptr ++)
	    out->puts(_htmlStyleSheet->get_entity(*ptr));

	  if (t->data[strlen((char *)t->data) - 1] == '\n')
            col = 0;
	  else
            col += strlen((char *)t->data);
	}
	else
	{
	  if ((col + strlen((char *)t->data)) > 72 && col > 0 &&
	      isspace(t->data[0] & 255))
	  {
            out->put('\n');
            col = 0;
	  }

          for (ptr = t->data; *ptr; ptr ++)
            out->puts(_htmlStyleSheet->get_entity(*ptr));

	  col += strlen((char *)t->data);

	  if (col > 72 && isspace(t->data[strlen((char *)t->data) - 1] & 255))
	  {
            out->put('\n');
            col = 0;
	  }
	}
	break;

    case HD_ELEMENT_COMMENT :
    case HD_ELEMENT_UNKNOWN :
        out->puts("\n<!--");
	for (ptr = t->data; *ptr; ptr ++)
	  out->puts(_htmlStyleSheet->get_entity(*ptr));
	out->puts("-->\n");
	col = 0;
	break;

    case HD_ELEMENT_AREA :
    case HD_ELEMENT_BODY :
    case HD_ELEMENT_DOCTYPE :
    case HD_ELEMENT_ERROR :
    case HD_ELEMENT_FILE :
    case HD_ELEMENT_HEAD :
    case HD_ELEMENT_HTML :
    case HD_ELEMENT_MAP :
    case HD_ELEMENT_META :
    case HD_ELEMENT_TITLE :
        break;

    case HD_ELEMENT_BR :
    case HD_ELEMENT_CENTER :
    case HD_ELEMENT_DD :
    case HD_ELEMENT_DL :
    case HD_ELEMENT_DT :
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
    case HD_ELEMENT_HR :
    case HD_ELEMENT_LI :
    case HD_ELEMENT_OL :
    case HD_ELEMENT_P :
    case HD_ELEMENT_PRE :
    case HD_ELEMENT_TABLE :
    case HD_ELEMENT_TR :
    case HD_ELEMENT_UL :
        if (col > 0)
        {
          out->put('\n');
          col = 0;
        }

    case HD_ELEMENT_IMG :
	if (OutputFiles && (src = htmlGetAttr(t, "SRC")) != NULL &&
            (realsrc = htmlGetAttr(t, "_HD_SRC")) != NULL)
	{
	  // Update and copy local images...
          if (hdFile::scheme((char *)src) == NULL &&
              src[0] != '/' && src[0] != '\\' &&
	      (!isalpha(src[0]) || src[1] != ':'))
          {
	    hdImage *img = hdImage::find((char *)realsrc, !OutputColor);
	    img->copy(OutputPath, (char *)newsrc, sizeof(newsrc));
	    hdFile::basename((char *)src, (char *)newsrc, sizeof(newsrc));
            htmlSetAttr(t, "SRC", newsrc);
          }
	}

    default :
        if (t->element != HD_ELEMENT_EMBED)
	{
	  col += out->printf("<%s", _htmlStyleSheet->get_element(t->element));
	  for (i = 0; i < t->nattrs; i ++)
	  {
            if (!strncasecmp((char *)t->attrs[i].name, "_HD_", 4))
	      continue;

	    if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE)
	    {
              out->put('\n');
              col = 0;
	    }

            if (col > 0)
            {
              out->put(' ');
              col ++;
            }

	    if (t->attrs[i].value == NULL)
              col += out->printf("%s", t->attrs[i].name);
	    else
	    {
	      col += out->printf("%s=\"", t->attrs[i].name);
	      for (ptr = t->attrs[i].value; *ptr; ptr ++)
	      {
		entity = _htmlStyleSheet->get_entity(*ptr);
		out->puts(entity);
		col += strlen(entity);
	      }

	      out->put('\"');
	      col ++;
	    }
	  }

	  out->put('>');
	  col ++;

	  if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE &&
	      t->style->display != HD_DISPLAY_INLINE)
	  {
	    out->put('\n');
	    col = 0;
	  }
	}
	break;
  }

  return (col);
}


//
// 'write_nodeclose()' - Close a single tree node.
//

static int				// O - Current column
write_nodeclose(hdFile *out,		// I - Output file
                hdTree *t,		// I - Document tree node
                int    col)		// I - Current column
{
  if (!out)
    return (0);

  if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
  {
    if (col > 72 && t->style->white_space != HD_WHITE_SPACE_PRE &&
        t->style->display != HD_DISPLAY_INLINE)
    {
      out->put('\n');
      col = 0;
    }

    switch (t->element)
    {
      case HD_ELEMENT_BODY :
      case HD_ELEMENT_ERROR :
      case HD_ELEMENT_FILE :
      case HD_ELEMENT_HEAD :
      case HD_ELEMENT_HTML :
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_TITLE :

      case HD_ELEMENT_APPLET :
      case HD_ELEMENT_AREA :
      case HD_ELEMENT_BR :
      case HD_ELEMENT_COMMENT :
      case HD_ELEMENT_DOCTYPE :
      case HD_ELEMENT_EMBED :
      case HD_ELEMENT_HR :
      case HD_ELEMENT_IMG :
      case HD_ELEMENT_INPUT :
      case HD_ELEMENT_ISINDEX :
      case HD_ELEMENT_LINK :
      case HD_ELEMENT_META :
      case HD_ELEMENT_NOBR :
      case HD_ELEMENT_SPACER :
      case HD_ELEMENT_WBR :
      case HD_ELEMENT_UNKNOWN :
          break;

      case HD_ELEMENT_CENTER :
      case HD_ELEMENT_DD :
      case HD_ELEMENT_DL :
      case HD_ELEMENT_DT :
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
      case HD_ELEMENT_LI :
      case HD_ELEMENT_OL :
      case HD_ELEMENT_P :
      case HD_ELEMENT_PRE :
      case HD_ELEMENT_TABLE :
      case HD_ELEMENT_TR :
      case HD_ELEMENT_UL :
          out->printf("</%s>\n", _htmlStyleSheet->get_element(t->element));
          col = 0;
          break;

      default :
          col += out->printf("</%s>", _htmlStyleSheet->get_element(t->element));
	  break;
    }
  }

  return (col);
}


//
// 'write_title()' - Write a title page.
//

static void
write_title(hdFile *out,		// I - Output file
            hdChar *title,		// I - Title for document
            hdChar *author,		// I - Author for document
            hdChar *copyright,		// I - Copyright for document
            hdChar *docnumber)		// I - ID number for document
{
  hdFile	*fp;			// Title file
  hdTree	*t;			// Title file document tree
  char		base[1024],		// Base filename of file
		temp[1024];		// Temporary filename buffer


  if (!out)
    return;

  if (TitleFile[0] && !TitleImage)
  {
    if (!hdFile::find(Path, TitleFile, temp, sizeof(temp)))
    {
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to find title file \"%s\"!", TitleFile);
      return;
    }

    // Write a title page from HTML source...
    if ((fp = hdFile::open(temp, HD_FILE_READ)) == NULL)
    {
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open title file \"%s\" - %s!",
                     TitleFile, strerror(errno));
      return;
    }

    t = htmlReadFile(NULL, fp, hdFile::dirname(TitleFile, temp, sizeof(temp)));
    htmlFixLinks(t, t, temp);
    delete fp;

    write_all(out, t, 0);
    htmlDeleteTree(t);
  }
  else
  {
    // Write a "standard" title page with image...
    out->puts("<CENTER><A HREF=\"#HD_CONTENTS\">");

    if (TitleImage)
    {
      if (OutputFiles)
	out->printf("<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"%d\" HEIGHT=\"%d\" "
	             "ALT=\"%s\"><BR>\n",
        	hdFile::basename((char *)TitleImage->uri(), base, sizeof(base)),
		TitleImage->width(), TitleImage->height(),
		title ? (char *)title : "");
      else
	out->printf("<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"%d\" HEIGHT=\"%d\" "
	             "ALT=\"%s\"><BR>\n",
        	TitleImage->uri(), TitleImage->width(), TitleImage->height(),
		title ? (char *)title : "");
    }

    if (title != NULL)
      out->printf("<H1>%s</H1></A><BR>\n", title);
    else
      out->puts("</A>\n");

    if (docnumber != NULL)
      out->printf("%s<BR>\n", docnumber);

    if (author != NULL)
      out->printf("%s<BR>\n", author);

    if (copyright != NULL)
      out->printf("%s<BR>\n", copyright);

    out->puts("</CENTER>\n");
  }
}


//
// 'write_toc()' - Write all markup text for the given table-of-contents.
//

static int				// O - Current column
write_toc(hdFile *out,			// I - Output file
          hdTree *t,			// I - Document tree
          int    col)			// I - Current column
{
  if (!out)
    return (0);

  while (t != NULL)
  {
    if (htmlGetAttr(t, "_HD_OMIT_TOC") == NULL)
    {
      col = write_node(out, t, col);

      if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
	col = write_toc(out, t->child, col);

      col = write_nodeclose(out, t, col);
    }

    t = t->next;
  }

  return (col);
}


//
// End of "$Id$".
//
