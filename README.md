[![Snap Status](https://build.snapcraft.io/badge/michaelrsweet/htmldoc.svg)](https://build.snapcraft.io/user/michaelrsweet/htmldoc)

# Introduction

HTMLDOC is a program that reads HTML and Markdown source files or web pages and
generates corresponding EPUB, HTML, PostScript, or PDF files with an optional
table of contents.

HTMLDOC was developed in the 1990's as a documentation generator for my previous
company, and has since seen a lot of usage as a report generator embedded in web
servers.  However, it does not support many things in "the modern web", such as:

- Cascading Style Sheets (CSS): While I have experimented with adding CSS
  support to HTMLDOC, proper CSS support is non-trivial especially for paged
  output (which is not well supported by CSS)

- Encryption: HTMLDOC currently supports the older (and very insecure) PDF 1.4
  (128-bit RC4) encryption.  I have looked at supporting AES (256-bit)
  encryption...

- Forms: HTML forms and PDF forms are very different things.  While I have had
  many requests to add support for PDF forms over the years, I have not found a
  satisfactory way to do so.

- Tables: HTMLDOC supports HTML 3.2 tables with basic support for TBODY and
  THEAD.

- Unicode: While HTMLDOC does support UTF-8 for "Western" languages, there is
  absolutely no support for languages that require dynamic rewriting or
  right-to-left text formatting.  Basically this means you can't use HTMLDOC
  to format Arabic, Chinese, Hebrew, Japanese, or other languages that are not
  based on latin-based alphabets that read left-to-right.

- Emoji: The fonts bundled with HTMLDOC do not include Unicode Emoji characters.


# Resources

The following HTMLDOC resources are available online:

- Official web site and online documentation:

    https://michaelrsweet.github.io/htmldoc

- Issue tracker and questions:

    https://github.com/michaelrsweet/htmldoc/issues


# Using HTMLDOC

Note: Complete documentation for HTMLDOC is available in the "doc" subdirectory.
The following provides basic information on using HTMLDOC at the command-line
and does not discuss the GUI or web server functionality.

HTMLDOC accepts a list of HTML and/or Markdown "source" files and will generate
EPUB, HTML, PostScript, or PDF output via command-line options.  A summary of
command-line options can be shown with the "--help" option:

    htmldoc --help

HTMLDOC normally expects "structured" documents, with chapters, etc.  Chapters
begin with a H1 markup and continue to the end of the listed HTML files or the
next H1 markup, whichever comes first.  To convert unstructured documents such
as web pages, use the "--webpage" option to HTMLDOC:

    htmldoc --webpage ...

To generate a Level 2 PostScript file you might use:

    htmldoc -f outfile.ps chapter1.html ... chapterN.html

Similarly you can generate an EPUB or PDF file of the same source files using:

    htmldoc -f outfile.epub chapter1.html ... chapterN.html

    htmldoc -f outfile.pdf chapter1.html ... chapterN.html

Finally, to generate HTML files for viewing (with a linked table-of-contents) do
the following:

    htmldoc -t html -d outdir chapter1.html ... chapterN.html

or:

    htmldoc -t html -f outfile.html chapter1.html ... chapterN.html

A complete description of all command-line options and HTML guidelines can be
found in the software users manual in the "doc" directory.


# Credits

Many thanks to Leonard Rosenthol for providing changes to support a macOS
version of HTMLDOC.

The table VALIGN and "HALF PAGE" code was contributed by D. Richard Hipp.

The multiple header/footer image code was contributed by Lynn Pye.

The RC4 encryption code is from librc4 1.1 by the folks at Carnegie Mellon
University.

The MD5 hash code is from L. Peter Deutsch at Aladdin Enterprises (creators
of Ghostscript, among other things).


# Legal Stuff

HTMLDOC is copyright © 1997-2017 by Michael R Sweet.  This program is free
software.  Distribution and use rights are outlined in the file "COPYING".

HTMLDOC includes code to encrypt PDF document files using the RC4 algorithm
with up to a 128-bit key. While this software and code may be freely used
and exported under current US laws, other countries may restrict your use
and possession of this code and software.

The Adobe Portable Document Format is Copyright 1985-2005 by Adobe Systems
Incorporated. Adobe, FrameMaker, and PostScript are registered trademarks of
Adobe Systems, Incorporated.

The Graphics Interchange Format is the copyright and GIF is the service mark
property of CompuServe Incorporated.

Intel is a registered trademark of Intel Corporation.

Linux is a registered trademark of Linus Torvalds.

Mac OS is a registered trademark of Apple Inc.

Microsoft and Windows are registered trademarks of Microsoft Corporation.

Solaris is a registered trademark of Sun Microsystems, Inc.

SPARC is a registered trademark of SPARC International, Inc.

UNIX is a registered trademark of the X/Open Company, Ltd.

This software is based in part on the work of the Independent JPEG Group and
FLTK project.
