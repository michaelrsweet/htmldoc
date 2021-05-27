/*
 * Table of contents generator for HTMLDOC, a HTML document processing
 * program.
 *
 * Copyright 2011-2019 by Michael R Sweet.
 * Copyright 1997-2010 by Easy Software Products.  All rights reserved.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

/*
 * Include necessary headers.
 */

#include "htmldoc.h"


/*
 * Local functions...
 */

static void	add_heading(tree_t *toc, tree_t *heading);
static void	parse_tree(tree_t *t);


/*
 * Local globals...
 */

static int	heading_numbers[15];
static uchar	heading_types[15] =
		{
		  '1', '1', '1', '1', '1', '1', '1', '1',
		  '1', '1', '1', '1', '1', '1', '1'
		};
static int	last_level;
static tree_t	*heading_parents[15];


/*
 * 'toc_build()' - Build a table of contents of the given HTML tree.
 */

tree_t *			/* O - Table of contents tree */
toc_build(tree_t *tree)		/* I - Document tree */
{
  tree_t	*toc,		/* TOC tree pointer */
		*title,		/* Title entry */
		*link;		/* Link entry */


  TocDocCount        = 0;
  last_level         = 0;	/* Currently at the "top" level */
  heading_numbers[0] = 0;	/* Start at 1 (see below) */

  toc = htmlAddTree(NULL, MARKUP_BODY, NULL);

  title = htmlAddTree(toc, MARKUP_H1, NULL);
  htmlSetVariable(title, (uchar *)"ALIGN", (uchar *)"CENTER");
  link = htmlAddTree(title, MARKUP_A, NULL);
  htmlSetVariable(link, (uchar *)"NAME", (uchar *)"CONTENTS");
  htmlAddTree(link, MARKUP_NONE, (uchar *)TocTitle);

  heading_parents[0]  = toc;
  heading_parents[1]  = toc;
  heading_parents[2]  = toc;
  heading_parents[3]  = toc;
  heading_parents[4]  = toc;
  heading_parents[5]  = toc;
  heading_parents[6]  = toc;
  heading_parents[7]  = toc;
  heading_parents[8]  = toc;
  heading_parents[9]  = toc;
  heading_parents[10] = toc;
  heading_parents[11] = toc;
  heading_parents[12] = toc;
  heading_parents[13] = toc;
  heading_parents[14] = toc;

  parse_tree(tree);

  return (toc);
}


/*
 * 'add_heading()' - Add heading records to the given toc entry...
 */

static void
add_heading(tree_t *toc,	/* I - Table of contents */
            tree_t *heading)	/* I - Heading entry */
{
  while (heading != NULL)
  {
    if (heading->markup != MARKUP_UNKNOWN && heading->child != NULL)
      add_heading(toc, heading->child);
    else if (heading->markup == MARKUP_NONE && heading->data != NULL)
      htmlAddTree(toc, MARKUP_NONE, heading->data);

    heading = heading->next;
  }
}


/*
 * 'parse_tree()' - Parse headings from the given tree...
 *
 * Note: We also add anchor points and numbers as necessary...
 */

