//
// "$Id$"
//
//   Separated HTML export functions for HTMLDOC, a HTML document processing
//   program.
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
#include <ctype.h>


//
// 'hdBook::htmlsep_export()' - Export to separated HTML files...
//

int					// O - 0 = success, -1 = failure
hdBook::htmlsep_export(hdTree *document,// I - Document to export
                       hdTree *toc)	// I - Table of contents for document
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
  if (LogoImage[0])
  {
    if (LogoImage[0])
      image_copy(LogoImage, OutputPath);

    for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (HFImage[hfi][0])
        image_copy(HFImage[hfi], OutputPath);
  }

  if (TitleImage[0] && TitlePage &&
      !strcasecmp(file_extension(TitleImage), "bmp") ||
      !strcasecmp(file_extension(TitleImage), "gif") ||
      !strcasecmp(file_extension(TitleImage), "jpg") ||
      !strcasecmp(file_extension(TitleImage), "png"))
    image_copy(TitleImage, OutputPath);

  // Get document strings...
  title     = get_title(document);
  author    = htmlGetMeta(document, (uchar *)"author");
  copyright = htmlGetMeta(document, (uchar *)"copyright");
  docnumber = htmlGetMeta(document, (uchar *)"docnumber");

  // Scan for all links in the document, and then update them...
  num_headings = 0;
  num_links    = 0;

  htmlsep_scan_links(document);

//  printf("num_headings = %d\n", num_headings);
//  for (i = 0; i < num_headings; i ++)
//    printf("headings[%d] = '%s'\n", i, headings[i]);

  heading = -1;
  htmlsep_update_links(document, &heading);
  htmlsep_update_links(toc, NULL);

  // Generate title pages and a table of contents...
  out = NULL;
  if (TitlePage)
  {
    htmlsep_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, -1);
    if (out != NULL)
      htmlsep_title(out, title, author, copyright, docnumber);
  }
  else
    htmlsep_header(&out, (uchar *)"index.html", title, author, copyright,
                 docnumber, -1);

  if (out != NULL)
    htmlsep_write(out, toc, 0);

  htmlsep_footer(&out, -1);

  // Then write each output file...
  heading = -1;
  htmlsep_doc(&out, document, 0, &heading, title, author, copyright, docnumber);

  if (out != NULL)
    htmlsep_footer(&out, heading);

  // Free memory...
  if (title != NULL)
    free(title);

  for (i = 0; i < num_headings; i ++)
    free(headings[i]);

  return (out == NULL);
}


//
// 'hdBook::htmlsep_header()' - Output the standard "header" for a HTML file.
//

void
hdBook::htmlsep_header(FILE   **out,	// IO - Output file
        	       uchar  *filename,// I - Output filename
		       uchar  *title,	// I - Title for document
        	       uchar  *author,	// I - Author for document
        	       uchar  *copyright,
					// I - Copyright for document
        	       uchar  *docnumber,
					// I - ID number for document
		       int    heading)	// I - Current heading
{
  char		realname[1024];		// Real filename
  const char	*basename;		// Filename without directory
  static const char *families[] =	// Typeface names
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
                   "Unable to create output file '%s' - %s.\n",
                   realname, strerror(errno));
    return;
  }

  fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
        "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n", *out);
  fputs("<html>\n", *out);
  fputs("<head>\n", *out);
  if (title != NULL)
    fprintf(*out, "<title>%s</title>\n", title);
  if (author != NULL)
    fprintf(*out, "<meta name='author' content='%s'>\n", author);
  if (copyright != NULL)
    fprintf(*out, "<meta name='copyright' content='%s'>\n", copyright);
  if (docnumber != NULL)
    fprintf(*out, "<meta name='docnumber' content='%s'>\n", docnumber);
  fprintf(*out, "<meta http-equiv='Content-Type' content='text/html; charset=%s'>\n",
          _htmlCharSet);

  fputs("<link rel='Start' href='index.html'>\n", *out);

  if (TitlePage)
    fputs("<link rel='Contents' href='toc.html'>\n", *out);
  else
    fputs("<link rel='Contents' href='index.html'>\n", *out);

  if (heading >= 0)
  {
    if (heading > 0)
      fprintf(*out, "<link rel='Prev' href='%s.html'>\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<link rel='Next' href='%s.html'>\n", headings[heading + 1]);
  }

  fputs("<style type='text/css'><!--\n", *out);
  fprintf(*out, "BODY { font-family: %s }\n", families[_htmlBodyFont]);
  fprintf(*out, "H1, H2, H3, H4, H5, H6 { font-family: %s }\n",
          families[_htmlHeadingFont]);
  fputs("SUB, SUP { font-size: 50% }\n", *out);
  fputs("PRE { font-family: monospace }\n", *out);

  if (!LinkStyle)
    fputs("A { text-decoration: none }\n", *out);

  fputs("--></style>\n", *out);
  fputs("</head>\n", *out);

  fputs("<body", *out);
  if (BodyImage[0])
    fprintf(*out, " background='%s'", file_basename(BodyImage));
  if (BodyColor[0])
    fprintf(*out, " bgcolor='%s'", BodyColor);
  if (_htmlTextColor[0])
    fprintf(*out, " text='%s'", _htmlTextColor);

  if (LinkColor[0])
    fprintf(*out, " link='%s' vlink='%s' alink='%s'", LinkColor,
            LinkColor, LinkColor);

  fputs(">\n", *out);

  if (heading >= 0)
  {
    if (LogoImage[0])
      fprintf(*out, "<img src='%s'>\n", file_basename(LogoImage));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi][0])
        fprintf(*out, "<img src='%s'>\n", file_basename(HFImage[hfi]));

    if (TitlePage)
      fputs("<a href='toc.html'>Contents</a>\n", *out);
    else
      fputs("<a href='index.html'>Contents</a>\n", *out);

    if (heading > 0)
      fprintf(*out, "<a href='%s.html'>Previous</a>\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<a href='%s.html'>Next</a>\n", headings[heading + 1]);

    fputs("<hr noshade>\n", *out);
  }
}


