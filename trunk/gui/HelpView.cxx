//
// "$Id: HelpView.cxx,v 1.22 2000/03/19 23:27:14 mike Exp $"
//
//   Help Viewer widget routines.
//
//   Copyright 1997-2000 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outblockd in the file
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
// Contents:
//
//   HelpView::add_block()       - Add a text block to the list.
//   HelpView::add_link()        - Add a new link to the list.
//   HelpView::add_target()      - Add a new target to the list.
//   HelpView::compare_targets() - Compare two targets.
//   HelpView::do_align()        - Compute the alignment for a line in a block.
//   HelpView::draw()            - Draw the HelpView widget.
//   HelpView::format()          - Format the help text.
//   HelpView::get_align()       - Get an alignment attribute.
//   HelpView::get_attr()        - Get an attribute value from the string.
//   HelpView::get_color()       - Get an alignment attribute.
//   HelpView::handle()          - Handle events in the widget.
//   HelpView::HelpView()        - Build a HelpView widget.
//   HelpView::~HelpView()       - Destroy a HelpView widget.
//   HelpView::load()            - Load the specified file.
//   HelpView::resize()          - Resize the help widget.
//   HelpView::topline()         - Set the top line to the named target.
//   HelpView::topline()         - Set the top line by number.
//   HelpView::value()           - Set the help text directly.
//   scrollbar_callback()        - A callback for the scrollbar.
//

//
// Include necessary header files...
//

#include "HelpView.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#if defined(WIN32)
#  include <io.h>
#  include <direct.h>
#  define strcasecmp(s,t)	stricmp((s), (t))
#  define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#elif defined(__EMX__)
#  define strcasecmp(s,t)	stricmp((s), (t))
#  define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#else
#  include <unistd.h>
#endif // WIN32


//
// Local functions...
//

static void	scrollbar_callback(Fl_Widget *s, void *);


//
// 'HelpView::add_block()' - Add a text block to the list.
//

HelpBlock *				// O - Pointer to new block
HelpView::add_block(const char *s,	// I - Pointer to start of block text
                    int        xx,	// I - X position of block
		    int        yy,	// I - Y position of block
		    int        ww,	// I - Right margin of block
		    int        hh,	// I - Height of block
		    uchar      border)	// I - Draw border?
{
  HelpBlock	*temp;			// New block


  if (nblocks_ >= ablocks_)
  {
    ablocks_ += 16;

    if (ablocks_ == 16)
      blocks_ = (HelpBlock *)malloc(sizeof(HelpBlock) * ablocks_);
    else
      blocks_ = (HelpBlock *)realloc(blocks_, sizeof(HelpBlock) * ablocks_);
  }

  temp = blocks_ + nblocks_;
  temp->start  = s;
  temp->x      = xx;
  temp->y      = yy;
  temp->w      = ww;
  temp->h      = hh;
  temp->border = border;
  nblocks_ ++;

  return (temp);
}


//
// 'HelpView::add_link()' - Add a new link to the list.
//

void
HelpView::add_link(const char *n,	// I - Name of link
                   int        xx,	// I - X position of link
		   int        yy,	// I - Y position of link
		   int        ww,	// I - Width of link text
		   int        hh)	// I - Height of link text
{
  HelpLink	*temp;			// New link
  char		*target;		// Pointer to target name


  if (nlinks_ >= alinks_)
  {
    alinks_ += 16;

    if (alinks_ == 16)
      links_ = (HelpLink *)malloc(sizeof(HelpLink) * alinks_);
    else
      links_ = (HelpLink *)realloc(links_, sizeof(HelpLink) * alinks_);
  }

  temp = links_ + nlinks_;

  temp->x       = xx;
  temp->y       = yy;
  temp->w       = xx + ww;
  temp->h       = yy + hh;

  strncpy(temp->filename, n, sizeof(temp->filename));
  temp->filename[sizeof(temp->filename) - 1] = '\0';

  if ((target = strrchr(temp->filename, '#')) != NULL)
  {
    *target++ = '\0';
    strncpy(temp->name, target, sizeof(temp->name));
    temp->name[sizeof(temp->name) - 1] = '\0';
  }
  else
    temp->name[0] = '\0';

  nlinks_ ++;
}


//
// 'HelpView::add_target()' - Add a new target to the list.
//

