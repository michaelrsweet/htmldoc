//
// "$Id: htmlsep.cxx,v 1.1.2.5 2003/12/18 21:55:42 mike Exp $"
//
//   Separated HTML export functions for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 1997-2003 by Easy Software Products.
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
//   htmlsep_export() - Export to separated HTML files.
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
  uchar		*filename;	/* File for link */
  uchar		name[124];	/* Reference name */
} link_t;


//
// Local globals...
//

// Heading strings used for filenames...
static int	num_headings = 0,	// Number of headings
		alloc_headings = 0;	// Allocated headings
static uchar	**headings;		// Heading strings

// Links in document - used to add the correct filename to the link
static int	num_links = 0,		// Number of links
		alloc_links = 0;	// Allocated links
static link_t	*links;			// Links


//
// Local functions...
//

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static void	write_header(FILE **out, uchar *filename, uchar *title,
		             uchar *author, uchar *copyright, uchar *docnumber,
			     int heading);
static void	write_footer(FILE **out, int heading);
static void	write_title(FILE *out, uchar *title, uchar *author,
		            uchar *copyright, uchar *docnumber);
static int	write_all(FILE *out, tree_t *t, int col);
static int	write_doc(FILE **out, tree_t *t, int col, int *heading,
		          uchar *title, uchar *author, uchar *copyright,
			  uchar *docnumber);
static int	write_node(FILE *out, tree_t *t, int col);
static int	write_nodeclose(FILE *out, tree_t *t, int col);
static uchar	*get_title(tree_t *doc);

static void	add_heading(tree_t *t);
static void	add_link(uchar *name);
static link_t	*find_link(uchar *name);
static int	compare_links(link_t *n1, link_t *n2);
static void	scan_links(tree_t *t);
static void	update_links(tree_t *t, int *heading);


//
// 'htmlsep_export()' - Export to separated HTML files...
//

int					// O - 0 = success, -1 = failure
htmlsep_export(tree_t *document,	// I - Document to export
               tree_t *toc)		// I - Table of contents for document
{
  int	i;				// Looping var
  int	heading;			// Current heading number
  uchar	*title,				// Title text
	*author,			// Author name
	*copyright,			// Copyright text
	*docnumber;			// Document number
  FILE	*out;				// Output file


  // We only support writing to a directory...
  if (!OutputFiles)
  {
    progress_error(HD_ERROR_INTERNAL_ERROR, "Unable to generate separated HTML to a single file!");
    return (-1);
  }

  // Copy logo and title images...
  if (LogoImage[0] != '\0')
    image_copy(LogoImage, OutputPath);

  if (TitleImage[0] != '\0' && TitlePage &&
#ifdef WIN32
      stricmp(file_extension(TitleImage), "bmp") == 0 ||
      stricmp(file_extension(TitleImage), "gif") == 0 ||
      stricmp(file_extension(TitleImage), "jpg") == 0 ||
      stricmp(file_extension(TitleImage), "png") == 0)
#else
      strcmp(file_extension(TitleImage), "bmp") == 0 ||
      strcmp(file_extension(TitleImage), "gif") == 0 ||
      strcmp(file_extension(TitleImage), "jpg") == 0 ||
      strcmp(file_extension(TitleImage), "png") == 0)
#endif // WIN32
    image_copy(TitleImage, OutputPath);

  // Get document strings...
  title     = get_title(document);
  author    = htmlGetMeta(document, (uchar *)"author");
  copyright = htmlGetMeta(document, (uchar *)"copyright");
  docnumber = htmlGetMeta(document, (uchar *)"docnumber");

  // Scan for all links in the document, and then update them...
  num_links   = 0;
  alloc_links = 0;
  links       = NULL;

  scan_links(document);

//  printf("num_headings = %d\n", num_headings);
//  for (i = 0; i < num_headings; i ++)
//    printf("headings[%d] = \"%s\"\n", i, headings[i]);

  heading = -1;
  update_links(document, &heading);
  update_links(toc, NULL);

  // Generate title pages and a table of contents...
  out = NULL;
  if (TitlePage)
  {
    write_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, -1);
    if (out != NULL)
      write_title(out, title, author, copyright, docnumber);
  }
  else
    write_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, -1);

  if (out != NULL)
    write_all(out, toc, 0);

  write_footer(&out, -1);

  // Then write each output file...
  heading = -1;
  write_doc(&out, document, 0, &heading, title, author, copyright, docnumber);

  if (out != NULL)
    write_footer(&out, heading);

  // Free memory...
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

  return (out == NULL);
}


