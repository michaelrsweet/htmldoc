//
// "$Id: HelpView.cxx,v 1.1 1999/09/22 16:51:23 mike Exp $"
//
//   Help Viewer routines for the Common UNIX Printing System (CUPS).
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
//

//
// Include necessary header files...
//

#include "HelpView.h"
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cups/string.h>


static void
scrollbar_callback(Fl_Widget *s, void *)
{
  ((HelpView *)(s->parent()))->topline(int(((Fl_Scrollbar*)s)->value()));
}


HelpLine *
HelpView::add_line(const char *s, int x, int y, int w, int h, int a)
{
  HelpLine	*temp;


  if (nlines_ >= alines_)
  {
    alines_ += 16;

    if (alines_ == 16)
      lines_ = (HelpLine *)malloc(sizeof(HelpLine) * alines_);
    else
      lines_ = (HelpLine *)realloc(lines_, sizeof(HelpLine) * alines_);
  }

  temp = lines_ + nlines_;
  temp->start = s;
  temp->x     = x;
  temp->y     = y;
  temp->w     = w;
  temp->h     = h;
  temp->align = a;

  nlines_ ++;

  return (temp);
}


void
HelpView::add_link(const char *n, int x, int y, int w, int h)
{
}


void
HelpView::add_target(const char *n, int y)
{
}


