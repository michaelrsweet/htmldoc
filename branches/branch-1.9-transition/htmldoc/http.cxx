//
// "$Id: http.cxx,v 1.7.2.2 2004/03/30 03:49:15 mike Exp $"
//
//   Hyper-Text Transport Protocol class routines for HTMLDOC.
//
//   Copyright 1997-2004 by Easy Software Products, all rights reserved.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "LICENSE.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: CUPS Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: cups-info@cups.org
//         WWW: http://www.cups.org
//
// Contents:
//
//   hdHTTP::clear_fields()       - Clear HTTP request/response fields.
//   hdHTTP::initialize()         - Initialize the HTTP interface library.
//   hdHTTP::check()              - Check to see if there is a pending
//                                  response from the server.
//   hdHTTP::~hdHTTP()            - Close the HTTP connection and free memory...
//   hdHTTP::hdHTTP()             - Connect to a HTTP server.
//   hdHTTP::set_encryption()     - Set the required encryption on the link.
//   hdHTTP::reconnect()          - Reconnect to a HTTP server...
//   hdHTTP::separate()           - Separate a Universal Resource Identifier
//                                  into its components.
//   hdHTTP::get_sub_field()      - Get a sub-field value.
//   hdHTTP::set_field()          - Set the value of an HTTP header.
//   hdHTTP::send_delete()        - Send a DELETE request to the server.
//   hdHTTP::send_get()           - Send a GET request to the server.
//   hdHTTP::send_head()          - Send a HEAD request to the server.
//   hdHTTP::send_options()       - Send an OPTIONS request to the server.
//   hdHTTP::send_post()          - Send a POST request to the server.
//   hdHTTP::send_put()           - Send a PUT request to the server.
//   hdHTTP::send_trace()         - Send an TRACE request to the server.
//   hdHTTP::flush()              - Flush data from a HTTP connection.
//   hdHTTP::read()               - Read data from a HTTP connection.
//   hdHTTP::write()              - Write data to a HTTP connection.
//   hdHTTP::gets()               - Get a line of text from a HTTP connection.
//   hdHTTP::printf()             - Print a formatted string to a HTTP
//                                  connection.
//   hdHTTP::status_string()      - Return a short string describing a HTTP
//                                  status code.
//   hdHTTP::get_date_string()    - Get a formatted date/time string from a
//                                  time value.
//   hdHTTP::get_date_time()      - Get a time value from a formatted date/time string.
//   hdHTTP::update()             - Update the current HTTP state for incoming
//                                  data.
//   hdHTTP::decode64()           - Base64-decode a string.
//   hdHTTP::encode64()           - Base64-encode a string.
//   hdHTTP::get_content_length() - Get the amount of data remaining from
//                                  the content-length or transfer-encoding
//                                  fields.
//   hdHTTP::field_number()       - Return the field index for a field name.
//   hdHTTP::send()               - Send a request with all fields and the
//                                  trailing blank line.
//   hdHTTP::upgrade()            - Force upgrade to TLS encryption.
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "http.h"
#include "hdstring.h"
#include "debug.h"

#if !defined(WIN32)
#  include <signal.h>
#endif // !WIN32

#ifdef HAVE_LIBSSL
#  include <openssl/err.h>
#  include <openssl/rand.h>
#  include <openssl/ssl.h>
#endif // HAVE_LIBSSL


//
// Some operating systems have done away with the Fxxxx constants for
// the fcntl() call; this works around that "feature"...
//

#ifndef FNONBLK
#  define FNONBLK O_NONBLOCK
#endif // !FNONBLK


//
// Local globals...
//

static const char	*http_fields[] =
			{
			  "Accept-Language",
			  "Accept-Ranges",
			  "Authorization",
			  "Connection",
			  "Content-Encoding",
			  "Content-Language",
			  "Content-Length",
			  "Content-Location",
			  "Content-MD5",
			  "Content-Range",
			  "Content-Type",
			  "Content-Version",
			  "Cookie",
			  "Date",
			  "Expect",
			  "Host",
			  "If-Modified-Since",
			  "If-Unmodified-since",
			  "Keep-Alive",
			  "Last-Modified",
			  "Link",
			  "Location",
			  "Range",
			  "Referer",
			  "Retry-After",
			  "Set-Cookie",
			  "Transfer-Encoding",
			  "Upgrade",
			  "User-Agent",
			  "WWW-Authenticate"
			};
static const char	*http_days[7] =
			{
			  "Sun",
			  "Mon",
			  "Tue",
			  "Wed",
			  "Thu",
			  "Fri",
			  "Sat"
			};
static const char	*http_months[12] =
			{
			  "Jan",
			  "Feb",
			  "Mar",
			  "Apr",
			  "May",
			  "Jun",
		          "Jul",
			  "Aug",
			  "Sep",
			  "Oct",
			  "Nov",
			  "Dec"
			};


//
// 'hdHTTP::clear_fields()' - Clear HTTP request/response fields.
//

void
hdHTTP::clear_fields()
{
  memset(fields, 0, sizeof(fields));
  set_field(HD_HTTP_FIELD_HOST, hostname);
}


//
// 'hdHTTP::initialize()' - Initialize the HTTP interface library.
//

void
hdHTTP::initialize(void)
{
  static int	initialized = 0;	// Have we initialized things yet?
#ifdef HAVE_LIBSSL
#  ifndef WIN32
  struct timeval	curtime;	// Current time in microseconds
#  endif // !WIN32
  int			i;		// Looping var
  unsigned char		data[1024];	// Seed data
#endif // HAVE_LIBSSL

  if (!initialized)
    return;

  initialized = 1;

#if defined(WIN32)
  WSADATA	winsockdata;		// WinSock data


  WSAStartup(MAKEWORD(1,1), &winsockdata);
#elif defined(HAVE_SIGSET)
  sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
  struct sigaction	action;		// POSIX sigaction data


  //
  // Ignore SIGPIPE signals...
  //

  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);
#else
  signal(SIGPIPE, SIG_IGN);
