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

#include <ctype.h>

#include "private.h"
#include "convert.h"

/*#define DEBUG*/

#if HAVE_SPEEX

#if HAVE_SPEEX_1_1
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_callbacks.h>

#ifdef SPEEX_DISABLE_GLOBAL_POINTERS
#include <speex/speex_noglobals.h>
#endif

#else /* Speex 1.0 */

#include <speex.h>
#include <speex_header.h>
#include <speex_stereo.h>
#include <speex_callbacks.h>
#endif

/* Format for the vendor string: "Encoded with Speex VERSION", where VERSION
 * is the libspeex version as read from a newly-generated Speex header.
 */
#define VENDOR_FORMAT "Encoded with Speex %s"

#define DEFAULT_ENH_ENABLED 1

#define MAX_FRAME_BYTES 2000

/* The type of audio PCM used natively by libspeex */
#if HAVE_SPEEX_1_1
typedef short FS_SpeexPCM;
#else
typedef float FS_SpeexPCM;
#endif

/* How to free memory allocated by libspeex */
#if !HAVE_SPEEX_FREE
#define speex_free fs_free
#endif

typedef struct _FishSoundSpeexEnc {
  int frame_offset; /* number of speex frames done in this packet */
  int pcm_offset;
  char cbits[MAX_FRAME_BYTES];
  int id;
} FishSoundSpeexEnc;

typedef struct _FishSoundSpeexInfo {
#ifdef SPEEX_DISABLE_GLOBAL_POINTERS
  SpeexMode * mode;
#endif
  int packetno;
  void * st;
  SpeexBits bits;
  int frame_size;
  int nframes;
  int extra_headers;
  SpeexStereoState stereo;
  int pcm_len; /* nr frames in pcm */

  union {
    short * s;
    float * f;
  } ipcm; /* interleaved pcm, working space */

  union {
    short * s;
    int * i;
    float * f;
    double * d;
  } ipcm_out; /* interleaved pcm, output */

  union {
    short * s[2];
    int * i[2];
    float * f[2];
    double * d[2];
  } pcm_out; /* Speex does max 2 channels */

  FishSoundSpeexEnc * enc;
} FishSoundSpeexInfo;

int
fish_sound_speex_identify (unsigned char * buf, long bytes)
{
  SpeexHeader * header;

  if (bytes < 8) return FISH_SOUND_UNKNOWN;

  if (!strncmp ((char *)buf, "Speex   ", 8)) {
    /* if only a short buffer was passed, do a weak identify */
    if (bytes == 8) return FISH_SOUND_SPEEX;

    /* otherwise, assume the buffer is an entire initial header and
     * feed it to speex_packet_to_header() */
    if ((header = speex_packet_to_header ((char *)buf, (int)bytes)) != NULL) {
      speex_free(header);
      return FISH_SOUND_SPEEX;
    }
  }

  return FISH_SOUND_UNKNOWN;
}

static int
fs_speex_command (FishSound * fsound, int command, void * data, int datasize)
{
  return 0;
}

