/*
 * "$Id: html.cxx,v 1.9 1999/11/22 18:07:40 mike Exp $"
 *
 *   HTML exporting functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-1999 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   html_export()   - Export to HTML...
 *   count_headers() - Count the number of first and second level headings
 *                     in a document tree.
 *   write_header()  - Output the standard "header" for a HTML file.
 *   write_footer()  - Output the standard "footer" for a HTML file.
 *   write_all()     - Write all markup text for the given tree.
 *   get_title()     - Get the title string for the given document...
 *   add_link()      - Add a named link...
 *   find_link()     - Find a named link...
 *   compare_links() - Compare two named links.
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include <ctype.h>


/*
 * Named link structure...
 */

typedef struct
{
  uchar		*filename;	/* File for link */
  uchar		name[124];	/* Reference name */
} link_t;


/*
 * Local globals...
 */


static int	num_links;
static link_t	links[MAX_LINKS];


/*
 * Local functions...
 */

static void	write_header(FILE **out, uchar *filename, uchar *title,
		             uchar *author, uchar *copyright, uchar *docnumber,
			     tree_t *t);
static void	write_footer(FILE **out, tree_t *t);
static void	write_title(FILE *out, uchar *title, uchar *author,
		            uchar *copyright, uchar *docnumber);
static int	write_all(FILE *out, tree_t *t, int col);
static uchar	*get_title(tree_t *doc);

static void	add_link(uchar *name, uchar *filename);
static link_t	*find_link(uchar *name);
static int	compare_links(link_t *n1, link_t *n2);
static void	scan_links(tree_t *t, uchar *filename);
static void	update_links(tree_t *t, uchar *filename);


/*
 * 'html_export()' - Export to HTML...
 */

