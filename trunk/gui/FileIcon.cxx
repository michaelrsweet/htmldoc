//
// "$Id: FileIcon.cxx,v 1.4 1999/04/28 15:53:05 mike Exp $"
//
//   FileIcon routines for the Common UNIX Printing System (CUPS).
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
// Contents:
//
//   FileIcon::FileIcon()  - Create a new file icon.
//   FileIcon::~FileIcon() - Remove a file icon.
//   FileIcon::add()       - Add data to an icon.
//   FileIcon::find()      - Find an icon based upon a given file.
//   FileIcon::draw()      - Draw an icon.
//   FileIcon::load()      - Load an SGI-format FTI file...
//

//
// Include necessary header files...
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#include "FileIcon.h"


//
// Define missing POSIX/XPG4 macros as needed...
//

#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */


//
// Icon cache...
//

FileIcon	*FileIcon::_first = (FileIcon *)0;


//
// 'FileIcon::FileIcon()' - Create a new file icon.
//

FileIcon::FileIcon(const char *p,	/* I - Filename pattern */
                   int        t,	/* I - File type */
		   int        nd,	/* I - Number of data values */
		   short      *d)	/* I - Data values */
{
  // Initialize the pattern and type...
  _pattern = p;
  _type    = t;

  // Copy icon data as needed...
  if (nd)
  {
    _num_data   = nd;
    _alloc_data = nd;
    _data       = (short *)calloc(sizeof(short), nd);
    memcpy(_data, d, nd * sizeof(short));
  }
  else
  {
    _num_data   = 0;
    _alloc_data = 0;
  }

  // And add the icon to the list of icons...
  _next  = _first;
  _first = this;
}


//
// 'FileIcon::~FileIcon()' - Remove a file icon.
//

FileIcon::~FileIcon()
{
  FileIcon	*current,	// Current icon in list
		*prev;		// Previous icon in list


  // Find the icon in the list...
  for (current = _first, prev = (FileIcon *)0;
       current != this && current != (FileIcon *)0;
       prev = current, current = current->_next);

  // Remove the icon from the list as needed...
  if (current)
  {
    if (prev)
      prev->_next = current->_next;
    else
      _first = current->_next;
  }

  // Free any memory used...
  if (_alloc_data)
    free(_data);
}


//
// 'FileIcon::add()' - Add data to an icon.
//

short *			// O - Pointer to new data value
FileIcon::add(short d)	// I - Data to add
{
  short	*dptr;		// Pointer to new data value


  // Allocate/reallocate memory as needed
  if ((_num_data + 1) >= _alloc_data)
  {
    _alloc_data += 128;

    if (_alloc_data == 128)
      dptr = (short *)malloc(sizeof(short) * _alloc_data);
    else
      dptr = (short *)realloc(_data, sizeof(short) * _alloc_data);

    if (dptr == NULL)
      return (NULL);

    _data = dptr;
  }

  // Store the new data value and return
  _data[_num_data++] = d;
  _data[_num_data]   = END;

  return (_data + _num_data - 1);
}


//
// 'FileIcon::find()' - Find an icon based upon a given file.
//

FileIcon *				// O - Matching file icon or NULL
FileIcon::find(const char *filename,	// I - Name of file */
               int        filetype)	// I - Enumerated file type
{
  FileIcon	*current;		// Current file in list
  struct stat	fileinfo;		// Information on file


  // Get file information if needed...
  if (filetype == ANY)
    if (!stat(filename, &fileinfo))
    {
      if (S_ISDIR(fileinfo.st_mode))
        filetype = DIRECTORY;
      else if (S_ISFIFO(fileinfo.st_mode))
        filetype = FIFO;
      else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
        filetype = DEVICE;
      else if (S_ISLNK(fileinfo.st_mode))
        filetype = LINK;
      else
        filetype = PLAIN;
    }

  // Loop through the available file types and return any match that
  // is found...
  for (current = _first; current != (FileIcon *)0; current = current->_next)
    if ((current->_type == filetype || current->_type == ANY) &&
        filename_match(filename, current->_pattern))
      break;

  // Return the match (if any)...
  return (current);
}