void
HelpView::draw()
{
  int		i;
  const HelpLine *line;
  const char	*ptr,
		*attrs;
  char		*s,
		buf[1024],
		attr[1024];
  int		xx, yy, ww;
  uchar		font, size;
  int		align, head, pre;
  Fl_Boxtype	b = box() ? box() : FL_DOWN_BOX;


  draw_child(scrollbar_);
  draw_box(b, x(), y(), w() - 20, h(), color());

  if (!value_)
    return;

  fl_push_clip(x() + 4, y() + 4, w() - 28, h() - 8);
  fl_color(FL_BLACK);
  fl_font(font = textfont_, size = textsize_);

  for (i = 0, line = lines_; i < nlines_ && (line->y - top_) < h(); i ++, line ++)
    if ((line->y + line->h) >= top_)
    {
      xx    = line->x;
      yy    = line->y - top_;
      pre   = 0;
      head  = 0;
      align = line->align;

      for (ptr = line->start, s = buf; ptr < line->end;)
      {
	if ((*ptr == '<' || isspace(*ptr)) && s > buf)
	{
	  if (!head && !pre)
	  {
            // Check width...
            if (isspace(*ptr))
              *s++ = ' ';

            *s = '\0';
            s  = buf;
            ww = fl_width(buf);

            if ((xx + ww) > line->w)
	    {
	      xx = line->x;
	      yy += size + 2;
	    }

            fl_draw(buf, xx + x(), yy + y());

            xx += ww;
	  }
	  else if (pre)
	  {
	    while (isspace(*ptr))
	    {
	      if (*ptr == '\n')
	      {
                s = buf;
	        *s = '\0';

                fl_draw(buf, xx + x(), yy + y());

        	xx = line->x;
		yy += size + 2;
	      }
	      else
	        *s++ = ' ';

              ptr ++;
	    }

            if (s > buf)
	    {
	      *s = '\0';
	      s = buf;

              fl_draw(buf, xx + x(), yy + y());

              xx += fl_width(buf);
	    }
	  }
	  else
	  {
            s = buf;

	    while (isspace(*ptr))
              ptr ++;
	  }
	}
	else if (*ptr == '<')
	{
	  ptr ++;
	  while (*ptr && *ptr != '>' && !isspace(*ptr))
            *s++ = *ptr++;

	  *s = '\0';
	  s = buf;

	  attrs = ptr;
	  while (*ptr && *ptr != '>')
            ptr ++;

	  if (*ptr == '>')
            ptr ++;

	  if (strcasecmp(buf, "HEAD") == 0)
            head = 1;
	  else if (strcasecmp(buf, "P") == 0 ||
        	   strcasecmp(buf, "H1") == 0 ||
		   strcasecmp(buf, "H2") == 0 ||
		   strcasecmp(buf, "H3") == 0 ||
		   strcasecmp(buf, "H4") == 0 ||
		   strcasecmp(buf, "H5") == 0 ||
		   strcasecmp(buf, "H6") == 0 ||
		   strcasecmp(buf, "BR") == 0 ||
		   strcasecmp(buf, "UL") == 0 ||
		   strcasecmp(buf, "OL") == 0 ||
		   strcasecmp(buf, "DL") == 0 ||
		   strcasecmp(buf, "LI") == 0 ||
		   strcasecmp(buf, "DD") == 0 ||
		   strcasecmp(buf, "DT") == 0 ||
		   strcasecmp(buf, "PRE") == 0)
	  {
            if (tolower(buf[0]) == 'h')
	    {
	      font = FL_HELVETICA_BOLD;
	      size = textsize_ + '7' - buf[1];
	    }
	    else if (strcasecmp(buf, "DT") == 0)
	    {
	      font = textfont_ | FL_ITALIC;
	      size = textsize_;
	    }
	    else if (strcasecmp(buf, "PRE") == 0)
	    {
	      font = FL_COURIER;
	      size = textsize_;
	      pre  = 1;
	    }
	    else if (strcasecmp(buf, "BR") != 0)
	    {
	      font = textfont_;
	      size = textsize_;
	    }

            if (strcasecmp(buf, "LI") == 0)
	    {
	      fl_font(FL_SYMBOL, size);
	      fl_draw("\267", xx - size + x(), yy + y());
	    }

	    fl_font(font, size);
	  }
	  else if (strcasecmp(buf, "A") == 0)
	    fl_color(FL_BLUE);
	  else if (strcasecmp(buf, "/A") == 0)
	    fl_color(FL_BLACK);
	  else if (strcasecmp(buf, "B") == 0)
	    fl_font(font |= FL_BOLD, size);
	  else if (strcasecmp(buf, "I") == 0)
	    fl_font(font |= FL_ITALIC, size);
	  else if (strcasecmp(buf, "CODE") == 0)
	    fl_font(font = FL_COURIER, size);
	  else if (strcasecmp(buf, "KBD") == 0)
	    fl_font(font = FL_COURIER_BOLD, size);
	  else if (strcasecmp(buf, "VAR") == 0)
	    fl_font(font = FL_COURIER_ITALIC, size);
	  else if (strcasecmp(buf, "/HEAD") == 0)
            head = 0;
	  else if (strcasecmp(buf, "/H1") == 0 ||
		   strcasecmp(buf, "/H2") == 0 ||
		   strcasecmp(buf, "/H3") == 0 ||
		   strcasecmp(buf, "/H4") == 0 ||
		   strcasecmp(buf, "/H5") == 0 ||
		   strcasecmp(buf, "/H6") == 0 ||
		   strcasecmp(buf, "/UL") == 0 ||
		   strcasecmp(buf, "/OL") == 0 ||
		   strcasecmp(buf, "/DL") == 0)
	  {
	    font = textfont_;
	    size = textsize_;

	    fl_font(font, size);
	  }
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    font = textfont_;
	    size = textsize_;

	    fl_font(font, size);
	    pre = 0;
	  }
	  else if (strcasecmp(buf, "/B") == 0 ||
		   strcasecmp(buf, "/I") == 0 ||
		   strcasecmp(buf, "/CODE") == 0 ||
		   strcasecmp(buf, "/KBD") == 0 ||
		   strcasecmp(buf, "/VAR") == 0)
	  {
	    font = textfont_;
	    size = textsize_;

	    fl_font(font, size);
	  }
	}
	else if (*ptr == '\n' && pre)
	{
	  *s = '\0';
	  s = buf;

          fl_draw(buf, xx + x(), yy + y());
	  xx = line->x;
	  yy += size + 2;
	  ptr ++;
	}
	else if (isspace(*ptr))
	{
	  if (pre)
	    *s++ = ' ';

          ptr ++;
	}
	else if (*ptr == '&')
	{
	  ptr ++;

	  if (strncasecmp(ptr, "amp;", 4) == 0)
	  {
            *s++ = '&';
	    ptr += 4;
	  }
	  else if (strncasecmp(ptr, "lt;", 3) == 0)
	  {
            *s++ = '<';
	    ptr += 3;
	  }
	  else if (strncasecmp(ptr, "gt;", 3) == 0)
	  {
            *s++ = '>';
	    ptr += 3;
	  }
	  else if (strncasecmp(ptr, "nbsp;", 5) == 0)
	  {
            *s++ = ' ';
	    ptr += 5;
	  }
	  else if (strncasecmp(ptr, "copy;", 5) == 0)
	  {
            *s++ = '\251';
	    ptr += 5;
	  }
	  else if (strncasecmp(ptr, "reg;", 4) == 0)
	  {
            *s++ = '\256';
	    ptr += 4;
	  }
	  else if (strncasecmp(ptr, "quot;", 5) == 0)
	  {
            *s++ = '\"';
	    ptr += 5;
	  }
	}
	else
	  *s++ = *ptr++;
      }

      *s = '\0';

      if (s > buf && !pre && !head)
      {
	ww = fl_width(buf);

	if ((xx + ww) > line->w)
	  yy += size + 2;
      }

      if (s > buf && !head)
        fl_draw(buf, xx + x(), yy + y());
    }

  fl_pop_clip();
}


