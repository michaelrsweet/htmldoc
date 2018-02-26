/*
 * Markdown parsing definitions for HTMLDOC, a HTML document processing program.
 *
 * Copyright © 2017-2018 by Michael R Sweet.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * Include necessary headers...
 */

#  include "markdown.h"
#  include "mmd.h"
#  include "progress.h"


/*
 * Local functions...
 */

static void       add_block(tree_t *hparent, mmd_t *parent);
static void       add_leaf(tree_t *hparent, mmd_t *node);
static uchar      *get_text(uchar *text);
static uchar      *make_anchor(mmd_t *block);
static uchar      *make_anchor(const uchar *text);


/*
 * 'mdReadFile()' - Read a Markdown file.
 */

tree_t *				/* O - HTML document tree */
mdReadFile(tree_t     *parent,		/* I - Parent node */
           FILE       *fp,		/* I - File to read from */
           const char *base)		/* I - Base path/URL */
{
  mmd_t       *doc = mmdLoadFile(fp);   /* Markdown document */
  tree_t      *html,                    /* HTML element */
              *head,                    /* HEAD element */
              *temp,                    /* META/TITLE element */
              *body;                    /* BODY element */
  const char  *meta;                    /* Title, author, etc. */


  html = htmlAddTree(parent, MARKUP_HTML, NULL);

  head = htmlAddTree(html, MARKUP_HEAD, NULL);
  if ((meta = mmdGetMetadata(doc, "title")) != NULL)
  {
    temp = htmlAddTree(head, MARKUP_TITLE, NULL);
    htmlAddTree(temp, MARKUP_NONE, get_text((uchar *)meta));
  }
  if ((meta = mmdGetMetadata(doc, "author")) != NULL)
  {
    temp = htmlAddTree(head, MARKUP_META, NULL);
    htmlSetVariable(temp, (uchar *)"name", (uchar *)"author");
    htmlSetVariable(temp, (uchar *)"content", get_text((uchar *)meta));
  }
  if ((meta = mmdGetMetadata(doc, "copyright")) != NULL)
  {
    temp = htmlAddTree(head, MARKUP_META, NULL);
    htmlSetVariable(temp, (uchar *)"name", (uchar *)"copyright");
    htmlSetVariable(temp, (uchar *)"content", get_text((uchar *)meta));
  }
  if ((meta = mmdGetMetadata(doc, "version")) != NULL)
  {
    temp = htmlAddTree(head, MARKUP_META, NULL);
    htmlSetVariable(temp, (uchar *)"name", (uchar *)"version");
    htmlSetVariable(temp, (uchar *)"content", get_text((uchar *)meta));
  }

  body = htmlAddTree(html, MARKUP_BODY, NULL);
  add_block(body, doc);

  mmdFree(doc);

  return (html);
}


/*
 * 'add_block()' - Add a block node.
 */