//
// 'hdBook::htmlsep_footer()' - Output the standard "footer" for a HTML file.
//

void
hdBook::htmlsep_footer(FILE **out,	// IO - Output file pointer
		       int  heading)	// I  - Current heading
{
  if (*out == NULL)
    return;

  fputs("<hr noshade>\n", *out);

  if (heading >= 0)
  {
    if (LogoImage[0])
      fprintf(*out, "<img src='%s'>\n", file_basename(LogoImage));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi][0])
        fprintf(*out, "<img src='%s'>\n", file_basename(HFImage[hfi]));

    if (TitlePage)
      fputs("<a href='toc.html'>Contents</a>\n", *out);
    else
      fputs("<a href='index.html'>Contents</a>\n", *out);

    if (heading > 0)
      fprintf(*out, "<a href='%s.html'>Previous</a>\n", headings[heading - 1]);

    if (heading < (num_headings - 1))
      fprintf(*out, "<a href='%s.html'>Next</a>\n", headings[heading + 1]);
  }

  fputs("</body>\n", *out);
  fputs("</html>\n", *out);

  progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(*out));

  fclose(*out);
  *out = NULL;
}


//
// 'hdBook::htmlsep_title()' - Write a title page...
//

void
hdBook::htmlsep_title(FILE  *out,	// I - Output file
        	      uchar *title,	// I - Title for document
        	      uchar *author,	// I - Author for document
        	      uchar *copyright,	// I - Copyright for document
        	      uchar *docnumber)	// I - ID number for document
{
  FILE		*fp;			// Title file
  const char	*title_file;		// Location of title file
  hdTree	*t;			// Title file document tree


  if (out == NULL)
    return;

  if (TitleImage[0] &&
      strcasecmp(file_extension(TitleImage), "bmp") &&
      strcasecmp(file_extension(TitleImage), "gif") &&
      strcasecmp(file_extension(TitleImage), "jpg") &&
      strcasecmp(file_extension(TitleImage), "png"))
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

    t = html_read_file(NULL, fp, file_directory(TitleImage));
    htmlFixLinks(t, t, (uchar *)file_directory(TitleImage));
    fclose(fp);

    htmlsep_write(out, t, 0);
    htmlDeleteTree(t);
  }
  else
  {
    // Write a "standard" title page with image...
    fputs("<center>", out);

    if (TitleImage[0])
    {
      image_t *img = image_load(TitleImage, !OutputColor);

      fprintf(out, "<img src='%s' width='%d' height='%d' "
                   "alt='%s'><br>\n",
              file_basename((char *)TitleImage), img->width, img->height,
	      title ? (char *)title : "");
    }

    if (title != NULL)
      fprintf(out, "<h1>%s</h1><br>\n", title);
    else
      fputs("\n", out);

    if (docnumber != NULL)
      fprintf(out, "%s<br>\n", docnumber);

    if (author != NULL)
      fprintf(out, "%s<br>\n", author);

    if (copyright != NULL)
      fprintf(out, "%s<br>\n", copyright);

    fputs("<a href='toc.html'>Table of Contents</a>", out);
    fputs("</center>\n", out);
  }
}


//
// 'hdBook::htmlsep_write()' - Write all markup text for the given tree.
//

int					// O - Current column
hdBook::htmlsep_write(FILE   *out,	// I - Output file
        	      hdTree *t,	// I - Document tree
        	      int    col)	// I - Current column
{
  if (out == NULL)
    return (0);

  while (t != NULL)
  {
    col = htmlsep_node(out, t, col);

    if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
      col = htmlsep_write(out, t->child, col);

    col = htmlsep_nodeclose(out, t, col);

    t = t->next;
  }

  return (col);
}


//
// 'hdBook::htmlsep_doc()' - Write the entire document.
//

