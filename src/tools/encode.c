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
#include <time.h>

#include <oggz/oggz.h>
#include <fishsound/fishsound.h>
#include <sndfile.h>

#define DEBUG

long serialno;
int b_o_s = 1;

static int
encoded (FishSound * fsound, unsigned char * buf, long bytes, void * user_data)
{
  OGGZ * oggz = (OGGZ *)user_data;
  ogg_packet op;
  int err;

  /*printf ("encoded %ld bytes\n", bytes);*/

  op.packet = buf;
  op.bytes = bytes;
  op.b_o_s = b_o_s;
  op.e_o_s = 0;
  op.granulepos = 0; /* frameno */
  op.packetno = -1;

  err = oggz_write_feed (oggz, &op, serialno, 0, NULL);
  if (err) printf ("err: %d\n", err);

  b_o_s = 0;

  return 0;
}

int
main (int argc, char ** argv)
{
  OGGZ * oggz;
  FishSound * fsound;
  FishSoundInfo fsinfo;
  SNDFILE * sndfile;
  SF_INFO sfinfo;

  char * infilename, * outfilename;

  float pcm[2048];
  int i;
  long n;

  if (argc < 3) {
    printf ("usage: %s infile outfile\n", argv[0]);
  }

  infilename = argv[1];
  outfilename = argv[2];

  sndfile = sf_open (infilename, SFM_READ, &sfinfo);

  if ((oggz = oggz_open (outfilename, OGGZ_WRITE)) == NULL) {
    printf ("unable to open file %s\n", argv[1]);
    exit (1);
  }

  serialno = oggz_serialno_new (oggz);

  fsinfo.channels = sfinfo.channels;
  fsinfo.samplerate = sfinfo.samplerate;
  fsinfo.format = FISH_SOUND_SPEEX;

  fsound = fish_sound_new (FISH_SOUND_ENCODE, &fsinfo);
  fish_sound_set_encoded_callback (fsound, encoded, oggz);

  i = 1;
  fish_sound_command (fsound, FISH_SOUND_SET_INTERLEAVE, &i, sizeof(int));

  while (sf_readf_float (sndfile, pcm, 1024) > 0) {
    fish_sound_encode (fsound, (float **)pcm, 1024);
    while ((n = oggz_write (oggz, 1024)) > 0);
  }

  fish_sound_flush (fsound);
  while ((n = oggz_write (oggz, 1024)) > 0);

  oggz_close (oggz);

  fish_sound_delete (fsound);

  sf_close (sndfile);

  exit (0);
}