static void
add_block(tree_t *html,                 /* I - Parent HTML node */
          mmd_t  *parent)               /* I - Parent node */
{
  markup_t      element;                /* Enclosing element, if any */
  mmd_t         *node;                  /* Current child node */
  mmd_type_t    type;                   /* Node type */
  tree_t        *block;                 /* Block node */
  const char	*align = NULL;		/* Alignment */


  switch (type = mmdGetType(parent))
  {
    case MMD_TYPE_BLOCK_QUOTE :
        element = MARKUP_BLOCKQUOTE;
        break;

    case MMD_TYPE_ORDERED_LIST :
        element = MARKUP_OL;
        break;

    case MMD_TYPE_UNORDERED_LIST :
        element = MARKUP_UL;
        break;

    case MMD_TYPE_LIST_ITEM :
        element = MARKUP_LI;
        break;

    case MMD_TYPE_HEADING_1 :
        element = MARKUP_H1;
        break;

    case MMD_TYPE_HEADING_2 :
        element = MARKUP_H2;
        break;

    case MMD_TYPE_HEADING_3 :
        element = MARKUP_H3;
        break;

    case MMD_TYPE_HEADING_4 :
        element = MARKUP_H4;
        break;

    case MMD_TYPE_HEADING_5 :
        element = MARKUP_H5;
        break;

    case MMD_TYPE_HEADING_6 :
        element = MARKUP_H6;
        break;

    case MMD_TYPE_PARAGRAPH :
        element = MARKUP_P;
        break;

    case MMD_TYPE_CODE_BLOCK :
        block = htmlAddTree(html, MARKUP_PRE, NULL);

        for (node = mmdGetFirstChild(parent); node; node = mmdGetNextSibling(node))
          htmlAddTree(block, MARKUP_NONE, get_text((uchar *)mmdGetText(node)));
        return;

    case MMD_TYPE_THEMATIC_BREAK :
        htmlAddTree(html, MARKUP_HR, NULL);
        return;

    case MMD_TYPE_TABLE :
        element = MARKUP_TABLE;
        break;

    case MMD_TYPE_TABLE_HEADER :
        element = MARKUP_THEAD;
        break;

    case MMD_TYPE_TABLE_BODY :
        element = MARKUP_TBODY;
        break;

    case MMD_TYPE_TABLE_ROW :
        element = MARKUP_TR;
        break;

    case MMD_TYPE_TABLE_HEADER_CELL :
        element = MARKUP_TH;
        break;

    case MMD_TYPE_TABLE_BODY_CELL_LEFT :
        element = MARKUP_TD;
        break;

    case MMD_TYPE_TABLE_BODY_CELL_CENTER :
        element = MARKUP_TD;
        align   = "center";
        break;

    case MMD_TYPE_TABLE_BODY_CELL_RIGHT :
        element = MARKUP_TD;
        align   = "right";
        break;

    default :
        element = MARKUP_NONE;
        break;
  }

  if (element != MARKUP_NONE)
    block = htmlAddTree(html, element, NULL);
  else
    block = html;

  if (align)
  {
    htmlSetVariable(block, (uchar *)"align", (uchar *)align);

    if (!strcmp(align, "center"))
      block->halignment = ALIGN_CENTER;
    else
      block->halignment = ALIGN_RIGHT;
  }
  else if (element == MARKUP_TH)
  {
    block->halignment = ALIGN_CENTER;
    htmlSetVariable(block, (uchar *)"bgcolor", (uchar *)"#cccccc");
  }
  else if (element == MARKUP_TABLE)
  {
    htmlSetVariable(block, (uchar *)"border", (uchar *)"1");
    htmlSetVariable(block, (uchar *)"cellpadding", (uchar *)"2");
  }

  if (type >= MMD_TYPE_HEADING_1 && type <= MMD_TYPE_HEADING_6)
  {
   /*
    * Add an anchor for each heading...
    */

    block = htmlAddTree(block, MARKUP_A, NULL);
    htmlSetVariable(block, (uchar *)"id", make_anchor(parent));
  }

  for (node = mmdGetFirstChild(parent); node; node = mmdGetNextSibling(node))
  {
    if (mmdIsBlock(node))
      add_block(block, node);
    else
      add_leaf(block, node);
  }
}


/*
 * 'add_leaf()' - Add a leaf node.
 */

static void
add_leaf(tree_t *html,                  /* I - Parent HTML node */
         mmd_t  *node)                  /* I - Leaf node */
{
  tree_t        *parent;                /* HTML node for this text */
  markup_t      element;                /* HTML element for this text */
  uchar         buffer[1024],           /* Text with any added whitespace */
                *text,                  /* Text to write */
                *url;                   /* URL to write */


  text = get_text((uchar *)mmdGetText(node));
  url  = (uchar *)mmdGetURL(node);

  switch (mmdGetType(node))
  {
    case MMD_TYPE_EMPHASIZED_TEXT :
        element = MARKUP_EM;
        break;

    case MMD_TYPE_STRONG_TEXT :
        element = MARKUP_STRONG;
        break;

    case MMD_TYPE_STRUCK_TEXT :
        element = MARKUP_DEL;
        break;

    case MMD_TYPE_LINKED_TEXT :
        element = MARKUP_A;
        break;

    case MMD_TYPE_CODE_TEXT :
        element = MARKUP_CODE;
        break;

    case MMD_TYPE_IMAGE :
        if (mmdGetWhitespace(node))
          htmlAddTree(html, MARKUP_NONE, (uchar *)" ");

        parent = htmlAddTree(html, MARKUP_IMG, NULL);
        htmlSetVariable(parent, (uchar *)"src", url);
        if (text)
          htmlSetVariable(parent, (uchar *)"alt", text);
        return;

    case MMD_TYPE_HARD_BREAK :
        htmlAddTree(html, MARKUP_BR, NULL);
        return;

    case MMD_TYPE_SOFT_BREAK :
        htmlAddTree(html, MARKUP_WBR, NULL);
        return;

    case MMD_TYPE_METADATA_TEXT :
        return;

    default :
        element = MARKUP_NONE;
        break;
  }

  if (element == MARKUP_NONE)
    parent = html;
  else if ((parent = html->last_child) == NULL || parent->markup != element)
  {
    parent = htmlAddTree(html, element, NULL);

    if (element == MARKUP_A && url)
    {
      if (!strcmp((char *)url, "@"))
        htmlSetVariable(parent, (uchar *)"href", make_anchor(text));
      else
        htmlSetVariable(parent, (uchar *)"href", url);
    }
  }

  if (mmdGetWhitespace(node))
  {
    buffer[0] = ' ';
    strlcpy((char *)buffer + 1, (char *)text, sizeof(buffer) - 1);
    text = buffer;
  }

  htmlAddTree(parent, MARKUP_NONE, text);
}


