//
// "$Id: FileChooser2.cxx,v 1.11 1999/04/28 21:28:35 mike Exp $"
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
//   FileChooser::init_icons() - Initialize the default icons used by the
//                               FileChooser widget.
//   FileChooser::directory()  - Set the directory in the file chooser.
//   FileChooser::count()      - Return the number of selected files.
//   FileChooser::value()      - Return a selected filename.
//   FileChooser::up()         - Go up one directory.
//   FileChooser::newdir()     - Make a new directory.
//   FileChooser::rescan()     - Rescan the current directory.
//   FileChooser::fileListCB() - Handle clicks (and double-clicks) in the
//                               FileBrowser.
//   FileChooser::fileNameCB() - Handle text entry in the FileBrowser.
//

//
// Include necessary headers.
//

#include "FileChooser.h"
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/x.H>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(__EMX__)
#  include <io.h>
#else
#  include <unistd.h>
#endif /* WIN32 || __EMX__ */


//
// 'FileChooser::init_icons()' - Initialize the default icons used by the
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
  int	levels;				// Number of levels in directory


  // NULL == current directory
  if (d == NULL)
    d = ".";

  // Make the directory absolute...
  if (d[0] != '\0')
  {
#if defined(WIN32) || defined(__EMX__)
    if (d[0] != '/' && d[0] != '\\' && d[1] != ':')
#else
    if (d[0] != '/' && d[0] != '\\')
#endif /* WIN32 || __EMX__ */
      filename_absolute(directory_, d);
    else
      strcpy(directory_, d);

    // Strip any trailing slash and/or period...
    dirptr = directory_ + strlen(directory_) - 1;
    if (*dirptr == '.')
      *dirptr-- = '\0';
    if ((*dirptr == '/' || *dirptr == '\\') && dirptr > directory_)
      *dirptr = '\0';
  }
  else
    directory_[0] = '\0';

  // Clear the directory menu and fill it as needed...
  dirMenu->clear();
#if defined(WIN32) || defined(__EMX__)
  dirMenu->add("My Computer");
#else
  dirMenu->add("File Systems");
#endif /* WIN32 || __EMX__ */

  levels = 0;
  for (dirptr = directory_, pathptr = pathname; *dirptr != '\0';)
  {
    if (*dirptr == '/' || *dirptr == '\\')
    {
      // Need to quote the slash first, and then add it to the menu...
      *pathptr++ = '\\';
      *pathptr++ = '/';
      *pathptr++ = '\0';
      dirptr ++;

      dirMenu->add(pathname);
      levels ++;
      pathptr = pathname;
    }
    else
      *pathptr++ = *dirptr++;
  }

  if (pathptr > pathname)
  {
    *pathptr = '\0';
    dirMenu->add(pathname);
    levels ++;
  }

  dirMenu->value(levels);

  // Rescan the directory...
  rescan();
}


//
// 'FileChooser::count()' - Return the number of selected files.
//

int				// O - Number of selected files
FileChooser::count()
{
  int		i;		// Looping var
  int		count;		// Number of selected files
  const char	*filename;	// Filename in input field or list
  char		pathname[1024];	// Full path to file


  if (type_ != MULTI)
  {


    // Check to see if the file name input field is blank...
    filename = fileName->value();
    if (filename == NULL || filename[0] == '\0')
      return (0);
    else
      return (1);
  }

  for (i = 1, count = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i))
    {
      // See if this file is a directory...
      filename = (char *)fileList->text(i);
      if (directory_[0] != '\0')
	sprintf(pathname, "%s/%s", directory_, filename);
      else
	strcpy(pathname, filename);

      if (!filename_isdir(pathname))
	count ++;
    }

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


  if (type_ != MULTI)
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
      // See if this file is a directory...
      name = fileList->text(i);
      sprintf(pathname, "%s/%s", directory_, name);

      if (!filename_isdir(pathname))
      {
        // Nope, see if this this is "the one"...
	count ++;
	if (count == f)
          return ((const char *)pathname);
      }
    }

  return (NULL);
}


//
// 'FileChooser::value()' - Set the current filename.
//

