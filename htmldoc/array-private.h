/*
 * Private array definitions for CUPS.
 *
 * Copyright 2011-2012 by Apple Inc.
 *
 * This program is free software.  Distribution and use rights are outlined in
 * the file "COPYING".
 */

#ifndef _CUPS_ARRAY_PRIVATE_H_
#  define _CUPS_ARRAY_PRIVATE_H_

/*
 * Include necessary headers...
 */

#  include "array.h"


/*
 * C++ magic...
 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Functions...
 */

extern int		_cupsArrayAddStrings(cups_array_t *a, const char *s,
			                     char delim) _CUPS_API_1_5;
extern cups_array_t	*_cupsArrayNewStrings(const char *s, char delim)
			                      _CUPS_API_1_5;

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !_CUPS_ARRAY_PRIVATE_H_ */
