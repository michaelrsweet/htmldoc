//
// "$Id: FileChooser2.cxx,v 1.1 1999/02/02 03:55:36 mike Exp $"
//
//   More FileChooser routines for HTMLDOC, an HTML document processing program.
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
// Include necessary headers.
//

#include "FileChooser.h"
#include <FL/filename.H>
#include <FL/fl_draw.H>


//
// Local functions...
//

static void	draw_htmlfile(Fl_Color c);
static void	draw_imagefile(Fl_Color c);
static void	draw_newfolder(Fl_Color c);
static void	draw_prevfolder(Fl_Color c);
static void	file(Fl_Color c, int filled);
static void	folder(Fl_Color c, int filled);
static void	outline(Fl_Color c);


//
// 'FileChooser::init_symbols()' - Initialize the symbols used for the
//                                 FileChooser widget.
//

void
FileChooser::init_symbols()
{
  static int init = 0;


  //
  // Add symbols if they haven't been already...
  //

  if (!init)
  {
    init = 1;

    fl_add_symbol("NewFolder", draw_newfolder, 1);
    fl_add_symbol("PrevFolder", draw_prevfolder, 1);

    fileList->icon("*.{html,htm}", draw_htmlfile);
    fileList->icon("*.{gif,ico,jpg,jpeg,png}", draw_imagefile);
  }
}


//
// 'FileChooser::new_directory()' - Create a new directory.
//

void
FileChooser::new_directory()
{
}


//
// 'FileChooser::rescan()' - Rescan the current directory.
//

void
FileChooser::rescan()
{
  int		i;		// Looping var
  char		filename[1024];	// Current filename
  char		*start,		// Start of name
		*slash;		// Slash in directory name


  //
  // Build the directory history chooser...
  //

  strcpy(filename, directory_);
  filename_absolute(filename, filename);
  dirHistory->clear();

#if defined(WIN32) || defined(__EMX__)
  dirHistory->add("My Computer");
#endif /* WIN32 */

  slash = filename;
  while (*slash != '\0')
  {
    start = slash;
    while (*slash != '/' && *slash != '\\' && *slash != '\0')
      slash ++;

    if (slash == start)
      dirHistory->add("Root Directory");
    else if (strcmp(start, ".") != 0)
    {
      if (*slash != '\0')
        *slash++ = '\0';

      dirHistory->add(start);
    }
  }

  dirHistory->value(dirHistory->size() - 1);

  //
  // Then build the file list...
  //

  fileList->load(directory_);
}


void
FileChooser::fileListCB(Fl_Widget *w, void *data)
{
  char	*filename,
	pathname[1024];


  filename = (char *)fileList->text(fileList->value());
  fileName->value(filename);
  sprintf(pathname, "%s/%s", directory_, filename);

  if (Fl::event_clicks() > 1)
  {
    if (filename_isdir(pathname))
      directory(pathname);
  };
}


//
// 'draw_htmlfile()' - Draw a "HTML file" icon.
//

