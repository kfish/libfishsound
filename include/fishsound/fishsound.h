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

#ifndef __FISH_SOUND_H__
#define __FISH_SOUND_H__

#include <fishsound/constants.h>

/** \mainpage
 *
 * \section intro FishSound, the sound of fish!
 *
 * This is the documentation for the FishSound C API. FishSound provides
 * a simple programming interface for decoding and encoding audio data
 * using Xiph.Org codecs (Vorbis and Speex).
 *
 * libfishsound by itself is designed to handle raw codec streams from
 * a lower level layer such as UDP datagrams.
 * When these codecs are used in files, they are commonly encapsulated in
 * <a href="http://www.xiph.org/ogg/">Ogg</a> to produce
 * <em>Ogg Vorbis</em> and <em>Speex</em> files.
 * Example C programs using
 * <a href="http://www.annodex.net/software/liboggz/">liboggz</a> to
 * read and write these files are provided in the libfishsound sources.
 *
 * For more information on the design and history of libfishsound, see the
 * \link about About \endlink section of this documentation, and the
 * <a href="http://www.annodex.net/software/libfishsound/">libfishsound</a>
 * homepage.
 *
 * \subsection contents Contents
 *
 * - \link fishsound.h fishsound.h \endlink:
 * Documentation of the FishSound API.
 *
 * - \link decode Decoding audio data \endlink:
 * How to decode audio data with FishSound, including source for a fully
 * working Ogg Vorbis and Ogg Speex decoder.
 *
 * - \link encode Encoding audio data \endlink:
 * How to encode audio data with FishSound, including source for a fully
 * working Ogg Vorbis and Ogg Speex encoder.
 *
 * - \link configuration Configuration \endlink:
 * Customizing libfishsound to only decode or encode,
 * or to disable support for a particular codec.
 *
 * - \link building Building \endlink:
 * Information related to building software that uses libfishsound.
 *
 * - \link about About \endlink:
 * Design, motivation, history and acknowledgements.
 *
 * \section Licensing 
 * 
 * libfishsound is provided under the following BSD-style open source license: 
 *
 * \include COPYING 
 *
 */

/** \defgroup about About
 *
 * \section design Design
 * libfishsound provides a simple programming interface for decoding and
 * encoding audio data using codecs from
 * <a href="http://www.xiph.org/">Xiph.Org</a>.
 *
 * libfishsound by itself is designed to handle raw codec streams from
 * a lower level layer such as UDP datagrams.
 * When these codecs are used in files, they are commonly encapsulated in
 * <a href="http://www.xiph.org/ogg/">Ogg</a> to produce
 * <em>Ogg Vorbis</em> and <em>Speex</em> files.
 * Example C programs using
 * <a href="http://www.annodex.net/software/liboggz/">liboggz</a> to
 * read and write these files are provided in the libfishsound sources.
 *
 * libfishsound is implemented as a wrapper around the existing codec
 * libraries and provides a consistent, higher-level programming
 * interface. The motivation for this is twofold: to simplify the task
 * of developing application software that supports these codecs,
 * and to ensure that valid codec streams are generated.
 *
 * \section history History
 * libfishsound was designed and developed by Conrad Parker on the
 * weekend of October 18-19 2003. Previously the author had implemented
 * Vorbis and Speex support in the following software:
 * - <a href="http://www.metadecks.org/software/sweep/">Sweep</a>, a
 * digital audio editor with decoding and GUI control of all encoding
 * options of Vorbis and Speex
 * - Speex support in the <a href="http://www.xinehq.org/">xine</a>
 * multimedia player
 * - Vorbis and Speex importers for
 * <a href="http://www.annodex.net/software/libannodex/">libannodex</a>,
 the basic library for reading and writing
 * <a href="http://www.annodex.net/">Annodex.net</a> media files.
 *
 * The implementation of libfishsound draws heavily on these sources, and
 * in turn the original example sources of libvorbis and libvorbisenc by
 * Monty, and libspeex by Jean-Marc Valin.
 *
 * The naming of libfishsound reflects both the Xiph.Org logo and
 * the author's reputation as a dirty, smelly old fish.
 *
 * \section limitations Limitations
 *
 * libfishsound has been designed to accomodate the various decoding and
 * encoding styles required by a wide variety of software. However, as it
 * is an abstraction of the underlying libvorbis, libvorbisenc and libspeex
 * libraries, it may not be possible to implement some low-level techniques
 * that these libraries enable, such as parallelization of Vorbis sub-block
 * decoding. Nevertheless it is expected that libfishsound is a useful
 * API for most software requiring Vorbis or Speex support, including most
 * applications the author has encountered.
 *
 * \section acknowledgements Acknowledgements
 * Much of the API design follows the style of
 * <a href="http://www.zip.com.au/~erikd/libsndfile/">libsndfile</a>.
 * The author would like to thank Erik de Castro Lopo for feedback on the
 * design of libfishsound.
 */

