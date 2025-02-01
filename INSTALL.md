Building and Installing HTMLDOC
===============================

This file describes how to compile and install HTMLDOC from source code.  For
more information on HTMLDOC see the file called `README.md`.


Before You Begin
----------------

To compile HTMLDOC you'll need C and C++ compilers (clang and gcc are fine)
along with the following libraries:

- FLTK 1.3.x+ for GUI support
- CUPS 2.2+ for HTTP/HTTPS support
- libjpeg 7+ or libjpeg-turbo for JPEG support
- libpng 1.6+ or higher for PNG support
- zlib 1.1+ or higher


Getting Prerequisites
---------------------

CentOS 8/Fedora 23+/RHEL 8:

    sudo dnf groupinstall 'Development Tools'
    sudo dnf install cups-devel libjpeg-turbo-devel libpng-devel zlib-devel

Debian/Raspbian/Ubuntu:

    sudo apt-get install build-essential libcups2-dev libjpeg-dev libpng-dev \
        zlib1g-dev

macOS:

- Install Xcode from the AppStore
- Install brew if necessary from <https://brew.sh>
- Run the following commands to install the required image libraries:

    brew install libjpeg
    brew install libpng

Windows:

- Install Visual Studio 2019+
- Install Advanced Installer for making MSI installer files


Building on Linux, macOS, and Other Unix Platforms
--------------------------------------------------

To compile the software you first need to run the "configure" script in the
source directory.  Usually this is just:

    ./configure

Then run "make" to build the software and generate the documentation:

    make

Finally, run "make install" (typically as root) to install the software:

    sudo make install


Building on Windows
-------------------

Visual Studio + Advanced Installer projects are included in the "vcnet"
directory.  The Visual Studio project uses NuGet packages for all of the
dependent libraries, and the current version of Advanced Installer is required
to build the installer (MSI) target.

We highly recommend building and installing the HTMLDOC MSI target, as it takes
care of registering the installation location with Windows.  If you want to
install the software by hand, create a directory for the software and copy the
HTMLDOC executable, the "fonts" directory, the "data" directory, and the "doc"
directory to it so that it looks like this:

    C:\Install\Dir\
        data\
            ... data files ...
        doc\
            ... doc files ...
        fonts\
            ... fonts files ...
        ghtmldoc.exe
        htmldoc.exe

Then create the following registry entries with REGEDIT:

    HKEY_LOCAL_MACHINE\Software\HTMLDOC\doc = C:\install\dir\doc
    HKEY_LOCAL_MACHINE\Software\HTMLDOC\data = C:\install\dir
