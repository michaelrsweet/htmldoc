//
// "$Id: CheckButton.cxx,v 1.1 1999/02/02 03:55:32 mike Exp $"
//
//   CheckButton routines for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the Aladdin Free Public License
//   which is described in the file "LICENSE.txt".
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

  size   = 5 * h() / 8;
  offset = (h() - size) / 2;

  if (type() == FL_RADIO_BUTTON)
  {
    // Draw the radio "hole"...
    draw_box(FL_ROUND_DOWN_BOX, x(), y() + offset, size, size, FL_WHITE);

    // Then the check...
    if (value())
    {
      fl_color(FL_BLACK);
      fl_pie(x() + 4, y() + offset + 4, size - 8, size - 8, 0.0, 360.0);
    }
  }
  else
  {
    // Draw the check "box"...
    draw_box(FL_DOWN_BOX, x(), y() + offset, size, size, FL_WHITE);

    // Then the check...
    if (value())
    {
      fl_color(FL_BLACK);
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
// End of "$Id: CheckButton.cxx,v 1.1 1999/02/02 03:55:32 mike Exp $".
//