/*
 * 'get_text()' - Get Markdown text in HTMLDOC's charset.
 */

static uchar *                          /* O - Encoded text */
get_text(uchar *text)                   /* I - Markdown text */
{
  uchar         *bufptr,                /* Pointer into buffer */
                *bufend;                /* End of buffer */
  int           unich;                  /* Unicode character */
  static uchar  buffer[8192];           /* Temporary buffer */


  if (!_htmlUTF8)
    return (text);

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while (*text && bufptr < bufend)
  {
    if (*text & 0x80)
    {
      unich = 0;

      if ((*text & 0xe0) == 0xc0)
      {
        if ((text[1] & 0xc0) != 0x80)
        {
          progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X %02X.", *text, text[1]);
          *bufptr++ = '?';
          text ++;
        }
        else
        {
          unich = ((*text & 0x1f) << 6) | (text[1] & 0x3f);
          text += 2;
        }
      }
      else if ((*text & 0xf0) == 0xe0)
      {
        if ((text[1] & 0xc0) != 0x80 || (text[2] & 0xc0) != 0x80)
        {
          progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X %02X %02X.", *text, text[1], text[2]);
          *bufptr++ = '?';
          text ++;
        }
        else
        {
          unich = ((*text & 0x0f) << 12) | ((text[1] & 0x3f) << 6) | (text[1] & 0x3f);
          text += 3;
        }
      }
      else
      {
        progress_error(HD_ERROR_READ_ERROR, "Bad UTF-8 character sequence %02X.", *text);
        *bufptr++ = '?';
        text ++;
      }

      if (unich)
      {
        if (_htmlCharacters[unich])
        {
          *bufptr++ = _htmlCharacters[unich];
        }
        else
        {
          uchar ch;                     /* 8-bit character */

          if (_htmlUTF8 >= 0x100)
          {
            progress_error(HD_ERROR_READ_ERROR, "Too many Unicode code points.");
            return (0);
          }

          ch = (uchar)_htmlUTF8++;

          _htmlCharacters[unich] = ch;
          _htmlUnicode[ch]       = unich;
          _htmlGlyphs[ch]        = _htmlGlyphsAll[unich];

          for (int i = 0; i < TYPE_MAX; i ++)
            for (int j = 0; j < STYLE_MAX; j ++)
              _htmlWidths[i][j][ch] = _htmlWidthsAll[i][j][unich];

          *bufptr++ = ch;
        }
      }
    }
    else
      *bufptr++ = *text++;
  }

  *bufptr = '\0';

  return (buffer);
}


/*
 * 'make_anchor()' - Make an anchor for internal links from a block node.
 */

static uchar *                          /* O - Anchor string */
make_anchor(mmd_t *block)               /* I - Block node */
{
  mmd_t         *node;                  /* Current child node */
  const char    *text;                  /* Text from block */
  uchar         *bufptr;                /* Pointer into buffer */
  static uchar  buffer[1024];           /* Buffer for anchor string */


  for (bufptr = buffer, node = mmdGetFirstChild(block); node; node = mmdGetNextSibling(node))
  {
    if (mmdGetWhitespace(node) && bufptr < (buffer + sizeof(buffer) - 1))
      *bufptr++ = '-';
    for (text = mmdGetText(node); text && *text && bufptr < (buffer + sizeof(buffer) -1); text ++)
    {
      if ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'z') || (*text >= 'A' && *text <= 'Z') || *text == '.' || *text == '-')
        *bufptr++ = (uchar)tolower(*text);
      else if (*text == ' ')
        *bufptr++ = '-';
    }
  }

  *bufptr = '\0';

  return (buffer);
}


/*
 * 'make_anchor()' - Make an anchor for internal links from text.
 */

static uchar *                          /* O - Anchor string */
make_anchor(const uchar *text)          /* I - Text */
{
  uchar         *bufptr;                /* Pointer into buffer */
  static uchar  buffer[1024];           /* Buffer for anchor string */


  for (bufptr = buffer; *text && bufptr < (buffer + sizeof(buffer) - 1); text ++)
  {
    if ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'z') || (*text >= 'A' && *text <= 'Z') || *text == '.' || *text == '-')
      *bufptr++ = (uchar)tolower(*text);
    else if (*text == ' ')
      *bufptr++ = '-';
  }

  *bufptr = '\0';

  return (buffer);
}
