//
// "$Id: FileChooser2.cxx,v 1.4 1999/02/24 02:04:09 mike Exp $"
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
static void	mix(int c);


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

    fileList->icon("*.{html,htm}", draw_htmlfile);
    fileList->icon("*.{bmp,bw,gif,ico,jpg,jpeg,png,rgb,tif,xbm,xpm}", draw_imagefile);
  }
}


//
// 'FileChooser::count()' - Return the number of selected files.
//

int
FileChooser::count()
{
  int	i;			// Looping var
  int	count;			// Number of selected files


  if (!(type_ & TYPE_MULTI))
    return (fileList->value() != 0);

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
      count ++;

  return (count);
}


//
// 'FileChooser::value()' - Return a selected filename.
//

const char *			// O - Filename or NULL
FileChooser::value(int f)	// I - File number
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*name;		// Current filename
  static char	pathname[1024];	// Filename + directory

  if (!(type_ & TYPE_MULTI))
  {
    name = fileName->value();
    if (name[0] == '\0')
      return (NULL);

    sprintf(pathname, "%s/%s", directory_, name);
    return ((const char *)pathname);
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      count ++;
      if (count == f)
      {
        name = fileList->text(i);

        sprintf(pathname, "%s/%s", directory_, name);
        return ((const char *)pathname);
      }
    }

  return (NULL);
}


//
// 'FileChooser::up()' - Go up one directory.
//

void
FileChooser::up()
{
  char *slash;


  if ((slash = strrchr(directory_, '/')) == NULL)
    slash = strrchr(directory_, '\\');

  if (slash != NULL)
    *slash = '\0';
  else
  {
    upButton->deactivate();
    directory_[0] = '\0';
  }

  rescan();
}


//
// 'FileChooser::rescan()' - Rescan the current directory.
//

void
FileChooser::rescan()
{
  char	*slash;		// Slash in directory name


  //
  // Then build the file list...
  //

  if (directory_[0] != '\0')
  {
    filename_absolute(directory_, directory_);
    if ((slash = strrchr(directory_, '/')) == NULL)
      slash = strrchr(directory_, '\\');

    if (slash != NULL &&
        (strcmp(slash, "/.") == 0 || strcmp(slash, "\\.") == 0))
      *slash = '\0';
  }

  fileName->value("");
  fileList->load(directory_);
}


//
// 'FileChooser::fileListCB()' - Handle clicks (and double-clicks) in the
//                               FileBrowser.
//

void
FileChooser::fileListCB()
{
  char	*filename,		// New filename
	pathname[1024];		// Full pathname to file


  filename = (char *)fileList->text(fileList->value());
  if (directory_[0] != '\0')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);

  if (Fl::event_clicks())
  {
#if defined(WIN32) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        filename_isdir(pathname))
#else
    if (filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
    {
      directory(pathname);
      upButton->activate();
    }
    else
      window->hide();
  }
  else
    fileName->value(filename);
}


//
// 'FileChooser::fileNameCB()' - Handle text entry in the FileBrowser.
//

void
FileChooser::fileNameCB()
{
  char	*filename,		// New filename
	pathname[1024];		// Full pathname to file


  filename = (char *)fileName->value();
#if 0
  if (directory_[0] != '\0')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);

#if defined(WIN32) || defined(__EMX__)
  if ((strlen(pathname) == 2 && pathname[1] == ':') ||
    filename_isdir(pathname))
#else
  if (filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
  {
    directory(pathname);
    upButton->activate();
  }
  else
  {
    type_ &= ~TYPE_MULTI;
    window->hide();
  }
#else
  strcpy(directory_, filename);
  fileList->load(filename);
#endif /* 0 */
}


//
// 'draw_htmlfile()' - Draw a "HTML file" icon.
//

