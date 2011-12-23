//
// "$Id$"
//
//   Generic array code for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdArray::hdArray()  - Copy the array.
//   hdArray::~hdArray() - Free all memory used by the array.
//   hdArray::add()      - Insert or append an element to the array.
//   hdArray::find()     - Find an element in the array.
//   hdArray::remove()   - Remove an element from the array.
//   hdArray::restore()  - Reset the current element to the last save.
//   hdArray::save()     - Mark the current element for a later
//                         hdArray::Restore.
//

//
// Include necessary headers...
//

#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// 'hdArray::hdArray()' - Create a new array.
//

hdArray::hdArray(hdArrayFunc f,		// I - Comparison function
                 int         min_alloc)	// I - Minimum allocation
{
  alloc_elements_ = 0;
  compare_        = f;
  current_        = -1;
  elements_       = NULL;
  insert_         = -1;
  min_alloc_      = min_alloc;
  num_elements_   = 0;
  num_saved_      = 0;
  unique_         = 1;
}


//
// 'hdArray::hdArray()' - Copy the array.
//

hdArray::hdArray(hdArray &a)		// I - Array
{
  // Copy the array...
  compare_      = a.compare_;
  current_      = a.current_;
  insert_       = a.insert_;
  unique_       = a.unique_;
  min_alloc_    = a.min_alloc_;
  num_elements_ = a.num_elements_;
  num_saved_    = a.num_saved_;

  memcpy(saved_, a.saved_, sizeof(saved_));

  if (a.num_elements_)
  {
    // Copy the elements...
    if (a.num_elements_ < min_alloc_)
      alloc_elements_ = min_alloc_;
    else
      alloc_elements_ = a.num_elements_;

    elements_ = (void **)malloc(alloc_elements_ * sizeof(void *));

    memcpy(elements_, a.elements_, num_elements_ * sizeof(void *));
  }
  else
  {
    alloc_elements_ = 0;
    num_elements_   = 0;
  }
}


//
// 'hdArray::~hdArray()' - Free all memory used by the array.
//

hdArray::~hdArray()
{
  // Free the array of element pointers - the caller is responsible
  // for freeing the elements themselves...
  if (alloc_elements_)
    free(elements_);
}


//
// 'hdArray::add()' - Insert or append an element to the array.
//

bool					// O - True on success, false on failure
hdArray::add(void *e,			// I - Element to add
	     bool insert)		// I - Insert instead of append?
{
  int	i,				// Looping var
	acurrent,			// Current element
	diff;				// Comparison with acurrent element


  // Verify we have room for the new element...
  if (num_elements_ >= alloc_elements_)
  {
    // Allocate additional elements; start with 16 elements, then
    // double the size until 1024 elements, then add 1024 elements
    // thereafter...
    void	**temp;			// New array elements
    int		tcount;			// New allocation count


    if (alloc_elements_ == 0)
    {
      tcount = min_alloc_;
      temp   = (void **)malloc(tcount * sizeof(void *));
    }
    else
    {
      if (alloc_elements_ < 1024)
        tcount = alloc_elements_ * 2;
      else
        tcount = alloc_elements_ + 1024;

      temp = (void **)realloc(elements_, tcount * sizeof(void *));
    }

    if (!temp)
      return (false);

    alloc_elements_ = tcount;
    elements_       = temp;
  }

  // Find the insertion point for the new element; if there is no
  // compare function or elements, just add it to the beginning or end...
  if (!num_elements_ || !compare_)
  {
    // No elements or comparison function, insert/append as needed...
    if (insert)
      acurrent = 0;			// Insert at beginning
    else
      acurrent = num_elements_;		// Append to the end
  }
  else
  {
    // Do a binary search for the insertion point...
    acurrent = find(e, insert_, &diff);

    if (diff > 0)
    {
      // Insert after the acurrent element...
      acurrent ++;
    }
    else if (!diff)
    {
      // Compared equal, make sure we add to the begining or end of
      // the acurrent run of equal elements...
      unique_ = 0;

      if (insert)
      {
        // Insert at beginning of run...
	while (acurrent > 0 && !(*(compare_))(e, elements_[acurrent - 1]))
          acurrent --;
      }
      else
      {
        // Append at end of run...
	do
	{
          acurrent ++;
	}
	while (acurrent < num_elements_ &&
               !(*(compare_))(e, elements_[acurrent]));
      }
    }
  }

  // Insert or append the element...
  if (acurrent < num_elements_)
  {
    // Shift other elements to the right...
    memmove(elements_ + acurrent + 1, elements_ + acurrent,
            (num_elements_ - acurrent) * sizeof(void *));

    if (current_ >= acurrent)
      current_ ++;

    for (i = 0; i < num_saved_; i ++)
      if (saved_[i] >= acurrent)
	saved_[i] ++;
  }

  elements_[acurrent] = e;
  num_elements_ ++;
  insert_ = acurrent;

  return (true);
}


