//
// "$Id$"
//
//   Filename routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 2011 by Michael R Sweet.
//   Copyright 1997-2010 by Easy Software Products.
//
//   This program is free software.  Distribution and use rights are outlined in
//   the file "COPYING.txt".
//
// Contents:
//
//   hdFile::basename()    - Return the base filename without directory or target.
//   hdFile::cleanup()     - Close an open HTTP connection and remove temporary files...
//   hdFile::directory()   - Return the directory without filename or target.
//   hdFile::extension()   - Return the extension of a file without the target.
//   hdFile::find()        - Find a file in one of the path directories.
//   hdFile::localize()    - Localize a filename for the new working directory.
//   hdFile::scheme()      - Return the scheme for a filename or URL.
//   hdFile::proxy()       - Set the proxy_ host for all HTTP requests.
//   hdFile::target()      - Return the target of a link.
//   hdFile::temp()        - Create and open a temporary file.
//

//
// Include necessary headers.
//

#include "file.h"
#include "hdstring.h"

//#include "progress.h"

#if defined(WIN32)
#  include <io.h>
#else
#  include <unistd.h>
#endif // WIN32

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>


//
// Temporary file definitions...
//

#ifdef WIN32
#  define getpid	GetCurrentProcessId
#  define TEMPLATE	"%s%08x.%06d.tmp"
#  define OPENMODE	(_O_CREAT | _O_RDWR | _O_EXCL | \
			 _O_TRUNC | _O_BINARY | _O_SHORT_LIVED)
#  define OPENPERM	(_S_IREAD | _S_IWRITE)
#else
#  define TEMPLATE	"%s/%06d.%06d.tmp"
#  define OPENMODE	(O_CREAT | O_RDWR | O_EXCL | O_TRUNC)
#  define OPENPERM	0600
#endif // WIN32


//
// Class globals...
//

char		*hdFile::cookies_ = NULL;
					// Cookies to pass through
bool		hdFile::no_local_ = false;
					// Non-zero to disable local files
char		*hdFile::proxy_ = NULL;	// Proxy URL
char		*hdFile::proxy_host_ = NULL;
					// Proxy hostname
int		hdFile::proxy_port_ = 0;// Proxy port
char		*hdFile::referer_ = NULL;
					// Referer to pass through
int		hdFile::temp_files_ = 0,// Number of temporary files
		hdFile::temp_alloc_ = 0;// Number of allocated files
hdCache		*hdFile::temp_cache_ = NULL;
					// Cache array


//
// 'hdFile::basename()' - Return the base filename without directory or target.
//

char *					// O - Base filename
hdFile::basename(const char *s,		// I - Filename or URL
                 char       *t,		// O - Base filename
		 size_t     tlen)	// I - Size of filename buffer
{
  char		*baseptr;		// Pointer to directory separator
  char		*target;		// Pointer to target separator


  if (!s || !t)
    return (NULL);

  if ((baseptr = (char *)strrchr(s, '/')) != NULL)
    baseptr ++;
#ifdef WIN32
  else if ((baseptr = (char *)strrchr(s, '\\')) != NULL)
    baseptr ++;
#endif /* WIN32 */
  else
    baseptr = (char *)s;

  if (baseptr[0] == '#')
    return (NULL);

  strlcpy(t, baseptr, tlen);

  if ((target = (char *)strchr(t, '#')) != NULL)
    *target = '\0';

  return (t);
}


//
// 'hdFile::cleanup()' - Close an open HTTP connection and remove temporary files...
//

void
hdFile::cleanup(void)
{
  char		filename[1024];		// Temporary file
#ifdef WIN32
  char		tmpdir[1024];		// Temporary directory
#else
  const char	*tmpdir;		// Temporary directory
#endif // WIN32


#ifdef WIN32
  GetTempPath(sizeof(tmpdir), tmpdir);
#else
  if ((tmpdir = getenv("TMPDIR")) == NULL)
    tmpdir = "/var/tmp";
#endif // WIN32

  while (temp_files_ > 0)
  {
    snprintf(filename, sizeof(filename), TEMPLATE, tmpdir,
             getpid(), temp_files_);

    unlink(filename);
//    if (unlink(filename))
//      progress_error(HD_ERROR_DELETE_ERROR,
//                     "Unable to delete temporary file \"%s\": %s",
//                     filename, strerror(errno));

    temp_files_ --;

    if (temp_cache_[temp_files_].name)
      free(temp_cache_[temp_files_].name);
    if (temp_cache_[temp_files_].url)
      free(temp_cache_[temp_files_].url);
  }

  if (temp_alloc_)
  {
    free(temp_cache_);

    temp_alloc_ = 0;
    temp_cache_ = NULL;
  }
}


