//
// "$Id: render-support.cxx,v 1.4 2004/03/08 01:01:41 mike Exp $"
//
//   Rendering support methods for HTMLDOC, a HTML document processing
//   program.
//
//   Copyright 1997-2004 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
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
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "debug.h"
#include "htmldoc.h"
#include "hdstring.h"


//
// Timezone offset for dates, below...
//

#ifdef HAVE_TM_GMTOFF
#  define timezone (date->tm_gmtoff)
#endif // HAVE_TM_GMTOFF


//
// 'hdRender::hdRender()' - Create a new render instance.
//

hdRender::hdRender(hdStyleSheet *s)	// I - Stylesheet
{
  struct tm	*date;			// Time/date info...


  // Initialize class members...
  css = s;

  media.page_width        = css->media.page_width;
  media.page_length       = css->media.page_length;
  media.page_left         = css->media.page_left;
  media.page_right        = css->media.page_right;
  media.page_bottom       = css->media.page_bottom;
  media.page_top          = css->media.page_top;
  media.page_print_width  = css->media.page_print_width;
  media.page_print_length = css->media.page_print_length;
  media.orientation       = css->media.orientation;
  media.sides             = css->media.sides;
  media.media_position    = css->media.media_position;

  strcpy(media.media_color, css->media.media_color);
  strcpy(media.media_type, css->media.media_type);

  background = NULL;
  doc_time   = time(NULL);
  date       = localtime(&doc_time);

  snprintf(doc_date, sizeof(doc_date), "D:%04d%02d%02d%02d%02d%02d%+03d%02d",
           date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
           date->tm_hour, date->tm_min, date->tm_sec,
	   (int)(-timezone / 3600),
	   (int)(((timezone < 0 ? -timezone : timezone) / 60) % 60));

  doc_title           = "Untitled";
  title_page          = 0;
  current_chapter     = 0;
  num_chapters        = 0;
  alloc_chapters      = 0;
  chapters            = NULL;
  current_heading     = NULL;
  num_headings        = 0;
  alloc_headings      = 0;
  headings            = NULL;
  num_imgmaps         = 0;
  alloc_imgmaps       = 0;
  imgmaps             = NULL;
  num_links           = 0;
  alloc_links         = 0;
  links               = NULL;
  num_pages           = 0;
  alloc_pages         = 0;
  pages               = NULL;
  render_font         = NULL;
  render_size         = -1.0f;
  render_rgb[0]       = -1.0f;
  render_rgb[1]       = -1.0f;
  render_rgb[2]       = -1.0f;
  render_x            = -1.0f;
  render_y            = -1.0f;
  render_startx       = -1.0f;
  render_spacing      = 0.0f;
}


//
// 'hdRender::~hdRender()' -
//

hdRender::~hdRender()
{
  int		i;			// Looping var...
  hdRenderNode	*r,			// Pointer to node
		*next;			// Pointer to next node


  if (alloc_chapters)
    delete[] chapters;

  if (alloc_headings)
    delete[] headings;

  if (alloc_links)
    delete[] links;

  if (alloc_pages)
  {
    for (i = 0; i < num_pages; i ++)
      for (r = pages[i].first; r; r = next)
      {
        next = r->next;
	delete r;
      }

    delete[] pages;
  }
}


//
// 'hdRender::add_chapter()' -
//

void
hdRender::add_chapter()
{
}


//
// 'hdRender::add_heading()' -
//

void
hdRender::add_heading(hdTree *node,
        	      int    page,
		      int    top)
{
}


//
// 'hdRender::add_imgmap()' - Add an image map to the document.
//

void
hdRender::add_imgmap(hdTree *t)		// I - MAP node
{
  hdTree	**temp;			// New image map array...


  if (num_imgmaps >= alloc_imgmaps)
  {
    temp = new hdTree *[alloc_imgmaps + 10];

    if (alloc_imgmaps)
    {
      memcpy(temp, imgmaps, alloc_imgmaps * sizeof(hdTree *));
      delete[] imgmaps;
    }

    imgmaps       = temp;
    alloc_imgmaps += 10;
  }

  imgmaps[num_imgmaps] = t;
  num_imgmaps ++;
}


//
// 'hdRender::find_imgmap()' - Find an image map node in the document.
//

hdTree *				// O - MAP node
hdRender::find_imgmap(const char *name)	// I - Name of image map
{
  int		i;			// Looping var
  const char	*temp;			// Pointer to map name


  // Get the map name after the #...
  if ((temp = strrchr(name, '#')) != NULL)
    name = temp + 1;

  // Loop until we find it...
  for (i = 0; i < num_imgmaps; i ++)
    if ((temp = imgmaps[i]->get_attr("name")) != NULL &&
        strcasecmp(name, temp) == 0)
      return (imgmaps[i]);

  return (NULL);
}


//
// 'hdRender::add_link()' - Add a link destination.
//

void
hdRender::add_link(const char *name,	// I - Name of link
        	   int        page,	// I - Link page
		   int        top)	// I - Link Y position
{
  hdRenderLink	*temp;			// New link


  if (name == NULL)
    return;

  if ((temp = find_link(name)) != NULL)
  {
    temp->page = page;
    temp->top  = top;
  }
  else
  {
    // See if we need to allocate memory for links...
    if (num_links >= alloc_links)
    {
      // Allocate more links...
      temp = new hdRenderLink[alloc_links + HD_ALLOC_LINKS];

      if (alloc_links)
      {
        memcpy(temp, links, alloc_links * sizeof(hdRenderLink));
	delete[] links;
      }

      links       = temp;
      alloc_links += HD_ALLOC_LINKS;
    }

    // Add a new link...
    temp = links + num_links;
    num_links ++;

    temp->name = strdup(name);
    temp->page = page;
    temp->top  = top;

    if (num_links > 1)
      qsort(links, num_links, sizeof(hdRenderLink),
            (hdCompareFunc)compare_links);
  }
}


