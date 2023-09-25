/*
 * File: codec_opencore_amr.c
 * 
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 */

#define DBG_SECTION_NAME "codec_opencore_amr" 

#include <stdint.h>
#include <string.h>

#include "player.h"
#include "player_system.h"
#include "audio_stream.h"
#include "audio_codec.h"
#include "audio_device.h"

#include <rtthread.h>

#include "codec_opencore_amr.h"
#include <interf_dec.h>

struct audio_codec_amr
{
    struct audio_codec parent;

    void *amr;

    int current_sample_rate;
};

static const char *AMR_MAGIC_NUMBER = "#!AMR\n";
static const int amr_frame[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };

struct audio_codec *codec_amr_create(struct audio_stream *stream)
{
    struct audio_codec_amr *codec_amr = 0;

    codec_amr = (struct audio_codec_amr *) player_malloc(sizeof(struct audio_codec_amr));
    if (!codec_amr) return 0;

    memset(codec_amr, 0, sizeof(struct audio_codec_amr));
    codec_amr->parent.seekable = 1;

    codec_amr->amr = Decoder_Interface_init();
    if (!codec_amr->amr) return 0;

    return (struct audio_codec *)codec_amr;
}

static int codec_amr_run(struct audio_codec *codec)
{
    int err, event;
    uint8_t *buffer;
    int bytes_read, buf_sz;
    uint32_t buffer_pos;

    struct audio_codec_amr *codec_amr;
    struct audio_stream *stream;

    codec_amr = (struct audio_codec_amr *)codec;
    if (!codec_amr) return -1;

    stream = CODEC_STREAM(codec);

    event = player_event_handle(0);
    if (event == PLAYER_BREAKOUT)
    {
        return -1;
    }
    else if (event == PLAYER_DO_SEEK)
    {
        LOG_D("codec_amr_run: event %d PLAYER_DO_SEEK.", event);
    }

    if (codec_amr->current_sample_rate == 0)
    {
        char amr_header[6 + 1], len;

        LOG_D("current_sample_rate == 0.");

        len  = strlen(AMR_MAGIC_NUMBER);
        amr_header[len] = 0;
        bytes_read = audio_stream_fetch(stream, (rt_uint8_t *)(amr_header), len);
        if (bytes_read != len)
        {
            LOG_E("read amr AMR_MAGIC_NUMBER err! %d:%d.", bytes_read, len);
            return -1;
        }

        if (strcmp(AMR_MAGIC_NUMBER, amr_header) != 0)
        {
            LOG_E("amr magic number error:%s.", amr_header);
            return -1;
        }

        codec_amr->current_sample_rate = 8000; /* aozima: fixed amr-nb */

        audio_device_set_rate(codec_amr->current_sample_rate);
    }

    buffer = (uint8_t *)audio_device_get_buffer(&buf_sz);
    if (buf_sz > (640 * 4))
    {
        buf_sz = 640 * 4;
    }

    for (buffer_pos = 0; buffer_pos < (buf_sz - 640); buffer_pos += 640)
    {
        uint8_t amr_data[32];
        int amr_size;

        /* get amr frame header. */
        bytes_read = audio_stream_fetch(stream, (rt_uint8_t *)(&amr_data[0]), 1);
        if (bytes_read != 1)
        {
            LOG_E("read amr frame header err, break!");
            break;
        }

        amr_size = amr_frame[(amr_data[0] >> 3) & 0x0f]; /* TODO: amr size check. */
        bytes_read = audio_stream_fetch(stream, (rt_uint8_t *)(&amr_data[1]), amr_size);
        if (bytes_read != amr_size)
        {
            LOG_E("read amr_size err, %d:%d.", amr_size, bytes_read);
            break;
        }

        Decoder_Interface_Decode(codec_amr->amr, (const unsigned char *)amr_data, (short *)(&buffer[buffer_pos]), 0);

        /* mono to stereo. */
        {
            int i;
            int16_t *ptr = (int16_t *)(&buffer[buffer_pos]);

            for (i = 160 - 1; i >= 0; i--)
            {
                ptr[i * 2]     = ptr[i];
                ptr[i * 2 + 1] = ptr[i];
            }
        }

        event = player_event_handle(0);
        if (event == PLAYER_BREAKOUT)
        {
            buffer_pos += 640;
            audio_device_write((uint8_t *)buffer, buffer_pos);
            return -1;
        }
    }


    if (buffer_pos > 0)
    {
        audio_device_write((uint8_t *)buffer, buffer_pos);
        return 0;
    }
    else
    {
        audio_device_put_buffer((uint8_t *)buffer);
        return -1;
    }

    return 0;
}

static int codec_amr_destory(struct audio_codec *codec)
{
    struct audio_codec_amr *codec_amr = (struct audio_codec_amr *)codec;

    if (codec_amr)
    {
        Decoder_Interface_exit(codec_amr->amr);
    }

    return 0;
}

static struct audio_codec_ops _amr_ops =
{
    codec_amr_create, 
    codec_amr_run,
    codec_amr_destory,
};

int player_codec_opencore_amr_register(void)
{
    return audio_codec_register(CODEC_AMR, &_amr_ops); 
}

