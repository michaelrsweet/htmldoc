//
// "$Id: Progress.cxx,v 1.1 2000/03/18 16:08:56 mike Exp $"
//
//   Progress bar widget routines.
//
//   Copyright 2000 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "LICENSE.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: Licensing Information
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
#include "Progress.h"
#include <FL/fl_draw.H>


//
// Progress is a progress bar widget based off Fl_Widget that shows a
// standard progress bar...
//


//
// 'Progress::draw()' - Draw the check button.
//

void Progress::draw()
{
  int	progress;	// Size of progress bar...


  // Draw the box...
  draw_box(box(), x(), y(), w(), h(), color());

  // Draw the progress bar...
  if (maximum_ > minimum_)
    progress = (int)((w() - 4) * (value_ - minimum_) / (maximum_ - minimum_) + 0.5f);
  else
    progress = 0;

  if (progress > 0)
  {
    fl_clip(x() + 2, y() + 2, w() - 4, h() - 4);

    fl_color(active_r() ? color2() : inactive(color2()));
    fl_polygon(x() + 2, y() + 2,
               x() + 2, y() + h() - 2,
	       x() + 3 + progress - h() / 4, y() + h() - 2,
               x() + 1 + progress + h() / 4, y() + 2);

    fl_pop_clip();
  }

  // Finally, the label...
  draw_label(x() + 2, y() + 2, w() - 4, h() - 4);
}


//
// 'Progress::Progress()' - Construct a Progress widget.
//

Progress::Progress(int x, int y, int w, int h, const char* l)
: Fl_Widget(x, y, w, h, l)
{
  align(FL_ALIGN_INSIDE);
  box(FL_DOWN_BOX);
  color(FL_WHITE, FL_YELLOW);
  minimum(0.0f);
  maximum(100.0f);
  value(0.0f);
}


//
// End of "$Id: Progress.cxx,v 1.1 2000/03/18 16:08:56 mike Exp $".
//
