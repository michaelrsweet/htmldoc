//
// "$Id: image.cxx,v 1.11.2.32.2.2 2004/03/30 03:49:15 mike Exp $"
//
//   Image handling routines for HTMLDOC, a HTML document processing program.
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
//   hdImage::hdImage()           - Create a new empty image...
//   hdImage::~hdImage()          - Delete an image...
//   hdImage::need_mask()         - Allocate memory for the image mask...
//   hdImage::need_pixels()       - Allocate memory for the image pixels...
//   hdImage::compare()           - Compare two images...
//   hdImage::copy()              - Copy image files to the destination
//                                  directory...
//   hdImage::find()              - Find an image file in memory...
//   hdImage::flush()             - Flush the image cache...
//   hdImage::free()              - Free an image from memory.
//   hdImage::load()              - Load an image into memory...
//   image_load()                 - Load an image file from disk...
//   hdImage::register_standard() - Register all of the standard image formats.
//   hdImage::register_format()   - Register an image file format.
//   hdImage::save()              - Save the image to the named destination.
//   hdImage::save_as_png()       - Save a PNG image to the named destination.
//   hdImage::set_mask()          - Clear a bit in the image mask.
//   hdImage::set_size()          - Set the size of the image...
//   hdImage::uri()               - Set the name of an image...
//

//
// Include necessary headers.
//

#include "image.h"
#include "hdstring.h"


//
// Class globals...
//

int	hdImage::num_images_ = 0,	// Number of images in cache
	hdImage::alloc_images_ = 0;	// Allocated images
hdImage	**hdImage::images_ = NULL;	// Images in cache

int	hdImage::num_formats_ = 0,	// Number of file formats
	hdImage::alloc_formats_ = 0;	// Allocated file formats
hdImageCheck	*hdImage::formats_ = 0;	// File formats


//
// 'hdImage::hdImage()' - Create a new empty image...
//

hdImage::hdImage()
{
  uri_       = (char *)0;
  width_     = 0;
  height_    = 0;
  depth_     = 0;
  use_       = 1;
  obj_       = 0;
  type_      = HD_IMAGE_RASTER;
  pixels_    = 0;
  mask_      = 0;
  maskwidth_ = 0;
}


//
// 'hdImage::~hdImage()' - Delete an image...
//

hdImage::~hdImage()
{
  int	i;	// Looping var


  free();

  if (uri_)
    delete[] uri_;

  for (i = 0; i < num_images_; i ++)
    if (images_[i] == this)
      break;

  if (i < num_images_)
  {
    num_images_ --;

    if (i < num_images_)
      memcpy(images_ + i, images_ + i + 1,
             sizeof(hdImage *) * (num_images_ - i));
  }
}


//
// 'hdImage::need_mask()' - Allocate memory for the image mask...
//

void
hdImage::alloc_mask()
{
  int	size;			// Byte size of mask image


  // See if we've already allocated the mask image...
  if (mask_)
    return;

  // Figure out the size of the mask image, and then allocate and clear
  // all the bits needed...
  maskwidth_ = (width_ + 7) / 8;
  size       = maskwidth_ * height_;

  mask_ = new hdByte[size];

  memset(mask_, 0, size);
}


//
// 'hdImage::need_pixels()' - Allocate memory for the image pixels...
//

void
hdImage::alloc_pixels()
{
  int	size;			// Byte size of pixels image


  // See if we've already allocated the pixels array...
  if (pixels_ || !width_ || !height_ || !depth_)
    return;

  // Figure out the size of the pixels array, and then allocate and clear
  // all the bits needed...
  size = width_ * height_ * depth_;

  pixels_ = new hdByte[size];

  memset(pixels_, 0, size);
}


//
// 'hdImage::compare()' - Compare two images...
//

int					// O - Result of comparison
hdImage::compare(hdImage **img1,	// I - First image
                 hdImage **img2)	// I - Second image
{
#ifdef WIN32
  return (strcasecmp((*img1)->uri_, (*img2)->uri_));
#else
  return (strcmp((*img1)->uri_, (*img2)->uri_));
#endif // WIN32
}


//
// 'hdImage::copy()' - Copy image files to the destination directory...
//

int				// O - 0 on success, -1 on failure
hdImage::copy(const char *path,	// I - Destination path
              char       *d,	// O - Destination filename
              int        dlen)	// I - Max length of destination
{
  hdFile	*in, *out;	// Input/output files
  char		buffer[8192];	// Data buffer
  int		nbytes;		// Number of bytes in buffer


  // Figure out the destination filename...
  if (strcmp(path, ".") == 0)
    hdFile::basename(uri_, d, dlen);
  else
    snprintf(d, dlen, "%s/%s", path,
             hdFile::basename(uri_, buffer, sizeof(buffer)));

  // Don't copy if the destination == source
  if (strcmp(d, uri_) == 0)
    return (0);

  // Open files and copy...
  if ((in = hdFile::open(uri_, HD_FILE_READ)) == NULL)
    return (-1);

  if ((out = hdFile::open(d, HD_FILE_WRITE)) == NULL)
  {
    delete in;
    return (-1);
  }

  while ((nbytes = in->read(buffer, sizeof(buffer))) > 0)
    out->write(buffer, nbytes);

  delete in;
  delete out;

  return (0);
}


