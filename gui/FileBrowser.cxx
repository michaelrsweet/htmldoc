//
// "$Id: FileBrowser.cxx,v 1.3 1999/02/05 02:15:59 mike Exp $"
//
//   FileBrowser routines for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the Aladdin Free Public License
//   which is described in the file "LICENSE.txt".
//
// Contents:
//
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


//
// Icon cache...
//

FBIcon		*FileBrowser::icons_ = 0;


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
  void		(*drawfunc)(Fl_Color);	// Pointer to draw function


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
    drawfunc = (void (*)(Fl_Color))line->data;
    fl_push_matrix();
      fl_translate((float)x, (float)y + 0.5 * (h + textsize() * 1.5));
      fl_scale(1.5 * textsize() / 100.0, -1.5 * textsize() / 100.0);
      (*drawfunc)((line->flags & SELECTED) ? FL_YELLOW : FL_LIGHT2);
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
  int	i;		// Looping var
  int	num_files;	// Number of files in directory
  char	filename[4096];	// Current file


  clear();
  directory_ = directory;

  if (directory_[0] == '\0')
  {
    //
    // No directory specified; for UNIX list all mount points.  For DOS
    // list all valid drive letters...
    //

    num_files = 0;

#if defined(WIN32) || defined(__EMX__)
    DWORD	drives;		// Drive available bits


    drives = GetLogicalDrives();
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:/", i);
	add(filename, (void *)draw_drive);
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

        add(filename, (void *)draw_drive);
	num_files ++;
      }

      fclose(mtab);
    }
#endif // WIN32 || __EMX__
  }
  else
  {
    dirent	**files;	// Files in in directory
    FBIcon	*ic;		// Icon pointer


    //
    // Build the file list...
    //

    sprintf(filename, "%s/", directory);
    num_files  = filename_list(filename, &files);

    for (i = 0; i < num_files; i ++)
    {
      if (strcmp(files[i]->d_name, ".") == 0 ||
          strcmp(files[i]->d_name, "..") == 0)
	continue;

      puts(files[i]->d_name);
      sprintf(filename, "%s/%s", directory_, files[i]->d_name);

      if (filename_isdir(filename))
	add(files[i]->d_name, (void *)draw_folder);
      else if (filename_match(files[i]->d_name, pattern_))
      {
	for (ic = icons_; ic != NULL; ic = ic->next)
          if (filename_match(files[i]->d_name, ic->pattern))
	    break;

	if (ic)
          add(files[i]->d_name, (void *)ic->drawfunc);
	else
          add(files[i]->d_name, (void *)draw_file);
      }

      free(files[i]);
    }

    free(files);
  }

  return (num_files);
}