//
// 'hdArray::find()' - Find an element in the array.
//

void *					// O - Element found or NULL
hdArray::find(void *e)			// I - Element
{
  int	temp,				// Current element
	diff;				// Difference


  // Range check input...
  if (!e)
    return (NULL);

  // See if we have any elements...
  if (!num_elements_)
    return (NULL);

  // Yes, look for a match...
  temp = find(e, current_, &diff);
  if (!diff)
  {
    // Found a match!  If the array does not contain unique values, find
    // the first element that is the same...
    if (!unique_ && compare_)
    {
      // The array is not unique, find the first match...
      while (temp > 0 && !(*(compare_))(e, elements_[temp - 1]))
        temp --;
    }

    current_ = temp;

    return (elements_[temp]);
  }
  else
  {
    // No match...
    current_ = -1;

    return (NULL);
  }
}


//
// 'hdArray::find()' - Find an element in the array.
//

int					// O - Index of match
hdArray::find(void *e,			// I - Element
	      int  fprev,		// I - Previous index
	      int  *rdiff)		// O - Difference of match
{
  int	left,				// Left side of search
	right,				// Right side of search
	fcurrent,			// Current element
	diff;				// Comparison with fcurrent element


  if (compare_)
  {
    // Do a binary search for the element...
    if (fprev >= 0 && fprev < num_elements_)
    {
      // Start search on either side of fprevious...
      if ((diff = (*(compare_))(e, elements_[fprev])) == 0 ||
          (diff < 0 && fprev == 0) ||
	  (diff > 0 && fprev == (num_elements_ - 1)))
      {
        // Exact or edge match, return it!
	*rdiff = diff;

	return (fprev);
      }
      else if (diff < 0)
      {
        // Start with fprevious on right side...
	left  = 0;
	right = fprev;
      }
      else
      {
        // Start wih fprevious on left side...
        left  = fprev;
	right = num_elements_ - 1;
      }
    }
    else
    {
      // Start search in the middle...
      left  = 0;
      right = num_elements_ - 1;
    }

    do
    {
      fcurrent = (left + right) / 2;
      diff    = (*(compare_))(e, elements_[fcurrent]);

      if (diff == 0)
	break;
      else if (diff < 0)
	right = fcurrent;
      else
	left = fcurrent;
    }
    while ((right - left) > 1);

    if (diff != 0)
    {
      // Check the last 1 or 2 elements...
      if ((diff = (*(compare_))(e, elements_[left])) <= 0)
        fcurrent = left;
      else
      {
        diff    = (*(compare_))(e, elements_[right]);
        fcurrent = right;
      }
    }
  }
  else
  {
    // Do a linear pointer search...
    diff = 1;

    for (fcurrent = 0; fcurrent < num_elements_; fcurrent ++)
      if (elements_[fcurrent] == e)
      {
        diff = 0;
        break;
      }
  }

  // Return the closest element and the difference...
  *rdiff = diff;

  return (fcurrent);
}


//
// 'hdArray::remove()' - Remove an element from the array.
//

bool					// O - True on success, false on failure
hdArray::remove(void *e)		// I - Element
{
  int	i,				// Looping var
	temp,				// Current element
	diff;				// Difference


  // Range check input...
  if (!e)
    return (false);

  // See if the element is in the array...
  if (!num_elements_)
    return (false);

  temp = find(e, current_, &diff);
  if (diff)
    return (false);

  // Yes, now remove it...
  num_elements_ --;

  if (temp < num_elements_)
    memmove(elements_ + temp, elements_ + temp + 1,
            (num_elements_ - temp) * sizeof(void *));

  if (temp <= current_)
    current_ --;

  if (temp < insert_)
    insert_ --;
  else if (temp == insert_)
    insert_ = -1;

  for (i = 0; i < num_saved_; i ++)
    if (temp <= saved_[i])
      saved_[i] --;

  if (num_elements_ <= 1)
    unique_ = 1;

  return (true);
}


//
// 'hdArray::restore()' - Reset the current element to the last save.
//

bool					// O - True on success, false on failure
hdArray::restore()
{
  if (num_saved_ <= 0)
    return (false);

  num_saved_ --;
  current_ = saved_[num_saved_];

  return (true);
}


//
// 'hdArray::save()' - Mark the current element for a later hdArray::Restore.
//

bool					// O - True on success, false on failure
hdArray::save()
{
  if (num_saved_ >= HD_ARRAY_MAXSAVE)
    return (false);

  saved_[num_saved_] = current_;
  num_saved_ ++;

  return (true);
}


//
// End of "$Id$".
//
