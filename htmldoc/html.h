/*
 * "$Id: html.h,v 1.9.2.14 2004/04/15 19:58:20 mike Exp $"
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
 *       Hollywood, Maryland 20636-3111 USA
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

#  include "file.h"
#  include "hdstring.h"
#  include "iso8859.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Define some compatibility macros for Microsoft Windows...
 */

#  ifdef WIN32
#    define strcasecmp(s,t)	stricmp(s,t)
#    define strncasecmp(s,t,n)	strnicmp(s,t,n)
#  endif /* WIN32 */


/*
 * Markup constants...
 */

typedef enum
{
	MARKUP_FILE = -3,	/* File Delimiter */
	MARKUP_UNKNOWN = -2,	/* Unknown element */
	MARKUP_ERROR = -1,	
	MARKUP_NONE = 0,
	MARKUP_COMMENT,
	MARKUP_DOCTYPE,
	MARKUP_A,
	MARKUP_ACRONYM,
	MARKUP_ADDRESS,
	MARKUP_APPLET,
	MARKUP_AREA,
	MARKUP_B,
	MARKUP_BASE,
	MARKUP_BASEFONT,
	MARKUP_BIG,
	MARKUP_BLINK,
	MARKUP_BLOCKQUOTE,
	MARKUP_BODY,
	MARKUP_BR,
	MARKUP_CAPTION,
	MARKUP_CENTER,
	MARKUP_CITE,
	MARKUP_CODE,
	MARKUP_COL,
	MARKUP_COLGROUP,
	MARKUP_DD,
	MARKUP_DEL,
	MARKUP_DFN,
	MARKUP_DIR,
	MARKUP_DIV,
	MARKUP_DL,
	MARKUP_DT,
	MARKUP_EM,
	MARKUP_EMBED,
	MARKUP_FONT,
	MARKUP_FORM,
	MARKUP_FRAME,
	MARKUP_FRAMESET,
	MARKUP_H1,
	MARKUP_H2,
	MARKUP_H3,
	MARKUP_H4,
	MARKUP_H5,
	MARKUP_H6,
	MARKUP_H7,
	MARKUP_H8,
	MARKUP_H9,
	MARKUP_H10,
	MARKUP_H11,
	MARKUP_H12,
	MARKUP_H13,
	MARKUP_H14,
	MARKUP_H15,
	MARKUP_HEAD,
	MARKUP_HR,
	MARKUP_HTML,
	MARKUP_I,
	MARKUP_IMG,
	MARKUP_INPUT,
	MARKUP_INS,
	MARKUP_ISINDEX,
	MARKUP_KBD,
	MARKUP_LI,
	MARKUP_LINK,
	MARKUP_MAP,
	MARKUP_MENU,
	MARKUP_META,
	MARKUP_MULTICOL,
	MARKUP_NOBR,
	MARKUP_NOFRAMES,
	MARKUP_OL,
	MARKUP_OPTION,
	MARKUP_P,
	MARKUP_PRE,
	MARKUP_S,
	MARKUP_SAMP,
	MARKUP_SCRIPT,
	MARKUP_SELECT,
	MARKUP_SMALL,
	MARKUP_SPACER,
	MARKUP_STRIKE,
	MARKUP_STRONG,
	MARKUP_STYLE,
	MARKUP_SUB,
	MARKUP_SUP,
	MARKUP_TABLE,
	MARKUP_TBODY,
	MARKUP_TD,
	MARKUP_TEXTAREA,
	MARKUP_TFOOT,
	MARKUP_TH,
	MARKUP_THEAD,
	MARKUP_TITLE,
	MARKUP_TR,
	MARKUP_TT,
	MARKUP_U,
	MARKUP_UL,
	MARKUP_VAR,
	MARKUP_WBR
} markup_t;

/*
 * Horizontal alignment...
 */

typedef enum
{
	ALIGN_LEFT = 0,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_JUSTIFY
} halignment_t;

