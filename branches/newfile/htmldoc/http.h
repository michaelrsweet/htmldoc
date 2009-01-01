//
// "$Id$"
//
//   Hyper-Text Transport Protocol class definitions for HTMLDOC.
//
//   Copyright 1997-2008 Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: HTMLDOC Licensing Information
//       Easy Software Products
//       516 Rio Grand Ct
//       Morgan Hill, CA 95037 USA
//
//       http://www.htmldoc.org/
//

#ifndef _HTMLDOC_HTTP_H_
#  define _HTMLDOC_HTTP_H_

//
// Include necessary headers...
//

#  include <time.h>
#  if defined(WIN32)
#    include <winsock.h>
#  else
#    include <unistd.h>
#    include <sys/time.h>
#    include <sys/socket.h>
#    include <netdb.h>
#    include <netinet/in.h>
#    include <arpa/inet.h>
#    include <netinet/in_systm.h>
#    include <netinet/ip.h>
#    include <netinet/tcp.h>
#  endif // WIN32

#  include "md5.h"


//
// Limits...
//

#  define HD_MAX_URI	1024		// Max length of URI string
#  define HD_MAX_HOST	256		// Max length of hostname string
#  define HD_MAX_BUFFER	2048		// Max length of data buffer
#  define HD_MAX_VALUE	256		// Max header field value length


//
// HTTP state values...
//

typedef enum				// States are server-oriented
{
  HD_HTTP_WAITING,			// Waiting for command
  HD_HTTP_OPTIONS,			// OPTIONS command, waiting for blank line
  HD_HTTP_GET,				// GET command, waiting for blank line
  HD_HTTP_GET_SEND,			// GET command, sending data
  HD_HTTP_HEAD,				// HEAD command, waiting for blank line
  HD_HTTP_POST,				// POST command, waiting for blank line
  HD_HTTP_POST_RECV,			// POST command, receiving data
  HD_HTTP_POST_SEND,			// POST command, sending data
  HD_HTTP_PUT,				// PUT command, waiting for blank line
  HD_HTTP_PUT_RECV,			// PUT command, receiving data
  HD_HTTP_DELETE,			// DELETE command, waiting for blank line
  HD_HTTP_TRACE,			// TRACE command, waiting for blank line
  HD_HTTP_CLOSE,			// CLOSE command, waiting for blank line
  HD_HTTP_STATUS			// Command complete, sending status
} hdHTTPState;


//
// HTTP version numbers...
//

typedef enum
{
  HD_HTTP_0_9 = 9,			// HTTP/0.9
  HD_HTTP_1_0 = 100,			// HTTP/1.0
  HD_HTTP_1_1 = 101			// HTTP/1.1
} hdHTTPVersion;


//
// HTTP keep-alive values...
//

typedef enum
{
  HD_HTTP_KEEPALIVE_OFF = 0,
  HD_HTTP_KEEPALIVE_ON
} hdHTTPKeepAlive;


//
// HTTP transfer encoding values...
//

typedef enum
{
  HD_HTTP_ENCODE_LENGTH,		// Data is sent with Content-Length
  HD_HTTP_ENCODE_CHUNKED		// Data is chunked
} hdHTTPEncoding;


//
// HTTP encryption values...
//

typedef enum
{
  HD_HTTP_ENCRYPT_IF_REQUESTED,		// Encrypt if requested (TLS upgrade)
  HD_HTTP_ENCRYPT_NEVER,		// Never encrypt
  HD_HTTP_ENCRYPT_REQUIRED,		// Encryption is required (TLS upgrade)
  HD_HTTP_ENCRYPT_ALWAYS		// Always encrypt (SSL)
} hdHTTPEncryption;


//
// HTTP authentication types...
//

typedef enum
{
  HD_HTTP_AUTH_NONE,			// No authentication in use
  HD_HTTP_AUTH_BASIC,			// Basic authentication in use
  HD_HTTP_AUTH_MD5,			// Digest authentication in use
  HD_HTTP_AUTH_MD5_SESS,		// MD5-session authentication in use
  HD_HTTP_AUTH_MD5_INT,			// Digest authentication in use for body
  HD_HTTP_AUTH_MD5_SESS_INT		// MD5-session authentication in use for body
} hdHTTPAuth;


//
// HTTP status codes...
//

