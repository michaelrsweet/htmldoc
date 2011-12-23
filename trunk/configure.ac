#
# "$Id$"
#
# Configuration script for HTMLDOC, an HTML document processing program.
#
# Copyright 2011 by Michael R Sweet.
# Copyright 1997-2010 by Easy Software Products.
#
# This program is free software.  Distribution and use rights are outlined in
# the file "COPYING.txt".
#

AC_INIT(htmldoc/htmldoc.h)
AC_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/)

dnl Define the version number...
if test -d .svn; then
    SVERSION="1.9svn-r`svnversion . | awk -F: '{print $NF}' | sed -e '1,$s/[[a-zA-Z]]*//g'`"
else
    SVERSION="1.9svn"
fi
AC_SUBST(SVERSION)
AC_DEFINE_UNQUOTED(SVERSION, "$SVERSION")

dnl Get the operating system and version number...
uname=`uname`
uversion=`uname -r | sed -e '1,$s/[[^0-9]]//g'`

if test "$uname" = "IRIX64"; then
    uname="IRIX"
fi

dnl Clear the debugging options unless the user asks for them...
OPTIM="${OPTIM:=}"
AC_SUBST(OPTIM)
CFLAGS="${CFLAGS:=}"
CXXFLAGS="${CXXFLAGS:=}"
LDFLAGS="${LDFLAGS:=}"

AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging, default=no],
    [if eval "test x$enable_debug = xyes"; then
	OPTIM="-g "
    fi])

dnl Checks for programs...
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PATH_PROG(AR,ar)
AC_PATH_PROG(CHMOD,chmod)
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(FLTKCONFIG,fltk-config)
AC_PATH_PROG(MV,mv)
AC_PATH_PROG(MKDIR,mkdir)
AC_PATH_PROG(RM,rm)

dnl See if we need a .exe extension on executables...
AC_EXEEXT

dnl Figure out the correct "ar" command flags...
if test "$ac_cv_prog_ranlib" = ":"; then
    ARFLAGS="crs"
else
    ARFLAGS="cr"
fi

AC_SUBST(ARFLAGS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(strings.h, AC_DEFINE(HAVE_STRINGS_H))
AC_CHECK_HEADER(locale.h, AC_DEFINE(HAVE_LOCALE_H))

dnl Checks for string functions.
AC_CHECK_FUNCS(strcasecmp strdup strdupf strlcat strlcpy strncasecmp)
if test "$uname" = "HP-UX" -a "$uversion" = "1020"; then
    AC_MSG_WARN(Forcing snprintf emulation for HP-UX...)
else
    AC_CHECK_FUNCS(snprintf vsnprintf)
fi

dnl See if the tm structure has the tm_gmtoff member...
AC_MSG_CHECKING(for tm_gmtoff member in tm structure)
AC_TRY_COMPILE([#include <time.h>],[struct tm t;
    int o = t.tm_gmtoff;],
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_TM_GMTOFF),
    AC_MSG_RESULT(no))

dnl Check for libraries...
LDFLAGS="${LDFLAGS:=}"
AC_SUBST(LDFLAGS)

AC_CHECK_LIB(m,pow)

dnl Networking stuff...
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(gethostbyaddr, nsl)
AC_SEARCH_LIBS(getaddrinfo, nsl, AC_DEFINE(HAVE_GETADDRINFO))
AC_SEARCH_LIBS(getnameinfo, nsl, AC_DEFINE(HAVE_GETNAMEINFO))
AC_SEARCH_LIBS(hstrerror, nsl socket resolv, AC_DEFINE(HAVE_HSTRERROR))

dnl Check for largefile support...
AC_SYS_LARGEFILE

dnl Define largefile options as needed...
LARGEFILE=""
if test x$enable_largefile != xno; then
    LARGEFILE="-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"

    if test x$ac_cv_sys_large_files = x1; then
	LARGEFILE="$LARGEFILE -D_LARGE_FILES"
    fi

    if test x$ac_cv_sys_file_offset_bits = x64; then
	LARGEFILE="$LARGEFILE -D_FILE_OFFSET_BITS=64"
    fi
fi
AC_SUBST(LARGEFILE)

dnl Check for "long long" support...
AC_CACHE_CHECK(for long long int, ac_cv_c_long_long,
    [if test "$GCC" = yes; then
	ac_cv_c_long_long=yes
    else
	AC_TRY_COMPILE(,[long long int i;],
		ac_cv_c_long_long=yes,
		ac_cv_c_long_long=no)
    fi])