#endif // WIN32

#ifdef HAVE_LIBSSL
  SSL_load_error_strings();
  SSL_library_init();

  //
  // Using the current time is a dubious random seed, but on some systems
  // it is the best we can do (on others, this seed isn't even used...)
  //

#  ifdef WIN32
  srand(GetTickCount());
#  else
  gettimeofday(&curtime, NULL);
  srand(curtime.tv_sec + curtime.tv_usec);
#  endif // WIN32
  for (i = 0; i < (int)sizeof(data); i ++)
    data[i] = rand(); // Yes, this is a poor source of random data...

  RAND_seed(&data, sizeof(data));
#endif // HAVE_LIBSSL
}


//
// 'hdHTTP::check()' - Check to see if there is a pending response from the server.
//

int				// O - 0 = no data, 1 = data available
hdHTTP::check()
{
  fd_set	input;		// Input set for select()
  struct timeval timeout;	// Timeout


  // First see if there is data in the buffer...
  if (used)
    return (1);

  // Then try doing a select() to poll the socket...
  FD_ZERO(&input);
  FD_SET(fd, &input);

  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;

  return (select(fd + 1, &input, NULL, NULL, &timeout) > 0);
}


//
// 'hdHTTP::~hdHTTP()' - Close the HTTP connection and free memory...
//

hdHTTP::~hdHTTP()
{
#ifdef HAVE_LIBSSL
  SSL_CTX	*context;	// Context for encryption
  SSL		*conn;		// Connection for encryption
#endif // HAVE_LIBSSL


#ifdef HAVE_LIBSSL
  if (tls)
  {
    conn    = (SSL *)(tls);
    context = SSL_get_SSL_CTX(conn);

    SSL_shutdown(conn);
    SSL_CTX_free(context);
    SSL_free(conn);

    tls = NULL;
  }
#endif // HAVE_LIBSSL

#ifdef WIN32
  closesocket(fd);
#else
  close(fd);
#endif // WIN32
}


//
// 'hdHTTP::hdHTTP()' - Connect to a HTTP server.
//

hdHTTP::hdHTTP(const char       *host,	// I - Host to connect to
               int              port,	// I - Port number
	       hdHTTPEncryption encrypt)// I - Type of encryption to use
{
  struct hostent	*h;		// Host address data


  // Make sure socket IO is initialized...
  initialize();

  // Initialize things...
  fd             = -1;
  blocking       = 1;
  error          = 0;
  activity       = time(NULL);
  state          = HD_HTTP_WAITING;
  status         = HD_HTTP_ERROR;
  version        = HD_HTTP_1_1;
  keep_alive     = HD_HTTP_KEEPALIVE_ON;
  data           = (char *)0;
  data_encoding  = HD_HTTP_ENCODE_LENGTH;
  data_remaining = 0;
  used           = 0;
  encryption     = port == 443 ? HD_HTTP_ENCRYPT_ALWAYS : encrypt;
  authtype       = HD_HTTP_AUTH_NONE;
  nonce_count    = 0;
  tls            = (void *)0;

  memset(&hostaddr, 0, sizeof(hostaddr));
  memset(hostname, 0, sizeof(hostname));
  memset(fields, 0, sizeof(fields));
  memset(buffer, 0, sizeof(buffer));
  memset(&md5_state, 0, sizeof(md5_state));
  memset(nonce, 0, sizeof(nonce));

  // Lookup the host...
  if (host)
    h = gethostbyname(host);
  else
    h = NULL;

  if (h == NULL)
  {
    // This hack to make users that don't have a localhost entry in
    // their hosts file or DNS happy...
    error = errno;

    if (strcasecmp(host, "localhost") != 0)
      return;
    else if ((h = gethostbyname("127.0.0.1")) == NULL)
      return;
  }

  // Verify that it is an IPv4 address (IPv6 support will come soon...)
  if (h->h_addrtype != AF_INET || h->h_length != 4)
    return;

  // Copy the hostname and port and then "reconnect"...
  strncpy(hostname, host, sizeof(hostname) - 1);
  memcpy((char *)&(hostaddr.sin_addr), h->h_addr, h->h_length);
  hostaddr.sin_family = h->h_addrtype;
#ifdef WIN32
  hostaddr.sin_port   = htons((u_short)port);
#else
  hostaddr.sin_port   = htons(port);
#endif // WIN32

  // Connect to the remote system...
  reconnect();
}


//
// 'hdHTTP::set_encryption()' - Set the required encryption on the link.
//

int					// O - -1 on error, 0 on success
hdHTTP::set_encryption(hdHTTPEncryption e)// I - New encryption preference
{
#ifdef HAVE_LIBSSL
  encryption = e;

  if ((encryption == HD_HTTP_ENCRYPT_ALWAYS && !tls) ||
      (encryption == HD_HTTP_ENCRYPT_NEVER && tls))
    return (reconnect());
  else if (encryption == HD_HTTP_ENCRYPT_REQUIRED && !tls)
    return (upgrade());
  else
    return (0);
#else
  if (e == HD_HTTP_ENCRYPT_ALWAYS || e == HD_HTTP_ENCRYPT_REQUIRED)
    return (-1);
  else
    return (0);
#endif // HAVE_LIBSSL
}


//
// 'hdHTTP::reconnect()' - Reconnect to a HTTP server...
//

int				// O - 0 on success, non-zero on failure
hdHTTP::reconnect()
{
  int		val;		// Socket option value
#ifdef HAVE_LIBSSL
  SSL_CTX	*context;	// Context for encryption
  SSL		*conn;		// Connection for encryption


  if (tls)
  {
    conn    = (SSL *)(tls);
    context = SSL_get_SSL_CTX(conn);

    SSL_shutdown(conn);
    SSL_CTX_free(context);
    SSL_free(conn);

    tls = NULL;
  }
#endif // HAVE_LIBSSL

  // Close any previously open socket...
  if (fd >= 0)
#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif // WIN32

  // Create the socket and set options to allow reuse.
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
#ifdef WIN32
    error  = WSAGetLastError();
#else
    error  = errno;
#endif // WIN32
    status = HD_HTTP_ERROR;
    return (-1);
  }

