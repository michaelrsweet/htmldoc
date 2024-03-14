dnl
dnl Configuration script for HTMLDOC, an HTML document processing program.
dnl
dnl Copyright © 2011-2024 by Michael R Sweet.
dnl Copyright © 1997-2010 by Easy Software Products.
dnl
dnl This program is free software.  Distribution and use rights are outlined in
dnl the file "COPYING".
dnl

dnl ***********************************************************************
dnl
dnl Note: Using autoheader or automake on this project will break the HTMLDOC
dnl build system.  Use "autoconf -f" to regenerate the configure script if
dnl you make changes to this file.
dnl
dnl ***********************************************************************


dnl We need at least autoconf 2.69 for the AS_HELP_STRING macro...
AC_PREREQ([2.69])


AC_INIT([HTMLDOC], [1.9.19], [https://github.com/michaelrsweet/htmldoc/issues], [htmldoc], [https://www.msweet.org/htmldoc])
AC_CONFIG_HEADERS([config.h])


dnl De])ne the version number...
SVERSION="AC_PACKAGE_VERSION"
AC_SUBST(SVERSION)
AC_DEFINE_UNQUOTED(SVERSION, "$SVERSION")


dnl GNU autoheader, automake, etc. will break the build system, this macro
dnl causes the autoheader utility to fail...
AC_DEFINE([HTMLDOC_DOES_NOT_USE_AUTOHEADER_OR_AUTOMAKE])


dnl Get the operating system and version number...
dnl Get the build and host platforms and split the host_os value
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

[host_os_name=`echo $host_os | sed -e '1,$s/[0-9.]*$//g'`]
[host_os_version=`echo $host_os | sed -e '1,$s/^[^0-9.]*//g' | awk -F. '{print $1 $2}'`]
# Linux often does not yield an OS version we can use...
AS_IF([test "x$host_os_version" = x], [
    host_os_version="0"
])


dnl Clear the debugging options unless the user asks for them...
INSTALL_STRIP="-s"
AC_SUBST(INSTALL_STRIP)
OPTIM="${OPTIM:=}"
AC_SUBST(OPTIM)
CFLAGS="${CFLAGS:=}"
CPPFLAGS="${CPPFLAGS:=}"
CXXFLAGS="${CXXFLAGS:=}"
LDFLAGS="${LDFLAGS:=}"

AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [turn on debugging, default=no]), [
    AS_IF([eval "test x$enable_debug = xyes"], [
	INSTALL_STRIP=""
	OPTIM="-g "
    ])
])

AC_ARG_WITH(gui, AS_HELP_STRING([--without-gui], [do not compile the GUI version of HTMLDOC, default=yes]))


dnl Checks for programs...
AC_PROG_AWK
AC_PROG_CC(clang cc gcc)
AC_PROG_CXX(clang++ c++ g++)
AC_PROG_CPP
AC_PROG_RANLIB
AC_PATH_PROG(AR,ar)
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(FLTKCONFIG,fltk-config)
AC_MSG_CHECKING(for install-sh script)
INSTALL="`pwd`/install-sh"
AC_SUBST(INSTALL)
AC_MSG_RESULT(using $INSTALL)
AC_PATH_PROG(MKDIR,mkdir)
AC_PATH_PROG(RM,rm)
AC_PATH_TOOL(PKGCONFIG, pkg-config)

AS_IF([test x$PKGCONFIG = x], [
    AC_MSG_ERROR([pkg-config is required for library checks.])
])


dnl See whether we need a .exe extension on executables...
AC_EXEEXT


dnl Figure out the correct "ar" command flags...
AS_IF([test "$ac_cv_prog_ranlib" = ":"], [
    ARFLAGS="crs"
], [
    ARFLAGS="cr"
])

AC_SUBST(ARFLAGS)