int				/* O - 0 = success, -1 = failure */
html_export(tree_t *document,	/* I - Document to export */
            tree_t *toc)	/* I - Table of contents for document */
{
  uchar	*title,			/* Title text */
	*author,		/* Author name */
	*copyright,		/* Copyright text */
	*docnumber;		/* Document number */
  FILE	*out;			/* Output file */


 /*
  * Copy logo and title images...
  */

  if (OutputFiles && LogoImage[0] != '\0')
    image_copy(LogoImage, OutputPath);

  if (OutputFiles && TitleImage[0] != '\0' && TitlePage)
    image_copy(TitleImage, OutputPath);

 /*
  * Get document strings...
  */

  title     = get_title(document);
  author    = htmlGetMeta(document, (uchar *)"author");
  copyright = htmlGetMeta(document, (uchar *)"copyright");
  docnumber = htmlGetMeta(document, (uchar *)"docnumber");

 /*
  * Scan for all links in the document, and then update them...
  */

  num_links = 0;

  scan_links(document, NULL);
  update_links(document, NULL);
  update_links(toc, NULL);

 /*
  * Generate title pages and a table of contents...
  */

  out = NULL;
  if (TitlePage)
  {
    write_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, NULL);
    write_title(out, title, author, copyright, docnumber);

    write_footer(&out, NULL);
    write_header(&out, (uchar *)"toc.html", title, author, copyright,
                 docnumber, NULL);
  }
  else
    write_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, NULL);

  write_all(out, toc, 0);
  write_footer(&out, NULL);

 /*
  * Then write each output file...
  */

  while (document != NULL)
  {
    write_header(&out, htmlGetVariable(document, (uchar *)"FILENAME"),
                 title, author, copyright, docnumber, document);
    write_all(out, document->child, 0);
    write_footer(&out, document);

    document = document->next;
  }

  if (!OutputFiles && out != stdout)
  {
    fputs("</BODY>\n", out);
    fputs("</HTML>\n", out);

    fclose(out);
  }

  if (title != NULL)
    free(title);

  return (0);
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
	     tree_t *t)		/* I - Current document file */
{
  char	realname[1024];		/* Real filename */
  char	*basename;		/* Filename without directory */
  int	newfile;		/* Non-zero if this is a new file */
  static char	*families[] =	/* Typeface names */
		{
		  "monospace",
		  "serif",
		  "sans-serif"
		};


  if (OutputFiles)
  {
    newfile  = 1;
    basename = file_basename((char *)filename);

    sprintf(realname, "%s/%s", OutputPath, basename);

    *out = fopen(realname, "w");
  }
  else if (OutputPath[0] != '\0')
  {
    if (*out == NULL)
    {
      *out    = fopen(OutputPath, "w");
      newfile = 1;
    }
    else
      newfile = 0;
  }
  else
  {
    if (*out == NULL)
    {
      *out    = stdout;
      newfile = 1;
    }
    else
      newfile = 0;
  }

  if (*out == NULL)
  {
    progress_error("Unable to create output file \"%s\" - %s.\n",
                   OutputFiles ? realname : OutputPath,
		   strerror(errno));
    return;
  }

  if (newfile)
  {
    fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
          "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n", *out);
    fputs("<HTML>\n", *out);
    fputs("<HEAD>\n", *out);
    fprintf(*out, "<TITLE>%s</TITLE>\n", title);
    if (author != NULL)
      fprintf(*out, "<META NAME=\"AUTHOR\" CONTENT=\"%s\">\n", author);
    if (copyright != NULL)
      fprintf(*out, "<META NAME=\"COPYRIGHT\" CONTENT=\"%s\">\n", copyright);
    if (docnumber != NULL)
      fprintf(*out, "<META NAME=\"DOCNUMBER\" CONTENT=\"%s\">\n", docnumber);
    fprintf(*out, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-%s\">\n",
            _htmlCharSet);

    fputs("<STYLE>\n", *out);
    fprintf(*out, "BODY { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlBodyFont], _htmlSizes[SIZE_P]);
    fprintf(*out, "H1 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "H2 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "H3 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "H4 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "H5 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "H6 { font-family: %s; font-size: %.1fpt }\n",
            families[_htmlHeadingFont], _htmlSizes[SIZE_H1]);
    fprintf(*out, "SUB { font-size: %.1fpt }\n", _htmlSizes[SIZE_SUB]);
    fprintf(*out, "SUP { font-size: %.1fpt }\n", _htmlSizes[SIZE_SUB]);
    fprintf(*out, "PRE { font-size: %.1fpt }\n", _htmlSizes[SIZE_PRE]);
    fputs("</STYLE>\n", *out);
    fputs("</HEAD>\n", *out);

    if (BodyImage[0] != '\0')
      fprintf(*out, "<BODY BACKGROUND=\"%s\"", file_basename(BodyImage));
    else if (BodyColor[0] != '\0')
      fprintf(*out, "<BODY BGCOLOR=\"%s\"", BodyColor);
    else
      fputs("<BODY", *out);

    if (_htmlTextColor[0] != '\0')
      fprintf(*out, " TEXT=\"%s\">\n", _htmlTextColor);
    else
      fputs(">\n", *out);
  }
  else
    fputs("<HR>\n", *out);

  if (OutputFiles && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    if (LogoImage[0] != '\0')
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    fputs("<A HREF=toc.html>Contents</a>\n", *out);

    if (t->prev != NULL)
      fprintf(*out, "<A HREF=\"%s\">Previous</a>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<A HREF=\"%s\">Next</a>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"FILENAME")));

    fputs("<HR>\n", *out);
  }
}


/*
 * 'write_footer()' - Output the standard "footer" for a HTML file.
 */

static void
write_footer(FILE **out,	/* IO - Output file pointer */
	     tree_t *t)		/* I - Current document file */
{
  if (*out == NULL)
    return;

  if (OutputFiles && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    fputs("<HR>\n", *out);

    if (LogoImage[0] != '\0')
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    fputs("<A HREF=\"toc.html\">Contents</a>\n", *out);

    if (t->prev != NULL)
      fprintf(*out, "<A HREF=\"%s\">Previous</a>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<A HREF=\"%s\">Next</a>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"FILENAME")));
  }

  if (OutputFiles)
  {
    fputs("</BODY>\n", *out);
    fputs("</HTML>\n", *out);

    fclose(*out);
    *out = NULL;
  }
}


/*
 * 'write_title()' - Write a title page...
 */

static void
write_title(FILE *out,		/* I - Output file */
            uchar  *title,	/* I - Title for document */
            uchar  *author,	/* I - Author for document */
            uchar  *copyright,	/* I - Copyright for document */
            uchar  *docnumber)	/* I - ID number for document */
{
  if (out == NULL)
    return;

  if (OutputFiles)
    fputs("<CENTER><A HREF=\"toc.html\">", out);
  else
    fputs("<CENTER><A HREF=\"#CONTENTS\">", out);

  if (TitleImage[0] != '\0')
  {
    if (OutputFiles)
      fprintf(out, "<IMG SRC=\"%s\" BORDER=\"0\"><BR>\n",
              file_basename((char *)TitleImage));
    else
      fprintf(out, "<IMG SRC=\"%s\" BORDER=\"0\"><BR>\n", TitleImage);
  }

  if (title != NULL)
    fprintf(out, "<H1>%s</H1></A><BR>\n", title);
  else
    fputs("</A>\n", out);

  if (docnumber != NULL)
    fprintf(out, "%s<BR>\n", docnumber);

  if (author != NULL)
    fprintf(out, "%s<BR>\n", author);

  if (copyright != NULL)
    fprintf(out, "%s<BR>\n", copyright);

  fputs("</CENTER>\n", out);
}


/*
 * 'write_all()' - Write all markup text for the given tree.
 */

static int			/* O - Current column */
write_all(FILE   *out,		/* I - Output file */
          tree_t *t,		/* I - Document tree */
          int    col)		/* I - Current column */
{
  int		i;		/* Looping var */
  uchar		*ptr,		/* Pointer to output string */
		*src,		/* Source image */
		newsrc[1024];	/* New source image filename */


  if (out == NULL)
    return (0);

  while (t != NULL)
  {
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
          fprintf(out, "\n<!--%s-->\n", t->data);
	  break;

      case MARKUP_AREA :
      case MARKUP_HEAD :
      case MARKUP_HTML :
      case MARKUP_MAP :
      case MARKUP_META :
      case MARKUP_TITLE :
      case MARKUP_BODY :
      case MARKUP_ERROR :
      case MARKUP_FILE :
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
	  if (t->markup == MARKUP_IMG && OutputFiles &&
              (src = htmlGetVariable(t, (uchar *)"SRC")) != NULL)
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

    if (t->child != NULL &&
        t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
    {
      col = write_all(out, t->child, col);

      if (col > 72 && !t->preformatted)
      {
	putc('\n', out);
	col = 0;
      }

      switch (t->markup)
      {
	case MARKUP_AREA :
	case MARKUP_HEAD :
	case MARKUP_HTML :
	case MARKUP_MAP :
	case MARKUP_META :
	case MARKUP_TITLE :
	case MARKUP_BODY :
	case MARKUP_ERROR :
	case MARKUP_FILE :
	case MARKUP_EMBED :
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

    t = t->next;
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


/*
 * 'add_link()' - Add a named link...
 */

static void
add_link(uchar *name,		/* I - Name of link */
         uchar *filename)	/* I - File for link */
{
  link_t	*temp;		/* New name */


  if ((temp = find_link(name)) != NULL)
    temp->filename = filename;
  else if (num_links < MAX_LINKS)
  {
    temp = links + num_links;
    num_links ++;

    strncpy((char *)temp->name, (char *)name, sizeof(temp->name) - 1);
    temp->name[sizeof(temp->name) - 1] = '\0';
    temp->filename = filename;

    if (num_links > 1)
      qsort(links, num_links, sizeof(link_t),
            (int (*)(const void *, const void *))compare_links);
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
                            (int (*)(const void *, const void *))compare_links);

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
scan_links(tree_t *t,		/* I - Document tree */
           uchar  *filename)	/* I - Filename */
{
  uchar	*name;			/* Name of link */


  while (t != NULL)
  {
    if (t->markup == MARKUP_FILE)
      scan_links(t->child, (uchar *)file_basename((char *)htmlGetVariable(t, (uchar *)"FILENAME")));
    else if (t->markup == MARKUP_A &&
             (name = htmlGetVariable(t, (uchar *)"NAME")) != NULL)
    {
      add_link(name, filename);
      scan_links(t->child, filename);
    }
    else if (t->child != NULL)
      scan_links(t->child, filename);

    t = t->next;
  }
}


/*
 * 'update_links()' - Update links as needed.
 */

static void
update_links(tree_t *t,		/* I - Document tree */
             uchar  *filename)	/* I - Current filename */
{
  link_t	*link;		/* Link */
  uchar		*href;		/* Reference name */
  uchar		newhref[1024];	/* New reference name */


  filename = (uchar *)file_basename((char *)filename);

  if (OutputFiles)
  {
   /*
    * Need to preserve/add filenames.
    */

    while (t != NULL)
    {
      if (t->markup == MARKUP_A &&
          (href = htmlGetVariable(t, (uchar *)"HREF")) != NULL)
      {
       /*
        * Update this link as needed...
	*/

        if (href[0] == '#' &&
	    (link = find_link(href)) != NULL)
	{
#if defined(WIN32) || defined(__EMX__)
	  if (filename == NULL ||
	      strcasecmp((char *)filename, (char *)link->filename) != 0)
#else
          if (filename == NULL ||
	      strcmp((char *)filename, (char *)link->filename) != 0)
#endif /* WIN32 || __EMX__ */
	  {
	    sprintf((char *)newhref, "%s%s", link->filename, href);
	    htmlSetVariable(t, (uchar *)"HREF", newhref);
	  }
	}
      }

      if (t->child != NULL)
      {
        if (t->markup == MARKUP_FILE)
          update_links(t->child, htmlGetVariable(t, (uchar *)"FILENAME"));
	else
          update_links(t->child, filename);
      }

      t = t->next;
    }
  }
  else
  {
   /*
    * Need to strip filenames.
    */

    while (t != NULL)
    {
      if (t->markup == MARKUP_A &&
          (href = htmlGetVariable(t, (uchar *)"HREF")) != NULL)
      {
       /*
        * Update this link as needed...
	*/

        if (href[0] != '#' && file_method((char *)href) == NULL &&
	    (link = find_link(href)) != NULL)
	{
	  sprintf((char *)newhref, "#%s", link->name);
	  htmlSetVariable(t, (uchar *)"HREF", newhref);
	}
      }

      if (t->child != NULL)
        update_links(t->child, filename);

      t = t->next;
    }
  }
}


/*
 * End of "$Id: html.cxx,v 1.9 1999/11/22 18:07:40 mike Exp $".
 */
