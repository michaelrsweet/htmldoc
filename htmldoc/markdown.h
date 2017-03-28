/*
 * Markdown parsing definitions for HTMLDOC, a HTML document processing program.
 *
 * Copyright 2017 by Michael R Sweet.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

#ifndef _MARKDOWN_H_
#  define _MARKDOWN_H_

/*
 * Include necessary headers...
 */

#  include "html.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Prototypes...
 */

extern tree_t	*mdReadFile(tree_t *parent, FILE *fp, const char *base);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !_MARKDOWN_H_ */