#ifdef FD_CLOEXEC
  fcntl(fd, F_SETFD, FD_CLOEXEC);	// Close this socket when starting
					// other processes...             
#endif // FD_CLOEXEC

  val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));

#ifdef SO_REUSEPORT
  val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
#endif // SO_REUSEPORT

  // Connect to the server...
  if (connect(fd, (struct sockaddr *)&(hostaddr), sizeof(hostaddr)) < 0)
  {
#ifdef WIN32
    error  = WSAGetLastError();
#else
    error  = errno;
#endif // WIN32
    status = HD_HTTP_ERROR;

#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif

    fd = -1;

    return (-1);
  }

  error  = 0;
  status = HD_HTTP_CONTINUE;

#ifdef HAVE_LIBSSL
  if (encryption == HD_HTTP_ENCRYPT_ALWAYS)
  {
    // Always do encryption via SSL.
    context = SSL_CTX_new(SSLv23_method());
    conn    = SSL_new(context);

    SSL_set_fd(conn, fd);
    if (SSL_connect(conn) != 1)
    {
      SSL_CTX_free(context);
      SSL_free(conn);

#ifdef WIN32
      error  = WSAGetLastError();
#else
      error  = errno;
#endif // WIN32
      status = HD_HTTP_ERROR;

#ifdef WIN32
      closesocket(fd);
#else
      close(fd);
#endif

      return (-1);
    }

    tls = conn;
  }
  else if (encryption == HD_HTTP_ENCRYPT_REQUIRED)
    return (upgrade());
#endif // HAVE_LIBSSL

  return (0);
}


//
// 'hdHTTP::separate()' - Separate a Universal Resource Identifier into its
//                        components.
//

void
hdHTTP::separate(const char *uri,	// I - Universal Resource Identifier
        	 char       *scheme,	// O - Method (http, https, etc.)
		 int        schemelen,	// I - Size of scheme buffer
		 char       *username,	// O - Username
		 int        usernamelen,// I - Size of username buffer
		 char       *host,	// O - Hostname
		 int        hostlen,	// I - Size of hostname buffer
		 int        *port,	// O - Port number to use
        	 char       *resource,	// O - Resource/filename
		 int        resourcelen)// I - Size of resource buffer
{
  char		*ptr;				// Pointer into string...
  const char	*atsign,			// @ sign
		*slash;				// Separator


  // Range check input...
  if (uri == NULL || scheme == NULL || username == NULL || host == NULL ||
      port == NULL || resource == NULL)
    return;

  // Grab the scheme portion of the URI...
  if (strncmp(uri, "//", 2) == 0)
  {
    // Workaround for common bad URLs...
    strncpy(scheme, "http", schemelen - 1);
    scheme[schemelen - 1] = '\0';
  }
  else
  {
    // Standard URI with scheme...
    for (ptr = host; *uri != ':' && *uri != '\0'; uri ++)
      if (ptr < (host + hostlen - 1))
        *ptr++ = *uri;

    *ptr = '\0';
    if (*uri == ':')
      uri ++;

    // If the scheme contains a period or slash, then it's probably
    // hostname/filename...
    if (strchr(host, '.') != NULL || strchr(host, '/') != NULL || *uri == '\0')
    {
      if ((ptr = strchr(host, '/')) != NULL)
      {
	strncpy(resource, ptr, resourcelen - 1);
	resource[resourcelen - 1] = '\0';
	*ptr = '\0';
      }
      else
	resource[0] = '\0';

      if (isdigit(*uri))
      {
        // OK, we have "hostname:port[/resource]"...
	*port = strtol(uri, (char **)&uri, 10);

	if (*uri == '/')
	{
          strncpy(resource, uri, resourcelen);
          resource[resourcelen - 1] = '\0';
	}
      }
      else
	*port = 80;

      strncpy(scheme, "http", schemelen - 1);
      scheme[schemelen - 1] = '\0';

      username[0] = '\0';
      return;
    }
    else
    {
      strncpy(scheme, host, schemelen - 1);
      scheme[schemelen - 1] = '\0';
    }
  }

  // If the URI starts with less than 2 slashes then it is a local resource...
  if (strncmp(uri, "//", 2) != 0)
  {
    strncpy(resource, uri, resourcelen - 1);
    resource[resourcelen - 1] = '\0';

    username[0] = '\0';
    host[0]     = '\0';
    *port       = 0;
    return;
  }

  // Grab the username, if any...
  while (*uri == '/')
    uri ++;

  if ((slash = strchr(uri, '/')) == NULL)
    slash = uri + strlen(uri);

  if ((atsign = strchr(uri, '@')) != NULL && atsign < slash)
  {
    // Got a username:password combo...
    for (ptr = username; uri < atsign; uri ++)
      if (ptr < (username + usernamelen - 1))
	*ptr++ = *uri;

    *ptr = '\0';

    uri = atsign + 1;
  }
  else
    username[0] = '\0';

  // Grab the hostname...
  for (ptr = host; *uri != ':' && *uri != '/' && *uri != '\0'; uri ++)
    if (ptr < (host + hostlen - 1))
      *ptr++ = *uri;

  *ptr = '\0';

  if (*uri != ':')
  {
    if (strcasecmp(scheme, "http") == 0)
      *port = 80;
    else if (strcasecmp(scheme, "https") == 0)
      *port = 443;
    else
      *port = 0;
  }
  else
  {
    // Parse port number...
    *port = strtol(uri, (char **)&uri, 10);
  }

  if (*uri == '\0')
  {
    // Hostname but no port or path...
    resource[0] = '/';
    resource[1] = '\0';
  }
  else
  {
    // The remaining portion is the resource string...
    strncpy(resource, uri, resourcelen);
    resource[resourcelen - 1] = '\0';
  }
}


//
// 'hdHTTP::get_sub_field()' - Get a sub-field value.
//

