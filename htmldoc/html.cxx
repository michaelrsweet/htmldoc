//
// "$Id: html.cxx,v 1.19 2000/10/19 00:41:42 mike Exp $"
//
//   HTML export functions for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
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
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include <ctype.h>


//
// 'HTMLDOC::html_write_document()' - Export to HTML...
//

void
HTMLDOC::html_write_document(uchar *title,	// I - Title string
                             uchar *author,	// I - Author
			     uchar *creator,	// I - Creator
		             uchar *copyright,	// I - Copyright
			     uchar *keywords)	// I - Search keywords
{
  HDtree	*t;				// Current node in tree
  uchar		*docnumber;			// Document number


  // Copy logo, body, and title images...
  //// MRS - NEED TO COPY ALL IMAGES!!!!!
  if (output_files_ && logo_image_)
    logo_image_->copy(output_path_);

  if (output_files_ && title_page_ && title_image_)
    title_image_->copy(output_path_);

  if (output_files_ && body_image_)
    body_image_->copy(output_path_);

  // Get document strings...
  docnumber = doc_->get_meta((uchar *)"docnumber");

  // Scan for all links in the doc_, and then update them...
  num_links_ = 0;

  scan_links(doc_, NULL);
  update_links(doc_, NULL);
  update_links(toc_, NULL);

  // Generate title pages and a table of contents...
  out_ = NULL;
  if (title_page_)
  {
    html_write_header((uchar *)"index.html", title, author, copyright,
                      docnumber, NULL);
    html_write_title(title, author, copyright, docnumber);

    html_write_footer(NULL);
    html_write_header((uchar *)"toc.html", title, author, copyright,
                      docnumber, NULL);
  }
  else
    html_write_header((uchar *)"index.html", title, author, copyright,
                      docnumber, NULL);

  html_write_all(toc_, 0);
  html_write_footer(NULL);

  // Then write each output file...
  for (t = doc_; t; t = t->next)
  {
    html_write_header(t->var((uchar *)"FILENAME"), title, author,
                      copyright, docnumber, t);
    html_write_all(t->child, 0);
    html_write_footer(t);
  }

  if (!output_files_ && out_ != stdout)
  {
    fputs("</BODY>\n", out_);
    fputs("</HTML>\n", out_);

    fclose(out_);
    out_ = NULL;
  }
}


//
// 'HTMLDOC::html_write_header()' - Output the standard "header" for a HTML file.
//

