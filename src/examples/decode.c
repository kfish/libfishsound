/*
   Copyright (c) 2002, 2003, Xiph.org Foundation

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
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

#include <oggz/oggz.h>
#include <fishsound/fishsound.h>

#include <sndfile.h>

#define DEBUG

static char * infilename, * outfilename;
static int begun = 0;
static FishSoundInfo fsinfo;
static SNDFILE * sndfile;

static long total_frames = 0;

static int
open_output (int samplerate, int channels)
{
  SF_INFO sfinfo;

  printf ("opening output %s, %d Hz\n", channels == 1 ? "MONO" : "STEREO",
	  samplerate);

  sfinfo.samplerate = samplerate;
  sfinfo.channels = channels;
  sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  sndfile = sf_open (outfilename, SFM_WRITE, &sfinfo);

  return 0;
}

static int
decoded (FishSound * fsound, float ** pcm, long frames, void * user_data)
{
  if (!begun) {
    fish_sound_command (fsound, FISH_SOUND_GET_INFO, &fsinfo,
			sizeof (FishSoundInfo));
    open_output (fsinfo.samplerate, fsinfo.channels);
    begun = 1;
  }

#if 0
  total_frames += frames;
  printf ("writing %ld frames, total %ld\n", frames, total_frames);
#endif

  sf_writef_float (sndfile, (float *)pcm, frames);

  return 0;
}

static int
read_packet (OGGZ * oggz, ogg_packet * op, long serialno, void * user_data)
{
  FishSound * fsound = (FishSound *)user_data;

  fish_sound_decode (fsound, op->packet, op->bytes);

  if (op->e_o_s) {
    printf ("*** eos ***\n");
    return 1;
  }

  return 0;
}

int
main (int argc, char ** argv)
{
  OGGZ * oggz;
  FishSound * fsound;
  long n;
  int i;

  if (argc < 3) {
    printf ("usage: %s infilename outfilename\n", argv[0]);
    printf ("Opens a speex or vorbis input file and decodes it to pcm.\n");
    exit (1);
  }

  infilename = argv[1];
  outfilename = argv[2];

  fsound = fish_sound_new (FISH_SOUND_DECODE, &fsinfo);

  i = 1;
  fish_sound_command (fsound, FISH_SOUND_SET_INTERLEAVE, &i, sizeof(int));

  fish_sound_set_decoded_callback (fsound, decoded, NULL);

  if ((oggz = oggz_open ((char *) infilename, OGGZ_READ)) == NULL) {
    printf ("unable to open file %s\n", infilename);
    exit (1);
  }

  oggz_set_read_callback (oggz, -1, read_packet, fsound);

  while ((n = oggz_read (oggz, 1024)) > 0);

  oggz_close (oggz);

  fish_sound_delete (fsound);
  
  sf_close (sndfile);

  exit (0);
}

