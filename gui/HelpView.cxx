//
// "$Id: HelpView.cxx,v 1.3 1999/09/24 16:06:52 mike Exp $"
//
//   Help Viewer routines for the Common UNIX Printing System (CUPS).
//
//   Copyright 1997-1999 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outblockd in the file
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cups/string.h>


static void
scrollbar_callback(Fl_Widget *s, void *)
{
  ((HelpView *)(s->parent()))->topline(int(((Fl_Scrollbar*)s)->value()));
}


HelpBlock *
HelpView::add_block(const char *s, int x, int y, int w, int h, int a)
{
  HelpBlock	*temp;


  if (nblocks_ >= ablocks_)
  {
    ablocks_ += 16;

    if (ablocks_ == 16)
      blocks_ = (HelpBlock *)malloc(sizeof(HelpBlock) * ablocks_);
    else
      blocks_ = (HelpBlock *)realloc(blocks_, sizeof(HelpBlock) * ablocks_);
  }

  temp = blocks_ + nblocks_;
  temp->start = s;
  temp->x     = x;
  temp->y     = y;
  temp->w     = w;
  temp->h     = h;
  temp->align = a;

  nblocks_ ++;

  return (temp);
}


//
// 'HelpView::add_link()' - Add a new link to the list.
//

void
HelpView::add_link(const char *n, int xx, int yy, int ww, int hh)
{
}


//
// 'HelpView::add_target()' - Add a new target to the list.
//

void
HelpView::add_target(const char *n, int yy)
{
}


//
// 'HelpView::compare_targets()' - Compare two targets.
//

int						// O - Result of comparison
HelpView::compare_targets(const HelpTarget *t0,	// I - First target
                          const HelpTarget *t1)	// I - Second target
{
  return (strcasecmp(t0->name, t1->name));
}