//
// 'hdImage::find()' - Find an image file in memory...
//

hdImage *			// O - Pointer to image
hdImage::find(const char *p,	// I - Name of image file
              int        gs,	// I - 1 = grayscale, 0 = color
              const char *path)	// I - Search path for files
{
  int		i;		// Looping var...
  hdImage	key,		// Search key...
		*keyptr,	// Pointer to search key...
		*img,		// Pointer to image...
		**match;	// Matching image
  hdImageCheck	*f;		// Pointer to check functions...
  char		filename[HD_MAX_URI];
				// Actual filename
  hdFile	*fp;		// File pointer
  char		header[16];	// File header


  // Range check...
  if (p == NULL)
    return (NULL);

  if (p[0] == '\0')	// Microsoft VC++ runtime bug workaround...
    return (NULL);

  // Find the file...
  if (hdFile::find(path, p, filename, sizeof(filename)) == NULL)
    return (NULL);

  // See if we've already loaded it...
  if (num_images_ > 0)
  {
    key.uri(filename);

    keyptr = &key;

    match = (hdImage **)bsearch(&keyptr, images_, num_images_, sizeof(hdImage *),
                                (int (*)(const void *, const void *))compare);
    if (match)
    {
      img = *match;
      img->use_ ++;

      return (img);
    }
  }

  // Nope, see if we can load it...
  if ((fp = hdFile::open(filename, HD_FILE_READ)) == NULL)
  {
    perror(filename);
    return (NULL);
  }

  // Read the first 16 bytes of the file...
  fp->read(header, sizeof(header));
  delete fp;

  printf("First 16 bytes of \"%s\" are:\n   ", filename);
  for (i = 0; i < 16; i ++)
    printf(" %02X", (hdByte)header[i]);
  putchar('\n');

  // See if any of the registered image classes can load it...
  for (i = 0, img = (hdImage *)0, f = formats_; i < num_formats_; i ++, f ++)
    if ((img = (*f)(filename, gs, header)) != NULL)
      break;

  if (!img)
    return ((hdImage *)0);

  // Add the new image to the cache...
  if (num_images_ >= alloc_images_)
  {
    hdImage **temp = new hdImage *[alloc_images_ + HD_ALLOC_FILES];

    if (num_images_)
    {
      memcpy(temp, images_, sizeof(hdImage *) * num_images_);
      delete[] images_;
    }

    images_       = temp;
    alloc_images_ += HD_ALLOC_FILES;
  }

  images_[num_images_] = img;
  num_images_ ++;

  if (num_images_ > 1)
    qsort(images_, num_images_, sizeof(hdImage *),
          (int (*)(const void *, const void *))compare);

  return (img);
}


//
// 'hdImage::flush()' - Flush the image cache...
//

void
hdImage::flush(void)
{
  int	i;			// Looping var


  // Free the memory used by each image...
  for (i = 0; i < num_images_; i ++)
    delete images_[i];

  if (alloc_images_)
  {
    delete[] images_;

    alloc_images_ = 0;
  }

  num_images_ = 0;
}


//
// 'hdImage::free()' - Free an image from memory.
//

void
hdImage::free()
{
  if (!pixels_ && !mask_)
    return;

  if (pixels_)
  {
    delete[] pixels_;
    pixels_ = (hdByte *)0;
  }

  if (mask_)
  {
    delete[] mask_;
    mask_ = (hdByte *)0;
  }
}


//
// 'hdImage::load()' - Load an image into memory...
//

int				// O - 0 on success, -1 on error
hdImage::load()
{
  return (-1);
}


//
// 'hdImage::register_standard()' - Register all of the standard image formats.
//

void
hdImage::register_standard()
{
  register_format(hdBMPImage::check);
//  register_format(hdEPSImage::check);
  register_format(hdGIFImage::check);
  register_format(hdJPEGImage::check);
  register_format(hdPNGImage::check);
//  register_format(hdPNMImage::check);
//  register_format(hdXBMImage::check);
//  register_format(hdXPMImage::check);
}


//
// 'hdImage::register_format()' - Register an image file format.
//

void
hdImage::register_format(hdImageCheck check)	// I - Check function
{
  int		i;				// Looping var
  hdImageCheck	*temp;				// Image check function array


  // Make sure this function has not been registered before...
  for (i = 0; i < num_formats_; i ++)
    if (check == formats_[i])
      return;

  if (num_formats_ >= alloc_formats_)
  {
    temp = new hdImageCheck[alloc_formats_ + HD_ALLOC_FILES];
    if (num_formats_)
    {
      memcpy(temp, formats_, sizeof(hdImageCheck) * num_formats_);
      delete[] formats_;
    }

    formats_       = temp;
    alloc_formats_ += HD_ALLOC_FILES;
  }

  formats_[num_formats_] = check;
  num_formats_ ++;
}