/** \defgroup configuration Configuration
 *
 * \section platforms Platform-specific configuration
 *
 * FishSound can be configured on most platforms using the GNU autoconf
 * ./configure system described below.
 *
 * For Win32, see the \link win32 README.win32 \endlink section. You will
 * need to edit <tt>win32/config.h</tt> by hand to achieve the customizations
 * described below.
 *
 * \section ./configure ./configure
 *
 * It is possible to customize the functionality of libfishsound
 * by using various ./configure flags when
 * building it from source; for example you can build a smaller
 * version of libfishsound to only decode or encode, or and you can
 * choose to disable support for a particular codec.
 * By default, both decoding and encoding support is built for all
 * codecs found on the system.
 *
 * For general information about using ./configure, see the file
 * \link install INSTALL \endlink
 *
 * \subsection no_encode Removing encoding support
 *
 * Configuring with \a --disable-encode will remove all support for encoding:
 * - All internal encoding related functions will not be built
 * - Any attempt to call fish_sound_new() with \a mode == FISH_SOUND_ENCODE
 *   will fail, returning NULL
 * - Any attempt to call fish_sound_encode() will return
 *   FISH_SOUND_ERR_DISABLED
 * - The resulting library will not be linked against libvorbisenc
 *
 * \subsection no_decode Removing decoding support
 *
 * Configuring with \a --disable-decode will remove all support for decoding:
 * - All internal decoding related functions will not be built
 * - Any attempt to call fish_sound_new() with \a mode == FISH_SOUND_DECODE
 *   will fail, returning NULL
 * - Any attempt to call fish_sound_decode() will return 
 *   FISH_SOUND_ERR_DISABLED
 *
 * \subsection no_vorbis Removing Vorbis support
 *
 * Configuring with \a --disable-vorbis will remove all support for Vorbis:
 * - All internal Vorbis related functions will not be built
 * - Any attempt to call fish_sound_new() with \a mode == FISH_SOUND_ENCODE
 *   and \a fsinfo->format == FISH_SOUND_VORBIS will fail, returning NULL
 * - The resulting library will not be linked against libvorbis or libvorbisenc
 *
 * \subsection no_speex Removing Speex support
 *
 * Configuring with \a --disable-speex will remove all support for Speex:
 * - All internal Speex related functions will not be built
 * - Any attempt to call fish_sound_new() with \a mode == FISH_SOUND_ENCODE
 *   and \a fsinfo->format == FISH_SOUND_SPEEX will fail, returning NULL
 * - The resulting library will not be linked against libspeex
 *
 * \subsection summary Configuration summary
 * 
 * Upon successful configuration, you should see something like this:
<pre>
------------------------------------------------------------------------
  libfishsound 0.6.0:  Automatic configuration OK.

  General configuration:

    Experimental code: ........... no
    Decoding support: ............ yes
    Encoding support: ............ yes

  Library configuration (./src/libfishsound):

    Vorbis support: .............. yes
    Speex support: ............... yes

  Example programs (./src/examples):

    identify decode encode

  Installation paths:

    libfishsound: ................ /usr/local/lib
    C header files: .............. /usr/local/include/fishsound
    Documentation: ............... /usr/local/share/doc/libfishsound

  Example programs will be built but not installed.
------------------------------------------------------------------------
</pre>
 */

