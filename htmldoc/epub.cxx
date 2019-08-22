/*
 * EPUB exporting functions for HTMLDOC, a HTML document processing program.
 *
 * Copyright 2017-2019 by Michael R Sweet.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"
#include "zipc.h"
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>


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


static size_t	num_links = 0,
		alloc_links = 0;
static link_t	*links;
static size_t   num_images = 0,
                alloc_images = 0;
static char     **images = NULL;


/*
 * Local functions...
 */

extern "C" {
typedef int	(*compare_func_t)(const void *, const void *);
}

static int	write_header(zipc_file_t *out, uchar *title, uchar *author, uchar *copyright, uchar *docnumber, tree_t *t);
static int	write_title(zipc_file_t *out, tree_t *title_tree, uchar *title, uchar *author, uchar *copyright, uchar *docnumber);
static int	write_all(zipc_file_t *out, tree_t *t);
static int	write_node(zipc_file_t *out, tree_t *t);
static int	write_nodeclose(zipc_file_t *out, tree_t *t);
static int	write_toc(zipc_file_t *out, tree_t *t);
static char     *get_iso_date(time_t t);
static uchar	*get_title(tree_t *doc);

static void	add_link(uchar *name, uchar *filename);
static link_t	*find_link(uchar *name);
static int	compare_links(link_t *n1, link_t *n2);
static int      compare_images(char **a, char **b);
static int      copy_image(zipc_t *zipc, const char *filename);
static int      copy_images(zipc_t *zipc, tree_t *t);
static void	scan_links(tree_t *t, uchar *filename);
static void	update_links(tree_t *t, uchar *filename);
static tree_t   *walk_next(tree_t *t);
static int      write_xhtml(zipc_file_t *out, uchar *s);
static int      write_xhtmlf(zipc_file_t *out, const char *format, ...);


/*
 * 'epub_export()' - Export to EPUB...
 */