typedef enum
{
  HD_HTTP_ERROR = -1,			// An error response from httpXxxx()

  HD_HTTP_CONTINUE = 100,		// Everything OK, keep going...
  HD_HTTP_SWITCHING_PROTOCOLS,		// HTTP upgrade to TLS/SSL

  HD_HTTP_OK = 200,			// OPTIONS/GET/HEAD/POST/TRACE command was successful
  HD_HTTP_CREATED,			// PUT command was successful
  HD_HTTP_ACCEPTED,			// DELETE command was successful
  HD_HTTP_NOT_AUTHORITATIVE,		// Information isn't authoritative
  HD_HTTP_NO_CONTENT,			// Successful command, no new data
  HD_HTTP_RESET_CONTENT,		// Content was reset/recreated
  HD_HTTP_PARTIAL_CONTENT,		// Only a partial file was recieved/sent

  HD_HTTP_MULTIPLE_CHOICES = 300,	// Multiple files match request
  HD_HTTP_MOVED_PERMANENTLY,		// Document has moved permanently
  HD_HTTP_MOVED_TEMPORARILY,		// Document has moved temporarily
  HD_HTTP_SEE_OTHER,			// See this other link...
  HD_HTTP_NOT_MODIFIED,			// File not modified
  HD_HTTP_USE_PROXY,			// Must use a proxy to access this URI

  HD_HTTP_BAD_REQUEST = 400,		// Bad request
  HD_HTTP_UNAUTHORIZED,			// Unauthorized to access host
  HD_HTTP_PAYMENT_REQUIRED,		// Payment required
  HD_HTTP_FORBIDDEN,			// Forbidden to access this URI
  HD_HTTP_NOT_FOUND,			// URI was not found
  HD_HTTP_METHOD_NOT_ALLOWED,		// Method is not allowed
  HD_HTTP_NOT_ACCEPTABLE,		// Not Acceptable
  HD_HTTP_PROXY_AUTHENTICATION,		// Proxy Authentication is Required
  HD_HTTP_REQUEST_TIMEOUT,		// Request timed out
  HD_HTTP_CONFLICT,			// Request is self-conflicting
  HD_HTTP_GONE,				// Server has gone away
  HD_HTTP_LENGTH_REQUIRED,		// A content length or encoding is required
  HD_HTTP_PRECONDITION,			// Precondition failed
  HD_HTTP_REQUEST_TOO_LARGE,		// Request entity too large
  HD_HTTP_URI_TOO_LONG,			// URI too long
  HD_HTTP_UNSUPPORTED_MEDIATYPE,	// The requested media type is unsupported
  HD_HTTP_UPGRADE_REQUIRED = 426,	// Upgrade to SSL/TLS required

  HD_HTTP_SERVER_ERROR = 500,		// Internal server error
  HD_HTTP_NOT_IMPLEMENTED,		// Feature not implemented
  HD_HTTP_BAD_GATEWAY,			// Bad gateway
  HD_HTTP_SERVICE_UNAVAILABLE,		// Service is unavailable
  HD_HTTP_GATEWAY_TIMEOUT,		// Gateway connection timed out
  HD_HTTP_NOT_SUPPORTED			// HTTP version not supported
} hdHTTPStatus;


//
// HTTP field names...
//

typedef enum
{
  HD_HTTP_FIELD_UNKNOWN = -1,
  HD_HTTP_FIELD_ACCEPT_LANGUAGE,
  HD_HTTP_FIELD_ACCEPT_RANGES,
  HD_HTTP_FIELD_AUTHORIZATION,
  HD_HTTP_FIELD_CONNECTION,
  HD_HTTP_FIELD_CONTENT_ENCODING,
  HD_HTTP_FIELD_CONTENT_LANGUAGE,
  HD_HTTP_FIELD_CONTENT_LENGTH,
  HD_HTTP_FIELD_CONTENT_LOCATION,
  HD_HTTP_FIELD_CONTENT_MD5,
  HD_HTTP_FIELD_CONTENT_RANGE,
  HD_HTTP_FIELD_CONTENT_TYPE,
  HD_HTTP_FIELD_CONTENT_VERSION,
  HD_HTTP_FIELD_COOKIE,
  HD_HTTP_FIELD_DATE,
  HD_HTTP_FIELD_EXPECT,
  HD_HTTP_FIELD_HOST,
  HD_HTTP_FIELD_IF_MODIFIED_SINCE,
  HD_HTTP_FIELD_IF_UNMODIFIED_SINCE,
  HD_HTTP_FIELD_KEEP_ALIVE,
  HD_HTTP_FIELD_LAST_MODIFIED,
  HD_HTTP_FIELD_LINK,
  HD_HTTP_FIELD_LOCATION,
  HD_HTTP_FIELD_RANGE,
  HD_HTTP_FIELD_REFERER,
  HD_HTTP_FIELD_RETRY_AFTER,
  HD_HTTP_FIELD_SET_COOKIE,
  HD_HTTP_FIELD_TRANSFER_ENCODING,
  HD_HTTP_FIELD_UPGRADE,
  HD_HTTP_FIELD_USER_AGENT,
  HD_HTTP_FIELD_WWW_AUTHENTICATE,
  HD_HTTP_FIELD_MAX
} hdHTTPField;
  

