//
// "$Id: FileBrowser.h,v 1.2 1999/02/02 21:55:06 mike Exp $"
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

#ifndef _FILE_BROWSER_H_
#  define _FILE_BROWSER_H_

#  include <FL/Fl_Browser.H>


//
// FileBrowser class and support structures...
//

struct FBIcon			//// Icon data
{
  FBIcon	*next;			// Pointer to next icon/filetype
  const char	*pattern;		// Pattern string 
  void		(*drawfunc)(Fl_Color);	// Draw function
};

class FileBrowser : public Fl_Browser
{
  int		item_height(void *) const { return (textsize() * 1.5 + 2); };
  int		item_width(void *) const;
  void		item_draw(void *, int, int, int, int) const;
  int		full_height() const { return (size() * item_height(0)); }
  int		incr_height() const { return (item_height(0)); }
  const char	*pattern_;
  const char	*directory_;
  static FBIcon	*icons_;

public:
  FileBrowser(int, int, int, int, const char * = 0);

  int		load(const char *directory);
  void		icon(const char *pattern, void (*drawfunc)(Fl_Color));
  void		filter(const char *pattern);
  const char	*filter() const { return (pattern_); };
  static void	draw_drive(Fl_Color c);
  static void	draw_folder(Fl_Color c);
  static void	draw_file(Fl_Color c);
};

#endif // !_FILE_BROWSER_H_

//
// End of "$Id: FileBrowser.h,v 1.2 1999/02/02 21:55:06 mike Exp $".
//
