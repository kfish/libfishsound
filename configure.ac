# Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([src/libfishsound/fishsound.c])

AC_PREREQ(2.53)

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(libfishsound, 0.6.3)
AM_CONFIG_HEADER(config.h)

SHARED_VERSION_INFO="1:0:0"
SHLIB_VERSION_ARG=""

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_C_CONST
AC_C_BIGENDIAN

# Check for doxygen
AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)
AM_CONDITIONAL(HAVE_DOXYGEN,$HAVE_DOXYGEN)
if test $HAVE_DOXYGEN = "false"; then
             AC_MSG_WARN([*** doxygen not found, docs will not be built])
fi

dnl Overall configuration success flag
fishsound_config_ok=yes

fishsound_examples=""

dnl
dnl  Configuration option for building of experimental code.
dnl
  
ac_enable_experimental=no
AC_ARG_ENABLE(experimental,
     [  --enable-experimental   enable building of experimental code ],
     [ ac_enable_experimental=yes ])

if test "x${ac_enable_experimental}" = xyes ; then
    AC_DEFINE(FS_EXPERIMENTAL, [], [Define to build experimental code])
fi

dnl
dnl  Configuration option for building of decoding support.
dnl
  
ac_enable_decode=yes
AC_ARG_ENABLE(decode,
     [  --disable-decode        disable building of decoding support ],
     [ ac_enable_decode=no ], [ ac_enable_decode=yes] )

if test "x${ac_enable_decode}" = xyes ; then
    AC_DEFINE(FS_DECODE, [1], [Build decoding support])
else
    AC_DEFINE(FS_DECODE, [0], [Do not build decoding support])
fi
AM_CONDITIONAL(FS_DECODE, [test "x${ac_enable_decode}" = "xyes"])

dnl
dnl  Configuration option for building of encoding support.
dnl
  
ac_enable_encode=yes
AC_ARG_ENABLE(encode,
     [  --disable-encode        disable building of encoding support ],
     [ ac_enable_encode=no ], [ ac_enable_encode=yes] )

if test "x${ac_enable_encode}" = xyes ; then
    AC_DEFINE(FS_ENCODE, [1], [Build encoding support])
else
    AC_DEFINE(FS_ENCODE, [0], [Do not build encoding support])
fi
AM_CONDITIONAL(FS_ENCODE, [test "x${ac_enable_encode}" = "xyes"])

dnl
dnl  Check decode/encode option sanity
dnl
if test "x${ac_enable_decode}" = xno && test "x${ac_enable_encode}" = xno ; then
  AC_MSG_ERROR([
***
*** You have attempted to explicitly disable both encoding and decoding.
***
*** If you want a fish that does nothing, try kfishd:
***
***   http://www.verge.net.au/~horms/junk/
***
*** or buy a dead one from your local pet store.
***
])
fi

dnl
dnl  Detect vorbis
dnl

HAVE_VORBIS=no
HAVE_VORBISENC=no
VORBIS_SUPPORT=no

ac_enable_vorbis=yes
AC_ARG_ENABLE(vorbis,
     [  --disable-vorbis        enable building of Vorbis codec support ],
     [ ac_enable_vorbis=no ], [ ac_enable_vorbis=yes] )

if test "x${ac_enable_vorbis}" = xyes ; then
  AC_CHECK_LIB(vorbis, vorbis_info_init, HAVE_VORBIS="maybe", , [-lm -logg])
  if test "x$HAVE_VORBIS" = xmaybe; then
    AC_CHECK_HEADER(vorbis/codec.h, HAVE_VORBIS="yes", HAVE_VORBIS="no")
  fi
  if test "x$HAVE_VORBIS" = xyes ; then
    AC_DEFINE(HAVE_VORBIS, [1], [Define to 1 if you have libvorbis])
  else
    AC_DEFINE(HAVE_VORBIS, [0], [Define to 1 if you have libvorbis])
  fi

  if test "x${ac_enable_encode}" = xyes ; then
    AC_CHECK_LIB(vorbisenc, vorbis_encode_init, HAVE_VORBISENC="maybe", ,
                 [-lm -logg -lvorbis])
    if test "x$HAVE_VORBISENC" = xmaybe ; then
      AC_CHECK_HEADER(vorbis/vorbisenc.h, HAVE_VORBISENC="yes",
                      HAVE_VORBISENC="no")
    fi
    if test "x$HAVE_VORBISENC" = xyes ; then
      AC_DEFINE(HAVE_VORBISENC, [1], [Define to 1 if you have libvorbisenc])
    else
      AC_DEFINE(HAVE_VORBISENC, [0], [Define to 1 if you have libvorbisenc])
    fi
  fi

  if test "x$HAVE_VORBIS" = "xyes" && test "x$HAVE_VORBISENC" = "xyes" ; then
    VORBIS_LIBS="-lvorbisenc -lvorbis -logg -lm"
    AC_SUBST(VORBIS_LIBS)
    VORBIS_SUPPORT="yes"
  elif test "x$HAVE_VORBIS" = xyes ; then
    VORBIS_LIBS="-lvorbis -logg -lm"
    AC_SUBST(VORBIS_LIBS)
    VORBIS_SUPPORT="decode only"
  fi