/*
 * 'write_header()' - Output the standard "header" for a HTML file.
 */

static void
write_header(FILE   **out,	/* IO - Output file */
             uchar  *filename,	/* I - Output filename */
	     uchar  *title,	/* I - Title for document */
             uchar  *author,	/* I - Author for document */
             uchar  *copyright,	/* I - Copyright for document */
             uchar  *docnumber,	/* I - ID number for document */
	     int    heading)	/* I - Current heading */
{
  char		realname[1024];	/* Real filename */
  const char	*basename;	/* Filename without directory */
  static const char *families[] =/* Typeface names */
		{
		  "monospace",
		  "serif",
		  "sans-serif"
		};


  basename = file_basename((char *)filename);

  snprintf(realname, sizeof(realname), "%s/%s", OutputPath, basename);

  *out = fopen(realname, "wb");

  if (*out == NULL)
  {
    progress_error(HD_ERROR_WRITE_ERROR,
                   "Unable to create output file \"%s\" - %s.\n",
                   realname, strerror(errno));
    return;
  }

  fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
        "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n", *out);
  fputs("<HTML>\n", *out);
  fputs("<HEAD>\n", *out);
  if (title != NULL)
    fprintf(*out, "<TITLE>%s</TITLE>\n", title);
  if (author != NULL)
    fprintf(*out, "<META NAME=\"author\" CONTENT=\"%s\">\n", author);
  if (copyright != NULL)
    fprintf(*out, "<META NAME=\"copyright\" CONTENT=\"%s\">\n", copyright);
  if (docnumber != NULL)
    fprintf(*out, "<META NAME=\"docnumber\" CONTENT=\"%s\">\n", docnumber);
  fprintf(*out, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=iso-%s\">\n",
          _htmlCharSet);

  fputs("<LINK REL=\"Start\" HREF=\"index.html\">\n", *out);

  if (TitlePage)
    fputs("<LINK REL=\"Contents\" HREF=\"toc.html\">\n", *out);
  else
    fputs("<LINK REL=\"Contents\" HREF=\"index.html\">\n", *out);

  if (heading >= 0)
  {
    if (heading > 0)
      fprintf(*out, "<LINK REL=\"Prev\" HREF=\"%s.html\">\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<LINK REL=\"Next\" HREF=\"%s.html\">\n", headings[heading + 1]);
  }

  fputs("<STYLE TYPE=\"text/css\"><!--\n", *out);
  fprintf(*out, "BODY { font-family: %s }\n", families[_htmlBodyFont]);
  fprintf(*out, "H1 { font-family: %s }\n", families[_htmlHeadingFont]);
  fprintf(*out, "H2 { font-family: %s }\n", families[_htmlHeadingFont]);
  fprintf(*out, "H3 { font-family: %s }\n", families[_htmlHeadingFont]);
  fprintf(*out, "H4 { font-family: %s }\n", families[_htmlHeadingFont]);
  fprintf(*out, "H5 { font-family: %s }\n", families[_htmlHeadingFont]);
  fprintf(*out, "H6 { font-family: %s }\n", families[_htmlHeadingFont]);
  fputs("SUB { font-size: smaller }\n", *out);
  fputs("SUP { font-size: smaller }\n", *out);
  fputs("PRE { font-family: monospace }\n", *out);

  if (!LinkStyle)
    fputs("A { text-decoration: none }\n", *out);

  fputs("--></STYLE>\n", *out);
  fputs("</HEAD>\n", *out);

  if (BodyImage[0] != '\0')
    fprintf(*out, "<BODY BACKGROUND=\"%s\"", file_basename(BodyImage));
  else if (BodyColor[0] != '\0')
    fprintf(*out, "<BODY BGCOLOR=\"%s\"", BodyColor);
  else
    fputs("<BODY", *out);

  if (_htmlTextColor[0] != '\0')
    fprintf(*out, " TEXT=\"%s\"", _htmlTextColor);

  if (LinkColor[0] != '\0')
    fprintf(*out, " LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\"", LinkColor,
            LinkColor, LinkColor);

  fputs(">\n", *out);

  if (heading >= 0)
  {
    if (LogoImage[0] != '\0')
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    if (TitlePage)
      fputs("<A HREF=\"toc.html\">Contents</A>\n", *out);
    else
      fputs("<A HREF=\"index.html\">Contents</A>\n", *out);

    if (heading > 0)
      fprintf(*out, "<A HREF=\"%s.html\">Previous</A>\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<A HREF=\"%s.html\">Next</A>\n", headings[heading + 1]);

    fputs("<HR>\n", *out);
  }
}


