/*
 * File: codec_helixmp3.c
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#define DBG_SECTION_NAME "codec_helixmp3" 

#include <rtthread.h>
#include <stdint.h>
#include <string.h>

#include "player.h"
#include "player_system.h"
#include "audio_stream.h"
#include "audio_codec.h"
#include "audio_device.h"
#include "codec_helixmp3.h"
#include "mp3dec.h"

#define MP3_AUDIO_BUF_SZ    (8 * 1024) /* feel free to change this, but keep big enough for >= one frame at high bitrates */
#define SEEK_SKIP_FRAME_CNT (40)

struct audio_codec_mp3
{
    struct audio_codec parent;

    /* mp3 information */
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
    rt_uint32_t frames;
    rt_uint32_t stream_offset;

    /* mp3 read session */
    rt_uint8_t *read_buffer, *read_ptr;
    rt_uint32_t bytes_left;

    int current_sample_rate;

    /* seek configuration */
    int seek_event;
    int seek_skip_cnt;  /* 记录seek时跳过的Frame */
};

static uint32_t calc_mp3_position(struct audio_codec_mp3 *codec, int second)
{
    uint32_t position = 0;

    if (codec)
    {
        int sample_rate = codec->frame_info.samprate;
        int bit_rate = codec->frame_info.bitrate;

        /* CBR:                    number_of_frames         * frame_size                     */
        position = (uint32_t)(second * sample_rate / 1152) * (4 + 144 * bit_rate / sample_rate);
        position += codec->stream_offset;
    }

    return position;
}

static rt_int32_t codec_mp3_fill_buffer(struct audio_codec_mp3 *codec)
{
    int bytes_read;
    rt_size_t bytes_to_read;
    struct audio_stream *stream = CODEC_STREAM(codec);

    /* adjust read ptr */
    if(codec->bytes_left > 0xffff0000)
    {
        LOG_D("c: 0x%x, buf: 0x%x.", codec, codec->read_buffer); 
        LOG_D("rd: 0x%x, left: 0x%x.", codec->read_ptr, codec->bytes_left); 
        
        return (-1); 
    }

    if (codec->bytes_left > 0)
    {
        memmove(codec->read_buffer, codec->read_ptr, codec->bytes_left);
    }
    codec->read_ptr = codec->read_buffer;

    bytes_to_read = (MP3_AUDIO_BUF_SZ - codec->bytes_left) & ~(512 - 1);

__retry:
    bytes_read = audio_stream_fetch(stream, (rt_uint8_t *)(codec->read_buffer + codec->bytes_left), bytes_to_read);
    if (bytes_read > 0)
    {
        codec->bytes_left = codec->bytes_left + bytes_read;
        return 0;
    }
    else
    {
        if (bytes_read == STREAM_STATUS_TIMEOUT)
        {
            int event = player_event_handle(0);

            /* handle player event */
            if (event == PLAYER_BREAKOUT)
            {
                return -1;
            }
            else if (event == PLAYER_DO_SEEK)
            {
                int offset = player_fetch_seek();

                offset = calc_mp3_position(codec, offset);
                audio_stream_seek(stream, offset);
                LOG_I("audio_stream_seek(stream, offset); offset = %d", offset); 
            }
            else goto __retry;
        }
        else if (codec->bytes_left != 0)
        {
            return 0;
        }
    }

    LOG_D("can't read more data, end of stream. left=%d.", codec->bytes_left);
    return -1;
}

static int check_mp3_sync_word(struct audio_codec_mp3 *codec)
{
    int err;

    memset(&codec->frame_info, 0, sizeof(MP3FrameInfo));

    err = MP3GetNextFrameInfo(codec->decoder, &codec->frame_info, codec->read_ptr);
    if (err == ERR_MP3_INVALID_FRAMEHEADER)
    {
        goto __err;
    }
    else if (err != ERR_MP3_NONE)
    {
        goto __err;
    }
    else if (codec->frame_info.nChans != 1 && codec->frame_info.nChans != 2)
    {
        goto __err;
    }
    else if (codec->frame_info.bitsPerSample != 16 && codec->frame_info.bitsPerSample != 8)
    {
        goto __err;
    }

    return 0;

__err:
    return -1;
}

static void calc_mp3_time(struct audio_codec_mp3 *codec, MP3FrameInfo *frame_info)
{
    uint32_t T1, T2;
    struct audio_codec *ac;

    T1 = (uint32_t)(frame_info->outputSamps / (frame_info->samprate / 1000.0));

    if (frame_info->nChans == 2) T1 /= 2;

    T2 = player_get_position() + T1;
    player_set_position(T2);

    ac = &codec->parent;
    if (ac->stream->total_length > 0)
    {
        player_set_duration(ac->stream->total_length / (frame_info->bitrate / 8));
    }
    else
    {
        player_set_duration(-1);
    }
}