//
// 'hdRender::find_link()' - Find a link...
//

hdRenderLink *				// O - Matching link
hdRender::find_link(const char *name)	// I - Name of link
{
  hdRenderLink	key,			// Search key
		*match;			// Matching name entry


  if (name == NULL || num_links == 0)
    return (NULL);

  if (name[0] == '#')
    name ++;

  key.name = (char *)name;

  match = (hdRenderLink *)bsearch(&key, links, num_links, sizeof(hdRenderLink),
                                  (hdCompareFunc)compare_links);

  return (match);
}


//
// 'hdRender::compare_links()' -
//

int						// O - Result of comparison
hdRender::compare_links(hdRenderLink *n1,	// I - First link
                        hdRenderLink *n2)	// I - Second link
{
  return (strcasecmp(n1->name, n2->name));
}


//
// 'hdRender::check_pages()' -
//

void
hdRender::check_pages(int page)
{
}


//
// 'hdRender::add_render()' - Add a render node.
//

hdRenderNode *				// O - New render node
hdRender::add_render(int          page,	// I - Page to add to
        	     hdRenderType type,	// I - Render type
		     float        x,	// I - X position of node
		     float        y,	// I - Y position of node
        	     float        width,// I - Width of node
		     float        height,// I - Height of node
		     const void   *data,// I - Data
		     int          alloc_data,
		     			// I - Did we allocate the data?
        	     int          insert)// I - Insert at beginning of page?
{
  hdRenderNode		*r;		// New render primitive


  DEBUG_printf(("add_render(page=%d, type=%d, x=%.1f, y=%.1f, width=%.1f, height=%.1f, data=%p, insert=%d)\n",
                page, type, x, y, width, height, data, insert));

  // Make sure the page node is allocated...
  check_pages(page);

  if (page < 0 || page >= alloc_pages)
  {
    hdGlobal.progress_error(HD_ERROR_INTERNAL_ERROR,
                            "Page number (%d) out of range (1...%d)\n", page + 1,
                            alloc_pages);

    return ((hdRenderNode *)0);
  }

  // Keep track of the type of render nodes on this page...
  pages[page].types |= type;

  // Create the new render node...
  r = new hdRenderNode(type, x, y, width, height, data, alloc_data);

  if (insert)
  {
    r->next           = pages[page].first;
    pages[page].first = r;

    if (pages[page].last == NULL)
      pages[page].last = r;
  }
  else
  {
    if (pages[page].last != NULL)
      pages[page].last->next = r;
    else
      pages[page].first = r;

    r->next         = NULL;
    pages[page].last = r;
  }

  DEBUG_printf(("    returning r = %p\n", r));

  return (r);
}


//
// 'hdRender::get_color()' - Get a color value.
//

void
hdRender::get_color(const char *c,	// I - Color name
                    float      *rgb,	// O - RGB color
	            int        defblack)// I - Default to black?
{
  unsigned char	rgbc[3];		// Integer RGB value...


  // Initialize color value...
  memset(rgbc, defblack ? 0 : 255, 3);

  hdStyle::get_color(c, rgbc);

  rgb[0] = rgbc[0] / 255.0f;
  rgb[1] = rgbc[1] / 255.0f;
  rgb[2] = rgbc[2] / 255.0f;
}


//
// 'hdRenderNode::hdRenderNode()' - Create a new render node.
//

hdRenderNode::hdRenderNode(hdRenderType t,	// I - Type of node
                           float        xx,	// I - X position of node
			   float        yy,	// I - Y position of node
			   float        w,	// I - Width of node
			   float        h,	// I - Height of node
                           const void   *d,	// I - Pointer to data
			   int          alloc_d)// I - Was data allocated?
{
  next   = (hdRenderNode *)0;
  type   = t;
  x      = xx;
  y      = yy;
  width  = w;
  height = h;

  switch (t)
  {
    case HD_RENDERTYPE_TEXT :
        data.text.font         = NULL;
	data.text.font_size    = h;
	data.text.char_spacing = 0.0f;
	data.text.rgb[0]       = 0.0f;
	data.text.rgb[1]       = 0.0f;
	data.text.rgb[2]       = 0.0f;
	data.text.alloc_string = alloc_d;
	data.text.string       = (char *)d;
        break;

    case HD_RENDERTYPE_IMAGE :
        data.image = (hdImage *)d;
        break;

    case HD_RENDERTYPE_BOX :
        if (d)
          memcpy(data.box, d, sizeof(data.box));
        break;

    case HD_RENDERTYPE_BACKGROUND :
        data.background = (hdStyle *)d;
	break;

    case HD_RENDERTYPE_LINK :
        data.link.alloc_url = alloc_d;
	data.link.url       = (char *)d;
        break;

    case HD_RENDERTYPE_FORM :
        break;
  }
}


//
// 'hdRenderNode::~hdRenderNode()' - Destroy a render node.
//

hdRenderNode::~hdRenderNode()
{
  switch (type)
  {
    case HD_RENDERTYPE_TEXT :
        if (data.text.alloc_string)
	  free(data.text.string);
	break;

    case HD_RENDERTYPE_LINK :
        if (data.link.alloc_url)
	  free(data.link.url);
	break;
  }
}


//
// End of "$Id: render-support.cxx,v 1.4 2004/03/08 01:01:41 mike Exp $".
//