else
  AC_DEFINE(HAVE_VORBIS, [0], [Define to 1 if you have libvorbis])
  AC_DEFINE(HAVE_VORBISENC, [0], [Define to 1 if you have libvorbisenc])
  VORBIS_SUPPORT="disabled"
fi
AM_CONDITIONAL(HAVE_VORBIS, [test "x$HAVE_VORBIS" = "xyes"])
AM_CONDITIONAL(HAVE_VORBISENC, [test "x$HAVE_VORBISENC" = "xyes"])

dnl
dnl  Detect speex
dnl

HAVE_SPEEX=no
HAVE_SPEEX_1_1=no
SPEEX_SUPPORT=no

ac_enable_speex=yes
AC_ARG_ENABLE(speex,
     [  --disable-speex         enable building of Speex codec support ],
     [ ac_enable_speex=no ], [ ac_enable_speex=yes] )

if test "x${ac_enable_speex}" = xyes ; then
  AC_CHECK_LIB(speex, speex_encoder_init, HAVE_SPEEX="maybe", , [-lm])
  if test "x$HAVE_SPEEX" = xmaybe; then
    AC_CHECK_HEADER(speex.h, HAVE_SPEEX="yes", HAVE_SPEEX="no")
  fi
  if test "x$HAVE_SPEEX" = xyes ; then

    dnl Test for libspeex 1.1.x
    AC_CHECK_LIB(speex, speex_preprocess, HAVE_SPEEX_1_1="maybe", , [-lm])
    if test "x$HAVE_SPEEX_1_1" = xmaybe; then
      AC_CHECK_HEADER(speex_preprocess.h, HAVE_SPEEX_1_1="yes",
        HAVE_SPEEX_1_1="no")
    fi
    if test "x$HAVE_SPEEX_1_1" = xyes ; then
      AC_DEFINE(HAVE_SPEEX_1_1, [1], [Define to 1 if you have libspeex 1.1.x])
      SPEEX_SUPPORT="yes (1.1.x)"
    else
      SPEEX_SUPPORT="yes (1.0.x)"
    fi

    AC_DEFINE(HAVE_SPEEX, [1], [Define to 1 if you have libspeex])
    SPEEX_LIBS="-lspeex -lm"
    AC_SUBST(SPEEX_LIBS)
  else
    AC_DEFINE(HAVE_SPEEX, [0], [Define to 1 if you have libspeex])
  fi
else
  AC_DEFINE(HAVE_SPEEX, [0], [Define to 1 if you have libspeex])
  SPEEX_SUPPORT=disabled
fi
AM_CONDITIONAL(HAVE_SPEEX, [test "x$HAVE_SPEEX" = "xyes"])

dnl
dnl  Check codec disabling sanity
dnl
if test "x${ac_enable_vorbis}" = xno && test "x${ac_enable_speex}" = xno ; then
  AC_MSG_ERROR([
***
*** You have attempted to explicitly disable both Vorbis and Speex support.
***
*** If you want a fish that does nothing, try kfishd:
***
***   http://www.verge.net.au/~horms/junk/
***
*** or buy a live one and let it watch television.
***
])
fi

dnl
dnl Set overall configuration success to no if neither vorbis or speex found
dnl
if test "x$HAVE_VORBIS" = "xno" && test "x$HAVE_SPEEX" = "xno"; then
  fishsound_config_ok=no
fi

dnl
dnl Example programs
dnl

dnl
dnl  Detect oggz
dnl

PKG_CHECK_MODULES(OGGZ, oggz >= 0.5.40,
                  HAVE_OGGZ="yes", HAVE_OGGZ="no")

if test "$HAVE_OGGZ" = "yes" ; then
  AC_DEFINE(HAVE_OGGZ, [], [Define if have liboggz])
  AC_SUBST(OGGZ_LIBS)
