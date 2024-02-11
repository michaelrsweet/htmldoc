How to Install HTMLDOC from Source
==================================

To compile HTMLDOC you'll need C and C++ compilers (clang and gcc are fine)
along with the following libraries:

- FLTK 1.1.x or higher for GUI support
- CUPS 2.2 or higher for HTTP/HTTPS support
- libjpeg (7 or higher) or libjpeg-turbo for JPEG support
- libpng 1.6 or higher for PNG support
- zlib 1.1 or higher


Windows
-------

Visual Studio + Advanced Installer solutions are included in the "vcnet"
directory.  The Visual Studio solution uses NuGet packages for all of the
dependent libraries, and the current version of Advanced Installer is required
to build the installer (MSI) target.

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


Ubuntu and Debian Notes
-----------------------

You should install the following packages:

    sudo apt-get install build-essential autoconf libfltk1.3-dev \
        libcups2-dev libjpeg-dev libpng-dev pkg-config zlib1g-dev


CentOS, Fedora, and RHEL Notes
------------------------------

Install the following packages to get full functionality:

    sudo dnf install autoconf fltk-devel cups-devel libjpeg-devel \
    	libpng-devel pkgconfig zlib-devel
