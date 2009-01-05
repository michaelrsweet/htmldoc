//
// "$Id$"
//
// Generic array definitions for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2008 Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//

#ifndef _HTMLDOC_ARRAY_H_
#  define _HTMLDOC_ARRAY_H_


//
// Constants...
//

#  define HD_ARRAY_MAXSAVE	32	// Maximum number of save levels


//
// Types and structures...
//

typedef int (*hdArrayFunc)(void *first, void *second);
					//// Array comparison function

class hdArray				//// Generic array container
{
  protected:

  int		num_elements_,		// Number of array elements
		alloc_elements_,	// Allocated array elements
		min_alloc_,		// Minimum allocation
		current_,		// Current element
		insert_,		// Last inserted element
		unique_,		// Are all elements unique?
		num_saved_,		// Number of saved elements
		saved_[HD_ARRAY_MAXSAVE];
					// Saved elements
  void		**elements_;		// Array elements
  hdArrayFunc	compare_;		// Element comparison function

  int		find(void *e, int prev, int *rdiff);

  public:

		hdArray(hdArrayFunc f = (hdArrayFunc)0, int min_alloc = 16);
		hdArray(hdArray &a);
		~hdArray();

  bool		add(void *e, bool insert = false);
  void		clear()
		{
		  num_elements_ = 0;
		  current_      = -1;
		  insert_       = -1;
		  unique_       = 1;
		  num_saved_    = 0;
		}
  int		count() { return (num_elements_); }
  void		*current()
		{
		  if (num_elements_ == 0 ||
		      current_ < 0 || current_ >= num_elements_)
		    return (0);
		  else
		    return (elements_[current_]);
		}
  void		*find(void *e);
  void		*first() { current_ = 0; return (current()); }
  void		*index(int n) { current_ = n; return (current()); }
  void		*last() { current_ = num_elements_ - 1; return (current()); }
  void		*next()
		{
		  if (current_ < num_elements_)
		    current_ ++;

		  return (current());
		}
  void		*prev()
		{
		  if (current_ >= 0)
		    current_ --;

		  return (current());
		}
  bool		remove(void *e);
  bool		restore();
  bool		save();
};

#endif /* !_HTMLDOC_ARRAY_H_ */

//
// End of "$Id$".
//