else
  AC_CHECK_LIB(oggz, oggz_open, OGGZ_LIBS="maybe")
  if test "$OGGZ_LIBS" = "maybe" ; then  
    AC_MSG_ERROR([
***
*** liboggz-0.5.20 or greater seems to exist on your system, however
*** the pkg-config tool cannot find its build information.
***
*** Please set your PKG_CONFIG_PATH environment variable to include the
*** directory in which oggz.pc was installed. For example, oggz.pc
*** is installed in /usr/local/lib/pkgconfig by default; if that is the
*** case do:
***
***     export PKG_CONFIG_PATH="\$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
***
*** then run this ./configure again. If you configured liboggz with a
*** different --prefix option, replace /usr/local above with that prefix.
***
])
  fi
fi
AM_CONDITIONAL(HAVE_OGGZ, [test "x$HAVE_OGGZ" = "xyes"])

dnl
dnl Detect libsndfile1
dnl

PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.0,
                  HAVE_LIBSNDFILE1="yes", HAVE_LIBSNDFILE1="no")

if test "x$HAVE_LIBSNDFILE1" = xyes ; then
  AC_DEFINE(HAVE_LIBSNDFILE1, [], [Define if have libsndfile])
  AC_SUBST(SNDFILE_CFLAGS)
  AC_SUBST(SNDFILE_LIBS)
fi
AM_CONDITIONAL(HAVE_LIBSNDFILE1, [test "x$HAVE_LIBSNDFILE1" = "xyes"])


dnl
dnl  Configuration tests complete -- provide summary of results.
dnl

if test $fishsound_config_ok = no ; then

AC_MSG_RESULT([
**************************************************************

*** $PACKAGE $VERSION: Automatic configuration FAILED.

*** The file config.log has full details.

*** The following required libraries are missing or
*** misconfigured on your system:
])

if test "x$HAVE_VORBIS" != xyes ; then
    AC_MSG_RESULT(
[*** libvorbis, available from  http://www.vorbis.com/])
fi
if test "x$HAVE_VORBIS" = xmaybe ; then
    AC_MSG_RESULT(
[      Development files missing: The libvorbis and libvorbisenc library
      binaries seem to be installed, but building of a test program failed.
])
fi

if test "x$HAVE_SPEEX" != xyes ; then
    AC_MSG_RESULT(
[*** libspeex, available from  http://www.speex.org/])
fi
if test "x$HAVE_SPEEX" = xmaybe ; then
    AC_MSG_RESULT(
[      Development files missing: The libspeex library binary seems to be
      installed, but building of a test program failed.
])
fi

AC_MSG_RESULT(
[*** If you install the required libraries from source, you
*** need to inform the dynamic linker of their location. If
*** you install them in a system-wide directory such as
*** /usr/local (the default), you must ensure that
*** /usr/local/lib is listed in /etc/ld.so.conf, then run
*** ldconfig to update the dynamic linking system.
*** Alternatively, you can set your LD_LIBRARY_PATH environment
*** variable to include the library installation directory.
])


AC_MSG_RESULT(
[**************************************************************
])


AC_MSG_ERROR([
***
*** After fixing the above problems, you must run ./configure again.
***
]) 

else

AC_SUBST(SHLIB_VERSION_ARG)
AC_SUBST(SHARED_VERSION_INFO)

if test "x$HAVE_OGGZ" = xyes ; then
  fishsound_examples="fishsound-identify"

  if test "x$HAVE_LIBSNDFILE1" = xyes ; then
    if test "x${ac_enable_decode}" = xyes ; then
      fishsound_examples="$fishsound_examples fishsound-decode"
    fi
    if test "x${ac_enable_encode}" = xyes ; then
      fishsound_examples="$fishsound_examples fishsound-encode"
    fi
  else
    fishsound_examples="$fishsound_examples (fishsound-decode and fishsound-encode require libsndfile)"
  fi
else
  fishsound_examples="(none; liboggz and libsndfile required)"
fi

AC_OUTPUT([
Makefile
doc/Makefile
doc/Doxyfile
include/Makefile
include/fishsound/Makefile
src/Makefile
src/libfishsound/Version_script
src/libfishsound/Makefile
src/tests/Makefile
src/examples/Makefile
fishsound.pc
])

AC_MSG_RESULT([
------------------------------------------------------------------------
  $PACKAGE $VERSION:  Automatic configuration OK.

  General configuration:

    Experimental code: ........... ${ac_enable_experimental}
    Decoding support: ............ ${ac_enable_decode}
    Encoding support: ............ ${ac_enable_encode}

  Library configuration (./src/libfishsound):

    Vorbis support: .............. $VORBIS_SUPPORT
    Speex support: ............... $SPEEX_SUPPORT

  Example programs (./src/examples):

    $fishsound_examples

  Installation paths:

    libfishsound: ................ ${prefix}/lib
    C header files: .............. ${prefix}/include/fishsound
    Documentation: ............... ${prefix}/share/doc/$PACKAGE

  Example programs will be built but not installed.
------------------------------------------------------------------------
])

fi
