//
// "$Id: Progress.h,v 1.2 2000/09/15 02:42:40 mike Exp $"
//
//   Progress bar widget definitions.
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

#ifndef _GUI_PROGRESS_H_
#  define _GUI_PROGRESS_H_

//
// Include necessary headers.
//

#include <FL/Fl_Widget.H>


//
// Progress class...
//

class Progress : public Fl_Widget
{
    float	value_,
		minimum_,
		maximum_;

protected:
    virtual void draw();

public:
    Progress(int x, int y, int w, int h, const char *l = 0);

    void	maximum(float v) { maximum_ = v; redraw(); }
    float	maximum() const { return (maximum_); }

    void	minimum(float v) { minimum_ = v; redraw(); }
    float	minimum() const { return (minimum_); }

    void	value(float v) { value_ = v; redraw(); }
    float	value() const { return (value_); }
};

#endif // !_GUI_PROGRESS_H_

//
// End of "$Id: Progress.h,v 1.2 2000/09/15 02:42:40 mike Exp $".
//
