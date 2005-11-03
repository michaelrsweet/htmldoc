/*
 * "$Id$"
 *
 *   HTML parsing definitions for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2005 by Easy Software Products.
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

#ifndef _HTMLDOC_HTML_H_
#  define _HTMLDOC_HTML_H_

/*
 * Include necessary headers...
 */

#  include <stdio.h>
#  include <stdlib.h>

#  include "file.h"
#  include "hdstring.h"
#  include "style.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Element attributes...
 */

typedef struct
{
  char		*name;			//* Attribute name
  hdChar	*value;			//* Attribute value
} hdTreeAttr;

/*
 * Parsing tree...
 */

struct hdTree
{
  struct hdTree	*parent,		//* Parent tree entry
		*child,			//* First child entry
		*last_child,		//* Last child entry
		*prev,			//* Previous entry on this level
		*next,			//* Next entry on this level
		*link;			//* Linked-to
  hdElement	element;		//* Markup code
  hdStyle	*style;			//* CSS data
  hdUTF8	data;			//* Text (HD_ELEMENT_NONE or HD_ELEMENT_COMMENT)
  float		width,			//* Width of this fragment in points
		height;			//* Height of this fragment in points
  int		nattrs;			//* Number of attributes...
  hdTreeAttr	*attrs;			//* Attributes...
};


/*
 * Globals...
 */

extern const char	*_htmlData;
extern hdFontFace	_htmlBodyFont,
			_htmlHeadingFont;
extern hdStyleSheet	*_htmlStyleSheet;


/*
 * Prototypes...
 */

extern hdTree	*htmlReadFile(hdTree *parent, FILE *fp, const char *base);
extern int	htmlWriteFile(hdTree *parent, FILE *fp);

extern hdTree	*htmlAddTree(hdTree *parent, hdElement element, hdChar *data);
extern int	htmlDeleteTree(hdTree *parent);
extern hdTree	*htmlInsertTree(hdTree *parent, hdElement element, hdChar *data);
extern hdTree	*htmlNewTree(hdTree *parent, hdElement element, hdChar *data);

extern hdTree	*htmlFindFile(hdTree *doc, const char *filename);
extern hdTree	*htmlFindTarget(hdTree *doc, hdChar *name);
extern void	htmlFixLinks(hdTree *doc, hdTree *tree, const char *base = 0);

extern hdElement htmlGetElement(const char *name);
extern hdChar	*htmlGetText(hdTree *tree);
extern hdChar	*htmlGetMeta(hdTree *tree, const char *name);

extern hdChar	*htmlGetAttr(hdTree *t, const char *name);
extern int	htmlSetAttr(hdTree *t, const char *name, hdChar *value);

extern void	htmlUpdateStyle(hdTree *t, const char *base);
extern void	htmlDeleteStyleSheet(void);
extern void	htmlInitStyleSheet(void);

extern void	htmlDebugStats(const char *title, hdTree *t);
extern void	htmlDebugStyleStats(void);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HTMLDOC_HTML_H_ */

/*
 * End of "$Id$".
 */