#if FS_DECODE
static void *
process_header(unsigned char * buf, long bytes, int enh_enabled,
	       int * frame_size, int * rate,
	       int * nframes, int forceMode, int * channels,
	       SpeexStereoState * stereo, int * extra_headers,
	       FishSoundSpeexInfo * fss)
{
  void *st;
  SpeexMode *mode;
  SpeexHeader *header;
  int modeID;
  SpeexCallback callback;

  header = speex_packet_to_header((char*)buf, (int)bytes);
  if (!header) {
    /*info_dialog_new ("Speex error", NULL, "Speex: cannot read header");*/
    return NULL;
  }

  if (header->mode >= SPEEX_NB_MODES) {
    /*
    info_dialog_new ("Speex error", NULL,
		     "Mode number %d does not (any longer) exist in this version\n",
		     header->mode);
    */
    return NULL;
  }

  modeID = header->mode;
  if (forceMode!=-1)
    modeID = forceMode;

#if HAVE_SPEEX_LIB_GET_MODE
  mode = (SpeexMode *) speex_lib_get_mode (modeID);
#elif defined(SPEEX_DISABLE_GLOBAL_POINTERS)
  mode = (SpeexMode *)speex_mode_new (modeID);
  fss->mode = mode;
#else
  /* speex_mode_list[] is declared const in speex 1.1.x, hence the cast */
  mode = (SpeexMode *)speex_mode_list[modeID];
#endif

  if (header->speex_version_id > 1) {
    /*
    info_dialog_new ("Speex error", NULL,
		     "This file was encoded with Speex bit-stream version %d, "
		     "which I don't know how to decode\n",
		     header->speex_version_id);
    */
    return NULL;
  }

  if (mode->bitstream_version < header->mode_bitstream_version) {
    /*
    info_dialog_new ("Speex error", NULL,
		     "The file was encoded with a newer version of Speex. "
		     "You need to upgrade in order to play it.\n");
    */
    return NULL;
  }

  if (mode->bitstream_version > header->mode_bitstream_version) {
    /*
    info_dialog_new ("Speex error", NULL,
		     "The file was encoded with an older version of Speex. "
		     "You would need to downgrade the version in order to play it.\n");
    */
    return NULL;
  }

  st = speex_decoder_init(mode);
  if (!st) {
    /*
      info_dialog_new ("Speex error", NULL,
      "Decoder initialization failed.\n");
    */
    return NULL;
  }

  speex_decoder_ctl(st, SPEEX_SET_ENH, &enh_enabled);
  speex_decoder_ctl(st, SPEEX_GET_FRAME_SIZE, frame_size);

  if (!(*channels==1))
    {
      callback.callback_id = SPEEX_INBAND_STEREO;
      callback.func = speex_std_stereo_request_handler;
      callback.data = stereo;
      speex_decoder_ctl(st, SPEEX_SET_HANDLER, &callback);
    }
  if (!*rate)
    *rate = header->rate;
  /* Adjust rate if --force-* options are used */
  if (forceMode!=-1)
    {
      if (header->mode < forceMode)
	*rate <<= (forceMode - header->mode);
      if (header->mode > forceMode)
	*rate >>= (header->mode - forceMode);
    }

  speex_decoder_ctl(st, SPEEX_SET_SAMPLING_RATE, rate);

  *nframes = header->frames_per_packet;

  if (*channels == -1)
    *channels = header->nb_channels;

#ifdef DEBUG
  fprintf (stderr, "Decoding %d Hz audio using %s mode",
	   *rate, mode->modeName);

  if (*channels==1)
      fprintf (stderr, " (mono");
   else
      fprintf (stderr, " (stereo");

  if (header->vbr)
    fprintf (stderr, " (VBR)\n");
  else
    fprintf(stderr, "\n");
#endif

  *extra_headers = header->extra_headers;

  speex_free(header);

  return st;
}

static inline void
fs_speex_short_dispatch (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundDecoded_ShortIlv ds;
  FishSoundDecoded_Short dsi;

  if (fsound->interleave) {
    dsi = (FishSoundDecoded_ShortIlv)fsound->callback.decoded_short_ilv;
    dsi (fsound, (short **)fss->ipcm_out.s, fss->frame_size,
	 fsound->user_data);
  } else {
    ds = (FishSoundDecoded_Short)fsound->callback.decoded_short;
    ds (fsound, fss->pcm_out.s, fss->frame_size, fsound->user_data);
  }
}

static inline void
fs_speex_int_dispatch (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundDecoded_IntIlv di;
  FishSoundDecoded_Int dii;

  if (fsound->interleave) {
    dii = (FishSoundDecoded_IntIlv)fsound->callback.decoded_int_ilv;
    dii (fsound, (int **)fss->ipcm_out.i, fss->frame_size, fsound->user_data);
  } else {
    di = (FishSoundDecoded_Int)fsound->callback.decoded_int;
    di (fsound, fss->pcm_out.i, fss->frame_size, fsound->user_data);
  }
}

#if FS_FLOAT
static inline void
fs_speex_float_dispatch (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundDecoded_FloatIlv df;
  FishSoundDecoded_Float dfi;

  if (fsound->interleave) {
    dfi = (FishSoundDecoded_FloatIlv)fsound->callback.decoded_float_ilv;
    dfi (fsound, (float **)fss->ipcm_out.f, fss->frame_size,
	 fsound->user_data);
  } else {
    df = (FishSoundDecoded_Float)fsound->callback.decoded_float;
    df (fsound, fss->pcm_out.f, fss->frame_size, fsound->user_data);
  }
}

static inline void
fs_speex_double_dispatch (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundDecoded_DoubleIlv dd;
  FishSoundDecoded_Double ddi;

  if (fsound->interleave) {
    ddi = (FishSoundDecoded_DoubleIlv)fsound->callback.decoded_double_ilv;
    ddi (fsound, (double **)fss->ipcm_out.f, fss->frame_size,
	 fsound->user_data);
  } else {
    dd = (FishSoundDecoded_Double)fsound->callback.decoded_double;
    dd (fsound, fss->pcm_out.d, fss->frame_size, fsound->user_data);
  }
}
#endif