void
HelpView::add_target(const char *n,	// I - Name of target
                     int        yy)	// I - Y position of target
{
  HelpTarget	*temp;			// New target


  if (ntargets_ >= atargets_)
  {
    atargets_ += 16;

    if (atargets_ == 16)
      targets_ = (HelpTarget *)malloc(sizeof(HelpTarget) * atargets_);
    else
      targets_ = (HelpTarget *)realloc(targets_, sizeof(HelpTarget) * atargets_);
  }

  temp = targets_ + ntargets_;

  temp->y = yy;
  strncpy(temp->name, n, sizeof(temp->name));
  temp->name[sizeof(temp->name) - 1] = '\0';

  ntargets_ ++;
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


//
// 'HelpView::do_align()' - Compute the alignment for a line in a block.
//

int					// O - New line
HelpView::do_align(HelpBlock *block,	// I - Block to add to
                   int       line,	// I - Current line
		   int       xx,	// I - Current X position
		   int       a,		// I - Current alignment
		   int       &l)	// IO - Starting link
{
  int	offset;				// Alignment offset


  switch (a)
  {
    case RIGHT :	// Right align
	offset = block->w - xx;
	break;
    case CENTER :	// Center
	offset = (block->w - xx) / 2;
	break;
    case LEFT :		// Left align
	offset = 0;
	break;
  }

  block->line[line] = block->x + offset;

  if (line < 31)
    line ++;

  while (l < nlinks_)
  {
    links_[l].x += offset;
    links_[l].w += offset;
    l ++;
  }

  return (line);
}


//
// 'HelpView::draw()' - Draw the HelpView widget.
//

void
HelpView::draw()
{
  int		i;		// Looping var
  const HelpBlock *block;	// Pointer to current block
  const char	*ptr,		// Pointer to text in block
		*attrs;		// Pointer to start of element attributes
  char		*s,		// Pointer into buffer
		buf[1024],	// Text buffer
		attr[1024];	// Attribute buffer
  int		xx, yy, ww, hh;	// Current positions and sizes
  int		line;		// Current line
  uchar		font, size;	// Current font and size
  int		head, pre,	// Flags for text
		needspace;	// Do we need whitespace?
  Fl_Boxtype	b = box() ? box() : FL_DOWN_BOX;
				// Box to draw...
  Fl_Color	tc, c;		// Table/cell background color


  // Draw the scrollbar and box first...
  if (scrollbar_.visible())
  {
    draw_child(scrollbar_);
    draw_box(b, x(), y(), w() - 17, h(), bgcolor_);
  }
  else
    draw_box(b, x(), y(), w(), h(), bgcolor_);

  if (!value_)
    return;

  // Clip the drawing to the inside of the box...
  fl_push_clip(x() + 4, y() + 4, w() - 28, h() - 8);
  fl_color(textcolor_);

  tc = c = bgcolor_;

  // Draw all visible blocks...
  for (i = 0, block = blocks_; i < nblocks_ && (block->y - topline_) < h(); i ++, block ++)
    if ((block->y + block->h) >= topline_)
    {
      line      = 0;
      xx        = block->line[line];
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
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
            ww = (int)fl_width(buf);

            if (needspace && xx > block->x)
	      xx += (int)fl_width(' ');

            if ((xx + ww) > block->w)
	    {
	      if (line < 31)
	        line ++;
	      xx = block->line[line];
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

		if (line < 31)
	          line ++;
		xx = block->line[line];
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
              xx += (int)fl_width(buf);
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
            if (s < (buf + sizeof(buf) - 1))
	      *s++ = *ptr++;
	    else
	      ptr ++;

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
	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "HR") == 0)
	  {
	    fl_line(block->x + x(), yy + y(), block->w + x(),
	            yy + y());

	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += 2 * hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "CENTER") == 0 ||
        	   strcasecmp(buf, "P") == 0 ||
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

            if (strcasecmp(buf, "LI") == 0)
	    {
	      fl_font(FL_SYMBOL, size);
	      fl_draw("\267", xx - size + x(), yy + y());
	    }

	    pushfont(font, size);

            if (c != bgcolor_)
	    {
	      fl_color(c);
              fl_rectf(block->x + x() - 4,
	               block->y - topline_ + y() - size - 3,
		       block->w - block->x + 7, block->h + size - 5);
              fl_color(textcolor_);
	    }
	  }
	  else if (strcasecmp(buf, "A") == 0 &&
	           get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	    fl_color(linkcolor_);
	  else if (strcasecmp(buf, "/A") == 0)
	    fl_color(textcolor_);
	  else if (strcasecmp(buf, "B") == 0)
	    pushfont(font |= FL_BOLD, size);
	  else if (strcasecmp(buf, "TABLE") == 0)
            tc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), bgcolor_);
	  else if (strcasecmp(buf, "TD") == 0 ||
	           strcasecmp(buf, "TH") == 0)
          {
	    if (tolower(buf[1]) == 'h')
	      pushfont(font |= FL_BOLD, size);
	    else
	      pushfont(font = textfont_, size);

            c = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), tc);

            if (c != bgcolor_)
	    {
	      fl_color(c);
              fl_rectf(block->x + x() - 4,
	               block->y - topline_ + y() - size - 3,
		       block->w - block->x + 7, block->h + size - 5);
              fl_color(textcolor_);
	    }

            if (block->border)
              fl_rect(block->x + x() - 4,
	              block->y - topline_ + y() - size - 3,
		      block->w - block->x + 7, block->h + size - 5);
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
	  else if (strcasecmp(buf, "/TABLE") == 0)
	    tc = c = bgcolor_;
	  else if (strcasecmp(buf, "/TD") == 0 ||
	           strcasecmp(buf, "/TH") == 0)
	    c = tc;
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    popfont(font, size);
	    pre = 0;
	  }
	}
	else if (strcasecmp(buf, "IMG") == 0 &&
        	 get_attr(attrs, "ALT", attr, sizeof(attr)) != NULL)
	{
	  // Show alt text...
	  sprintf(buf, "[%s]", attr);
	  ww = (int)fl_width(buf);

          if (needspace && xx > block->x)
	    xx += (int)fl_width(' ');

          if ((xx + ww) > block->w)
	  {
	    if (line < 31)
	      line ++;
	    xx = block->line[line];
	    yy += hh;
	    hh = 0;
	  }

          fl_draw(buf, xx + x(), yy + y());

          xx += ww;
	  if ((size + 2) > hh)
	    hh = size + 2;

	  needspace = 0;
	}
	else if (*ptr == '\n' && pre)
	{
	  *s = '\0';
	  s = buf;

          fl_draw(buf, xx + x(), yy + y());

	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = size + 2;
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
	ww = (int)fl_width(buf);

        if (needspace && xx > block->x)
	  xx += (int)fl_width(' ');

	if ((xx + ww) > block->w)
	{
	  if (line < 31)
	    line ++;
	  xx = block->line[line];
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
  HelpBlock	*block,		// Current block
		*cell;		// Current table cell
  int		row;		// Current table row (block number)
  const char	*ptr,		// Pointer into block
		*start,		// Pointer to start of element
		*attrs;		// Pointer to start of element attributes
  char		*s,		// Pointer into buffer
		buf[1024],	// Text buffer
		attr[1024],	// Attribute buffer
		link[1024];	// Link destination
  int		xx, yy, ww, hh;	// Size of current text fragment
  int		line;		// Current line in block
  int		links;		// Links for current line
  uchar		font, size;	// Current font and size
  uchar		border;		// Draw border?
  int		align,		// Current alignment
		head,		// In the <HEAD> section?
		pre,		// <PRE> text?
		needspace;	// Do we need whitespace?
  int		table_width;	// Width of table
  int		column,		// Current table column number
		columns[200];	// Column widths


  nblocks_   = 0;
  nlinks_    = 0;
  ntargets_  = 0;
  size_      = 0;
  bgcolor_   = color();
  textcolor_ = textcolor();
  linkcolor_ = selection_color();

  strcpy(title_, "Untitled");

  if (!value_)
    return;

  initfont(font, size);

  line      = 0;
  links     = 0;
  xx        = 4;
  yy        = size + 2;
  hh        = 0;
  block     = add_block(value_, xx, yy, w() - 24, 0);
  row       = 0;
  head      = 0;
  pre       = 0;
  align     = LEFT;
  needspace = 0;
  link[0]   = '\0';

  for (ptr = value_, s = buf; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf)
    {
      if (!head && !pre)
      {
        // Check width...
        *s = '\0';
        ww = (int)fl_width(buf);

        if (needspace && xx > block->x)
	  ww += (int)fl_width(' ');

        if ((xx + ww) > block->w)
	{
          line     = do_align(block, line, xx, align, links);
	  xx       = block->x;
	  yy       += hh;
	  block->h += hh;
	  hh       = 0;
	}

        if (link[0])
	  add_link(link, xx, yy - size, ww, size);

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
            if (link[0])
	      add_link(link, xx, yy - hh, ww, hh);

            line     = do_align(block, line, xx, align, links);
            xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = size + 2;
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
        if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
	  ptr ++;

      *s = '\0';
      s = buf;

      attrs = ptr;
      while (*ptr && *ptr != '>')
        ptr ++;

      if (*ptr == '>')
        ptr ++;

      if (strcasecmp(buf, "HEAD") == 0)
        head = 1;
      else if (strcasecmp(buf, "/HEAD") == 0)
        head = 0;
      else if (strcasecmp(buf, "TITLE") == 0)
      {
        // Copy the title in the document...
        for (s = title_;
	     *ptr != '<' && *ptr && s < (title_ + sizeof(title_) - 1);
	     *s++ = *ptr++);

	*s = '\0';
	s = buf;
      }
      else if (strcasecmp(buf, "A") == 0)
      {
        if (get_attr(attrs, "NAME", attr, sizeof(attr)) != NULL)
	  add_target(attr, yy - size - 2);
	else if (get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	{
	  strncpy(link, attr, sizeof(link) - 1);
	  link[sizeof(link) - 1] = '\0';
	}
      }
      else if (strcasecmp(buf, "/A") == 0)
        link[0] = '\0';
      else if (strcasecmp(buf, "BODY") == 0)
      {
        bgcolor_   = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)),
	                       color());
        textcolor_ = get_color(get_attr(attrs, "TEXT", attr, sizeof(attr)),
	                       textcolor());
        linkcolor_ = get_color(get_attr(attrs, "LINK", attr, sizeof(attr)),
	                       selection_color());
      }
      else if (strcasecmp(buf, "BR") == 0)
      {
        line     = do_align(block, line, xx, align, links);
        xx       = block->x;
	block->h += hh;
        yy       += hh;
	hh       = 0;
      }
      else if (strcasecmp(buf, "CENTER") == 0 ||
               strcasecmp(buf, "P") == 0 ||
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
	       strcasecmp(buf, "HR") == 0 ||
	       strcasecmp(buf, "PRE") == 0 ||
	       strcasecmp(buf, "TABLE") == 0)
      {
        block->end = start;
        line       = do_align(block, line, xx, align, links);
        xx         = block->x;
        block->h   += hh;

        if (!block->h && nblocks_ > 1)
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
	{
	  if (get_attr(attrs, "BORDER", attr, sizeof(attr)))
	    border = atoi(attr);
	  else
	    border = 0;

	  block->h += size + 2;

          if (get_attr(attrs, "WIDTH", attr, sizeof(attr)))
	  {
	    if (attr[strlen(attr) - 1] == '%')
	      table_width = atoi(attr) * w() / 100;
	    else
	      table_width = atoi(attr);
	  }
	  else
	    table_width = w();

          for (column = 0; column < 200; column ++)
	    columns[column] = table_width / 3;

	  column = 0;
	}

        if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
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
	else
	{
	  font = textfont_;
	  size = textsize_;
	}

	pushfont(font, size);

        yy = block->y + block->h;
        hh = 0;

        if ((tolower(buf[0]) == 'h' && isdigit(buf[1])) ||
	    strcasecmp(buf, "DD") == 0 ||
	    strcasecmp(buf, "DT") == 0 ||
	    strcasecmp(buf, "UL") == 0 ||
	    strcasecmp(buf, "OL") == 0 ||
	    strcasecmp(buf, "P") == 0)
          yy += size + 2;
	else if (strcasecmp(buf, "HR") == 0)
	{
	  hh += 2 * size;
	  yy += size;
	}

        if (row)
	  block = add_block(start, block->x, yy, block->w, 0);
	else
	  block = add_block(start, xx, yy, w() - 24, 0);

	needspace = 0;
	line      = 0;

	if (strcasecmp(buf, "CENTER") == 0)
	  align = CENTER;
	else
	  align = get_align(attrs, LEFT);
      }
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
        line       = do_align(block, line, xx, align, links);
        xx         = block->x;
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

        block     = add_block(ptr, xx, yy, w() - 24, 0);
	needspace = 0;
	hh        = 0;
	line      = 0;
	align     = LEFT;
      }
      else if (strcasecmp(buf, "TR") == 0)
      {
        block->end = start;
        line       = do_align(block, line, xx, align, links);
        xx         = block->x;
        block->h   += hh;

        if (row)
	{
          yy = blocks_[row].y + blocks_[row].h;

	  for (cell = blocks_ + row + 1; cell <= block; cell ++)
	    if ((cell->y + cell->h) > yy)
	      yy = cell->y + cell->h;

          block->h = yy - block->y + 2;

	  for (cell = blocks_ + row + 1; cell < block; cell ++)
	    cell->h = block->h;
	}

	yy        = block->y + block->h - 4;
	hh        = 0;
        block     = add_block(start, xx, yy, w() - 24, 0);
	row       = block - blocks_;
	needspace = 0;
	column    = 0;
	line      = 0;
      }
      else if (strcasecmp(buf, "/TR") == 0 && row)
      {
        line       = do_align(block, line, xx, align, links);
        block->end = start;
	block->h   += hh;

        xx = blocks_[row].x;

        if (block->end == block->start && nblocks_ > 1)
	{
	  nblocks_ --;
	  block --;
	}

        yy = blocks_[row].y + blocks_[row].h;

	for (cell = blocks_ + row + 1; cell <= block; cell ++)
	  if ((cell->y + cell->h) > yy)
	    yy = cell->y + cell->h;

        block->h = yy - block->y + 2;

	for (cell = blocks_ + row + 1; cell < block; cell ++)
	  cell->h = block->h;

	yy        = block->y + block->h - 4;
        block     = add_block(start, xx, yy, w() - 24, 0);
	needspace = 0;
	row       = 0;
	line      = 0;
      }
      else if ((strcasecmp(buf, "TD") == 0 ||
                strcasecmp(buf, "TH") == 0) && row)
      {
        line       = do_align(block, line, xx, align, links);
        block->end = start;
	block->h   += hh;

        if (strcasecmp(buf, "TH") == 0)
	  font = textfont_ | FL_BOLD;
	else
	  font = textfont_;

        size = textsize_;

        if (column == 0)
          xx = block->x + size + 3;
	else
          xx = block->w + 6;

        if (block->end == block->start && nblocks_ > 1)
	{
	  nblocks_ --;
	  block --;
	}

	pushfont(font, size);

        if (get_attr(attrs, "WIDTH", attr, sizeof(attr)) != NULL)
	{
	  ww = atoi(attr);

	  if (attr[strlen(attr) - 1] == '%')
	    ww = ww * w() / 100;

          columns[column] = ww;
	}
	else
	  ww = columns[column];

	yy        = blocks_[row].y;
	hh        = 0;
        block     = add_block(start, xx, yy, xx + ww, 0, border);
	needspace = 0;
	line      = 0;

	align = get_align(attrs, tolower(buf[1]) == 'h' ? CENTER : LEFT);

	column ++;
      }
      else if ((strcasecmp(buf, "/TD") == 0 ||
                strcasecmp(buf, "/TH") == 0) && row)
        popfont(font, size);
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
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, size);
    }
    else if (strcasecmp(buf, "IMG") == 0 &&
             get_attr(attrs, "ALT", attr, sizeof(attr)) != NULL)
    {
      // Show alt text...
      ww = (int)(fl_width(attr) + fl_width('[') + fl_width(']'));

      if (needspace && xx > block->x)
	ww += (int)fl_width(' ');

      if ((xx + ww) > block->w)
      {
        line     = do_align(block, line, xx, align, links);
	xx       = block->x;
	yy       += hh;
	block->h += hh;
	hh       = 0;
      }

      if (link[0])
	add_link(link, xx, yy - size, ww, size);

      xx += ww;
      if ((size + 2) > hh)
	hh = size + 2;

      needspace = 0;
    }
    else if (*ptr == '\n' && pre)
    {
      if (link[0])
	add_link(link, xx, yy - hh, ww, hh);

      line      = do_align(block, line, xx, align, links);
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
    else if (*ptr == '&' && s < (buf + sizeof(buf) - 1))
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
      if (s < (buf + sizeof(buf) - 1))
        *s++ = *ptr++;
      else
        ptr ++;

      if ((size + 2) > hh)
        hh = size + 2;
    }
  }

  if (s > buf && !pre && !head)
  {
    *s = '\0';
    ww = (int)fl_width(buf);

    if (needspace && xx > block->x)
      ww += (int)fl_width(' ');

    if ((xx + ww) > block->w)
    {
      line     = do_align(block, line, xx, align, links);
      xx       = block->x;
      yy       += hh;
      block->h += hh;
      hh       = 0;
    }

    if (link[0])
      add_link(link, xx, yy - size, ww, size);

    xx += ww;
    if ((size + 2) > hh)
      hh = size + 2;

    needspace = 0;
  }

  block->end = ptr;
  size_      = yy + hh;

  if (ntargets_ > 1)
    qsort(targets_, ntargets_, sizeof(HelpTarget),
          (int (*)(const void *, const void *))compare_targets);

  if (size_ < (h() - 8))
    scrollbar_.hide();
  else
    scrollbar_.show();

  topline(topline_);
}


