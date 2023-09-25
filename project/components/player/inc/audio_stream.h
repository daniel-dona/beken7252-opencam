/*
 * File: audio_stream.h
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

struct audio_stream;
struct audio_codec;

enum audio_stream_status
{
    STREAM_STATUS_OK = 0,
    STREAM_STATUS_ERROR = -1,
    STREAM_STATUS_TIMEOUT = -2,
    STREAM_STATUS_EOF = -3,
};

struct audio_stream_ops
{
    struct audio_stream* (*open)(const char* URI);
    int (*close)(struct audio_stream *stream);

    /* return stream status */
    int (*fetch)(struct audio_stream *stream, void *buffer, int size);
    int (*seek) (struct audio_stream *stream, int offset);
};

struct audio_stream
{
    const struct audio_stream_ops *ops;
	uint32_t seekable;

	int eof;	/* end of stream */
	int sof;	/* start offset of frame */
    uint32_t position;
    uint32_t total_length;

    int codec_type;
    struct audio_codec *codec;
    char *uri; 
};

void audio_stream_init(struct audio_stream *stream, const struct audio_stream_ops *ops);
struct audio_stream* audio_stream_open(const char* URI);
void audio_stream_close(struct audio_stream* stream);
int audio_stream_fetch(struct audio_stream *stream, void* buffer, int size);
int audio_stream_seek(struct audio_stream *stream, int offset);
int audio_stream_skip(struct audio_stream *stream, int skip);
struct audio_codec *audio_stream_create_codec(struct audio_stream *stream);

#endif