#if HAVE_SPEEX_1_1
static long
fs_speex_decode_short (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  int i;

  for (i = 0; i < fss->nframes; i++) {
    /* Decode frame */
    speex_decode_int (fss->st, &fss->bits, fss->ipcm.s);

    if (fsound->info.channels == 2)
      speex_decode_stereo_int (fss->ipcm.s, fss->frame_size, &fss->stereo);

    fsound->frameno += fss->frame_size;

    switch (fsound->pcm_type) {
    case FISH_SOUND_PCM_SHORT:
      _fs_convert_s_s (fss->ipcm.s, fss->ipcm_out.s,
		       fss->frame_size * fsound->info.channels);
      fs_speex_short_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_INT:
      _fs_convert_s_i (fss->ipcm.s, fss->ipcm_out.i,
		       fss->frame_size * fsound->info.channels);
      fs_speex_int_dispatch (fsound);
      break;
#if FS_FLOAT
    case FISH_SOUND_PCM_FLOAT:
      _fs_convert_s_f (fss->ipcm.s, fss->ipcm_out.f,
		       fss->frame_size * fsound->info.channels,
		       (float)(1/32767.0));
      fs_speex_float_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_DOUBLE:
      _fs_convert_s_d (fss->ipcm.s, fss->ipcm_out.d,
		       fss->frame_size * fsound->info.channels,
		       (double)(1/32767.0));
      fs_speex_double_dispatch (fsound);
      break;
#endif
    default:
      /* notreached */
      break;
    }
  }

  return 0;
}

static long
fs_speex_decode_short_dlv (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  int i, channels;

  channels = fsound->info.channels;

  for (i = 0; i < fss->nframes; i++) {
    /* Decode frame */
    speex_decode_int (fss->st, &fss->bits, fss->ipcm.s);

    speex_decode_stereo_int (fss->ipcm.s, fss->frame_size, &fss->stereo);

    fsound->frameno += fss->frame_size;

    switch (fsound->pcm_type) {
    case FISH_SOUND_PCM_SHORT:
      _fs_deinterleave_s_s ((short **)fss->ipcm.s, fss->pcm_out.s,
			    fss->frame_size, channels);
      fs_speex_short_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_INT:
      _fs_deinterleave_s_i ((short **)fss->ipcm.s, fss->pcm_out.i,
			    fss->frame_size, channels);
      fs_speex_int_dispatch (fsound);
      break;
#if FS_FLOAT
    case FISH_SOUND_PCM_FLOAT:
      _fs_deinterleave_s_f ((short **)fss->ipcm.s, fss->pcm_out.f,
			    fss->frame_size, channels, (float)(1/32767.0));
      fs_speex_float_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_DOUBLE:
      _fs_deinterleave_s_d ((short **)fss->ipcm.s, fss->pcm_out.d,
			    fss->frame_size, channels, (double)(1/32767.0));
      fs_speex_double_dispatch (fsound);
      break;
#endif
    default:
      /* notreached */
      break;
    }
  }

  return 0;
}
#endif /* HAVE_SPEEX_1_1 */

#if (FS_FLOAT && !HAVE_SPEEX_1_1)
static long
fs_speex_decode_float (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  int i;

  for (i = 0; i < fss->nframes; i++) {
    /* Decode frame */
    speex_decode (fss->st, &fss->bits, fss->ipcm.f);

    if (fsound->info.channels == 2)
      speex_decode_stereo (fss->ipcm.f, fss->frame_size, &fss->stereo);

    fsound->frameno += fss->frame_size;

    switch (fsound->pcm_type) {
    case FISH_SOUND_PCM_SHORT:
      _fs_convert_f_s (fss->ipcm.f, fss->ipcm_out.s,
		       fss->frame_size * fsound->info.channels);
      fs_speex_short_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_INT:
      _fs_convert_f_i (fss->ipcm.f, fss->ipcm_out.i,
		       fss->frame_size * fsound->info.channels);
      fs_speex_int_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_FLOAT:
      _fs_convert_f_f (fss->ipcm.f, fss->ipcm_out.f,
		       fss->frame_size * fsound->info.channels,
		       (float)(1/32767.0));
      fs_speex_float_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_DOUBLE:
      _fs_convert_f_d (fss->ipcm.f, fss->ipcm_out.d,
		       fss->frame_size * fsound->info.channels,
		       (double)(1/32767.0));
      fs_speex_double_dispatch (fsound);
      break;
    default:
      /* notreached */
      break;
    }
  }

  return 0;
}