static void			/* O - Tree of TOC entries */
parse_tree(tree_t *t)		/* I - Document tree */
{
  tree_t	*parent;	/* Parent of toc entry (DD or LI) */
  tree_t	*target,	/* Link target */
		*temp;		/* Looping var */
  uchar		heading[255],	/* Heading numbers */
		link[255],	/* Actual link */
		baselink[255],	/* Base link (numbered) */
		*existing;	/* Existing link string */
  int		i, level;	/* Header level */
  uchar		*var;		/* Starting value/type for this level */


  while (t != NULL)
  {
    switch (t->markup)
    {
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
          level = t->markup - MARKUP_H1;

	  if ((level - last_level) > 1)
	  {
	   /*
	    * This step necessary to keep page numbers synced up...
	    */

	    level     = last_level + 1;
	    t->markup = (markup_t)(MARKUP_H1 + level);
	  }

          if ((var = htmlGetVariable(t, (uchar *)"VALUE")) != NULL)
            heading_numbers[level] = atoi((char *)var);
          else
            heading_numbers[level] ++;

          if (level == 0)
            TocDocCount ++;

          if ((var = htmlGetVariable(t, (uchar *)"TYPE")) != NULL)
            heading_types[level] = var[0];

          for (i = level + 1; i < 15; i ++)
            heading_numbers[i] = 0;

          heading[0]  = '\0';
	  baselink[0] = '\0';

          for (i = 0; i <= level; i ++)
          {
            uchar	*baseptr = baselink + strlen((char *)baselink);
            uchar	*headptr = heading + strlen((char *)heading);

            if (i == 0)
              snprintf((char *)baseptr, sizeof(baselink) - (size_t)(baseptr - baselink), "%d", TocDocCount);
            else
              snprintf((char *)baseptr, sizeof(baselink) - (size_t)(baseptr - baselink), "%d", heading_numbers[i]);

            strlcpy((char *)headptr, format_number(heading_numbers[i], heading_types[i]), sizeof(heading) - (size_t)(headptr - heading));

            if (i < level)
            {
              strlcat((char *)heading, ".", sizeof(heading));
              strlcat((char *)baselink, "_", sizeof(baselink));
            }
          }

         /*
	  * See if we have an existing <A NAME=...> or <A ID=...> for this
	  * heading...
	  */

          existing = NULL;

          if (t->parent != NULL && t->parent->markup == MARKUP_A)
          {
	    existing = htmlGetVariable(t->parent, (uchar *)"NAME");

	    if (!existing)
              existing = htmlGetVariable(t->parent, (uchar *)"ID");
          }

	  if (existing == NULL &&
              t->child != NULL && t->child->markup == MARKUP_A)
          {
	    existing = htmlGetVariable(t->child, (uchar *)"NAME");

	    if (!existing)
              existing = htmlGetVariable(t->child, (uchar *)"ID");
          }

          if (existing != NULL &&
	      strlen((char *)existing) >= 124)	/* Max size of link name */
	    existing = NULL;

          if (existing != NULL)
	    snprintf((char *)link, sizeof(link), "#%s", existing);
	  else
	    snprintf((char *)link, sizeof(link), "#%s", baselink);

         /*
	  * Number the headings as needed...
	  */

          if (TocNumbers)
	  {
            strlcat((char *)heading, " ", sizeof(heading));

            htmlInsertTree(t, MARKUP_NONE, heading);
	  }

         /*
	  * Add the heading to the table of contents...
	  */

          if (level < TocLevels)
          {
            if (level > last_level)
	    {
	      if (heading_parents[last_level]->last_child && level > 1)
        	heading_parents[level] =
		    htmlAddTree(heading_parents[last_level]->last_child,
                                MARKUP_UL, NULL);
              else
        	heading_parents[level] =
		    htmlAddTree(heading_parents[last_level], MARKUP_UL, NULL);

              DEBUG_printf(("level=%d, last_level=%d, created new UL parent %p\n",
	                    level, last_level, (void *)heading_parents[level]));
	    }

            if (level == 0)
            {
              if (last_level == 0)
              {
                htmlAddTree(heading_parents[level], MARKUP_BR, NULL);
                htmlAddTree(heading_parents[level], MARKUP_BR, NULL);
              }

              parent = htmlAddTree(heading_parents[level], MARKUP_B, NULL);
            }
            else
              parent = htmlAddTree(heading_parents[level], MARKUP_LI, NULL);

            DEBUG_printf(("parent=%p\n", (void *)parent));

            if ((var = htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC")) != NULL)
	      htmlSetVariable(parent, (uchar *)"_HD_OMIT_TOC", var);

            if (TocLinks)
            {
             /*
              * Add a link for the toc...
              */

              parent = htmlAddTree(parent, MARKUP_A, NULL);
              htmlSetVariable(parent, (uchar *)"HREF", link);

             /*
              * Insert a NAME marker if needed and reparent all the
	      * heading children.
              */

              if (existing == NULL)
	      {
	       /*
	        * Add NAME to existing A element, if present.
		*/

                if (t->parent != NULL && t->parent->markup == MARKUP_A)
	          htmlSetVariable(t->parent, (uchar *)"NAME", baselink);
		else if (t->child != NULL && t->child->markup == MARKUP_A)
	          htmlSetVariable(t->child, (uchar *)"NAME", baselink);
		else
		{
        	  target = htmlNewTree(t, MARKUP_A, NULL);

        	  htmlSetVariable(target, (uchar *)"NAME", baselink);
        	  for (temp = t->child; temp != NULL; temp = temp->next)
                    temp->parent = target;

        	  target->child = t->child;
        	  t->child      = target;
	        }
	      }
            }

            add_heading(parent, t->child);
          }

          last_level = level;
          break;

      default :
          if (t->child != NULL)
            parse_tree(t->child);
          break;
    }

    t = t->next;
  }
}