char *						// O - Value or NULL
hdHTTP::get_sub_field(hdHTTPField field,	// I - Field index
                      const char   *name,	// I - Name of sub-field
		      char         *value,	// O - Value string
		      int          valuelen)	// I - Value size
{
  const char	*fptr;			// Pointer into field
  char		temp[HD_MAX_VALUE],	// Temporary buffer for name
		*ptr;			// Pointer into string buffer


  if (field < HD_HTTP_FIELD_ACCEPT_LANGUAGE ||
      field > HD_HTTP_FIELD_WWW_AUTHENTICATE ||
      name == NULL || value == NULL)
    return (NULL);

  DEBUG_printf(("get_sub_field(%d, \"%s\", %p)\n",
                field, name, value));

  for (fptr = fields[field]; *fptr;)
  {
    // Skip leading whitespace...
    while (isspace(*fptr))
      fptr ++;

    if (*fptr == ',')
    {
      fptr ++;
      continue;
    }

    // Get the sub-field name...
    for (ptr = temp;
         *fptr && *fptr != '=' && !isspace(*fptr) && ptr < (temp + sizeof(temp) - 1);
         *ptr++ = *fptr++);

    *ptr = '\0';

    DEBUG_printf(("name = \"%s\"\n", temp));

    // Skip trailing chars up to the '='...
    while (isspace(*fptr))
      fptr ++;

    if (!*fptr)
      break;

    if (*fptr != '=')
      continue;

    // Skip = and leading whitespace...
    fptr ++;

    while (isspace(*fptr))
      fptr ++;

    if (*fptr == '\"')
    {
      // Read quoted string...
      for (ptr = value, fptr ++;
           *fptr && *fptr != '\"' && ptr < (value + valuelen - 1);
	   *ptr++ = *fptr++);

      *ptr = '\0';

      while (*fptr && *fptr != '\"')
        fptr ++;

      if (*fptr)
        fptr ++;
    }
    else
    {
      // Read unquoted string...
      for (ptr = value;
           *fptr && !isspace(*fptr) && *fptr != ',' && ptr < (value + valuelen - 1);
	   *ptr++ = *fptr++);

      *ptr = '\0';

      while (*fptr && !isspace(*fptr) && *fptr != ',')
        fptr ++;
    }

    DEBUG_printf(("value = \"%s\"\n", value));

    // See if this is the one...
    if (strcmp(name, temp) == 0)
      return (value);
  }

  value[0] = '\0';

  return (NULL);
}


//
// 'hdHTTP::set_field()' - Set the value of an HTTP header.
//

void
hdHTTP::set_field(hdHTTPField field,	// I - Field index
	          const char  *value)	// I - Value
{
  if (field < HD_HTTP_FIELD_ACCEPT_LANGUAGE ||
      field > HD_HTTP_FIELD_WWW_AUTHENTICATE ||
      value == NULL)
    return;

  strncpy(fields[field], value, HD_MAX_VALUE - 1);
  fields[field][HD_MAX_VALUE - 1] = '\0';
}


//
// 'hdHTTP::send_delete()' - Send a DELETE request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_delete(const char *uri)	// I - URI to delete
{
  return (send(HD_HTTP_DELETE, uri));
}


//
// 'hdHTTP::send_get()' - Send a GET request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_get(const char *uri)		// I - URI to get
{
  return (send(HD_HTTP_GET, uri));
}


//
// 'hdHTTP::send_head()' - Send a HEAD request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_head(const char *uri)	// I - URI for head
{
  return (send(HD_HTTP_HEAD, uri));
}


//
// 'hdHTTP::send_options()' - Send an OPTIONS request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_options(const char *uri)	// I - URI for options
{
  return (send(HD_HTTP_OPTIONS, uri));
}


//
// 'hdHTTP::send_post()' - Send a POST request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_post(const char *uri)	// I - URI for post
{
  get_content_length();

  return (send(HD_HTTP_POST, uri));
}


//
// 'hdHTTP::send_put()' - Send a PUT request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_put(const char *uri)		// I - URI to put
{
  get_content_length();

  return (send(HD_HTTP_PUT, uri));
}


//
// 'hdHTTP::send_trace()' - Send an TRACE request to the server.
//

int					// O - Status of call (0 = success)
hdHTTP::send_trace(const char *uri)	// I - URI for trace
{
  return (send(HD_HTTP_TRACE, uri));
}


//
// 'hdHTTP::flush()' - Flush data from a HTTP connection.
//

void
hdHTTP::flush()
{
  char	buffer[8192];	// Junk buffer


  while (read(buffer, sizeof(buffer)) > 0);
}


//
// 'hdHTTP::read()' - Read data from a HTTP connection.
//

