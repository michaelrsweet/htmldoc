//
// "$Id: png2fti.cxx,v 1.1.2.5 2004/06/14 16:03:10 mike Exp $"
//
// Simple utility which converts a PNG or XPM icon file to a 3D SGI FTI
// icon file.
//
// Copyright 2004 by Michael Sweet.
//

#include <stdio.h>
#include <string.h>
#include <FL/Fl_Shared_Image.H>


//#define DEBUG
#ifdef DEBUG
#  define DEBUG_printf(x) printf x
#else
#  define DEBUG_printf(x)
#endif // DEBUG


//
// 'print_vertex()' - Print an image vertex...
//

void
print_vertex(Fl_Shared_Image *img,	// I  - Shared image
             int             x,		// I  - X position in image
	     int             y,		// I  - Y position in image
	     const float     matrix[2][3],
					// I  - Transform matrix
	     float           &xmin,	// IO - Minimum X
	     float           &ymin,	// IO - Minimum Y
	     float           &xmax,	// IO - Maximum X
	     float           &ymax)	// IO - Maximum Y
{
  float		fx, fy;			// Floating point X/Y
  float		tx, ty;			// Transformed x/y


  // Map integer coords to floating point...
  fx = 100.0f * (float)x / (float)(img->w() - 1);
  fy = 100.0f - 100.0f * (float)y / (float)(img->h() - 1);

  // Project x, y in image space to 3D using the current matrix...
  tx = matrix[0][0] * fx + matrix[0][1] * fy + matrix[0][2];
  ty = matrix[1][0] * fx + matrix[1][1] * fy + matrix[1][2];

  // Update the bounding box...
  if (tx < xmin)
    xmin = tx;
  if (tx > xmax)
    xmax = tx;

  if (ty < ymin)
    ymin = ty;
  if (ty > ymax)
    ymax = ty;

  // Write the current vertex...
  DEBUG_printf(("\t# %d,%d\n", x, y));
  printf("\tvertex(%.2f, %.2f);\n", tx, ty);
}


//
// 'main()' - Convert a PNG image to FTI format...
//

