//
// "$Id: testsuite.cxx,v 1.4 2002/02/23 04:03:31 mike Exp $"
//
//   Test program for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2002 by Easy Software Products.
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


//
// 'main()' - Main entry.
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  int		i, j;		// Looping vars
  hdImage	*img;		// Image
  char		filename[1024];	// Remote file from local cache
  hdFile	*fp;		// File to read
  hdStyleSheet	*css;		// Style sheet
  hdTree	*html;		// HTML file


  setbuf(stdout, NULL);

  hdImage::register_standard();

  if ((img = hdImage::find("testimg.jpg", 0, ".;../jpeg")) == NULL)
  {
    puts("Unable to load testimg.jpg using path \".;../jpeg\"...");
  }
  else
    printf("testimg.jpg: %dx%d pixels\n", img->width(), img->height());

#if 0
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
#endif // 0

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

    if ((fp = hdFile::open("../testsuite/basic.html", HD_FILE_READ)) != NULL)
    {
      html = hdTree::read(fp, "../testsuite", NULL, css);

      delete fp;

      if (html != NULL)
      {
        print_tree(html, 0);
        delete html;
      }
      else
        puts("Unable to load HTML file!");
    }
    else
      puts("Unable to open ../testsuite/basic.html!");

    delete css;
  }

  return (0);
}


//
// 'print_tree()' - Print HTML tree nodes...
//

void
print_tree(hdTree *t,		// I - Tree node
           int    indent)	// I - Indentation
{
  int	i;			// Looping var


  while (t)
  {
    for (i = 0; i < indent; i ++)
      putchar(' ');

    switch (t->element)
    {
      case HD_ELEMENT_NONE :
          printf("(none) \"%s\" %.1fx%.1f\n", t->data ? t->data : "(null)",
	         t->width, t->height);
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

      case HD_ELEMENT_IMG :
      case HD_ELEMENT_HR :
      case HD_ELEMENT_BR :
      case HD_ELEMENT_SPACER :
          printf("%s %.1fx%.1f\n", hdTree::elements[t->element],
	         t->width, t->height);
	  break;

      default :
          printf("%s\n", hdTree::elements[t->element]);
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
// End of "$Id: testsuite.cxx,v 1.4 2002/02/23 04:03:31 mike Exp $".
//
