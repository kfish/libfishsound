## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -pedantic

INCLUDES = $(INCLTDL) -I$(top_srcdir)/include

EXTRA_DIST = Version_script.in

# Libraries to build
lib_LTLIBRARIES = libfishsound.la

libfishsound_la_SOURCES = \
	fishsound.c \
	private.h \
	comments.c \
	speex.c \
	vorbis.c \
	fs_vector.c fs_vector.h \
	fs_compat.h

libfishsound_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libfishsound_la_LIBADD = $(VORBIS_LIBS) $(SPEEX_LIBS)
