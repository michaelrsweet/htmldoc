/*
 * "$Id$"
 *
 *   HTML exporting functions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 2011-2013 by Michael R Sweet.
 *   Copyright 1997-2010 by Easy Software Products.  All rights reserved.
 *
 *   This program is free software.  Distribution and use rights are outlined in
 *   the file "COPYING.txt".
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


static int	num_links = 0,
		alloc_links = 0;
static link_t	*links;


/*
 * Local functions...
 */

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static void	write_header(FILE **out, uchar *filename, uchar *title,
		             uchar *author, uchar *copyright, uchar *docnumber,
			     tree_t *t);
static void	write_footer(FILE **out, tree_t *t);
static void	write_title(FILE *out, uchar *title, uchar *author,
		            uchar *copyright, uchar *docnumber);
static int	write_all(FILE *out, tree_t *t, int col);
static int	write_node(FILE *out, tree_t *t, int col);
static int	write_nodeclose(FILE *out, tree_t *t, int col);
static int	write_toc(FILE *out, tree_t *t, int col);
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

  if (OutputFiles)
  {
    if (LogoImage[0])
      image_copy(LogoImage, file_find(LogoImage, Path), OutputPath);

    for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (HFImage[hfi][0])
        image_copy(HFImage[hfi], file_find(HFImage[hfi], Path), OutputPath);
  }

  if (OutputFiles && TitleImage[0] && TitlePage &&
#ifdef WIN32
      (stricmp(file_extension(TitleImage), "bmp") == 0 ||
       stricmp(file_extension(TitleImage), "gif") == 0 ||
       stricmp(file_extension(TitleImage), "jpg") == 0 ||
       stricmp(file_extension(TitleImage), "png") == 0))
#else
      (strcmp(file_extension(TitleImage), "bmp") == 0 ||
       strcmp(file_extension(TitleImage), "gif") == 0 ||
       strcmp(file_extension(TitleImage), "jpg") == 0 ||
       strcmp(file_extension(TitleImage), "png") == 0))
