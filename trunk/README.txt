README - 11/10/1999
-------------------

INTRODUCTION

    This README file describes HTMLDOC, a HTML processing program that
    generates HTML, PostScript, and PDF files with a table of contents.

    HTMLDOC is copyright 1997-1999 by Easy Software Products and is
    currently available under the GNU General Public License, version
    2.  As such, there is NO WARRANTY, EXPRESSED OR IMPLIED.  This
    software is based in part on the work of the Independent JPEG
    Group.

    Documentation for HTMLDOC is available in the "doc" subdirectory
    and has been generated from HTML "source" files into HTML,
    PostScript, and PDF using HTMLDOC.

    READ THE DOCUMENTATION BEFORE ASKING QUESTIONS.

    HTMLDOC supports most HTML 3.2 and some HTML 4.0 markups as well as
    GIF, JPEG, and PNG images.  The ultimate goal is to make HTMLDOC
    compliant with HTML 4.0 and support style sheets to better control
    the PostScript and PDF output.


INTERNET RESOURCES

    Problem reports should be addressed to "htmldoc-support@easysw.com".  For
    general discussions about HTMLDOC, subscribe to the HTMLDOC mailing
    list by sending a message to "majordomo@easysw.com" with the text
    "subscribe htmldoc".

    The HTMLDOC home page is located at:

        http://www.easysw.com/htmldoc

    The current version of HTMLDOC can be also downloaded from:

        http://www.easysw.com/software.html


