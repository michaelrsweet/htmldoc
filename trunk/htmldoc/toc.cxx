//
// "$Id: toc.cxx,v 1.14 2003/01/02 03:10:12 mike Exp $"
//
//   Table-of-contents methods for HTMLDOC, a HTML document processing program.
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
//   hdTree::build_toc() - Build a table-of-contents...
//

//
// Include necessary headers...
//

#include "htmldoc.h"
#include "hdstring.h"


//
// 'hdTree::build_toc()' - Build a table-of-contents...
//

hdTree *				// O - Table of contents tree
hdTree::build_toc(hdStyleSheet *css,	// I - Style sheet
                  int          levels,	// I - Number of levels
		  bool         numbered)// I - Number the headings?
{
  int		i;			// Looping var
  int		chapter;		// Chapter number
  int		numbers[6];		// Current heading numbers
  char		formats[6];		// Current heading formats
  int		toclevel,		// Current TOC level
		level,			// Current heading level
		newlevel;		// New heading level
  hdTree	*t,			// Current node
		*tnext,			// Next node
		*tlink,			// Pointer to link
		*ttemp;			// Temporary node pointer
  hdTree	*toc,			// Table of contents
		*tocptr,		// Pointer into TOC
		*tocnode,		// Node in TOC
		*tocnumber,		// Number node in TOC
		*toclink;		// Link node in TOC
  char		s[1024],		// Text for heading number
		*sptr;			// Pointer into heading number
  const char	*val;			// Attribute value


  // Initialize the numbers and formats...
  memset(numbers, 0, sizeof(numbers));
  memset(formats, '1', sizeof(formats));

  level    = 0;
  toclevel = 0;
  chapter  = 0;

  // Create a root node for the TOC...
  toc = new hdTree(HD_ELEMENT_BODY);
  toc->set_attr("class", "TOC");
  toc->style = css->find_style(toc);

  // Scan the document tree for headings...
  for (tocptr = toc, t = this; t; t = tnext)
  {
    // Point to the next node in the tree...
    tnext = t->real_next();

    // See if we have a heading...
    if (t->element >= HD_ELEMENT_H1 && t->element <= HD_ELEMENT_H6)
    {
      // Yes, handle it...
      tnext    = t->next;
      newlevel = t->element - HD_ELEMENT_H1;

      // Bump the chapter number as needed...
      if (newlevel == 0)
        chapter ++;

      // See if we have a new format or number value...
      while (level < newlevel)
      {
        level ++;
        numbers[level] = 0;
      }

      level = newlevel;

      if ((val = t->get_attr("VALUE")) != NULL)
        numbers[newlevel] = atoi(val);
      else
        numbers[newlevel] ++;

      if ((val = t->get_attr("TYPE")) != NULL)
        formats[newlevel] = *val;

      // Do we need heading numbers?
      if (numbers)
      {
        // Yes, format the number...
        s[0]             = '\0';
	s[sizeof(s) - 1] = '\0';

        for (i = 0, sptr = s; i <= newlevel; i ++, sptr += strlen(sptr))
	{
	  hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                         formats[i], numbers[i]);

          if (i < newlevel)
	    strncat(sptr, ".", sizeof(s) - (sptr - s) - 1);
        }

        tocnumber = new hdTree(HD_ELEMENT_NONE, s);
	tocnumber->insert(t);
	tocnumber->style = t->style;
	tocnumber->compute_size(css);

	if (tocnumber->next)
	  tocnumber->next->whitespace = 1;
      }

      // Do we need to add a link target?
      if ((tlink = t->find(HD_ELEMENT_A)) == NULL)
      {
        // No link node; insert one...
	tlink = new hdTree(HD_ELEMENT_A);
	tlink->child      = t->child;
	tlink->last_child = t->last_child;

	for (ttemp = t->child; ttemp != NULL; ttemp = ttemp->next)
	  ttemp->parent = tlink;

        t->child      = tlink;
	t->last_child = tlink;
	tlink->parent = t;
      }

      if (tlink->get_attr("name") == NULL)
      {
        // Add a named link...
	strcpy(s, "HD_");
	s[sizeof(s) - 1] = '\0';

        for (i = 0, sptr = s + 3; i <= level; i ++, sptr += strlen(sptr))
	{
	  hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                         formats[i], numbers[i]);

          if (i < level)
	    strncat(sptr, "_", sizeof(s) - (sptr - s) - 1);
        }

	tlink->set_attr("name", s);
      }

      if (level < levels)
      {
	// If the new level is not the same as the old level, (un)indent
	// as needed...
	while (level > toclevel)
	{
          // Indent lower...
          toclevel ++;

	  tocnode = new hdTree(HD_ELEMENT_UL, NULL, tocptr);
	  tocnode->set_attr("class", "TOC");
	  tocnode->style = css->find_style(tocnode);
	  tocptr = tocnode;
	}

	while (level < toclevel)
	{
          // Unindent higher...
          toclevel --;

	  tocptr = tocptr->parent;
	}

	// Then add a new node for the heading...
	tocnode = new hdTree(toclevel == 0 ? HD_ELEMENT_P : HD_ELEMENT_LI,
	                     NULL, tocptr);
	tocnode->set_attr("class", "TOC");
	tocnode->style = css->find_style(tocnode);

	// And a node for the link...
	snprintf(s, sizeof(s), "#%s", tlink->get_attr("name"));

	toclink = new hdTree(HD_ELEMENT_A, "", tocnode);
	toclink->set_attr("href", s);
	toclink->set_attr("class", "TOC");
	toclink->style = css->find_style(toclink);

	// Copy the text to the TOC...
	t->copy_text(css, toclink);

	// Finally, add a pseudo-attribute for the chapter number...
	snprintf(s, sizeof(s), "%d", chapter);
	tlink->set_attr("_HD_CHAPTER", s);
	toclink->set_attr("_HD_CHAPTER", s);
      }
    }
  }

  return (toc);
}


//
// End of "$Id: toc.cxx,v 1.14 2003/01/02 03:10:12 mike Exp $".
//