//
// 'HelpView::get_align()' - Get an alignment attribute.
//

int					// O - Alignment
HelpView::get_align(const char *p,	// I - Pointer to start of attrs
                    int        a)	// I - Default alignment
{
  char	buf[255];			// Alignment value


  if (get_attr(p, "ALIGN", buf, sizeof(buf)) == NULL)
    return (a);

  if (strcasecmp(buf, "CENTER") == 0)
    return (CENTER);
  else if (strcasecmp(buf, "RIGHT") == 0)
    return (RIGHT);
  else
    return (LEFT);
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
  char	name[255],			// Name from string
	*ptr,				// Pointer into name or value
	quote;				// Quote


  while (*p && *p != '>')
  {
    while (isspace(*p))
      p ++;

    if (*p == '>' || !*p)
      return (NULL);

    for (ptr = name; *p && !isspace(*p) && *p != '=' && *p != '>';)
      if (ptr < (name + sizeof(name) - 1))
        *ptr++ = *p++;
      else
        p ++;

    *ptr = '\0';

    if (isspace(*p) || !*p || *p == '>')
      buf[0] = '\0';
    else
    {
      if (*p == '=')
        p ++;

      for (ptr = buf; *p && !isspace(*p) && *p != '>';)
        if (*p == '\'' || *p == '\"')
	{
	  quote = *p++;

	  while (*p && *p != quote)
	    if ((ptr - buf + 1) < bufsize)
	      *ptr++ = *p++;
	    else
	      p ++;

          if (*p == quote)
	    p ++;
	}
	else if ((ptr - buf + 1) < bufsize)
	  *ptr++ = *p++;
	else
	  p ++;

      *ptr = '\0';
    }

    if (strcasecmp(n, name) == 0)
      return (buf);

    if (*p == '>')
      return (NULL);
  }

  return (NULL);
}


