/*
   Copyright (C) 2003 Commonwealth Scientific and Industrial Research
   Organisation (CSIRO) Australia

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of CSIRO Australia nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fishsound/fishsound.h>

#include "fs_tests.h"

#define DEBUG

typedef struct {
  FishSound * encoder;
  FishSound * decoder;
  float ** pcm;
} FS_EncDec;

static int
decoded (FishSound * fsound, float ** pcm, long frames, void * user_data)
{
  /* Boo! */
  return 0;
}

static int
encoded (FishSound * fsound, unsigned char * buf, long bytes, void * user_data)
{
  FS_EncDec * ed = (FS_EncDec *) user_data;
  fish_sound_decode (ed->decoder, buf, bytes);
  return 0;
}

/* Fill a PCM buffer with a squarish wave */
static void
fs_fill_square (float * pcm, int length)
{
  float value = 0.5;
  int i;

  for (i = 0; i < length; i++) {
    pcm[i] = value;
    if ((i % 100) == 0) {
      value = -value;
    }
  }
}

static FS_EncDec *
fs_encdec_new (int samplerate, int channels, int format, int interleave,
	       int blocksize)
{
  FS_EncDec * ed;
  FishSoundInfo fsinfo;
  int i;

  ed = malloc (sizeof (FS_EncDec));

  fsinfo.samplerate = samplerate;
  fsinfo.channels = channels;
  fsinfo.format = format;

  ed->encoder = fish_sound_new (FISH_SOUND_ENCODE, &fsinfo);
  ed->decoder = fish_sound_new (FISH_SOUND_DECODE, &fsinfo);

  fish_sound_set_interleave (ed->encoder, interleave);
  fish_sound_set_interleave (ed->decoder, interleave);

  fish_sound_set_encoded_callback (ed->encoder, encoded, ed);
  fish_sound_set_decoded_callback (ed->decoder, decoded, ed);

  if (interleave) {
    ed->pcm = (float **) malloc (sizeof (float) * channels * blocksize);
    fs_fill_square ((float *)ed->pcm, channels * blocksize);
  } else {
    ed->pcm = (float **) malloc (sizeof (float *) * channels);
    for (i = 0; i < channels; i++) {
      ed->pcm[i] = (float *) malloc (sizeof (float) * blocksize);
      fs_fill_square (ed->pcm[i], blocksize);
    }
  }

  return ed;
}

static int
fs_encdec_delete (FS_EncDec * ed)
{
  if (!ed) return -1;

  fish_sound_delete (ed->encoder);
  fish_sound_delete (ed->decoder);
  free (ed);

  return 0;
}

static int
fs_encdec_test (int samplerate, int channels, int format, int interleave,
		int blocksize)
{
  FS_EncDec * ed;
  int i;
  
  ed = fs_encdec_new (samplerate, channels, format, interleave, blocksize);

  for (i = 0; i < 2; i++) {
    fish_sound_encode (ed->encoder, ed->pcm, blocksize);
  }

  fs_encdec_delete (ed);

  return 0;
}

int
main (int argc, char * argv[])
{

#ifdef NASTY
  int blocksizes[6] = {128, 256, 512, 1024, 2048, 4096};
  int samplerates[4] = {8000, 16000, 32000, 48000};
  int channels[9] = {1, 2, 4, 5, 6, 8, 10, 16, 32};
#else
  int blocksizes[2] = {128, 1024};
  int samplerates[2] = {8000, 48000};
  int channels[4] = {1, 2, 6, 16};
#endif
  int interleave, b, s, c;
  char buf[128];

  INFO ("Testing encode/decode pipeline for audio");

  for (b = 0; b < sizeof (blocksizes) / sizeof (int); b++) {
    for (s = 0; s < sizeof (samplerates) / sizeof (int); s++) {
      for (c = 0; c < sizeof (channels) / sizeof (int); c++) {
	for (interleave = 0; interleave < 2; interleave++) {

	  /* Test VORBIS */
	  snprintf (buf, 128, "+ %2d channel %6d Hz Vorbis, %d frame buffer (%s)",
		    channels[c], samplerates[s], blocksizes[b],
		    interleave ? "interleave" : "non-interleave");
	  INFO (buf);
	  fs_encdec_test (samplerates[s], channels[c], FISH_SOUND_VORBIS,
			  interleave, blocksizes[b]);

	  /* Test SPEEX */
	  if (channels[c] <= 2) {
	    snprintf (buf, 128, "+ %2d channel %6d Hz Speex,  %d frame buffer (%s)",
		      channels[c], samplerates[s], blocksizes[b],
		      interleave ? "interleave" : "non-interleave");
	    INFO (buf);
	    fs_encdec_test (samplerates[s], channels[c], FISH_SOUND_SPEEX,
			    interleave, blocksizes[b]);

	  }
	}
      }
    }
  }

  exit (0);
}
