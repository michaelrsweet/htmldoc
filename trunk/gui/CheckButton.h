//
// "$Id: CheckButton.h,v 1.1 1999/02/02 03:55:34 mike Exp $"
//
//   CheckButton definitions for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the Aladdin Free Public License
//   which is described in the file "LICENSE.txt".
//

#ifndef _CHECKBUTTON_H_
#  define _CHECKBUTTON_H_

//
// Include necessary headers.
//

#include <FL/Fl_Button.H>


//
// CheckButton class...
//

class CheckButton : public Fl_Button
{
protected:
    virtual void draw();

public:
    CheckButton(int x, int y, int w, int h, const char *l = 0);
};

#endif // !_CHECKBUTTON_H_

//
// End of "$Id: CheckButton.h,v 1.1 1999/02/02 03:55:34 mike Exp $".
//