void
HelpView::draw()
{
  int		i;
  const HelpBlock *block;
  const char	*ptr,
		*attrs;
  char		*s,
		buf[1024],
		attr[1024];
  int		xx, yy, ww, hh;
  uchar		font, size;
  int		align, head, pre, needspace;
  Fl_Boxtype	b = box() ? box() : FL_DOWN_BOX;


  if (scrollbar_.visible())
  {
    draw_child(scrollbar_);
    draw_box(b, x(), y(), w() - 20, h(), color());
  }
  else
    draw_box(b, x(), y(), w(), h(), color());

  if (!value_)
    return;

  fl_push_clip(x() + 4, y() + 4, w() - 28, h() - 8);
  fl_color(FL_BLACK);

  for (i = 0, block = blocks_; i < nblocks_ && (block->y - topline_) < h(); i ++, block ++)
    if ((block->y + block->h) >= topline_)
    {
      xx        = block->x;
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
      align     = block->align;
      needspace = 0;

      initfont(font, size);

      for (ptr = block->start, s = buf; ptr < block->end;)
      {
	if ((*ptr == '<' || isspace(*ptr)) && s > buf)
	{
	  if (!head && !pre)
	  {
            // Check width...
            *s = '\0';
            s  = buf;
            ww = fl_width(buf);

            if (needspace && xx > block->x)
	      xx += fl_width(' ');

            if ((xx + ww) > block->w)
	    {
	      xx = block->x;
	      yy += hh;
	      hh = 0;
	    }

            fl_draw(buf, xx + x(), yy + y());

            xx += ww;
	    if ((size + 2) > hh)
	      hh = size + 2;

	    needspace = 0;
	  }
	  else if (pre)
	  {
	    while (isspace(*ptr))
	    {
	      if (*ptr == '\n')
	      {
	        *s = '\0';
                s = buf;

                fl_draw(buf, xx + x(), yy + y());

        	xx = block->x;
		yy += hh;
		hh = size + 2;
	      }
	      else if (*ptr == '\t')
	      {
		// Do tabs every 8 columns...
		while (((s - buf) & 7))
	          *s++ = ' ';
	      }
	      else
	        *s++ = ' ';

              if ((size + 2) > hh)
	        hh = size + 2;

              ptr ++;
	    }

            if (s > buf)
	    {
	      *s = '\0';
	      s = buf;

              fl_draw(buf, xx + x(), yy + y());
              xx += fl_width(buf);
	    }

	    needspace = 0;
	  }
	  else
	  {
            s = buf;

	    while (isspace(*ptr))
              ptr ++;
	  }
	}

	if (*ptr == '<')
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
	  else if (strcasecmp(buf, "BR") == 0)
	  {
            xx       = block->x;
            yy       += hh;
	    hh       = 0;
	  }
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

	    pushfont(font, size);
	  }
	  else if (strcasecmp(buf, "A") == 0)
	    fl_color(FL_BLUE);
	  else if (strcasecmp(buf, "/A") == 0)
	    fl_color(FL_BLACK);
	  else if (strcasecmp(buf, "B") == 0)
	    pushfont(font |= FL_BOLD, size);
	  else if (strcasecmp(buf, "TD") == 0 ||
	           strcasecmp(buf, "TH") == 0)
          {
	    if (tolower(buf[1]) == 'h')
	      pushfont(font |= FL_BOLD, size);
	    else
	      pushfont(font = textfont_, size);

            fl_rect(block->x + x() - 4,
	            block->y - topline_ + y() - size - 3,
		    block->w - block->x + 7, block->h + 7);
	  }
	  else if (strcasecmp(buf, "I") == 0)
	    pushfont(font |= FL_ITALIC, size);
	  else if (strcasecmp(buf, "CODE") == 0)
	    pushfont(font = FL_COURIER, size);
	  else if (strcasecmp(buf, "KBD") == 0)
	    pushfont(font = FL_COURIER_BOLD, size);
	  else if (strcasecmp(buf, "VAR") == 0)
	    pushfont(font = FL_COURIER_ITALIC, size);
	  else if (strcasecmp(buf, "/HEAD") == 0)
            head = 0;
	  else if (strcasecmp(buf, "/H1") == 0 ||
		   strcasecmp(buf, "/H2") == 0 ||
		   strcasecmp(buf, "/H3") == 0 ||
		   strcasecmp(buf, "/H4") == 0 ||
		   strcasecmp(buf, "/H5") == 0 ||
		   strcasecmp(buf, "/H6") == 0 ||
		   strcasecmp(buf, "/B") == 0 ||
		   strcasecmp(buf, "/I") == 0 ||
		   strcasecmp(buf, "/CODE") == 0 ||
		   strcasecmp(buf, "/KBD") == 0 ||
		   strcasecmp(buf, "/VAR") == 0)
	    popfont(font, size);
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    popfont(font, size);
	    pre = 0;
	  }
	}
	else if (*ptr == '\n' && pre)
	{
	  *s = '\0';
	  s = buf;

          fl_draw(buf, xx + x(), yy + y());

	  xx        = block->x;
	  yy        += hh;
	  hh        = size + 2;
	  needspace = 0;

	  ptr ++;
	}
	else if (isspace(*ptr))
	{
	  if (pre)
	  {
	    if (*ptr == ' ')
	      *s++ = ' ';
	    else
	    {
	      // Do tabs every 8 columns...
	      while (((s - buf) & 7))
	        *s++ = ' ';
            }
	  }

          ptr ++;
	  needspace = 1;
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

          if ((size + 2) > hh)
	    hh = size + 2;
	}
	else
	{
	  *s++ = *ptr++;

          if ((size + 2) > hh)
	    hh = size + 2;
        }
      }

      *s = '\0';

      if (s > buf && !pre && !head)
      {
	ww = fl_width(buf);

        if (needspace && xx > block->x)
	  xx += fl_width(' ');

	if ((xx + ww) > block->w)
	{
	  xx = block->x;
	  yy += hh;
	  hh = 0;
	}
      }

      if (s > buf && !head)
        fl_draw(buf, xx + x(), yy + y());
    }

  fl_pop_clip();
}


//
// 'HelpView::format()' - Format the help text.
//

