//
// "$Id: CheckButton.cxx,v 1.10.2.4 2002/09/11 19:24:38 swdev Exp $"
//
//   CheckButton routines for the HTMLDOC software.
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
// Contents:
//

//
// Include necessary header files...
//

#include <FL/Fl.H>
#include "CheckButton.h"
#include <FL/fl_draw.H>


//
// CheckButton is a subclass of Fl_Button like Fl_Check_Button, only the
// toggle and radio images are more like Microsoft Windows.
//


//
// 'CheckButton::draw()' - Draw the check button.
//

void CheckButton::draw()
{
  int	size,		// Size of button...
	offset;		// Box Y offset...

  size   = labelsize();
  offset = (h() - size) / 2;

  if (type() == FL_RADIO_BUTTON)
  {
    // Draw the radio "hole"...
    draw_box(FL_ROUND_DOWN_BOX, x(), y() + offset, size, size, FL_WHITE);

    // Then the check...
    if (value())
    {
      fl_color(active_r() ? FL_BLACK : FL_GRAY);

      if (size > 14)
        fl_pie(x() + 5, y() + offset + 5, size - 10, size - 10, 0.0, 360.0);
      else
      {
        // Small circles don't draw well with some X servers...
	fl_rect(x() + 6, y() + offset + 5, 2, 4);
	fl_rect(x() + 5, y() + offset + 6, 4, 2);
      }
    }
  }
  else
  {
    // Draw the check "box"...
    draw_box(FL_DOWN_BOX, x(), y() + offset, size, size, FL_WHITE);

    // Then the check...
    if (value())
    {
      fl_color(active_r() ? FL_BLACK : FL_GRAY);
      fl_line(x() + 3, y() + offset + 3,
              x() + size - 4, y() + offset + size - 4);
      fl_line(x() + 4, y() + offset + 3,
              x() + size - 3, y() + offset + size - 4);
      fl_line(x() + 3, y() + offset + size - 4,
              x() + size - 4, y() + offset + 3);
      fl_line(x() + 4, y() + offset + size - 4,
              x() + size - 3, y() + offset + 3);
    }
  }

  // Finally, the label...
  draw_label(x() + 5 * size / 4, y(), w() - 5 * size / 4, h());
}


//
// 'CheckButton::CheckButton()' - Construct a CheckButton widget.
//

CheckButton::CheckButton(int x, int y, int w, int h, const char* l)
: Fl_Button(x, y, w, h, l)
{
  type(FL_TOGGLE_BUTTON);
  align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
}


//
// End of "$Id: CheckButton.cxx,v 1.10.2.4 2002/09/11 19:24:38 swdev Exp $".
//
