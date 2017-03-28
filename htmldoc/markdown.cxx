/*
 * Markdown parsing definitions for HTMLDOC, a HTML document processing program.
 *
 * Copyright 2017 by Michael R Sweet.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * Include necessary headers...
 */

#  include "markdown.h"


/*
 * 'mdReadFile()' - Read a Markdown file.
 */

tree_t *				/* O - HTML document tree */
mdReadFile(tree_t     *parent,		/* I - Parent node */
           FILE       *fp,		/* I - File to read from */
           const char *base)		/* I - Base path/URL */
{
  tree_t	*mdblock,		/* Current block element */
		*mdinline,		/* Current inline element */
		*lists[8];		/* Current list elements */
  int		num_lists = 0;		/* Number of list elements */
  char		line[65536],		/* Line from file */
		*lineptr;		/* Pointer into line */


}
