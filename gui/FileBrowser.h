//
// "$Id: FileBrowser.h,v 1.6 1999/04/29 01:34:36 mike Exp $"
//
//   FileBrowser definitions for the Common UNIX Printing System (CUPS).
//
//   Copyright 1997-1999 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "LICENSE.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: CUPS Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9603
//       EMail: cups-info@cups.org
//         WWW: http://www.cups.org
//

//
// Include necessary header files...
//

#ifndef _GUI_FILEBROWSER_H_
#  define _GUI_FILEBROWSER_H_

#  include <FL/Fl_Browser.H>
#  include "FileIcon.h"


//
// FileBrowser class...
//

class FileBrowser : public Fl_Browser
{
  int		item_height(void *) const { return ((int)(textsize() * 1.5 + 2)); };
  int		item_width(void *) const;
  void		item_draw(void *, int, int, int, int) const;
  int		full_height() const { return (size() * item_height(0)); }
  int		incr_height() const { return (item_height(0)); }
  const char	*pattern_;
  const char	*directory_;

public:
  FileBrowser(int, int, int, int, const char * = 0);

  int		load(const char *directory);
  void		filter(const char *pattern);
  const char	*filter() const { return (pattern_); };
};

#endif // !_GUI_FILEBROWSER_H_

//
// End of "$Id: FileBrowser.h,v 1.6 1999/04/29 01:34:36 mike Exp $".
//
