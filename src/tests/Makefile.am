## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -pedantic

INCLUDES = -I$(top_srcdir)/include

FISHSOUNDDIR = ../libfishsound
FISHSOUND_LIBS = $(FISHSOUNDDIR)/libfishsound.la $(VORBIS_LIBS) $(SPEEX_LIBS)

# Test programs

test: check

if FS_ENCODE
encode_tests = comment-test
endif

if FS_DECODE
if FS_ENCODE
encdec_tests = noop encdec-comments encdec-audio
endif
endif

TESTS = $(encode_tests) $(encdec_tests)

noinst_PROGRAMS = $(TESTS)
noinst_HEADERS = fs_tests.h

noop_SOURCES = noop.c
noop_LDADD = $(FISHSOUND_LIBS)

comment_test_SOURCES = comment-test.c
comment_test_LDADD = $(FISHSOUND_LIBS)

encdec_comments_SOURCES = encdec-comments.c
encdec_comments_LDADD = $(FISHSOUND_LIBS)

encdec_audio_SOURCES = encdec-audio.c
encdec_audio_LDADD = $(FISHSOUND_LIBS)