void
HelpView::format()
{
  HelpLine	*line;
  const char	*ptr,
		*start,
		*attrs;
  char		*s,
		buf[1024],
		attr[1024];
  int		xx, yy, ww;
  uchar		font, size;
  int		align, head, pre;


  nlines_ = 0;
  size_   = 0;

  if (!value_)
    return;

  fl_font(font = textfont_, size = textsize_);

  xx    = 4;
  yy    = size + 2;
  line  = add_line(value_, xx, yy, w() - 24, size + 2, 1);
  head  = 0;
  pre   = 0;
  align = 1;

  for (ptr = value_, s = buf; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf)
    {
      if (!head && !pre)
      {
        // Check width...
        if (isspace(*ptr))
          *s++ = ' ';

        *s = '\0';
        ww = fl_width(buf);

        if ((xx + ww) > line->w)
	{
	  xx = line->x;
	  yy += size + 2;
	  line->h += size + 2;
	}
	else
	  xx += ww;
      }
      else if (pre)
      {
	while (isspace(*ptr))
	{
	  if (*ptr == '\n')
	  {
            xx = line->x;
	    yy += size + 2;
	    line->h += size + 2;
	  }

          ptr ++;
	}
      }
      else
      {
	while (isspace(*ptr))
          ptr ++;
      }

      s = buf;
    }
    else if (*ptr == '<')
    {
      start = ptr;
      ptr ++;
      while (*ptr && *ptr != '>' && !isspace(*ptr))
        *s++ = *ptr++;

      *s = '\0';
      s = buf;

      attrs = ptr;
      while (*ptr && *ptr != '>')
        ptr ++;

      if (*ptr == '>')
        ptr ++;

      if (strcasecmp(buf, "HEAD") == 0)
        head = 1;
      else if (strcasecmp(buf, "P") == 0 ||
               strcasecmp(buf, "H1") == 0 ||
	       strcasecmp(buf, "H2") == 0 ||
	       strcasecmp(buf, "H3") == 0 ||
	       strcasecmp(buf, "H4") == 0 ||
	       strcasecmp(buf, "H5") == 0 ||
	       strcasecmp(buf, "H6") == 0 ||
	       strcasecmp(buf, "BR") == 0 ||
	       strcasecmp(buf, "UL") == 0 ||
	       strcasecmp(buf, "OL") == 0 ||
	       strcasecmp(buf, "DL") == 0 ||
	       strcasecmp(buf, "LI") == 0 ||
	       strcasecmp(buf, "DD") == 0 ||
	       strcasecmp(buf, "DT") == 0 ||
	       strcasecmp(buf, "PRE") == 0)
      {
        line->end = start;

        xx = line->x;

        if (strcasecmp(buf, "UL") == 0 ||
	    strcasecmp(buf, "OL") == 0 ||
	    strcasecmp(buf, "DL") == 0)
	  xx += 4 * size;

        if (tolower(buf[0]) == 'h')
	{
	  font = FL_HELVETICA_BOLD;
	  size = textsize_ + '7' - buf[1];
	}
	else if (strcasecmp(buf, "DT") == 0)
	{
	  font = textfont_ | FL_ITALIC;
	  size = textsize_;
	}
	else if (strcasecmp(buf, "PRE") == 0)
	{
	  font = FL_COURIER;
	  size = textsize_;
	  pre  = 1;
	}
	else if (strcasecmp(buf, "BR") != 0)
	{
	  font = textfont_;
	  size = textsize_;
	}

	fl_font(font, size);

        if (line->start == start)
	{
	  line->x     = xx;
	  line->w     = w() - 20 - xx;
	  line->h     = size + 2;
	  line->align = align;
	}
	else
	{
	  yy = line->y + line->h;

          if (tolower(buf[0]) == 'b' ||
	      tolower(buf[0]) == 'h' ||
	      tolower(buf[0]) == 'p')
	    yy += size + 2;

          line = add_line(start, xx, yy, w() - 20 - xx, size + 2, align);
        }
      }
      else if (strcasecmp(buf, "B") == 0)
	fl_font(font |= FL_BOLD, size);
      else if (strcasecmp(buf, "I") == 0)
	fl_font(font |= FL_ITALIC, size);
      else if (strcasecmp(buf, "CODE") == 0)
	fl_font(font = FL_COURIER, size);
      else if (strcasecmp(buf, "KBD") == 0)
	fl_font(font = FL_COURIER_BOLD, size);
      else if (strcasecmp(buf, "VAR") == 0)
	fl_font(font = FL_COURIER_ITALIC, size);
      else if (strcasecmp(buf, "/HEAD") == 0)
        head = 0;
      else if (strcasecmp(buf, "/P") == 0 ||
	       strcasecmp(buf, "/H1") == 0 ||
	       strcasecmp(buf, "/H2") == 0 ||
	       strcasecmp(buf, "/H3") == 0 ||
	       strcasecmp(buf, "/H4") == 0 ||
	       strcasecmp(buf, "/H5") == 0 ||
	       strcasecmp(buf, "/H6") == 0 ||
	       strcasecmp(buf, "/PRE") == 0 ||
	       strcasecmp(buf, "/UL") == 0 ||
	       strcasecmp(buf, "/OL") == 0 ||
	       strcasecmp(buf, "/DL") == 0)
      {
        xx = line->x;

        line->end = ptr;

        if (strcasecmp(buf, "/UL") == 0 ||
	    strcasecmp(buf, "/OL") == 0 ||
	    strcasecmp(buf, "/DL") == 0)
	  xx -= 4 * size;
	else
	{
	  if (strcasecmp(buf, "/PRE") == 0)
	    pre = 0;

          yy += textsize_ + 2;
	}

	font = textfont_;
	size = textsize_;

	fl_font(font, size);

        while (isspace(*ptr))
	  ptr ++;

        line = add_line(ptr, xx, yy, w() - 20 - xx, size + 2, align);
      }
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
      {
	font = textfont_;
	size = textsize_;

	fl_font(font, size);
      }
    }
    else if (*ptr == '\n' && pre)
    {
      xx = line->x;
      yy += size + 2;
      line->h += size + 2;
      ptr ++;
    }
    else if (isspace(*ptr))
      ptr ++;
    else if (*ptr == '&')
    {
      ptr ++;

      if (strncasecmp(ptr, "amp;", 4) == 0)
      {
        *s++ = '&';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "lt;", 3) == 0)
      {
        *s++ = '<';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "gt;", 3) == 0)
      {
        *s++ = '>';
	ptr += 3;
      }
      else if (strncasecmp(ptr, "nbsp;", 5) == 0)
      {
        *s++ = ' ';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "copy;", 5) == 0)
      {
        *s++ = '\251';
	ptr += 5;
      }
      else if (strncasecmp(ptr, "reg;", 4) == 0)
      {
        *s++ = '\256';
	ptr += 4;
      }
      else if (strncasecmp(ptr, "quot;", 5) == 0)
      {
        *s++ = '\"';
	ptr += 5;
      }
    }
    else
      *s++ = *ptr++;
  }

  if (s > buf && !pre && !head)
  {
    *s = '\0';
    ww = fl_width(buf);

    if ((xx + ww) > line->w)
    {
      yy += size + 2;
      line->h += size + 2;
    }
  }

  line->end = ptr;
  size_     = yy;

  topline(top_);
}