//
// HTTP connection structure...
//

class hdHTTP
{
  int			fd;		// File descriptor for this socket
  int			blocking;	// To block or not to block
  int			error;		// Last error on read
  time_t		activity;	// Time since last read/write
  hdHTTPState		state;		// State of client
  hdHTTPStatus		status;		// Status of last request
  hdHTTPVersion		version;	// Protocol version
  hdHTTPKeepAlive	keep_alive;	// Keep-alive supported?
  struct sockaddr_in	hostaddr;	// Address of connected host
  char			hostname[HD_MAX_HOST],
  					// Name of connected host
			fields[HD_HTTP_FIELD_MAX][HD_MAX_VALUE];
					// Field values
  char			*data;		// Pointer to data buffer
  hdHTTPEncoding	data_encoding;	// Chunked or not
  int			data_remaining;	// Number of bytes left
  int			used;		// Number of bytes used in buffer
  char			buffer[HD_MAX_BUFFER];
					// Buffer for messages
  int			authtype;	// Authentication in use
  hdMD5			md5_state;	// MD5 state
  char			nonce[HD_MAX_VALUE];
					// Nonce value
  int			nonce_count;	// Nonce count
  void			*tls;		// TLS state information
  hdHTTPEncryption	encryption;	// Encryption requirements

  int			send(hdHTTPState request, const char *uri);
  int			upgrade();

  static void		initialize();

  public:

  hdHTTP(const char *h, int port = 80, hdHTTPEncryption e = HD_HTTP_ENCRYPT_IF_REQUESTED);
  ~hdHTTP();

  int			check();
  void			clear_fields();
  void			flush();
  int			get_blocking() { return blocking; }
  hdHTTPEncryption	get_encryption() { return encryption; }
  int			get_error() { return error; }
  int			get_fd() { return fd; }
  const char		*get_field(hdHTTPField field) { return fields[field]; }
  int			get_content_length();
  const char		*get_hostname() { return hostname; }
  hdHTTPState		get_state() { return state; }
  hdHTTPStatus		get_status() { return status; }
  char			*get_sub_field(hdHTTPField field, const char *name,
			               char *value, int length);
  char			*gets(char *line, int length);
  int			printf(const char *format, ...);
  int			read(char *buffer, int length);
  int			reconnect();
  int			send_delete(const char *uri);
  int			send_get(const char *uri);
  int			send_head(const char *uri);
  int			send_options(const char *uri);
  int			send_post(const char *uri);
  int			send_put(const char *uri);
  int			send_trace(const char *uri);
  void			set_blocking(int b) { blocking = b; }
  int			set_encryption(hdHTTPEncryption e);
  void			set_field(hdHTTPField field, const char *value);
  hdHTTPStatus		update();
  int			write(const char *buffer, int length);

  static char		*decode64(char *out, int outlen, const char *in);
  static char		*encode64(char *out, int outlen, const char *in);
  static hdHTTPField	field_number(const char *name);
  static const char	*get_date_string(time_t t);
  static time_t		get_date_time(const char *s);
  static char		*md5(const char *username, const char *realm,
			     const char *passwd, char *s, int slen);
  static char		*md5_final(const char *nonce, const char *scheme,
			           const char *resource, char *s, int slen);
  static char		*md5_string(const hdByte *sum, char *s, int slen);
  static void		separate(const char *uri, char *scheme, int schemelen,
			         char *username, int usernamelen,
				 char *host, int hostlen,
				 int *port,
				 char *resource, int resourcelen);
  static const char	*status_string(hdHTTPStatus status);
};


#endif // !_HTMLDOC_HTTP_H_

//
// End of "$Id$".
//
