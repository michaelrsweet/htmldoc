//
// "$Id: viewfti.cxx,v 1.1.2.2 2004/06/14 12:16:56 mike Exp $"
//
// FTI file viewer.
//
// Copyright 1999-2004 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//
// Contents:
//
//   main() - Show the named FTI file.
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Icon.H>


//
// 'main()' - Create a file chooser and wait for a selection to be made.
//

int					// O - Exit status
main(int  argc,				// I - Number of command-line arguments
     char *argv[])			// I - Command-line arguments
{
  Fl_Window	*window;		// Main window
  Fl_Box	*box;			// Buttons
  Fl_File_Icon	*icon;			// New file icon
  int		i;			// Looping var


  if (argc < 2)
  {
    puts("Usage: viewfti filename.fti ...");
    return (1);
  }

  // Make the file chooser...
  Fl::scheme(NULL);

  // Make the main window...
  window = new Fl_Window(200, 200, "FTI Viewer");


  box = new Fl_Box(10, 10, 180, 180);
  box->box(FL_UP_BOX);

  icon = new Fl_File_Icon("", 0);
  for (i = 1; i < argc; i ++)
    icon->load_fti(argv[i]);
  box->labelcolor(FL_WHITE);
  icon->label(box);

  window->resizable(box);
  window->end();
  window->show();

  Fl::run();

  return (0);
}


//
// End of "$Id: viewfti.cxx,v 1.1.2.2 2004/06/14 12:16:56 mike Exp $".
//
