//
// "$Id: image.cxx,v 1.15 2001/12/07 18:26:58 mike Exp $"
//
//   Image handling routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2001 by Easy Software Products.
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

#include "image.h"
#include "hdstring.h"


//
// Class globals...
//

int	hdImage::num_images_ = 0,	// Number of images in cache
	hdImage::alloc_images_ = 0;	// Allocated images
hdImage	**hdImage::images_ = NULL;	// Images in cache


//
// 'hdImage::compare()' - Compare two image filenames...
//

static int			// O - Result of comparison
image_compare(hdImage **img1,	// I - First image
              hdImage **img2)	// I - Second image
{
#ifdef WIN32
  return (strcasecmp((*img1)->filename, (*img2)->filename));
#else
  return (strcmp((*img1)->filename, (*img2)->filename));
#endif // WIN32
}


//
// 'image_copy()' - Copy image files to the destination directory...
//

int				// O - 0 on success, -1 on failure
hdImage::copy(char *dest,	// I - Destination path
              int  destlen)	// I - Max length of destination
{
  FILE	*in, *out;		// Input/output files
  uchar	buffer[8192];		// Data buffer
  int	nbytes;			// Number of bytes in buffer


  // Figure out the destination filename...
  if (strcmp(destpath, ".") == 0)
    strcpy(dest, file_basename(filename));
  else
    sprintf(dest, "%s/%s", destpath, file_basename(filename));

  if (strcmp(dest, filename) == 0)
    return;

  // Open files and copy...
  if ((filename = file_find(Path, filename)) == NULL)
    return;

  if ((in = fopen(filename, "rb")) == NULL)
    return;

  if ((out = fopen(dest, "wb")) == NULL)
  {
    fclose(in);
    return;
  }

  while ((nbytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
    fwrite(buffer, 1, nbytes, out);

  fclose(in);
  fclose(out);
}


//
// 'hdImage::find()' - Find an image file in memory...
//

hdImage *			// O - Pointer to image
hdImage::find(const char *uri,	// I - Name of image file
              int        load_data)// I - 1 = load image data
{
  hdImage	key,		// Search key...
		*keyptr,	// Pointer to search key...
		**match;	// Matching image


  // Range check...
  if (filename == NULL)
    return (NULL);

  if (filename[0] == '\0')	// Microsoft VC++ runtime bug workaround...
    return (NULL);

  // See if we've already loaded it...
  if (num_images_ > 0)
  {
    key.uri = uri;
    keyptr  = &key;

    match = (hdImage **)bsearch(&keyptr, images_, num_images_, sizeof(hdImage *),
                                (int (*)(const void *, const void *))compare);
    if (match)
    {
      if (load_data && !(*match)->pixels_)
        (*match)->load();

      return (*match);
    }
  }

  return ((hdImage *)0);
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


#if 0
//
// 'image_load()' - Load an image file from disk...
//

hdImage *			// O - Pointer to image
image_load(const char *filename,// I - Name of image file
           int        gray,	// I - 0 = color, 1 = grayscale
           int        load_data)// I - 1 = load image data, 0 = just info
{
  FILE		*fp;		// File pointer
  uchar		header[16];	// First 16 bytes of file
  hdImage	*img,		// New image buffer
		key,		// Search key...
		*keyptr,	// Pointer to search key...
		**match,	// Matching image
		**temp;		// Temporary array pointer
  int		status;		// Status of load...
  const char	*realname;	// Real filename


  // Range check...
  if (filename == NULL)
    return (NULL);

  if (filename[0] == '\0')	// Microsoft VC++ runtime bug workaround...
    return (NULL);

  // See if we've already loaded it...
  if (num_images > 0)
  {
    strcpy(key.filename, filename);
    keyptr = &key;

    match = (hdImage **)bsearch(&keyptr, images, num_images, sizeof(hdImage *),
                                (int (*)(const void *, const void *))image_compare);
    if (match != NULL && (!load_data || (*match)->pixels))
    {
      (*match)->use ++;
      return (*match);
    }
  }
  else
    match = NULL;

 //
  * Figure out the file type...
 

  if ((realname = file_find(Path, filename)) == NULL)
  {
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to find image file \"%s\"!", filename);
    return (NULL);
  }

  if ((fp = fopen(realname, "rb")) == NULL)
  {
    progress_error(HD_ERROR_FILE_NOT_FOUND,
                   "Unable to open image file \"%s\" (%s) for reading!",
		   filename, realname);
    return (NULL);
  }

  if (fread(header, 1, sizeof(header), fp) == 0)
  {
    progress_error(HD_ERROR_READ_ERROR,
                   "Unable to read image file \"%s\"!", filename);
    fclose(fp);
    return (NULL);
  }

  rewind(fp);

  // See if the images array needs to be resized...
  if (!match)
  {
    if (num_images >= alloc_images)
    {
      // Yes...
      alloc_images += ALLOC_FILES;

      if (num_images == 0)
	temp = (hdImage **)malloc(sizeof(hdImage *) * alloc_images);
      else
	temp = (hdImage **)realloc(images, sizeof(hdImage *) * alloc_images);

      if (temp == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
	               "Unable to allocate memory for %d images - %s",
                       alloc_images, strerror(errno));
	fclose(fp);
	return (NULL);
      }

      images = temp;
    }

    // Allocate memory...
    img = (hdImage *)calloc(sizeof(hdImage), 1);

    if (img == NULL)
    {
      progress_error(HD_ERROR_READ_ERROR, "Unable to allocate memory for \"%s\"",
                     filename);
      fclose(fp);
      return (NULL);
    }

    images[num_images] = img;

    strcpy(img->filename, filename);
    img->use = 1;
  }
  else
    img = *match;

  // Load the image as appropriate...
  if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0)
    status = image_load_gif(img,  fp, gray, load_data);
  else if (memcmp(header, "BM", 2) == 0)
    status = image_load_bmp(img, fp, gray, load_data);
  else if (memcmp(header, "\211PNG", 4) == 0)
    status = image_load_png(img, fp, gray, load_data);
  else if (memcmp(header, "\377\330\377", 3) == 0 &&	// Start-of-Image
	   header[3] >= 0xe0 && header[3] <= 0xef)	// APPn
    status = image_load_jpeg(img, fp, gray, load_data);
  else
  {
    progress_error(HD_ERROR_BAD_FORMAT, "Unknown image file format for \"%s\"!", filename);
    fclose(fp);
    free(img);
    return (NULL);
  }

  fclose(fp);

  if (status)
  {
    progress_error(HD_ERROR_READ_ERROR, "Unable to load image file \"%s\"!", filename);
    if (!match)
      free(img);
    return (NULL);
  }

  if (!match)
  {
    num_images ++;
    if (num_images > 1)
      qsort(images, num_images, sizeof(hdImage *),
            (int (*)(const void *, const void *))image_compare);
  }

  return (img);
}


