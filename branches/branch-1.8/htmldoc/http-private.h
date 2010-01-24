/*
 * "$Id$"
 *
 *   Private HTTP definitions for HTMLDOC.
 *
 *   Copyright 1997-2010 by Easy Software Products.  All rights reserved.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *     Attn: HTMLDOC Licensing Information
 *     Easy Software Products
 *     516 Rio Grand Ct
 *     Morgan Hill, CA 95037 USA
 *
 *     http://www.htmldoc.org/
 */

#ifndef _CUPS_HTTP_PRIVATE_H_
#  define _CUPS_HTTP_PRIVATE_H_

/*
 * Include necessary headers...
 */

#  include "hdstring.h"
#  ifdef __sun
#    include <sys/select.h>
#  endif /* __sun */
#  include <limits.h>
#  ifdef WIN32
#    include <io.h>
#    include <winsock2.h>
#  else
#    include <unistd.h>
#    include <fcntl.h>
#    include <sys/socket.h>
#    define closesocket(f) close(f)
#  endif /* WIN32 */

#  if defined(__sgi) || (defined(__APPLE__) && !defined(_SOCKLEN_T))
/*
 * IRIX and MacOS X 10.2.x do not define socklen_t, and in fact use an int instead of
 * unsigned type for length values...
 */

typedef int socklen_t;
#  endif /* __sgi || (__APPLE__ && !_SOCKLEN_T) */

#  include "http.h"

#  if defined HAVE_LIBSSL
/*
 * The OpenSSL library provides its own SSL/TLS context structure for its
 * IO and protocol management.  However, we need to provide our own BIO
 * (basic IO) implementation to do timeouts...
 */

#    include <openssl/err.h>
#    include <openssl/rand.h>
#    include <openssl/ssl.h>

typedef SSL http_tls_t;

extern BIO_METHOD *_httpBIOMethods(void);

#  elif defined HAVE_GNUTLS
/*
 * The GNU TLS library is more of a "bare metal" SSL/TLS library...
 */
#    include <gnutls/gnutls.h>

typedef struct
{
  gnutls_session	session;	/* GNU TLS session object */
  void			*credentials;	/* GNU TLS credentials object */
} http_tls_t;

extern ssize_t	_httpReadGNUTLS(gnutls_transport_ptr ptr, void *data,
		                size_t length);
extern ssize_t	_httpWriteGNUTLS(gnutls_transport_ptr ptr, const void *data,
		                 size_t length);

#  elif defined(HAVE_CDSASSL)
/*
 * Darwin's Security framework provides its own SSL/TLS context structure
 * for its IO and protocol management...
 */

#    include <Security/SecureTransport.h>

typedef struct				/**** CDSA connection information ****/
{
  SSLContextRef		session;	/* CDSA session object */
  CFArrayRef		certsArray;	/* Certificates array */
} http_tls_t;

extern OSStatus	_httpReadCDSA(SSLConnectionRef connection, void *data,
		              size_t *dataLength);
extern OSStatus	_httpWriteCDSA(SSLConnectionRef connection, const void *data,
		               size_t *dataLength);
#  endif /* HAVE_LIBSSL */

/*
 * Some OS's don't have hstrerror(), most notably Solaris...
 */

#  ifndef HAVE_HSTRERROR
extern const char *_hd_hstrerror(int error);
#    define hstrerror _hd_hstrerror
#  elif defined(_AIX) || defined(__osf__)
/*
 * AIX and Tru64 UNIX don't provide a prototype but do provide the function...
 */
extern const char *hstrerror(int error);
#  endif /* !HAVE_HSTRERROR */

#endif /* !_CUPS_HTTP_PRIVATE_H_ */

/*
 * End of "$Id$".
 */