void
HTMLDOC::html_write_header(uchar  *filename,	// I - Output filename
			   uchar  *title,	// I - Title for document
        		   uchar  *author,	// I - Author for document
        		   uchar  *copyright,	// I - Copyright for document
        		   uchar  *docnumber,	// I - ID number for document
			   HDtree *t)		// I - Current document file
{
  char	realname[1024];		// Real filename
  char	*basename;		// Filename without directory
  int	newfile;		// Non-zero if this is a new file
  static char	*families[] =	// Typeface names
		{
		  "monospace",
		  "serif",
		  "sans-serif"
		};


  if (output_files_)
  {
    newfile  = 1;
    basename = file_basename((char *)filename);

    sprintf(realname, "%s/%s", output_path_, basename);

    out_ = fopen(realname, "w");
  }
  else if (output_path_[0] != '\0')
  {
    if (out_ == NULL)
    {
      out_    = fopen(output_path_, "w");
      newfile = 1;
    }
    else
      newfile = 0;
  }
  else
  {
    if (out_ == NULL)
    {
      out_    = stdout;
      newfile = 1;
    }
    else
      newfile = 0;
  }

  if (out_ == NULL)
  {
    progress_error("Unable to create output file \"%s\" - %s.\n",
                   output_files_ ? realname : output_path_,
		   strerror(errno));
    return;
  }

  if (newfile)
  {
    fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
          "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n", out_);
    fputs("<HTML>\n", out_);
    fputs("<HEAD>\n", out_);
    fprintf(out_, "<TITLE>%s</TITLE>\n", title);
    if (author != NULL)
      fprintf(out_, "<META NAME=\"AUTHOR\" CONTENT=\"%s\">\n", author);
    if (copyright != NULL)
      fprintf(out_, "<META NAME=\"COPYRIGHT\" CONTENT=\"%s\">\n", copyright);
    if (docnumber != NULL)
      fprintf(out_, "<META NAME=\"DOCNUMBER\" CONTENT=\"%s\">\n", docnumber);
    fprintf(out_, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-%s\">\n",
            HDtree::char_set);

    fputs("<STYLE>\n", out_);
    fprintf(out_, "BODY { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::body_font], HDtree::sizes[SIZE_P]);
    fprintf(out_, "H1 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H1]);
    fprintf(out_, "H2 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H2]);
    fprintf(out_, "H3 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H3]);
    fprintf(out_, "H4 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H4]);
    fprintf(out_, "H5 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H5]);
    fprintf(out_, "H6 { font-family: %s; font-size: %.1fpt }\n",
            families[HDtree::heading_font], HDtree::sizes[SIZE_H6]);
    fprintf(out_, "SUB { font-size: %.1fpt }\n", HDtree::sizes[SIZE_SUB]);
    fprintf(out_, "SUP { font-size: %.1fpt }\n", HDtree::sizes[SIZE_SUB]);
    fprintf(out_, "PRE { font-size: %.1fpt }\n", HDtree::sizes[SIZE_PRE]);

    if (!link_style_)
      fputs("A { text-decoration: none }\n", out_);

    fputs("</STYLE>\n", out_);
    fputs("</HEAD>\n", out_);

    if (body_file_[0] != '\0')
      fprintf(out_, "<BODY BACKGROUND=\"%s\"", file_basename(body_file_));
    else if (body_color_[0] != '\0')
      fprintf(out_, "<BODY BGCOLOR=\"%s\"", body_color_);
    else
      fputs("<BODY", out_);

    if (HDtree::text_color[0] != '\0')
      fprintf(out_, " TEXT=\"%s\"", HDtree::text_color);

    if (link_color_[0] != '\0')
      fprintf(out_, " LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\"", link_color_,
              link_color_, link_color_);

    fputs(">\n", out_);
  }
  else
    fputs("<HR>\n", out_);

  if (output_files_ && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    if (logo_file_[0] != '\0')
      fprintf(out_, "<IMG SRC=\"%s\">\n", file_basename(logo_file_));

    if (title_page_)
      fputs("<A HREF=\"toc.html\">Contents</a>\n", out_);
    else
      fputs("<A HREF=\"index.html\">Contents</a>\n", out_);

    if (t->prev != NULL)
      fprintf(out_, "<A HREF=\"%s\">Previous</a>\n",
              file_basename((char *)t->prev->var((uchar *)"FILENAME")));

    if (t->next != NULL)
      fprintf(out_, "<A HREF=\"%s\">Next</a>\n",
              file_basename((char *)t->next->var((uchar *)"FILENAME")));

    fputs("<HR>\n", out_);
  }
}


//
// 'HTMLDOC::html_write_footer()' - Output the standard "footer" for a HTML file.
//

void
HTMLDOC::html_write_footer(HDtree *t)	// I - Current document file
{
  if (out_ == NULL)
    return;

  if (output_files_ && t != NULL && (t->prev != NULL || t->next != NULL))
  {
    fputs("<HR>\n", out_);

    if (logo_file_[0] != '\0')
      fprintf(out_, "<IMG SRC=\"%s\">\n", file_basename(logo_file_));

    if (title_page_)
      fputs("<A HREF=\"toc.html\">Contents</a>\n", out_);
    else
      fputs("<A HREF=\"index.html\">Contents</a>\n", out_);


    if (t->prev != NULL)
      fprintf(out_, "<A HREF=\"%s\">Previous</a>\n",
              file_basename((char *)t->prev->var((uchar *)"FILENAME")));

    if (t->next != NULL)
      fprintf(out_, "<A HREF=\"%s\">Next</a>\n",
              file_basename((char *)t->next->var((uchar *)"FILENAME")));
  }

  if (output_files_)
  {
    fputs("</BODY>\n", out_);
    fputs("</HTML>\n", out_);

    fclose(out_);
    out_ = NULL;
  }
}


//
// 'HTMLDOC::html_write_title()' - Write a title page...
//

void
HTMLDOC::html_write_title(uchar *title,		// I - Title for document
        		  uchar *author,	// I - Author for document
        		  uchar *copyright,	// I - Copyright for document
        		  uchar *docnumber)	// I - ID number for document
{
  FILE		*fp;		// Title file
  HDtree	*t;		// Title file document tree


  if (out_ == NULL)
    return;

  if (title_file_[0] && !title_image_)
  {
    // Write a title page from HTML source...
    if ((fp = fopen(title_file_, "rb")) == NULL)
    {
      progress_error("Unable to open title file \"%s\" - %s!",
                     title_file_, strerror(errno));
      return;
    }

    t = new HDtree();
    t->read(fp, file_directory(title_file_));
    fclose(fp);

    html_write_all(t, 0);
    delete t;
  }
  else
  {
    // Write a "standard" title page with image...
    if (output_files_)
      fputs("<CENTER><A HREF=\"toc.html\">", out_);
    else
      fputs("<CENTER><A HREF=\"#CONTENTS\">", out_);

    if (output_files_)
      fprintf(out_, "<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"100%%\"><BR>\n",
              file_basename((char *)title_file_));
    else
      fprintf(out_, "<IMG SRC=\"%s\" BORDER=\"0\" WIDTH=\"100%%\"><BR>\n",
              title_file_);

    if (title != NULL)
      fprintf(out_, "<H1>%s</H1></A><BR>\n", title);
    else
      fputs("</A>\n", out_);

    if (docnumber != NULL)
      fprintf(out_, "%s<BR>\n", docnumber);

    if (author != NULL)
      fprintf(out_, "%s<BR>\n", author);

    if (copyright != NULL)
      fprintf(out_, "%s<BR>\n", copyright);

    fputs("</CENTER>\n", out_);
  }
}


//
// 'HTMLDOC::html_write_all()' - Write all markup text for the given tree.
//

int					// O - Current column
HTMLDOC::html_write_all(HDtree *t,	// I - Document tree
        		int    col)	// I - Current column
{
  int		i;		// Looping var
  uchar		*ptr,		// Pointer to output string
		*src,		// Source image
		newsrc[1024];	// New source image filename


  if (out_ == NULL)
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
              fputs((char *)iso8859(*ptr), out_);

	    if (t->data[strlen((char *)t->data) - 1] == '\n')
              col = 0;
	    else
              col += strlen((char *)t->data);
	  }
	  else
	  {
	    if ((col + strlen((char *)t->data)) > 72 && col > 0)
	    {
              putc('\n', out_);
              col = 0;
	    }

            for (ptr = t->data; *ptr != '\0'; ptr ++)
              fputs((char *)iso8859(*ptr), out_);

	    col += strlen((char *)t->data);

	    if (col > 72)
	    {
              putc('\n', out_);
              col = 0;
	    }
	  }
	  break;

      case MARKUP_COMMENT :
          fprintf(out_, "\n<!--%s-->\n", t->data);
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
            putc('\n', out_);
            col = 0;
          }

      default :
	  if (t->markup == MARKUP_IMG && output_files_ &&
              (src = t->var((uchar *)"SRC")) != NULL)
	  {
	    // Update local images...
            if (file_method((char *)src) == NULL &&
        	src[0] != '/' && src[0] != '\\' &&
		(!isalpha(src[0]) || src[1] != ':'))
            {
              strcpy((char *)newsrc, file_basename((char *)src));
              t->var((uchar *)"SRC", newsrc);
            }
	  }

          if (t->markup != MARKUP_EMBED)
	  {
	    col += fprintf(out_, "<%s", HDtree::markups[t->markup]);
	    for (i = 0; i < t->nvars; i ++)
	    {
	      if (strcasecmp((char *)t->vars[i].name, "BREAK") == 0 &&
	          t->markup == MARKUP_HR)
		continue;

	      if (col > 72 && !t->preformatted)
	      {
        	putc('\n', out_);
        	col = 0;
	      }

              if (col > 0)
              {
        	putc(' ', out_);
        	col ++;
              }

	      if (t->vars[i].value == NULL)
        	col += fprintf(out_, "%s", t->vars[i].name);
	      else
        	col += fprintf(out_, "%s=\"%s\"", t->vars[i].name, t->vars[i].value);
	    }

	    putc('>', out_);
	    col ++;

	    if (col > 72 && !t->preformatted)
	    {
	      putc('\n', out_);
	      col = 0;
	    }
	  }
	  break;
    }

    if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
    {
      col = html_write_all(t->child, col);

      if (col > 72 && !t->preformatted)
      {
	putc('\n', out_);
	col = 0;
      }

      switch (t->markup)
      {
        case MARKUP_INPUT :
	case MARKUP_IMG :
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
            fprintf(out_, "</%s>\n", HDtree::markups[t->markup]);
            col = 0;
            break;

	default :
            col += fprintf(out_, "</%s>", HDtree::markups[t->markup]);
	    break;
      }
    }

    t = t->next;
  }

  return (col);
}


//
// End of "$Id: html.cxx,v 1.19 2000/10/19 00:41:42 mike Exp $".
//