//
// 'FileIcon::draw()' - Draw an icon.
//

void
FileIcon::draw(Fl_Color ic,	// I - Icon color...
               short *d)	// I - Data to draw...
{
  Fl_Color	c;		// Current color
  short		*prim;		// Pointer to start of primitive...


  // Don't try to draw a NULL array!
  if (d == NULL)
    return;

  // Loop through the array until we see an unmatched END...
  prim = NULL;
  c    = ic;
  fl_color(c);

  while (*d != END || prim)
    switch (*d)
    {
      case END :
          switch (*prim)
	  {
	    case LINE :
		fl_end_line();
		break;

	    case CLOSEDLINE :
		fl_end_loop();
		break;

	    case POLYGON :
		fl_end_polygon();
		break;

	    case OUTLINEPOLYGON :
		fl_end_polygon();

                if (prim[1] == 256)
		  fl_color(ic);
		else
		  fl_color((Fl_Color)prim[1]);
		fl_begin_loop();

		prim += 2;
		while (*prim == VERTEX)
		{
		  fl_vertex(prim[1] * 0.0001, prim[2] * 0.0001);
		  prim += 3;
		}

        	fl_end_loop();
		fl_color(c);
		break;
	  }

          prim = NULL;
	  d ++;
	  break;

      case COLOR :
          if (d[1] == 256)
	    c = ic;
	  else
	    c = (Fl_Color)d[1];
          fl_color(c);
	  d += 2;
	  break;

      case LINE :
          prim = d;
	  d ++;
	  fl_begin_line();
	  break;

      case CLOSEDLINE :
          prim = d;
	  d ++;
	  fl_begin_loop();
	  break;

      case POLYGON :
          prim = d;
	  d ++;
	  fl_begin_polygon();
	  break;

      case OUTLINEPOLYGON :
          prim = d;
	  d += 2;
	  fl_begin_polygon();
	  break;

      case VERTEX :
          if (prim)
	    fl_vertex(d[1] * 0.0001, d[2] * 0.0001);
	  d += 3;
	  break;
    }

  // If we still have an open primitive, close it...
  if (prim)
    switch (*prim)
    {
      case LINE :
	  fl_end_line();
	  break;

      case CLOSEDLINE :
	  fl_end_loop();
	  break;

      case POLYGON :
	  fl_end_polygon();
	  break;

      case OUTLINEPOLYGON :
	  fl_end_polygon();

          if (prim[1] == 256)
	    fl_color(ic);
	  else
	    fl_color((Fl_Color)prim[1]);
	  fl_begin_loop();

	  prim += 2;
	  while (*prim == VERTEX)
	  {
	    fl_vertex(prim[1] * 0.0001, prim[2] * 0.0001);
	    prim += 3;
	  }

          fl_end_loop();
	  fl_color(c);
	  break;
    }
}


//
// 'FileIcon::load()' - Load an SGI-format FTI file...
//