//
// 'HelpView::get_color()' - Get an alignment attribute.
//

Fl_Color				// O - Color value
HelpView::get_color(const char *n,	// I - Color name
                    Fl_Color   c)	// I - Default color value
{
  int	rgb, r, g, b;			// RGB values


  if (!n)
    return (c);

  if (n[0] == '#')
  {
    // Do hex color lookup
    rgb = strtol(n + 1, NULL, 16);

    r = rgb >> 16;
    g = (rgb >> 8) & 255;
    b = rgb & 255;

    if (r == g && g == b)
      return (fl_gray_ramp(FL_NUM_GRAY * r / 256));
    else
      return (fl_color_cube((FL_NUM_RED - 1) * r / 255,
                            (FL_NUM_GREEN - 1) * g / 255,
			    (FL_NUM_BLUE - 1) * b / 255));
  }
  else if (strcasecmp(n, "black") == 0)
    return (FL_BLACK);
  else if (strcasecmp(n, "red") == 0)
    return (FL_RED);
  else if (strcasecmp(n, "green") == 0)
    return (fl_color_cube(0, 4, 0));
  else if (strcasecmp(n, "yellow") == 0)
    return (FL_YELLOW);
  else if (strcasecmp(n, "blue") == 0)
    return (FL_BLUE);
  else if (strcasecmp(n, "magenta") == 0 || strcasecmp(n, "fuchsia") == 0)
    return (FL_MAGENTA);
  else if (strcasecmp(n, "cyan") == 0 || strcasecmp(n, "aqua") == 0)
    return (FL_CYAN);
  else if (strcasecmp(n, "white") == 0)
    return (FL_WHITE);
  else if (strcasecmp(n, "gray") == 0 || strcasecmp(n, "grey") == 0)
    return (FL_GRAY);
  else if (strcasecmp(n, "lime") == 0)
    return (FL_GREEN);
  else if (strcasecmp(n, "maroon") == 0)
    return (fl_color_cube(2, 0, 0));
  else if (strcasecmp(n, "navy") == 0)
    return (fl_color_cube(0, 0, 2));
  else if (strcasecmp(n, "olive") == 0)
    return (fl_color_cube(2, 4, 0));
  else if (strcasecmp(n, "purple") == 0)
    return (fl_color_cube(2, 0, 2));
  else if (strcasecmp(n, "silver") == 0)
    return (FL_LIGHT2);
  else if (strcasecmp(n, "teal") == 0)
    return (fl_color_cube(0, 4, 2));
  else
    return (c);
}


