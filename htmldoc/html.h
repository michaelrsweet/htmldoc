/*
 * "$Id: html.h,v 1.16 2004/03/31 10:35:07 mike Exp $"
 *
 *   HTML parsing definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2004 by Easy Software Products.
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
 *       Hollywood, Maryland 20636-3142 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 */

#ifndef _HTML_H_
#  define _HTML_H_

/*
 * Include necessary headers...
 */

#  include <stdio.h>
#  include <stdlib.h>

#  include "style.h"
#  include "hdstring.h"
#  include "iso8859.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Sizes...
 */

#  define SIZE_H1	6
#  define SIZE_H2	5
#  define SIZE_H3	4
#  define SIZE_H4	3
#  define SIZE_H5	2
#  define SIZE_H6	1
#  define SIZE_H7	0
#  define SIZE_P	3
#  define SIZE_PRE	2
#  define SIZE_SUB	-2
#  define SIZE_SUP	-2


/*
 * Prototypes...
 */

struct hdTree;

extern hdTree	*htmlReadFile(hdTree *parent, FILE *fp, const char *base);
extern int	htmlWriteFile(hdTree *parent, FILE *fp);

extern hdTree	*htmlAddTree(hdTree *parent, hdElement markup, uchar *data);
extern int	htmlDeleteTree(hdTree *parent);
extern hdTree	*htmlInsertTree(hdTree *parent, hdElement markup, uchar *data);
extern hdTree	*htmlNewTree(hdTree *parent, hdElement markup, uchar *data);

extern uchar	*htmlGetText(hdTree *tree);
extern uchar	*htmlGetMeta(hdTree *tree, uchar *name);

extern uchar	*htmlGetVariable(hdTree *t, uchar *name);
extern int	htmlSetVariable(hdTree *t, uchar *name, uchar *value);

extern uchar	*htmlGetStyle(hdTree *t, uchar *name);

extern void	htmlSetBaseSize(float p, float s);
extern void	htmlSetCharSet(const char *cs);
extern void	htmlSetTextColor(uchar *color);

extern void	htmlDebugStats(const char *title, hdTree *t);


/*
 * Markup variables...
 */

struct hdAttr
{
  uchar			*name,		/* Variable name */
			*value;		/* Variable value */
};

/*
 * Parsing tree...
 */

struct hdTree
{
  hdTree	*parent,		/* Parent tree entry */
		*child,			/* First child entry */
		*last_child,		/* Last child entry */
		*prev,			/* Previous entry on this level */
		*next,			/* Next entry on this level */
		*link;			/* Linked-to */
  hdElement	element;		/* Element */
  uchar		*data;			/* Text (HD_ELEMENT_NONE or HD_ELEMENT_COMMENT) */
  hdStyle	*css;			/* Stylesheet data */
  unsigned	halignment:2,		/* Horizontal alignment */
		valignment:3,		/* Vertical alignment */
		typeface:4,		/* Typeface code */
		style:2,		/* Style of text */
		underline:1,		/* Text is underlined? */
		strikethrough:1,	/* Text is struck-through? */
		subscript:1,		/* Text is subscripted? */
		superscript:1,		/* Text is superscripted? */
		preformatted:2,		/* Preformatted text? */
		indent:4;		/* Indentation level 0-15 */
  uchar		red,			/* Color of this fragment */
		green,
		blue;
  float		width,			/* Width of this fragment in points */
		height,			/* Height of this fragment in points */
		size;			/* Point size of text */
  int		nvars;			/* Number of variables... */
  hdAttr	*vars;			/* Variables... */

  const char	*get_attr(const char *n) { return ((const char *)htmlGetVariable(this, (uchar *)n)); }
  static hdElement get_element(const char *n);
};


/*
 * Globals...
 */

extern const char	*_htmlMarkups[];
extern const char	*_htmlData;
extern float		_htmlPPI;
extern int		_htmlGrayscale;
extern uchar		_htmlTextColor[];
extern float		_htmlBrowserWidth;
extern float		_htmlSizes[],
			_htmlSpacings[];
extern hdFontFace	_htmlBodyFont,
			_htmlHeadingFont;
extern char		_htmlCharSet[];
extern float		_htmlWidths[4][4][256];
extern const char	*_htmlGlyphs[];
extern const char	*_htmlFonts[4][4];


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HTML_H_ */

/*
 * End of "$Id: html.h,v 1.16 2004/03/31 10:35:07 mike Exp $".
 */
