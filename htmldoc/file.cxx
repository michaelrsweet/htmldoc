//
// "$Id: file.cxx,v 1.8 2004/02/03 02:55:28 mike Exp $"
//
//   Filename routines for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2004 by Easy Software Products.
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

const char	*hdFile::proxy_ = NULL;			// Proxy URL
char		hdFile::proxy_host_[HD_MAX_URI] = "";	// Proxy hostname
int		hdFile::proxy_port_ = 0;		// Proxy port
int		hdFile::temp_files_ = 0,		// Number of temporary files
		hdFile::temp_alloc_ = 0;		// Number of allocated files
hdCache		*hdFile::temp_cache_ = NULL;		// Cache array
int		hdFile::no_local_ = 0;			// Non-zero to disable local files


//
// 'hdFile::basename()' - Return the base filename without directory or target.
//

char *				// O - Base filename
hdFile::basename(const char *s,	// I - Filename or URL
                 char       *t,	// O - Base filename
		 int        tlen)// I - Size of filename buffer
{
  char		*basename;	// Pointer to directory separator
  char		*target;	// Pointer to target separator


  if (s == NULL || t == NULL)
    return (NULL);

  if ((basename = strrchr(s, '/')) != NULL)
    basename ++;
  else if ((basename = strrchr(s, '\\')) != NULL)
    basename ++;
#ifdef MAC
  else if ((basename = strrchr(s, ':')) != NULL)
    basename ++;
#endif // MAC
  else
    basename = (char *)s;

  if (basename[0] == '#')
    return (NULL);

  strncpy(t, basename, tlen - 1);
  t[tlen - 1] = '\0';

  if ((target = strchr(t, '#')) != NULL)
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
// 'hdFile::directory()' - Return the directory without filename or target.
//

char *				// O - Directory for file
hdFile::directory(const char *s,// I - Filename or URL
                  char       *t,// O - Base filename
		  int        tlen)// I - Size of filename buffer
{
  char		*dir;		// Pointer to directory separator


  if (s == NULL || t == NULL)
    return (NULL);

  if (strncmp(s, "http://", 7) == 0 || strncmp(s, "https://", 8) == 0)
  {
    // Handle URLs...
    char	scheme[HD_MAX_URI],
		username[HD_MAX_URI],
		hostname[HD_MAX_URI],
		resource[HD_MAX_URI];
    int		port;


    hdHTTP::separate(s, scheme, sizeof(scheme), username, sizeof(username),
                     hostname, sizeof(hostname), &port, resource,
		     sizeof(resource));
    if ((dir = strrchr(resource, '/')) != NULL)
      *dir = '\0';

    if (username[0])
      snprintf(t, tlen, "%s://%s@%s:%d%s", scheme, username, hostname,
               port, resource);
    else
      snprintf(t, tlen, "%s://%s:%d%s", scheme, hostname, port,
               resource);
  }
  else
  {
    // Normal stuff...
    strncpy(t, s, tlen - 1);
    t[tlen - 1] = '\0';

    if ((dir = strrchr(t, '/')) != NULL)
      *dir = '\0';
    else if ((dir = strrchr(t, '\\')) != NULL)
      *dir = '\0';
#ifdef MAC
    else if ((dir = strrchr(t, ':')) != NULL)
      *dir = '\0';
#endif // MAC
    else
      return (".");

    if (strncmp(t, "file:", 5) == 0)
      strcpy(t, t + 5);
  }

  return (t);
}


//
// 'hdFile::extension()' - Return the extension of a file without the target.
//

char *				// O - File extension
hdFile::extension(const char *s,// I - Filename or URL
                  char       *t,// O - Base filename
		  int        tlen)// I - Size of filename buffer
{
  char		*extension;	// Pointer to directory separator
  char		*target;	// Pointer to target separator


  if (s == NULL || t == NULL)
    return (NULL);

  if ((extension = strrchr(s, '/')) != NULL)
    extension ++;
  else if ((extension = strrchr(s, '\\')) != NULL)
    extension ++;
#ifdef MAC
  else if ((extension = strrchr(s, ':')) != NULL)
    extension ++;
#endif // MAC
  else
    extension = (char *)s;

  if ((extension = strrchr(extension, '.')) == NULL)
    t[0] = '\0';
  else
  {
    extension ++;

    strncpy(t, extension, tlen - 1);
    t[tlen - 1] = '\0';

    if ((target = strchr(t, '#')) != NULL)
      *target = '\0';
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
	     int        namelen)	// I - Size of filename buffer
{
  int		i;			// Looping var
  int		retry;			// Current retry
  char		*temp,			// Current position in filename
		scheme[HD_MAX_URI],	// Method/scheme
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
  if (uri == NULL || name == NULL)
    return (NULL);

  if (strncmp(uri, "http:", 5) == 0 || strncmp(uri, "//", 2) == 0 ||
      (path != NULL && strncmp(path, "http:", 5) == 0))
    strcpy(scheme, "http");
#ifdef HAVE_LIBSSL
  else if (strncmp(uri, "https:", 6) == 0 ||
           (path != NULL && strncmp(path, "https:", 6) == 0))
    strcpy(scheme, "https");
#endif // HAVE_LIBSSL
  else
    strcpy(scheme, "file");

  if (strcmp(scheme, "file") == 0)
  {
    // If we are not allowing access to local files, return NULL...
    if (no_local_)
    {
      // See if we are accessing a known temp file...
      for (i = 0; i < temp_files_; i ++)
        if (strcmp(uri, temp_cache_[i].name) == 0)
	{
	  strncpy(name, uri, namelen - 1);
	  name[namelen - 1] = '\0';

	  return (name);
	}

      return (NULL);
    }

    // If the path is NULL or empty, return the filename...
    if (path == NULL || !path[0])
    {
      strncpy(name, uri, namelen - 1);
      name[namelen - 1] = '\0';

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
      strncpy(temp, uri, namelen - (temp - name) - 1);
      name[namelen - 1] = '\0';

      // See if the file exists...
      if (!access(name, 0))
	return (name);
    }

    // If the name exists, return the name...
    if (!access(uri, 0))
    {
      strncpy(name, uri, namelen - 1);
      name[namelen - 1] = '\0';

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
    if (strncmp(uri, "http:", 5) == 0 || strncmp(uri, "//", 2) == 0 ||
        strncmp(uri, "https:", 6) == 0)
#else
    if (strncmp(uri, "http:", 5) == 0 || strncmp(uri, "//", 2) == 0)
#endif // HAVE_LIBSSL
      hdHTTP::separate(uri, scheme, sizeof(scheme), username, sizeof(username),
                       hostname, sizeof(hostname), &port, resource,
		       sizeof(resource));
    else if (uri[0] == '/')
    {
      hdHTTP::separate(path, scheme, sizeof(scheme), username, sizeof(username),
                       hostname, sizeof(hostname), &port, resource,
		       sizeof(resource));
      strncpy(resource, uri, sizeof(resource) - 1);
      resource[sizeof(resource) - 1] = '\0';
    }
    else
    {
      if (strncmp(uri, "./", 2) == 0)
        snprintf(name, namelen, "%s/%s", path, uri + 2);
      else
        snprintf(name, namelen, "%s/%s", path, uri);

      hdHTTP::separate(name, scheme, sizeof(scheme), username,
                       sizeof(username), hostname, sizeof(hostname), &port,
		       resource, sizeof(resource));
    }

    for (status = HD_HTTP_ERROR, retry = 0;
         status == HD_HTTP_ERROR && retry < 5;
         retry ++)
    {
      if (proxy_port_)
      {
        // Send request to proxy_ host...
        connhost = proxy_host_;
        connport = proxy_port_;
        snprintf(connpath, sizeof(connpath), "%s://%s:%d%s", scheme,
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
      if (strcmp(scheme, "http") == 0)
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
      http->set_field(HD_HTTP_FIELD_USER_AGENT, "HTMLDOC v" HD_SVERSION);
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
	                 scheme, sizeof(scheme), username, sizeof(username),
                	 hostname, sizeof(hostname), &port, resource,
			 sizeof(resource));
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

    if ((fp = hdFile::temp(uri, name, namelen)) == NULL)
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
                 int        namelen,	// I  - Size of name buffer
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

#if defined(WIN32) || defined(__EMX__)
  if (name[0] != '/' &&
      name[0] != '\\' &&
      !(isalpha(name[0]) && name[1] == ':'))
#else
  if (name[0] != '/')
#endif // WIN32 || __EMX__
  {
    for (newslash = name; strncmp(newslash, "../", 3) == 0; newslash += 3)
#if defined(WIN32) || defined(__EMX__)
    {
      if ((slash = strrchr(cwd, '/')) == NULL)
        slash = strrchr(cwd, '\\');
      if (slash != NULL)
        *slash = '\0';
    }
#else
      if ((slash = strrchr(cwd, '/')) != NULL)
        *slash = '\0';
#endif // WIN32 || __EMX__

    snprintf(temp, sizeof(temp), "%s/%s", cwd, newslash);
  }
  else
  {
    strncpy(temp, name, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
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

#if defined(WIN32) || defined(__EMX__)
  if (isalpha(slash[0]) && slash[1] == ':')
    return (name); // Different drive letter...
#endif // WIN32 || __EMX__

  if (*newslash != '\0')
    while (*newslash != '/' && *newslash != '\\' && newslash > newcwd)
      newslash --;

  name[0]           = '\0';
  name[namelen - 1] = '\0';

  while (*newslash != '\0')
  {
    if (*newslash == '/' || *newslash == '\\')
      strncat(name, "../", sizeof(name) - 1);

    newslash ++;
  }

  strncat(name, slash, sizeof(name) - 1);

  return (name);
}


//
// 'hdFile::scheme()' - Return the scheme for a filename or URL.
//
// Returns NULL if the URL is a local file.
//

const char *			// O - Method string ("http", "ftp", etc.)
hdFile::scheme(const char *s)	// I - Filename or URL
{
  if (strncmp(s, "http:", 5) == 0)
    return ("http");
  else if (strncmp(s, "https:", 6) == 0)
    return ("https");
  else if (strncmp(s, "ftp:", 4) == 0)
    return ("ftp");
  else if (strncmp(s, "mailto:", 7) == 0)
    return ("mailto");
  else
    return (NULL);
}


//
// 'hdFile::proxy()' - Set the proxy_ host for all HTTP requests.
//

void
hdFile::proxy(const char *url)	// I - URL of proxy_ server
{
  char	scheme[HD_MAX_URI],	// Method name (must be http)
	username[HD_MAX_URI],	// Username:password information
	hostname[HD_MAX_URI],	// Hostname
	resource[HD_MAX_URI];	// Resource name
  int	port;			// Port number


  if (url == NULL || url[0] == '\0')
  {
    proxy_host_[0] = '\0';
    proxy_port_    = 0;
  }
  else
  {
    hdHTTP::separate(url, scheme, sizeof(scheme), username, sizeof(username),
                     hostname, sizeof(hostname), &port, resource,
		     sizeof(resource));

    if (strcmp(scheme, "http") == 0)
    {
      strncpy(proxy_host_, hostname, sizeof(proxy_host_) - 1);
      proxy_host_[sizeof(proxy_host_) - 1] = '\0';
      proxy_port_ = port;
    }
  }
}


//
// 'hdFile::target()' - Return the target of a link.
//

const char *			// O - Target name
hdFile::target(const char *s)	// I - Filename or URL
{
  char		*basename;	// Pointer to directory separator
  char		*target;	// Pointer to target


  if (s == NULL)
    return (NULL);

  if ((basename = strrchr(s, '/')) != NULL)
    basename ++;
  else if ((basename = strrchr(s, '\\')) != NULL)
    basename ++;
#ifdef MAC
  else if ((basename = strrchr(s, ':')) != NULL)
    basename ++;
#endif // MAC
  else
    basename = (char *)s;

  if ((target = strchr(basename, '#')) != NULL)
    return (target + 1);
  else
    return (NULL);
}


//
// 'hdFile::temp()' - Create and open a temporary file.
//

hdFile *				// O - Temporary file
hdFile::temp(const char *uri,		// I - URI to associate with file
             char       *name,		// O - Filename
             int        len)		// I - Length of filename buffer
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
    temp_alloc_ += HD_ALLOC_FILES;

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

  snprintf(name, len, TEMPLATE, tmpdir, getpid(), temp_files_);

  if ((fd = ::open(name, OPENMODE, OPENPERM)) >= 0)
  {
    temp->url = uri ? strdup(uri) : NULL;

    return (new hdStdFile(fd, HD_FILE_UPDATE));
  }
  else
  {
    temp_files_ --;
    return (NULL);
  }
}


//
// End of "$Id: file.cxx,v 1.8 2004/02/03 02:55:28 mike Exp $".
//
