//
// "$Id: FileBrowser.cxx,v 1.5 1999/04/27 12:43:35 mike Exp $"
//
//   FileBrowser routines for the Common UNIX Printing System (CUPS).
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

//
// Include necessary header files...
//

#include "FileBrowser.h"
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(__EMX__)
#  include <windows.h>
#  include <direct.h>
#endif /* WIN32 || __EMX__ */


// From "Fl_Browser.cxx"...
#define SELECTED 1
#define NOTDISPLAYED 2

struct FL_BLINE			// data is in a linked list of these
{
  FL_BLINE	*prev;		// Previous item in list
  FL_BLINE	*next;		// Next item in list
  void		*data;		// Pointer to data (function)
  short		length;		// sizeof(txt)-1, may be longer than string
  char		flags;		// selected, displayed
  char		txt[1];		// start of allocated array
};


int
FileBrowser::item_width(void *p) const
{
  fl_font(textfont(), textsize());
  return ((int)(fl_width(((FL_BLINE *)p)->txt) + 2 * textsize() + 4.5));
}


void
FileBrowser::item_draw(void *p, int x, int y, int w, int h) const
{
  FL_BLINE	*line;			// Pointer to line


  line = (FL_BLINE *)p;

  fl_font(textfont(), textsize());
  if (line->flags & SELECTED)
    fl_color(contrast(textcolor(), selection_color()));
  else
    fl_color(textcolor());

  fl_draw(line->txt, x + 2 * textsize() + 2, y, w - 2 * textsize() - 2, h,
          FL_ALIGN_LEFT);

  if (line->data)
  {
    fl_push_matrix();
      fl_translate((float)x, (float)y + 0.5 * (h + textsize() * 1.5));
      fl_scale(1.5 * textsize(), -1.5 * textsize());
      FileIcon::draw((line->flags & SELECTED) ? FL_YELLOW : FL_LIGHT2,
                     (short *)line->data);
    fl_pop_matrix();
  }
}


FileBrowser::FileBrowser(int x, int y, int w, int h, const char *l) :
Fl_Browser(x, y, w, h, l)
{
  pattern_   = "*";
  directory_ = "";
}


int
FileBrowser::load(const char *directory)
{
  int		i;		// Looping var
  int		num_files;	// Number of files in directory
  char		filename[4096];	// Current file
  FileIcon	*icon;		// Icon to use
  short		*data;		// Icon data


  clear();
  directory_ = directory;

  if (directory_[0] == '\0')
  {
    //
    // No directory specified; for UNIX list all mount points.  For DOS
    // list all valid drive letters...
    //

    num_files = 0;

    if ((icon = FileIcon::find("any", FileIcon::DIRECTORY)) != NULL)
      data = icon->value();
    else
      data = NULL;

#if defined(WIN32) || defined(__EMX__)
    DWORD	drives;		// Drive available bits


    drives = GetLogicalDrives();
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:", i);

	if (i < 'C')
	  add(filename, data);
	else
	  add(filename, data);

	num_files ++;
      }
#else
    FILE	*mtab;		// /etc/mtab or /etc/mnttab file
    char	line[1024];	// Input line


    //
    // Open the file that contains a list of mounted filesystems...
    //
#  if defined(hpux) || defined(__sun)
    mtab = fopen("/etc/mnttab", "r");	// Fairly standard
#  elif defined(__sgi) || defined(linux)
    mtab = fopen("/etc/mtab", "r");	// More standard
#  else
    mtab = fopen("/etc/fstab", "r");	// Otherwise fallback to full list
    if (mtab == NULL)
      mtab = fopen("/etc/vfstab", "r");
#  endif

    if (mtab != NULL)
    {
      while (fgets(line, sizeof(line), mtab) != NULL)
      {
        if (line[0] == '#' || line[0] == '\n')
	  continue;
        if (sscanf(line, "%*s%s", filename) != 1)
	  continue;

        add(filename, data);
	num_files ++;
      }

      fclose(mtab);
    }
#endif // WIN32 || __EMX__
  }
  else
  {
    dirent	**files;	// Files in in directory


    //
    // Build the file list...
    //

#if defined(WIN32) || defined(__EMX__)
    strcpy(filename, directory_);
    i = strlen(filename) - 1;

    if (i == 2 && filename[1] == ':' &&
        (filename[2] == '/' || filename[2] == '\\'))
      filename[2] = '/';
    else if (filename[i] != '/' && filename[i] != '\\')
      strcat(filename, "/");

    num_files = filename_list(filename, &files);
#else
    num_files = filename_list(directory_, &files);
#endif /* WIN32 || __EMX__ */

    for (i = 0; i < num_files; i ++)
    {
      if (strcmp(files[i]->d_name, ".") == 0 ||
          strcmp(files[i]->d_name, "..") == 0)
	continue;

      sprintf(filename, "%s/%s", directory_, files[i]->d_name);

      if (filename_isdir(filename) ||
          filename_match(files[i]->d_name, pattern_))
      {
	if ((icon = FileIcon::find(filename)) != NULL)
          add(files[i]->d_name, icon->value());
	else
          add(files[i]->d_name);
      }

      free(files[i]);
    }

    free(files);
  }

  return (num_files);
}


void
FileBrowser::filter(const char *pattern)
{
  if (pattern)
    pattern_ = pattern;
  else
    pattern_ = "*";

  load(directory_);
}


//
// End of "$Id: FileBrowser.cxx,v 1.5 1999/04/27 12:43:35 mike Exp $".
//