void
FileChooser::value(const char *filename)	// I - Filename + directory
{
  int	i,					// Looping var
  	count;					// Number of items in list
  char	*slash;					// Directory separator
  char	pathname[1024];				// Local copy of filename


  // See if the filename is actually a directory...
  if (filename_isdir(pathname))
  {
    // Yes, just change the current directory...
    directory(pathname);
    return;
  }

  // Switch to single-selection mode as needed
  if (type_ != MULTI)
    type(SINGLE);

  // See if there is a directory in there...
  strcpy(pathname, filename);
  if ((slash = strrchr(pathname, '/')) == NULL)
    slash = strrchr(pathname, '\\');

  if (slash != NULL)
  {
    // Yes, change the display to the directory... 
    *slash++ = '\0';
    directory(pathname);
  }
  else
    slash = pathname;

  // Set the input field to the remaining portion
  fileName->value(slash);
  fileName->position(0, strlen(slash));

  // Then find the file in the file list and select it...
  count = fileList->size();

  for (i = 1; i <= count; i ++)
    if (strcmp(fileList->text(i), slash) == 0)
    {
      fileList->select(i);
      break;
    }
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

  if (directory_[0] != '\0')
    dirMenu->value(dirMenu->value() - 1);

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
  // Build the file list...
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
  char		*filename,	// New filename
		*slash,		// Pointer to trailing slash
		pathname[1024];	// Full pathname to file
  int		i,		// Looping var
		min_match,	// Minimum number of matching chars
		max_match,	// Maximum number of matching chars
		num_files;	// Number of files in directory
  const char	*file;		// File from directory


  // Get the filename from the text field...
  filename = (char *)fileName->value();

  if (filename == NULL || filename[0] == '\0')
    return;

#if defined(WIN32) || defined(__EMX__)
  if (directory_[0] != '\0' &&
      filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0]) && filename[1] == ':'))
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);
#else
  if (directory_[0] != '\0' &&
      filename[0] != '/')
    sprintf(pathname, "%s/%s", directory_, filename);
  else
    strcpy(pathname, filename);
#endif /* WIN32 || __EMX__ */

  if (Fl::event_key() == FL_Enter)
  {
    // Enter pressed - select or change directory...

#if defined(WIN32) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        filename_isdir(pathname))
#else
    if (filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
      directory(pathname);
    else if (type_ == CREATE || access(pathname, 0) == 0)
    {
      // New file or file exists...  If we are in multiple selection mode,
      // switch to single selection mode...
      if (type_ == MULTI)
        type(SINGLE);

      // Hide the window to signal things are done...
      window->hide();
    }
    else
    {
      // File doesn't exist, so beep at the user...
      // TODO: NEED TO ADD fl_beep() FUNCTION TO 2.0!
#ifdef WIN32
      MessageBeep(MB_ICONEXCLAMATION);
#else
      XBell(fl_display, 100);
#endif // WIN32
    }
  }
  else if (Fl::event_key() != FL_Delete)
  {
    // Check to see if the user has entered a directory...

    if ((slash = strrchr(filename, '/')) == NULL)
      slash = strrchr(filename, '\\');

    if (slash != NULL)
    {
      // Yes, change directories and update the file name field...
      if ((slash = strrchr(pathname, '/')) == NULL)
	slash = strrchr(pathname, '\\');

      if (slash > pathname)		// Special case for "/"
        *slash++ = '\0';
      else
        slash++;

      if (strcmp(filename, "../") == 0)	// Special case for "../"
        up();
      else
        directory(pathname);

      // If the string ended after the slash, we're done for now...
      if (*slash == '\0')
        return;

      // Otherwise copy the remainder and proceed...
      fileName->value(slash);
      fileName->position(strlen(slash));
      filename = slash;
    }

    // Other key pressed - do filename completion as possible...

    num_files = fileList->size();
    min_match = strlen(filename);
    max_match = 100000;

    for (i = 1; i <= num_files && max_match > min_match; i ++)
    {
      file = fileList->text(i);

      if (strncmp(filename, file, min_match) == 0)
      {
        // OK, this one matches; check against the previous match

	if (max_match == 100000)
	{
	  // First match; copy stuff over...
	  max_match = strlen(file);
	  strcpy(pathname, file);
	}
	else
	{
	  // Succeeding match; compare to find maximum string match...
	  while (max_match > min_match)
	    if (strncmp(file, pathname, max_match) == 0)
	      break;
	    else
	      max_match --;

          // Truncate the string as needed...
          pathname[max_match] = '\0';
	}
      }
    }

    // If we have any matches, add them to the input field...
    if (max_match > min_match && max_match != 100000)
    {
      fileName->insert(pathname + min_match);

      if (Fl::event_key() == FL_BackSpace)
        fileName->position(min_match - 1, max_match);
      else
        fileName->position(min_match, max_match);
    }
  }
}


//
// End of "$Id: FileChooser2.cxx,v 1.11 1999/04/28 21:28:35 mike Exp $".
//