if test $ac_cv_c_long_long = yes; then
    AC_DEFINE(HAVE_LONG_LONG)
fi

AC_CHECK_FUNC(strtoll, AC_DEFINE(HAVE_STRTOLL))

dnl Check for OpenSSL libraries...
AC_ARG_ENABLE(ssl, [  --enable-ssl            turn on SSL/TLS support, default=yes])
AC_ARG_WITH(openssl-libs, [  --with-openssl-libs     set directory for OpenSSL library],
    LDFLAGS="-L$withval $LDFLAGS"
    DSOFLAGS="-L$withval $DSOFLAGS",)
AC_ARG_WITH(openssl-includes, [  --with-openssl-includes set directory for OpenSSL includes],
    CFLAGS="-I$withval $CFLAGS"
    CXXFLAGS="-I$withval $CXXFLAGS"
    CPPFLAGS="-I$withval $CPPFLAGS",)

SSLFLAGS=""
SSLLIBS=""

if test x$enable_ssl != xno; then
AC_CHECK_HEADER(openssl/ssl.h,
    dnl Save the current libraries so the crypto stuff isn't always
    dnl included...
    SAVELIBS="$LIBS"

    dnl Some ELF systems can't resolve all the symbols in libcrypto
    dnl if libcrypto was linked against RSAREF, and fail to link the
    dnl test program correctly, even though a correct installation
    dnl of OpenSSL exists.  So we test the linking three times in
    dnl case the RSAREF libraries are needed.

    for libcrypto in \
	"-lcrypto" \
	"-lcrypto -lrsaref" \
	"-lcrypto -lRSAglue -lrsaref"
    do
	AC_CHECK_LIB(ssl,SSL_new,
	    [SSLFLAGS="-DOPENSSL_DISABLE_OLD_DES_SUPPORT -DOPENSSL_NO_KRB5"
	     SSLLIBS="-lssl $libcrypto"
	     AC_DEFINE(HAVE_SSL)],,
	    $libcrypto)

	if test "x${SSLLIBS}" != "x"; then
	    break
	fi
    done

    LIBS="$SAVELIBS")
fi

AC_SUBST(SSLFLAGS)
AC_SUBST(SSLLIBS)

dnl Check for image libraries...
AC_ARG_ENABLE(localjpeg, [  --enable-localjpeg      use local JPEG library, default=auto],
    [if eval "test x$enable_localjpeg = xyes"; then
	ac_cv_lib_jpeg_jpeg_CreateCompress=no
    fi])

AC_CHECK_LIB(jpeg,jpeg_CreateCompress,
    JPEGDEP=""
    JPEGDIR=""
    JPEGINC=""
    LIBS="-ljpeg $LIBS",
    JPEGDEP="../jpeg/libjpeg.a"
    JPEGDIR="jpeg"
    JPEGINC="-I../jpeg")

AC_ARG_ENABLE(localzlib, [  --enable-localzlib      use local ZLIB library, default=auto],
    [if eval "test x$enable_localzlib = xyes"; then
	ac_cv_lib_z_gzgets=no
    fi])

AC_CHECK_LIB(z,gzgets,
    ZLIBDEP=""
    ZLIBDIR=""
    ZLIBINC=""
    LIBS="-lz $LIBS",
    ZLIBDEP="../zlib/libz.a"
    ZLIBDIR="zlib"
    ZLIBINC="-I../zlib")

AC_ARG_ENABLE(localpng, [  --enable-localpng       use local PNG library, default=auto],
    [if eval "test x$enable_localpng = xyes"; then
	ac_cv_lib_png_png_set_tRNS_to_alpha=no
    fi])

AC_CHECK_LIB(png,png_set_tRNS_to_alpha,
    PNGDEP=""
    PNGDIR=""
    PNGINC=""
    LIBS="-lpng $LIBS",
    PNGDEP="../png/libpng.a"
    PNGDIR="png"
    PNGINC="-I../png")

AC_SUBST(JPEGDEP)
AC_SUBST(JPEGDIR)
AC_SUBST(JPEGINC)

AC_SUBST(PNGDEP)
AC_SUBST(PNGDIR)
AC_SUBST(PNGINC)

AC_SUBST(ZLIBDEP)
AC_SUBST(ZLIBDIR)
AC_SUBST(ZLIBINC)

