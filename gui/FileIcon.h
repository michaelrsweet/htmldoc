//
// "$Id: FileIcon.h,v 1.2 1999/04/21 23:51:53 mike Exp $"
//
//   FileIcon definitions for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the Aladdin Free Public License
//   which is described in the file "LICENSE.txt".
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
  short		*_data;		// Icon data

  public:

  enum				// File types
  {
    ANY,			// Any kind of file
    PLAIN,			// Only plain files
    PIPE,			// Only named pipes
    DEVICE,			// Only character and block devices
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
  }

  FileIcon(const char *p, int t, int nd = 0, short *d = 0);
  FileIcon(const char *p, int t, const char *fti);
  ~FileIcon();

  short		*add(short d);
  short		*add_color(short c);
  short		*add_vertex(short x, short y);
  short		*add_vertex(float x, float y)
		{ add_vertex((int)(x * 32767.0), (int)(y * 32767.0)); }
  void		draw() { draw(_data); }
  const char	*pattern() { return (_pattern); }
  int		size() { return (_num_data); }
  int		type() { return (_type); }
  short		*value() { return (_data) }

  static FileIcon *find(const char *filename);
  static FileIcon *first() { return (_first); }
  static void	draw(short *d);
};

#endif // !_GUI_FILEICON_H_

//
// End of "$Id: FileIcon.h,v 1.2 1999/04/21 23:51:53 mike Exp $".
//
