## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -pedantic -g

INCLUDES = -I$(top_srcdir)/include $(OGGZ_CFLAGS)

FISHSOUNDDIR = ../libfishsound
FISHSOUND_LIBS = $(FISHSOUNDDIR)/libfishsound.la $(VORBIS_LIBS) $(SPEEX_LIBS)

# Programs to build
noinst_PROGRAMS = identify decode encode

identify_SOURCES = identify.c
identify_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

decode_SOURCES = decode.c
decode_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

encode_SOURCES = encode.c
encode_LDADD = $(FISHSOUND_LIBS) $(OGGZ_LIBS) $(SNDFILE_LIBS)