/* skip id3 tag */
static int codec_mp3_skip_idtag(struct audio_codec_mp3 *codec_mp3, struct audio_stream *stream)
{
    int  offset = 0;
    uint8_t *tag;

    /* set the read_ptr to the read buffer */
    codec_mp3->read_ptr = codec_mp3->read_buffer;

    tag = codec_mp3->read_ptr;
    /* read idtag v2 */
    if (audio_stream_fetch(stream, codec_mp3->read_ptr, 3) != 3)
        goto __exit;

    codec_mp3->bytes_left = 3; 
    if (tag[0] == 'I' &&
        tag[1] == 'D' &&
        tag[2] == '3')
    {
        int  size;

        if (audio_stream_fetch(stream, codec_mp3->read_ptr + 3, 7) != 7)
            goto __exit;

        // http://id3.org/id3v2.3.0 -> ID3v2_size = (4 * %0xxxxxxx)
        size = ((tag[6] & 0x7F) << 21) | ((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | ((tag[9] & 0x7F));

        /* invalid id3 tag */
        if (stream->total_length != -1 && size > stream->total_length)
            return 0;

        offset = size + 10;

        /* read all of idv3 */
        {
            int rest_size = size;
            while (1)
            {
                int length;
                int chunk;

                if (rest_size > MP3_AUDIO_BUF_SZ) chunk = MP3_AUDIO_BUF_SZ;
                else chunk = rest_size;

                length = audio_stream_fetch(stream, codec_mp3->read_buffer, chunk);
                if (length > 0)
                {
                    rest_size -= length;
                }
                else
                {
                    break; /* read failed */
                }
            }

            codec_mp3->bytes_left = 0;
        }

        return offset;
    }

__exit:
    return offset;
}

static struct audio_codec *codec_mp3_create(struct audio_stream *stream)
{
    struct audio_codec_mp3 *codec_mp3 = RT_NULL;

    codec_mp3 = (struct audio_codec_mp3 *) tcm_malloc(sizeof(struct audio_codec_mp3));
    if (!codec_mp3) return RT_NULL;

    memset(codec_mp3, 0x00, sizeof(struct audio_codec_mp3));

    codec_mp3->parent.seekable = 1;

    /* init read session */
    codec_mp3->read_ptr = RT_NULL;
    codec_mp3->bytes_left = 0;
    codec_mp3->frames = 0;

    codec_mp3->read_buffer = tcm_malloc(MP3_AUDIO_BUF_SZ);
    if (codec_mp3->read_buffer == RT_NULL)
    {
        LOG_E("codec_mp3->read_buffer malloc failed!"); 
        goto _failed;
    }
    
    codec_mp3->stream_offset = codec_mp3_skip_idtag(codec_mp3, stream);

    codec_mp3->decoder = MP3InitDecoder();
    if (!codec_mp3->decoder)
    {
        LOG_E("codec_mp3->decoder create failed!"); 
        goto _failed;
    }

    return (struct audio_codec *)codec_mp3;

_failed:
    if (codec_mp3->decoder)
        MP3FreeDecoder(codec_mp3->decoder);

    if (codec_mp3->read_buffer)
        tcm_free(codec_mp3->read_buffer);

    tcm_free(codec_mp3);

    return RT_NULL;
}

static int codec_mp3_run(struct audio_codec *codec)
{
    int read_offset;
    int err, event;
    rt_uint16_t *buffer;

    struct audio_codec_mp3 *codec_mp3;

    codec_mp3 = (struct audio_codec_mp3 *)codec;
    if (!codec_mp3) return -1;

    event = player_event_handle(0);
    if (event == PLAYER_BREAKOUT)
    {
        return -1;
    }
    else if (event == PLAYER_DO_SEEK)
    {
        int offset = player_fetch_seek();

        offset = calc_mp3_position(codec_mp3, offset);
        audio_stream_seek(codec_mp3->parent.stream, offset);
        LOG_E("audio_stream_seek."); 
    }

    if ((codec_mp3->read_ptr == RT_NULL) || codec_mp3->bytes_left < 2 * MAINBUF_SIZE)
    {
        if (codec_mp3_fill_buffer(codec_mp3) != 0) return -1;
    }

    read_offset = MP3FindSyncWord(codec_mp3->read_ptr, codec_mp3->bytes_left);
    if (read_offset < 0)
    {
        /* discard this data */
        LOG_D("outof sync, byte left: %d.", codec_mp3->bytes_left);

        codec_mp3->bytes_left = 0;
        return 0;
    }

    if(read_offset > codec_mp3->bytes_left)
    {
        LOG_D("find sync exception: %d:%d.", read_offset, codec_mp3->bytes_left); 
        codec_mp3->read_ptr += codec_mp3->bytes_left;
        codec_mp3->bytes_left = 0; 
    }
    else
    {
        codec_mp3->read_ptr += read_offset;
        codec_mp3->bytes_left -= read_offset;
    }

    if (check_mp3_sync_word(codec_mp3) == -1)
    {
        if (codec_mp3->bytes_left > 0)
        {
            codec_mp3->bytes_left --;
            codec_mp3->read_ptr ++;
        }

        return 0;
    }

    if (codec_mp3->bytes_left < 1024)
    {
        /* fill more data */
        if (codec_mp3_fill_buffer(codec_mp3) != 0)
            return -1;
    }

    /* get a decoder buffer */
    buffer = (rt_uint16_t *)audio_device_get_buffer(RT_NULL);
    if (!buffer)
    {
        LOG_E("get audio device buffer failed!");
        return -1;
    }

    err = MP3Decode(codec_mp3->decoder, &codec_mp3->read_ptr,
                    (int *)&codec_mp3->bytes_left, (short *)buffer, 0);

    codec_mp3->frames++;

    if (err != ERR_MP3_NONE)
    {
        switch (err)
        {
        case ERR_MP3_INDATA_UNDERFLOW:
            // LOG_E("ERR_MP3_INDATA_UNDERFLOW.");
            // codec_mp3->bytes_left = 0;
            if (codec_mp3_fill_buffer(codec_mp3) != 0)
            {
                /* release this memory block */
                audio_device_put_buffer((uint8_t *)buffer);
                return -1;
            }
            break;

        case ERR_MP3_MAINDATA_UNDERFLOW:
            /* do nothing - next call to decode will provide more mainData */
            // LOG_E("ERR_MP3_MAINDATA_UNDERFLOW.");
            break;

        default:
            // LOG_E("unknown error: %d, left: %d.", err, codec_mp3->bytes_left);
            // LOG_D("stream position: %d.", codec_mp3->parent.stream->position);
            // stream_buffer(0, NULL);

            // skip this frame
            if (codec_mp3->bytes_left > 0)
            {
                codec_mp3->bytes_left --;
                codec_mp3->read_ptr ++;
            }
            else
            {
                // TODO
                RT_ASSERT(0);
            }
            break;
        }

        /* release this memory block */
        audio_device_put_buffer((uint8_t *)buffer);
    }
    else
    {
        int outputSamps;
        /* no error */
        MP3GetLastFrameInfo(codec_mp3->decoder, &codec_mp3->frame_info);

        /* for the first frame, whether it's a play session with position */
        if (player_get_position() == -1)
        {
            int offset = player_fetch_seek();

            if (offset > 0)
            {
                audio_device_put_buffer((uint8_t *)buffer);

                /* set music position */
                player_set_position(offset * 1000);

                offset = calc_mp3_position(codec_mp3, offset);
                offset -= (512 * 20);
                if (offset <= 0)
                {
                    offset = 0;
                }
                audio_stream_seek(codec_mp3->parent.stream, offset);
                LOG_E("audio_stream_seek(codec_mp3->parent.stream, offset);2 offset = %d", offset); 

                /* reset read buffer */
                codec_mp3->read_ptr = RT_NULL;
                codec_mp3->bytes_left = 0;
                codec_mp3->frames = 0;
                codec_mp3->seek_event = 1;

                return 0; /* try the next frame */
            }
        }

        calc_mp3_time(codec_mp3, &codec_mp3->frame_info);

        /* write to sound device */
        outputSamps = codec_mp3->frame_info.outputSamps;
        if (outputSamps > 0)
        {
            if (codec_mp3->frame_info.nChans == 1)
            {
                int i;
                for (i = outputSamps - 1; i >= 0; i--)
                {
                    buffer[i * 2] = buffer[i];
                    buffer[i * 2 + 1] = buffer[i];
                }
                outputSamps *= 2;
            }

            if (codec_mp3->seek_event == 1)
            {
                codec_mp3->seek_skip_cnt ++;
                if (codec_mp3->seek_skip_cnt >= SEEK_SKIP_FRAME_CNT)
                {
                    codec_mp3->seek_skip_cnt = 0;
                    codec_mp3->seek_event = 0;
                }
                audio_device_put_buffer((uint8_t *)buffer);
            }
            else
            {
                /* set sample rate */
                if (codec_mp3->frame_info.samprate != codec_mp3->current_sample_rate)
                {
                    rt_uint16_t *buffer = RT_NULL;

                    codec_mp3->current_sample_rate = codec_mp3->frame_info.samprate;
                    audio_device_set_rate(codec_mp3->current_sample_rate);
                }

                audio_device_write((uint8_t *)buffer, outputSamps * sizeof(rt_uint16_t));
            }
        }
        else
        {
            /* no output */
            audio_device_put_buffer((uint8_t *)buffer);
        }
    }

    return 0;
}

static int codec_mp3_destory(struct audio_codec *codec)
{
    struct audio_codec_mp3 *codec_mp3;

    codec_mp3 = (struct audio_codec_mp3 *)codec;
    if (codec_mp3)
    {
        /* release read buffer */
        if (codec_mp3->read_buffer)
        {
            tcm_free(codec_mp3->read_buffer);
        }

        /* release mp3 decoder */
        MP3FreeDecoder(codec_mp3->decoder);
    }

    return 0;
}

static struct audio_codec_ops _mp3_ops =
{
    codec_mp3_create, 
    codec_mp3_run,
    codec_mp3_destory,
};

int player_codec_helixmp3_register(void)
{
    return audio_codec_register(CODEC_MP3, &_mp3_ops); 
}
