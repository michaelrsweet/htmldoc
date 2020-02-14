How to Install HTMLDOC from Source
==================================

To compile HTMLDOC you'll need C and C++ compilers (gcc is fine, most vendor
compilers work, too).  The JPEG, PNG, and ZLIB libraries are provided with
HTMLDOC.

For the GUI support you'll need FLTK 1.1.x or 1.3.x.  FLTK is a LGPL'd cross-
platform GUI toolkit and can be downloaded from:

    http://www.fltk.org/

For HTTPS support you'll need GNU TLS on Linux and UNIX.


Windows
-------

A Visual Studio solution is included in the "vcnet" directory.  You must add the
FLTK include and library directories separately for the solution to build.

We highly recommend building and installing the HTMLDOC MSI target, as it takes
care of registering the installation location with Windows.  If you want to
install the software by hand, create a directory for the software and copy the
HTMLDOC executable, the "fonts" directory, the "data" directory, and the "doc"
directory to it so that it looks like this:

    C:\Install\Dir\
        htmldoc.exe
        data\
            ... data files ...
        doc\
            ... doc files ...
        fonts\
            ... fonts files ...

Then create the following registry entries with REGEDIT:

    HKEY_LOCAL_MACHINE\Software\HTMLDOC\doc = C:\install\dir\doc
    HKEY_LOCAL_MACHINE\Software\HTMLDOC\data = C:\install\dir


Linux, macOS, and Other UNIX Platforms
--------------------------------------

To compile the software under UNIX you first need to run the "configure" script
in the source directory.  Usually this is just:

    ./configure

Then run "make" to build the software and generate the documentation:

    make

Finally, run "make install" (typically as root) to install the software:

    sudo make install


Ubuntu Notes
------------

You should install the following packages:

    sudo apt-get install build-essential autoconf libfltk1.3-dev \
        libgnutls28-dev libjpeg-dev libpng-dev pkg-config zlib1g-dev


CentOS, Fedora, and RHEL Notes
------------------------------

The version of GCC bundled with older releases of these operating systems cannot
handle the version of libpng that is bundled with HTMLDOC.  Install the
following packages to avoid this and get full functionality:

    sudo yum install autoconf fltk-devel gnutls-devel libjpeg-devel \
    	libpng-devel pkgconfig zlib-devel