static void
draw_htmlfile(Fl_Color c)	// I - Color to use
{
  FileBrowser::draw_file(c);

  fl_color(FL_DARK3);
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

  fl_color(FL_BLACK);
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

#if 0
  fl_color(164);
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

  fl_color(FL_BLACK);
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

  mix(119);
  fl_begin_polygon();
    fl_vertex(68.33, 69.41);
    fl_vertex(74.40, 70.89);
    fl_vertex(80.61, 69.11);
    fl_vertex(74.17, 67.49);
  fl_end_polygon();

  mix(195);
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

  fl_color(FL_BLACK);
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
#endif /* 0 */

  fl_color(FL_DARK3);
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
  fl_color(FL_DARK3);
  fl_begin_polygon();
    fl_vertex(32.50, 0.00);
    fl_vertex(22.50, 5.00);
    fl_vertex(68.50, 28.00);
    fl_vertex(77.50, 23.00);
  fl_end_polygon();

  fl_color(c);
  fl_begin_polygon();
    fl_vertex(27.50, 13.00);
    fl_vertex(27.50, 78.00);
    fl_vertex(72.50, 100.00);
    fl_vertex(72.50, 35.00);
  fl_end_polygon();

  fl_color(FL_BLACK);
  fl_begin_loop();
    fl_vertex(27.50, 13.00);
    fl_vertex(27.50, 78.00);
    fl_vertex(72.50, 100.00);
    fl_vertex(72.50, 35.00);
  fl_end_loop();

  fl_color(164);
  fl_begin_polygon();
    fl_vertex(28.83, 24.88);
    fl_vertex(28.79, 72.51);
    fl_vertex(35.60, 67.75);
    fl_vertex(35.60, 63.66);
    fl_vertex(35.60, 59.58);
    fl_vertex(36.28, 55.49);
    fl_vertex(36.96, 52.77);
    fl_vertex(38.32, 48.75);
    fl_vertex(41.04, 45.96);
    fl_vertex(43.09, 42.83);
    fl_vertex(45.13, 39.15);
    fl_vertex(52.77, 36.91);
  fl_end_polygon();

  fl_color(12);
  fl_begin_line();
    fl_vertex(29.47, 23.49);
    fl_vertex(71.00, 44.60);
  fl_end_line();

  fl_color(0);
  fl_begin_line();
    fl_vertex(29.47, 72.51);
    fl_vertex(36.96, 75.92);
  fl_end_line();

  fl_color(14);
  fl_begin_line();
    fl_vertex(57.38, 82.73);
    fl_vertex(71.00, 89.54);
  fl_end_line();

  fl_color(FL_BLACK);
  fl_begin_line();
    fl_vertex(57.38, 80.00);
    fl_vertex(65.55, 84.09);
  fl_end_line();

  fl_begin_line();
    fl_vertex(65.55, 82.73);
    fl_vertex(71.00, 85.45);
  fl_end_line();

  fl_color(12);
  fl_begin_line();
    fl_vertex(58.74, 77.28);
    fl_vertex(64.19, 80.00);
  fl_end_line();

  fl_color(FL_BLUE);
  fl_begin_line();
    fl_vertex(57.38, 74.56);
    fl_vertex(64.19, 77.96);
  fl_end_line();

  fl_color(14);
  fl_begin_line();
    fl_vertex(68.28, 79.32);
    fl_vertex(71.00, 80.68);
  fl_end_line();

  fl_color(12);
  fl_begin_line();
    fl_vertex(58.74, 73.20);
    fl_vertex(66.91, 77.28);
  fl_end_line();

  fl_begin_line();
    fl_vertex(68.28, 58.90);
    fl_vertex(71.00, 60.26);
  fl_end_line();

  fl_begin_line();
    fl_vertex(66.91, 53.45);
    fl_vertex(71.00, 55.49);
  fl_end_line();

  fl_begin_line();
    fl_vertex(65.55, 48.69);
    fl_vertex(71.00, 51.41);
  fl_end_line();

  fl_begin_line();
    fl_vertex(64.19, 44.06);
    fl_vertex(71.00, 47.46);
  fl_end_line();

  fl_color(FL_BLACK);
  fl_begin_line();
    fl_vertex(41.04, 74.56);
    fl_vertex(47.85, 77.96);
  fl_end_line();

  fl_begin_line();
    fl_vertex(38.32, 71.83);
    fl_vertex(49.21, 77.28);
  fl_end_line();

  fl_color(12);
  fl_begin_line();
    fl_vertex(42.40, 69.11);
    fl_vertex(49.21, 72.51);
  fl_end_line();

  fl_color(FL_BLUE);
  fl_begin_line();
    fl_vertex(41.04, 65.03);
    fl_vertex(51.26, 70.13);
  fl_end_line();

  fl_color(12);
  fl_begin_line();
    fl_vertex(46.49, 66.39);
    fl_vertex(49.21, 67.75);
  fl_end_line();

  fl_color(8);
  fl_begin_line();
    fl_vertex(51.94, 60.94);
    fl_vertex(54.66, 62.30);
  fl_end_line();

  fl_begin_line();
    fl_vertex(51.94, 59.58);
    fl_vertex(54.66, 60.94);
  fl_end_line();

  fl_begin_line();
    fl_vertex(57.38, 62.30);
    fl_vertex(60.11, 63.66);
  fl_end_line();

  fl_color(1);
  fl_begin_line();
    fl_vertex(51.94, 52.09);
    fl_vertex(53.30, 52.77);
  fl_end_line();

  mix(17);
  fl_begin_line();
    fl_vertex(56.70, 54.47);
    fl_vertex(62.83, 57.54);
  fl_end_line();

  fl_color(1);
  fl_begin_line();
    fl_vertex(50.57, 49.37);
    fl_vertex(62.83, 55.49);
  fl_end_line();

  fl_color(9);
  fl_begin_line();
    fl_vertex(51.94, 49.37);
    fl_vertex(62.83, 54.81);
  fl_end_line();

  mix(221);
  fl_begin_line();
    fl_vertex(53.30, 48.00);
    fl_vertex(62.83, 52.77);
  fl_end_line();

  fl_color(12);
  fl_begin_line();
    fl_vertex(29.00, 20.50);
    fl_vertex(70.80, 41.40);
  fl_end_line();
}


//
// 'mix()' - Mix colors.
//

static void
mix(int c)		// I - Color value
{
  int	c2;		// Upper nibble of color
  int	r, g, b;	// RGB values
  int	colors[16][3] =	// Standard IRIS color table
	{
	  { 0,   0,   0   },
	  { 255, 0,   0   },
	  { 0,   255, 0   },
	  { 255, 255, 0   },
	  { 0,   0,   255 },
	  { 255, 0,   255 },
	  { 0,   255, 255 },
	  { 255, 255, 255 },
	  { 85,  85,  85  },
	  { 198, 113, 113 },
	  { 113, 198, 113 },
	  { 142, 142, 56  },
	  { 113, 113, 198 },
	  { 142, 56,  142 },
	  { 56,  142, 142 },
	  { 170, 170, 170 }
	};


  c2 = c >> 4;
  c  &= 15;

  r = (colors[c][0] + colors[c2][0]) / 2;
  g = (colors[c][1] + colors[c2][1]) / 2;
  b = (colors[c][2] + colors[c2][2]) / 2;

  fl_color(r, g, b);
}


//
// End of "$Id: FileChooser2.cxx,v 1.4 1999/02/24 02:04:09 mike Exp $".
//