//
// 'hdImage::save()' - Save the image to the named destination.
//

int				// O - 0 on success, -1 on failure
hdImage::save(const char *path,	// I - Destination path
              char       *d,	// O - Destination filename
	      int        dlen)	// I - Size of destination filename
{
  if (type_ == HD_IMAGE_VECTOR)
    return (save_as_png(path, d, dlen));
  else
    return (copy(path, d, dlen));
}


//
// 'hdImage::save_as_png()' - Save a PNG image to the named destination.
//

int					// O - 0 on success, -1 on failure
hdImage::save_as_png(const char *path,	// I - Destination path
                     char       *d,	// O - Destination filename
	             int        dlen)	// I - Size of destination filename
{
  // TODO: implement this!
  return (-1);
}


//
// 'hdImage::set_mask()' - Clear a bit in the image mask.
//

void
hdImage::set_mask(int   x,	// I - X coordinate
                  int   y,	// I - Y coordinate
		  hdByte a)	// I - Alpha value
{
  hdByte	*maskptr;	// Pointer into mask image
  static hdByte	masks[8] =	// Masks for each bit
		{
		  0x80, 0x40, 0x20, 0x10,
		  0x08, 0x04, 0x02, 0x01
		};
  static hdByte	dither[16][16] = // Simple 16x16 Floyd dither
		{
		 { 0,   128, 32,  160, 8,   136, 40,  168,
		   2,   130, 34,  162, 10,  138, 42,  170 },
		 { 192, 64,  224, 96,  200, 72,  232, 104,
		   194, 66,  226, 98,  202, 74,  234, 106 },
		 { 48,  176, 16,  144, 56,  184, 24,  152,
		   50,  178, 18,  146, 58,  186, 26,  154 },
		 { 240, 112, 208, 80,  248, 120, 216, 88,
		   242, 114, 210, 82,  250, 122, 218, 90 },
		 { 12,  140, 44,  172, 4,   132, 36,  164,
		   14,  142, 46,  174, 6,   134, 38,  166 },
		 { 204, 76,  236, 108, 196, 68,  228, 100,
		   206, 78,  238, 110, 198, 70,  230, 102 },
		 { 60,  188, 28,  156, 52,  180, 20,  148,
		   62,  190, 30,  158, 54,  182, 22,  150 },
		 { 252, 124, 220, 92,  244, 116, 212, 84,
		   254, 126, 222, 94,  246, 118, 214, 86 },
		 { 3,   131, 35,  163, 11,  139, 43,  171,
		   1,   129, 33,  161, 9,   137, 41,  169 },
		 { 195, 67,  227, 99,  203, 75,  235, 107,
		   193, 65,  225, 97,  201, 73,  233, 105 },
		 { 51,  179, 19,  147, 59,  187, 27,  155,
		   49,  177, 17,  145, 57,  185, 25,  153 },
		 { 243, 115, 211, 83,  251, 123, 219, 91,
		   241, 113, 209, 81,  249, 121, 217, 89 },
		 { 15,  143, 47,  175, 7,   135, 39,  167,
		   13,  141, 45,  173, 5,   133, 37,  165 },
		 { 207, 79,  239, 111, 199, 71,  231, 103,
		   205, 77,  237, 109, 197, 69,  229, 101 },
		 { 63,  191, 31,  159, 55,  183, 23,  151,
		   61,  189, 29,  157, 53,  181, 21,  149 },
		 { 254, 127, 223, 95,  247, 119, 215, 87,
		   253, 125, 221, 93,  245, 117, 213, 85 }
	       };


  // Make sure we have a mask and the coordinates lie inside...
  if (!mask_ || x < 0 || x >= width_ || y < 0 || y > height_)
    return;

  // Conditionally set the mask bit depending on the alpha value...
  // Note that the mask image contains 1 bits for each *transparent*
  // pixel...
  maskptr  = mask_ + y * maskwidth_ + x / 8;

  if (a <= dither[x & 15][y & 15])
    *maskptr |= masks[x & 7];
}


//
// 'hdImage::set_size()' - Set the size of the image...
//

void
hdImage::set_size(int w,		// I - Width of image
                  int h,		// I - Height of image
		  int d)		// I - Depth of image
{
  // Free any existing pixel data as needed...
  if (pixels_ || mask_)
    free();

  // Set the size of the image...
  width_  = w;
  height_ = h;
  depth_  = d;
}


//
// 'hdImage::uri()' - Set the name of an image...
//

void
hdImage::uri(const char *p)		// I - New URI
{
  if (uri_)
    delete[] uri_;

  if (p)
  {
    uri_ = new char[strlen(p) + 1];
    strcpy(uri_, p);
  }
  else
    uri_ = (char *)0;
}


//
// End of "$Id: image.cxx,v 1.11.2.32.2.2 2004/03/30 03:49:15 mike Exp $".
//
