//
// "$Id: FileInput.cxx,v 1.3.2.2 2001/04/18 23:24:10 mike Exp $"
//
//   FileInput routines.
//
//   Copyright 1997-2001 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
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
// End of "$Id: FileInput.cxx,v 1.3.2.2 2001/04/18 23:24:10 mike Exp $".
//
