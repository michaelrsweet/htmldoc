//
// String definitions for HTMLDOC, a HTML document processing program.
//
// Copyright © 2011-2024 by Michael R Sweet.
// Copyright © 1997-2010 by Easy Software Products.  All rights reserved.
//
// This program is free software.  Distribution and use rights are outlined in
// the file "COPYING".
//

#ifndef _HDSTRING_H_
#  define _HDSTRING_H_
#  include "config.h"
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <ctype.h>
#  ifdef HAVE_STRINGS_H
#    include <strings.h>
#  endif // HAVE_STRINGS_H
#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus


//
// Define some compatibility macros for Microsoft Windows...
//

#  ifdef WIN32
#    define strcasecmp(s,t)	stricmp(s,t)
#    define strncasecmp(s,t,n)	strnicmp(s,t,n)
#    define snprintf		_snprintf
#    define vsnprintf		_vsnprintf
#  endif // WIN32


//
// Implementation of strcpy() that allows for overlapping buffers.
//

extern void	hd_strcpy(char *dst, const char *src);


//
// Simple string pool functions to replace strdup/free usage in most places...
//

extern char	*hd_strdup(const char *);
extern void	hd_strfreeall(void);
extern size_t	hd_strgetsize(void);


//
// Standard string functions that might not be available...
//

#  ifndef HAVE_SNPRINTF
extern int	hd_snprintf(char *, size_t, const char *, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif // __GNUC__
;
#    define snprintf hd_snprintf
#  endif // !HAVE_SNPRINTF

#  ifndef HAVE_STRCASECMP
extern int	hd_strcasecmp(const char *, const char *);
#    define strcasecmp hd_strcasecmp
#  endif // !HAVE_STRCASECMP

#  ifndef HAVE_STRLCAT
extern size_t hd_strlcat(char *, const char *, size_t);
#    define strlcat hd_strlcat
#  endif // !HAVE_STRLCAT

#  ifndef HAVE_STRLCPY
extern size_t hd_strlcpy(char *, const char *, size_t);
#    define strlcpy hd_strlcpy
#  endif // !HAVE_STRLCPY

#  ifndef HAVE_STRNCASECMP
extern int	hd_strncasecmp(const char *, const char *, size_t n);
#    define strncasecmp hd_strncasecmp
#  endif // !HAVE_STRNCASECMP

#  ifndef HAVE_VSNPRINTF
extern int	hd_vsnprintf(char *, size_t, const char *, va_list);
#    define vsnprintf hd_vsnprintf
#  endif // !HAVE_VSNPRINTF


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !_HDSTRING_H_