/*
 * Vertical alignment...
 */

typedef enum
{
	ALIGN_TOP = 0,
	ALIGN_MIDDLE,
	ALIGN_BOTTOM
} valignment_t;

/*
 * Typeface...
 */

typedef enum
{
	TYPE_COURIER = 0,
	TYPE_TIMES,
	TYPE_HELVETICA,
	TYPE_SYMBOL
} typeface_t;

/*
 * Style...
 */

typedef enum
{
	STYLE_NORMAL = 0,
	STYLE_BOLD,
	STYLE_ITALIC,
	STYLE_BOLD_ITALIC
} style_t;

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
 * Markup variables...
 */

typedef struct
{
  uchar			*name,		/* Variable name */
			*value;		/* Variable value */
} var_t;

/*
 * Parsing tree...
 */

typedef struct tree_str
{
  struct tree_str	*parent,	/* Parent tree entry */
			*child,		/* First child entry */
			*last_child,	/* Last child entry */
			*prev,		/* Previous entry on this level */
			*next,		/* Next entry on this level */
			*link;		/* Linked-to */
  markup_t		markup;		/* Markup code */
  uchar			*data;		/* Text (MARKUP_NONE or MARKUP_COMMENT) */
  unsigned		halignment:2,	/* Horizontal alignment */
			valignment:2,	/* Vertical alignment */
			typeface:2,	/* Typeface code */
			size:3,		/* Size of text */
			style:2,	/* Style of text */
			underline:1,	/* Text is underlined? */
			strikethrough:1,/* Text is struck-through? */
			subscript:1,	/* Text is subscripted? */
			superscript:1,	/* Text is superscripted? */
			preformatted:1,	/* Preformatted text? */
			indent:4;	/* Indentation level 0-15 */
  uchar			red,		/* Color of this fragment */
			green,
			blue;
  float			width,		/* Width of this fragment in points */
			height;		/* Height of this fragment in points */
  int			nvars;		/* Number of variables... */
  var_t			*vars;		/* Variables... */
} tree_t;


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
extern typeface_t	_htmlBodyFont,
			_htmlHeadingFont;
extern char		_htmlCharSet[];
extern float		_htmlWidths[4][4][256];
extern const char	*_htmlGlyphs[];
extern const char	*_htmlFonts[4][4];


/*
 * Prototypes...
 */

extern tree_t	*htmlReadFile(tree_t *parent, FILE *fp, const char *base);
extern int	htmlWriteFile(tree_t *parent, FILE *fp);

extern tree_t	*htmlAddTree(tree_t *parent, markup_t markup, uchar *data);
extern int	htmlDeleteTree(tree_t *parent);
extern tree_t	*htmlInsertTree(tree_t *parent, markup_t markup, uchar *data);
extern tree_t	*htmlNewTree(tree_t *parent, markup_t markup, uchar *data);

extern tree_t	*htmlFindFile(tree_t *doc, uchar *filename);
extern tree_t	*htmlFindTarget(tree_t *doc, uchar *name);
extern void	htmlFixLinks(tree_t *doc, tree_t *tree, uchar *base = 0);

extern uchar	*htmlGetText(tree_t *tree);
extern uchar	*htmlGetMeta(tree_t *tree, uchar *name);

extern uchar	*htmlGetVariable(tree_t *t, uchar *name);
extern int	htmlSetVariable(tree_t *t, uchar *name, uchar *value);

extern uchar	*htmlGetStyle(tree_t *t, uchar *name);

extern void	htmlSetBaseSize(float p, float s);
extern void	htmlSetCharSet(const char *cs);
extern void	htmlSetTextColor(uchar *color);

extern void	htmlDebugStats(const char *title, tree_t *t);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_HTML_H_ */

/*
 * End of "$Id: html.h,v 1.9.2.14 2004/04/15 19:58:20 mike Exp $".
 */