void
HelpView::format()
{
  HelpBlock	*block,
		*row,
		*cell;
  const char	*ptr,
		*start,
		*attrs;
  char		*s,
		buf[1024],
		attr[1024];
  int		xx, yy, ww, hh;
  uchar		font, size;
  int		align,
		head,
		pre,
		needspace;


  nblocks_  = 0;
  nlinks_   = 0;
  ntargets_ = 0;
  size_     = 0;

  if (!value_)
    return;

  initfont(font, size);

  xx        = 4;
  yy        = size + 2;
  hh        = 0;
  block     = add_block(value_, xx, yy, w() - 24, 0, 1);
  row       = NULL;
  head      = 0;
  pre       = 0;
  align     = 1;
  needspace = 0;

  for (ptr = value_, s = buf; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf)
    {
      if (!head && !pre)
      {
        // Check width...
        *s = '\0';
        ww = fl_width(buf);

        if (needspace && xx > block->x)
	  xx += fl_width(' ');

        if ((xx + ww) > block->w)
	{
	  xx       = block->x;
	  yy       += hh;
	  block->h += hh;
	  hh       = 0;
	}

	xx += ww;
	if ((size + 2) > hh)
	  hh = size + 2;

	needspace = 0;
      }
      else if (pre)
      {
        // Handle preformatted text...
	while (isspace(*ptr))
	{
	  if (*ptr == '\n')
	  {
            xx = block->x;
	    yy += hh;
	    block->h += hh;
	    hh = size + 2;
	  }

          if ((size + 2) > hh)
	    hh = size + 2;

          ptr ++;
	}

	needspace = 0;
      }
      else
      {
        // Handle normal text or stuff in the <HEAD> section...
	while (isspace(*ptr))
          ptr ++;
      }

      s = buf;
    }

    if (*ptr == '<')
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
      else if (strcasecmp(buf, "BR") == 0)
      {
        xx       = block->x;
	block->h += hh;
        yy       += hh;
	hh       = 0;
      }
      else if (strcasecmp(buf, "P") == 0 ||
               strcasecmp(buf, "H1") == 0 ||
	       strcasecmp(buf, "H2") == 0 ||
	       strcasecmp(buf, "H3") == 0 ||
	       strcasecmp(buf, "H4") == 0 ||
	       strcasecmp(buf, "H5") == 0 ||
	       strcasecmp(buf, "H6") == 0 ||
	       strcasecmp(buf, "UL") == 0 ||
	       strcasecmp(buf, "OL") == 0 ||
	       strcasecmp(buf, "DL") == 0 ||
	       strcasecmp(buf, "LI") == 0 ||
	       strcasecmp(buf, "DD") == 0 ||
	       strcasecmp(buf, "DT") == 0 ||
	       strcasecmp(buf, "PRE") == 0 ||
	       strcasecmp(buf, "TABLE") == 0)
      {
        block->end = start;

        xx       = block->x;
        block->h += hh;

        if (!block->h)
	{
	  nblocks_ --;
	  block --;
	}

        if (strcasecmp(buf, "UL") == 0 ||
	    strcasecmp(buf, "OL") == 0 ||
	    strcasecmp(buf, "DL") == 0)
	{
	  xx += 4 * size;
	  block->h += size + 2;
	}
        else if (strcasecmp(buf, "TABLE") == 0)
	  block->h += size + 2;

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

	pushfont(font, size);

        yy = block->y + block->h;

        if (tolower(buf[0]) == 'h' ||
	    strcasecmp(buf, "DD") == 0 ||
	    strcasecmp(buf, "DT") == 0 ||
	    strcasecmp(buf, "UL") == 0 ||
	    strcasecmp(buf, "OL") == 0 ||
	    strcasecmp(buf, "P") == 0)
          yy += size + 2;

        block     = add_block(start, xx, yy, w() - 24, 0, align);
	needspace = 0;
        hh        = 0;
      }
      else if (strcasecmp(buf, "TR") == 0)
      {
        block->end = start;

        xx       = block->x;
        block->h += hh;

        if (!block->h)
	{
	  nblocks_ --;
	  block --;
	}

        if (row)
	{
          yy = row->y + row->h;

	  for (cell = row + 1; cell <= block; cell ++)
	    if ((cell->y + cell->h) > yy)
	      yy = cell->y + cell->h;

          block->h = yy - block->y + 2;

	  for (cell = row + 1; cell < block; cell ++)
	    cell->h = block->h;
	}

	yy        = block->y + block->h - 4;
	hh        = 0;
        block     = row = add_block(start, xx, yy, w() - 24, 0, align);
	needspace = 0;
      }
      else if ((strcasecmp(buf, "TD") == 0 ||
                strcasecmp(buf, "TH") == 0) && row)
      {
        block->end = start;
	block->h   += hh;

        if (strcasecmp(buf, "TH") == 0)
	  font = textfont_ | FL_BOLD;
	else
	  font = textfont_;

        size = textsize_;

        if (strncasecmp(block->start, "<TR", 3) == 0)
          xx = block->x + size + 3;
	else
          xx = block->x + 14 * size + 6;

        if (block->end == block->start)
	{
	  nblocks_ --;
	  block --;
	}

	pushfont(font, size);

	yy        = row->y;
	hh        = 0;
        block     = add_block(start, xx, yy, xx + 14 * size, 0, align);
	needspace = 0;
      }
      else if ((strcasecmp(buf, "/TD") == 0 ||
                strcasecmp(buf, "/TH") == 0) && row)
        popfont(font, size);
      else if (strcasecmp(buf, "/TR") == 0 && row != NULL)
      {
        block->end = start;
	block->h   += hh;

        xx = row->x;

        if (block->end == block->start)
	{
	  nblocks_ --;
	  block --;
	}

        yy = row->y + row->h;

	for (cell = row + 1; cell <= block; cell ++)
	  if ((cell->y + cell->h) > yy)
	    yy = cell->y + cell->h;

        block->h = yy - block->y + 2;

	for (cell = row + 1; cell < block; cell ++)
	  cell->h = block->h;

	yy        = block->y + block->h - 4;
        block     = add_block(start, xx, yy, w() - 24, 0, align);
	needspace = 0;
	row       = NULL;
      }
      else if (strcasecmp(buf, "B") == 0)
	pushfont(font |= FL_BOLD, size);
      else if (strcasecmp(buf, "I") == 0)
	pushfont(font |= FL_ITALIC, size);
      else if (strcasecmp(buf, "CODE") == 0)
	pushfont(font = FL_COURIER, size);
      else if (strcasecmp(buf, "KBD") == 0)
	pushfont(font = FL_COURIER_BOLD, size);
      else if (strcasecmp(buf, "VAR") == 0)
	pushfont(font = FL_COURIER_ITALIC, size);
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
	       strcasecmp(buf, "/DL") == 0 ||
	       strcasecmp(buf, "/TABLE") == 0)
      {
        xx = block->x;

        block->end = ptr;

        if (strcasecmp(buf, "/UL") == 0 ||
	    strcasecmp(buf, "/OL") == 0 ||
	    strcasecmp(buf, "/DL") == 0)
	{
	  xx       -= 4 * size;
	  block->h += size + 2;
	}
	else if (strcasecmp(buf, "/TABLE") == 0)
	  block->h += size + 2;
	else if (strcasecmp(buf, "/PRE") == 0)
	{
	  pre = 0;
	  hh  = 0;
	}

        initfont(font, size);

        while (isspace(*ptr))
	  ptr ++;

        block->h += hh;
        yy       += hh;

        if (tolower(buf[2]) == 'l')
          yy += size + 2;

        block     = add_block(ptr, xx, yy, w() - 24, 0, align);
	needspace = 0;
	hh        = 0;
      }
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, size);
    }
    else if (*ptr == '\n' && pre)
    {
      xx        = block->x;
      yy        += hh;
      block->h  += hh;
      needspace = 0;
      ptr ++;
    }
    else if (isspace(*ptr))
    {
      needspace = 1;

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
        *s++ = '\240';
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

      if ((size + 2) > hh)
        hh = size + 2;
    }
    else
    {
      *s++ = *ptr++;

      if ((size + 2) > hh)
        hh = size + 2;
    }
  }

  if (s > buf && !pre && !head)
  {
    *s = '\0';
    ww = fl_width(buf);

    if (needspace && xx > block->x)
      xx += fl_width(' ');

    if ((xx + ww) > block->w)
    {
      yy       += hh;
      block->h += hh;
      hh       = 0;
    }
  }

  block->end = ptr;
  size_      = yy + hh;

  if (size_ < (h() - 8))
    scrollbar_.hide();
  else
    scrollbar_.show();

  topline(topline_);
}


