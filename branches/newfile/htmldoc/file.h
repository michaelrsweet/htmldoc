//
// "$Id: file.h 1116 2004-03-08 01:01:41Z mike $"
//
//   File class definitions for HTMLDOC, a HTML document processing program.
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

#ifndef HTMLDOC_FILE_H
#  define HTMLDOC_FILE_H

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <errno.h>
#  include "http.h"
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
  static char		proxy_host_[HD_MAX_URI];
					// HTTP proxy hostname
  static int		proxy_port_;	// HTTP proxy port
  static char		*referer_;	// HTTP referer, if any
  static int		temp_files_,	// Number of temporary files
			temp_alloc_;	// Allocated temporary files
  static hdCache	*temp_cache_;	// Cache data

  // Common per-file data...
  hdMode	mode_;			// Open mode
  long		pos_;			// Position in file
  int		error_;			// Last error

  protected:

  void		mode(hdMode m) { mode_ = m; }
  void		pos(long p) { pos_ = p; }
  void		error(int e) { error_ = e; }

  public:

  // Virtual methods that must be implemented by subclasses
  virtual ~hdFile();

  virtual int	get() = 0;
  virtual int	put(int c) = 0;
  virtual int	read(void *b, int len) = 0;
  virtual int	seek(long p, int w) = 0;
  virtual long	size() = 0;
  virtual int	write(const void *b, int len) = 0;
  virtual int	unget(int c) = 0;

  // Common methods for all file classes...
  int		error() { return error_; }
  char		*gets(char *s, int slen);
  hdMode	mode() { return (mode_); }
  long		pos() { return (pos_); }
  int		printf(const char *f, ...);
  int		puts(const char *s);

  // Static (global) methods for all files...
  static char	*basename(const char *uri, char *t, int tlen);
  static void	cleanup(void);
  static const char *cookies() { return (cookies_); }
  static void	cookies(const char *c);
  static char	*directory(const char *uri, char *t, int tlen);
  static char	*extension(const char *uri, char *t, int tlen);
  static char	*find(const char *path, const char *uri, char *name, int namelen);
  static char	*localize(char *name, int namelen, const char *newcwd);
  static const char *scheme(const char *uri);
  static hdFile	*open(const char *uri, hdMode m, const char *path = 0);
  static void	proxy(const char *url);
  static const char *proxy() { return (proxy_); }
  static void	no_local(bool b) { no_local_ = b; }
  static bool	no_local() { return (no_local_); }
  static const char *referer() { return (referer_); }
  static void	referer(const char *r);
  static const char *target(const char *uri);
  static hdFile	*temp(const char *uri, char *name, int len);
};

class hdStdFile : public hdFile		//// STDIO-based files...

{
  FILE	*fp_;				// STDIO file pointer
  long	size_;				// Size of file

  public:

  hdStdFile(const char *name, hdMode m);
  hdStdFile(FILE *f, hdMode m);
  hdStdFile(int fd, hdMode m);
  virtual ~hdStdFile();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

class hdMemFile : public hdFile		//// Memory buffer-based files...
{
  char	*buffer_,			// Start of buffer
	*current_,			// Current position in buffer
	*end_;				// End of buffer
  long	size_,				// Current size of buffer file
	alloc_size_,			// Allocated size of buffer
	max_size_,			// Maximum size of buffer
	incr_size_;			// Size increment

  public:

  hdMemFile(char *buffer, long buflen = 0, hdMode m = HD_FILE_READ);
  hdMemFile(long init_size, hdMode m, long max_size = 0, long incr_size = 0);
  virtual ~hdMemFile();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

class hdASCII85Filter : public hdFile	//// Base85 encoding filter
{
  hdFile	*chain_;		// Pointer to next file/filter
  unsigned char	buffer_[4];		// Buffer of up to 4 chars
  int		bufused_;		// Used bytes in buffer
  int		column_;		// Column in output

  public:

  hdASCII85Filter(hdFile *f);
  virtual ~hdASCII85Filter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

class hdASCIIHexFilter : public hdFile	//// Hex encoding filter
{
  hdFile	*chain_;		// Pointer to next file/filter
  int		column_;		// Column in output

  public:

  hdASCIIHexFilter(hdFile *f);
  virtual ~hdASCIIHexFilter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

class hdFlateFilter : public hdFile	//// Flate compression filter
{
  hdFile	*chain_;		// Pointer to next file/filter
  char		buffer_[16384];		// Compression buffer
  z_stream	stream_;		// Compression state

  public:

  hdFlateFilter(hdFile *f, int level = 1);
  virtual ~hdFlateFilter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
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

  hdJPEGFilter(hdFile *f, int width, int height, int depth, int quality = 50);
  virtual ~hdJPEGFilter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

class hdRC4Filter : public hdFile	//// RC4 encryption filter
{
  hdFile	*chain_;		// Pointer to next filter or file in chain
  unsigned char	sbox_[256];		// S boxes for encryption
  int		si_, sj_;		// Current indices into S boxes
  unsigned char	buffer_[16384];		// Encryption buffer

  void		init(const unsigned char *key, unsigned keylen);
  void		encrypt(const unsigned char *input, unsigned char *output,
		        unsigned len);

  public:

  hdRC4Filter(hdFile *f, const unsigned char *key, unsigned keylen);
  virtual ~hdRC4Filter();

  virtual int	get();
  virtual int	put(int c);
  virtual int	read(void *b, int len);
  virtual int	seek(long p, int w);
  virtual long	size();
  virtual int	write(const void *b, int len);
  virtual int	unget(int c);
};

#endif // !HTMLDOC_FILE_H

//
// End of "$Id: file.h 1116 2004-03-08 01:01:41Z mike $".
//
