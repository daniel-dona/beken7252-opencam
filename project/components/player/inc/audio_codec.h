/*
 * File: audio_codec.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __AUDIO_CODEC_H__
#define __AUDIO_CODEC_H__

#include "audio_stream.h"

enum audio_codec_type
{
    CODEC_UNKNOWN = 0, 
    CODEC_PCM,
    CODEC_WAV,
    CODEC_MP3,
    CODEC_AMR,
    CODEC_AAC,
    CODEC_OGG,
    CODEC_WMA,
    CODEC_FLAC,
    FORMAT_M4A,
    FORMAT_TS,
    CODEC_CUSTOM,
    CODEC_MAX
};
typedef int audio_codec_type_t; 

struct audio_codec
{
    uint32_t seekable;
    struct audio_stream *stream;
};
#define CODEC_STREAM(codec) (((struct audio_codec*)(codec))->stream)

struct audio_codec_ops
{
    struct audio_codec *(*create)(struct audio_stream *stream); 
    int (*run)(struct audio_codec *codec);
    int (*destory)(struct audio_codec *codec);
};

int audio_codec_get_type(const char *URI);
int audio_codec_get_type_by_mime(const char *mime);
struct audio_codec* audio_codec_create(struct audio_stream *stream);
int audio_codec_destory(struct audio_codec *codec);
int audio_codec_run(struct audio_codec *codec);
int audio_codec_register(audio_codec_type_t type, struct audio_codec_ops *ops); 
char *audio_codec_tostring(int codec_type); 
void audio_codec_dump(void); 

#endif
