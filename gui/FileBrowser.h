//
// "$Id: FileBrowser.h,v 1.10.2.3 2001/04/18 23:24:09 mike Exp $"
//
//   FileBrowser definitions.
//
//   Copyright 1997-2001 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING" which should have been included with this file.  If this
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
  const char	*directory_;
  uchar		iconsize_;
  const char	*pattern_;

  int		full_height() const;
  int		item_height(void *) const;
  int		item_width(void *) const;
  void		item_draw(void *, int, int, int, int) const;
  int		incr_height() const { return (item_height(0)); }

public:
  FileBrowser(int, int, int, int, const char * = 0);

  uchar		iconsize() const { return (iconsize_); };
  void		iconsize(uchar s) { iconsize_ = s; redraw(); };

  void		filter(const char *pattern);
  const char	*filter() const { return (pattern_); };

  int		load(const char *directory);

  uchar		textsize() const { return (Fl_Browser::textsize()); };
  void		textsize(uchar s) { Fl_Browser::textsize(s); iconsize_ = 3 * s / 2; };

};

#endif // !_GUI_FILEBROWSER_H_

//
// End of "$Id: FileBrowser.h,v 1.10.2.3 2001/04/18 23:24:09 mike Exp $".
//
