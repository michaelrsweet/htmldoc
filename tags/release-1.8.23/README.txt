README.txt - 10/24/2002
-----------------------

INTRODUCTION

    This README file describes HTMLDOC, a HTML processing program that
    generates HTML, PostScript, and PDF files with a table of contents.

    **** HTMLDOC CURRENTLY ONLY SUPPORTS HTML 3.2 AND DOES NOT ****
    **** SUPPORT STYLE SHEETS!  PLEASE READ THE DOCUMENTATION  ****
    **** BEFORE ASKING QUESTIONS.                              ****

    Commercial support for HTMLDOC is available from Easy Software
    Products for $495 US.  Besides giving you priority bug fixes and
    guaranteed support, the commercial support fee includes access to a
    "members only" web site that provides examples, tutorials, and tips
    for using HTMLDOC to publish documents on your web site.

    HTMLDOC is copyright 1997-2002 by Easy Software Products and is
    currently available under the GNU General Public License, version
    2.  See the LEGAL STUFF section below for details.

    Documentation for HTMLDOC is available in the "doc" subdirectory
    and is been generated from HTML "source" files into HTML,
    PostScript, and PDF using HTMLDOC.

    HTMLDOC supports most HTML 3.2 and some HTML 4.0 markups as well as
    GIF, JPEG, and PNG images.  Eventually HTMLDOC will be compliant
    with HTML 4.0 and support style sheets.


INTERNET RESOURCES

    For general discussions about HTMLDOC, subscribe to the HTMLDOC
    mailing list by sending a message to "majordomo@easysw.com" with
    the text "subscribe htmldoc".

    The HTMLDOC home page is located at:

        http://www.easysw.com/htmldoc

    The current version of HTMLDOC can be also downloaded from:

        http://www.easysw.com/software.html

    Commercial support is available from Easy Software Products; send
    requests to "htmldoc-support@easysw.com" (note: support via this
    email address is only provided to customers with a valid support
    contract!)


REQUIREMENTS

    HTMLDOC requires an average of 2MB of disk space for installation.
    Binary distributions are available for the following platforms for
    customers with a support contract:

	- AIX 4.3.3 or higher
        - Compaq Tru64 UNIX 4.0 or higher
        - Digital UNIX 4.0 or higher
	- FreeBSD 4.5 or higher
	- HP-UX 10.20 or higher
	- IRIX 5.3 or higher
	- Linux 2.0 or higher (Intel only)
	- MacOS X 10.1 or higher
	- Microsoft Windows 95
	- Microsoft Windows 98
	- Microsoft Windows Me
	- Microsoft Windows NT 4.0
	- Microsoft Windows 2000
	- Red Hat Linux 5.2 or higher (Intel only)
	- Solaris 2.5 or higher (SPARC and Intel)

    A free, unsupported binary distribution is also available for
    Microsoft Windows.

    See the file "COMPILE.txt" for instructions on compiling HTMLDOC
    from the source code.


INSTALLING HTMLDOC UNDER MICROSOFT WINDOWS

    HTMLDOC comes in a self-extracting archive.  Double-click on or run
    the "htmldoc-1.8.23-windows.exe" or "htmldoc-1.8.23-winfree.exe" files
    to start the installation wizard.

    (the -windows file is the supported version, while the -winfree file
     is the unsupported version)


INSTALLING HTMLDOC UNDER RED HAT LINUX

    Type the following command to install HTMLDOC under Red Hat Linux:

        rpm -i htmldoc-1.8.23-linux-2.0-intel.rpm ENTER

    or:

        rpm -i htmldoc-1.8.23-linux-2.2-intel.rpm ENTER

    or:

        rpm -i htmldoc-1.8.23-linux-2.4-intel.rpm ENTER


INSTALLING HTMLDOC UNDER UNIX

    Type the following commands to install HTMLDOC under UNIX:

        gunzip htmldoc-1.8.23-platform.tar.gz ENTER
	tar xf htmldoc-1.8.23-platform.tar ENTER
	./setup ENTER *or* ./htmldoc.install ENTER

    (replace "platform" with the appropriate platform name)


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
    guidelines can be found in the software users manual in the "doc"
    directory.


RUNNING THE GUI SOFTWARE

    To run the UNIX GUI type:

        htmldoc ENTER

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
    need to send this header prior to running htmldoc.  See Chapter 5
    for more information.


CURRENT LIMITATIONS

    - No support for style sheets.
    - No support for HTML forms.
    - CAPTIONs are always shown at the top of the table.
    - HTML 4.0 table elements and attributes are not supported
      (rules, THEAD, TFOOT, etc.)


CREDITS

    Many thanks to Leonard Rosenthol (leonardr@lazerware.com) for
    providing changes to support a MacOS version of HTMLDOC.

    The table VALIGN and "HALF PAGE" code was contributed by
    D. Richard Hipp (drh@acm.org).

    The RC4 encryption code is from librc4 1.1 by the folks at
    Carnegie Mellon University.

    The MD5 hash code is from L. Peter Deutsch (ghost@aladdin.com)
    at Aladdin Enterprises (creators of Ghostscript).


ENCRYPTION SUPPORT

    HTMLDOC includes code to encrypt PDF document files using
    the RC4 algorithm with up to a 128-bit key. While this
    software and code may be freely used and exported under
    current US laws, other countries may restrict your use and
    possession of this code and software.


LEGAL STUFF

    The Adobe Portable Document Format is Copyright 1993-1999 by Adobe
    Systems Incorporated. Adobe, FrameMaker, and PostScript are
    registered trademarks of Adobe Systems, Incorporated.

    The Graphics Interchange Format is the copyright and GIF is the
    service mark property of CompuServe Incorporated.

    Digital is a registered trademark of Compaq Computer Corporation.

    Intel is a registered trademark of Intel Corporation.

    IRIX and sgi are registered trademarks of sgi.

    Linux is a registered trademark of Linus Torvalds.

    Microsoft, Windows, Windows 95, Windows 98, and Windows NT are
    registered trademarks of Microsoft Corporation.

    Solaris is a registered trademark of Sun Microsystems, Inc.

    SPARC is a registered trademark of SPARC International, Inc.

    UNIX is a registered trademark of the X/Open Company, Ltd.

    HTMLDOC is copyright 1997-2002 by Easy Software Products. This
    program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    A copy of the GNU General Public License is included in the file
    "COPYING.txt" and in Appendix A of the Software Users Manual. If
    the file or appendix is missing from your copy of HTMLDOC, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    This software is based in part on the work of the Independent JPEG
    Group.
