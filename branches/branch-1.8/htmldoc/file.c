/*
 * "$Id: file.c,v 1.13 2000/10/12 23:07:34 mike Exp $"
 *
 *   Filename routines for HTMLDOC, a HTML document processing program.
 *
 *   Copyright 1997-2000 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
 *
 * Contents:
 *
 *   file_basename()  - Return the base filename without directory or target.
 *   file_directory() - Return the directory without filename or target.
 *   file_extension() - Return the extension of a file without the target.
 *   file_find()      - Find a file in one of the path directories.
 *   file_localize()  - Localize a filename for the new working directory.
 *   file_method()    - Return the method for a filename or URL.
 *   file_proxy()     - Set the proxy host for all HTTP requests.
 *   file_target()    - Return the target of a link.
 */

/*
 * Include necessary headers.
 */

#include "file.h"
#include "http.h"
#include "progress.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/*
 * Local globals...
 */

char	proxy_host[HTTP_MAX_URI] = "";
int	proxy_port = 0;
http_t	*http = NULL;
int	web_files = 0;


/*
 * Local functions...
 */

static void	close_connection(void);


/*
 * 'file_basename()' - Return the base filename without directory or target.
 */

char *				/* O - Base filename */
file_basename(const char *s)	/* I - Filename or URL */
{
  char		*basename;	/* Pointer to directory separator */
  static char	buf[1024];	/* Buffer for files with targets */


  if (s == NULL)
    return (NULL);

  if ((basename = strrchr(s, '/')) != NULL)
    basename ++;
  else if ((basename = strrchr(s, '\\')) != NULL)
    basename ++;
#ifdef MAC
  else if ((basename = strrchr(s, ':')) != NULL)
    basename ++;
#endif /* MAC */
  else
    basename = (char *)s;

  if (basename[0] == '#')
    return (NULL);

  if (strchr(basename, '#') == NULL)
    return (basename);

  strcpy(buf, basename);
  *strchr(buf, '#') = '\0';

  return (buf);
}


/*
 * 'file_directory()' - Return the directory without filename or target.
 */

char *				/* O - Directory for file */
file_directory(const char *s)	/* I - Filename or URL */
{
  char		*dir;		/* Pointer to directory separator */
  static char	buf[1024];	/* Buffer for files with targets */


  if (s == NULL)
    return (NULL);

  strcpy(buf, s);

  if ((dir = strrchr(buf, '/')) != NULL)
    *dir = '\0';
  else if ((dir = strrchr(buf, '\\')) != NULL)
    *dir = '\0';
#ifdef MAC
  else if ((dir = strrchr(buf, ':')) != NULL)
    *dir = '\0';
#endif /* MAC */
  else
    return (".");

  if (strncmp(buf, "file:", 5) == 0)
    strcpy(buf, buf + 5);

  return (buf);
}


/*
 * 'file_extension()' - Return the extension of a file without the target.
 */

char *				/* O - File extension */
file_extension(const char *s)	/* I - Filename or URL */
{
  char		*extension;	/* Pointer to directory separator */
  static char	buf[1024];	/* Buffer for files with targets */


  if (s == NULL)
    return (NULL);

  if ((extension = strrchr(s, '/')) != NULL)
    extension ++;
  else if ((extension = strrchr(s, '\\')) != NULL)
    extension ++;
#ifdef MAC
  else if ((extension = strrchr(s, ':')) != NULL)
    extension ++;
#endif /* MAC */
  else
    extension = (char *)s;

  if ((extension = strrchr(extension, '.')) == NULL)
    return ("");
  else
    extension ++;

  if (strchr(extension, '#') == NULL)
    return (extension);

  strcpy(buf, extension);
  *strchr(buf, '#') = '\0';

  return (buf);
}


/*
 * 'file_find()' - Find a file in one of the path directories.
 */

