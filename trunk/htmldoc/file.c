/*
 * "$Id: file.c,v 1.7 2000/06/29 01:15:56 mike Exp $"
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
 *   file_localize()  - Localize a filename for the new working directory.
 *   file_method()    - Return the method for a filename or URL.
 *   file_target()    - Return the target of a link.
 */

/*
 * Include necessary headers.
 */

#include "file.h"


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
  char		*temp;			/* Current position in filename */
  static char	filename[1024];		/* Current filename */


 /*
  * If the path or filename is NULL, return the filename...
  */

  if (path == NULL || !path[0] || s == NULL)
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
        s[0] != '/')
      *temp++ = '/';

   /*
    * Append the filename...
    */

    strncpy(temp, s, sizeof(filename) - (temp - filename));
    filename[sizeof(filename) - 1] = '\0';

   /*
    * See if the file exists...
    */

    if (!access(filename, 0))
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
 * End of "$Id: file.c,v 1.7 2000/06/29 01:15:56 mike Exp $".
 */