dnl Checks for header files.
AC_CHECK_HEADER(strings.h, AC_DEFINE(HAVE_STRINGS_H))
AC_CHECK_HEADER(locale.h, AC_DEFINE(HAVE_LOCALE_H))


dnl Checks for string functions.
AC_CHECK_FUNCS(strdup strcasecmp strncasecmp strlcat strlcpy snprintf vsnprintf)


dnl See whether the tm structure has the tm_gmtoff member...
AC_MSG_CHECKING([for tm_gmtoff member in tm structure])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <time.h>]], [[
    struct tm t;
    int o = t.tm_gmtoff;
]])], [
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_TM_GMTOFF)
], [
    AC_MSG_RESULT([no])
])


dnl Check for libraries...
LDFLAGS="${LDFLAGS:=}"
AC_SUBST(LDFLAGS)


dnl Math functions...
AC_CHECK_LIB([m], [pow])


dnl Check for largefile support...
AC_SYS_LARGEFILE

LARGEFILE=""
AS_IF([test x$enable_largefile != xno], [
    LARGEFILE="-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"

    AS_IF([test x$ac_cv_sys_largefiles = x1], [
	LARGEFILE="$LARGEFILE -D_LARGE_FILES"
    ])

    AS_IF([test x$ac_cv_sys_file_offset_bits = x64], [
	LARGEFILE="$LARGEFILE -D_FILE_OFFSET_BITS=64"
    ])
])
AC_SUBST(LARGEFILE)


dnl Check for CUPS libraries...
AC_PATH_TOOL([CUPSCONFIG], [cups-config])
AC_MSG_CHECKING([for CUPS library v2.2 or higher])
AS_IF([$PKGCONFIG --exists cups], [
    CUPS_VERSION="$($PKGCONFIG --modversion cups)"
    AC_MSG_RESULT([yes, v$CUPS_VERSION])

    CPPFLAGS="$CPPFLAGS $($PKGCONFIG --cflags cups)"
    LIBS="$LIBS $($PKGCONFIG --libs cups)"
], [test "x$CUPSCONFIG" = x], [
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Sorry, this software requires libcups-dev.])
], [
    CUPS_VERSION="$($CUPSCONFIG --api-version)"
    AS_CASE(["$CUPS_VERSION"],
	[1.* | 2.0 | 2.1], [
	    AC_MSG_RESULT(no)
	    AC_MSG_ERROR([Sorry, you need CUPS 2.2.0 or higher.])
    ])

    AC_MSG_RESULT([yes, v$CUPS_VERSION])

    CPPFLAGS="$CPPFLAGS $($CUPSCONFIG --cflags)"
    LDFLAGS="$LDFLAGS $($CUPSCONFIG --ldflags)"
    LIBS="$LIBS $($CUPSCONFIG --libs)"
])


dnl Check for GUI libraries...
have_fltk=no
POST=:
AS_IF([test "x$with_gui" != xno], [
    AS_IF([test "x$FLTKCONFIG" != x], [
        have_fltk=yes
	CPPFLAGS="$CPPFLAGS $($FLTKCONFIG --use-images --cflags)"
	LIBS="$LIBS $($FLTKCONFIG --use-images --ldflags)"
	AC_DEFINE([HAVE_LIBFLTK], [1], [Do we have the FLTK library?])
	AC_DEFINE([HAVE_LIBJPEG], 1, [Have JPEG library?])
	AC_DEFINE([HAVE_LIBPNG], 1, [Have PNG library?])
	POST="$FLTKCONFIG --post"

	AC_CHECK_LIB(Xpm,XpmCreatePixmapFromData)
	AC_CHECK_LIB(X11,XCreateBitmapFromData)
    ], [
        AS_IF([test "x$with_gui" = xyes], [
	    AC_MSG_ERROR([FLTK not available.])
	], [
	    AC_MSG_WARN([FLTK not available so no GUI will be built.])
	])
    ])
])

AC_SUBST(POST)