char *					/* O - Pathname or NULL */
file_find(const char *path,		/* I - Path "dir;dir;dir" */
          const char *s)		/* I - File to find */
{
  int		retry;			/* Current retry */
  char		*temp,			/* Current position in filename */
		method[HTTP_MAX_URI],	/* Method/scheme */
		username[HTTP_MAX_URI],	/* Username:password */
		hostname[HTTP_MAX_URI],	/* Hostname */
		resource[HTTP_MAX_URI];	/* Resource */
  int		port;			/* Port number */
  const char	*connhost;		/* Host to connect to */
  int		connport;		/* Port to connect to */
  char		connpath[HTTP_MAX_URI],	/* Path for GET */
		connauth[HTTP_MAX_VALUE];/* Auth string */
  http_status_t	status;			/* Status of request... */
  FILE		*fp;			/* Web file */
  int		bytes,			/* Bytes read */
		count,			/* Number of bytes so far */
		total;			/* Total bytes in file */
  const char	*tmpdir;		/* Temporary directory */
  int		fd;			/* File descriptor */
  static char	filename[HTTP_MAX_URI];	/* Current filename */


 /*
  * If the filename is NULL, return NULL...
  */

  if (s == NULL)
    return (NULL);

  if (strncmp(s, "http:", 5) == 0 ||
      (path != NULL && strncmp(path, "http:", 5) == 0))
    strcpy(method, "http");
  else
    strcpy(method, "file");

  if (strcmp(method, "file") == 0)
  {
   /*
    * If the path is NULL or empty, return the filename...
    */

    if (path == NULL || !path[0])
      return ((char *)s);

   /*
    * Else loop through the path string until we reach the end...
    */

    while (*path != '\0')
    {
     /*
      * Copy the path directory...
      */

      temp = filename;

      while (*path != ';' && !*path && temp < (filename + sizeof(filename) - 1))
	*temp++ = *path++;

      if (*path == ';')
	path ++;

     /*
      * Append a slash as needed...
      */

      if (temp > filename && temp < (filename + sizeof(filename) - 1) &&
          resource[0] != '/')
	*temp++ = '/';

     /*
      * Append the filename...
      */

      strncpy(temp, resource, sizeof(filename) - (temp - filename));
      filename[sizeof(filename) - 1] = '\0';

     /*
      * See if the file exists...
      */

      if (!access(filename, 0))
	return (filename);
    }
  }
  else
  {
   /*
    * Remote file; try getting it from the remote system...
    */

    if (strncmp(s, "http:", 5) == 0)
      httpSeparate(s, method, username, hostname, &port, resource);
    else if (s[0] == '/')
    {
      httpSeparate(path, method, username, hostname, &port, resource);
      strcpy(resource, s);
    }
    else
    {
      if (strncmp(s, "./", 2) == 0)
        snprintf(filename, sizeof(filename), "%s/%s", path, s + 2);
      else
        snprintf(filename, sizeof(filename), "%s/%s", path, s);

      httpSeparate(filename, method, username, hostname, &port, resource);
    }

    if (resource[strlen(resource) - 1] == '/' &&
        s[strlen(s) - 1] != '/')
    {
     /*
      * This is a questionable hack, but necessary until we make the
      * interface smarter...
      */

      strcat((char *)s, "/");
    }

    if (proxy_port)
    {
     /*
      * Send request to proxy host...
      */

      connhost = proxy_host;
      connport = proxy_port;
      snprintf(connpath, sizeof(connpath), "%s://%s:%d%s", method,
               hostname, port, resource);
    }
    else
    {
     /*
      * Send request to host directly...
      */

      connhost = hostname;
      connport = port;
      strcpy(connpath, resource);
    }

    if (http != NULL && strcasecmp(http->hostname, hostname) != 0)
    {
      httpClose(http);
      http = NULL;
    }

    if (http == NULL)
    {
      progress_show("Connecting to %s...", connhost);
      atexit(close_connection);
      if ((http = httpConnect(connhost, connport)) == NULL)
      {
        progress_hide();
        progress_error("Unable to connect to %s!", connhost);
        return (NULL);
      }
    }

    httpClearFields(http);
    httpSetField(http, HTTP_FIELD_HOST, hostname);
    httpSetField(http, HTTP_FIELD_USER_AGENT, "HTMLDOC v" SVERSION);
    httpSetField(http, HTTP_FIELD_CONNECTION, "Keep-Alive");

    if (username[0])
    {
      strcpy(connauth, "Basic ");
      httpEncode64(connauth + 6, username);
      httpSetField(http, HTTP_FIELD_AUTHORIZATION, connauth);
    }

    progress_show("Getting %s...", connpath);

    for (status = HTTP_ERROR, retry = 0;
         status == HTTP_ERROR && retry < 5;
         retry ++)
    {
      if (!httpGet(http, connpath))
	while ((status = httpUpdate(http)) == HTTP_CONTINUE);
      else
	status = HTTP_ERROR;
    }

    if (status != HTTP_OK)
    {
      progress_hide();
      progress_error("HTTP error %d!", status);
      httpFlush(http);
      return (NULL);
    }

    web_files ++;

#if defined(WIN32) || defined(__EMX__)
    if ((tmpdir = getenv("TMPDIR")) == NULL)
      tmpdir = "C:/WINDOWS/TEMP";

    snprintf(filename, sizeof(filename), "%s/%06d.dat", tmpdir, web_files);
#else
    if ((tmpdir = getenv("TMPDIR")) == NULL)
      tmpdir = "/var/tmp";

    snprintf(filename, sizeof(filename), "%s/%06d.%06d", tmpdir, web_files,
             getpid());
    if ((fd = open(filename, O_CREAT | O_EXCL | O_TRUNC, 0600)) >= 0)
      close(fd);
    else
    {
      progress_hide();
      progress_error("Unable to create temp file - %s!", strerror(errno));
      httpFlush(http);
      return (NULL);
    }
#endif /* WIN32 || __EMX__ */

    if ((total = atoi(httpGetField(http, HTTP_FIELD_CONTENT_LENGTH))) == 0)
      total = 1024 * 1024;

    if ((fp = fopen(filename, "wb")) == NULL)
    {
      progress_hide();
      progress_error("Unable to create temporary file \"%s\"!", filename);
      httpFlush(http);
      return (NULL);
    }

    count = 0;
    while ((bytes = httpRead(http, resource, sizeof(resource))) > 0)
    {
      count += bytes;
      progress_update(100 * count / total);
      fwrite(resource, 1, bytes, fp);
    }

    progress_hide();

    fclose(fp);

    return (filename);
  }

  return (NULL);
}