static long
fs_speex_decode_float_dlv (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  int i, channels;

  channels = fsound->info.channels;

  for (i = 0; i < fss->nframes; i++) {
    /* Decode frame */
    speex_decode (fss->st, &fss->bits, fss->ipcm.f);

    speex_decode_stereo (fss->ipcm.f, fss->frame_size, &fss->stereo);

    fsound->frameno += fss->frame_size;

    switch (fsound->pcm_type) {
    case FISH_SOUND_PCM_SHORT:
      _fs_deinterleave_f_s ((float **)fss->ipcm.f, fss->pcm_out.s,
			    fss->frame_size, channels, (float)1.0);
      fs_speex_short_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_INT:
      _fs_deinterleave_f_i ((float **)fss->ipcm.f, fss->pcm_out.i,
			    fss->frame_size, channels, (float)32767.0);
      fs_speex_int_dispatch (fsound);
      break;
    case FISH_SOUND_PCM_FLOAT:
      _fs_deinterleave_f_f ((float **)fss->ipcm.f, fss->pcm_out.f,
			    fss->frame_size, channels, (float)(1/32767.0));
      fs_speex_float_dispatch (fsound);
    break;
    case FISH_SOUND_PCM_DOUBLE:
      _fs_deinterleave_f_d ((float **)fss->ipcm.f, fss->pcm_out.d,
			    fss->frame_size, channels, (double)(1/32767.0));
      fs_speex_double_dispatch (fsound);
      break;
    default:
      /* notreached */
      break;
    }
  }

  return 0;
}
#endif /* FS_FLOAT */

#if 0
static int
fs_speex_update (FishSound * fsound, int interleave, FishSoundPCM pcm_type)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  size_t pcm_size, pcm_out_size = 0, ilv_len;
  short * tmp;

  pcm_size = sizeof (FS_SpeexPCM);

  switch (pcm_type) {
  case FISH_SOUND_PCM_SHORT:
    pcm_out_size = sizeof (short); break;
  case FISH_SOUND_PCM_INT:
    pcm_out_size = sizeof (int); break;
  case FISH_SOUND_PCM_FLOAT:
    pcm_out_size = sizeof (float); break;
  case FISH_SOUND_PCM_DOUBLE:
    pcm_out_size = sizeof (double); break;
  default: /* notreached */ break;
  }

  ilv_len = fss->frame_size * fsound->info.channels;
  tmp = (short *) fs_realloc (fss->ipcm.s, pcm_size * ilv_len);

  if (tmp == NULL) {
    return FISH_SOUND_ERR_OUT_OF_MEMORY;
  } else {
    fss->ipcm.s = tmp;
  }

  if (interleave) {
    /* first remove any previous ilv shortcut pointers, don't free etc. */
    if (fss->ipcm_out.s == fss->ipcm.s) fss->ipcm_out.s = NULL;

    /* set ipcm_out buffers accordingly */
    if (HAVE_SPEEX_1_1 && pcm_type == FISH_SOUND_PCM_SHORT) {
      /* if ipcm_out was previously malloc'd, free it */
      if (fss->ipcm_out.s) fs_free (fss->ipcm_out.s);

      /* set a shortcut pointer */
      fss->ipcm_out.s = fss->ipcm.s;
    } else if (!HAVE_SPEEX_1_1 && pcm_type == FISH_SOUND_PCM_FLOAT) {
      /* if ipcm_out was previously malloc'd, free it */
      if (fss->ipcm_out.s) fs_free (fss->ipcm_out.s);

      /* set a shortcut pointer */
      fss->ipcm_out.f = fss->ipcm.f;
    } else {
      /* realloc an existing, or malloc a NULL, ipcm_out buffer */
      fss->ipcm_out.s =
	(short *) fs_realloc (fss->ipcm_out.s, pcm_out_size * ilv_len);
    }

    /* if transitioning from non-interleave to interleave,
       free non-ilv buffers */
    if (!fsound->interleave && fsound->info.channels == 2) {
      if (fss->pcm.s[0]) fs_free (fss->pcm.s[0]);
      if (fss->pcm.s[1]) fs_free (fss->pcm.s[1]);
      fss->pcm.s[0] = NULL;
      fss->pcm.s[1] = NULL;
    }
  } else {
    /* first remove any previous shortcut pointers, don't free etc. */
    if (fss->pcm.s[0] == fss->ipcm.s) fss->pcm.s[0] = NULL;

    /* if transitioning from interleave to non-interleave,
       free ilv buffers */
    if (fsound->interleave) {
      if (fss->ipcm_out.s != fss->ipcm.s) fs_free (fss->ipcm_out.s);
      fss->ipcm_out.s = NULL;
    }

    if (fsound->info.channels == 1) {
      if (HAVE_SPEEX_1_1 && pcm_type == FISH_SOUND_PCM_SHORT) {
	/* if pcm.s[0] was previously malloc'd, free it */
	if (fss->pcm.s[0]) fs_free (fss->pcm.s[0]);

	/* set a shortcut pointer */
	fss->pcm.s[0] = (short *) fss->ipcm.s;
      } else if (!HAVE_SPEEX_1_1 && pcm_type == FISH_SOUND_PCM_FLOAT) {
	/* if pcm.s[0] was previously malloc'd, free it */
	if (fss->pcm.f[0]) fs_free (fss->pcm.f[0]);

	/* set a shortcut pointer */
	fss->pcm.f[0] = (float *) fss->ipcm.f;
      } else {
	/* realloc an existing, or malloc if NULL, pcm buffer */
	tmp = fs_realloc (fss->pcm.s[0], pcm_out_size * fss->frame_size);
	if (tmp == NULL)
	  return FISH_SOUND_ERR_OUT_OF_MEMORY;
	else
	  fss->pcm.s[0] = tmp;
      }
    } else if (fsound->info.channels == 2) {
      /* realloc existing, or malloc if NULL, pcm buffers */
      tmp = fs_realloc (fss->pcm.s[0], pcm_out_size * fss->frame_size);
      if (tmp == NULL)
	return FISH_SOUND_ERR_OUT_OF_MEMORY;
      else
	fss->pcm.s[0] = tmp;
      
      tmp = fs_realloc (fss->pcm.s[1], pcm_out_size * fss->frame_size);
      if (tmp == NULL)
	return FISH_SOUND_ERR_OUT_OF_MEMORY;
      else
	fss->pcm.s[1] = tmp;
    }
  }

  return 0;
}
#endif