const char *
HelpView::get_attr(const char *p, const char *n, char *buf, int bufsize)
{
}


int
HelpView::handle(int event)
{
#if 0
  switch (event)
  {
    case FL_KEYBOARD :
        break;
    case FL_BUTTON :
    case FL_MOTION :
        break;
  }
#endif /* 0 */

  // Use the Fl_Input handler...
  return (Fl_Group::handle(event));
}


//
// 'HelpView::HelpView()' - Build a HelpView widget.
//

HelpView::HelpView(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l),
      scrollbar_(x + w - 20, y, 20, h)
{
  value_     = NULL;

  alines_    = 0;
  nlines_    = 0;
  lines_     = (HelpLine *)0;

  alinks_    = 0;
  nlinks_    = 0;
  links_     = (HelpLink *)0;

  atargets_  = 0;
  ntargets_  = 0;
  targets_   = (HelpTarget *)0;

  color(FL_WHITE);

  textfont_ = FL_TIMES;
  textsize_ = 12;

  top_  = 0;
  size_ = 0;

  scrollbar_.value(0, h, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback(scrollbar_callback);

  end();
}


int
HelpView::load(const char *f)
{
  FILE *fp;
  long len;


  if ((fp = fopen(f, "r")) == NULL)
    return (-1);

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  rewind(fp);

  if (value_ != NULL)
    free((void *)value_);

  value_ = (const char *)calloc(len + 1, 1);
  fread((void *)value_, len, 1, fp);
  fclose(fp);

  format();
  redraw();

  return (0);
}


void
HelpView::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);
  scrollbar_.resize(x + w - 20, y, 20, h);

  format();
  redraw();
}


void
topline(const char *n)
{
}


void
HelpView::topline(int t)
{
  int	hh;


  if (!value_)
    return;

  if (t < 0)
    t = 0;
  else if (t > (size_ - h()))
    t = size_ - h();

  top_ = t;

  hh = h() - size_ / h();
  if (hh < 20)
    hh = 20;

  scrollbar_.value(top_, hh, 0, size_ - h());

  redraw();
}


void
HelpView::value(const char *v)
{
  if (value_ != NULL)
    free((void *)value_);

  value_ = strdup(v);

  format();
  redraw();
}


//
// End of "$Id: HelpView.cxx,v 1.1 1999/09/22 16:51:23 mike Exp $".
//