int					// O - Number of bytes read
hdHTTP::read(char *b,			// I - Buffer for data
	     int  length)		// I - Maximum number of bytes
{
  int		bytes;			// Bytes read
  char		len[32];		// Length string


  DEBUG_printf(("hdHTTP::read(%p, %d)\n", buffer, length));

  if (b == NULL)
    return (-1);

  activity = time(NULL);

  if (length <= 0)
    return (0);

  if (data_encoding == HD_HTTP_ENCODE_CHUNKED &&
      data_remaining <= 0)
  {
    DEBUG_puts("hdHTTP::read: Getting chunk length...");

    if (gets(len, sizeof(len)) == NULL)
    {
      DEBUG_puts("hdHTTP::read: Could not get length!");
      return (0);
    }

    data_remaining = strtol(len, NULL, 16);
  }

  DEBUG_printf(("hdHTTP::read: data_remaining = %d\n", data_remaining));

  if (data_remaining == 0)
  {
    // A zero-length chunk ends a transfer; unless we are reading POST
    // data, go idle...
    if (data_encoding == HD_HTTP_ENCODE_CHUNKED)
      gets(len, sizeof(len));

    if (state == HD_HTTP_POST_RECV)
      state = HD_HTTP_POST_SEND;
    else
      state = HD_HTTP_WAITING;

    return (0);
  }
  else if (length > data_remaining)
    length = data_remaining;

  if (used > 0)
  {
    if (length > used)
      length = used;

    bytes = length;

    DEBUG_printf(("hdHTTP::read: grabbing %d bytes from input buffer...\n", bytes));

    memcpy(b, buffer, length);
    used -= length;

    if (used > 0)
      memcpy(buffer, buffer + length, used);
  }
#ifdef HAVE_LIBSSL
  else if (tls)
    bytes = SSL_read((SSL *)(tls), b, length);
#endif // HAVE_LIBSSL
  else
  {
    DEBUG_printf(("hdHTTP::read: reading %d bytes from socket...\n", length));
    bytes = recv(fd, b, length, 0);
    DEBUG_printf(("hdHTTP::read: read %d bytes from socket...\n", bytes));
  }

  if (bytes > 0)
    data_remaining -= bytes;
  else if (bytes < 0)
#ifdef WIN32
    error = WSAGetLastError();
#else
    error = errno;
#endif // WIN32

  if (data_remaining == 0)
  {
    if (data_encoding == HD_HTTP_ENCODE_CHUNKED)
      gets(len, sizeof(len));

    if (data_encoding != HD_HTTP_ENCODE_CHUNKED)
    {
      if (state == HD_HTTP_POST_RECV)
	state = HD_HTTP_POST_SEND;
      else
	state = HD_HTTP_WAITING;
    }
  }

  return (bytes);
}


//
// 'hdHTTP::write()' - Write data to a HTTP connection.
//
 
int					// O - Number of bytes written
hdHTTP::write(const char *b,		// I - Buffer for data
	      int        length)	// I - Number of bytes to write
{
  int	tbytes,				// Total bytes sent
	bytes;				// Bytes sent


  if (b == NULL)
    return (-1);

  activity = time(NULL);

  if (data_encoding == HD_HTTP_ENCODE_CHUNKED)
  {
    if (printf("%x\r\n", length) < 0)
      return (-1);

    if (length == 0)
    {
      // A zero-length chunk ends a transfer; unless we are sending POST
      // data, go idle...
      DEBUG_puts("hdHTTP::write: changing states...");

      if (state == HD_HTTP_POST_RECV)
	state = HD_HTTP_POST_SEND;
      else
	state = HD_HTTP_WAITING;

      if (printf("\r\n") < 0)
	return (-1);

      return (0);
    }
  }

  tbytes = 0;

  while (length > 0)
  {
#ifdef HAVE_LIBSSL
    if (tls)
      bytes = SSL_write((SSL *)(tls), b, length);
    else
#endif // HAVE_LIBSSL
    bytes = ::send(fd, b, length, 0);

    if (bytes < 0)
    {
      DEBUG_puts("hdHTTP::write: error writing data...\n");

      return (-1);
    }

    b      += bytes;
    tbytes += bytes;
    length -= bytes;
    if (data_encoding == HD_HTTP_ENCODE_LENGTH)
      data_remaining -= bytes;
  }

  if (data_encoding == HD_HTTP_ENCODE_CHUNKED)
    if (printf("\r\n") < 0)
      return (-1);

  if (data_remaining == 0 && data_encoding == HD_HTTP_ENCODE_LENGTH)
  {
    // Finished with the transfer; unless we are sending POST data, go idle...
    DEBUG_puts("hdHTTP::write: changing states...");

    if (state == HD_HTTP_POST_RECV)
      state = HD_HTTP_POST_SEND;
    else
      state = HD_HTTP_WAITING;
  }

  DEBUG_printf(("hdHTTP::write: wrote %d bytes...\n", tbytes));

  return (tbytes);
}


//
// 'hdHTTP::gets()' - Get a line of text from a HTTP connection.
//

char *					// O - Line or NULL
hdHTTP::gets(char *line,		// O - Line buffer
             int  length)		// I - Max length of buffer
{
  char	*lineptr,			// Pointer into line
	*bufptr,			// Pointer into input buffer
	*bufend;			// Pointer to end of buffer
  int	bytes;				// Number of bytes read


  DEBUG_printf(("hdHTTP::gets(%p, %d)\n", line, length));

  if (line == NULL)
    return (NULL);

  // Pre-scan the buffer and see if there is a newline in there...
#if defined(WIN32)
  WSASetLastError(0);
#else
  errno = 0;
#endif // WIN32

  do
  {
    bufptr  = buffer;
    bufend  = buffer + used;

    while (bufptr < bufend)
      if (*bufptr == 0x0a)
	break;
      else
	bufptr ++;

    if (bufptr >= bufend && used < HD_MAX_BUFFER)
    {
      // No newline; see if there is more data to be read...
#ifdef HAVE_LIBSSL
      if (tls)
        bytes = SSL_read((SSL *)(tls), bufend,
	                 HD_MAX_BUFFER - used);
      else
#endif // HAVE_LIBSSL
      bytes = recv(fd, bufend, HD_MAX_BUFFER - used, 0);

      if (bytes < 0)
      {
        // Nope, can't get a line this time...
#ifdef WIN32
        if (WSAGetLastError() != error)
	{
	  error = WSAGetLastError();
	  continue;
	}

        DEBUG_printf(("hdHTTP::gets(): recv() error %d!\n", WSAGetLastError()));
#else
        if (errno != error)
	{
	  error = errno;
	  continue;
	}

        DEBUG_printf(("hdHTTP::gets(): recv() error %d!\n", errno));
#endif // WIN32

        return (NULL);
      }
      else if (bytes == 0)
      {
        if (blocking)
	  error = EPIPE;

        return (NULL);
      }

      // Yup, update the amount used and the end pointer...
      used += bytes;
      bufend     += bytes;
    }
  }
  while (bufptr >= bufend && used < HD_MAX_BUFFER);

  activity = time(NULL);

  // Read a line from the buffer...
  lineptr = line;
  bufptr  = buffer;
  bytes   = 0;
  length --;

  while (bufptr < bufend && bytes < length)
  {
    bytes ++;

    if (*bufptr == 0x0a)
    {
      bufptr ++;
      break;
    }
    else if (*bufptr == 0x0d)
      bufptr ++;
    else
      *lineptr++ = *bufptr++;
  }

  if (bytes > 0)
  {
    *lineptr = '\0';

    used -= bytes;
    if (used > 0)
      memcpy(buffer, bufptr, used);

    DEBUG_printf(("hdHTTP::gets(): Returning \"%s\"\n", line));
    return (line);
  }

  DEBUG_puts("hdHTTP::gets(): No new line available!");

  return (NULL);
}