dnl Check for image libraries...
AC_MSG_CHECKING([for libjpeg])
AS_IF([$PKGCONFIG --exists libjpeg], [
    AC_MSG_RESULT([yes]);
    AC_DEFINE([HAVE_LIBJPEG], 1, [Have JPEG library?])
    CPPFLAGS="$CPPFLAGS $($PKGCONFIG --cflags libjpeg)"
    LIBS="$LIBS $($PKGCONFIG --libs libjpeg)"
], [
    AC_MSG_RESULT([no]);
])


AC_MSG_CHECKING([for zlib])
AS_IF([$PKGCONFIG --exists zlib], [
    AC_MSG_RESULT([yes]);
    CPPFLAGS="$CPPFLAGS $($PKGCONFIG --cflags zlib)"
    LIBS="$($PKGCONFIG --libs zlib) $LIBS"
], [
    AC_MSG_RESULT([no pkg-config definition]);
    AC_CHECK_HEADER([zlib.h])
    AC_CHECK_LIB([z], [gzgets], [
	LIBS="$LIBS -lz"
    ], [
	AC_MSG_ERROR([HTMLDOC requires zlib.])
    ])
])


AC_MSG_CHECKING([for libpng-1.6.x])
AS_IF([$PKGCONFIG --exists libpng16], [
    AC_MSG_RESULT([yes]);
    AC_DEFINE([HAVE_LIBPNG], 1, [Have PNG library?])
    CPPFLAGS="$CPPFLAGS $($PKGCONFIG --cflags libpng16)"
    LIBS="$($PKGCONFIG --libs libpng16) $LIBS"
], [
    AC_MSG_RESULT([no]);
])


dnl Directories for config.h...
AS_IF([test "$prefix" = "NONE"], [
    prefix="/usr/local"
])

AS_IF([test "$exec_prefix" = "NONE"], [
    exec_prefix="$prefix"
])

AS_IF([test "$bindir" = "\${exec_prefix}/bin"], [
    bindir="$exec_prefix/bin"
])

AS_IF([test "$datarootdir" = "\${prefix}/share"], [
    AS_IF([test "$prefix" = "/"], [
	datarootdir="/usr/share"
    ], [
	datarootdir="$prefix/share"
    ])
])

AS_IF([test "$datadir" = "\${prefix}/share"], [
    AS_IF([test "$prefix" = "/"], [
	datadir="/usr/share"
    ], [
	datadir="$prefix/share"
    ])
], [test "$datadir" = "\${datarootdir}"], [
    datadir="$datarootdir"
])

AC_DEFINE_UNQUOTED(DOCUMENTATION, "$datadir/doc/htmldoc")
AC_DEFINE_UNQUOTED(HTML_DATA, "$datadir/htmldoc")


dnl Update compiler options...
AC_ARG_ENABLE(maintainer, AS_HELP_STRING([--enable-maintainer], [turn on maintainer mode, default=no]))
AC_ARG_ENABLE(sanitizer, AS_HELP_STRING([--enable-sanitizer], [build with AddressSanitizer]))
WARNINGS=""
AC_SUBST(WARNINGS)