int                                     /* O - 0 = success, -1 = failure */
epub_export(tree_t *document,           /* I - Document to export */
            tree_t *toc)                /* I - Table of contents for document */
{
  uchar       *title,                   /* Title text */
              *author,                  /* Author name */
              *copyright,               /* Copyright text */
              *docnumber,               /* Document number */
              *language,		/* Language */
              *subject;			/* Subject/category */
  zipc_t      *epub;                    /* EPUB output file */
  zipc_file_t *epubf;                   /* File in container */
  struct stat epubinfo;                 /* EPUB file information */
  const char  *title_ext;               /* Extension of title image */
  tree_t      *title_tree = NULL;	/* Title file document tree */
  const char  *cover_image = NULL;      /* Do we have a cover image? */
  int         status = 0;               /* Return status */
  static const char *mimetype =		/* mimetype file as a string */
		"application/epub+zip";
  static const char *container_xml =	/* container.xml file as a string */
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                "<container xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\" version=\"1.0\">\n"
                "  <rootfiles>\n"
                "    <rootfile full-path=\"OEBPS/package.opf\" media-type=\"application/oebps-package+xml\"/>\n"
                "  </rootfiles>\n"
                "</container>\n";


 /*
  * Create the EPUB file...
  */

  if ((epub = zipcOpen(OutputPath, "w")) == NULL)
  {
    progress_error(HD_ERROR_WRITE_ERROR, "Unable to create \"%s\": %s", OutputPath, strerror(errno));
    return (-1);
  }

 /*
  * Add the mimetype file...
  */

  status |= zipcCreateFileWithString(epub, "mimetype", mimetype);

 /*
  * The META-INF/ directory...
  */

  status |= zipcCreateDirectory(epub, "META-INF/");

 /*
  * The META-INF/container.xml file...
  */

  if ((epubf = zipcCreateFile(epub, "META-INF/container.xml", 1)) != NULL)
  {
    status |= zipcFilePuts(epubf, container_xml);
    status |= zipcFileFinish(epubf);
  }
  else
    status = -1;

 /*
  * The OEBPS/ directory...
  */

  status |= zipcCreateDirectory(epub, "OEBPS/");

 /*
  * Copy logo and title images...
  */

  if (LogoImage[0])
    status |= copy_image(epub, file_find(Path, LogoImage));

  for (int hfi = 0; hfi < MAX_HF_IMAGES; hfi ++)
  {
    if (HFImage[hfi][0])
      status |= copy_image(epub, file_find(Path, HFImage[hfi]));
  }

  title_ext = file_extension(TitleImage);

  if (TitleImage[0] && TitlePage)
  {
#ifdef WIN32
    if (!stricmp(title_ext, "bmp") || !stricmp(title_ext, "gif") || !stricmp(title_ext, "jpg") || !stricmp(title_ext, "png"))
#else
    if (!strcmp(title_ext, "bmp") || !strcmp(title_ext, "gif") || !strcmp(title_ext, "jpg") || !strcmp(title_ext, "png"))
#endif // WIN32
    {
      status |= copy_image(epub, file_find(Path, TitleImage));
      cover_image = file_basename(TitleImage);
    }
    else
    {
      FILE	*fp;			/* Title file */
      const char *title_file;		/* Location of title file */

      // Find the title page file...
      if ((title_file = file_find(Path, TitleImage)) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND, "Unable to find title file \"%s\".", TitleImage);
	return (-1);
      }

      // Read a HTML title page...
      if ((fp = fopen(title_file, "rb")) == NULL)
      {
	progress_error(HD_ERROR_FILE_NOT_FOUND,
		       "Unable to open title file \"%s\" - %s!",
		       TitleImage, strerror(errno));
	return (-1);
      }

      title_tree = htmlReadFile(NULL, fp, file_directory(TitleImage));
      htmlFixLinks(title_tree, title_tree, (uchar *)file_directory(TitleImage));
      fclose(fp);

      status |= copy_images(epub, title_tree);
    }
  }

  status |= copy_images(epub, document);

 /*
  * Get document strings...
  */

  if ((title = get_title(title_tree)) == NULL)
    title = get_title(document);

  if ((author = htmlGetMeta(title_tree, (uchar *)"author")) == NULL)
    author = htmlGetMeta(document, (uchar *)"author");

  if ((copyright = htmlGetMeta(title_tree, (uchar *)"copyright")) == NULL)
    copyright = htmlGetMeta(document, (uchar *)"copyright");

  if ((docnumber = htmlGetMeta(title_tree, (uchar *)"docnumber")) == NULL)
    docnumber = htmlGetMeta(document, (uchar *)"docnumber");
  if (!docnumber)
  {
    if ((docnumber = htmlGetMeta(title_tree, (uchar *)"version")) == NULL)
      docnumber = htmlGetMeta(document, (uchar *)"version");
  }

  if ((language = htmlGetMeta(title_tree, (uchar *)"lang")) == NULL)
    language = htmlGetMeta(document, (uchar *)"lang");
  if (!language)
    language = (uchar *)"en-US";

  if ((subject = htmlGetMeta(title_tree, (uchar *)"keywords")) == NULL)
    subject = htmlGetMeta(document, (uchar *)"keywords");
  if (!subject)
    subject = (uchar *)"Unknown";

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
  * Write the document content...
  */

  if (!status && (epubf = zipcCreateFile(epub, "OEBPS/body.xhtml", 1)) != NULL)
  {
    status |= write_header(epubf, title, author, copyright, docnumber, NULL);
    if (TitlePage)
    {
      progress_show("Copying title page to EPUB container...");

      status |= write_title(epubf, title_tree, title, author, copyright, docnumber);
    }

    while (document != NULL)
    {
      progress_show("Copying \"%s\" to EPUB container...", (char *)htmlGetVariable(document, (uchar *)"_HD_FILENAME"));

      status |= write_all(epubf, document->child);

      document = document->next;
    }

    status |= zipcFilePuts(epubf, "</body>\n</html>\n");
    status |= zipcFileFinish(epubf);
  }
  else
    status = -1;

 /*
  * Write the package manifest...
  */

  if (!status && (epubf = zipcCreateFile(epub, "OEBPS/package.opf", 1)) != NULL)
  {
    const char *uid = docnumber ? (char *)docnumber : file_basename(OutputPath);

    status |= write_xhtmlf(epubf,
                           "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                           "<package xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"%s\" version=\"3.0\">\n"
                           "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n"
                           "    <dc:title>%s</dc:title>\n"
                           "    <dc:creator>%s</dc:creator>\n"
                           "    <meta property=\"dcterms:modified\">%s</meta>\n"
                           "    <dc:language>%s</dc:language>\n"
                           "    <dc:subject>%s</dc:subject>\n"
                           "    <dc:rights>%s</dc:rights>\n"
                           "    <dc:publisher>htmldoc</dc:publisher>\n"
                           "    <dc:identifier id=\"bookid\">%s</dc:identifier>\n",
                           uid, title, author, get_iso_date(time(NULL)), language, subject, copyright, uid);

    if (cover_image)
      status |= write_xhtmlf(epubf, "    <meta name=\"cover\" content=\"%s\" />\n", cover_image);
    status |= zipcFilePuts(epubf,
                           "  </metadata>\n"
                           "  <manifest>\n"
                           "    <item id=\"nav\" href=\"nav.xhtml\" media-type=\"application/xhtml+xml\" properties=\"nav\" />\n"
                           "    <item id=\"body\" href=\"body.xhtml\" media-type=\"application/xhtml+xml\" />\n");
    for (size_t i = 0; !status && i < num_images; i ++)
    {
      const char *mimetype, *image_ext = file_extension(images[i]);

      if (!strcmp(image_ext, "bmp"))
        mimetype = "image/bmp";
      else if (!strcmp(image_ext, "gif"))
        mimetype = "image/gif";
      else if (!strcmp(image_ext, "jpg"))
        mimetype = "image/jpeg";
      else
        mimetype = "image/png";

      status |= write_xhtmlf(epubf, "    <item id=\"%s\" href=\"%s\" media-type=\"%s\" />\n", images[i], images[i], mimetype);
    }
    status |= zipcFilePuts(epubf,
                           "  </manifest>\n"
                           "  <spine>\n"
                           "    <itemref idref=\"body\" />\n"
                           "  </spine>\n"
                           "</package>\n");
    status |= zipcFileFinish(epubf);
  }

 /*
  * Finally the table-of-contents file...
  */

  if ((epubf = zipcCreateFile(epub, "OEBPS/nav.xhtml", 1)) != NULL)
  {
    progress_show("Copying table of contents to EPUB container...");

    status |= write_xhtmlf(epubf,
                           "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                           "<!DOCTYPE html>\n"
                           "<html xmlns=\"http://www.w3.org/1999/xhtml\" "
                           "xmlns:epub=\"http://www.idpf.org/2007/ops\">\n"
                           "  <head>\n"
                           "    <title>%s</title>\n"
                           "    <style>ol { list-style-type: none; }</style>\n"
                           "  </head>\n"
                           "  <body>\n"
                           "    <nav epub:type=\"toc\">\n"
                           "      <ol>\n", title ? (char *)title : "Unknown");
    status |= write_toc(epubf, toc);
    status |= zipcFilePuts(epubf, "      </ol>\n"
                                  "    </nav>\n"
                                  "  </body>\n"
                                  "</html>\n");
    status |= zipcFileFinish(epubf);
  }
  else
    status = -1;

  status |= zipcClose(epub);

  if (!stat(OutputPath, &epubinfo))
    progress_error(HD_ERROR_NONE, "BYTES: %ld", (long)epubinfo.st_size);

  if (title != NULL)
    free(title);

  if (title_tree)
    htmlDeleteTree(title_tree);

  if (alloc_links)
  {
    free(links);

    num_links   = 0;
    alloc_links = 0;
    links       = NULL;
  }

  return (status);
}