/** \defgroup install Installation
 * \section install INSTALL
 *
 * \include INSTALL
 */

/** \defgroup win32 Building on Win32
 * \section win32 README.Win32
 *
 * \include README.win32
 */

/** \defgroup building Building against libfishsound
 *
 *
 * \section autoconf Using GNU autoconf
 *
 * If you are using GNU autoconf, you do not need to call pkg-config
 * directly. Use the following macro to determine if libfishsound is
 * available:
 *
 <pre>
 PKG_CHECK_MODULES(FISHSOUND, fishsound >= 0.6.0,
                   HAVE_FISHSOUND="yes", HAVE_FISHSOUND="no")
 if test "x$HAVE_FISHSOUND" = "xyes" ; then
   AC_SUBST(FISHSOUND_CFLAGS)
   AC_SUBST(FISHSOUND_LIBS)
 fi
 </pre>
 *
 * If libfishsound is found, HAVE_FISHSOUND will be set to "yes", and
 * the autoconf variables FISHSOUND_CFLAGS and FISHSOUND_LIBS will
 * be set appropriately.
 *
 * \section pkg-config Determining compiler options with pkg-config
 *
 * If you are not using GNU autoconf in your project, you can use the
 * pkg-config tool directly to determine the correct compiler options.
 *
 <pre>
 FISHSOUND_CFLAGS=`pkg-config --cflags fishsound`

 FISHSOUND_LIBS=`pkg-config --libs fishsound`
 </pre>
 *
 */

/** \file
 * The libfishsound C API.
 *
 * \section general General usage
 *
 * All access is managed via a FishSound* handle. This is instantiated
 * using fish_sound_new() and should be deleted with fish_sound_delete()
 * when no longer required. If there is a discontinuity in the input
 * data (eg. after seeking in an input file), call fish_sound_reset() to
 * reset the internal codec state.
 *
 * \section decoding Decoding
 *
 * libfishsound provides callback based decoding: you feed it encoded audio
 * data, and it will call your callback with decoded PCM. A more detailed
 * explanation and a full example of decoding Ogg Vorbis and Speex files is
 * provided in the \link decode Decoding audio data \endlink section.
 *
 * \section encoding Encoding
 *
 * libfishsound provides callback based encoding: you feed it PCM audio,
 * and it will call your callback with encoded audio data. A more detailed
 * explanation and a full example of encoding Ogg Vorbis and Speex files is
 * provided in the \link encode Encoding audio data \endlink section.
 */

/** \defgroup decode Decoding audio data
 *
 * To decode audio data using libfishsound:
 *
 * - create a FishSound* object with mode FISH_SOUND_DECODE. fish_sound_new()
 * will return a new FishSound* object, initialised for decoding, and the
 * FishSoundInfo structure will be cleared.
 * - provide a FishSoundDecoded callback for libfishsound to call when it has
 * decoded audio.
 * - (optionally) specify whether you want to receive interleaved or
 * per-channel PCM data, using a fish_sound_set_interleave().
 * The default is for per-channel (non-interleaved) PCM.
 * - feed encoded audio data to libfishsound via fish_sound_decode().
 * libfishsound will decode the audio for you, calling the FishSoundDecoded
 * callback you provided earlier each time it has a block of audio ready.
 * - when finished, call fish_sound_delete().
 *
 * This procedure is illustrated in src/examples/decode.c. Note that this
 * example additionally:
 * - uses <a href="http://www.annodex.net/software/liboggz/">liboggz</a> to
 * demultiplex audio data from an Ogg encapsulated Vorbis or Speex stream.
 * Hence, the step of feeding encoded data to libfishsound is done within
 * the OggzReadPacket callback.
 * - uses <a href="http://www.mega-nerd.com/libsndfile/">libsndfile</a> to
 * write the decoded audio to a WAV file.
 *
 * Hence this example code demonstrates all that is needed to decode both
 * Ogg Vorbis and Ogg Speex files:
 *
 * \include decode.c
 */

