//
// "$Id: FileInput.cxx,v 1.1 1999/05/06 17:28:32 mike Exp $"
//
//   FileInput routines for the Common UNIX Printing System (CUPS).
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
//   FileInput::handle() - Handle tab events.
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include "FileInput.h"

//
// Note: This has got to be a runner for the world's shortest class.
//       Basically the whole purpose of this subclass is to alter the
//       behavior of the Tab key; specifically if the user hits the
//       Tab key while text is selected, the cursor is moved to the
//       end of the selected text rather than doing the navigation
//       thing.
//
//       This is a likely enhancement to the 2.0 Fl_Input widget...
//

//
// 'FileInput::handle()' - Handle tab events.
//

int				// O - 1 if we handled the event
FileInput::handle(int event)	// I - Event to handle
{
  if (event == FL_KEYBOARD && Fl::event_key() == FL_Tab &&
      mark() != position())
  {
    // Set the current cursor position to the end of the selection...
    if (mark() > position())
      position(mark());
    else
      position(position());

    return (1);
  }
  else
  {
    // Use the Fl_Input handler...
    return (Fl_Input::handle(event));
  }
}


//
// End of "$Id: FileInput.cxx,v 1.1 1999/05/06 17:28:32 mike Exp $".
//