//
// 'hdFile::cookies()' - Set the cookies to use for HTTP requests.
//

void
hdFile::cookies(const char *c)		// I - Cookies or NULL
{
  if (cookies_)
    free(cookies_);

  if (c)
    cookies_ = strdup(c);
  else
    cookies_ = NULL;
}


//
// 'hdFile::dirname()' - Return the directory without filename or target.
//

char *					// O - Directory for file
hdFile::dirname(const char *s,		// I - Filename or URL
                char       *t,		// O - Base filename
		size_t     tlen)	// I - Size of filename buffer
{
  char		*dir;			// Pointer to directory separator


  if (!s || !t)
    return (NULL);

  if (!strncmp(s, "file://", 7) ||
      !strncmp(s, "http://", 7) ||
      !strncmp(s, "https://", 8))
  {
    // Handle URLs...
    char	myscheme[HD_MAX_URI],
		username[HD_MAX_URI],
		hostname[HD_MAX_URI],
		resource[HD_MAX_URI];
    int		port;


    hdHTTP::separate(s, myscheme, sizeof(myscheme), username, sizeof(username),
                     hostname, sizeof(hostname), &port, resource,
		     sizeof(resource));

    if ((dir = strrchr(resource, '/')) != NULL)
    {
      if (dir > resource)
        *dir = '\0';
      else
        dir[1] = '\0';
    }

    if (!strcmp(myscheme, "file"))
      strlcpy(resource, t, tlen);
    else if (username[0]) // TODO: Replace with hdHTTP::assemble!
      snprintf(t, tlen, "%s://%s@%s:%d%s", myscheme, username, hostname,
               port, resource);
    else // TODO: Replace with hdHTTP::assemble!
      snprintf(t, tlen, "%s://%s:%d%s", myscheme, hostname, port,
               resource);
  }
  else
  {
    // Normal stuff...
    strlcpy(t, s, tlen);

    if ((dir = (char *)strrchr(t, '/')) != NULL)
      *dir = '\0';
#ifdef WIN32
    else if ((dir = (char *)strrchr(t, '\\')) != NULL)
      *dir = '\0';
#endif /* WIN32 */
    else
      strlcpy(t, ".", tlen);
  }

  return (t);
}


//
// 'hdFile::extension()' - Return the extension of a file without the target.
//

char *					// O - File extension
hdFile::extension(const char *s,	// I - Filename or URL
                  char       *t,	// O - Base filename
		  size_t     tlen)	// I - Size of filename buffer
{
  char	base[1024],			// Basename
	*extptr,			// Pointer to extension separator
	*target;			// Pointer to target separator


  if (!s || !t)
    return (NULL);

  hdFile::basename(s, base, sizeof(base));

  if ((target = (char *)strchr(base, '#')) != NULL)
    *target = '\0';

  if ((extptr = (char *)strrchr(base, '.')) == NULL)
    t[0] = '\0';
  else
  {
    extptr ++;

    strlcpy(t, extptr, tlen);
  }

  return (t);
}


//
// 'hdFile::find()' - Find a file in one of the path directories.
//