/** \defgroup encode Encoding audio data
 *
 * To encode audio data using libfishsound:
 *
 * - create a FishSound* object with mode FISH_SOUND_ENCODE, and with a
 * FishSoundInfo structure filled in with the required encoding parameters.
 * fish_sound_new()  will return a new FishSound* object initialised for
 * encoding.
 * - provide a FishSoundEncoded callback for libfishsound to call when it
 * has a block of encoded audio
 * - (optionally) specify whether you will be providing interleaved or
 * per-channel PCM data, using a fish_sound_set_interleave().
 * The default is for per-channel (non-interleaved) PCM.
 * - feed raw PCM audio data to libfishsound via fish_sound_encode().
 * libfishsound will encode the audio for you, calling the FishSoundEncoded
 * callback you provided earlier each time it has a block of encoded audio
 * ready.
 * - when finished, call fish_sound_delete().
 *
 * This procedure is illustrated in src/examples/encode.c. Note that this
 * example additionally:
 * - uses <a href="http://www.mega-nerd.com/libsndfile/">libsndfile</a> to
 * read input from a PCM audio file (WAV, AIFF, etc.)
 * - uses <a href="http://www.annodex.net/software/liboggz/">liboggz</a> to
 * encapsulate the encoded Vorbis or Speex data in an Ogg stream.
 *
 * Hence this example code demonstrates all that is needed to encode
 * Ogg Vorbis and Ogg Speex files:
 *
 * \include encode.c
 */

/**
 * Info about a particular encoder/decoder instance
 */
typedef struct {
  /** Sample rate of audio data in Hz */
  int samplerate;

  /** Count of channels */
  int channels;

  /** FISH_SOUND_VORBIS, FISH_SOUND_SPEEX etc. */
  int format;
} FishSoundInfo;

/**
 * Info about a particular sound format
 */
typedef struct {
  /** FISH_SOUND_VORBIS, FISH_SOUND_SPEEX etc. */
  int format;

  /** Printable name */
  const char * name;     

  /** Commonly used file extension */
  const char * extension;
} FishSoundFormat;

/**
 * An opaque handle to a FishSound. This is returned by fishsound_new()
 * and is passed to all other fish_sound_*() functions.
 */
typedef void * FishSound;

/**
 * Signature of a callback for libfishsound to call when it has decoded
 * audio PCM data.
 * \param fsound The FishSound* handle
 * \param pcm The decoded audio
 * \param frames The count of frames decoded
 * \param user_data Arbitrary user data
 * \retval 0 to continue
 * \retval non-zero to stop decoding immediately and
 * return control to the fish_sound_decode() caller
 */
typedef int (*FishSoundDecoded) (FishSound * fsound, float ** pcm,
				 long frames, void * user_data);

/**
 * Signature of a callback for libfishsound to call when it has encoded
 * data.
 * \param fsound The FishSound* handle
 * \param buf The encoded data
 * \param bytes The count of bytes encoded
 * \param user_data Arbitrary user data
 * \retval 0 to continue
 * \retval non-zero to stop encoding immediately and
 * return control to the fish_sound_encode() caller
 */
typedef int (*FishSoundEncoded) (FishSound * fsound, unsigned char * buf,
				 long bytes, void * user_data);

/**
 * Identify a codec based on the first few bytes of data.
 * \param buf A pointer to the first few bytes of the data
 * \param bytes The count of bytes available at buf
 * \retval FISH_SOUND_xxxxxx FISH_SOUND_VORBIS, FISH_SOUND_SPEEX if
 * \a buf was identified as the initial bytes of a supported codec
 * \retval FISH_SOUND_UNKNOWN if the codec could not be identified
 * \retval FISH_SOUND_ERR_SHORT_IDENTIFY if \a bytes is less than 8
 * \note If \a bytes is exactly 8, then only a weak check is performed,
 * which is fast but may return a false positive.
 * \note If \a bytes is greater than 8, then a stronger check is performed
 * in which an attempt is made to decode \a buf as the initial header of
 * each supported codec. This is unlikely to return a false positive but
 * is only useful if \a buf is the entire payload of a packet derived from
 * a lower layer such as Ogg framing or UDP datagrams.
 */
