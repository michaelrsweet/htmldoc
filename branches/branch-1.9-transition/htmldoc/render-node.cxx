//
// "$Id: render-node.cxx,v 1.1.2.2 2004/03/30 03:49:15 mike Exp $"
//
//   Render node methods for HTMLDOC, a HTML document processing
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
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdRenderNode::hdRenderNode()  - Create a new empty render node.
//   hdRenderNode::hdRenderNode()  - Create a new render node.
//   hdRenderNode::~hdRenderNode() - Destroy a render node.
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "debug.h"
#include "render.h"
#include "hdstring.h"


//
// 'hdRenderNode::hdRenderNode()' - Create a new empty render node.
//

hdRenderNode::hdRenderNode()
{
  memset(this, 0, sizeof(hdRenderNode));
}


//
// 'hdRenderNode::hdRenderNode()' - Create a new render node.
//

hdRenderNode::hdRenderNode(hdRenderType t,
					// I - Type of node
                           float        xx,
					// I - X position of node
			   float        yy,
					// I - Y position of node
			   float        w,
					// I - Width of node
			   float        h,
					// I - Height of node
			   hdStyle      *s,
					// I - Style for node
                           const void   *d)
					// I - Pointer to data
{
  prev   = (hdRenderNode *)0;
  next   = (hdRenderNode *)0;
  type   = t;
  x      = xx;
  y      = yy;
  width  = w;
  height = h;
  style  = s;

  switch (t)
  {
    case HD_RENDERTYPE_TEXT :
        data.text = (const char *)d;
        break;

    case HD_RENDERTYPE_IMAGE :
        data.image = (hdImage *)d;
        break;

    case HD_RENDERTYPE_BOX :
        if (d)
          memcpy(data.box, d, sizeof(data.box));
        break;

    case HD_RENDERTYPE_LINK :
        data.link = (const char *)d;
        break;

    default :
        break;
  }
}


//
// End of "$Id: render-node.cxx,v 1.1.2.2 2004/03/30 03:49:15 mike Exp $".
//