REQUIREMENTS

    To compile HTMLDOC you'll need a C++ compiler (gcc is fine, most
    vendor compilers work, too).  The JPEG, PNG, and ZLIB libraries are
    provided with HTMLDOC.

    For the GUI support you'll need FLTK 1.0 or higher (this works for
    both UNIX and Windows).  I have only successfully compiled HTMLDOC
    under Windows using the Microsoft Visual C++ compiler (Borland C++
    5.02 didn't work), so you may have trouble with other PC compilers.


COMPILING THE SOFTWARE

    To compile the software under UNIX you just need to run the
    "configure" script in this directory.  If the libraries listed
    above are in non-standard locations you can specify them using:

        CXXFLAGS="-I/some/directory"; export CXXFLAGS
	LDFLAGS="-L/some/directory"; export LDFLAGS

    for Bourne and Korn shells, and:

        setenv CXXFLAGS "-I/some/directory"
        setenv LDFLAGS "-L/some/directory"

    for CSH and TCSH.

    If you aren't using "gcc", "g++", "c++", or "CC" for your C++
    compiler, you'll also need to set the CXX environment variable:

        CXX=compiler; export CXX

    for Bourne and Korn shells, and:

        setenv CXX "compiler"

    for CSH and TCSH.

    Once you have set any necessary environment variables, run
    configure with:

        ./configure

    Then just run "make" to build the software.

    For the Windows version I have included Visual C++ workspace and
    project files under the "visualc" directory.  You will probably
    have to adjust the locations of the FLTK project file and include
    directory.


INSTALLING THE SOFTWARE

    To install the software you just need to run "make install".


READING THE DOCUMENTATION

    PLEASE READ THE DOCUMENTATION PROVIDED.  Most (if not all)
    questions are answered there.  The documentation is provided in
    several forms for your convenience and is located in the "doc"
    sub-directory.  This documentation can be accessed from within
    HTMLDOC by clicking on the "Help" button.

    Under Windows the documentation is generally located in the
    "C:\Program Files\HTMLDOC\doc" directory.

    The documentation was of course produced using HTMLDOC... 


RUNNING HTMLDOC FROM THE COMMAND-LINE

    HTMLDOC accepts a list of HTML "source" files and will generate
    HTML, PostScript, or PDF output via command-line options.  A
    summary of command-line options can be shown with the "--help"
    option:

        htmldoc --help

    HTMLDOC normally expects "structured" documents, with chapters,
    etc.  Chapters begin with a <H1> markup and continue to the end of
    the listed HTML files or the next <H1> markup, whichever comes
    first.  To convert unstructured documents such as web pages, use
    the "--webpage" option to HTMLDOC:

        htmldoc --webpage ...

    To generate a Level 2 PostScript file you might use:

        htmldoc -f outfile.ps chapter1.html ... chapterN.html

    Similarly you can generate a PDF file of the same source files
    using:

        htmldoc -f outfile.pdf chapter1.html ... chapterN.html

    Finally, to generate HTML files for viewing (with a linked table-
    of-contents) do the following:

        htmldoc -t html -d outdir chapter1.html ... chapterN.html

    or:

        htmldoc -t html -f outfile.html chapter1.html ... chapterN.html

    A complete description of all command-line options and HTML
    guidelines can be found in the user's manual in the "doc"
    directory.


RUNNING THE GUI SOFTWARE

    To run the UNIX GUI type:

        htmldoc

    To run the Windows GUI choose "HTMLDOC" from the "Start" menu.

    This version of HTMLDOC reads and writes "book" files containing a
    list of HTML source files and a set of output options.  Eventually
    the GUI will support multiple output files so you can automagically
    generate N files, each formatted differently.


RUNNING HTMLDOC FROM YOUR WEB SERVER

    HTMLDOC supports filtering of documents from stdin to stdout.  This
    can be useful for producing PDF or PostScript versions of HTML
    reports on your web server.

    The special filename "-" specifies the standard input or standard
    output.  To filter web pages to PDF use the command:
    
         htmldoc --webpage -t pdf -

    Note that this does not handle sending the "Content-Type:
    application/pdf" header, so if you do this in a CGI script you'll
    need to send this header prior to running htmldoc.  For a typical
    C program you could use:

	 FILE *fp;

	 puts("Content-Type: application/pdf\n");
	 fp = popen("htmldoc --webpage -t pdf -", "w");
	 ... write HTML to fp ...
	 pclose(fp);


CURRENT LIMITATIONS

    - No support for style sheets.
    - HTML 4.0 table markups and variables are not supported.
    - Vertical alignment is not supported in tables - all cells are
      aligned to the top of the cell.
    - Captions are not supported in tables.
    - A maximum of 5000 pages, 100 chapters, and 1000 images is
      currently enforced by the PostScript and PDF output code.  This
      is configurable in the source (see config.h) but is not
      dynamically adjusted at runtime as it should be.


CREDITS

    Many thanks to Leonard Rosenthol (leonardr@lazerware.com) for
    providing changes to support a MacOS version of HTMLDOC.


CHANGE LOG

    11/10/1999 - HTMLDOC v1.8

	- Added support for PDF 1.1 (Acrobat 2.x) and PDF 1.3 (Acrobat
	  4.0).
	- Changed HTML output to use less invasive navigation bars at
	  the top and bottom of each file.  This also means that the
	  "--barcolor" option is no longer supported!
	- Updated to use existing filenames in HTML (directory) output.
	- Now add filenames as needed to HTML links.
	- Now recognize any local PDF file as a local file link (i.e.
	  you just need "HREF=filename.pdf" and not
	  "HREF=file:filename.pdf")
	- Added optimizations to output code to further reduce PDF and
	  PostScript file size.
	- Wasn't escaping &,<, or > in HTML output
	- Wasn't preserving &nbsp;
	- Links in multi-file HTML output were off-by-one.
	- BLOCKQUOTE needed to be like CENTER and DIV.
	- Needed to use existing link name if present for headings to
	  avoid nested link name bug in Netscape and MSIE.
	- Extremely long link names could cause TOC generation to fail
	  and HTMLDOC to crash.
	- PDF output was not compatible with Ghostscript/Ghostview
	  because Ghostscript does not support inherited page resources
	  or the "Fl" abbreviation for the "FlateDecode" compression
	  filter.
	- PostScript DSC comments didn't have unique page numbers. This
	  caused Ghostview (among others) to get confused.
	- Some functions didn't handle empty text fragments.
	- Images couldn't be scaled both horizontally and vertically.
	- <LI> didn't support the VALUE attribute (but <OL> did...)
	- <TT>, <CODE>, and <SAMP> no longer reduce the font size.
	- Fixed whitespace problems before and after some markups that
	  was caused by intervening links.

    01/07/1999 - HTMLDOC v1.7

	- Added option for overriding the background color or image.
        - Added default font typeface and size options.
	- Added progress indicator for page formatting.
	- The HTMLDOC window is now resizeable.
	- The <TABLE> and <CENTER> markups didn't start a new block.
	- strcasecmp and friends are not available on all platforms.
	- Added support for MacOS (command-line only).
	- The width of table cells could be off by 1 point causing
	  unnecessary text wrapping.
	- The GUI's default center footer wasn't "blank".
	- Images could be "lost" if they reside in the current
	  directory or use an absolute path.
        - Documents without titles or headings could crash HTMLDOC.
	- The image loading code could crash due to a MSVC++ runtime
          library bug.
        - Spacing before <HR>'s wasn't consistent.
	- Buffer overflow problems causing crashes.
	- Didn't accept whitespace in variables, e.g. "<TAG NAME = VALUE>"
	- Links didn't always get propagated.
	- The Flate compressor data was not getting freed, so HTMLDOC
	  could use a lot of memory when compression was enabled.

    11/20/1998 - HTMLDOC v1.6

	- Now support JPEG compression of images.
	- Now have selectable Flate (ZIP) compression level.
	- Now only adjust top and bottom margins if headers and
	  footers are used.
	- Better HTML output support (now remember files for
	  links in multi-file output).
	- Increased maximum page count to 5000.
	- Needed to show headers on all pages in web page mode.
	- Now recognize both "in" and "inch" for measurements.
	- <BR> was not handled properly.
	- Selecting "web page" in the GUI clears the title toggle.
	- TABLE row spacing was not right...
	- <TD COLSPAN=n> now draws multi-column borders.
	- Column widths were computed wrong when COLSPAN was used.
	- Nested lists were not handled right.
	- Internal links didn't work for PDF output.
	- Block spacing should now be more consistent.
	- Image scaling was off - now only use page width so that
	  images are not warped.
        - The footer was always one line too low.
	- Couldn't double-click on input filename to edit.

    11/06/1998 - HTMLDOC v1.5

	- Added customization of headers and footers.
	- Added new "--title" image option.
	- Can now put logo image in header or footer.
	- <MARKUP ID="name"> now works for link destinations.
	- The table of contents now appears as part of the document
	  outline in PDF output.
	- Links to local PDF files are now treated as file links in PDF
	  output instead of web links.
	- You can now turn the title page on/off as desired.
	- PostScript and PDF output to stdout now works.
	- Nested tables now format properly.
	- <HR> now provides horizontal rule; to get a page break use
	  "<HR BREAK>".
        - Fixed <TABLE BORDER=0> bug.
	- Fixed GIF loader bug (caused problems on Alpha machines)
	- No longer get extra line after list items.
	- <FONT> markup nesting now works.
	- "&" by itself would cause loss of 15 characters.
	- The current directory was not tracked properly under Windows.
	- The right, top, and bottom margins were not being saved properly.
	- The htmlReadFile() function could consume too much stack space,
	  causing a program failure.
	- PostScript and PDF files were corrupt when generating a web
	  page with a title page.

    08/06/1998 - HTMLDOC v1.4

	- Now use autoconf "configure" script to build UNIX makefile.
        - Now handle relative filenames a lot better when loading images
	  and files.
	- Added "--webpage" option to support printing of plain HTML
	  files (i.e. not documents with chapters)
	- Added support for document backgrounds in PostScript and PDF
	  output
	- Added "--no-toc" and "--no-title" options to disable the
	  table-of-contents and title pages, respectively
	- PDF files now store all named links for use from a web page
	  (HREF="filename.pdf#name")
        - Converted to C++
	- Now using FLTK for the GUI under UNIX and Windows (yeah, one
	  set of code!)
	- Merged GUI and command-line versions
	- Greatly enhanced GUI now supports nearly all command-line
	  options.
	- Miscellaneous fixes to HTML parsing code
	- PDF links should now go to the right page all the time
	- Fixed DSC comments in PostScript output to conform to the
	  standard
	- Fixed dumb bug in Windows version - didn't handle HTML files
	  with only a LF at the end of each line (this is a BUG in the
	  MSVC++ runtime libraries!)
	- <PRE> inside a list didn't work
	- parse_table() and friends didn't check for a NULL parent
	  pointer.
	- Paragraph text that wasn't enclosed by P markups was
	  located on the wrong page when followed by a H1 markup.

    03/13/1998 - HTMLDOC v1.3.1

	- Fixed font encoding vector in PostScript output (minus instead
	  of hyphen for '-' character).

    03/12/1998 - HTMLDOC v1.3

        - New GUI for managing documents (Windows + X11/Motif)
	- Better table printing with support for user-specified column
	  widths and better automatic-sizing
        - PNG loading now works when grayscale output is requested
	- No image optimization was performed in PDF or Level 2 PostScript
	  files.  HTMLDOC now converts images to indexed (1,2,4,8 bits) if
	  there is an advantage (fewer bits per pixel) and no loss of color
	  would occur
	- The filenames in links were getting lost when writing indexed
	  HTML to a directory
	- The logo image filename wasn't being localized when writing
	  indexed HTML to a directory
	- Fonts, images, and links weren't supported inside a PRE tag
	- Added support for the <!DOCTYPE> markup
	- No longer assume that chars are unsigned by default
	- Invalid or missing links no longer generate bad PDF files
	- External links (http:, ftp:, etc) now work
	- Escaped characters are now decoded correctly in the table of
	  contents in PDF files
	- Image scaling is now more intelligent

    01/30/1998 - HTMLDOC v1.2

        - Now support "internal" links in a document (PDF & HTML).
        - Added "no compression" option for PDF files; this is needed for
          older PDF readers like Acroread 2.x.
        - Much better parsing of HTML; should now work very well with the
          HTML output by Netscape Composer.
        - Wasn't opening image files in "binary" mode (Windows).
        - The htmlReadString() and htmlWriteString() functions were removed
          because of portability problems to HP-UX and Windows, among others.

    01/05/1998 - HTMLDOC v1.1

        - Ordered (numbered) lists are now supported, as are the TYPE=, START=,
          and VALUE= option variables.
        - Now support coverpages for PS and PDF output with optional logo image.
        - Running headings (at the bottom of PS/PDF pages) are now tracked
          correctly.
        - Fixed parsing of lists so lists generated by Netscape Composer work
          right...
        - Fixed HTML links when generating a single HTML file.
        - The --numbered option didn't number all headings (only those in the
          table-of-contents).

    12/31/1997 - HTMLDOC v1.0

        - Initial version.
