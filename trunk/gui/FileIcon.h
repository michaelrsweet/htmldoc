//
// "$Id: FileIcon.h,v 1.3 1999/04/27 12:43:37 mike Exp $"
//
//   FileIcon definitions for the Common UNIX Printing System (CUPS).
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

#ifndef _GUI_FILEICON_H_
#  define _GUI_FILEICON_H_

//
// FileIcon class...
//

class FileIcon			//// Icon data
{
  static FileIcon *_first;	// Pointer to first icon/filetype
  FileIcon	*_next;		// Pointer to next icon/filetype
  const char	*_pattern;	// Pattern string
  int		_type;		// Match only if directory or file?
  int		_num_data;	// Number of data elements
  int		_alloc_data;	// Number of allocated elements
  short		*_data;		// Icon data

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
  void		clear() { _num_data = 0; }
  void		draw(Fl_Color ic) { draw(ic, _data); }
  void		load(const char *fti);
  const char	*pattern() { return (_pattern); }
  int		size() { return (_num_data); }
  int		type() { return (_type); }
  short		*value() { return (_data); }

  static FileIcon *find(const char *filename, int filetype  = ANY);
  static FileIcon *first() { return (_first); }
  static void	draw(Fl_Color ic, short *d);
};

#endif // !_GUI_FILEICON_H_

//
// End of "$Id: FileIcon.h,v 1.3 1999/04/27 12:43:37 mike Exp $".
//