static int
fs_speex_update (FishSound * fsound, int interleave, FishSoundPCM pcm_type)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  size_t pcm_size, pcm_out_size = 0, ilv_len;
  short * tmp;

#ifdef DEBUG
  printf ("[fs_speex_update] %s %d\n", interleave ? "ilv" : "non-ilv",
	  pcm_type);
#endif

  pcm_size = sizeof (FS_SpeexPCM);

  switch (pcm_type) {
  case FISH_SOUND_PCM_SHORT:
    pcm_out_size = sizeof (short); break;
  case FISH_SOUND_PCM_INT:
    pcm_out_size = sizeof (int); break;
  case FISH_SOUND_PCM_FLOAT:
    pcm_out_size = sizeof (float); break;
  case FISH_SOUND_PCM_DOUBLE:
    pcm_out_size = sizeof (double); break;
  default: /* notreached */ break;
  }

  /* Create the working PCM space as an interleaved chunk */
  ilv_len = fss->frame_size * fsound->info.channels;
  tmp = (short *) fs_realloc (fss->ipcm.s, pcm_size * ilv_len);

  if (tmp == NULL) {
    return FISH_SOUND_ERR_OUT_OF_MEMORY;
  } else {
    fss->ipcm.s = tmp;
  }

  if (interleave) {
    /* free unused non-interleave buffers */
    if (fss->pcm_out.s[0]) fs_free (fss->pcm_out.s[0]);
    if (fss->pcm_out.s[1]) fs_free (fss->pcm_out.s[1]);
    fss->pcm_out.s[0] = NULL;
    fss->pcm_out.s[1] = NULL;

    /* realloc an existing, or malloc a NULL, ipcm_out buffer */
    tmp = (short *) fs_realloc (fss->ipcm_out.s, pcm_out_size * ilv_len);
    if (tmp == NULL)
      return FISH_SOUND_ERR_OUT_OF_MEMORY;
    else
      fss->ipcm_out.s = tmp;
  } else {
    /* free unused ilv buffers */
    if (fss->ipcm_out.s) fs_free (fss->ipcm_out.s);
    fss->ipcm_out.s = NULL;

    /* realloc existing, or malloc if NULL, buffer for channel 0 */
    tmp = fs_realloc (fss->pcm_out.s[0], pcm_out_size * fss->frame_size);
    if (tmp == NULL)
      return FISH_SOUND_ERR_OUT_OF_MEMORY;
    else
      fss->pcm_out.s[0] = tmp;
      
    if (fsound->info.channels == 1) {
      /* free unused buffer for channel 1 */
      if (fss->pcm_out.s[1]) fs_free (fss->pcm_out.s[1]);
      fss->pcm_out.s[1] = NULL;
    } else {
      /* realloc existing, or malloc if NULL, buffer for channel 1 */
      tmp = fs_realloc (fss->pcm_out.s[1], pcm_out_size * fss->frame_size);
      if (tmp == NULL)
	return FISH_SOUND_ERR_OUT_OF_MEMORY;
      else
	fss->pcm_out.s[1] = tmp;
    }
  }

  return 0;
}

