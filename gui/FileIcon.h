//
// "$Id: FileIcon.h,v 1.5 1999/10/13 15:34:48 mike Exp $"
//
//   FileIcon definitions for the Common UNIX Printing System (CUPS).
//
//   Copyright 1997-1999 by Easy Software Products.
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

#ifndef _GUI_FILEICON_H_
#  define _GUI_FILEICON_H_

//
// FileIcon class...
//

class FileIcon			//// Icon data
{
  static FileIcon *first_;	// Pointer to first icon/filetype
  FileIcon	*next_;		// Pointer to next icon/filetype
  const char	*pattern_;	// Pattern string
  int		type_;		// Match only if directory or file?
  int		num_data_;	// Number of data elements
  int		alloc_data_;	// Number of allocated elements
  short		*data_;		// Icon data

  public:

  enum				// File types
  {
    ANY,			// Any kind of file
    PLAIN,			// Only plain files
    FIFO,			// Only named pipes
    DEVICE,			// Only character and block devices
    LINK,			// Only symbolic links
    DIRECTORY			// Only directories
  };

  enum				// Data opcodes
  {
    END,			// End of primitive/icon
    COLOR,			// Followed by color index
    LINE,			// Start of line
    CLOSEDLINE,			// Start of closed line
    POLYGON,			// Start of polygon
    OUTLINEPOLYGON,		// Followed by outline color
    VERTEX			// Followed by scaled X,Y
  };

  FileIcon(const char *p, int t, int nd = 0, short *d = 0);
  ~FileIcon();

  short		*add(short d);
  short		*add_color(short c)
		{ short *d = add(COLOR); add(c); return (d); }
  short		*add_vertex(short x, short y)
		{ short *d = add(VERTEX); add(x); add(y); return (d); }
  short		*add_vertex(float x, float y)
		{ short *d = add(VERTEX); add((int)(x * 10000.0));
		  add((int)(y * 10000.0)); return (d); }
  void		clear() { num_data_ = 0; }
  void		draw(int x, int y, int w, int h, Fl_Color ic);
  void		load(const char *fti);
  const char	*pattern() { return (pattern_); }
  int		size() { return (num_data_); }
  int		type() { return (type_); }
  short		*value() { return (data_); }

  static FileIcon *find(const char *filename, int filetype  = ANY);
  static FileIcon *first() { return (first_); }
  static void	load_system_icons(void);
};

#endif // !_GUI_FILEICON_H_

//
// End of "$Id: FileIcon.h,v 1.5 1999/10/13 15:34:48 mike Exp $".
//
