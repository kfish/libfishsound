/*
   Copyright (C) 2004 Commonwealth Scientific and Industrial Research
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

#ifndef CONFIG_H
#define CONFIG_H

/* Build decoding support */

#define FS_DECODE 1

/* Do not build encoding support */

#define FS_ENCODE 0

#ifdef __WINS__

/* We have libvorbis */

#define HAVE_VORBIS 1

/* Use floating point arithmetic when decoding on the emulator */

#define FS_FLOAT 1

#else /* ! __WINS__ */

/* We do not have libvorbis */

#define HAVE_VORBIS 0

/* Use fixed point arithmetic when decoding on the device */

#define FS_FLOAT 0

#endif /* __WINS__ */

/* We have liboggz */

#define HAVE_OGGZ 1

/* We have libspeex */

#define HAVE_SPEEX 1

/* We have libspeex 1.1 */

#define HAVE_SPEEX_1_1 1

/* We have speex_free() */

#define HAVE_SPEEX_FREE 1

/* We have speex_lib_get_mode() */

#define HAVE_SPEEX_LIB_GET_MODE 1

/* We do not have libvorbisenc */

#define HAVE_VORBISENC 0

#ifdef __WINS__

/* Disable some pointless warnings */

#pragma warning(disable : 4100) /* unreferenced formal parameter */
#pragma warning(disable : 4127) /* conditional expression is constant */
#pragma warning(disable : 4514) /* unreferenced inline function has been removed */
#pragma warning(disable : 4702) /* unreachable code */

#endif /* __WINS__ */

#endif /* ! CONFIG_H */