/*
 * 'write_header()' - Output the standard "header" for a HTML file.
 */

static int                              /* O - 0 on success, -1 on failure */
write_header(
    zipc_file_t *out,                   /* I - Output file */
    uchar       *title,                 /* I - Title for document */
    uchar       *author,                /* I - Author for document */
    uchar       *copyright,             /* I - Copyright for document */
    uchar       *docnumber,             /* I - ID number for document */
    tree_t      *t)                     /* I - Current document file */
{
  int status = 0;                       /* Write status */
  static const char *families[] =       /* Typeface names */
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


  status |= zipcFilePuts(out,
                         "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                         "<!DOCTYPE html>\n"
                         "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\">\n"
                         "  <head>\n");
  if (title != NULL)
    status |= write_xhtmlf(out, "    <title>%s</title>\n", title);
  if (author != NULL)
    status |= write_xhtmlf(out, "    <meta name=\"author\" content=\"%s\" />\n", author);
  if (copyright != NULL)
    status |= write_xhtmlf(out, "    <meta name=\"copyright\" content=\"%s\" />\n", copyright);
  if (docnumber != NULL)
    status |= write_xhtmlf(out, "    <meta name=\"docnumber\" content=\"%s\" />\n", docnumber);
  status |= zipcFilePuts(out,
                         "    <style type=\"text/css\"><![CDATA[\n"
                         "body {\n");

  if (BodyImage[0])
    status |= write_xhtmlf(out, "  background: url(%s);\n", file_basename(BodyImage));
  else if (BodyColor[0])
    status |= zipcFilePrintf(out, "  background: #%s;\n", BodyColor);

  if (_htmlTextColor[0])
    status |= zipcFilePrintf(out, "  color: #%s;\n", _htmlTextColor);

  status |= zipcFilePrintf(out, "  font-family: %s;\n", families[_htmlBodyFont]);

  status |= zipcFilePuts(out, "}\n");

  if (!LinkStyle)
    status |= zipcFilePuts(out, "a:link {\n"
                                "  text-decoration: none;\n"
                                "}\n");
  if (LinkColor[0])
    status |= zipcFilePrintf(out, "a:link, a:link:visited, a:link:active {\n"
                                  "  color: #%s;\n"
                                  "}\n", LinkColor);

  status |= zipcFilePrintf(out, "h1, h2, h3, h4, h5, h6 {\n"
                                "  font-family: %s;\n"
                                "  page-break-inside: avoid;\n"
                                "}\n", families[_htmlHeadingFont]);
  status |= zipcFilePuts(out, "h1 {\n"
                              "  font-size: 250%;\n"
                              "  font-weight: bold;\n"
                              "  margin: 0;\n"
                              "}\n"
                              "h2 {\n"
                              "  font-size: 250%;\n"
                              "  margin: 1.5em 0 0;\n"
                              "}\n"
                              "h3 {\n"
                              "  font-size: 150%;\n"
                              "  margin: 1.5em 0 0.5em;\n"
                              "}\n"
                              "h4 {\n"
                              "  font-size: 110%;\n"
                              "  margin: 1.5em 0 0.5em;\n"
                              "}\n"
                              "h5, h6 {\n"
                              "  font-size: 100%;\n"
                              "  margin: 1.5em 0 0.5em;\n"
                              "}\n");

  status |= zipcFilePuts(out, "sub, sup {\n"
                              "  font-size: smaller;\n"
                              "}\n");

  status |= zipcFilePuts(out, "blockquote {\n"
                              "  border: solid thin gray;\n"
                              "  box-shadow: 3px 3px 5px rgba(0,0,0,0.5);\n"
                              "  padding: 0px 10px;\n"
                              "  page-break-inside: avoid;\n"
                              "}\n");

  status |= zipcFilePuts(out, "code, kbd, pre {\n"
                              "  font-family: monospace;\n"
                              "  font-size: 90%;\n"
                              "}\n"
                              "p code, li code, pre {\n"
                              "  background: rgba(127,127,127,0.1);\n"
                              "  border: thin dotted gray;\n"
                              "  hyphens: manual;\n"
                              "  -webkit-hyphens: manual;\n"
                              "  page-break-inside: avoid;\n"
                              "}\n"
                              "pre {\n"
                              "  padding: 10px;\n"
                              "}\n"
                              "p code, li code {\n"
                              "  padding: 2px 5px;\n"
                              "}\n");

  status |= zipcFilePuts(out, "dl {\n"
                              "  margin-top: 0;\n"
                              "}\n"
                              "dt {\n"
                              "  font-style: italic;\n"
                              "  margin-top: 0;\n"
                              "}\n"
                              "dd {\n"
                              "  margin-bottom: 0.5em;\n"
                              "}\n");

  status |= zipcFilePuts(out, "table {\n"
                              "  border-collapse: collapse;\n"
                              "  page-break-inside: avoid;\n"
                              "}\n"
                              "td, th {\n"
                              "  border: thin solid black;\n"
                              "  padding: 5px;\n"
                              "}\n"
                              "th {\n"
                              "  background: #444;\n"
                              "  color: white;\n"
                              "  font-weight: bold;\n"
                              "  text-align: center;\n"
                              "}\n");

  status |= zipcFilePuts(out, "]]></style>\n"
                              "  </head>\n"
                              "  <body>\n");

  return (status);
}


