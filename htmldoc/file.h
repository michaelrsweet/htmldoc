//
// "$Id: file.h,v 1.17 2004/02/03 02:55:28 mike Exp $"
//
//   File class definitions for HTMLDOC, a HTML document processing program.
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
// Enumerations...
//

enum hdMode				// Open modes...
{
  HD_FILE_READ,
  HD_FILE_WRITE,
  HD_FILE_UPDATE
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
  static char		proxy_host_[HD_MAX_URI];
					// HTTP proxy hostname
  static int		proxy_port_;	// HTTP proxy port

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
  static char	*directory(const char *uri, char *t, int tlen);
  static char	*extension(const char *uri, char *t, int tlen);
  static char	*find(const char *path, const char *uri, char *name, int namelen);
  static char	*localize(char *name, int namelen, const char *newcwd);
  static const char *scheme(const char *uri);
  static void	no_local() { no_local_ = 1; }
  static hdFile	*open(const char *uri, hdMode m, const char *path = 0);
  static void	proxy(const char *url);
  static const char *proxy() { return (proxy_); }
  static const char *target(const char *uri);
  static hdFile	*temp(const char *uri, char *name, int len);
};


//
// STDIO-based files...
//

class hdStdFile : public hdFile
{
  FILE	*fp_;
  long	size_;

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


//
// Memory buffer-based files...
//

class hdMemFile : public hdFile
{
  char	*buffer_,	// Start of buffer
	*current_,	// Current position in buffer
	*end_;		// End of buffer
  long	size_,		// Current size of buffer file
	alloc_size_,	// Allocated size of buffer
	max_size_,	// Maximum size of buffer
	incr_size_;	// Size increment

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


//
// File filters...
//

class hdASCII85Filter : public hdFile
{
  hdFile	*chain_;	// Pointer to next file/filter
  unsigned char	buffer_[4];	// Buffer of up to 4 chars
  int		bufused_;	// Used bytes in buffer
  int		column_;	// Column in output

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

class hdASCIIHexFilter : public hdFile
{
  hdFile	*chain_;	// Pointer to next file/filter
  int		column_;	// Column in output

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

class hdFlateFilter : public hdFile
{
  hdFile	*chain_;	// Pointer to next file/filter
  char		buffer_[16384];	// Compression buffer
  z_stream	stream_;	// Compression state

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

class hdJPEGFilter : public hdFile
{
  hdFile		*chain_;	// Pointer to next file or filter
  JOCTET		buffer_[16384];	// Compression buffer
  jpeg_compress_struct	cinfo_;		// Compression information
  struct hdJPEGDest
  {
    jpeg_destination_mgr pub_;           // Public JPEG destination manager data
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

class hdRC4Filter : public hdFile
{
  hdFile	*chain_;	// Pointer to next filter or file in chain
  unsigned char	sbox_[256];	// S boxes for encryption
  int		si_, sj_;	// Current indices into S boxes
  unsigned char	buffer_[16384];	// Encryption buffer

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
// End of "$Id: file.h,v 1.17 2004/02/03 02:55:28 mike Exp $".
//
