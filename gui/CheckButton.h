//
// "$Id: CheckButton.h,v 1.3.2.4 2002/09/11 19:24:38 swdev Exp $"
//
//   CheckButton definitions for the HTMLDOC software.
//
//   Copyright 1997-2001 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "LICENSE.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: HTMLDOC Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9603
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
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
// End of "$Id: CheckButton.h,v 1.3.2.4 2002/09/11 19:24:38 swdev Exp $".
//