static void
draw_htmlfile(Fl_Color c)	// I - Color to use
{
  Fl_Color	oc;		// Outline color


  oc = fl_color_average(c, FL_BLACK, 0.5);

  fl_color(FL_GRAY);
  fl_begin_line();
    fl_vertex(57.00, 61.10);
    fl_vertex(42.00, 68.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(57.00, 56.10);
    fl_vertex(42.00, 48.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(62.00, 38.60);
    fl_vertex(62.00, 53.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(79.40, 52.00);
    fl_vertex(71.40, 56.10);
  fl_end_line();

  fl_begin_line();
    fl_vertex(62.00, 78.60);
    fl_vertex(62.00, 63.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(79.76, 65.28);
    fl_vertex(71.40, 61.10);
  fl_end_line();

  fl_begin_loop();
    fl_vertex(62.00, 53.60);
    fl_vertex(67.00, 56.10);
    fl_vertex(67.00, 61.10);
    fl_vertex(62.00, 63.60);
    fl_vertex(57.00, 61.10);
    fl_vertex(57.00, 56.10);
  fl_end_loop();

  fl_begin_loop();
    fl_vertex(62.00, 47.20);
    fl_vertex(73.47, 52.93);
    fl_vertex(73.47, 64.40);
    fl_vertex(62.00, 70.14);
    fl_vertex(50.53, 64.40);
    fl_vertex(50.53, 52.93);
  fl_end_loop();

  fl_begin_polygon();
    fl_vertex(68.18, 1.58);
    fl_vertex(63.30, 3.68);
    fl_vertex(61.67, 7.77);
    fl_vertex(63.30, 11.62);
    fl_vertex(67.80, 13.91);
    fl_vertex(74.40, 14.60);
    fl_vertex(80.61, 13.64);
    fl_vertex(85.49, 11.62);
    fl_vertex(87.26, 7.85);
    fl_vertex(85.49, 3.68);
    fl_vertex(80.61, 1.50);
    fl_vertex(74.40, 0.70);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_line();
    fl_vertex(43.80, 18.80);
    fl_vertex(43.80, 29.32);
  fl_end_line();

  fl_begin_line();
    fl_vertex(49.26, 21.38);
    fl_vertex(49.26, 32.18);
  fl_end_line();

  fl_begin_line();
    fl_vertex(49.26, 26.77);
    fl_vertex(43.80, 23.92);
  fl_end_line();

  fl_begin_line();
    fl_vertex(57.80, 36.00);
    fl_vertex(51.89, 32.88);
  fl_end_line();

  fl_begin_line();
    fl_vertex(55.00, 34.20);
    fl_vertex(55.00, 24.00);
  fl_end_line();

  fl_begin_line();
    fl_vertex(67.20, 30.40);
    fl_vertex(67.20, 40.06);
    fl_vertex(64.05, 33.84);
    fl_vertex(60.64, 36.74);
    fl_vertex(60.64, 26.71);
  fl_end_line();

  fl_begin_line();
    fl_vertex(75.80, 34.60);
    fl_vertex(70.80, 32.10);
    fl_vertex(70.80, 42.10);
  fl_end_line();

  fl_color(FL_RED);
  fl_begin_polygon();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_loop();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_loop();

  fl_color(FL_GREEN);
  fl_begin_polygon();
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(74.17, 67.49);
  fl_end_polygon();

  fl_color(FL_BLUE);
  fl_begin_polygon();
    fl_vertex(66.44, 67.88);
    fl_vertex(63.30, 65.34);
    fl_vertex(61.67, 58.16);
    fl_vertex(62.50, 54.28);
    fl_vertex(63.56, 56.55);
    fl_vertex(65.62, 55.69);
    fl_vertex(67.33, 54.66);
    fl_vertex(68.52, 57.06);
    fl_vertex(70.06, 58.77);
    fl_vertex(70.41, 60.65);
    fl_vertex(67.84, 61.25);
    fl_vertex(65.62, 62.19);
    fl_vertex(65.62, 64.07);
    fl_vertex(67.84, 65.53);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(70.80, 51.60);
    fl_vertex(71.60, 48.00);
    fl_vertex(71.82, 45.68);
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(62.50, 54.28);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(72.97, 64.41);
    fl_vertex(75.20, 65.78);
    fl_vertex(76.39, 63.05);
    fl_vertex(74.34, 60.99);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(83.63, 66.78);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(86.83, 56.38);
    fl_vertex(83.75, 57.06);
    fl_vertex(82.38, 57.40);
    fl_vertex(81.87, 55.69);
    fl_vertex(80.84, 57.06);
    fl_vertex(78.96, 56.03);
    fl_vertex(78.96, 56.03);
    fl_vertex(77.76, 57.23);
    fl_vertex(78.10, 59.11);
    fl_vertex(78.96, 60.31);
    fl_vertex(80.84, 60.31);
    fl_vertex(81.87, 61.68);
    fl_vertex(81.18, 62.88);
    fl_vertex(79.64, 62.88);
    fl_vertex(79.13, 64.93);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(79.64, 54.49);
    fl_vertex(82.04, 53.98);
    fl_vertex(86.60, 55.38);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(78.64, 46.01);
    fl_vertex(79.47, 47.65);
    fl_vertex(77.59, 48.34);
    fl_vertex(76.39, 50.90);
    fl_vertex(78.10, 53.98);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_loop();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_loop();

  fl_color(FL_GRAY);
  fl_begin_line();
    fl_vertex(72.40, 71.60);
    fl_vertex(62.00, 76.60);
    fl_vertex(44.08, 67.56);
    fl_vertex(43.92, 49.56);
    fl_vertex(62.00, 40.60);
    fl_vertex(70.00, 45.00);
  fl_end_line();
}


//
// 'draw_imagefile()' - Draw an "image file" icon.
//

static void
draw_imagefile(Fl_Color c)	// I - Color to use
{
  Fl_Color	oc;		// Outline color


  oc = fl_color_average(c, FL_BLACK, 0.5);

  fl_color(FL_GRAY);
  fl_begin_line();
    fl_vertex(57.00, 61.10);
    fl_vertex(42.00, 68.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(57.00, 56.10);
    fl_vertex(42.00, 48.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(62.00, 38.60);
    fl_vertex(62.00, 53.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(79.40, 52.00);
    fl_vertex(71.40, 56.10);
  fl_end_line();

  fl_begin_line();
    fl_vertex(62.00, 78.60);
    fl_vertex(62.00, 63.60);
  fl_end_line();

  fl_begin_line();
    fl_vertex(79.76, 65.28);
    fl_vertex(71.40, 61.10);
  fl_end_line();

  fl_begin_loop();
    fl_vertex(62.00, 53.60);
    fl_vertex(67.00, 56.10);
    fl_vertex(67.00, 61.10);
    fl_vertex(62.00, 63.60);
    fl_vertex(57.00, 61.10);
    fl_vertex(57.00, 56.10);
  fl_end_loop();

  fl_begin_loop();
    fl_vertex(62.00, 47.20);
    fl_vertex(73.47, 52.93);
    fl_vertex(73.47, 64.40);
    fl_vertex(62.00, 70.14);
    fl_vertex(50.53, 64.40);
    fl_vertex(50.53, 52.93);
  fl_end_loop();

  fl_begin_polygon();
    fl_vertex(68.18, 1.58);
    fl_vertex(63.30, 3.68);
    fl_vertex(61.67, 7.77);
    fl_vertex(63.30, 11.62);
    fl_vertex(67.80, 13.91);
    fl_vertex(74.40, 14.60);
    fl_vertex(80.61, 13.64);
    fl_vertex(85.49, 11.62);
    fl_vertex(87.26, 7.85);
    fl_vertex(85.49, 3.68);
    fl_vertex(80.61, 1.50);
    fl_vertex(74.40, 0.70);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_line();
    fl_vertex(43.80, 18.80);
    fl_vertex(43.80, 29.32);
  fl_end_line();

  fl_begin_line();
    fl_vertex(49.26, 21.38);
    fl_vertex(49.26, 32.18);
  fl_end_line();

  fl_begin_line();
    fl_vertex(49.26, 26.77);
    fl_vertex(43.80, 23.92);
  fl_end_line();

  fl_begin_line();
    fl_vertex(57.80, 36.00);
    fl_vertex(51.89, 32.88);
  fl_end_line();

  fl_begin_line();
    fl_vertex(55.00, 34.20);
    fl_vertex(55.00, 24.00);
  fl_end_line();

  fl_begin_line();
    fl_vertex(67.20, 30.40);
    fl_vertex(67.20, 40.06);
    fl_vertex(64.05, 33.84);
    fl_vertex(60.64, 36.74);
    fl_vertex(60.64, 26.71);
  fl_end_line();

  fl_begin_line();
    fl_vertex(75.80, 34.60);
    fl_vertex(70.80, 32.10);
    fl_vertex(70.80, 42.10);
  fl_end_line();

  fl_color(FL_RED);
  fl_begin_polygon();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_loop();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_loop();

  fl_color(FL_GREEN);
  fl_begin_polygon();
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(74.17, 67.49);
  fl_end_polygon();

  fl_color(FL_BLUE);
  fl_begin_polygon();
    fl_vertex(66.44, 67.88);
    fl_vertex(63.30, 65.34);
    fl_vertex(61.67, 58.16);
    fl_vertex(62.50, 54.28);
    fl_vertex(63.56, 56.55);
    fl_vertex(65.62, 55.69);
    fl_vertex(67.33, 54.66);
    fl_vertex(68.52, 57.06);
    fl_vertex(70.06, 58.77);
    fl_vertex(70.41, 60.65);
    fl_vertex(67.84, 61.25);
    fl_vertex(65.62, 62.19);
    fl_vertex(65.62, 64.07);
    fl_vertex(67.84, 65.53);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(70.80, 51.60);
    fl_vertex(71.60, 48.00);
    fl_vertex(71.82, 45.68);
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(62.50, 54.28);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(72.97, 64.41);
    fl_vertex(75.20, 65.78);
    fl_vertex(76.39, 63.05);
    fl_vertex(74.34, 60.99);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(83.63, 66.78);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(86.83, 56.38);
    fl_vertex(83.75, 57.06);
    fl_vertex(82.38, 57.40);
    fl_vertex(81.87, 55.69);
    fl_vertex(80.84, 57.06);
    fl_vertex(78.96, 56.03);
    fl_vertex(78.96, 56.03);
    fl_vertex(77.76, 57.23);
    fl_vertex(78.10, 59.11);
    fl_vertex(78.96, 60.31);
    fl_vertex(80.84, 60.31);
    fl_vertex(81.87, 61.68);
    fl_vertex(81.18, 62.88);
    fl_vertex(79.64, 62.88);
    fl_vertex(79.13, 64.93);
  fl_end_polygon();

  fl_begin_polygon();
    fl_vertex(79.64, 54.49);
    fl_vertex(82.04, 53.98);
    fl_vertex(86.60, 55.38);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(78.64, 46.01);
    fl_vertex(79.47, 47.65);
    fl_vertex(77.59, 48.34);
    fl_vertex(76.39, 50.90);
    fl_vertex(78.10, 53.98);
  fl_end_polygon();

  fl_color(oc);
  fl_begin_loop();
    fl_vertex(68.18, 46.63);
    fl_vertex(63.30, 50.54);
    fl_vertex(61.67, 58.16);
    fl_vertex(63.30, 65.34);
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(85.49, 65.34);
    fl_vertex(87.26, 58.32);
    fl_vertex(85.49, 50.54);
    fl_vertex(80.61, 46.48);
    fl_vertex(74.40, 45.00);
  fl_end_loop();

  fl_color(FL_GRAY);
  fl_begin_line();
    fl_vertex(72.40, 71.60);
    fl_vertex(62.00, 76.60);
    fl_vertex(44.08, 67.56);
    fl_vertex(43.92, 49.56);
    fl_vertex(62.00, 40.60);
    fl_vertex(70.00, 45.00);
  fl_end_line();
}


//
// 'draw_newfolder()' - Draw a "new folder" icon.
//

static void
draw_newfolder(Fl_Color c)	// I - Color to use
{
  fl_push_matrix();
  fl_scale(0.6);
    folder(c, 1);
    folder(c, 0);
  fl_pop_matrix();

  fl_begin_line();
    fl_vertex(0.3, -0.5);
    fl_vertex(0.5, -0.3);
  fl_end_line();

  fl_begin_line();
    fl_vertex(0.5, -0.5);
    fl_vertex(0.3, -0.3);
  fl_end_line();

  fl_begin_line();
    fl_vertex(0.4, -0.5);
    fl_vertex(0.4, -0.3);
  fl_end_line();

  fl_begin_line();
    fl_vertex(0.3, -0.4);
    fl_vertex(0.5, -0.4);
  fl_end_line();
}


//
// 'draw_prevfolder()' - Draw a "previous folder" icon.
//

static void
draw_prevfolder(Fl_Color c)
{
  folder(c, 1);
  folder(c, 0);

  fl_begin_line();
    fl_vertex(0.3, -0.3);
    fl_vertex(-0.1, 0.3);
    fl_vertex(-0.1, -0.1);
  fl_end_line();

  fl_begin_polygon();
    fl_vertex(-0.3, 0.1);
    fl_vertex(0.1, 0.1);
    fl_vertex(-0.1, -0.1);
  fl_end_polygon();
}


//
// 'file()' - Draw a generic file icon.
//

static void
file(Fl_Color c,	// I - Color to set
     int      filled)	// I - O = outline, 1 = fill
{
  if (filled)
  {
    fl_color(c);
    fl_begin_polygon();
  }
  else
  {
    outline(c);
    fl_begin_loop();
  }

  fl_vertex(-0.3, 0.5);
  fl_vertex(-0.3, -0.5);
  fl_vertex(0.1, -0.5);
  fl_vertex(0.3, -0.3);
  fl_vertex(0.3, 0.5);

  if (filled)
    fl_end_polygon();
  else
    fl_end_loop();
}


//
// 'folder()' - Draw a generic folder icon.
//

static void
folder(Fl_Color c,	// I - Color to set
       int      filled)	// I - 0 = outline, 1 = fill
{
  if (filled)
  {
    fl_color(FL_YELLOW);
    fl_begin_polygon();
  }
  else
  {
    outline(FL_YELLOW);
    fl_begin_loop();
  }

  fl_vertex(-0.5, 0.5);
  fl_vertex(-0.5, -0.3);
  fl_vertex(-0.3, -0.5);
  fl_vertex(-0.1, -0.5);
  fl_vertex(0.1, -0.3);
  if (!filled)
    fl_vertex(-0.5, -0.3);
  fl_vertex(0.5, -0.3);
  fl_vertex(0.5, 0.5);

  if (filled)
    fl_end_polygon();
  else
    fl_end_loop();
}


//
// 'outline()' - Choose an appropriate outline color.
//

static void
outline(Fl_Color c)	// I - Color to set
{
  fl_color(fl_color_average(c, FL_BLACK, 0.5));
}


//
// End of "$Id: FileChooser2.cxx,v 1.1 1999/02/02 03:55:36 mike Exp $".
//