void
FileIcon::load(const char *fti)	// File to read from
{
  FILE	*fp;			// File pointer
  int	ch;			// Current character
  char	command[255],		// Command string ("vertex", etc.)
	params[255],		// Parameter string ("10.0,20.0", etc.)
	*ptr;			// Pointer into strings
  short	*outline;		// Outline polygon


  // Try to open the file...
  if ((fp = fopen(fti, "r")) == NULL)
    return;

  // Read the entire file, adding data as needed...
  outline = NULL;

  while ((ch = getc(fp)) != EOF)
  {
    // Skip whitespace
    if (isspace(ch))
      continue;

    // Skip comments starting with "#"...
    if (ch == '#')
    {
      while ((ch = getc(fp)) != EOF)
        if (ch == '\n')
	  break;

      if (ch == EOF)
        break;
      else
        continue;
    }

    // OK, this character better be a letter...
    if (!isalpha(ch))
      break;

    // Scan the command name...
    ptr    = command;
    *ptr++ = ch;

    while ((ch = getc(fp)) != EOF)
    {
      if (ch == '(')
        break;
      else if ((ptr - command) < (sizeof(command) - 1))
        *ptr++ = ch;
    }

    *ptr++ = '\0';

    // Make sure we stopped on a parenthesis...
    if (ch != '(')
      break;

    // Scan the parameters...
    ptr = params;

    while ((ch = getc(fp)) != EOF)
    {
      if (ch == ')')
        break;
      else if ((ptr - params) < (sizeof(params) - 1))
        *ptr++ = ch;
    }

    *ptr++ = '\0';

    // Make sure we stopped on a parenthesis...
    if (ch != ')')
      break;

    // Make sure the next character is a semicolon...
    if ((ch = getc(fp)) != ';')
      break;

    // Now process the command...
    if (strcmp(command, "color") == 0)
    {
      // Set the color; for negative colors blend the two primaries to
      // produce a composite color.  Also, the following symbolic color
      // names are understood:
      //
      //     name           FLTK color
      //     -------------  ----------
      //     iconcolor      256; mapped to the icon color in FileIcon::draw()
      //     shadowcolor    FL_DARK3
      //     outlinecolor   FL_BLACK
      if (strcmp(params, "iconcolor") == 0)
        add_color(256);
      else if (strcmp(params, "shadowcolor") == 0)
        add_color(FL_DARK3);
      else if (strcmp(params, "outlinecolor") == 0)
        add_color(FL_BLACK);
      else
      {
        short c = atoi(params);	// Color value


        if (c < 0)
	{
	  // Composite color; compute average...
	  c = -c;
	  add_color(fl_color_average((Fl_Color)(c >> 4), (Fl_Color)(c & 15), 0.5));
	}
	else
	  add_color(c);
      }
    }
    else if (strcmp(command, "bgnline") == 0)
      add(LINE);
    else if (strcmp(command, "bgnclosedline") == 0)
      add(CLOSEDLINE);
    else if (strcmp(command, "bgnpolygon") == 0)
      add(POLYGON);
    else if (strcmp(command, "bgnoutlinepolygon") == 0)
    {
      add(OUTLINEPOLYGON);
      outline = add(0);
    }
    else if (strcmp(command, "endoutlinepolygon") == 0 &&
             outline != NULL)
    {
      // Set the outline color; see above for valid values...
      if (strcmp(params, "iconcolor") == 0)
        *outline = 256;
      else if (strcmp(params, "shadowcolor") == 0)
        *outline = FL_DARK3;
      else if (strcmp(params, "outlinecolor") == 0)
        *outline = FL_BLACK;
      else
      {
        short c = atoi(params);	// Color value


        if (c < 0)
	{
	  // Composite color; compute average...
	  c = -c;
	  *outline = fl_color_average((Fl_Color)(c >> 4), (Fl_Color)(c & 15), 0.5);
	}
	else
	  *outline = c;
      }

      outline = NULL;
      add(END);
    }
    else if (strncmp(command, "end", 3) == 0)
      add(END);
    else if (strcmp(command, "vertex") == 0)
    {
      float x, y;		// Coordinates of vertex


      if (sscanf(params, "%f,%f", &x, &y) != 2)
        break;

      add_vertex((short)(x * 100.0 + 0.5), (short)(y * 100.0 + 0.5));
    }
    else
      break;
  }

  // Close the file and return...
  fclose(fp);

#ifdef DEBUG
  printf("Icon File \"%s\":\n", fti);
  for (int i = 0; i < _num_data; i ++)
    printf("    %d,\n", _data[i]);
#endif /* DEBUG */
}


//
// End of "$Id: FileIcon.cxx,v 1.4 1999/04/28 15:53:05 mike Exp $".
//