static int
fs_speex_free_buffers (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;

  if (fsound->mode == FISH_SOUND_DECODE) {
    /* free working buffer */
    if (fss->ipcm.s) fs_free (fss->ipcm.s);
    fss->ipcm.s = NULL;

    /* free non-interleave buffers */
    if (fss->pcm_out.s[0]) fs_free (fss->pcm_out.s[0]);
    fss->pcm_out.s[0] = NULL;
    if (fss->pcm_out.s[1]) fs_free (fss->pcm_out.s[1]);
    fss->pcm_out.s[1] = NULL;

    /* free ilv buffers */
    if (fss->ipcm_out.s) fs_free (fss->ipcm_out.s);
    fss->ipcm_out.s = NULL;
  } else {
    fs_free (fss->ipcm.f);
  }

  return 0;
}

static long
fs_speex_decode (FishSound * fsound, unsigned char * buf, long bytes)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  int enh_enabled = DEFAULT_ENH_ENABLED;
  int rate = 0;
  int channels = -1;
  int forceMode = -1;

#if !FS_FLOAT
  if (fsound->pcm_type == FISH_SOUND_PCM_FLOAT ||
      fsound->pcm_type == FISH_SOUND_PCM_DOUBLE) {
    return FISH_SOUND_ERR_DISABLED; /* paranoid, notreached */
  }
#endif

  if (fss->packetno == 0) {
    fss->st = process_header (buf, bytes, enh_enabled,
			      &fss->frame_size, &rate,
			      &fss->nframes, forceMode, &channels,
			      &fss->stereo,
			      &fss->extra_headers, fss);

    if (fss->st == NULL) {
      /* XXX: error */
    }

#ifdef DEBUG
    printf ("speex: got %d channels, %d Hz\n", channels, rate);
#endif

    fsound->info.samplerate = rate;
    fsound->info.channels = channels;

    fs_speex_update (fsound, fsound->interleave, fsound->pcm_type);

    if (fss->nframes == 0) fss->nframes = 1;

  } else if (fss->packetno == 1) {
    /* Comments */
    fish_sound_comments_decode (fsound, buf, bytes);
  } else if (fss->packetno <= 1+fss->extra_headers) {
    /* Unknown extra headers */
  } else {
#ifdef DEBUG
    printf ("[fs_speex_decode] decode bits\n");
#endif

    speex_bits_read_from (&fss->bits, (char *)buf, (int)bytes);

#if HAVE_SPEEX_1_1
    if (fsound->interleave) {
      fs_speex_decode_short (fsound);
    } else {
      fs_speex_decode_short_dlv (fsound);
    }
#elif FS_FLOAT      
    if (fsound->interleave) {
      fs_speex_decode_float (fsound);
    } else {
      fs_speex_decode_float_dlv (fsound);
    }
#else
    return FISH_SOUND_ERR_DISABLED; /* notreached */
#endif
  }

  fss->packetno++;

  return 0;
}
#else /* !FS_DECODE */

#define fs_speex_decode NULL

#endif


