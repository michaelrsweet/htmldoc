//
// "$Id: file.h,v 1.10 2001/12/07 18:26:58 mike Exp $"
//
//   File class definitions for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2001 by Easy Software Products.
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

#ifndef HTMLDOC_FILE_H
#  define HTMLDOC_FILE_H

//
// Include necessary headers...
//

#  include <stdio.h>


//
// Enumerations...
//

enum hdMode				// Open modes...
{
  HD_MODE_READ,
  HD_MODE_WRITE,
  HD_MODE_UPDATE
};


//
// HTTP cache data...
//

struct hdCache
{
  char	*name;				// Temporary filename
  char	*url;				// URL
};


//
// Base file class...
//

class hdFile
{
  // Global class state data...
  static int		no_local_;	// Allow local files to be opened?

  static const char	*proxy_;	// HTTP proxy URL
  static char		proxy_host_[256];// HTTP proxy hostname
  static int		proxy_port_;	// HTTP proxy port

  static int		temp_files_,	// Number of temporary files
			temp_alloc_;	// Allocated temporary files
  static hdCache	*temp_cache_;	// Cache data

  // Common per-file data...
  hdMode	mode_;			// Open mode
  long		pos_;			// Position in file

  protected:

  void		mode(hdMode m) { mode_ = m; }
  void		pos(long p) { pos_ = p; }

  public:

  // Virtual methods that must be implemented by subclasses
  virtual ~hdFile();

  virtual int	get() = 0;
  virtual int	put(int c) = 0;
  virtual int	read(void *b, int len) = 0;
  virtual int	seek(long pos, int whence) = 0;
  virtual int	size() = 0;
  virtual int	write(void *b, int len) = 0;
  virtual int	unget(int c) = 0;

  // Common methods for all file classes...
  char		*gets(char *s, int slen);
  hdMode	mode() { return (mode_); }
  long		pos() { return (pos_); }
  int		printf(const char *f, ...);
  int		puts(const char *s);

  // Static (global) methods for all files...
  static char	*basename(const char *uri, char *t, int tlen);
  static void	cleanup(void);
  static char	*directory(const char *uri, char *t, int tlen);
  static char	*extension(const char *uri, char *t, int tlen);
  static char	*find(const char *path, char *uri, int urilen, char *name, int namelen);
  static char	*localize(char *name, int namelen, const char *newcwd);
  static char	*method(const char *uri, char *meth, int methlen);
  static void	no_local() { no_local_ = 1; }
  static hdFile	*open(const char *uri, hdMode m);
  static void	proxy(const char *url);
  static const char *proxy() { return (proxy_); }
  static const char *target(const char *uri);
  static hdFile	*temp(const char *uri, char *name, int len);
};


//
// STDIO-based files...
//

class hdStdFile public hdFile
{
  FILE	*fp_;

  public:

  hdStdFile(const char *name, hdMode m);
  hdStdFile(FILE *f, hdMode m);
  virtual ~hdStdFile();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long pos, hdWhence whence);
  virtual int	size();
  virtual int	write(void *b, int len);
  virtual int	unget(int c);
}


//
// File filter...
//

class hdFlateFilter public hdFile
{
  hdFile	*chain_;
  char		buffer_[16384];
  void		*state_;

  public:

  hdFlateFilter(hdFile *f, int level = 1);
  virtual ~hdFlateFilter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long pos, hdWhence whence);
  virtual int	size();
  virtual int	write(void *b, int len);
  virtual int	unget(int c);
};

class hdJPEGFilter public hdFile
{
  hdFile	*chain_;
  char		*buffer_;
  int		bufsize_,
		bufused_;
  void		*state_;

  public:

  hdJPEGFilter(hdFile *f, int width, int height, int depth, int quality = 50);
  virtual ~hdJPEGFilter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long pos, hdWhence whence);
  virtual int	size();
  virtual int	write(void *b, int len);
  virtual int	unget(int c);
};

class hdRC4Filter public hdFile
{
  hdFile	*chain_;
  void		*state_;

  public:

  hdRC4Filter(hdFile *f, unsigned char *key, int keylen);
  virtual ~hdRC4Filter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long pos, int whence);
  virtual int	size();
  virtual int	write(void *b, int len);
  virtual int	unget(int c);
};

#endif // !HTMLDOC_FILE_H

//
// End of "$Id: file.h,v 1.10 2001/12/07 18:26:58 mike Exp $".
//