/*
 * 'write_footer()' - Output the standard "footer" for a HTML file.
 */

static void
write_footer(FILE **out,	/* IO - Output file pointer */
	     int  heading)	/* I  - Current heading */
{
  if (*out == NULL)
    return;

  fputs("<HR>\n", *out);

  if (heading >= 0)
  {
    if (LogoImage[0] != '\0')
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    if (TitlePage)
      fputs("<A HREF=\"toc.html\">Contents</A>\n", *out);
    else
      fputs("<A HREF=\"index.html\">Contents</A>\n", *out);

    if (heading > 0)
      fprintf(*out, "<A HREF=\"%s.html\">Previous</A>\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<A HREF=\"%s.html\">Next</A>\n", headings[heading + 1]);
  }

  fputs("</BODY>\n", *out);
  fputs("</HTML>\n", *out);

  progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(*out));

  fclose(*out);
  *out = NULL;
}


/*
 * 'write_title()' - Write a title page...
 */

static void
write_title(FILE  *out,		/* I - Output file */
            uchar *title,	/* I - Title for document */
            uchar *author,	/* I - Author for document */
            uchar *copyright,	/* I - Copyright for document */
            uchar *docnumber)	/* I - ID number for document */
{
  FILE		*fp;		/* Title file */
  const char	*title_file;	/* Location of title file */
  tree_t	*t;		/* Title file document tree */


  if (out == NULL)
    return;

#ifdef WIN32
  if (TitleImage[0] &&
      stricmp(file_extension(TitleImage), "bmp") != 0 &&
      stricmp(file_extension(TitleImage), "gif") != 0 &&
      stricmp(file_extension(TitleImage), "jpg") != 0 &&
      stricmp(file_extension(TitleImage), "png") != 0)
#else
  if (TitleImage[0] &&
      strcmp(file_extension(TitleImage), "bmp") != 0 &&
      strcmp(file_extension(TitleImage), "gif") != 0 &&
      strcmp(file_extension(TitleImage), "jpg") != 0 &&
      strcmp(file_extension(TitleImage), "png") != 0)
#endif // WIN32
  {
    // Find the title page file...
    if ((title_file = file_find(Path, TitleImage)) == NULL)
    {
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to find title file \"%s\"!", TitleImage);
      return;
    }

    // Write a title page from HTML source...
    if ((fp = fopen(title_file, "rb")) == NULL)
    {
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open title file \"%s\" - %s!",
                     TitleImage, strerror(errno));
      return;
    }

    t = htmlReadFile(NULL, fp, file_directory(TitleImage));
    fclose(fp);

    write_all(out, t, 0);
    htmlDeleteTree(t);
  }
  else
  {
    // Write a "standard" title page with image...
    fputs("<CENTER>", out);

    if (TitleImage[0] != '\0')
    {
      image_t *img = image_load(TitleImage, !OutputColor);

      fprintf(out, "<IMG SRC=\"%s\" WIDTH=\"%d\" HEIGHT=\"%d\"><BR>\n",
              file_basename((char *)TitleImage), img->width, img->height);
    }

    if (title != NULL)
      fprintf(out, "<H1>%s</H1><BR>\n", title);
    else
      fputs("\n", out);

    if (docnumber != NULL)
      fprintf(out, "%s<BR>\n", docnumber);

    if (author != NULL)
      fprintf(out, "%s<BR>\n", author);

    if (copyright != NULL)
      fprintf(out, "%s<BR>\n", copyright);

    fputs("</CENTER>\n", out);
  }
}


/*
 * 'write_all()' - Write all markup text for the given tree.
 */

static int			/* O - Current column */
write_all(FILE   *out,		/* I - Output file */
          tree_t *t,		/* I - Document tree */
          int    col)		/* I - Current column */
{
  if (out == NULL)
    return (0);

  while (t != NULL)
  {
    col = write_node(out, t, col);

    if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
      col = write_all(out, t->child, col);

    col = write_nodeclose(out, t, col);

    t = t->next;
  }

  return (col);
}


/*
 * 'write_doc()' - Write the entire document.
 */