char *					// O - Pathname or NULL
hdFile::find(const char *path,		// I - Path "dir;dir;dir"
             const char *uri,		// I - URI to find
	     char       *name,		// O - Found filename
	     size_t     namelen)	// I - Size of filename buffer
{
  int		i;			// Looping var
  int		retry;			// Current retry
  char		*temp,			// Current position in filename
		myscheme[HD_MAX_URI],	// Method/scheme
		username[HD_MAX_URI],	// Username:password
		hostname[HD_MAX_URI],	// Hostname
		resource[HD_MAX_URI];	// Resource
  int		port;			// Port number
  const char	*connhost;		// Host to connect to
  int		connport;		// Port to connect to
  char		connpath[HD_MAX_URI],	// Path for GET
		connauth[HD_MAX_VALUE];	// Auth string
  hdHTTP	*http;			// Connection to remote host
  hdHTTPStatus	status;			// Status of request...
  hdFile	*fp;			// Web file
  int		bytes,			// Bytes read
		count,			// Number of bytes so far
		total;			// Total bytes in file
  

  // If the filename or name buffer is NULL, return NULL...
  if (!uri || !name)
    return (NULL);

  if (!strncmp(uri, "http:", 5) || !strncmp(uri, "//", 2) ||
      (path && !strncmp(path, "http:", 5)))
    strcpy(myscheme, "http");
#ifdef HAVE_LIBSSL
  else if (!strncmp(uri, "https:", 6) ||
           (path && !strncmp(path, "https:", 6)))
    strcpy(myscheme, "https");
#endif // HAVE_LIBSSL
  else
    strcpy(myscheme, "file");

  if (!strcmp(myscheme, "file"))
  {
    // If we are not allowing access to local files, return NULL...
    if (no_local_)
    {
      // See if we are accessing a known temp file...
      for (i = 0; i < temp_files_; i ++)
        if (strcmp(uri, temp_cache_[i].name) == 0)
	{
	  strlcpy(name, uri, namelen);

	  return (name);
	}

      return (NULL);
    }

    // If the path is NULL or empty, return the filename...
    if (!path || !path[0])
    {
      strlcpy(name, uri, namelen);

      return (name);
    }

    // Else loop through the path string until we reach the end...
    while (*path != '\0')
    {
      // Copy the path directory...
      temp = name;

      while (*path != ';' && *path && temp < (name + namelen - 1))
	*temp++ = *path++;

      if (*path == ';')
	path ++;

      // Append a slash as needed...
      if (temp > name && temp < (name + namelen - 1) &&
          uri[0] != '/')
	*temp++ = '/';

      // Append the name...
      strlcpy(temp, uri, namelen - (temp - name));

      // See if the file exists...
      if (!access(name, 0))
	return (name);
    }

    // If the name exists, return the name...
    if (!access(uri, 0))
    {
      strlcpy(name, uri, namelen);

      return (name);
    }
  }
  else
  {
    // Remote file; look it up in the web cache, and then try getting it
    // from the remote system...
    for (i = 0; i < temp_files_; i ++)
      if (temp_cache_[i].url && strcmp(temp_cache_[i].url, uri) == 0)
        return (temp_cache_[i].name);

#ifdef HAVE_LIBSSL
    if (!strncmp(uri, "http:", 5) || !strncmp(uri, "//", 2) ||
        !strncmp(uri, "https:", 6))
#else
    if (!strncmp(uri, "http:", 5) || !strncmp(uri, "//", 2))
#endif // HAVE_LIBSSL
      hdHTTP::separate(uri, myscheme, sizeof(myscheme),
                       username, sizeof(username),
                       hostname, sizeof(hostname), &port,
		       resource, sizeof(resource));
    else if (uri[0] == '/')
    {
      hdHTTP::separate(path, myscheme, sizeof(myscheme),
                       username, sizeof(username),
                       hostname, sizeof(hostname), &port,
		       resource, sizeof(resource));
      strlcpy(resource, uri, sizeof(resource));
    }
    else
    {
      if (!strncmp(uri, "./", 2))
        snprintf(name, namelen, "%s/%s", path, uri + 2);
      else
        snprintf(name, namelen, "%s/%s", path, uri);

      hdHTTP::separate(name, myscheme, sizeof(myscheme),
                       username, sizeof(username),
		       hostname, sizeof(hostname), &port,
		       resource, sizeof(resource));
    }

    for (status = HD_HTTP_ERROR, retry = 0;
         status == HD_HTTP_ERROR && retry < 5;
         retry ++)
    {
      if (proxy_host_)
      {
        // Send request to proxy_ host...
        connhost = proxy_host_;
        connport = proxy_port_;
	// TODO: Use hdHTTP::assemble
        snprintf(connpath, sizeof(connpath), "%s://%s:%d%s", myscheme,
                 hostname, port, resource);
      }
      else
      {
        // Send request to host directly...
        connhost = hostname;
        connport = port;
        strcpy(connpath, resource);
      }

//      progress_show("Connecting to %s...", connhost);
      ::printf("Connecting to %s...\n", connhost);
      atexit(hdFile::cleanup);

#ifdef HAVE_LIBSSL
      if (!strcmp(myscheme, "http"))
        http = new hdHTTP(connhost, connport);
      else
        http = new hdHTTP(connhost, connport, HD_HTTP_ENCRYPT_ALWAYS);
#else
      http = new hdHTTP(connhost, connport);
#endif // HAVE_LIBSSL

      if (http->get_status() == HD_HTTP_ERROR)
      {
//        progress_hide();
//        progress_error(HD_ERROR_NETWORK_ERROR,
//	                 "Unable to connect to %s!", connhost);
        ::printf("Unable to connect to %s - %s!\n", connhost, strerror(errno));
        delete http;
        return (NULL);
      }

//      progress_show("Getting %s...", connpath);
      ::printf("Getting %s...\n", connpath);

      http->clear_fields();
      http->set_field(HD_HTTP_FIELD_HOST, hostname);
      http->set_field(HD_HTTP_FIELD_USER_AGENT, "HTMLDOC v" SVERSION);
      http->set_field(HD_HTTP_FIELD_CONNECTION, "Keep-Alive");

      if (username[0])
      {
        strcpy(connauth, "Basic ");
        hdHTTP::encode64(connauth + 6, sizeof(connauth) - 6, username);
        http->set_field(HD_HTTP_FIELD_AUTHORIZATION, connauth);
      }

      if (!http->send_get(connpath))
	while ((status = http->update()) == HD_HTTP_CONTINUE);
      else
	status = HD_HTTP_ERROR;

      if (status >= HD_HTTP_MOVED_PERMANENTLY && status <= HD_HTTP_SEE_OTHER)
      {
        // Flush text...
	http->flush();

        // Grab new location from HTTP data...
	hdHTTP::separate(http->get_field(HD_HTTP_FIELD_LOCATION),
	                 myscheme, sizeof(myscheme), username, sizeof(username),
                	 hostname, sizeof(hostname), &port,
			 resource, sizeof(resource));
        status = HD_HTTP_ERROR;
      }
    }

    if (status != HD_HTTP_OK)
    {
//      progress_hide();
//      progress_error((HDerror)status, "%s", httpStatus(status));
      ::printf("%d %s\n", status, hdHTTP::status_string(status));
      http->flush();
      delete http;
      return (NULL);
    }

    if ((fp = hdFile::temp(name, namelen, uri)) == NULL)
    {
//      progress_hide();
//      progress_error(HD_ERROR_WRITE_ERROR,
//                     "Unable to create temporary file \"%s\": %s", name,
//                     strerror(errno));
      ::printf("Unable to create temporary file \"%s\": %s\n", name,
               strerror(errno));
      http->flush();
      delete http;
      return (NULL);
    }

    if ((total = atoi(http->get_field(HD_HTTP_FIELD_CONTENT_LENGTH))) == 0)
      total = 8192;

    count = 0;
    while ((bytes = http->read(resource, sizeof(resource))) > 0)
    {
      count += bytes;
//      progress_update((100 * count / total) % 101);
      fp->write(resource, bytes);
    }

//    progress_hide();

    delete fp;

    delete http;

    return (name);
  }

  return (NULL);
}