AC_DEFINE(HAVE_LIBJPEG)
AC_DEFINE(HAVE_LIBPNG)
AC_DEFINE(HAVE_LIBZ)

dnl Directories for config.h...
if test "$prefix" = "NONE"; then
    prefix="/"
fi

if test "$exec_prefix" = "NONE"; then
    if test "$prefix" = "/"; then
	exec_prefix="/usr"
    else
	exec_prefix="$prefix"
    fi
fi

if test "$bindir" = "\${exec_prefix}/bin"; then
    bindir="$exec_prefix/bin"
fi

if test "$datarootdir" = "\${prefix}/share"; then
    if test "$prefix" = "/"; then
	datarootdir="/usr/share"
    else
	datarootdir="$prefix/share"
    fi
fi

if test "$datadir" = "\${prefix}/share"; then
    if test "$prefix" = "/"; then
	datadir="/usr/share"
    else
	datadir="$prefix/share"
    fi
elif test "$datadir" = "\${datarootdir}"; then
    datadir="$datarootdir"
fi

AC_DEFINE_UNQUOTED(HTMLDOC_DATA, "$datadir/htmldoc")

if test "$sysconfdir" = "\${prefix}/etc"; then
    if test "$prefix" = "/"; then
	if test "$uname" = Darwin; then
	    sysconfdir="/private/etc"
	else
	    sysconfdir="/etc"
	fi
    else
	sysconfdir="$prefix/etc"
    fi
fi

AC_ARG_WITH(docdir, [  --with-docdir           set path for documentation],docdir="$withval",docdir="")

if test x$docdir = x; then
    docdir="$datadir/doc/htmldoc"
fi

AC_DEFINE_UNQUOTED(HTMLDOC_DOCDIR, "$docdir")

dnl Update compiler options...
if test -n "$GXX"; then
    dnl Set optimization flags...
    if test -z "$OPTIM"; then
        OPTIM="-Os -g"
    fi

    dnl Show all standard warnings + unused variables when compiling...
    OPTIM="-Wall -Wunused -Wno-char-subscripts $OPTIM"

    if test -d .svn; then
	dnl The following additional warnings are useful for tracking down problems...
	OPTIM="-Wshadow -Wconversion -Winline $OPTIM"
    fi

    dnl See if GCC supports -fno-rtti...
    AC_MSG_CHECKING(if GCC supports -fno-rtti)
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-rtti"
    AC_TRY_COMPILE(,,
	CXXFLAGS="$CXXFLAGS -fno-rtti"
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    CFLAGS="$OLDCFLAGS"

    dnl See if GCC supports -fno-exceptions...
    AC_MSG_CHECKING(if GCC supports -fno-exceptions)
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-exceptions"
    AC_TRY_COMPILE(,,
	CXXFLAGS="$CXXFLAGS -fno-exceptions"
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    CFLAGS="$OLDCFLAGS"

    dnl See if we are running Solaris; if so, try the -fpermissive option...
    if test "$uname" = SunOS; then
	AC_MSG_CHECKING(if GCC supports -fpermissive)

	OLDCFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -fpermissive"
	AC_TRY_COMPILE(,,
	    CXXFLAGS="$CXXFLAGS -fpermissive"
	    AC_MSG_RESULT(yes),
	    AC_MSG_RESULT(no))
	CFLAGS="$OLDCFLAGS"
    fi
else
    case $uname in
       	IRIX*)
	    if test -z "$OPTIM"; then
		if test `uname -r | sed -e '1,$s/\.//g'` -ge 62; then
		    OPTIM="-O3 -OPT:Olimit=5000"
        	else
		    OPTIM="-O2"
		fi
	    fi
	    ;;
	HP-UX*)
	    if test -z "$OPTIM"; then
		OPTIM="+O2 +DAportable"
	    fi
	    ;;
	SunOS*)
	    dnl Solaris
	    if test -z "$OPTIM"; then
		OPTIM="-xO4 -xarch=generic"
	    fi
	    ;;
	UNIX_SV*)
	    dnl NCR SVR4 MP-RAS
	    CXXFLAGS="-Hcppext=cxx $CXXFLAGS"
    esac
fi

dnl Generate the top-level Makefile and Makedefs file...
AC_OUTPUT(Makedefs htmldoc.list)

#
# End of "$Id$".
#