//
// 'image_need_mask()' - Allocate memory for the image mask...
//

static void
image_need_mask(hdImage *img)	// I - Image to add mask to
{
  int	size;			// Byte size of mask image


  if (img == NULL || img->mask != NULL)
    return;

 // 
  * Figure out the size of the mask image, and then allocate and set all the
  * bits needed...
 

  img->maskwidth = (img->width + 7) / 8;
  size           = img->maskwidth * img->height;
  
  img->mask = (uchar *)calloc(size, 1);
}


//
// 'image_set_mask()' - Clear a bit in the image mask.
//

static void
image_set_mask(hdImage *img,	// I - Image to operate on
               int     x,	// I - X coordinate
               int     y)	// I - Y coordinate
{
  uchar		*maskptr;	// Pointer into mask image
  static uchar	masks[8] =	// Masks for each bit
		{
		  0x80, 0x40, 0x20, 0x10,
		  0x08, 0x04, 0x02, 0x01
		};


  if (img == NULL || img->mask == NULL || x < 0 || x >= img->width ||
      y < 0 || y > img->height)
    return;

  maskptr  = img->mask + y * img->maskwidth + x / 8;
  *maskptr |= masks[x & 7];
}


//
// 'image_unload()' - Unload an image from memory.
//

void
image_unload(hdImage *img)	// I - Image
{
  if (!img)
    return;

  if (!img->use || !img->pixels)
    return;

  if (img->obj)
    img->use = 0;
  else
    img->use --;

  if (img->use)
    return;

  free(img->pixels);
  img->pixels = NULL;
}
#endif // 0


//
// End of "$Id: image.cxx,v 1.15 2001/12/07 18:26:58 mike Exp $".
//