//
// 'hdFile::localize()' - Localize a filename for the new working directory.
//

char *					// O  - New filename
hdFile::localize(char       *name,	// IO - Name of file
                 size_t     namelen,	// I  - Size of name buffer
                 const char *newcwd)	// I  - New directory
{
  const char	*newslash;		// Directory separator
  char		*slash;			// Directory separator
  char		cwd[1024];		// Current directory
  char		temp[1024];		// Temporary pathname


  if (!name || !name[0])
    return ("");

  getcwd(cwd, sizeof(cwd));
  if (newcwd == NULL)
    newcwd = cwd;

#if defined(WIN32)
  if (name[0] != '/' &&
      name[0] != '\\' &&
      !(isalpha(name[0]) && name[1] == ':'))
#else
  if (name[0] != '/')
#endif // WIN32
  {
    for (newslash = name; strncmp(newslash, "../", 3) == 0; newslash += 3)
    {
#if defined(WIN32)
      if ((slash = (char *)strrchr(cwd, '/')) == NULL)
        slash = (char *)strrchr(cwd, '\\');
      if (slash != NULL)
#else
      if ((slash = (char *)strrchr(cwd, '/')) != NULL)
#endif // WIN32
        *slash = '\0';
    }

    snprintf(temp, sizeof(temp), "%s/%s", cwd, newslash);
  }
  else
  {
    strlcpy(temp, name, sizeof(temp));
  }

  for (slash = temp, newslash = newcwd;
       *slash != '\0' && *newslash != '\0';
       slash ++, newslash ++)
    if ((*slash == '/' || *slash == '\\') &&
        (*newslash == '/' || *newslash == '\\'))
      continue;
    else if (*slash != *newslash)
      break;

  while (*slash != '/' && *slash != '\\' && slash > temp)
    slash --;

  if (*slash == '/' || *slash == '\\')
    slash ++;

#if defined(WIN32)
  if (isalpha(slash[0]) && slash[1] == ':')
    return (name); // Different drive letter...
#endif // WIN32

  if (*newslash != '\0')
    while (*newslash != '/' && *newslash != '\\' && newslash > newcwd)
      newslash --;

  name[0]           = '\0';
  name[namelen - 1] = '\0';

  while (*newslash != '\0')
  {
    if (*newslash == '/' || *newslash == '\\')
      strlcat(name, "../", namelen);

    newslash ++;
  }

  strlcat(name, slash, namelen);

  return (name);
}