int					// O - Current column
hdBook::htmlsep_doc(FILE   **out,	// I - Output file
        	    hdTree *t,		// I - Document tree
        	    int    col,		// I - Current column
        	    int    *heading,	// IO - Current heading
		    uchar  *title,	// I  - Title
        	    uchar  *author,	// I  - Author
		    uchar  *copyright,	// I  - Copyright
		    uchar  *docnumber)	// I  - Document number
{
  uchar	filename[1024];			// Filename


  while (t != NULL)
  {
    if (t->element >= HD_ELEMENT_H1 && t->element < (HD_ELEMENT_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
    {
      if (heading >= 0)
        htmlsep_footer(out, *heading);

      (*heading) ++;

      if (*heading >= 0)
      {
	snprintf((char *)filename, sizeof(filename), "%s.html",
	         headings[*heading]);
	htmlsep_header(out, filename, title, author, copyright, docnumber,
                     *heading);
      }
    }

    col = htmlsep_node(*out, t, col);

    if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
      col = htmlsep_doc(out, t->child, col, heading,
                      title, author, copyright, docnumber);

    col = htmlsep_nodeclose(*out, t, col);

    t = t->next;
  }

  return (col);
}


//
// 'hdBook::htmlsep_node()' - Write a single tree node.
//

int					// O - Current column
hdBook::htmlsep_node(FILE   *out,	// I - Output file
        	     hdTree *t,		// I - Document tree node
        	     int    col)	// I - Current column
{
  int		i;			// Looping var
  uchar		*ptr,			// Pointer to output string
		*entity,		// Entity string
		*src,			// Source image
		newsrc[1024];		// New source image filename


  if (out == NULL)
    return (0);

  switch (t->element)
  {
    case HD_ELEMENT_NONE :
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

    case HD_ELEMENT_COMMENT :
    case HD_ELEMENT_UNKNOWN :
        fputs("\n<!--", out);
        for (ptr = t->data; *ptr; ptr ++)
          fputs((char *)iso8859(*ptr), out);
	fputs("-->\n", out);
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
          putc('\n', out);
          col = 0;
        }

    default :
	if (t->element == HD_ELEMENT_IMG &&
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
            strlcpy((char *)newsrc, file_basename((char *)src), sizeof(newsrc));
            htmlSetVariable(t, (uchar *)"SRC", newsrc);
          }
	}

        if (t->element != HD_ELEMENT_EMBED)
	{
	  col += fprintf(out, "<%s", _htmlMarkups[t->element]);
	  for (i = 0; i < t->nvars; i ++)
	  {
	    if (strcasecmp((char *)t->vars[i].name, "BREAK") == 0 &&
	        t->element == HD_ELEMENT_HR)
	      continue;

	    if (strcasecmp((char *)t->vars[i].name, "REALSRC") == 0 &&
	        t->element == HD_ELEMENT_IMG)
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


//
// 'hdBook::htmlsep_nodeclose()' - Close a single tree node.
//

int					// O - Current column
hdBook::htmlsep_nodeclose(FILE   *out,	// I - Output file
                          hdTree *t,	// I - Document tree node
                          int    col)	// I - Current column
{
  if (out == NULL)
    return (0);

  if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
  {
    if (col > 72 && !t->preformatted)
    {
      putc('\n', out);
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
          fprintf(out, "</%s>\n", _htmlMarkups[t->element]);
          col = 0;
          break;

      default :
          col += fprintf(out, "</%s>", _htmlMarkups[t->element]);
	  break;
    }
  }

  return (col);
}


//
// 'hdBook::add_heading()' - Add a heading to the list of headings...
//

void
hdBook::add_heading(hdTree *t)		// I - Heading node
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


//
// 'hdBook::htmlsep_scan_links()' - Scan a document for link targets, and
//                                  keep track of the files they are in...
//

void
hdBook::htmlsep_scan_links(hdTree *t)	// I - Document tree
{
  uchar	*name;				// Name of link


  while (t != NULL)
  {
    if (t->element >= HD_ELEMENT_H1 && t->element < (HD_ELEMENT_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
      add_heading(t);

    if (t->element == HD_ELEMENT_A &&
        (name = htmlGetVariable(t, (uchar *)"NAME")) != NULL)
      add_link(name, num_headings ? headings[num_headings - 1] :
                                    (uchar *)"noheading");

    if (t->child != NULL)
      htmlsep_scan_links(t->child);

    t = t->next;
  }
}


/*
 * 'hdBook::htmlsep_update_links()' - Update links as needed.
 */

void
hdBook::htmlsep_update_links(hdTree *t,	// I - Document tree
                             int    *heading)
					// I - Current heading
{
  hdLink	*link;			// Link
  uchar		*href;			// Reference name
  uchar		newhref[1024];		// New reference name
  uchar		*filename;		// Current filename


  // Scan the document, rewriting HREF's as needed...
  while (t != NULL)
  {
    if (t->element >= HD_ELEMENT_H1 && t->element < (HD_ELEMENT_H1 + TocLevels) &&
        htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL && heading)
      (*heading) ++;

    // Figure out the current filename based upon the current heading number...
    if (!heading || *heading < 0 || *heading >= num_headings)
      filename = (uchar *)"noheading";
    else
      filename = headings[*heading];

    if (t->element == HD_ELEMENT_A &&
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
      htmlsep_update_links(t->child, heading);

    // Move to the next node at this level...
    t = t->next;
  }
}


//
// End of "$Id$".
//