#endif // WIN32
    image_copy(TitleImage, file_find(TitleImage, Path), OutputPath);

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

  num_links   = 0;
  alloc_links = 0;
  links       = NULL;

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
    if (out != NULL)
      write_title(out, title, author, copyright, docnumber);

    write_footer(&out, NULL);
    write_header(&out, (uchar *)"toc.html", title, author, copyright,
                 docnumber, NULL);
  }
  else
    write_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, NULL);

  if (out != NULL)
    write_toc(out, toc, 0);
  write_footer(&out, NULL);

 /*
  * Then write each output file...
  */

  while (document != NULL)
  {
    write_header(&out, htmlGetVariable(document, (uchar *)"_HD_FILENAME"),
                 title, author, copyright, docnumber, document);
    if (out != NULL)
      write_all(out, document->child, 0);
    write_footer(&out, document);

    document = document->next;
  }

  if (!OutputFiles && out != stdout && out != NULL)
  {
    fputs("</BODY>\n", out);
    fputs("</HTML>\n", out);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

    fclose(out);
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
	     tree_t *t)		/* I - Current document file */
{
  char		realname[1024];	/* Real filename */
  const char	*basename;	/* Filename without directory */
  int		newfile;	/* Non-zero if this is a new file */
  static const char *families[] =/* Typeface names */
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
    basename = file_basename((char *)filename);

    snprintf(realname, sizeof(realname), "%s/%s", OutputPath, basename);

    *out = fopen(realname, "wb");
  }
  else if (OutputPath[0])
  {
    if (*out == NULL)
    {
      *out    = fopen(OutputPath, "wb");
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
    progress_error(HD_ERROR_WRITE_ERROR,
                   "Unable to create output file \"%s\" - %s.\n",
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

    if (OutputFiles)
    {
      fputs("<LINK REL=\"Start\" HREF=\"index.html\">\n", *out);

      if (TitlePage)
	fputs("<LINK REL=\"Contents\" HREF=\"toc.html\">\n", *out);
      else
	fputs("<LINK REL=\"Contents\" HREF=\"index.html\">\n", *out);

      if (t)
      {
	if (t->prev != NULL)
	  fprintf(*out, "<LINK REL=\"Prev\" HREF=\"%s\">\n",
        	  file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

	if (t->next != NULL)
	  fprintf(*out, "<LINK REL=\"Next\" HREF=\"%s\">\n",
        	  file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));
      }
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

    if (BodyImage[0])
      fprintf(*out, "<BODY BACKGROUND=\"%s\"", file_basename(BodyImage));
    else if (BodyColor[0])
      fprintf(*out, "<BODY BGCOLOR=\"%s\"", BodyColor);
    else
      fputs("<BODY", *out);

    if (_htmlTextColor[0])
      fprintf(*out, " TEXT=\"%s\"", _htmlTextColor);

    if (LinkColor[0])
      fprintf(*out, " LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\"", LinkColor,
              LinkColor, LinkColor);

    fputs(">\n", *out);
  }
  else
    fputs("<HR NOSHADE>\n", *out);

  if (OutputFiles && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    if (LogoImage[0])
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi][0])
        fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(HFImage[hfi]));

    if (TitlePage)
      fputs("<A HREF=\"toc.html\">Contents</A>\n", *out);
    else
      fputs("<A HREF=\"index.html\">Contents</A>\n", *out);

    if (t->prev != NULL)
      fprintf(*out, "<A HREF=\"%s\">Previous</A>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<A HREF=\"%s\">Next</A>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));

    fputs("<HR NOSHADE>\n", *out);
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
    fputs("<HR NOSHADE>\n", *out);

    if (LogoImage[0])
      fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(LogoImage));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi][0])
        fprintf(*out, "<IMG SRC=\"%s\">\n", file_basename(HFImage[hfi]));

    if (TitlePage)
      fputs("<A HREF=\"toc.html\">Contents</A>\n", *out);
    else
      fputs("<A HREF=\"index.html\">Contents</A>\n", *out);


    if (t->prev != NULL)
      fprintf(*out, "<A HREF=\"%s\">Previous</A>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<A HREF=\"%s\">Next</A>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));
  }

  if (OutputFiles)
  {
    fputs("</BODY>\n", *out);
    fputs("</HTML>\n", *out);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(*out));

    fclose(*out);
    *out = NULL;
  }
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
    htmlFixLinks(t, t, (uchar *)file_directory(TitleImage));
    fclose(fp);

    write_all(out, t, 0);
    htmlDeleteTree(t);
  }
  else
  {
    // Write a "standard" title page with image...
    if (OutputFiles)
      fputs("<CENTER><A HREF=\"toc.html\">", out);
    else
      fputs("<CENTER><A HREF=\"#CONTENTS\">", out);

    if (TitleImage[0])
    {
      image_t *img = image_load(TitleImage, !OutputColor);

      if (OutputFiles)
	fprintf(out, "<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"%d\" HEIGHT=\"%d\" "
	             "ALT=\"%s\"><BR>\n",
        	file_basename((char *)TitleImage), img->width, img->height,
		title ? (char *)title : "");
      else
	fprintf(out, "<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"%d\" HEIGHT=\"%d\" "
	             "ALT=\"%s\"><BR>\n",
        	TitleImage, img->width, img->height,
		title ? (char *)title : "");
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
 * 'write_node()' - Write a single tree node.
 */

static int			/* O - Current column */
write_node(FILE   *out,		/* I - Output file */
           tree_t *t,		/* I - Document tree node */
           int    col)		/* I - Current column */
{
  int		i;		/* Looping var */
  uchar		*ptr,		/* Pointer to output string */
		*entity,	/* Entity string */
		*src,		/* Source image */
		*realsrc,	/* Real source image */
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
          for (ptr = t->data; *ptr; ptr ++)
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

          for (ptr = t->data; *ptr; ptr ++)
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
        fputs("\n<!--", out);
	for (ptr = t->data; *ptr; ptr ++)
	  fputs((char *)iso8859(*ptr), out);
	fputs("-->\n", out);
	col = 0;
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
	if (t->markup == MARKUP_IMG && OutputFiles &&
            (src = htmlGetVariable(t, (uchar *)"SRC")) != NULL &&
            (realsrc = htmlGetVariable(t, (uchar *)"REALSRC")) != NULL)
	{
	 /*
          * Update and copy local images...
          */

          if (file_method((char *)src) == NULL &&
              src[0] != '/' && src[0] != '\\' &&
	      (!isalpha(src[0]) || src[1] != ':'))
          {
            image_copy((char *)src, (char *)realsrc, OutputPath);
            strlcpy((char *)newsrc, file_basename((char *)src), sizeof(newsrc));
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
	    else
	    {
	      col += fprintf(out, "%s=\"", t->vars[i].name);
	      for (ptr = t->vars[i].value; *ptr; ptr ++)
	      {
		entity = iso8859(*ptr);
		fputs((char *)entity, out);
		col += strlen((char *)entity);
	      }

	      putc('\"', out);
	      col ++;
	    }
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
 * 'write_toc()' - Write all markup text for the given table-of-contents.
 */

static int			/* O - Current column */
write_toc(FILE   *out,		/* I - Output file */
          tree_t *t,		/* I - Document tree */
          int    col)		/* I - Current column */
{
  if (out == NULL)
    return (0);

  while (t != NULL)
  {
    if (htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
    {
      col = write_node(out, t, col);

      if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
	col = write_toc(out, t->child, col);

      col = write_nodeclose(out, t, col);
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

    strlcpy((char *)temp->name, (char *)name, sizeof(temp->name));
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

  strlcpy((char *)key.name, (char *)target, sizeof(key.name));
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
scan_links(tree_t *t,		/* I - Document tree */
           uchar  *filename)	/* I - Filename */
{
  uchar	*name;			/* Name of link */


  while (t != NULL)
  {
    if (t->markup == MARKUP_FILE)
      scan_links(t->child, (uchar *)file_basename((char *)htmlGetVariable(t, (uchar *)"_HD_FILENAME")));
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
	    snprintf((char *)newhref, sizeof(newhref), "%s%s",
	             link->filename, href);
	    htmlSetVariable(t, (uchar *)"HREF", newhref);
	  }
	}
      }

      if (t->child != NULL)
      {
        if (t->markup == MARKUP_FILE)
          update_links(t->child, htmlGetVariable(t, (uchar *)"_HD_FILENAME"));
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
	  snprintf((char *)newhref, sizeof(newhref), "#%s", link->name);
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
 * End of "$Id$".
 */
