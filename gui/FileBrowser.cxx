//
// "$Id: FileBrowser.cxx,v 1.1 1999/02/02 03:55:35 mike Exp $"
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
  return ((int)(fl_width(((FL_BLINE *)p)->txt) + textsize() + 4.5));
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

  fl_draw(line->txt, x + textsize() + 2, y, w - textsize() - 2, h, FL_ALIGN_LEFT);

  if (line->data)
  {
    drawfunc = (void (*)(Fl_Color))line->data;
    fl_push_matrix();
      fl_translate((float)x, (float)(y + h - 2));
      fl_scale((textsize() - 1) / 100.0, -(textsize() - 1) / 100.0);
      (*drawfunc)((line->flags & SELECTED) ? FL_YELLOW : FL_LIGHT2);
    fl_pop_matrix();
  }
}


FileBrowser::FileBrowser(int x, int y, int w, int h, const char *l) :
Fl_Browser(x, y, w, h, l)
{
  pattern_   = "*";
  directory_ = ".";
}


int
FileBrowser::load(const char *directory)
{
  int		i;		// Looping var
  char		filename[1024];	// Current file
  int		num_files;	// Number of files in directory
  dirent	**files;	// Files in in directory
  FBIcon	*ic;		// Icon pointer


  //
  // Build the file list...
  //

  directory_ = directory;
  num_files  = filename_list(directory_, &files);

  clear();
  for (i = 0; i < num_files; i ++)
  {
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
// 'FileBrowser::draw_file()' - Draw a "generic file" icon.
//

void
FileBrowser::draw_file(Fl_Color c)	// I - Selection color
{
  Fl_Color	oc;			// Outline color...


  oc = fl_color_average(c, FL_BLACK, 0.5);

  fl_color(FL_GRAY);
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

  fl_color(oc);
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

  fl_color(oc);
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

  fl_color(oc);
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
  Fl_Color	oc;			// Outline color...


  oc = fl_color_average(c, FL_BLACK, 0.5);

  fl_color(FL_GRAY);
  fl_begin_polygon();
    fl_vertex(26.37, 0.00);
    fl_vertex(15.00, 5.69);
    fl_vertex(71.86, 34.11);
    fl_vertex(83.23, 28.43);
  fl_end_polygon();

  fl_color(c);
  fl_begin_polygon();
    fl_vertex(21.14, 13.87);
    fl_vertex(17.27, 15.81);
    fl_vertex(17.27, 63.22);
    fl_vertex(26.83, 68.23);
    fl_vertex(45.25, 77.67);
    fl_vertex(73.22, 91.65);
    fl_vertex(73.22, 44.01);
    fl_vertex(76.97, 41.85);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(26.83, 68.23);
    fl_vertex(28.42, 73.00);
    fl_vertex(43.66, 81.76);
    fl_vertex(45.25, 77.67);
  fl_end_polygon();

  fl_color(oc);
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
  fl_begin_polygon();
    fl_vertex(21.14, 13.87);
    fl_vertex(21.14, 60.50);
    fl_vertex(29.44, 64.82);
    fl_vertex(31.49, 61.06);
    fl_vertex(47.07, 68.80);
    fl_vertex(47.64, 73.91);
    fl_vertex(76.97, 88.47);
    fl_vertex(76.97, 41.85);
  fl_end_polygon();

  fl_color(oc);
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
// End of "$Id: FileBrowser.cxx,v 1.1 1999/02/02 03:55:35 mike Exp $".
//
