//
// "$Id: FileBrowser.h,v 1.4 1999/04/21 01:22:21 mike Exp $"
//
//   FileBrowser definitions for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the Aladdin Free Public License
//   which is described in the file "LICENSE.txt".
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
  int		item_height(void *) const { return (textsize() * 1.5 + 2); };
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
// End of "$Id: FileBrowser.h,v 1.4 1999/04/21 01:22:21 mike Exp $".
//
