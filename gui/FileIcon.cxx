//
// "$Id: FileIcon.cxx,v 1.1 1999/04/21 23:51:53 mike Exp $"
//
//   FileIcon routines for HTMLDOC, an HTML document processing program.
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

#include "FileIcon.h"
#include <FL/fl_draw.H>

//
// Icon cache...
//

FileIcon	*FileIcon::_first = 0;



FileIcon::FileIcon(const char *p,
                   int        t,
		   int        nd,
		   short      *d)
{
}


FileIcon::FileIcon(const char *p,
                   int        t,
		   const char *fti)
{
}


FileIcon::~FileIcon()
{
}


short *
FileIcon::add(short d)
{
}


short *
FileIcon::add_color(short c)
{
}


short *
FileIcon::add_vertex(short x,
                     short y)
{
}


FileIcon *
FileIcon::find(const char *filename)
{
}


void
FileIcon::draw(short *d)
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
// End of "$Id: FileIcon.cxx,v 1.1 1999/04/21 23:51:53 mike Exp $".
//