//
// 'HelpView::handle()' - Handle events in the widget.
//

int				// O - 1 if we handled it, 0 otherwise
HelpView::handle(int event)	// I - Event to handle
{
  int		i;		// Looping var
  int		xx, yy;		// Adjusted mouse position
  HelpLink	*link;		// Current link
  char		target[32];	// Current target


  switch (event)
  {
    case FL_MOVE :
    case FL_PUSH :
        xx = Fl::event_x() - x();
	yy = Fl::event_y() - y() + topline_;
        if (!scrollbar_.visible() || xx < (w() - 20))
	  break;

    default :
	// Use the Fl_Group handler...
	return (Fl_Group::handle(event));
  }

  // Handle mouse clicks on links...
  for (i = nlinks_, link = links_; i > 0; i --, link ++)
    if (xx >= link->x && xx < link->w &&
        yy >= link->y && yy < link->h)
      break;

  if (!i)
  {
    fl_cursor(FL_CURSOR_DEFAULT);
    return (1);
  }

  // Change the cursor for FL_MOTION events, and go to the link for
  // clicks...
  if (event == FL_MOVE)
    fl_cursor(FL_CURSOR_HAND);
  else
  {
    fl_cursor(FL_CURSOR_DEFAULT);

    strncpy(target, link->name, sizeof(target) - 1);
    target[sizeof(target) - 1] = '\0';

    set_changed();

    if (strcmp(link->filename, filename_) != 0 && link->filename[0])
    {
      char	dir[1024];	// Current directory
      char	temp[1024];	// Temporary filename


      if (link->filename[0] != '/' && strchr(link->filename, ':') == NULL)
      {
	if (directory_[0])
	  sprintf(temp, "%s/%s", directory_, link->filename);
	else
	{
	  getcwd(dir, sizeof(dir));
	  sprintf(temp, "file:%s/%s", dir, link->filename);
	}

        load(temp);
      }
      else
        load(link->filename);
    }
    else if (target[0])
      topline(target);
    else
      topline(0);
  }

  return (1);
}