#if FS_ENCODE
static FishSound *
fs_speex_enc_headers (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  SpeexMode * mode = NULL;
  int modeID;
  SpeexHeader header;
  unsigned char * buf;
  int bytes;

  /* XXX: set wb, nb, uwb modes */
  modeID = 1;

#if HAVE_SPEEX_LIB_GET_MODE
  mode = (SpeexMode *) speex_lib_get_mode (modeID);
#elif defined(SPEEX_DISABLE_GLOBAL_POINTERS)
  mode = (SpeexMode *)speex_mode_new (modeID);
  fss->mode = mode;
#else
  /* speex_mode_list[] is declared const in speex 1.1.x, hence the cast */
  mode = (SpeexMode *)speex_mode_list[modeID];
#endif

  speex_init_header (&header, fsound->info.samplerate, 1, mode);
  header.frames_per_packet = fss->nframes; /* XXX: frames per packet */
  header.vbr = 1; /* XXX: VBR */
  header.nb_channels = fsound->info.channels;

  fss->st = speex_encoder_init (mode);

  if (fsound->callback.encoded) {
    FishSoundEncoded encoded = (FishSoundEncoded)fsound->callback.encoded;
    char vendor_string[128];

    /* header */
    buf = (unsigned char *) speex_header_to_packet (&header, &bytes);
    encoded (fsound, buf, (long)bytes, fsound->user_data);
    fss->packetno++;
    fs_free (buf);

    /* comments */
    snprintf (vendor_string, 128, VENDOR_FORMAT, header.speex_version);
    fish_sound_comment_set_vendor (fsound, vendor_string);
    bytes = fish_sound_comments_encode (fsound, NULL, 0);
    buf = fs_malloc (bytes);
    bytes = fish_sound_comments_encode (fsound, buf, bytes);
    encoded (fsound, buf, (long)bytes, fsound->user_data);
    fss->packetno++;
    fs_free (buf);
  }

  speex_encoder_ctl (fss->st, SPEEX_SET_SAMPLING_RATE,
		     &fsound->info.samplerate);

  speex_encoder_ctl (fss->st, SPEEX_GET_FRAME_SIZE, &fss->frame_size);

#ifdef DEBUG
  printf ("[fs_speex_enc_headers] got frame size %d\n", fss->frame_size);
#endif

  /* XXX: blah blah blah ... set VBR etc. */

  fss->ipcm.f =
    fs_malloc (fss->frame_size * fsound->info.channels * sizeof (float));
  
  return fsound;
}

static long
fs_speex_encode_write (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse = (FishSoundSpeexEnc *)fss->enc;
  int bytes;

  bytes = speex_bits_write (&fss->bits, fse->cbits, MAX_FRAME_BYTES);
  speex_bits_reset (&fss->bits);

  if (fsound->callback.encoded) {
    FishSoundEncoded encoded = (FishSoundEncoded)fsound->callback.encoded;

    encoded (fsound, (unsigned char *)fse->cbits, (long)bytes,
	     fsound->user_data);
  }

  return bytes;
}

static long
fs_speex_encode_block (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse = (FishSoundSpeexEnc *)fss->enc;
  long nencoded = 0;

  if (fsound->info.channels == 2)
    speex_encode_stereo ((float *)fss->ipcm.f, fse->pcm_offset, &fss->bits);

  speex_encode (fss->st, fss->ipcm.f, &fss->bits);

  fse->frame_offset++;
  if (fse->frame_offset == fss->nframes) {
    fsound->frameno += fss->frame_size * fss->nframes;
    nencoded = fs_speex_encode_write (fsound);
    fse->frame_offset = 0;
  }

  fse->pcm_offset = 0;

  return nencoded;
}

static long
fs_speex_encode_f (FishSound * fsound, float * pcm[], long frames)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse = (FishSoundSpeexEnc *)fss->enc;
  long remaining = frames, len, n = 0, nencoded = 0;
  int j, start;

  if (fss->packetno == 0)
    fs_speex_enc_headers (fsound);

  while (remaining > 0) {
    len = MIN (remaining, fss->frame_size - fse->pcm_offset);

    start = fse->pcm_offset;
    fss->pcm_out.f[0] = &pcm[0][n];

    if (fsound->info.channels == 2) {
      fss->pcm_out.f[1] = &pcm[1][n];
      _fs_interleave_f_f (fss->pcm_out.f, (float **)&fss->ipcm.f[start*2],
			  len, 2, 32767.0);
    } else {
      for (j = 0; j < len; j++) {
	fss->ipcm.f[start + j] = fss->pcm_out.f[0][j] * (float)32767.0;
      }
    }

    fse->pcm_offset += len;

    if (fse->pcm_offset == fss->frame_size) {
      nencoded += fs_speex_encode_block (fsound);
    }

    remaining -= len;
    n += len;
  }

  return nencoded;
}

static long
fs_speex_encode_f_ilv (FishSound * fsound, float ** pcm, long frames)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse = (FishSoundSpeexEnc *)fss->enc;
  long remaining = frames, len, nencoded = 0;
  int j, start, end;
  int channels = fsound->info.channels;
  float * p = (float *)pcm;

  if (fss->packetno == 0)
    fs_speex_enc_headers (fsound);

  while (remaining > 0) {
    len = MIN (remaining, fss->frame_size - fse->pcm_offset);

    start = fse->pcm_offset * channels;
    end = (len + fse->pcm_offset) * channels;
    for (j = start; j < end; j++) {
      fss->ipcm.f[j] = *p++ * (float)32767.0;
    }

    fse->pcm_offset += len;

    if (fse->pcm_offset == fss->frame_size) {
      nencoded += fs_speex_encode_block (fsound);
    }

    remaining -= len;
  }

  return nencoded;
}

