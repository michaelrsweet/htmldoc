//
// "$Id$"
//
//   Test program for HTMLDOC, a HTML document processing program.
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
//

//
// Include necessary headers.
//

#include "htmldoc.h"
#include "hdstring.h"


//
// Functions...
//

void	print_tree(hdTree *t, int indent);
void	print_style(hdStyle *s);
void	write_book(hdFile *fp, hdStyleSheet *css, hdTree *toc,
	           hdTree *figures, hdTree *doc, hdTree *ind);
void	write_css(hdFile *fp, hdStyleSheet *css);
void	write_html(hdFile *fp, hdTree *t);
void	write_test(hdFile *fp);


//
// 'main()' - Main entry.
//

int					// O - Exit status
main(int  argc,				// I - Number of command-line arguments
     char *argv[])			// I - Command-line arguments
{
  int			i, j;		// Looping vars
  hdImage		*img;		// Image
  char			filename[3072];	// Remote file from local cache
  hdFile		*fp;		// File to read/write
  hdASCII85Filter	*ascii85;	// ASCII85 filter
  hdASCIIHexFilter	*asciihex;	// ASCIIHex filter
  hdFlateFilter		*flate;		// Flate filter
  hdJPEGFilter		*jpeg;		// JPEG filter
  hdStyleSheet		*css;		// Style sheet
  hdTree		*html,		// HTML file
			*toc,		// Table-of-contents
			*figures,	// List of figures...
			*ind;		// Index


  setbuf(stdout, NULL);

  hdImage::register_standard();

  if ((img = hdImage::find("testimg.jpg", 0, ".;../jpeg")) == NULL)
  {
    puts("Unable to load testimg.jpg using path \".;../jpeg\"...");
  }
  else
    printf("testimg.jpg: %dx%d pixels\n", img->width(), img->height());

  if (hdFile::find(0, "http://www.easysw.com/images/title-htmldoc.gif",
                   filename, sizeof(filename)))
  {
    puts("Copied title-htmldoc.gif from www.easysw.com, trying to load...");

    if ((img = hdImage::find(filename, 0)) == NULL)
    {
      puts("Unable to load title-htmldoc.gif from cache file...");
      return (1);
    }

    printf("title-htmldoc.gif: %dx%d pixels\n", img->width(), img->height());
  }
  else
    puts("Unable to load title-htmldoc.gif from www.easysw.com...");

  if ((fp = hdFile::open("../data/webpage.css", HD_FILE_READ)) == NULL)
    puts("Unable to open ../data/webpage.css...");
  else
  {
    css = new hdStyleSheet();

    printf("hdStyleSheet::load() returned %d...\n", css->load(fp));

    delete fp;

    printf("%d styles in stylesheet.\n", css->num_styles);

    for (i = 0; i < css->num_styles; i ++)
    {
      printf("Style #%d:", i + 1);

      for (j = css->styles[i]->num_selectors - 1; j >= 0; j --)
      {
        switch (css->styles[i]->selectors[j].element)
	{
	  case HD_ELEMENT_NONE :
	      printf(" (none)");
	      break;
	  case HD_ELEMENT_UNKNOWN :
	      printf(" (unknown)");
	      break;
	  default :
	      printf(" %s",
	             hdTree::elements[css->styles[i]->selectors[j].element]);
	      break;
	}

	if (css->styles[i]->selectors[j].pseudo)
          printf(":%s", css->styles[i]->selectors[j].pseudo);
      }

      putchar('\n');
      print_style(css->styles[i]);
    }

    if ((fp = hdFile::open("../testsuite/book.html", HD_FILE_READ)) != NULL)
    {
      html = hdTree::read(fp, "../testsuite", NULL, css);

      delete fp;

      if (html != NULL)
      {
        hdMargin *m = new hdMargin(72.0, 576.0, 36.0, 756.0);
	float x = 0.0, y = 0.0;
	int page = 0;

        html->format_doc(css, m, x, y, page);

        print_tree(html, 0);

        toc     = html->build_toc(css, 3, 1);
	figures = html->build_list(css, "FIGURE", "Figure");

        fp  = hdFile::open("../testsuite/book.words", HD_FILE_READ);
	ind = html->build_index(css, fp);
	delete fp;

        fp = hdFile::open("test.html", HD_FILE_WRITE);
	write_book(fp, css, toc, figures, html, ind);
	delete fp;

        delete html;
	delete toc;
	delete figures;
	delete ind;
      }
      else
        puts("Unable to load HTML file!");
    }
    else
      puts("Unable to open ../testsuite/book.html!");

    delete css;
  }

  // Test output filters...
  if ((fp = hdFile::open("test.file", HD_FILE_WRITE)) == NULL)
  {
    puts("Unable to create test.file!");
  }
  else
  {
    fp->puts("Text no filters:\n\n");
    write_test(fp);

    fp->puts("\nText ASCIIHex filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    write_test(asciihex);
    delete asciihex;

    fp->puts("\nText ASCII85 filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    write_test(ascii85);
    delete ascii85;

    fp->puts("\nText ASCIIHex + Flate filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    flate    = new hdFlateFilter(asciihex, 9);
    write_test(flate);
    delete flate;
    delete asciihex;

    fp->puts("\nText ASCII85 + Flate filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    flate   = new hdFlateFilter(ascii85, 9);
    write_test(flate);
    delete flate;
    delete ascii85;

    for (i = 0; i < 96; i += 3)
      for (j = i; j < 3072; j += 96)
      {
        filename[j + 0] = i * 255 / 95;
        filename[j + 1] = 255 - i * 255 / 95;
        filename[j + 2] = j * 255 / 3071;
      }

    fp->puts("\nImage ASCIIHex filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    asciihex->write(filename, 32 * 32 * 3);
    delete asciihex;

    fp->puts("\nImage ASCII85 filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    ascii85->write(filename, 32 * 32 * 3);
    delete ascii85;

    fp->puts("\nImage ASCIIHex + Flate filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    flate    = new hdFlateFilter(asciihex, 9);
    flate->write(filename, 32 * 32 * 3);
    delete flate;
    delete asciihex;

    fp->puts("\nImage ASCII85 + Flate filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    flate   = new hdFlateFilter(ascii85, 9);
    flate->write(filename, 32 * 32 * 3);
    delete flate;
    delete ascii85;

    fp->puts("\nImage ASCIIHex + JPEG filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    jpeg     = new hdJPEGFilter(asciihex, 32, 32, 3);
    jpeg->write(filename, 32 * 32 * 3);
    delete jpeg;
    delete asciihex;

    fp->puts("\nImage ASCII85 + JPEG filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    jpeg    = new hdJPEGFilter(ascii85, 32, 32, 3);
    jpeg->write(filename, 32 * 32 * 3);
    delete jpeg;
    delete ascii85;

    fp->puts("\nImage ASCIIHex + Flate + JPEG filter:\n\n");
    asciihex = new hdASCIIHexFilter(fp);
    flate    = new hdFlateFilter(asciihex, 9);
    jpeg     = new hdJPEGFilter(flate, 32, 32, 3);
    jpeg->write(filename, 32 * 32 * 3);
    delete jpeg;
    delete flate;
    delete asciihex;

    fp->puts("\nImage ASCII85 + Flate + JPEG filter:\n\n");
    ascii85 = new hdASCII85Filter(fp);
    flate   = new hdFlateFilter(ascii85, 9);
    jpeg    = new hdJPEGFilter(flate, 32, 32, 3);
    jpeg->write(filename, 32 * 32 * 3);
    delete jpeg;
    delete flate;
    delete ascii85;

    delete fp;

    fp = hdFile::open("test.ppm", HD_FILE_WRITE);
    fp->printf("P6\n32\n32\n255\n");
    fp->write(filename, 32 * 32 * 3);
    delete fp;
  }

  return (0);
}


//
// 'print_tree()' - Print HTML tree nodes...
//

void
print_tree(hdTree *t,				// I - Tree node
           int    indent)			// I - Indentation
{
  int			i;			// Looping var
  static const char	*nodebreaks[] =		// Nodebreak strings
			{
			  "none",
			  "left",
			  "right",
			  "line",
			  "page",
			  "sheet"
			};


  while (t)
  {
    for (i = 0; i < indent; i ++)
      putchar(' ');

    switch (t->element)
    {
      case HD_ELEMENT_NONE :
          printf("(none) \"%s\" %.1fx%.1f (whitespace=%s, nodebreak=%s)\n",
	         t->data ? t->data : "(null)", t->width, t->height,
		 t->whitespace ? "true" : "false", nodebreaks[t->nodebreak]);
	  break;

      case HD_ELEMENT_UNKNOWN :
          printf("(unknown) \"%s\" (nodebreak=%s)\n", t->data,
	         nodebreaks[t->nodebreak]);
	  break;

      case HD_ELEMENT_FILE :
          puts("(file)");
	  break;

      case HD_ELEMENT_COMMENT :
          printf("(comment) \"%s\" (nodebreak=%s)\n", t->data,
	         nodebreaks[t->nodebreak]);
	  break;

      case HD_ELEMENT_IMG :
      case HD_ELEMENT_HR :
      case HD_ELEMENT_BR :
      case HD_ELEMENT_SPACER :
          printf("%s %.1fx%.1f (whitespace=%s, nodebreak=%s)\n",
	         hdTree::elements[t->element], t->width, t->height,
		 t->whitespace ? "true" : "false", nodebreaks[t->nodebreak]);
	  break;

      default :
          printf("%s (whitespace=%s, nodebreak=%s)\n",
	         hdTree::elements[t->element],
		 t->whitespace ? "true" : "false", nodebreaks[t->nodebreak]);
	  break;
    }

    if (t->child)
    {
      print_tree(t->child, indent + 4);

      for (i = 0; i < indent; i ++)
	putchar(' ');

      putchar('/');

      switch (t->element)
      {
	case HD_ELEMENT_NONE :
            printf("(none) \"%s\"\n", t->data ? t->data : "(null)");
	    break;

	case HD_ELEMENT_UNKNOWN :
            printf("(unknown) \"%s\"\n", t->data);
	    break;

	case HD_ELEMENT_FILE :
            puts("(file)");
	    break;

	case HD_ELEMENT_COMMENT :
            printf("(comment) \"%s\"\n", t->data);
	    break;

	default :
            printf("%s\n", hdTree::elements[t->element]);
	    break;
      }
    }

    t = t->next;
  }
}


//
// 'print_style()' - Print a style definition.
//

void
print_style(hdStyle *s)		// I - Style
{
  static const char	*displays[] =
			{
			  "none",
			  "block",
			  "compact",
			  "inline",
			  "inline-table",
			  "list-item",
			  "marker",
			  "run-in",
			  "table",
			  "table-caption",
			  "table-cell",
			  "table-column",
			  "table-column-group",
			  "table-footer-group",
			  "table-header-group",
			  "table-row",
			  "table-row-group"
			};
  static const char	*styles[] =
			{
			  "normal",
			  "italic",
			  "oblique"
			};
  static const char	*weights[] =
			{
			  "normal",
			  "bold",
			  "bolder",
			  "lighter"
			};


  if (s->background_color_set)
    printf("    background-color: rgb(%d,%d,%d)\n", s->background_color[0],
           s->background_color[1], s->background_color[2]);

  if (s->background_image)
    printf("    background-image: url(%s)\n", s->background_image);

  if (s->color_set)
    printf("    color: rgb(%d,%d,%d)\n", s->color[0],
	   s->color[1], s->color[2]);

  printf("    display: %s\n", displays[s->display]);

  if (s->font_family)
    printf("    font-family: %s\n", s->font_family);

  if (s->font_style)
    printf("    font-style: %s\n", styles[s->font_style]);

  if (s->font_weight)
    printf("    font-weight: %s\n", weights[s->font_weight]);
}


//
// 'write_book()' - Write a book in HTML format...
//

void
write_book(hdFile       *fp,
           hdStyleSheet *css,
           hdTree       *toc,
	   hdTree       *figures,
	   hdTree       *doc,
	   hdTree       *ind)
{
  fp->puts("<html>\n");
  fp->puts("<head>\n");
  write_css(fp, css);
  fp->puts("</head>\n");
  fp->puts("<body>\n");
  fp->puts("<h1>Table of Contents</h1>\n");
  write_html(fp, toc);
  fp->puts("<p><a href=\"#HD_INDEX\">Index</a></p>\n");
  fp->puts("<hr>\n");
  fp->puts("<h1>List of Figures</h1>\n");
  write_html(fp, figures);
  fp->puts("<hr>\n");
  write_html(fp, doc);
  fp->puts("<hr>\n");
  fp->puts("<h1><a name=\"HD_INDEX\">Index</a></h1>\n");
  write_html(fp, ind);
  fp->puts("</body>\n");
  fp->puts("</html>\n");
}


//
// 'write_css()' - Write stylesheet data...
//

void
write_css(hdFile       *fp,
          hdStyleSheet *css)
{
  int			i, j;
  hdStyle		*style;
  static const char	*displays[] =
			{
			  "none",
			  "block",
			  "compact",
			  "inline",
			  "inline-table",
			  "list-item",
			  "marker",
			  "run-in",
			  "table",
			  "table-caption",
			  "table-cell",
			  "table-column",
			  "table-column-group",
			  "table-footer-group",
			  "table-header-group",
			  "table-row",
			  "table-row-group"
			};
  static const char	*styles[] =
			{
			  "normal",
			  "italic",
			  "oblique"
			};
  static const char	*weights[] =
			{
			  "normal",
			  "bold",
			  "bolder",
			  "lighter"
			};


  fp->puts("<style>\n");

  for (i = 0; i < css->num_styles; i ++)
  {
    style = css->styles[i];

    if (style->selectors[0].id &&
        strncmp(style->selectors[0].id, "_HD", 3) == 0)
      continue;

    for (j = style->num_selectors - 1; j >= 0; j --)
    {
      fp->puts(hdTree::elements[style->selectors[j].element]);

      if (style->selectors[j].class_)
        fp->printf(".%s", style->selectors[j].class_);

      if (style->selectors[j].pseudo)
        fp->printf(":%s", style->selectors[j].pseudo);

      if (style->selectors[j].id)
        fp->printf("#%s", style->selectors[j].id);

      fp->puts(" ");
    }

    fp->puts("{\n");

    if (style->background_color_set)
      fp->printf("    background-color: rgb(%d,%d,%d)\n", style->background_color[0],
             style->background_color[1], style->background_color[2]);

    if (style->background_image)
      fp->printf("    background-image: url(%s)\n", style->background_image);

    if (style->color_set)
      fp->printf("    color: rgb(%d,%d,%d)\n", style->color[0],
	     style->color[1], style->color[2]);

    fp->printf("    display: %s\n", displays[style->display]);

    if (style->font_family)
      fp->printf("    font-family: %s\n", style->font_family);

    if (style->font_style)
      fp->printf("    font-style: %s\n", styles[style->font_style]);

    if (style->font_weight)
      fp->printf("    font-weight: %s\n", weights[style->font_weight]);

    fp->puts("}\n");
  }

  fp->puts("</style>\n");
}


//
// 'write_html()' - Write HTML to a file...
//

void
write_html(hdFile *fp,
           hdTree *t)
{
  int i, len, col = 0;


  while (t)
  {
    switch (t->element)
    {
      case HD_ELEMENT_FILE :
      case HD_ELEMENT_HTML :
      case HD_ELEMENT_HEAD :
      case HD_ELEMENT_BODY :
          break;

      case HD_ELEMENT_NONE :
          len = strlen(t->data);
	  if ((col + len) >= 80)
	  {
	    fp->puts("\n");
	    col = 0;
	  }

	  if (t->whitespace && col && t->prev != NULL)
	  {
	    col ++;
	    fp->printf(" %s", t->data);
	  }
	  else
	    fp->puts(t->data);

          col += len;
	  break;
          
      default :
	  len = strlen(hdTree::elements[t->element]) + 1;

	  if ((col + len) > 80 || hdElIsBlock(t->element) ||
	      hdElIsTable(t->element) || hdElIsRowCol(t->element) ||
	      hdElIsList(t->element) || hdElIsItem(t->element))
	  {
            col = 0;
	    fp->puts("\n");
	  }
	  else if (t->whitespace || (t->child && t->child->whitespace))
	  {
	    fp->puts(" ");
	    col ++;
	  }

	  fp->printf("<%s", hdTree::elements[t->element]);
	  for (i = 0; i < t->nattrs; i ++)
	  {
	    if (strncasecmp(t->attrs[i].name, "_HD", 3) == 0)
	      continue;

	    len = strlen(t->attrs[i].name) + strlen(t->attrs[i].value) + 4;
	    if ((col + len) > 80)
	    {
	      fp->printf("\n%s=\"%s\"", t->attrs[i].name, t->attrs[i].value);
	      col = len - 1;
	    }
	    else
	    {
	      fp->printf(" %s=\"%s\"", t->attrs[i].name, t->attrs[i].value);
	      col += len;
	    }
	  }
	  if (t->child == NULL)
	  {
	    fp->puts("/");
	    col ++;
	  }
	  fp->puts(">");
	  col ++;
          break;
    }

    if (t->child == NULL)
    {
      while (t->next == NULL && t->parent)
      {
	t = t->parent;
	len = strlen(hdTree::elements[t->element]) + 3;

	if ((col + len) > 80)
	{
          col = 0;
	  fp->puts("\n");
	}

	fp->printf("</%s>", hdTree::elements[t->element]);
	col += len;
      }

      t = t->next;
    }
    else
      t = t->real_next();
  }

  if (col)
    fp->puts("\n");
}


//
// 'write_test()' - Write test data to the specified file.
//

void
write_test(hdFile *fp)	// I - File to write to...
{
  int	i, j, k;	// Looping vars...


  fp->puts("Now is the time for all good men to come to "
           "the aide of their country.\n");

  for (i = 1, j = 1; i < 100; k = i, i += j, j = k)
    fp->printf("%d\n", i);
}


//
// End of "$Id$".
//
