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
#include <string.h>

#include "private.h"

static int
fs_decode_update (FishSound * fsound, int interleave, FishSoundPCM pcm_type)
{
  int ret = 0;

  if (fsound->codec && fsound->codec->update)
    ret = fsound->codec->update (fsound, interleave, pcm_type);

  if (ret >= 0) {
    fsound->interleave = interleave;
    fsound->pcm_type = pcm_type;
  }

  return ret;
}

int fish_sound_set_decoded_short (FishSound * fsound,
				  FishSoundDecoded_Short decoded,
				  void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if FS_DECODE
  ret = fs_decode_update (fsound, 0, FISH_SOUND_PCM_SHORT);

  if (ret >= 0) {
    fsound->callback.decoded_short = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_short_ilv (FishSound * fsound,
				      FishSoundDecoded_ShortIlv decoded,
				      void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if FS_DECODE
  ret = fs_decode_update (fsound, 1, FISH_SOUND_PCM_SHORT);

  if (ret >= 0) {
    fsound->callback.decoded_short_ilv = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_int (FishSound * fsound,
				FishSoundDecoded_Int decoded,
				void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if FS_DECODE
  ret = fs_decode_update (fsound, 0, FISH_SOUND_PCM_INT);

  if (ret >= 0) {
    fsound->callback.decoded_int = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_int_ilv (FishSound * fsound,
				    FishSoundDecoded_IntIlv decoded,
				    void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if FS_DECODE
  ret = fs_decode_update (fsound, 1, FISH_SOUND_PCM_INT);

  if (ret >= 0) {
    fsound->callback.decoded_int_ilv = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_float (FishSound * fsound,
				  FishSoundDecoded_Float decoded,
				  void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if (FS_DECODE && FS_FLOAT)
  ret = fs_decode_update (fsound, 0, FISH_SOUND_PCM_FLOAT);

  if (ret >= 0) {
    fsound->callback.decoded_float = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_float_ilv (FishSound * fsound,
				      FishSoundDecoded_FloatIlv decoded,
				      void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if (FS_DECODE && FS_FLOAT)
  ret = fs_decode_update (fsound, 1, FISH_SOUND_PCM_FLOAT);

  if (ret >= 0) {
    fsound->callback.decoded_float_ilv = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_double (FishSound * fsound,
				   FishSoundDecoded_Double decoded,
				   void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if (FS_DECODE && FS_FLOAT)
  ret = fs_decode_update (fsound, 0, FISH_SOUND_PCM_DOUBLE);
  
  if (ret >= 0) {
    fsound->callback.decoded_double = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

int fish_sound_set_decoded_double_ilv (FishSound * fsound,
				       FishSoundDecoded_DoubleIlv decoded,
				       void * user_data)
{
  int ret = 0;

  if (fsound == NULL) return -1;

#if (FS_DECODE && FS_FLOAT)
  ret = fs_decode_update (fsound, 1, FISH_SOUND_PCM_DOUBLE);

  if (ret >= 0) {
    fsound->callback.decoded_double_ilv = decoded;
    fsound->user_data = user_data;
  }
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return ret;
}

long
fish_sound_decode (FishSound * fsound, unsigned char * buf, long bytes)
{
  int format;

  if (fsound == NULL) return -1;

#if FS_DECODE
  if (fsound->info.format == FISH_SOUND_UNKNOWN) {
    format = fish_sound_identify (buf, bytes);
    if (format == FISH_SOUND_UNKNOWN) return -1;

    fish_sound_set_format (fsound, format);
  }

  /*printf ("format: %s\n", fsound->codec->format->name);*/

  if (fsound->codec && fsound->codec->decode)
    return fsound->codec->decode (fsound, buf, bytes);
#else
  return FISH_SOUND_ERR_DISABLED;
#endif

  return 0;
}


/* DEPRECATED */
int fish_sound_set_decoded_callback (FishSound * fsound,
				     FishSoundDecoded_Float decoded,
				     void * user_data)
{
  if (fsound == NULL) return -1;

  return fsound->interleave ?
    fish_sound_set_decoded_float_ilv (fsound, decoded, user_data) :
    fish_sound_set_decoded_float (fsound, decoded, user_data);
}