//
// 'hdFile::scheme()' - Return the scheme for a filename or URL.
//
// Returns NULL if the URL is a local file.
//

const char *				// O - Scheme string ("http", "ftp", etc.)
hdFile::scheme(const char *myuri)	// I - Filename or URL
{
  if (!strncmp(myuri, "http:", 5))
    return ("http");
  else if (!strncmp(myuri, "https:", 6))
    return ("https");
  else if (!strncmp(myuri, "ftp:", 4))
    return ("ftp");
  else if (!strncmp(myuri, "mailto:", 7))
    return ("mailto");
  else
    return (NULL);
}


//
// 'hdFile::proxy()' - Set the proxy_ host for all HTTP requests.
//

void
hdFile::proxy(const char *proxy_url)	// I - URL of proxy_ server
{
  char	myscheme[HD_MAX_URI],		// Scheme name (must be http)
	username[HD_MAX_URI],		// Username:password information
	hostname[HD_MAX_URI],		// Hostname
	resource[HD_MAX_URI];		// Resource name
  int	port;				// Port number


  if (proxy_host_)
  {
    free(proxy_host_);
    proxy_host_ = NULL;
    proxy_port_ = 0;
  }

  if (proxy_url  && proxy_url[0])
  {
    hdHTTP::separate(proxy_url, myscheme, sizeof(myscheme),
                     username, sizeof(username),
                     hostname, sizeof(hostname), &port,
		     resource, sizeof(resource));

    if (!strcmp(myscheme, "http"))
    {
      proxy_host_ = strdup(hostname);
      proxy_port_ = port;
    }
  }
}


//
// 'hdFile::referer()' - Set the referer to use for HTTP requests.
//

void
hdFile::referer(const char *r)		// I - Referer or NULL
{
  if (referer_)
    free(referer_);

  if (r)
    referer_ = strdup(r);
  else
    referer_ = NULL;
}


//
// 'hdFile::target()' - Return the target of a link.
//

const char *				// O - Target name
hdFile::target(const char *myuri)	// I - Filename or URL
{
  const char	*baseptr,		// Pointer to directory separator
		*target;		// Pointer to target


  if (!myuri)
    return (NULL);

  if ((baseptr = (char *)strrchr(myuri, '/')) != NULL)
    baseptr ++;
#ifdef WIN32
  else if ((baseptr = (char *)strrchr(myuri, '\\')) != NULL)
    baseptr ++;
#endif // WIN32
  else
    baseptr = (char *)myuri;

  if ((target = (char *)strchr(baseptr, '#')) != NULL)
    return (target + 1);
  else
    return (NULL);
}


//
// 'hdFile::temp()' - Create and open a temporary file.
//

hdFile *				// O - Temporary file
hdFile::temp(char       *name,		// O - Filename
             size_t     namelen,	// I - Length of filename buffer
             const char *myuri)		// I - URI to associate with file
             
{
  hdCache	*temp;			// Pointer to cache entry
  int		fd;			// File descriptor
#ifdef WIN32
  char		tmpdir[1024];		// Buffer for temp dir
#else
  const char	*tmpdir;		// Temporary directory
#endif // WIN32


  // Allocate memory for the file cache as needed...
  if (temp_files_ >= temp_alloc_)
  {
    temp_alloc_ += ALLOC_FILES;

    temp = new hdCache[temp_alloc_];

    if (temp_files_)
    {
      memcpy(temp, temp_cache_, sizeof(hdCache) * temp_files_);
      delete[] temp_cache_;
    }

    temp_cache_ = temp;
  }

  // Clear a new file cache entry...
  temp = temp_cache_ + temp_files_;

  temp->name = NULL;
  temp->url  = NULL;
  temp_files_ ++;

#ifdef WIN32
  GetTempPath(sizeof(tmpdir), tmpdir);
#else
  if ((tmpdir = getenv("TMPDIR")) == NULL)
    tmpdir = "/var/tmp";
#endif // WIN32

  snprintf(name, namelen, TEMPLATE, tmpdir, getpid(), temp_files_);

  if ((fd = ::open(name, OPENMODE, OPENPERM)) >= 0)
  {
    temp->url = myuri ? strdup(myuri) : NULL;

    return (new hdStdFile(fd, HD_FILE_UPDATE));
  }
  else
  {
    temp_files_ --;
    return (NULL);
  }
}


//
// End of "$Id$".
//