AS_IF([test -n "$GXX"], [
    AS_IF([test x$enable_sanitizer = xyes], [
	# Use -fsanitize=address with debugging...
	OPTIM="$OPTIM -g -fsanitize=address"
    ], [
	# Otherwise use the Fortify enhancements to catch any unbounded
	# string operations...
	CFLAGS="$CFLAGS -D_FORTIFY_SOURCE=3"
	CXXFLAGS="$CXXFLAGS -D_FORTIFY_SOURCE=3"
    ])

    dnl Set optimization flags...
    AS_IF([test -z "$OPTIM"], [
        OPTIM="-Os -g"
    ])

    dnl Show all standard warnings + unused variables when compiling...
    WARNINGS="-Wall -Wunused"

    dnl Drop some not-useful/unreliable warnings...
    AC_LANG_PUSH(C++)
    for warning in char-subscripts deprecated-declarations format-truncation format-y2k switch unused-result; do
	AC_MSG_CHECKING([whether compiler supports -Wno-$warning])
	OLDCXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -Wno-$warning -Werror"
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], [
	    AC_MSG_RESULT([yes])
	    WARNINGS="$WARNINGS -Wno-$warning"
	], [
	    AC_MSG_RESULT([no])
	])
	CXXFLAGS="$OLDCXXFLAGS"
    done
    AC_LANG_POP()

    dnl Maintainer mode enables -Werror...
    AS_IF([test x$enable_maintainer = xyes], [
	WARNINGS="$WARNINGS -Werror"
    ])

    dnl See whether GCC supports -fno-rtti...
    AC_MSG_CHECKING([whether GCC supports -fno-rtti])
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-rtti"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [
        CXXFLAGS="$CXXFLAGS -fno-rtti"
	AC_MSG_RESULT([yes])
    ], [
        AC_MSG_RESULT([no])
    ])
    CFLAGS="$OLDCFLAGS"

    dnl See whether GCC supports -fno-exceptions...
    AC_MSG_CHECKING([whether GCC supports -fno-exceptions])
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-exceptions"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [
        CXXFLAGS="$CXXFLAGS -fno-exceptions"
	AC_MSG_RESULT([yes])
    ], [
        AC_MSG_RESULT([no])
    ])
    CFLAGS="$OLDCFLAGS"

    dnl OS-specific options...
    AC_MSG_CHECKING([for OS-specific compiler options])
    AS_CASE(["$host_os_name"], [linux*], [
    	# Make sure we get the full set of 64-bit Linux APIs from the headers...
	CPPFLAGS="$CPPFLAGS -D__USE_MISC -D_GNU_SOURCE"

	# Mark read-only sections as relocatable to random addresses...
	LDFLAGS="$LDFLAGS -Wl,-z,relro,-z,now"

        AC_MSG_RESULT([-D__USE_MISC -D_GNU_SOURCE -Wl,-z,relro,-z,now])
    ], [darwin*], [
        # When not building for debug, target macOS 11 or later, "universal"
        # binaries when possible...
        AS_IF([echo "$CPPFLAGS $CFLAGS $LDFLAGS $OPTIM" | grep -q "\\-arch "], [
            # Don't add architecture/min-version flags if they are already present
            AC_MSG_RESULT([none])
        ], [echo "$CPPFLAGS $CFLAGS $LDFLAGS $OPTIM" | grep -q "\\-mmacosx-version-"], [
            # Don't add architecture/min-version flags if they are already present
            AC_MSG_RESULT([none])
        ], [test "$host_os_version" -ge 200 -a x$enable_debug != xyes], [
	    # macOS 11.0 and higher support the Apple Silicon (arm64) CPUs
	    OPTIM="$OPTIM -mmacosx-version-min=11.0 -arch x86_64 -arch arm64"
            AC_MSG_RESULT([-mmacosx-version-min=11.0 -arch x86_64 -arch arm64])
	], [
	    # Don't add architecture/min-version flags if debug enabled
            AC_MSG_RESULT([none])
	])
    ], [*], [
        AC_MSG_RESULT([none])
    ])
])

AS_IF([test "x$with_gui" != xno], [
    AS_IF([test "x$FLTKCONFIG" != x], [
	    OPTIM="`$FLTKCONFIG --cflags` $OPTIM"
    ])
])


dnl Generate the top-level Makefile and Makedefs file...
AC_CONFIG_FILES([Makedefs Makefile desktop/htmldoc.plist])
AC_OUTPUT

AC_MSG_NOTICE([Using CFLAGS=$CFLAGS])
AC_MSG_NOTICE([Using CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([Using CXXFLAGS=$CXXFLAGS])
AC_MSG_NOTICE([Using LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([Using LIBS=$LIBS])