/*
 * 'write_title()' - Write a title page...
 */

static int                              /* O - 0 on success, -1 on failure */
write_title(zipc_file_t *out,           /* I - Output file */
            tree_t      *title_tree,	/* I - Title tree, if any */
            uchar       *title,         /* I - Title for document */
            uchar       *author,	/* I - Author for document */
            uchar       *copyright,	/* I - Copyright for document */
            uchar       *docnumber)	/* I - ID number for document */
{
  int           status = 0;             /* Write status */


  if (title_tree)
  {
    // Write a custom HTML title page...
    status |= write_all(out, title_tree);
  }
  else
  {
    // Write a "standard" title page with image...
    status |= zipcFilePuts(out, "    <div style=\"text-align: center;\">\n");

    if (TitleImage[0])
    {
      image_t *img = image_load(TitleImage, !OutputColor);

      status |= write_xhtmlf(out, "      <p><img src=\"%s\" width=\"%d\" height=\"%d\" alt=\"%s\" /></p>\n", file_basename((char *)TitleImage), img->width, img->height, title ? title : (uchar *)"");
    }

    if (title != NULL)
      status |= write_xhtmlf(out, "      <h1 style=\"text-align: center;\">%s</h1>\n", title);

    const char *prefix = "      <p>";

    if (docnumber)
    {
      status |= zipcFilePuts(out, prefix);
      status |= write_xhtml(out, docnumber);
      prefix = "<br />\n";
    }

    if (author)
    {
      status |= zipcFilePuts(out, prefix);
      status |= write_xhtml(out, author);
      prefix = "<br />\n";
    }

    if (copyright)
    {
      status |= zipcFilePuts(out, prefix);
      status |= write_xhtml(out, copyright);
      prefix = "<br />\n";
    }

    if (prefix[0] == '<')
      status |= zipcFilePuts(out, "</p>\n");

    status |= zipcFilePuts(out, "    </div>\n");
  }

  return (status);
}


