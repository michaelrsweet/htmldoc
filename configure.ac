dnl
dnl Con])guration script for HTMLDOC, an HTML document processing program.
dnl
dnl Copyright © 2011-2021 by Michael R Sweet.
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


AC_INIT([HTMLDOC], [1.9.15], [https://github.com/michaelrsweet/htmldoc/issues], [htmldoc], [https://www.msweet.org/htmldoc])
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
AC_PATH_PROG(CHMOD,chmod)
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(FLTKCONFIG,fltk-config)
AC_MSG_CHECKING(for install-sh script)
INSTALL="`pwd`/install-sh"
AC_SUBST(INSTALL)
AC_MSG_RESULT(using $INSTALL)
AC_PATH_PROG(MV,mv)
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

dnl Check for random number functions...
AC_CHECK_FUNCS(random lrand48 arc4random)

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

AC_CHECK_LIB(m,pow)
AC_CHECK_FUNC(poll, AC_DEFINE(HAVE_POLL))
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(gethostbyaddr, nsl)
AC_SEARCH_LIBS(getaddrinfo, nsl, AC_DEFINE(HAVE_GETADDRINFO))
AC_SEARCH_LIBS(getnameinfo, nsl, AC_DEFINE(HAVE_GETNAMEINFO))
AC_SEARCH_LIBS(hstrerror, nsl socket resolv, AC_DEFINE(HAVE_HSTRERROR))
AC_SEARCH_LIBS(__res_init, resolv bind, AC_DEFINE(HAVE_RES_INIT), [
    AC_SEARCH_LIBS(res_9_init, resolv bind, AC_DEFINE(HAVE_RES_INIT),
    AC_SEARCH_LIBS(res_init, resolv bind, AC_DEFINE(HAVE_RES_INIT)))
])
AC_CHECK_HEADER(resolv.h, AC_DEFINE(HAVE_RESOLV_H))

dnl Check for largefile support...
AC_SYS_LARGEFILE

dnl Define largefile options as needed...
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

dnl Check for "long long" support...
AC_CACHE_CHECK([for long long int], ac_cv_c_long_long, [
    AS_IF([test "$GCC" = yes], [
	ac_cv_c_long_long=yes
    ], [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
	    long long int i;
	]])], [
	    ac_cv_c_long_long=yes
	], [
	    ac_cv_c_long_long=no
	])
    ])
])

AS_IF([test $ac_cv_c_long_long = yes], [
    AC_DEFINE(HAVE_LONG_LONG)
])

AC_CHECK_FUNC(strtoll, AC_DEFINE(HAVE_STRTOLL))

dnl Check for TLS/SSL libraries...
AC_ARG_ENABLE(ssl, AS_HELP_STRING([--disable-ssl], [turn off SSL/TLS support, default=enabled]))
AC_ARG_ENABLE(gnutls, AS_HELP_STRING([--enable-gnutls], [use GNU TLS for SSL/TLS support, default=auto]))
AC_ARG_ENABLE(cdsassl, AS_HELP_STRING([--enable-cdsassl], [use CDSA for SSL/TLS support, default=auto]))

SSLFLAGS=""
SSLLIBS=""
have_ssl=0

AS_IF([test x$enable_ssl], [
    # Look for CDSA...
    AS_IF([test $have_ssl = 0 -a "x$enable_cdsassl" != "xno"], [
	AS_IF([test $host_os_name = darwin], [
	    AC_CHECK_HEADER(Security/SecureTransport.h, [
	    	have_ssl=1
		AC_DEFINE(HAVE_SSL)
		AC_DEFINE(HAVE_CDSASSL)
		SSLLIBS="-framework Security -framework CoreFoundation"

		AC_CHECK_HEADER(Security/SecCertificate.h, AC_DEFINE(HAVE_SECCERTIFICATE_H))
		AC_CHECK_HEADER(Security/SecItem.h, AC_DEFINE(HAVE_SECITEM_H))
		AC_CHECK_HEADER(Security/SecPolicy.h, AC_DEFINE(HAVE_SECPOLICY_H))
	    ])
	])
    ])

    # Then look for GNU TLS...
    AS_IF([test $have_ssl = 0 -a "x$enable_gnutls" != "xno" -a "x$PKGCONFIG" != x], [
    	AC_PATH_TOOL(LIBGNUTLSCONFIG,libgnutls-config)
    	AC_PATH_TOOL(LIBGCRYPTCONFIG,libgcrypt-config)
	AS_IF([$PKGCONFIG --exists gnutls], [
	    have_ssl=1
	    SSLLIBS=`$PKGCONFIG --libs gnutls`
	    SSLFLAGS=`$PKGCONFIG --cflags gnutls`
	    AC_DEFINE(HAVE_SSL)
	    AC_DEFINE(HAVE_GNUTLS)
	], [test "x$LIBGNUTLSCONFIG" != x], [
	    have_ssl=1
	    SSLLIBS=`$LIBGNUTLSCONFIG --libs`
	    SSLFLAGS=`$LIBGNUTLSCONFIG --cflags`
	    AC_DEFINE(HAVE_SSL)
	    AC_DEFINE(HAVE_GNUTLS)
	])

	AS_IF([test $have_ssl = 1], [
	    SAVELIBS="$LIBS"
	    LIBS="$LIBS $SSLLIBS"
	    AC_CHECK_FUNC(gnutls_transport_set_pull_timeout_function, AC_DEFINE(HAVE_GNUTLS_TRANSPORT_SET_PULL_TIMEOUT_FUNCTION))
	    AC_CHECK_FUNC(gnutls_priority_set_direct, AC_DEFINE(HAVE_GNUTLS_PRIORITY_SET_DIRECT))
	    LIBS="$SAVELIBS"
	])
    ])
])

AC_MSG_NOTICE([Using SSLFLAGS="$SSLFLAGS", SSLLIBS="$SSLLIBS"])

AC_SUBST(SSLFLAGS)
AC_SUBST(SSLLIBS)

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
AS_IF([test "x$PKGCONFIG" != x], [
    AC_MSG_CHECKING([for libjpeg])
    AS_IF([$PKGCONFIG --exists libjpeg], [
	AC_MSG_RESULT([yes]);
	AC_DEFINE([HAVE_LIBJPEG], 1, [Have JPEG library?])
	CPPFLAGS="$CPPFLAGS $($PKGCONFIG --cflags libjpeg)"
	LIBS="$LIBS $($PKGCONFIG --libs libjpeg)"
    ], [
	AC_MSG_RESULT([no]);
    ])
])

AC_SEARCH_LIBS([gzgets], [z], [
    LIBS="$LIBS -lz"
], [
    AC_MSG_ERROR([HTMLDOC requires zlib.])
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
	CFLAGS="$CFLAGS -D_FORTIFY_SOURCE=2"
	CXXFLAGS="$CXXFLAGS -D_FORTIFY_SOURCE=2"
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
    AS_IF([test "$host_os_name" = darwin -a x$enable_debug != xyes], [
        OPTIM="$OPTIM -mmacosx-version-min=10.14 -arch x86_64 -arch arm64"
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

AC_MSG_NOTICE([Using CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([Using CXXFLAGS=$CXXFLAGS])
AC_MSG_NOTICE([Using LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([Using LIBS=$LIBS])