int					// O - Exit status
main(int  argc,				// I - Number of command-line args
     char *argv[])			// I - Command-line arguments
{
  Fl_Shared_Image	*img;		// PNG icon
  int			x, y;		// X & Y in image
  int			startx,		// Starting X in image
			endy,		// Ending Y in image
			tempy;		// Current Y in image
  int			c,		// Current color
			temp;		// Temporary color
  const uchar		*data;		// Pointer into image
  float			xmin, xmax,	// Bounding box
			ymin, ymax;
  float			matrix[2][3];	// Transform matrix
  int			lines[256][768],// Lines (color, startx, endx)
			*lineptr;	// Pointer into line
  int			polys[256];	// Number of polygons per line
  int			did_poly;	// Non-zero if we did a polygon


  if (argc != 3 ||
      (strcmp(argv[1], "-c") && strcmp(argv[1], "-d") && strcmp(argv[1], "-o")))
  {
    puts("Usage: png2fti -[cdo] filename.png >filename.fti");
    puts("       -c = closed exec");
    puts("       -d = document");
    puts("       -o = open exec");
    return (1);
  }

  fl_register_images();

  img = Fl_Shared_Image::get(argv[2]);
  if (!img || !img->count() || !img->w() || !img->h())
  {
    printf("Unable to open image file \"%s\"!\n", argv[2]);
    return (1);
  }

  if (img->count() != 1)
  {
    puts("Sorry, indexed images are not supported at this time.");
    return (1);
  }

  xmin = 1000.0f;
  xmax = -1000.0f;
  ymin = 1000.0f;
  ymax = -1000.0f;

  printf("# Converted using \"png2fti %s %s\"...\n\n", argv[1], argv[2]);

  // Create the proper transform for the specified type of icon...
  switch (argv[1][1])
  {
    case 'c' : // Closed exec
        matrix[0][0] = 1.0f / 3.0f;
	matrix[0][1] = -1.0f / 3.0f;
	matrix[0][2] = 49.0f;

        matrix[1][0] = 0.5f / 3.0f;
	matrix[1][1] = 0.5f / 3.0f;
	matrix[1][2] = 14.5f;
        break;

    case 'd' : // Document
        matrix[0][0] = 1.0f / 3.0f;
	matrix[0][1] = 0.0f;
	matrix[0][2] = 42.5f;

        matrix[1][0] = 0.5f / 3.0f;
	matrix[1][1] = 1.0f / 3.0f;
	matrix[1][2] = 26.0f;
        break;

    case 'o' : // Open exec
        matrix[0][0] = 1.0f / 3.0f;
	matrix[0][1] = 0.0f;
	matrix[0][2] = 14.0f;

        matrix[1][0] = 0.5f / 3.0f;
	matrix[1][1] = 1.0f / 3.0f;
	matrix[1][2] = 38.0f;
        break;
  }

  // Loop through grayscale or RGB image and add polys...
  for (y = 0, data = (const uchar *)(*(img->data())); y < img->h(); y ++, data += img->ld())
  {
    polys[y] = 0;
    lineptr  = lines[y];

    for (x = 0, startx = 0, c = -1; x < img->w(); x ++, data += img->d())
    {
      switch (img->d())
      {
        case 2 :
	    if (data[1] < 128)
	    {
	      temp = -1;
	      break;
	    }

        case 1 :
            temp = fl_gray_ramp(data[0] * (FL_NUM_GRAY - 1) / 255);
	    break;

        default :
	    if (data[3] < 128)
	    {
	      temp = -1;
	      break;
	    }

	case 3 :
	    if (data[0] == data[1] && data[1] == data[2])
              temp = fl_gray_ramp(data[0] * (FL_NUM_GRAY - 1) / 255);
	    else
              temp = fl_color_cube(data[0] * (FL_NUM_RED - 1) / 255,
	                           data[1] * (FL_NUM_GREEN - 1) / 255,
	                           data[2] * (FL_NUM_BLUE - 1) / 255);
	    break;
      }

      if (temp != c)
      {
        if (c != -1)
	{
          polys[y] ++;
	  *lineptr++ = c;
	  *lineptr++ = startx;
	  *lineptr++ = x;
        }

        c      = temp;
	startx = x;
      }
    }

    if (x > startx && c != -1)
    {
      polys[y] ++;
      *lineptr++ = c;
      *lineptr++ = startx;
      *lineptr++ = x;
    }
  }

#if 1
  // Now loop through the polygons and find polys with a common prefix color
  for (did_poly = 1; did_poly;)
  {
    for (y = 0, did_poly = 0; y < img->h();)
    {
      if (polys[y] == 0)
      {
        y ++;
        continue;
      }

      fprintf(stderr, "\ry = %d      ", y);
      fflush(stderr);

      did_poly = 1;

      for (endy = y + 1;
           endy < (img->h() - 1) && polys[endy] > 0 && lines[y][0] == lines[endy][0];
	   endy ++)
	if (lines[y][1] > lines[endy][2] ||
	    lines[y][2] < lines[endy][1])
	  break;

      if ((endy - y) == 1)
      {
        // Just do a single row...
	DEBUG_printf(("# %d,%d to %d,%d...\n", lines[y][1], y, lines[y][2],
	              y + 1));
	printf("color(%d);\n", lines[y][0]);
	puts("bgnoutlinepolygon();");
	print_vertex(img, lines[y][1], y, matrix, xmin, ymin, xmax, ymax);
	print_vertex(img, lines[y][2], y, matrix, xmin, ymin, xmax, ymax);
	print_vertex(img, lines[y][2], y + 1, matrix, xmin, ymin, xmax, ymax);
	print_vertex(img, lines[y][1], y + 1, matrix, xmin, ymin, xmax, ymax);
	printf("endoutlinepolygon(%d);\n", lines[y][0]);
      }
      else
      {
        // Combine multiple rows...
#ifdef DEBUG
        for (tempy = y; tempy < endy; tempy ++)
	  printf("# %d,%d to %d,%d...\n", lines[tempy][1], tempy,
	         lines[tempy][2], tempy + 1);
#endif // DEBUG

	printf("color(%d);\n", lines[y][0]);
	puts("bgnpolygon();");
	print_vertex(img, lines[y][1], y, matrix, xmin, ymin, xmax, ymax);

        for (tempy = y; tempy < endy; tempy ++)
	{
	  print_vertex(img, lines[tempy][2], tempy, matrix, xmin, ymin, xmax, ymax);
	  while (tempy < (endy - 2) && lines[tempy][2] == lines[tempy + 1][2])
	    tempy ++;
	  if (tempy >= (endy - 1) || lines[tempy][2] != lines[tempy + 1][2])
	    print_vertex(img, lines[tempy][2], tempy + 1, matrix, xmin, ymin, xmax, ymax);
	}

        for (tempy = endy - 1; tempy >= y; tempy --)
	{
	  print_vertex(img, lines[tempy][1], tempy + 1, matrix, xmin, ymin, xmax, ymax);
	  while (tempy > y && lines[tempy][1] == lines[tempy - 1][1])
	    tempy --;
	  if (tempy > y && lines[tempy][1] != lines[tempy - 1][1])
	    print_vertex(img, lines[tempy][1], tempy, matrix, xmin, ymin, xmax, ymax);
	}

	puts("endpolygon();");
      }

      for (tempy = y; tempy < endy; tempy ++)
      {
        polys[tempy] --;
	if (polys[tempy] > 0)
          memmove(lines[tempy], lines[tempy] + 3, 3 * polys[tempy] * sizeof(int));
      }

      y = endy;
    }
  }
#else
  for (y = 0; y < img->h(); y ++)
  {
    for (x = polys[y], lineptr = lines[y]; x > 0; x --, lineptr += 3)
    {
      printf("color(%d);\n", lineptr[0]);
      puts("bgnpolygon();");
      print_vertex(img, lineptr[1], y, matrix, xmin, ymin, xmax, ymax);
      print_vertex(img, lineptr[2], y, matrix, xmin, ymin, xmax, ymax);
      print_vertex(img, lineptr[2], y + 1, matrix, xmin, ymin, xmax, ymax);
      print_vertex(img, lineptr[1], y + 1, matrix, xmin, ymin, xmax, ymax);
      puts("endpolygon();");
    }
  }
#endif // 0

  printf("# Bounding box = [ %.2f %.2f %.2f %.2f ]\n", xmin, ymin, xmax, ymax);

  fputs("\rDone!          \n", stderr);

  return (0);
}


//
// End of "$Id: png2fti.cxx,v 1.1.2.5 2004/06/14 16:03:10 mike Exp $".
//