int
fish_sound_identify (unsigned char * buf, long bytes);

/**
 * Instantiate a new FishSound* handle
 * \param mode FISH_SOUND_DECODE or FISH_SOUND_ENCODE
 * \param fsinfo
 * \returns A new FishSound* handle, or NULL on error
 */
FishSound * fish_sound_new (int mode, FishSoundInfo * fsinfo);

/**
 * Set the callback for libfishsound to call when it has a block of
 * decoded audio ready
 * \param fsound A FishSound* handle (created with mode FISH_SOUND_DECODE)
 * \param decoded The callback to call
 * \param user_data Arbitrary user data to pass to the callback
 * \returns 0 on success, -1 on failure
 */
int fish_sound_set_decoded_callback (FishSound * fsound,
				     FishSoundDecoded decoded,
				     void * user_data);

/**
 * Set the callback for libfishsound to call when it has a block of
 * encoded data ready
 * \param fsound A FishSound* handle (created with mode FISH_SOUND_ENCODE)
 * \param encoded The callback to call
 * \param user_data Arbitrary user data to pass to the callback
 * \returns 0 on success, -1 on failure
 */
int fish_sound_set_encoded_callback (FishSound * fsound,
				     FishSoundEncoded encoded,
				     void * user_data);

/**
 * Decode a block of data
 * \param fsound A FishSound* handle (created with mode FISH_SOUND_DECODE)
 * \param buf A buffer of data
 * \param bytes A count of bytes to decode (ie. the length of buf)
 * \returns The number of bytes consumed
 */
long fish_sound_decode (FishSound * fsound, unsigned char * buf, long bytes);

/**
 * Encode a block of audio
 * \param fsound A FishSound* handle (created with mode FISH_SOUND_ENCODE)
 * \param pcm The audio data to encode
 * \param frames A count of frames to encode
 * \returns The number of frames encoded
 * \note For multichannel audio, the audio data is interpreted according
 * to the current PCM style
 */
long fish_sound_encode (FishSound * fsound, float ** pcm, long frames);

/**
 * Flush any internally buffered data, forcing encode
 * \param fsound A FishSound* handle
 * \returns 0 on success, -1 on failure
 */
long fish_sound_flush (FishSound * fsound);

/**
 * Reset the codec state of a FishSound object
 * \param fsound A FishSound* handle
 * \returns 0 on success, -1 on failure
 */
int fish_sound_reset (FishSound * fsound);

/**
 * Delete a FishSound object
 * \param fsound A FishSound* handle
 * \returns 0 on success, -1 on failure
 */
int fish_sound_delete (FishSound * fsound);

/**
 * Command interface
 * \param fsound A FishSound* handle
 * \param command The command action
 * \param data Command data
 * \param datasize Size of the data in bytes
 * \returns 0 on success, -1 on failure
 */
int fish_sound_command (FishSound * fsound, int command, void * data,
			int datasize);

/**
 * Query whether a FishSound object is using interleaved PCM
 * \param fsound A FishSound* handle
 * \retval 0 \a fsound uses non-interleaved PCM
 * \retval 1 \a fsound uses interleaved PCM
 * \retval -1 Invalid \a fsound
 */
int fish_sound_get_interleave (FishSound * fsound);

/**
 * Set the PCM format used by a FishSound object. The default value is
 * non-interleaved.
 * \param fsound A FishSound* handle
 * \param interleave Whether to use interleaved PCM or not. Valid values are
 * 0 for non-interleaved, and 1 for interleaved.
 * \retval 0 Success
 * \retval -1 Invalid \a fsound
 */
int fish_sound_set_interleave (FishSound * fsound, int interleave);

#endif /* __FISH_SOUND_H__ */