//
// 'hdHTTP::printf()' - Print a formatted string to a HTTP connection.
//

int					// O - Number of bytes written
hdHTTP::printf(const char *format,	// I - printf-style format string
	       ...)			// I - Additional args as needed
{
  int		bytes,			// Number of bytes to write
		nbytes,			// Number of bytes written
		tbytes;			// Number of bytes all together
  char		buf[HD_MAX_BUFFER],	// Buffer for formatted string
		*bufptr;		// Pointer into buffer
  va_list	ap;			// Variable argument pointer


  va_start(ap, format);
  bytes = vsnprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  DEBUG_printf(("hdHTTP::printf: %s", buf));

  for (tbytes = 0, bufptr = buf; tbytes < bytes; tbytes += nbytes, bufptr += nbytes)
  {
#ifdef HAVE_LIBSSL
    if (tls)
      nbytes = SSL_write((SSL *)(tls), bufptr, bytes - tbytes);
    else
#endif // HAVE_LIBSSL
    nbytes = ::send(fd, bufptr, bytes - tbytes, 0);

    if (nbytes < 0)
      return (-1);
  }

  return (bytes);
}


//
// 'hdHTTP::status_string()' - Return a short string describing a HTTP status code.
//

const char *				// O - String or NULL
hdHTTP::status_string(hdHTTPStatus s)	// I - HTTP status code
{
  switch (s)
  {
    case HD_HTTP_CONTINUE :
        return ("Continue");
    case HD_HTTP_SWITCHING_PROTOCOLS :
        return ("Switching Protocols");
    case HD_HTTP_OK :
        return ("OK");
    case HD_HTTP_CREATED :
        return ("Created");
    case HD_HTTP_ACCEPTED :
        return ("Accepted");
    case HD_HTTP_NO_CONTENT :
        return ("No Content");
    case HD_HTTP_NOT_MODIFIED :
        return ("Not Modified");
    case HD_HTTP_BAD_REQUEST :
        return ("Bad Request");
    case HD_HTTP_UNAUTHORIZED :
        return ("Unauthorized");
    case HD_HTTP_FORBIDDEN :
        return ("Forbidden");
    case HD_HTTP_NOT_FOUND :
        return ("Not Found");
    case HD_HTTP_REQUEST_TOO_LARGE :
        return ("Request Entity Too Large");
    case HD_HTTP_URI_TOO_LONG :
        return ("URI Too Long");
    case HD_HTTP_UPGRADE_REQUIRED :
        return ("Upgrade Required");
    case HD_HTTP_NOT_IMPLEMENTED :
        return ("Not Implemented");
    case HD_HTTP_NOT_SUPPORTED :
        return ("Not Supported");
    default :
        return ("Unknown");
  }
}


//
// 'hdHTTP::get_date_string()' - Get a formatted date/time string from a time value.
//

const char *				// O - Date/time string
hdHTTP::get_date_string(time_t t)	// I - UNIX time
{
  struct tm	*tdate;
  static char	datetime[256];


  tdate = gmtime(&t);
  snprintf(datetime, sizeof(datetime), "%s, %02d %s %d %02d:%02d:%02d GMT",
           http_days[tdate->tm_wday], tdate->tm_mday,
	   http_months[tdate->tm_mon],
	   tdate->tm_year + 1900, tdate->tm_hour, tdate->tm_min, tdate->tm_sec);

  return (datetime);
}


//
// 'hdHTTP::get_date_time()' - Get a time value from a formatted date/time string.
//

time_t					// O - UNIX time
hdHTTP::get_date_time(const char *s)	// I - Date/time string
{
  int		i;			// Looping var
  struct tm	tdate;			// Time/date structure
  char		mon[16];		// Abbreviated month name
  int		day, year;		// Day of month and year
  int		hour, min, sec;		// Time


  if (sscanf(s, "%*s%d%15s%d%d:%d:%d", &day, mon, &year, &hour, &min, &sec) < 6)
    return (0);

  for (i = 0; i < 12; i ++)
    if (strcasecmp(mon, http_months[i]) == 0)
      break;

  if (i >= 12)
    return (0);

  tdate.tm_mon   = i;
  tdate.tm_mday  = day;
  tdate.tm_year  = year - 1900;
  tdate.tm_hour  = hour;
  tdate.tm_min   = min;
  tdate.tm_sec   = sec;
  tdate.tm_isdst = 0;

  return (mktime(&tdate));
}


//
// 'hdHTTP::update()' - Update the current HTTP state for incoming data.
//