/*
 * 'write_all()' - Write all markup text for the given tree.
 */

static int                              /* O - 0 on success, -1 on error */
write_all(zipc_file_t *out,		/* I - Output file */
          tree_t      *t)		/* I - Document tree */
{
  while (t != NULL)
  {
    if (write_node(out, t))
      return (-1);

    if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
    {
      if (write_all(out, t->child))
        return (-1);
    }

    if (write_nodeclose(out, t))
      return (-1);

    t = t->next;
  }

  return (0);
}


/*
 * 'write_node()' - Write a single tree node.
 */

static int                              /* O - 0 on success, -1 on error */
write_node(zipc_file_t *out,		/* I - Output file */
           tree_t      *t)		/* I - Document tree node */
{
  int status = 0;                       /* Write status */
  int i;                                /* Looping var */


  switch (t->markup)
  {
    case MARKUP_NONE :
        if (t->data == NULL)
	  break;

        if (!t->prev && t->parent && t->parent->markup == MARKUP_PRE && !strcmp((char *)t->data, "\n"))
          break;                        /* Skip initial blank line */

        status |= write_xhtml(out, t->data);
        break;

    case MARKUP_CENTER : /* Not in XHTML, use <div style ...> instead */
        if (t->child)
          status |= zipcFilePuts(out, "<div style=\"margin-left: auto; margin-right: auto;\">");
        break;

    case MARKUP_TABLE : /* No HTML 3.x table attributes in XHTML... */
        if (t->child)
          status |= zipcFilePuts(out, "<table>\n");
        break;

    case MARKUP_TT : /* Not in XHTML, use <code> instead... */
        if (t->child)
          status |= zipcFilePuts(out, "<code>");
        break;

    case MARKUP_COMMENT :
    case MARKUP_UNKNOWN :
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
    case MARKUP_UL :
        status |= zipcFilePuts(out, "\n");

    default :
        if (t->markup != MARKUP_EMBED)
	{
	  status |= zipcFilePrintf(out, "<%s", _htmlMarkups[t->markup]);

	  for (i = 0; i < t->nvars; i ++)
	  {
	    if (strcasecmp((char *)t->vars[i].name, "BREAK") == 0 && t->markup == MARKUP_HR)
	      continue;

	    if (strcasecmp((char *)t->vars[i].name, "REALSRC") == 0 && t->markup == MARKUP_IMG)
	      continue;

            if (strncasecmp((char *)t->vars[i].name, "_HD_", 4) == 0)
	      continue;

	    if (t->vars[i].value == NULL)
              status |= write_xhtmlf(out, " %ls=\"%ls\"", t->vars[i].name, t->vars[i].name);
            else if (t->markup == MARKUP_A && !strcasecmp((char *)t->vars[i].name, "NAME"))
              status |= write_xhtmlf(out, " id=\"%s\"", t->vars[i].value);
            else if (!strcasecmp((char *)t->vars[i].name, "ALIGN"))
              status |= write_xhtmlf(out, " style=\"text-align: %ls;\"", t->vars[i].value);
	    else
              status |= write_xhtmlf(out, " %ls=\"%s\"", t->vars[i].name, t->vars[i].value);
	  }

          if (t->child)
            status |= zipcFilePuts(out, ">");
          else
            status |= zipcFilePuts(out, " />");
	}
	break;
  }

  return (status);
}


