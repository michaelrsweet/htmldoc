//
// "$Id: html.cxx,v 1.26 2004/10/23 04:45:54 mike Exp $"
//
//   HTML exporting functions for HTMLDOC, a HTML document processing program.
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
//       Hollywood, Maryland 20636-3142 USA
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
// 'hdBook::html_export()' - Export to HTML...
//

int					// O - 0 = success, -1 = failure
hdBook::html_export(hdTree *document,	// I - Document to export
                    hdTree *toc)	// I - Table of contents for document
{
  uchar	*title,				// Title text
	*author,			// Author name
	*copyright,			// Copyright text
	*docnumber;			// Document number
  FILE	*out;				// Output file


  // Copy logo and title images...
  if (OutputFiles)
  {
    if (LogoImage[0])
      image_copy(LogoImage, OutputPath);

    for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
      if (HFImage[hfi][0])
        image_copy(HFImage[hfi], OutputPath);
  }

  if (OutputFiles && TitleImage[0] && TitlePage &&
      strcasecmp(file_extension(TitleImage), "bmp") == 0 ||
      strcasecmp(file_extension(TitleImage), "gif") == 0 ||
      strcasecmp(file_extension(TitleImage), "jpg") == 0 ||
      strcasecmp(file_extension(TitleImage), "png") == 0)
    image_copy(TitleImage, OutputPath);

  // Get document strings...
  title     = get_title(document);
  author    = htmlGetMeta(document, (uchar *)"author");
  copyright = htmlGetMeta(document, (uchar *)"copyright");
  docnumber = htmlGetMeta(document, (uchar *)"docnumber");

  // Scan for all links in the document, and then update them...
  num_links = 0;

  html_scan_links(document, NULL);
  html_update_links(document, NULL);
  html_update_links(toc, NULL);

  // Generate title pages and a table of contents...
  out = NULL;

  if (TitlePage)
  {
    html_header(&out, (uchar *)"index.html", title, author, copyright,
                docnumber, NULL);

    if (out != NULL)
      html_title(out, title, author, copyright, docnumber);

    html_footer(&out, NULL);
    html_header(&out, (uchar *)"toc.html", title, author, copyright,
                docnumber, NULL);
  }
  else
    html_header(&out, (uchar *)"index.html", title, author, copyright,
                docnumber, NULL);

  if (out != NULL)
    html_write(out, toc, 0);

  html_footer(&out, NULL);

  // Then write each output file...
  while (document != NULL)
  {
    html_header(&out, htmlGetVariable(document, (uchar *)"_HD_FILENAME"),
                title, author, copyright, docnumber, document);
    if (out != NULL)
      html_write(out, document->child, 0);
    html_footer(&out, document);

    document = document->next;
  }

  if (!OutputFiles && out != stdout && out != NULL)
  {
    fputs("</body>\n", out);
    fputs("</html>\n", out);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(out));

    fclose(out);
  }

  if (title != NULL)
    free(title);

  return (out == NULL);
}


//
// 'hdBook::html_header()' - Output the standard "header" for a HTML file.
//

