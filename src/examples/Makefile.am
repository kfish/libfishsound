## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -pedantic -g

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src/libfishsound \
	$(OGGZ_CFLAGS)

FISHSOUNDDIR = ../libfishsound
FISHSOUND_LIBS = $(FISHSOUNDDIR)/libfishsound.la $(VORBIS_LIBS) $(SPEEX_LIBS)

if HAVE_OGGZ
oggz_examples = identify

if HAVE_LIBSNDFILE1
oggz_sndfile_examples = decode encode
endif

endif

# Programs to build
noinst_PROGRAMS = $(oggz_examples) $(oggz_sndfile_examples)

identify_SOURCES = identify.c
identify_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

decode_SOURCES = decode.c
decode_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

encode_SOURCES = encode.c
encode_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