//
// 'HelpView::HelpView()' - Build a HelpView widget.
//

HelpView::HelpView(int        xx,	// I - Left position
                   int        yy,	// I - Top position
		   int        ww,	// I - Width in pixels
		   int        hh,	// I - Height in pixels
		   const char *l)
    : Fl_Group(xx, yy, ww, hh, l),
      scrollbar_(xx + ww - 17, yy, 17, hh)
{
  filename_[0] = '\0';
  value_       = NULL;

  ablocks_     = 0;
  nblocks_     = 0;
  blocks_      = (HelpBlock *)0;

  alinks_      = 0;
  nlinks_      = 0;
  links_       = (HelpLink *)0;

  atargets_    = 0;
  ntargets_    = 0;
  targets_     = (HelpTarget *)0;

  nfonts_      = 0;
  textfont_    = FL_TIMES;
  textsize_    = 12;

  topline_     = 0;
  size_        = 0;

  color(FL_WHITE);
  textcolor(FL_BLACK);
  selection_color(FL_BLUE);

  scrollbar_.value(0, hh, 0, 1);
  scrollbar_.step(8.0);
  scrollbar_.show();
  scrollbar_.callback(scrollbar_callback);

  end();
}


//
// 'HelpView::~HelpView()' - Destroy a HelpView widget.
//