void
hdBook::html_header(FILE   **out,	// IO - Output file
        	    uchar  *filename,	// I - Output filename
		    uchar  *title,	// I - Title for document
        	    uchar  *author,	// I - Author for document
        	    uchar  *copyright,	// I - Copyright for document
        	    uchar  *docnumber,	// I - ID number for document
		    hdTree *t)		// I - Current document file
{
  char		realname[1024];		// Real filename
  const char	*basename;		// Filename without directory
  int		newfile;		// Non-zero if this is a new file
  static const char * const families[] =// Typeface names
		{
		  "monospace",
		  "serif",
		  "sans-serif"
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

    if (OutputFiles)
    {
      fputs("<link rel='Start' href='index.html'>\n", *out);

      if (TitlePage)
	fputs("<link rel='Contents' href='toc.html'>\n", *out);
      else
	fputs("<link rel='Contents' href='index.html'>\n", *out);

      if (t)
      {
	if (t->prev != NULL)
	  fprintf(*out, "<link rel='Prev' href='%s'>\n",
        	  file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

	if (t->next != NULL)
	  fprintf(*out, "<link rel='Next' href='%s'>\n",
        	  file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));
      }
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
  }
  else
    fputs("<hr noshade>\n", *out);

  if (OutputFiles && t != NULL && (t->prev != NULL || t->next != NULL))
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

    if (t->prev != NULL)
      fprintf(*out, "<a href='%s'>Previous</a>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<a href='%s'>Next</a>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));

    fputs("<hr noshade>\n", *out);
  }
}


//
// 'hdBook::html_footer()' - Output the standard "footer" for a HTML file.
//

void
hdBook::html_footer(FILE   **out,	// IO - Output file pointer
	            hdTree *t)		// I - Current document file
{
  if (*out == NULL)
    return;

  if (OutputFiles && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    fputs("<hr noshade>\n", *out);

    if (LogoImage[0])
      fprintf(*out, "<img src='%s'>\n", file_basename(LogoImage));

    for (int hfi = 0; hfi < MAX_HF_IMAGES; ++hfi)
      if (HFImage[hfi][0])
        fprintf(*out, "<img src='%s'>\n", file_basename(HFImage[hfi]));

    if (TitlePage)
      fputs("<a href='toc.html'>Contents</a>\n", *out);
    else
      fputs("<a href='index.html'>Contents</a>\n", *out);


    if (t->prev != NULL)
      fprintf(*out, "<a href='%s'>Previous</a>\n",
              file_basename((char *)htmlGetVariable(t->prev, (uchar *)"_HD_FILENAME")));

    if (t->next != NULL)
      fprintf(*out, "<a href='%s'>Next</a>\n",
              file_basename((char *)htmlGetVariable(t->next, (uchar *)"_HD_FILENAME")));
  }

  if (OutputFiles)
  {
    fputs("</body>\n", *out);
    fputs("</html>\n", *out);

    progress_error(HD_ERROR_NONE, "BYTES: %ld", ftell(*out));

    fclose(*out);
    *out = NULL;
  }
}


//
// 'hdBook::html_title()' - Write a title page...
//

void
hdBook::html_title(FILE  *out,		// I - Output file
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
      strcasecmp(file_extension(TitleImage), "bmp") != 0 &&
      strcasecmp(file_extension(TitleImage), "gif") != 0 &&
      strcasecmp(file_extension(TitleImage), "jpg") != 0 &&
      strcasecmp(file_extension(TitleImage), "png") != 0)
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

    html_write(out, t, 0);
    htmlDeleteTree(t);
  }
  else
  {
    // Write a "standard" title page with image...
    if (OutputFiles)
      fputs("<center><a href='toc.html'>", out);
    else
      fputs("<center><a href='#CONTENTS'>", out);

    if (TitleImage[0])
    {
      image_t *img = image_load(TitleImage, !OutputColor);

      if (OutputFiles)
	fprintf(out, "<img src='%s' border='0' width='%d' height='%d' "
	             "alt='%s'><br>\n",
        	file_basename((char *)TitleImage), img->width, img->height,
		title ? (char *)title : "");
      else
	fprintf(out, "<img src='%s' border='0' width='%d' height='%d' "
	             "alt='%s'><br>\n",
        	TitleImage, img->width, img->height,
		title ? (char *)title : "");
    }

    if (title != NULL)
      fprintf(out, "<h1>%s</h1></a><br>\n", title);
    else
      fputs("</a>\n", out);

    if (docnumber != NULL)
      fprintf(out, "%s<br>\n", docnumber);

    if (author != NULL)
      fprintf(out, "%s<br>\n", author);

    if (copyright != NULL)
      fprintf(out, "%s<br>\n", copyright);

    fputs("</center>\n", out);
  }
}


//
// 'hdBook::html_write()' - Write all markup text for the given tree.
//

int					// O - Current column
hdBook::html_write(FILE   *out,		// I - Output file
        	   hdTree *t,		// I - Document tree
        	   int    col)		// I - Current column
{
  int		i;			// Looping var
  uchar		*ptr,			// Pointer to output string
		*src,			// Source image
		newsrc[1024],		// New source image filename
		*entity;		// Entity string


  if (out == NULL)
    return (0);

  while (t != NULL)
  {
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
	  if (t->element == HD_ELEMENT_IMG && OutputFiles &&
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

    if (t->element != HD_ELEMENT_HEAD && t->element != HD_ELEMENT_TITLE)
    {
      col = html_write(out, t->child, col);

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

    t = t->next;
  }

  return (col);
}


//
// 'hdBook::html_scan_links()' - Scan a document for link targets, and keep track
//                               of the files they are in...
//

void
hdBook::html_scan_links(hdTree *t,	// I - Document tree
                        uchar  *filename)
					// I - Filename
{
  uchar	*name;				// Name of link


  while (t != NULL)
  {
    if (t->element == HD_ELEMENT_FILE)
      html_scan_links(t->child, (uchar *)file_basename(
                                        (char *)htmlGetVariable(t, 
		                                    (uchar *)"_HD_FILENAME")));
    else if (t->element == HD_ELEMENT_A &&
             (name = htmlGetVariable(t, (uchar *)"NAME")) != NULL)
    {
      add_link(name, filename);
      html_scan_links(t->child, filename);
    }
    else if (t->child != NULL)
      html_scan_links(t->child, filename);

    t = t->next;
  }
}


//
// 'hdBook::html_update_links()' - Update links as needed.
//

void
hdBook::html_update_links(hdTree *t,	// I - Document tree
                          uchar  *filename)
					// I - Current filename
{
  hdLink	*link;			// Link
  uchar		*href;			// Reference name
  uchar		newhref[1024];		// New reference name


  filename = (uchar *)file_basename((char *)filename);

  if (OutputFiles)
  {
    // Need to preserve/add filenames.
    while (t != NULL)
    {
      if (t->element == HD_ELEMENT_A &&
          (href = htmlGetVariable(t, (uchar *)"HREF")) != NULL)
      {
        // Update this link as needed...
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
        if (t->element == HD_ELEMENT_FILE)
          html_update_links(t->child, htmlGetVariable(t, (uchar *)"_HD_FILENAME"));
	else
          html_update_links(t->child, filename);
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
      if (t->element == HD_ELEMENT_A &&
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
        html_update_links(t->child, filename);

      t = t->next;
    }
  }
}


//
// 'hdBook::get_title()' - Get the title string for the given document...
//

uchar *					// O - Title string
hdBook::get_title(hdTree *doc)		// I - Document tree
{
  uchar	*temp;				// Temporary pointer to title


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


/*
 * End of "$Id: html.cxx,v 1.26 2004/10/23 04:45:54 mike Exp $".
 */
