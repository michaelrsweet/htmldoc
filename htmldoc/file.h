//
// "$Id$"
//
// File class definitions for HTMLDOC, a HTML document processing program.
//
// Copyright 1997-2009 Easy Software Products.
//
// These coded instructions, statements, and computer programs are the
// property of Easy Software Products and are protected by Federal
// copyright law.  Distribution and use rights are outlined in the file
// "COPYING.txt" which should have been included with this file.  If this
// file is missing or damaged please contact Easy Software Products
// at:
//
//     Attn: HTMLDOC Licensing Information
//     Easy Software Products
//     516 Rio Grand Ct
//     Morgan Hill, CA 95037 USA
//
//     http://www.htmldoc.org/
//

#ifndef HTMLDOC_FILE_H
#  define HTMLDOC_FILE_H

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <errno.h>
#  include "http.h"
#  include "rc4.h"
#  include <zlib.h>

extern "C"
{
#  include <jpeglib.h>
}


//
// Types, structures, and classes...
//

enum hdMode				//// Open modes...
{
  HD_FILE_READ,				// Open for reading
  HD_FILE_WRITE,			// Open for writing
  HD_FILE_UPDATE			// Open for read/write
};

struct hdCache				//// HTTP cache data
{
  char	*name;				// Temporary filename
  char	*url;				// URL
};

class hdFile				//// Base file class...
{
  // Global class state data...
  static char		*cookies_;	// HTTP cookies, if any
  static bool		no_local_;	// Allow local files to be opened?

  static char		*proxy_;	// HTTP proxy URL
  static char		*proxy_host_;	// HTTP proxy hostname
  static int		proxy_port_;	// HTTP proxy port
  static char		*referer_;	// HTTP referer, if any
  static int		temp_files_,	// Number of temporary files
			temp_alloc_;	// Allocated temporary files
  static hdCache	*temp_cache_;	// Cache data

  // Common per-file data...
  int			error_;		// Last error
  hdMode		mode_;		// Open mode
  size_t		pos_;		// Position in file
  char			*uri_;		// URI of opened file

  protected:

  void			error(int e) { error_ = e; }
  void			mode(hdMode m) { mode_ = m; }
  void			pos(size_t p) { pos_ = p; }
  void			uri(const char *u);

  public:

  // Virtual methods that must be implemented by subclasses
  virtual		~hdFile();

  virtual int		get() = 0;
  virtual int		put(int c) = 0;
  virtual ssize_t	read(void *b, size_t len) = 0;
  virtual ssize_t	seek(ssize_t p, int whence) = 0;
  virtual size_t	size() = 0;
  virtual ssize_t	write(const void *b, size_t len) = 0;
  virtual int		unget(int c) = 0;

  // Common methods for all file classes...
  char			*basename(char *t, size_t tlen)
			{ return (basename(uri_, t, tlen)); }
  char			*dirname(char *t, size_t tlen)
			{ return (dirname(uri_, t, tlen)); }
  int			error() { return error_; }
  const char		*error_string();
  char			*extension(char *t, size_t tlen)
			{ return (extension(uri_, t, tlen)); }
  char			*getline(char *s, size_t slen);
  char			*gets(char *s, size_t slen);
  hdMode		mode() { return (mode_); }
  size_t		pos() { return (pos_); }
  ssize_t		printf(const char *f, ...);
  ssize_t		putline(const char *s);
  ssize_t		puts(const char *s);
  const char		*scheme() { return (scheme(uri_)); }
  const char		*target() { return (target(uri_)); }
  const char		*uri() { return (uri_); }

  // Static (global) methods for all files...
  static char		*basename(const char *myuri, char *t, size_t tlen);
  static void		cleanup(void);
  static const char	*cookies() { return (cookies_); }
  static void		cookies(const char *cookie_data);
  static char		*dirname(const char *myuri, char *t, size_t tlen);
  static char		*extension(const char *myuri, char *t, size_t tlen);
  static char		*find(const char *path, const char *myuri, char *name,
			      size_t namelen);
  static char		*localize(char *name, size_t namelen,
			          const char *newcwd);
  static hdFile		*open(const char *myuri, hdMode m,
			      const char *path = 0);
  static void		proxy(const char *proxy_url);
  static const char	*proxy() { return (proxy_); }
  static void		no_local(bool b) { no_local_ = b; }
  static bool		no_local() { return (no_local_); }
  static const char	*referer() { return (referer_); }
  static void		referer(const char *r);
  static const char	*scheme(const char *myuri);
  static const char	*target(const char *myuri);
  static hdFile		*temp(char *name, size_t namelen,
			      const char *myuri = (const char *)0);
};