void
FileBrowser::icon(const char *pattern,
                  void       (*drawfunc)(Fl_Color))
{
  FBIcon	*ic;


  ic = new FBIcon;
  ic->pattern  = pattern;
  ic->drawfunc = drawfunc;

  ic->next = icons_;
  icons_   = ic;
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
// 'FileBrowser::draw_drive()' - Draw a "drive" icon.
//

void
FileBrowser::draw_drive(Fl_Color c)	// I - Selection color
{
  fl_color(c);
  fl_begin_polygon();
    fl_vertex(16.97, 45.89);
    fl_vertex(72.05, 73.44);
    fl_vertex(72.05, 41.30);
    fl_vertex(16.97, 13.76);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(16.97, 45.89);
    fl_vertex(72.05, 73.44);
    fl_vertex(72.05, 41.30);
    fl_vertex(16.97, 13.76);
  fl_end_loop();

  fl_color(FL_DARK3);
  fl_begin_polygon();
    fl_vertex(30.74, 39.01);
    fl_vertex(21.56, 34.42);
    fl_vertex(21.56, 29.83);
    fl_vertex(35.33, 36.71);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(49.10, 52.78);
    fl_vertex(35.33, 45.89);
    fl_vertex(35.33, 41.30);
    fl_vertex(49.10, 48.19);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(53.69, 50.48);
    fl_vertex(49.10, 52.78);
    fl_vertex(49.10, 48.19);
    fl_vertex(53.69, 45.89);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(67.46, 57.37);
    fl_vertex(53.69, 50.48);
    fl_vertex(53.69, 45.89);
    fl_vertex(67.46, 52.78);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(26.15, -0.01);
    fl_vertex(12.38, 6.89);
    fl_vertex(67.46, 34.43);
    fl_vertex(81.24, 27.53);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(21.56, 34.42);
    fl_vertex(35.33, 41.30);
    fl_vertex(35.33, 45.89);
    fl_vertex(53.69, 55.08);
    fl_vertex(53.69, 50.48);
    fl_vertex(67.46, 57.37);
    fl_vertex(67.46, 52.78);
    fl_vertex(53.69, 45.89);
    fl_vertex(53.69, 36.71);
    fl_vertex(35.33, 27.53);
    fl_vertex(35.33, 36.71);
    fl_vertex(21.56, 29.83);
  fl_end_loop();

  fl_begin_line();
    fl_vertex(53.69, 36.71);
    fl_vertex(49.10, 48.19);
    fl_vertex(53.69, 45.89);
  fl_end_line();

  fl_begin_line();
    fl_vertex(35.33, 36.71);
    fl_vertex(30.74, 39.01);
    fl_vertex(49.10, 48.19);
  fl_end_line();

  fl_begin_line();
    fl_vertex(49.10, 52.78);
    fl_vertex(53.69, 50.48);
  fl_end_line();

  fl_color(FL_DARK3);
  fl_begin_polygon();
    fl_vertex(60.76, 44.83);
    fl_vertex(60.68, 42.11);
    fl_vertex(62.88, 43.39);
    fl_vertex(62.87, 45.89);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(60.76, 44.83);
    fl_vertex(60.68, 42.11);
    fl_vertex(62.88, 43.39);
    fl_vertex(62.87, 45.89);
  fl_end_loop();
}


//
// 'FileBrowser::draw_file()' - Draw a "generic file" icon.
//

void
FileBrowser::draw_file(Fl_Color c)	// I - Selection color
{
  fl_color(FL_DARK3);
  fl_begin_polygon();
    fl_vertex(38.83, 0.22);
    fl_vertex(22.00, 9.12);
    fl_vertex(60.55, 28.78);
    fl_vertex(77.82, 20.20);
  fl_end_polygon();

  fl_color(c);
  fl_begin_polygon();
    fl_vertex(22.00, 19.87);
    fl_vertex(22.00, 79.50);
    fl_vertex(60.01, 98.93);
    fl_vertex(60.01, 39.20);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(22.00, 19.87);
    fl_vertex(22.00, 79.50);
    fl_vertex(60.01, 98.93);
    fl_vertex(60.01, 39.20);
  fl_end_loop();

  fl_color(c);
  fl_begin_polygon();
    fl_vertex(30.69, 15.53);
    fl_vertex(30.69, 74.83);
    fl_vertex(68.70, 94.59);
    fl_vertex(68.70, 34.97);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(30.69, 15.53);
    fl_vertex(30.69, 74.83);
    fl_vertex(68.70, 94.59);
    fl_vertex(68.70, 34.97);
  fl_end_loop();

  fl_color(c);
  fl_begin_polygon();
    fl_vertex(39.59, 11.51);
    fl_vertex(39.59, 70.48);
    fl_vertex(77.39, 89.92);
    fl_vertex(77.39, 30.84);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(39.59, 11.51);
    fl_vertex(39.59, 70.48);
    fl_vertex(77.39, 89.92);
    fl_vertex(77.39, 30.84);
  fl_end_loop();
}


//
// 'FileBrowser::draw_folder()' - Draw a "folder" icon.
//

void
FileBrowser::draw_folder(Fl_Color c)	// I - Selection color
{
  fl_color(FL_DARK3);
  fl_begin_polygon();
    fl_vertex(26.37, 0.00);
    fl_vertex(15.00, 5.69);
    fl_vertex(71.86, 34.11);
    fl_vertex(83.23, 28.43);
  fl_end_polygon();

  fl_color(c);
  fl_begin_complex_polygon();
    fl_vertex(21.14, 13.87);
    fl_vertex(17.27, 15.81);
    fl_vertex(17.27, 63.22);
    fl_vertex(26.83, 68.23);
    fl_vertex(45.25, 77.67);
    fl_vertex(73.22, 91.65);
    fl_vertex(73.22, 44.01);
    fl_vertex(76.97, 41.85);
  fl_end_complex_polygon();

  fl_begin_polygon();
    fl_vertex(26.83, 68.23);
    fl_vertex(28.42, 73.00);
    fl_vertex(43.66, 81.76);
    fl_vertex(45.25, 77.67);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_line();
    fl_vertex(21.14, 13.87);
    fl_vertex(17.27, 15.81);
    fl_vertex(17.27, 63.22);
    fl_vertex(26.83, 68.23);
    fl_vertex(28.42, 73.00);
    fl_vertex(28.42, 73.00);
    fl_vertex(43.66, 81.76);
    fl_vertex(45.25, 77.67);
    fl_vertex(73.22, 91.65);
    fl_vertex(73.22, 44.01);
    fl_vertex(76.97, 41.85);
  fl_end_line();

  fl_color(c);
  fl_begin_complex_polygon();
    fl_vertex(21.14, 13.87);
    fl_vertex(21.14, 60.50);
    fl_vertex(29.44, 64.82);
    fl_vertex(31.49, 61.06);
    fl_vertex(47.07, 68.80);
    fl_vertex(47.64, 73.91);
    fl_vertex(76.97, 88.47);
    fl_vertex(76.97, 41.85);
  fl_end_complex_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(21.14, 13.87);
    fl_vertex(21.14, 60.50);
    fl_vertex(29.44, 64.82);
    fl_vertex(31.49, 61.06);
    fl_vertex(47.07, 68.80);
    fl_vertex(47.64, 73.91);
    fl_vertex(76.97, 88.47);
    fl_vertex(76.97, 41.85);
  fl_end_loop();
}


//
// End of "$Id: FileBrowser.cxx,v 1.3 1999/02/05 02:15:59 mike Exp $".
//