static int				// O - Current column
write_doc(FILE   **out,			// I - Output file
          tree_t *t,			// I - Document tree
          int    col,			// I - Current column
          int    *heading,		// IO - Current heading
	  uchar  *title,		// I  - Title
          uchar  *author,		// I  - Author
	  uchar  *copyright,		// I  - Copyright
	  uchar  *docnumber)		// I  - Document number
{
  uchar	filename[1024];			// Filename


  while (t != NULL)
  {
    if (t->markup >= MARKUP_H1 && t->markup < (MARKUP_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
    {
      if (heading >= 0)
        write_footer(out, *heading);

      (*heading) ++;

      if (*heading >= 0)
      {
	snprintf((char *)filename, sizeof(filename), "%s.html",
	         headings[*heading]);
	write_header(out, filename, title, author, copyright, docnumber,
                     *heading);
      }
    }

    col = write_node(*out, t, col);

    if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
      col = write_doc(out, t->child, col, heading,
                      title, author, copyright, docnumber);

    col = write_nodeclose(*out, t, col);

    t = t->next;
  }

  return (col);
}


/*
 * 'write_node()' - Write a single tree node.
 */

static int			/* O - Current column */
write_node(FILE   *out,		/* I - Output file */
           tree_t *t,		/* I - Document tree node */
           int    col)		/* I - Current column */
{
  int		i;		/* Looping var */
  uchar		*ptr,		/* Pointer to output string */
		*src,		/* Source image */
		newsrc[1024];	/* New source image filename */


  if (out == NULL)
    return (0);

  switch (t->markup)
  {
    case MARKUP_NONE :
        if (t->data == NULL)
	  break;

	if (t->preformatted)
	{
          for (ptr = t->data; *ptr != '\0'; ptr ++)
            fputs((char *)iso8859(*ptr), out);

	  if (t->data[strlen((char *)t->data) - 1] == '\n')
            col = 0;
	  else
            col += strlen((char *)t->data);
	}
	else
	{
	  if ((col + strlen((char *)t->data)) > 72 && col > 0)
	  {
            putc('\n', out);
            col = 0;
	  }

          for (ptr = t->data; *ptr != '\0'; ptr ++)
            fputs((char *)iso8859(*ptr), out);

	  col += strlen((char *)t->data);

	  if (col > 72)
	  {
            putc('\n', out);
            col = 0;
	  }
	}
	break;

    case MARKUP_COMMENT :
    case MARKUP_UNKNOWN :
        fprintf(out, "\n<!--%s-->\n", t->data);
	break;

    case MARKUP_AREA :
    case MARKUP_BODY :
    case MARKUP_DOCTYPE :
    case MARKUP_ERROR :
    case MARKUP_FILE :
    case MARKUP_HEAD :
    case MARKUP_HTML :
    case MARKUP_MAP :
    case MARKUP_META :
    case MARKUP_TITLE :
        break;

    case MARKUP_BR :
    case MARKUP_CENTER :
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
    case MARKUP_H8 :
    case MARKUP_H9 :
    case MARKUP_H10 :
    case MARKUP_H11 :
    case MARKUP_H12 :
    case MARKUP_H13 :
    case MARKUP_H14 :
    case MARKUP_H15 :
    case MARKUP_HR :
    case MARKUP_LI :
    case MARKUP_OL :
    case MARKUP_P :
    case MARKUP_PRE :
    case MARKUP_TABLE :
    case MARKUP_TR :
    case MARKUP_UL :
        if (col > 0)
        {
          putc('\n', out);
          col = 0;
        }

    default :
	if (t->markup == MARKUP_IMG &&
            (src = htmlGetVariable(t, (uchar *)"REALSRC")) != NULL)
	{
	 /*
          * Update local images...
          */

          if (file_method((char *)src) == NULL &&
              src[0] != '/' && src[0] != '\\' &&
	      (!isalpha(src[0]) || src[1] != ':'))
          {
            image_copy((char *)src, OutputPath);
            strcpy((char *)newsrc, file_basename((char *)src));
            htmlSetVariable(t, (uchar *)"SRC", newsrc);
          }
	}

        if (t->markup != MARKUP_EMBED)
	{
	  col += fprintf(out, "<%s", _htmlMarkups[t->markup]);
	  for (i = 0; i < t->nvars; i ++)
	  {
	    if (strcasecmp((char *)t->vars[i].name, "BREAK") == 0 &&
	        t->markup == MARKUP_HR)
	      continue;

	    if (strcasecmp((char *)t->vars[i].name, "REALSRC") == 0 &&
	        t->markup == MARKUP_IMG)
	      continue;

            if (strncasecmp((char *)t->vars[i].name, "_HD_", 4) == 0)
	      continue;

	    if (col > 72 && !t->preformatted)
	    {
              putc('\n', out);
              col = 0;
	    }

            if (col > 0)
            {
              putc(' ', out);
              col ++;
            }

	    if (t->vars[i].value == NULL)
              col += fprintf(out, "%s", t->vars[i].name);
	    else if (strchr((char *)t->vars[i].value, '\"') != NULL)
              col += fprintf(out, "%s=\'%s\'", t->vars[i].name, t->vars[i].value);
	    else
              col += fprintf(out, "%s=\"%s\"", t->vars[i].name, t->vars[i].value);
	  }

	  putc('>', out);
	  col ++;

	  if (col > 72 && !t->preformatted)
	  {
	    putc('\n', out);
	    col = 0;
	  }
	}
	break;
  }

  return (col);
}


/*
 * 'write_nodeclose()' - Close a single tree node.
 */

static int			/* O - Current column */
write_nodeclose(FILE   *out,	/* I - Output file */
                tree_t *t,	/* I - Document tree node */
                int    col)	/* I - Current column */
{
  if (out == NULL)
    return (0);

  if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
  {
    if (col > 72 && !t->preformatted)
    {
      putc('\n', out);
      col = 0;
    }

    switch (t->markup)
    {
      case MARKUP_BODY :
      case MARKUP_ERROR :
      case MARKUP_FILE :
      case MARKUP_HEAD :
      case MARKUP_HTML :
      case MARKUP_NONE :
      case MARKUP_TITLE :

      case MARKUP_APPLET :
      case MARKUP_AREA :
      case MARKUP_BR :
      case MARKUP_COMMENT :
      case MARKUP_DOCTYPE :
      case MARKUP_EMBED :
      case MARKUP_HR :
      case MARKUP_IMG :
      case MARKUP_INPUT :
      case MARKUP_ISINDEX :
      case MARKUP_LINK :
      case MARKUP_META :
      case MARKUP_NOBR :
      case MARKUP_SPACER :
      case MARKUP_WBR :
      case MARKUP_UNKNOWN :
          break;

      case MARKUP_CENTER :
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
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
      case MARKUP_LI :
      case MARKUP_OL :
      case MARKUP_P :
      case MARKUP_PRE :
      case MARKUP_TABLE :
      case MARKUP_TR :
      case MARKUP_UL :
          fprintf(out, "</%s>\n", _htmlMarkups[t->markup]);
          col = 0;
          break;

      default :
          col += fprintf(out, "</%s>", _htmlMarkups[t->markup]);
	  break;
    }
  }

  return (col);
}


/*
 * 'get_title()' - Get the title string for the given document...
 */

static uchar *		/* O - Title string */
get_title(tree_t *doc)	/* I - Document tree */
{
  uchar	*temp;		/* Temporary pointer to title */


  while (doc != NULL)
  {
    if (doc->markup == MARKUP_TITLE)
      return (htmlGetText(doc->child));
    else if (doc->child != NULL)
      if ((temp = get_title(doc->child)) != NULL)
        return (temp);

    doc = doc->next;
  }

  return (NULL);
}


//
// 'add_heading()' - Add a heading to the list of headings...
//

static void
add_heading(tree_t *t)			// I - Heading node
{
  int	i,				// Looping var
	count;				// Count of headings with this name
  uchar	*heading,			// Heading text for this node
	*ptr,				// Pointer into text
	*ptr2,				// Second pointer into text
	s[1024],			// New text if we have a conflict
	**temp;				// New heading array pointer


  // Start by getting the heading text...
  heading = htmlGetText(t->child);
  if (!heading || !*heading)
    return;				// Nothing to do!

  // Sanitize the text...
  for (ptr = heading; *ptr;)
    if (!isalnum(*ptr))
    {
      // Remove anything but letters and numbers from the filename
      for (ptr2 = ptr; *ptr2; ptr2 ++)
        *ptr2 = ptr2[1];

      *ptr2 = '\0';
    }
    else
      ptr ++;

  // Now loop through the existing headings and check for dups...
  for (ptr = heading, i = 0, count = 0; i < num_headings; i ++)
    if (strcmp((char *)headings[i], (char *)ptr) == 0)
    {
      // Create a new instance of the heading...
      count ++;
      snprintf((char *)s, sizeof(s), "%s%d", heading, count);
      ptr = s;
    }

  // Now add the heading...
  if (num_headings >= alloc_headings)
  {
    // Allocate more headings...
    alloc_headings += ALLOC_HEADINGS;

    if (num_headings == 0)
      temp = (uchar **)malloc(sizeof(uchar *) * alloc_headings);
    else
      temp = (uchar **)realloc(headings, sizeof(uchar *) * alloc_headings);

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

  if (ptr == heading)
  {
    // Reuse the already-allocated string...
    headings[num_headings] = ptr;
  }
  else
  {
    // Make a copy of the string "s" and free the old heading string...
    headings[num_headings] = (uchar *)strdup((char *)s);
    free(heading);
  }

  num_headings ++;
}


/*
 * 'add_link()' - Add a named link...
 */

static void
add_link(uchar *name)		/* I - Name of link */
{
  uchar		*filename;	/* File for link */
  link_t	*temp;		/* New name */


  if (num_headings)
    filename = headings[num_headings - 1];
  else
    filename = (uchar *)"noheading";

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
        temp = (link_t *)malloc(sizeof(link_t) * alloc_links);
      else
        temp = (link_t *)realloc(links, sizeof(link_t) * alloc_links);

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

    strncpy((char *)temp->name, (char *)name, sizeof(temp->name) - 1);
    temp->name[sizeof(temp->name) - 1] = '\0';
    temp->filename = filename;

    if (num_links > 1)
      qsort(links, num_links, sizeof(link_t), (compare_func_t)compare_links);
  }
}


/*
 * 'find_link()' - Find a named link...
 */

static link_t *
find_link(uchar *name)		/* I - Name to find */
{
  uchar		*target;	/* Pointer to target name portion */
  link_t	key,		/* Search key */
		*match;		/* Matching name entry */


  if (name == NULL || num_links == 0)
    return (NULL);

  if ((target = (uchar *)file_target((char *)name)) == NULL)
    return (NULL);

  strncpy((char *)key.name, (char *)target, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';
  match = (link_t *)bsearch(&key, links, num_links, sizeof(link_t),
                            (compare_func_t)compare_links);

  return (match);
}


/*
 * 'compare_links()' - Compare two named links.
 */

static int			/* O - 0 = equal, -1 or 1 = not equal */
compare_links(link_t *n1,	/* I - First name */
              link_t *n2)	/* I - Second name */
{
  return (strcasecmp((char *)n1->name, (char *)n2->name));
}


/*
 * 'scan_links()' - Scan a document for link targets, and keep track of
 *                  the files they are in...
 */

static void
scan_links(tree_t *t)		/* I - Document tree */
{
  uchar	*name;			/* Name of link */


  while (t != NULL)
  {
    if (t->markup >= MARKUP_H1 && t->markup < (MARKUP_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
      add_heading(t);

    if (t->markup == MARKUP_A &&
        (name = htmlGetVariable(t, (uchar *)"NAME")) != NULL)
      add_link(name);

    if (t->child != NULL)
      scan_links(t->child);

    t = t->next;
  }
}


/*
 * 'update_links()' - Update links as needed.
 */

static void
update_links(tree_t *t,		/* I - Document tree */
             int    *heading)	/* I - Current heading */
{
  link_t	*link;		/* Link */
  uchar		*href;		/* Reference name */
  uchar		newhref[1024];	/* New reference name */
  uchar		*filename;	/* Current filename */


  // Scan the document, rewriting HREF's as needed...
  while (t != NULL)
  {
    if (t->markup >= MARKUP_H1 && t->markup < (MARKUP_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL && heading)
      (*heading) ++;

    // Figure out the current filename based upon the current heading number...
    if (!heading || *heading < 0 || *heading >= num_headings)
      filename = (uchar *)"noheading";
    else
      filename = headings[*heading];

    if (t->markup == MARKUP_A &&
        (href = htmlGetVariable(t, (uchar *)"HREF")) != NULL)
    {
      // Update this link as needed...
      if (href[0] == '#' && (link = find_link(href)) != NULL)
      {
        // The filename in the link structure is a copy of the heading
	// pointer...
        if (filename != link->filename)
	{
	  // Rewrite using the new name...
	  snprintf((char *)newhref, sizeof(newhref), "%s.html%s",
	           link->filename, href);
	  htmlSetVariable(t, (uchar *)"HREF", newhref);
	}
      }
    }

    // Descend the tree as needed...
    if (t->child != NULL)
      update_links(t->child, heading);

    // Move to the next node at this level...
    t = t->next;
  }
}


//
// End of "$Id: htmlsep.cxx,v 1.1.2.5 2003/12/18 21:55:42 mike Exp $".
//