/*
 * 'write_nodeclose()' - Close a single tree node.
 */

static int                              /* O - 0 on success, -1 on error */
write_nodeclose(zipc_file_t   *out,	/* I - Output file */
                tree_t        *t)	/* I - Document tree node */
{
  if (t->markup != MARKUP_HEAD && t->markup != MARKUP_TITLE)
  {
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
      case MARKUP_TR :
      case MARKUP_UL :
          if (!t->child)
            break;

          if (zipcFilePrintf(out, "</%s>\n", _htmlMarkups[t->markup]))
            return (-1);
          break;

      case MARKUP_CENTER : /* Not in XHTML, use <div style ...> instead */
          if (t->child)
          {
            if (zipcFilePuts(out, "</div>\n"))
              return (-1);
          }
          break;

      case MARKUP_TABLE : /* No HTML 3.x table attributes in XHTML... */
          if (t->child)
          {
            if (zipcFilePuts(out, "</table>\n"))
              return (-1);
          }
          break;

      case MARKUP_TT : /* Not in XHTML, use <code> instead... */
          if (t->child)
          {
            if (zipcFilePuts(out, "</code>"))
              return (-1);
          }
          break;

      default :
          if (!t->child)
            break;

          if (zipcFilePrintf(out, "</%s>", _htmlMarkups[t->markup]))
            return (-1);
          break;
    }
  }

  return (0);
}


/*
 * 'write_toc()' - Write all markup text for the given table-of-contents.
 */