hdHTTPStatus				// O - HTTP status
hdHTTP::update()
{
  char		line[1024],		// Line from connection...
		*value;			// Pointer to value on line
  hdHTTPField	field;			// Field index
  int		major, minor;		// HTTP version numbers
  hdHTTPStatus	status;			// Authorization status
#ifdef HAVE_LIBSSL
  SSL_CTX	*context;		// Context for encryption
  SSL		*conn;			// Connection for encryption
#endif // HAVE_LIBSSL


  DEBUG_puts("hdHTTP::update()");

  // If we haven't issued any commands, then there is nothing to "update"...
  if (state == HD_HTTP_WAITING)
    return (HD_HTTP_CONTINUE);

  // Grab all of the lines we can from the connection...
  while (gets(line, sizeof(line)) != NULL)
  {
    DEBUG_puts(line);

    if (line[0] == '\0')
    {
      //
      // Blank line means the start of the data section (if any).  Return
      // the result code, too...
      //
      // If we get status 100 (HD_HTTP_CONTINUE), then we *don't// change states.
      // Instead, we just return HD_HTTP_CONTINUE to the caller and keep on
      // tryin'...
      //

      if (status == HD_HTTP_CONTINUE)
        return (status);

#ifdef HAVE_LIBSSL
      if (status == HD_HTTP_SWITCHING_PROTOCOLS && !tls)
      {
	context = SSL_CTX_new(SSLv23_method());
	conn    = SSL_new(context);

	SSL_set_fd(conn, fd);
	if (SSL_connect(conn) != 1)
	{
	  SSL_CTX_free(context);
	  SSL_free(conn);

#ifdef WIN32
	  error  = WSAGetLastError();
#else
	  error  = errno;
#endif // WIN32
	  status = HD_HTTP_ERROR;

#ifdef WIN32
	  closesocket(fd);
#else
	  close(fd);
#endif

	  return (HD_HTTP_ERROR);
	}

	tls = conn;

        return (HD_HTTP_CONTINUE);
      }
      else if (status == HD_HTTP_UPGRADE_REQUIRED &&
               encryption != HD_HTTP_ENCRYPT_NEVER)
        encryption = HD_HTTP_ENCRYPT_REQUIRED;
#endif // HAVE_LIBSSL

      get_content_length();

      switch (state)
      {
        case HD_HTTP_GET :
	    state = HD_HTTP_GET_SEND;
	    break;

	case HD_HTTP_POST :
	    state = HD_HTTP_POST_RECV;
	    break;

	case HD_HTTP_POST_RECV :
	    state = HD_HTTP_POST_SEND;
	    break;

	case HD_HTTP_PUT :
	    state = HD_HTTP_PUT_RECV;
	    break;

	default :
	    state = HD_HTTP_WAITING;
	    break;
      }

      return (status);
    }
    else if (strncmp(line, "HTTP/", 5) == 0)
    {
      // Got the beginning of a response...
      if (sscanf(line, "HTTP/%d.%d%d", &major, &minor, (int *)&status) != 3)
        return (HD_HTTP_ERROR);

      version = (hdHTTPVersion)(major * 100 + minor);
      status  = status;
    }
    else if ((value = strchr(line, ':')) != NULL)
    {
      // Got a value...
      *value++ = '\0';
      while (isspace(*value))
        value ++;

      // Be tolerants of servers that send unknown attribute fields...
      if ((field = field_number(line)) == HD_HTTP_FIELD_UNKNOWN)
      {
        DEBUG_printf(("hdHTTP::update: unknown field %s seen!\n", line));
        continue;
      }

      set_field(field, value);
    }
    else
    {
      status = HD_HTTP_ERROR;
      return (HD_HTTP_ERROR);
    }
  }

  // See if there was an error...
  if (error)
  {
    status = HD_HTTP_ERROR;
    return (HD_HTTP_ERROR);
  }

  // If we haven't already returned, then there is nothing new...
  return (HD_HTTP_CONTINUE);
}


//
// 'hdHTTP::decode64()' - Base64-decode a string.
//

char *					// O - Decoded string
hdHTTP::decode64(char       *out,	// O - String to write to
                 int        outlen,	// I - Size of buffer
                 const char *in)	// I - String to read from
{
  int	pos,				// Bit position
	base64;				// Value of this character
  char	*outptr;			// Output pointer


  for (outptr = out, pos = 0; *in != '\0' && outptr < (out + outlen - 1); in ++)
  {
    // Decode this character into a number from 0 to 63...
    if (*in >= 'A' && *in <= 'Z')
      base64 = *in - 'A';
    else if (*in >= 'a' && *in <= 'z')
      base64 = *in - 'a' + 26;
    else if (*in >= '0' && *in <= '9')
      base64 = *in - '0' + 52;
    else if (*in == '+')
      base64 = 62;
    else if (*in == '/')
      base64 = 63;
    else if (*in == '=')
      break;
    else
      continue;

    // Store the result in the appropriate chars...
    switch (pos)
    {
      case 0 :
          *outptr = base64 << 2;
	  pos ++;
	  break;
      case 1 :
          *outptr++ |= (base64 >> 4) & 3;
	  *outptr = (base64 << 4) & 255;
	  pos ++;
	  break;
      case 2 :
          *outptr++ |= (base64 >> 2) & 15;
	  *outptr = (base64 << 6) & 255;
	  pos ++;
	  break;
      case 3 :
          *outptr++ |= base64;
	  pos = 0;
	  break;
    }
  }

  *outptr = '\0';

  // Return the decoded string...
  return (out);
}


//
// 'hdHTTP::encode64()' - Base64-encode a string.
//

char *					// O - Encoded string
hdHTTP::encode64(char       *out,	// O - String to write to
                 int        outlen,	// I - Size of buffer
                 const char *in)	// I - String to read from
{
  char		*outptr;		// Output pointer
  static char	base64[] =		// Base64 characters...
  		{
		  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		  "abcdefghijklmnopqrstuvwxyz"
		  "0123456789"
		  "+/"
  		};


  for (outptr = out; *in != '\0' && outptr < (out + outlen - 5); in ++)
  {
    // Encode the up to 3 characters as 4 Base64 numbers...
    *outptr++ = base64[in[0] >> 2];
    *outptr++ = base64[((in[0] << 4) | (in[1] >> 4)) & 63];

    in ++;
    if (*in == '\0')
    {
      *outptr ++ = '=';
      break;
    }

    *outptr++ = base64[((in[0] << 2) | (in[1] >> 6)) & 63];

    in ++;
    if (*in == '\0')
      break;

    *outptr++ = base64[in[0] & 63];
  }

  // Add trailing "=" and nul...
  *outptr ++ = '=';
  *outptr = '\0';

  // Return the encoded string...
  return (out);
}


//
// 'hdHTTP::get_content_length()' - Get the amount of data remaining from the content-length or transfer-encoding fields.
//

