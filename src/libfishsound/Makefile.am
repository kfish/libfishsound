## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -pedantic

INCLUDES = $(INCLTDL) -I$(top_srcdir)/include $(VORBIS_CFLAGS) $(SPEEX_CFLAGS)

EXTRA_DIST = Version_script.in

# Libraries to build
lib_LTLIBRARIES = libfishsound.la

noinst_HEADERS = \
	private.h \
	fs_compat.h \
	fs_vector.h

libfishsound_la_SOURCES = \
	fishsound.c \
	decode.c \
	encode.c \
	comments.c \
	speex.c \
	vorbis.c \
	fs_vector.c

libfishsound_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libfishsound_la_LIBADD = $(VORBIS_LIBS) $(SPEEX_LIBS)
