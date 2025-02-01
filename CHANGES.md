Changes in HTMLDOC
==================

v1.9.21 (YYYY-MM-DD)
--------------------

- Updated HTTP/HTTPS connection error reporting to include the reason.
- Updated markdown parser.
- Fixed a bug in the new PDF link code (Issue #536)
- Fixed a bug in the number-up code (Issue #539)


v1.9.20 (2024-12-09)
--------------------

- Fixed a regression that caused spaces to disappear between some words
  (Issue #533)
- Fixed resolution of relative links within a document (Issue #534)


v1.9.19 (2024-11-21)
--------------------

- Security: Fixed an issue with the `file_basename` implementation (Issue #532)
- Added support for 'file' method in links (Issue #512)
- Updated HTML and header/footer code to use a string pool to simplify memory
  management and fix potential double-free bugs.
- Updated configure script to look for zlib with pkg-config (Issue #519)
- Updated markdown support code to mmd.
- Fixed hyperlinks to subfolders (Issue #525)
- Fixed export of UTF-8 HTML (Issue #526)
- Fixed handling of whitespace-only nodes (Issue #528)
- Fixed handling of tabs in PRE nodes (Issue #529)
- Fixed case sensitivity of link targets (Issue #530)


v1.9.18 (2024-02-11)
--------------------

- Fixed table rendering when there are missing `</tr>` (Issue #494)
- Fixed support for links of the form "filename.html#anchor" in PDF output
  (Issue #514)
- Fixed `--header1` support for web page output (Issue #515)
- Fixed markdown emphasized, strong, and struck-through text (Issue 517)


v1.9.17 (2023-09-17)
--------------------

- Added new `--pre-indent` option to control indentation of pre-formatted text
  (Issue #505)
- Now link to CUPS library instead of embedding its HTTP code.
- Updated PostScript and PDF date/time information to use UTC (Issue #490)
- Fixed multiple conversions of UTF-8 HTML files from the GUI (Issue #496)
- Fixed a compile bug on Solaris (Issue #498)
- Fixed a markdown parsing bug (Issue #503)
- Fixed a relative URL handling bug (Issue #507)
- Fixed a crash bug with bad title images (Issue #510)
- Fixed some minor CodeQL warnings.


v1.9.16 (2022-05-19)
--------------------

- Added support for `$DATE(format)` and `$TIME(format)` header/footer strings
  (Issue #472)
- Fixed a potential image overflow bug with JPEG and PNG images (Issue #471)
- Fixed potential heap overflow bugs with pages (Issue #477, Issue #478,
  Issue #480, Issue #482, Issue #483)
- Fixed potential use-after-free in blocks (Issue #484)
- Updated the GNU TLS HTTPS support code to use a faster connection shutdown
  mode (Issue #487)
- Fixed some minor Coverity warnings.
- Updated the GUI interface for current display fonts.


v1.9.15 (2022-02-05)
--------------------

- Fixed a potential heap overflow bug with GIF images (Issue #461)
- Fixed a potential double-free bug with PNG images (Issue #462)
- Fixed a potential stack overflow bug with GIF images (Issue #463)
- Fixed a potential heap underflow bug with empty attributes (Issue #464)
- Fixed a potential stack overflow bug with BMP images (Issue #466)
- Fixed a potential heap overflow bug with the table-of-contents (Issue #467)
- Fixed a potential heap overflow bug with headings (Issue #468)
- Fixed a potential stack overflow bug with GIF images (Issue #470)


v1.9.14 (2021-12-22)
--------------------

- BMP image support is now deprecated and will be removed in a future
  release of HTMLDOC.
- Fixed a potential stack overflow bug with GIF images.
- Fixed the PDF creation date (Issue #455)
- Fixed a potential stack overflow bug with BMP images (Issue #456)
- Fixed a compile issue when libpng was not available (Issue #458)


v1.9.13 (2021-11-05)
--------------------

- Now install a 32x32 icon for Linux (Issue #432)
- Fixed an issue with large values for roman numerals and letters in headings
  (Issue #433)
- Fixed a crash bug when a HTML comment contains an invalid nul character
  (Issue #439)
- Fixed a crash bug with bogus BMP images (Issue #444)
- Fixed a potential heap overflow bug with bogus GIF images (Issue #451)
- Fixed a potential stack overflow bug with bogus BMP images (Issue #453)


v1.9.12 (2021-05-17)
--------------------

- Fixed a crash bug with "data:" URIs and EPUB output (Issue #410)
- Fixed crash bugs for books (Issue #412, Issue #414)
- Fixed a number-up crash bug (Issue #413)
- Fixed JPEG error handling (Issue #415)
- Fixed crash bugs with bogus table attributes (Issue #416, Issue #417)
- Fixed a crash bug with malformed URIs (Issue #418)
- Fixed a crash bug with malformed GIF files (Issue #423)
- Fixed a crash bug with empty titles (Issue #425)
- Fixed crash bugs with bogus text (Issue #426, Issue #429, Issue #430,
  Issue #431)
- Fixed some issues reported by Coverity.
- Removed the bundled libjpeg, libpng, and zlib.


v1.9.11 (2020-12-24)
--------------------

- Added high-resolution desktop icons for Linux.
- Updated the internal HTTP library to fix truncation of redirection URLs
  (Issue #396)
- Fixed a regression in the handling of character entities for UTF-8 input
  (Issue #401)
- The `--numbered` option did not work when the table-of-contents was disabled
  (Issue #405)


v1.9.10 (2020-09-05)
--------------------

- Updated local zlib to v1.2.11.
- Updated local libpng to v1.6.37.
- Fixed packaging issues on macOS and Windows (Issue #377, Issue #386)
- Now ignore sRGB profile errors in PNG files (Issue #390)
- The GUI would crash when saving (Issue #391)
- Page comments are now allowed in `pre` text (Issue #394)


v1.9.9 (2020-06-11)
-------------------

- Fixed a redirection issue - some sites (incorrectly) provide an incomplete
  Location: URL in the HTTP response.
- Fixed https: support on newer versions of Windows (Issue #378)
- Fixed a problem with remote URLs containing spaces (Issue #379)
- Fixed a UTF-8 processing bug for Markdown files (Issue #383)
- Added support for `<FONT FACE="monospace">` (Issue #385)


v1.9.8 (2020-02-15)
-------------------

- Added support for a `HTMLDOC.filename` META keyword that controls the filename
  reported in CGI mode; the default remains "htmldoc.pdf" (Issue #367)
- Fixed a paragraph formatting issue with large inline images (Issue #369)
- Fixed a buffer underflow issue (Issue #370)
- Fixed PDF page numbers (Issue #371)
- Added support for a new `L` header/footer format (`$LETTERHEAD`), which
  inserts a letterhead image at its full size (Issue #372, Issue #373,
  Issue #375)
- Updated the build documentation (Issue #374)


v1.9.7 (2019-10-09)
-------------------

- Refactored the PRE rendering code to work around compiler optimization bugs
  (Issue #349)
- Added support for links with targets (Issue #351)
- Fixed a table rowspan + valign bug (Issue #360)


v1.9.6 (2019-09-25)
-------------------

- Added support for data URIs (Issue #340)
- HTMLDOC no longer includes a PDF table of contents when converting a single
  web page (Issue #344)
- Updated the markdown support with external links, additional inline markup,
  and hard line breaks.
- Links in markdown text no longer render with a leading space as part of the
  link (Issue #346)
- Fixed a buffer underflow bug discovered by AddressSanitizer.
- Fixed a bug in UTF-8 support (Issue #348)
- PDF output now includes the base language of the input document(s)
  (Issue #350)
- Optimized the loading of font widths (Issue #354)
- Optimized PDF page resources (Issue #356)
- Optimized the base memory used for font widths (Issue #357)
- Added proper `&shy;` support (Issue #361)
- Title files can now be markdown.


v1.9.5 (2019-01-23)
-------------------

- The GUI did not support EPUB output.
- Empty markdown table cells were not rendered in PDF or PostScript output.
- The automatically-generated title page now supports both "docnumber" and
  "version" metadata.
- Added support for dc:subject and dc:language metadata in EPUB output from the
  HTML keywords and lang values.
- Added support for the subject and language metadata in markdown input.
- Fixed a buffer underflow bug (Issue #338)
- `htmldoc --help` now reports whether HTTPS URLs are supported (Issue #339)
- Fixed an issue with HTML title pages and EPUB output.


v1.9.4 (2018-08-31)
-------------------

- Inline fixed-width text is no longer reduced in size automatically
  (Issue #309)
- Optimized initialization of font width data (Issue #334)


v1.9.3 (2018-04-10)
-------------------

- Fixed formatting bugs with aligned images (Issue #322, Issue #324)
- Fixed support for three digit "#RGB" color values (Issue #323)
- Fixed character set support for markdown metadata.
- Updated libpng to v1.6.34 (Issue #326)
- The makefiles did not use the CPPFLAGS value (Issue #328)


v1.9.2 (2018-02-03)
-------------------

- Added Markdown table support.
- Fixed parsing of TBODY, TFOOT, and THEAD elements in HTML files.


v1.9.1 (2017-10-29)
-------------------

- Fixed monospace font size issue (Issue #309)
- Added support for reproducible builds (Issue #310)
- Added limited support for the HTML 4.0 SPAN element (Issue #311)
- Added (extremely limited) UTF-8 support for input files (Issue #314)
- Fixed buffer underflow for (invalid) short HTML comments (Issue #316)
- Now indent PRE text, by popular request.
- EPUB output now makes sure that `<element property>` is written as
  `<element property="property">`.
- Now support both NAME and ID for table-of-contents targets.


v1.9.0 (2017-07-04)
-------------------

- Added support for repeating a single header row for tables that span multiple
  pages (Issue #16)
- Added support for embedding the current filename/URL in the header or footer
  (Issue #50)
- Added EPUB support (Issue #301)
- Added Markdown support (Issue #302)
- Fixed a regression in header/footer image scaling (Issue #303)
- Documentation updates (Issue #305)
- Compiler fixes (Issue #304, Issue #306)
- Fixed a bug when running HTMLDOC as a macOS application.
- Updated the bundled libpng to v1.6.29.