static int                              /* O - 0 on success, -1 on error */
write_toc(zipc_file_t *out,		/* I - Output file */
          tree_t      *t)		/* I - Document tree */
{
  int   status = 0;                     /* Write status */
  uchar *href;                          /* Link to heading */


  while (t)
  {
    if (htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC") == NULL)
    {
      switch (t->markup)
      {
        case MARKUP_NONE :
            status |= write_xhtml(out, t->data);
            break;

        case MARKUP_A :
            if ((href = htmlGetVariable(t, (uchar *)"HREF")) != NULL)
            {
              status |= write_xhtmlf(out, "<a href=\"body.xhtml%s\">", href);
              status |= write_toc(out, t->child);
              status |= zipcFilePuts(out, "</a>");
            }
            break;

        case MARKUP_B :
            status |= zipcFilePuts(out, "        <li>");
            status |= write_toc(out, t->child);
            if (!t->next || t->next->markup != MARKUP_UL)
              status |= zipcFilePuts(out, "</li>\n");
            break;

        case MARKUP_LI :
            status |= zipcFilePuts(out, "          <li>");
            status |= write_toc(out, t->child);
            status |= zipcFilePuts(out, "</li>\n");
            break;

        case MARKUP_UL :
            status |= zipcFilePuts(out, "<ol>\n");
            status |= write_toc(out, t->child);
            status |= zipcFilePuts(out, "        </ol>");
            if (t->prev && t->prev->markup == MARKUP_B)
              status |= zipcFilePuts(out, "</li>\n");
            break;

        case MARKUP_H1 :
        case MARKUP_BR :
            break;

        default :
            if (t->child)
              status |= write_toc(out, t->child);
            break;

      }
    }

    t = t->next;
  }

  return (status);
}


/*
 * 'get_iso_date()' - Get an ISO-formatted date/time string.
 */

static char *				/* O - ISO date/time string */
get_iso_date(time_t t)			/* I - Time value */
{
  struct tm	*date;			/* UTC date/time */
  static char	buffer[100];		/* String buffer */


  date = gmtime(&t);

  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ", date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
  return (buffer);
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
 * 'compare_images()' - Compare two image filenames...
 */

static int
compare_images(char **a,
               char **b)
{
  return (strcmp(*a, *b));
}


/*
 * 'copy_image()' - Copy an image to the ZIP container.
 */

static int                              /* O - 0 on success, -1 on failure */
copy_image(zipc_t     *zipc,            /* I - ZIP container */
           const char *filename)        /* I - File to copy */
{
  const char  *base = file_basename(filename);
                                      /* Base filename */
  char        epubname[1024];         /* Name in ZIP container */


 /*
  * Don't copy more than once for the same file...
  */

  if (num_images > 0 && bsearch(&base, images, num_images, sizeof(char *), (compare_func_t)compare_images))
    return (0);

  progress_show("Copying \"%s\" to EPUB container...", base);

 /*
  * Copy the file...
  */

  snprintf(epubname, sizeof(epubname), "OEBPS/%s", base);

  if (zipcCopyFile(zipc, epubname, filename, 0, 0))
  {
    progress_error(HD_ERROR_WRITE_ERROR, "Unable to copy \"%s\": %s", base, zipcError(zipc));
    return (-1);
  }

 /*
  * Add it to the array of images...
  */

  if (num_images >= alloc_images)
  {
    char **temp;

    alloc_images += 128;
    if (alloc_images == 128)
      temp = (char **)malloc(alloc_images * sizeof(char *));
    else
      temp = (char **)realloc(images, alloc_images * sizeof(char *));

    if (!temp)
      return (-1);

    images = temp;
  }

  images[num_images] = strdup(base);
  num_images ++;

  if (num_images > 1)
    qsort(images, num_images, sizeof(char *), (compare_func_t)compare_images);

  return (0);
}


/*
 * 'copy_images()' - Scan the tree for images and copy as needed...
 */

static int                              /* O - 0 on success, -1 on failure */
copy_images(zipc_t *zipc,               /* I - ZIP container */
            tree_t *t)                  /* I - Document tree */
{
  uchar     *src,                       /* Image source */
            *realsrc;                   /* Real image source */


  while (t)
  {
   /*
    * If this is an image node, copy the image and update the SRC...
    */

    if (t->markup == MARKUP_IMG && (src = htmlGetVariable(t, (uchar *)"SRC")) != NULL && (realsrc = htmlGetVariable(t, (uchar *)"REALSRC")) != NULL && file_method((char *)src) == NULL)
    {
      if (copy_image(zipc, (char *)realsrc))
        return (-1);

      htmlSetVariable(t, (uchar *)"SRC", (uchar *)file_basename((char *)realsrc));
    }

   /*
    * Move to the next node in the document tree...
    */

    t = walk_next(t);
  }

  return (0);
}


/*
 * 'walk_next()' - Return the next node in the tree.
 */

static tree_t *                         /* O - Next node */
walk_next(tree_t *t)                    /* I - Current node */
{
  if (t->child)
    return (t->child);
  else if (t->next)
    return (t->next);
  else if (t->parent)
  {
    do
    {
      t = t->parent;
    }
    while (t && !t->next);

    if (t)
      return (t->next);
    else
      return (NULL);
  }
  else
    return (NULL);
}


/*
 * 'write_xhtml()' - Write an XHTML-safe string.
 */

static int                              /* O - 0 on success, -1 on error */
write_xhtml(zipc_file_t *out,           /* I - Output file */
            uchar       *s)             /* I - String to write */
{
  int   status = 0;                     /* Return status */
  uchar *start,                         /* First character in sequence */
        *ptr;                           /* Current character */


  for (ptr = s, start = s; *ptr; ptr ++)
  {
    if (*ptr > 0x7f || strchr("<>&\"", *ptr))
    {
      if (ptr > start)
        status |= zipcFileWrite(out, start, (size_t)(ptr - start));

      status |= zipcFilePuts(out, (char *)xhtml_entity(*ptr));
      start = ptr + 1;
    }
  }

  if (ptr > start)
    status |= zipcFileWrite(out, start, (size_t)(ptr - start));

  return (status);
}


/*
 * 'write_xhtmlf()' - Write an XHTML-safe printf string.
 */

static int                              /* O - 0 on success, -1 on error */
write_xhtmlf(zipc_file_t *out,          /* I - Output file */
             const char  *format,       /* I - Printf-style string to write */
             ...)                       /* I - Additional args as needed */
{
  int           status = 0;             /* Return status */
  va_list       ap;                     /* Additional arguments */
  uchar         *start,                 /* First character in sequence */
                *ptr;                   /* Current character */
  const char	*s;			/* String pointer */
  int		d;			/* Number */
  char          temp[32];               /* Temporary string buffer */

  va_start(ap, format);

  for (ptr = (uchar *)format, start = (uchar *)format; *ptr; ptr ++)
  {
    if (*ptr == '%')
    {
     /*
      * Format character - write any pending text fragment...
      */

      if (ptr > start)
      {
       /*
        * Include the % if the format char is %%...
        */

        if (ptr[1] == '%')
          status |= zipcFileWrite(out, start, (size_t)(ptr - start + 1));
        else
          status |= zipcFileWrite(out, start, (size_t)(ptr - start));
      }

     /*
      * Start over and process the character...
      */

      ptr ++;
      start = ptr + 1;

      switch (*ptr)
      {
        case '%' : /* Escaped % */
            break;

        case 'd' : /* Substitute a single integer */
            d = va_arg(ap, int);
            snprintf(temp, sizeof(temp), "%d", d);
            status |= zipcFilePuts(out, temp);
            break;

        case 'l' : /* Substitude (and lower-case) a single string */
            if (ptr[1] != 's')
            {
              start = ptr - 1;
              break;
            }

            ptr ++;
            start = ptr + 1;

            s = va_arg(ap, const char *);
            if (!s)
              s = "(null)";
            while (*s)
            {
              status |= zipcFilePuts(out, (char *)xhtml_entity((uchar)tolower(*s & 255)));
              s ++;
            }
            break;

        case 's' : /* Substitute a single string */
            s = va_arg(ap, const char *);
            if (!s)
              s = "(null)";
            status |= write_xhtml(out, (uchar *)s);
            break;

        default : /* Something else we don't support... */
            start = ptr - 1;
            break;
      }
    }
    else if (*ptr > 0x7f)
    {
      if (ptr > start)
        status |= zipcFileWrite(out, start, (size_t)(ptr - start));

      status |= zipcFilePuts(out, (char *)xhtml_entity(*ptr));
      start = ptr + 1;
    }
  }

  if (ptr > start)
    status |= zipcFileWrite(out, start, (size_t)(ptr - start));

  return (status);
}