class hdStdFile : public hdFile		//// STDIO-based files...

{
  FILE			*fp_;		// STDIO file pointer
  size_t		size_;		// Size of file

  public:

			hdStdFile(const char *name, hdMode m);
			hdStdFile(FILE *f, hdMode m);
			hdStdFile(int fd, hdMode m);
  virtual		~hdStdFile();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdMemFile : public hdFile		//// Memory buffer-based files...
{
  char			*buffer_,	// Start of buffer
			*current_,	// Current position in buffer
			*end_;		// End of buffer
  size_t		size_,		// Current size of buffer file
			alloc_size_,	// Allocated size of buffer
			max_size_,	// Maximum size of buffer
			incr_size_;	// Size increment

  public:

			hdMemFile(char *buffer, size_t buflen = 0,
			          hdMode m = HD_FILE_READ);
			hdMemFile(size_t init_size, hdMode m,
				  size_t max_size = 0, size_t incr_size = 0);
  virtual		~hdMemFile();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdASCII85Filter : public hdFile	//// Base85 encoding filter
{
  hdFile		*chain_;	// Pointer to next file/filter
  unsigned char		buffer_[4];	// Buffer of up to 4 chars
  int			bufused_;	// Used bytes in buffer
  int			column_;	// Column in output

  public:

			hdASCII85Filter(hdFile *f);
  virtual		~hdASCII85Filter();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdASCIIHexFilter : public hdFile	//// Hex encoding filter
{
  hdFile		*chain_;	// Pointer to next file/filter
  int			column_;	// Column in output

  public:

			hdASCIIHexFilter(hdFile *f);
  virtual		~hdASCIIHexFilter();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdFlateFilter : public hdFile	//// Flate compression filter
{
  hdFile		*chain_;	// Pointer to next file/filter
  char			buffer_[16384];	// Compression buffer
  z_stream		stream_;	// Compression state

  public:

			hdFlateFilter(hdFile *f, int level = 1);
  virtual		~hdFlateFilter();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdJPEGFilter : public hdFile	//// JPEG compression filter
{
  hdFile		*chain_;	// Pointer to next file or filter
  JOCTET		buffer_[16384];	// Compression buffer
  jpeg_compress_struct	cinfo_;		// Compression information
  struct hdJPEGDest
  {
    jpeg_destination_mgr pub_;		// Public JPEG destination manager data
    hdJPEGFilter	*jpeg_filter_;	// JPEG filter pointer
  }			dest_mgr_;	// Destination manager
  jpeg_error_mgr	error_mgr_;	// Error manager

  static void		init(j_compress_ptr cinfo);
  static boolean	empty(j_compress_ptr cinfo);
  static void		term(j_compress_ptr cinfo);

  public:

			hdJPEGFilter(hdFile *f, int width, int height,
			             int depth, int quality = 50);
  virtual		~hdJPEGFilter();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

class hdRC4Filter : public hdFile	//// RC4 encryption filter
{
  hdFile		*chain_;	// Pointer to next filter or file in chain
  hdByte		buffer_[16384];	// Encryption buffer
  hdRC4			rc4_;		// RC4 context

  public:

			hdRC4Filter(hdFile *f, const hdByte *key,
			            size_t keylen);
  virtual		~hdRC4Filter();

  virtual int		get();
  virtual int		put(int c);
  virtual ssize_t	read(void *b, size_t len);
  virtual ssize_t	seek(ssize_t p, int w);
  virtual size_t	size();
  virtual ssize_t	write(const void *b, size_t len);
  virtual int		unget(int c);
};

#endif // !HTMLDOC_FILE_H

//
// End of "$Id$".
//