int				// O - Content length
hdHTTP::get_content_length()
{
  DEBUG_puts("hdHTTP::get_content_length()");

  if (strcasecmp(fields[HD_HTTP_FIELD_TRANSFER_ENCODING], "chunked") == 0)
  {
    DEBUG_puts("hdHTTP::getLength: chunked request!");

    data_encoding  = HD_HTTP_ENCODE_CHUNKED;
    data_remaining = 0;
  }
  else
  {
    data_encoding = HD_HTTP_ENCODE_LENGTH;

    // The following is a hack for HTTP servers that don't send a
    // content-length or transfer-encoding field...
    //
    // If there is no content-length then the connection must close
    // after the transfer is complete...
    if (fields[HD_HTTP_FIELD_CONTENT_LENGTH][0] == '\0')
      data_remaining = 2147483647;
    else
      data_remaining = atoi(fields[HD_HTTP_FIELD_CONTENT_LENGTH]);

    DEBUG_printf(("hdHTTP::getLength: content_length = %d\n", data_remaining));
  }

  return (data_remaining);
}


//
// 'hdHTTP::field_number()' - Return the field index for a field name.
//

hdHTTPField				// O - Field index
hdHTTP::field_number(const char *name)	// I - String name
{
  int	i;				// Looping var


  for (i = 0; i < HD_HTTP_FIELD_MAX; i ++)
    if (strcasecmp(name, http_fields[i]) == 0)
      return ((hdHTTPField)i);

  return (HD_HTTP_FIELD_UNKNOWN);
}


//
// 'hdHTTP::send()' - Send a request with all fields and the trailing blank line.
//

int					// O - 0 on success, non-zero on error
hdHTTP::send(hdHTTPState request,	// I - Request code
	     const char  *uri)		// I - URI
{
  int		i;			// Looping var
  char		*ptr,			// Pointer in buffer
		buf[HD_MAX_URI + 1];	// Encoded URI buffer
  static const char *codes[] =		// Request code strings
		{
		  NULL,
		  "OPTIONS",
		  "GET",
		  NULL,
		  "HEAD",
		  "POST",
		  NULL,
		  NULL,
		  "PUT",
		  NULL,
		  "DELETE",
		  "TRACE",
		  "CLOSE"
		};
  static const char *hex = "0123456789ABCDEF";
					// Hex digits


  if (uri == NULL)
    return (-1);

  // Encode the URI as needed...
  for (ptr = buf; *uri != '\0' && ptr < (buf + sizeof(buf) - 1); uri ++)
    if (*uri <= ' ' || *uri >= 127)
    {
      if (ptr < (buf + sizeof(buf) - 1))
        *ptr ++ = '%';
      if (ptr < (buf + sizeof(buf) - 1))
        *ptr ++ = hex[(*uri >> 4) & 15];
      if (ptr < (buf + sizeof(buf) - 1))
        *ptr ++ = hex[*uri & 15];
    }
    else
      *ptr ++ = *uri;

  *ptr = '\0';

  // See if we had an error the last time around; if so, reconnect...
  if (status == HD_HTTP_ERROR || status >= HD_HTTP_BAD_REQUEST)
    reconnect();

  // Send the request header...
  if (request == HD_HTTP_POST)
    state = HD_HTTP_POST_RECV;
  else if (request == HD_HTTP_PUT)
    state = HD_HTTP_PUT_RECV;
  else
    state = request;

  status = HD_HTTP_CONTINUE;

#ifdef HAVE_LIBSSL
  if (encryption == HD_HTTP_ENCRYPT_REQUIRED && !tls)
  {
    set_field(HD_HTTP_FIELD_CONNECTION, "Upgrade");
    set_field(HD_HTTP_FIELD_UPGRADE, "TLS/1.0,SSL/2.0,SSL/3.0");
  }
#endif // HAVE_LIBSSL

  if (printf("%s %s HTTP/1.1\r\n", codes[request], buf) < 1)
  {
    status = HD_HTTP_ERROR;
    return (-1);
  }

  for (i = 0; i < HD_HTTP_FIELD_MAX; i ++)
    if (fields[i][0] != '\0')
    {
      DEBUG_printf(("%s: %s\n", http_fields[i], fields[i]));

      if (printf("%s: %s\r\n", http_fields[i], fields[i]) < 1)
      {
	status = HD_HTTP_ERROR;
	return (-1);
      }
    }

  if (printf("\r\n") < 1)
  {
    status = HD_HTTP_ERROR;
    return (-1);
  }

  clear_fields();

  return (0);
}


//
// 'hdHTTP::upgrade()' - Force upgrade to TLS encryption.
//

int				// O - Status of connection
hdHTTP::upgrade()
{
#ifdef HAVE_LIBSSL
  int		ret;		// Return value
  char		save_fields[HD_HTTP_FIELD_MAX][HD_MAX_VALUE];


  DEBUG_puts("hdHTTP::upgrade()");

  // Copy the HTTP data to a local variable so we can do the OPTIONS
  // request without interfering with the existing request data...
  memcpy(save_fields, fields, sizeof(save_fields));

  // Send an OPTIONS request to the server, requiring SSL or TLS
  // encryption on the link...
  clear_fields();
  set_field(HD_HTTP_FIELD_CONNECTION, "upgrade");
  set_field(HD_HTTP_FIELD_UPGRADE, "TLS/1.0, SSL/2.0, SSL/3.0");

  if ((ret = send_options("*")) == 0)
  {
    // Wait for the secure connection...
    while (update() == HD_HTTP_CONTINUE);
  }

  flush();

  // Copy the HTTP data back over, if any...
  memcpy(fields, save_fields, sizeof(save_fields));
 
  // See if we actually went secure...
  if (!tls)
  {
    // Server does not support HTTP upgrade...
    DEBUG_puts("Server does not support HTTP upgrade!");

#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif

    fd = -1;

    return (-1);
  }
  else
    return (ret);
#else
  return (-1);
#endif // HAVE_LIBSSL
}


//
// End of "$Id: http.cxx,v 1.7.2.2 2004/03/30 03:49:15 mike Exp $".
//
