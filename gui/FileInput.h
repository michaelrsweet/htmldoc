//
// "$Id: FileInput.h,v 1.1 1999/05/06 17:28:32 mike Exp $"
//
//   FileInput definitions for the Common UNIX Printing System (CUPS).
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

#ifndef _GUI_FILEINPUT_H_
#  define _GUI_FILEINPUT_H_

//
// Include necessary header files...
//

#  include <FL/Fl_Input.H>


//
// (world's shortest) FileInput class...
//

class FileInput : public Fl_Input	//// File input widget
{
  public:

  FileInput(int x, int y, int w, int h, const char *l = 0) :
      Fl_Input(x, y, w, h, l) {}
  int handle(int);
};

#endif // !_GUI_FILEINPUT_H_

//
// End of "$Id: FileInput.h,v 1.1 1999/05/06 17:28:32 mike Exp $".
//