HelpView::~HelpView()
{
  if (nblocks_)
    free(blocks_);
  if (nlinks_)
    free(links_);
  if (ntargets_)
    free(targets_);
  if (value_)
    free((void *)value_);
}


//
// 'HelpView::load()' - Load the specified file.
//

int				// O - 0 on success, -1 on error
HelpView::load(const char *f)	// I - Filename to load (may also have target)
{
  FILE		*fp;		// File to read from
  long		len;		// Length of file
  char		*target;	// Target in file
  char		*slash;		// Directory separator
  const char	*localname;	// Local filename
  char		error[1024];	// Error buffer


  strcpy(filename_, f);
  strcpy(directory_, filename_);

  if ((slash = strrchr(directory_, '/')) == NULL)
    directory_[0] = '\0';
  else if (slash > directory_ && slash[-1] != '/')
    *slash = '\0';

  if ((target = strrchr(filename_, '#')) != NULL)
    *target++ = '\0';

  if (link_)
    localname = (*link_)(filename_);
  else
    localname = filename_;

  if (localname != NULL &&
      (strncmp(localname, "ftp:", 4) == 0 ||
       strncmp(localname, "http:", 5) == 0 ||
       strncmp(localname, "https:", 6) == 0 ||
       strncmp(localname, "ipp:", 4) == 0 ||
       strncmp(localname, "mailto:", 7) == 0 ||
       strncmp(localname, "news:", 5) == 0))
    localname = NULL;	// Remote link wasn't resolved...
  else if (localname != NULL &&
           strncmp(localname, "file:", 5) == 0)
    localname += 5;	// Adjust for local filename...
      
  if (value_ != NULL)
  {
    free((void *)value_);
    value_ = NULL;
  }

  if (localname)
  {
    if ((fp = fopen(localname, "rb")) != NULL)
    {
      fseek(fp, 0, SEEK_END);
      len = ftell(fp);
      rewind(fp);

      value_ = (const char *)calloc(len + 1, 1);
      fread((void *)value_, 1, len, fp);
      fclose(fp);
    }
    else
    {
      sprintf(error, "%s: %s\n", localname, strerror(errno));
      value_ = strdup(error);
    }
  }
  else
    value_ = strdup("File or link could not be opened.\n");

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
HelpView::resize(int xx,	// I - New left position
                 int yy,	// I - New top position
		 int ww,	// I - New width
		 int hh)	// I - New height
{
  Fl_Widget::resize(xx, yy, ww, hh);
  scrollbar_.resize(xx + ww - 17, yy, 17, hh);

  format();
}


//
// 'HelpView::topline()' - Set the top line to the named target.
//

void
HelpView::topline(const char *n)	// I - Target name
{
  HelpTarget	key,			// Target name key
		*target;		// Pointer to matching target


  if (ntargets_ == 0)
    return;

  strncpy(key.name, n, sizeof(key.name) - 1);
  key.name[sizeof(key.name) - 1] = '\0';

  target = (HelpTarget *)bsearch(&key, targets_, ntargets_, sizeof(HelpTarget),
                                 (int (*)(const void *, const void *))compare_targets);

  if (target != NULL)
    topline(target->y);
}


//
// 'HelpView::topline()' - Set the top line by number.
//

void
HelpView::topline(int t)	// I - Top line number
{
  if (!value_)
    return;

  if (size_ < (h() - 8) || t < 0)
    t = 0;
  else if (t > size_)
    t = size_;

  topline_ = t;

  scrollbar_.value(topline_, h(), 0, size_);

  do_callback();
  clear_changed();

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

  value_ = strdup(v);

  format();

  set_changed();
  topline(0);
}


//
// 'scrollbar_callback()' - A callback for the scrollbar.
//

static void
scrollbar_callback(Fl_Widget *s, void *)
{
  ((HelpView *)(s->parent()))->topline(int(((Fl_Scrollbar*)s)->value()));
}


//
// End of "$Id: HelpView.cxx,v 1.22 2000/03/19 23:27:14 mike Exp $".
//
