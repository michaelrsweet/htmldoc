//
// "$Id: list.cxx,v 1.5.2.2 2004/03/30 03:49:15 mike Exp $"
//
//   List generation methods for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2004 by Easy Software Products.
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
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdTree::build_list() - Build a list-of-whatever...
//

//
// Include necessary headers...
//

#include "htmldoc.h"
#include "hdstring.h"


//
// 'hdTree::build_list()' - Build a list-of-whatever...
//

hdTree *				// O - Table of contents tree
hdTree::build_list(hdStyleSheet *css,	// I - Style sheet
                   const char   *class_,// I - Name of caption class
		   const char	*prefix)// I - Prefix for numbers entries
{
  int		i;			// Looping var
  int		chapter,		// Chapter number
		numbers[2];		// List numbers
  char		formats[2];		// Format types
  hdTree	*t,			// Current node
		*tnext,			// Next node
		*tlink;			// Pointer to link
  hdTree	*list,			// List
		*listptr,		// Pointer into list
		*listnode,		// Node in list
		*listnumber,		// Number node in list
		*listlink;		// Link node in list
  char		s[1024],		// Text for heading number
		*sptr;			// Pointer into heading number
  const char	*val;			// Attribute value


  // Initialize the chapter and list numbers...
  memset(numbers, 0, sizeof(numbers));
  memset(formats, '1', sizeof(formats));

  chapter = 0;

  // Create a root node for the list...
  list = new hdTree(HD_ELEMENT_BODY);
  list->set_attr("class", class_);
  list->style = css->find_style(list);

  // Scan the document tree for headings...
  for (listptr = list, t = this; t; t = tnext)
  {
    // Point to the next node in the tree...
    tnext = t->real_next();

    // See if we have a chapter heading...
    if (t->element == HD_ELEMENT_H1)
    {
      // Yes...
      tnext = t->next;
      chapter ++;

      // See if we have a new format or number value...
      if ((val = t->get_attr("VALUE")) != NULL)
        numbers[0] = atoi(val);
      else
        numbers[0] ++;

      if ((val = t->get_attr("TYPE")) != NULL)
        formats[0] = *val;

      numbers[1] = 0;
    }
    else if (t->element == HD_ELEMENT_CAPTION)
    {
      // Have a caption; see if we have the right class name...
      tnext = t->next;

      if ((val = t->get_attr("class")) == NULL ||
          strcasecmp(class_, val) != 0)
	continue;

      // See if we have a new format or number value...
      if ((val = t->get_attr("VALUE")) != NULL)
        numbers[1] = atoi(val);
      else
        numbers[1] ++;

      if ((val = t->get_attr("TYPE")) != NULL)
        formats[1] = *val;

      // Do we need list numbers?
      if (prefix)
      {
        // Yes, format the number...
	snprintf(s, sizeof(s), "%s ", prefix);
	s[sizeof(s) - 1] = '\0';

        for (i = 0, sptr = s; i < 2; i ++)
	{
	  sptr += strlen(sptr);

	  hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                         formats[i], numbers[i]);

          if (!i)
	    strncat(sptr, ".", sizeof(s) - (sptr - s) - 1);
        }

        listnumber = new hdTree(HD_ELEMENT_NONE, s);
	listnumber->insert(t);
	listnumber->style = t->style;
	listnumber->compute_size(css);

	if (listnumber->next)
	  listnumber->next->whitespace = 1;
      }

      // Add a link target?
      tlink = new hdTree(HD_ELEMENT_A);
      tlink->insert(t->parent);

      // Add a named link...
      snprintf(s, sizeof(s), "HD_%s_", class_);
      s[sizeof(s) - 1] = '\0';

      for (i = 0, sptr = s; i < 2; i ++)
      {
	sptr += strlen(sptr);

	hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                       formats[i], numbers[i]);

        if (!i)
	  strncat(sptr, "_", sizeof(s) - (sptr - s) - 1);
      }

      tlink->set_attr("name", s);

      // Then add a new node for the heading...
      listnode = new hdTree(HD_ELEMENT_P, NULL, listptr);
      listnode->set_attr("class", class_);
      listnode->style = css->find_style(listnode);

      // And a node for the link...
      snprintf(s, sizeof(s), "#%s", tlink->get_attr("name"));

      listlink = new hdTree(HD_ELEMENT_A, "", listnode);
      listlink->set_attr("href", s);
      listlink->set_attr("class", class_);
      listlink->style = css->find_style(listlink);

      // Copy the text to the TOC...
      t->copy_text(css, listlink);

      // Finally, add a pseudo-attribute for the chapter number...
      snprintf(s, sizeof(s), "%d", chapter);
      tlink->set_attr("_HD_CHAPTER", s);
      listlink->set_attr("_HD_CHAPTER", s);
    }
  }

  return (list);
}


//
// End of "$Id: list.cxx,v 1.5.2.2 2004/03/30 03:49:15 mike Exp $".
//
