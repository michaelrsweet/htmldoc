//
// "$Id: FileChooser2.cxx,v 1.6 1999/04/27 12:43:36 mike Exp $"
//
//   More FileChooser routines for the Common UNIX Printing System (CUPS).
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
// Include necessary headers.
//

#include "FileChooser.h"
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


//
// 'FileChooser::init_icons()' - Initialize the icons used for the
//                               FileChooser widget.
//

void
FileChooser::init_icons()
{
  static int	init = 0;	// Have the icons been initialized?
  static short	plain[] =	// Plain file icon
		{
		  1, 39, 4, 6, 3883, 22, 6, 2200, 912, 6, 6055, 2878,
		  6, 7782, 2020, 0, 1, 256, 5, 0, 6, 2200, 1987, 6,
		  2200, 7950, 6, 6001, 9893, 6, 6001, 3920, 0, 5, 0, 6,
		  3069, 1553, 6, 3069, 7483, 6, 6870, 9459, 6, 6870,
		  3497, 0, 5, 0, 6, 3959, 1151, 6, 3959, 7048, 6, 7739,
		  8992, 6, 7739, 3084,
		  FileIcon::END
		};
  static short	dir[] =		// Directory icon
		{
		  1, 256, 5, 256, 6, 2842, 7300, 6, 2683, 6823, 6,
		  4525, 7767, 6, 4366, 8176, 0, 5, 256, 6, 7697, 4185,
		  6, 7282, 3977, 6, 7320, 8660, 6, 7697, 8847, 0, 5,
		  256, 6, 7282, 3977, 6, 2114, 1387, 6, 1727, 1581, 6,
		  1727, 6322, 6, 2683, 6823, 6, 4525, 7767, 6, 7322,
		  9165, 0, 1, 39, 4, 6, 2637, 0, 6, 1500, 569, 6, 7186,
		  3411, 6, 8323, 2843, 0, 1, 0, 3, 6, 7282, 3977, 6,
		  2114, 1387, 6, 2114, 6050, 6, 2944, 6482, 6, 3149,
		  6106, 6, 4707, 6880, 6, 4764, 7391, 6, 7697, 8847, 6,
		  7697, 4185, 0, 2, 6, 2114, 1387, 6, 1727, 1581, 6,
		  1727, 6322, 6, 2683, 6823, 6, 2842, 7300, 6, 4366,
		  8176, 6, 4525, 7767, 6, 7322, 9165, 6, 7320, 8660,
		  FileIcon::END
		};


  //
  // Add symbols if they haven't been already...
  //

  if (!init)
  {
    // Mark things as initialized...
    init = 1;

    // Create the default icons...
    new FileIcon("*", FileIcon::PLAIN, sizeof(plain) / sizeof(plain[0]), plain);
    new FileIcon("*", FileIcon::DIRECTORY, sizeof(dir) / sizeof(dir[0]), dir);
  }
}


//
// 'FileChooser::directory()' - Set the directory in the file chooser.
//

void
FileChooser::directory(const char *d)	// I - Directory to change to
{
  char	pathname[1024],			// Full path of directory
	*pathptr,			// Pointer into full path
	*dirptr;			// Pointer into directory


  // NULL == current directory
  if (d == NULL)
    d = "";

  // Make the directory absolute...
#if defined(WIN32) || defined(__EMX__)
  if (d[0] != '/' && d[0] != '\\' && d[1] != ':')
#else
  if (d[0] != '/' && d[0] != '\\')
#endif /* WIN32 || __EMX__ */
    filename_absolute(directory_, d);
  else
    strcpy(directory_, d);

  // Set the dirName field...
  dirName->value(directory_);

  // Clear the directory menu and fill it as needed...
  dirMenu->clear();

  for (dirptr = directory_, pathptr = pathname; *dirptr != '\0'; pathptr ++)
  {
    *pathptr = *dirptr++;

    if (*pathptr == '/' || *pathptr == '\\')
    {
      // Need to quote the slash first, and then add it to the menu...
      *pathptr++ = '\\';
      pathptr[0] = '/';
      pathptr[1] = '\0';

      dirMenu->add(pathname);
    }
  }

  // Rescan the directory...
  rescan();
}


//
// 'FileChooser::count()' - Return the number of selected files.
//

int
FileChooser::count()
{
  int	i;			// Looping var
  int	count;			// Number of selected files


  if (!multi_)
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


  if (!multi_)
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
  char *slash;		// Trailing slash


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
// 'FileChooser::newdir()' - Make a new directory.
//

void
FileChooser::newdir()
{
  const char	*dir;		// New directory name
  char		pathname[1024];	// Full path of directory


  // Get a directory name from the user
  if ((dir = fl_input("New Directory?")) == NULL)
    return;

  // Make it relative to the current directory as needed...
#if defined(WIN32) || defined(__EMX__)
  if (dir[0] != '/' && dir[0] != '\\' && dir[1] != ':')
#else
  if (dir[0] != '/' && dir[0] != '\\')
#endif /* WIN32 || __EMX__ */
    sprintf(pathname, "%s/%s", directory_, dir);
  else
    strcpy(pathname, dir);

  // Create the directory; ignore EEXIST errors...
  if (mkdir(pathname, 0777))
    if (errno != EEXIST)
    {
      fl_alert("Unable to create directory!");
      return;
    }

  // Show the new directory...
  directory(pathname);
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
    // Drop trailing "/." and "\."...
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

#if defined(WIN32) || defined(__EMX__)
  if (directory_[0] != '\0' &&
      filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0]) && filename[1] == ':'))
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);

  if ((strlen(pathname) == 2 && pathname[1] == ':') ||
    filename_isdir(pathname))
#else
  if (directory_[0] != '\0' &&
      filename[0] != '/')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);

  if (filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
  {
    directory(pathname);
    upButton->activate();
  }
  else
  {
    multi_ = 0;
    window->hide();
  }
}


//
// End of "$Id: FileChooser2.cxx,v 1.6 1999/04/27 12:43:36 mike Exp $".
//