/*
 * 'file_localize()' - Localize a filename for the new working directory.
 */

char *					/* O - New filename */
file_localize(const char *filename,	/* I - Filename */
              const char *newcwd)	/* I - New directory */
{
  const char	*newslash;		/* Directory separator */
  char		*slash;			/* Directory separator */
  char		cwd[1024];		/* Current directory */
  char		temp[1024];		/* Temporary pathname */
  static char	newfilename[1024];	/* New filename */


  if (filename[0] == '\0')
    return ("");

  getcwd(cwd, sizeof(cwd));
  if (newcwd == NULL)
    newcwd = cwd;

#if defined(WIN32) || defined(__EMX__)
  if (filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0]) && filename[1] == ':'))
#else
  if (filename[0] != '/')
#endif /* WIN32 || __EMX__ */
  {
    for (newslash = filename; strncmp(newslash, "../", 3) == 0; newslash += 3)
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
#endif /* WIN32 || __EMX__ */

    sprintf(temp, "%s/%s", cwd, newslash);
  }
  else
    strcpy(temp, filename);

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
    return ((char *)filename); /* Different drive letter... */
#endif /* WIN32 || __EMX__ */

  if (*newslash != '\0')
    while (*newslash != '/' && *newslash != '\\' && newslash > newcwd)
      newslash --;

  newfilename[0] = '\0';

  while (*newslash != '\0')
  {
    if (*newslash == '/' || *newslash == '\\')
      strcat(newfilename, "../");
    newslash ++;
  }

  strcat(newfilename, slash);

  return (newfilename);
}


/*
 * 'file_method()' - Return the method for a filename or URL.
 *
 * Returns NULL if the URL is a local file.
 */

char *				/* O - Method string ("http", "ftp", etc.) */
file_method(const char *s)	/* I - Filename or URL */
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


/*
 * 'file_proxy()' - Set the proxy host for all HTTP requests.
 */

void
file_proxy(const char *url)	/* I - URL of proxy server */
{
   char	method[HTTP_MAX_URI],	/* Method name (must be HTTP) */
	username[HTTP_MAX_URI],	/* Username:password information */
	hostname[HTTP_MAX_URI],	/* Hostname */
	resource[HTTP_MAX_URI];	/* Resource name */
  int	port;			/* Port number */


  if (url == NULL || url[0] == '\0')
  {
    proxy_host[0] = '\0';
    proxy_port    = 0;
  }
  else
  {
    httpSeparate(url, method, username, hostname, &port, resource);

    if (strcmp(method, "http") == 0)
    {
      strcpy(proxy_host, hostname);
      proxy_port = port;
    }
  }
}


/*
 * 'file_target()' - Return the target of a link.
 */

char *				/* O - Target name */
file_target(const char *s)	/* I - Filename or URL */
{
  char		*basename;	/* Pointer to directory separator */
  char		*target;	/* Pointer to target */


  if (s == NULL)
    return (NULL);

  if ((basename = strrchr(s, '/')) != NULL)
    basename ++;
  else if ((basename = strrchr(s, '\\')) != NULL)
    basename ++;
#ifdef MAC
  else if ((basename = strrchr(s, ':')) != NULL)
    basename ++;
#endif /* MAC */
  else
    basename = (char *)s;

  if ((target = strchr(basename, '#')) != NULL)
    return (target + 1);
  else
    return (NULL);
}


/*
 * 'close_connection()' - Close an open HTTP connection on exit...
 */

static void
close_connection(void)
{
  char		filename[1024];
  const char	*tmpdir;


  if (http)
  {
    httpClose(http);
    http = NULL;
  }

#if defined(WIN32) || defined(__EMX__)
    if ((tmpdir = getenv("TMPDIR")) == NULL)
      tmpdir = "C:/WINDOWS/TEMP";
#else
    if ((tmpdir = getenv("TMPDIR")) == NULL)
      tmpdir = "/var/tmp";
#endif /* WIN32 || __EMX__ */

  while (web_files > 0)
  {
#if defined(WIN32) || defined(__EMX__)
    snprintf(filename, sizeof(filename), "%s/%06d.dat", tmpdir, web_files);
#else
    snprintf(filename, sizeof(filename), "%s/%06d.%06d", tmpdir, web_files,
             getpid());
#endif /* WIN32 || __EMX__ */

    unlink(filename);
    web_files --;
  }
}


/*
 * End of "$Id: file.c,v 1.13 2000/10/12 23:07:34 mike Exp $".
 */