//
// 'HelpView::get_attr()' - Get an attribute value from the string.
//

const char *				// O - Pointer to buf or NULL
HelpView::get_attr(const char *p,	// I - Pointer to start of attributes
                   const char *n,	// I - Name of attribute
		   char       *buf,	// O - Buffer for attribute value
		   int        bufsize)	// I - Size of buffer
{
  return (NULL);
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

HelpView::HelpView(int xx, int yy, int ww, int hh, const char *l)
    : Fl_Group(xx, yy, ww, hh, l),
      scrollbar_(xx + ww - 20, yy, 20, hh)
{
  value_     = NULL;

  ablocks_    = 0;
  nblocks_    = 0;
  blocks_     = (HelpBlock *)0;

  alinks_    = 0;
  nlinks_    = 0;
  links_     = (HelpLink *)0;

  atargets_  = 0;
  ntargets_  = 0;
  targets_   = (HelpTarget *)0;

  color(FL_WHITE);

  textfont_ = FL_TIMES;
  textsize_ = 12;

  topline_  = 0;
  size_ = 0;

  scrollbar_.value(0, hh, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback(scrollbar_callback);

  end();
}


//
// 'HelpView::load()' - Load the specified file.
//

int				// O - 0 on success, -1 on error
HelpView::load(const char *f)	// I - Filename to load (may also have target)
{
  FILE	*fp;		// File to read from
  long	len;		// Length of file
  char	*target;	// Target in file


  if ((fp = fopen(f, "r")) == NULL)
    return (-1);

  strcpy(filename_, f);
  if ((target = strrchr(filename_, '#')) != NULL)
    *target++ = '\0';

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  rewind(fp);

  if (value_ != NULL)
    free((void *)value_);

  value_ = (const char *)calloc(len + 1, 1);
  fread((void *)value_, len, 1, fp);
  fclose(fp);

  format();

  if (target)
    topline(target);
  else
    topline(0);

  return (0);
}


//
// 'HelpView::resize()' - Resize the help widget.
//

void
HelpView::resize(int x, int y, int w, int h)
{
  Fl_Widget::resize(x, y, w, h);
  scrollbar_.resize(x + w - 20, y, 20, h);

  format();
}


//
// 'HelpView::topline()' - Set the top line to the named target.
//

void
HelpView::topline(const char *n)	// I - Target name
{
}


//
// 'HelpView::topline()' - Set the top line by number.
//

void
HelpView::topline(int t)	// I - Top line number
{
  int	hh;			// Height of scroller


  if (!value_)
    return;

  if (size_ < (h() - 8) || t < 0)
    t = 0;
  else if (t > (size_ - h() - 8))
    t = size_ - h() - 8;

  topline_ = t;

  hh = h() - size_ / h();
  if (hh < 20)
    hh = 20;

  scrollbar_.value(topline_, hh, 0, size_ - h());

  redraw();
}


//
// 'HelpView::value()' - Set the help text directly.
//

void
HelpView::value(const char *v)	// I - Text to view
{
  if (!v)
    return;

  if (value_ != NULL)
    free((void *)value_);

  value_   = strdup(v);
  topline_ = 0;

  format();
}


//
// End of "$Id: HelpView.cxx,v 1.3 1999/09/24 16:06:52 mike Exp $".
//