static long
fs_speex_flush (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse = (FishSoundSpeexEnc *)fss->enc;
  long nencoded = 0;

  if (fsound->mode != FISH_SOUND_ENCODE)
    return 0;

  if (fse->pcm_offset > 0) {
    nencoded += fs_speex_encode_block (fsound);
  }

  if (fse->frame_offset == 0) return 0;

  while (fse->frame_offset < fss->nframes) {
    speex_bits_pack (&fss->bits, 15, 5);
    fse->frame_offset++;
  }

  nencoded += fs_speex_encode_write (fsound);
  fse->frame_offset = 0;

  return nencoded;
}

#else /* !FS_ENCODE */

#define fs_speex_encode_f NULL
#define fs_speex_encode_f_ilv NULL
#define fs_speex_flush NULL

#endif

static int
fs_speex_reset (FishSound * fsound)
{
  /*FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;*/

  return 0;
}

static FishSound *
fs_speex_enc_init (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;
  FishSoundSpeexEnc * fse;

  fse = fs_malloc (sizeof (FishSoundSpeexEnc));
  if (fse == NULL) return NULL;

  fse->frame_offset = 0;
  fse->pcm_offset = 0;
  fse->id = 0;

  fss->enc = fse;

  return fsound;
}

static FishSound *
fs_speex_init (FishSound * fsound)
{
  FishSoundSpeexInfo * fss;
  SpeexStereoState stereo_init = SPEEX_STEREO_STATE_INIT;

  fss = fs_malloc (sizeof (FishSoundSpeexInfo));
  if (fss == NULL) return NULL;

  fss->packetno = 0;
  fss->st = NULL;
  fss->frame_size = 0;
  fss->nframes = 1;
  fss->pcm_len = 0;
  fss->ipcm.s = NULL;
  fss->ipcm_out.s = NULL;
  fss->pcm_out.s[0] = NULL;
  fss->pcm_out.s[1] = NULL;

  memcpy (&fss->stereo, &stereo_init, sizeof (SpeexStereoState));

  speex_bits_init (&fss->bits);

  fsound->codec_data = fss;

  if (fsound->mode == FISH_SOUND_ENCODE)
    fs_speex_enc_init (fsound);

  return fsound;
}

static FishSound *
fs_speex_delete (FishSound * fsound)
{
  FishSoundSpeexInfo * fss = (FishSoundSpeexInfo *)fsound->codec_data;

  fs_speex_free_buffers (fsound);

  if (fsound->mode == FISH_SOUND_DECODE) {
    if (fss->st) speex_decoder_destroy (fss->st);
  } else if (fsound->mode == FISH_SOUND_ENCODE) {
    if (fss->st) speex_encoder_destroy (fss->st);
    if (fss->enc) fs_free (fss->enc);
  }
  speex_bits_destroy (&fss->bits);

#ifdef SPEEX_DISABLE_GLOBAL_POINTERS
  if (fss->mode) speex_mode_destroy (fss->mode);
#endif

  fs_free (fss);
  fsound->codec_data = NULL;

  return fsound;
}

FishSoundCodec *
fish_sound_speex_codec (void)
{
  FishSoundCodec * codec;

  codec = (FishSoundCodec *) fs_malloc (sizeof (FishSoundCodec));

  codec->format.format = FISH_SOUND_SPEEX;
  codec->format.name = "Speex (Xiph.Org)";
  codec->format.extension = "spx";

  codec->init = fs_speex_init;
  codec->del = fs_speex_delete;
  codec->reset = fs_speex_reset;
  codec->update = fs_speex_update;
  codec->command = fs_speex_command;
  codec->decode = fs_speex_decode;
  codec->encode_s = NULL;
  codec->encode_s_ilv = NULL;
  codec->encode_i = NULL;
  codec->encode_i_ilv = NULL;
  codec->encode_f = fs_speex_encode_f;
  codec->encode_f_ilv = fs_speex_encode_f_ilv;
  codec->encode_d = NULL;
  codec->encode_d_ilv = NULL;
  codec->flush = fs_speex_flush;

  return codec;
}

#else /* !HAVE_SPEEX */

int
fish_sound_speex_identify (unsigned char * buf, long bytes)
{
  return FISH_SOUND_UNKNOWN;
}

FishSoundCodec *
fish_sound_speex_codec (void)
{
  return NULL;
}

#endif

