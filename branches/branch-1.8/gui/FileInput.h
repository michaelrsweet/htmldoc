//
// "$Id: FileInput.h,v 1.3.2.2 2001/04/18 23:24:10 mike Exp $"
//
//   FileInput definitions.
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
// End of "$Id: FileInput.h,v 1.3.2.2 2001/04/18 23:24:10 mike Exp $".
//
