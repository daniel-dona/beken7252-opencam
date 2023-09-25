#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <rtthread.h>

#include <player.h>
#include <player_system.h>

#include <audio_stream.h>
#include <audio_device.h>
#include <audio_codec.h>

#include "format_m4a.h"
#include "lib_aac_decode/aac_decoder_api.h"

#define debug_printf(...)
//#define debug_printf    rt_kprintf("[m4a] ");rt_kprintf

#define RAW_DATA_BUFSZ      2048

enum m4a_phase
{
    M4A_PHASE_INIT,
    M4A_PHASE_PARSE_DONE,
};

struct format_m4a
{
    struct audio_codec parent;
    int phase;

    // beken liaixing
    uint32_t aac_decoder_sample_rate;
    uint32_t aac_decoder_num_channels;
    uint32_t aac_decoder_pcm_samples;
    uint8_t *aac_decoder_pcm_buffer;

    int current_sample_rate;
    void *decoder;

    float T1; 
};

static int empty = 0; 

static void calc_aac_time(struct format_m4a *format_m4a, int outputSamps)
{
    format_m4a->T1 += ((float)outputSamps / ((float)(4 * format_m4a->aac_decoder_sample_rate) / 1000)); 

    player_set_position((uint32_t)format_m4a->T1); 
}

static struct audio_stream *stream = 0;
static uint32_t http_data_fetch_aac(void *buffer, uint32_t size, uint32_t count, void *handle)
{
    int need_read, bytes_read, retry, tmp;

    need_read = size * count;
    bytes_read = 0;
    retry = 5;

    if (!stream)
    {
        debug_printf("%s:%d stream: 0x%08X, need_read:%d\n", __FUNCTION__, __LINE__, stream, need_read);
        return 0;
    }

    while ((bytes_read < need_read) && (retry--))
    {
        tmp = audio_stream_fetch(stream, ((rt_uint8_t *)buffer) + bytes_read, need_read - bytes_read);
        //debug_printf("%s:%d read: %d\n", __FUNCTION__, __LINE__, tmp);

        if (tmp > 0)
        {
            bytes_read += tmp;
        }
        else if (tmp == 0)
        {
            rt_thread_delay(10);
        }
        else
        {
            rt_thread_delay(10);
        }
    }

    if(bytes_read == 0)
    {
        empty = 1; 
    }

    if (bytes_read != need_read)
    {
        rt_kprintf("%s:%d bytes_read: %d:%d\n", __FUNCTION__, __LINE__, bytes_read, need_read);
    }

    return bytes_read;
}

struct audio_codec *format_m4a_create(struct audio_stream *stream)
{
    struct format_m4a *m4a;
    struct audio_codec *ac = NULL;

    m4a = (struct format_m4a *) player_malloc(sizeof(struct format_m4a));
    if (!m4a) return NULL;

    memset(m4a, 0x0, sizeof(struct format_m4a));

    m4a->phase = M4A_PHASE_INIT;
    m4a->parent.seekable = 0;
    m4a->T1 = 0.0f; 
    m4a->decoder = RT_NULL; 
    empty = 0; 

    return &(m4a->parent);
}

static int format_m4a_run(struct audio_codec *codec)
{
    int result = 0;
    int err, event;
    struct format_m4a *m4a;
    int bytes_read;

    m4a = (struct format_m4a *)codec; 
    if (!m4a) return -1;

    if (!stream)
    {
        stream = CODEC_STREAM(codec);
        debug_printf("%s:%d update stream: 0x%08X\n", __FUNCTION__, __LINE__, stream);
    }

    event = player_event_handle(0);
    if (event == PLAYER_BREAKOUT)
    {
        debug_printf("%s%d: PLAYER_BREAKOUT\n", __FUNCTION__, __LINE__);
        return -1;
    }
    else if (event == PLAYER_DO_SEEK)
    {
        debug_printf("%s%d: NO Support PLAYER_DO_SEEK\n", __FUNCTION__, __LINE__);
    }

    if (m4a->phase == M4A_PHASE_INIT)
    {
        unsigned char *buffer;
        int buffer_size;

        debug_printf("%s%d: M4A_PHASE_INIT\n", __FUNCTION__, __LINE__);

        list_memheap(); //TODO: 
        m4a->decoder = (void *)rt_malloc(aac_decoder_get_ram_size());
        debug_printf("m4a->decoder malloc size = %d.\n", aac_decoder_get_ram_size()); 
        if (m4a->decoder)
        {
            err = aac_decoder_initialize(m4a->decoder, NULL, http_data_fetch_aac, NULL, NULL);
            if (0 == err)
            {
                debug_printf("%s%d: aac_decoder_initialize\n", __FUNCTION__, __LINE__);

                m4a->aac_decoder_sample_rate  = aac_decoder_get_sample_rate(m4a->decoder);
                m4a->aac_decoder_num_channels = aac_decoder_get_num_channels(m4a->decoder);
                m4a->aac_decoder_pcm_samples  = aac_decoder_get_pcm_samples(m4a->decoder);
                m4a->aac_decoder_pcm_buffer   = aac_decoder_get_pcm_buffer(m4a->decoder);

                audio_device_set_rate(m4a->aac_decoder_sample_rate);

                debug_printf("%s%d: aac_decoder_sample_rate: %d\n", __FUNCTION__, __LINE__, m4a->aac_decoder_sample_rate);
                debug_printf("%s%d: aac_decoder_num_channels: %d\n", __FUNCTION__, __LINE__, m4a->aac_decoder_num_channels);
                debug_printf("%s%d: aac_decoder_pcm_samples: %d\n", __FUNCTION__, __LINE__, m4a->aac_decoder_pcm_samples);
                debug_printf("%s%d: aac_decoder_pcm_buffer: 0x%08X\n", __FUNCTION__, __LINE__, m4a->aac_decoder_pcm_buffer);
            }
            else
            {
                debug_printf("%s%d: m4a->decoder aac_decoder_initialize err: %d\n", __FUNCTION__, __LINE__, err);
                result = -1;
                goto __exit;
            }
        }
        else
        {
            debug_printf("%s%d: m4a->decoder null\n", __FUNCTION__, __LINE__);
            result = -1;
            goto __exit; // TODO: 修复内存不足, 分配失败导致死机
        }

        m4a->phase = M4A_PHASE_PARSE_DONE;
        debug_printf("%s%d: ==> M4A_PHASE_PARSE_DONE\n", __FUNCTION__, __LINE__);
    }
    else if (m4a->phase == M4A_PHASE_PARSE_DONE)
    {
        int sample_size, read_size;
        uint16_t *buffer;
        int buffer_size;
        uint8_t  *ptr;
        int event;
        int32_t ec;

        //debug_printf("%s%d: M4A_PHASE_PARSE_DONE\n", __FUNCTION__, __LINE__);

        /* get a decoder buffer */
        buffer = (rt_uint16_t *)audio_device_get_buffer(&buffer_size);
        if (!buffer)
        {
            printf("get audio device buffer failed!\n");
            result = -1;
            goto __exit;
        }

        ec = aac_decoder_decode(m4a->decoder);
        if (ec == 0)
        {
            if (m4a->aac_decoder_num_channels == 2)
            {
                memcpy(buffer, m4a->aac_decoder_pcm_buffer, m4a->aac_decoder_pcm_samples * 2 * sizeof(rt_uint16_t));

                /* calc_aac_time */
                calc_aac_time(m4a, m4a->aac_decoder_pcm_samples * 2 * sizeof(rt_uint16_t)); 

                audio_device_write(buffer, m4a->aac_decoder_pcm_samples * 2 * sizeof(rt_uint16_t));
            }
            else
            {
                int i;
                int16_t *src, *dst;

                memcpy(buffer + m4a->aac_decoder_pcm_samples,
                       m4a->aac_decoder_pcm_buffer,
                       m4a->aac_decoder_pcm_samples * sizeof(rt_uint16_t));

                // convert to two channel
                src = (int16_t *)(buffer + m4a->aac_decoder_pcm_samples);
                dst = (int16_t *)(buffer);
                for (i = 0; i < m4a->aac_decoder_pcm_samples; i++)
                {
                    dst[2 * i] = src[i];
                    dst[2 * i + 1] = src[i];
                }

                /* calc_aac_time */
                calc_aac_time(m4a, m4a->aac_decoder_pcm_samples * 2 * sizeof(rt_uint16_t)); 

                audio_device_write(buffer, m4a->aac_decoder_pcm_samples * 2 * sizeof(rt_uint16_t));
            }
        }
        else
        {
            if ((empty) && (ec == 2 || ec == 9 || ec == 3))
            {
                rt_kprintf("%s%d: aac_decoder_decode finish: %d\n", __FUNCTION__, __LINE__, ec);
                result = -1;
            }

            rt_kprintf("%s%d: aac_decoder_decode: %d\n", __FUNCTION__, __LINE__, ec);
            audio_device_put_buffer(buffer);
        }
    }

__exit:
    return result;
}

// enum
// {
//     ERR_AAC_UNRECOVERABLE = -256,
//     ERR_AAC_END_OF_STREAM,
//     ERR_AAC_INIT_FAIL,
//     ERR_AAC_INCORECT_MP4_FTYP,
//     ERR_AAC_INCORRECT_ADIF_ID,
//     ERR_AAC_INVALID_ADIF_HEADER,
//     ERR_AAC_UNSUPPORT_AUDIO_OBJECT_TYPE,
//     ERR_AAC_UNSUPPORT_FRAME_LENGTH,
//     ERR_AAC_UNSUPPORT_SAMPLING_RATE,
//     ERR_AAC_UNSUPPORT_CHANNEL_MODE,
//     ERR_AAC_CHANNEL_MAP,
//     ERR_AAC_INVALID_ATOM_HEADER,
//     ERR_AAC_UNSUPPORT_STSC_ENTRIES_MODE,
//     ERR_AAC_FSEEK_FAIL,

//     ERR_AAC_NONE = 0,

//     ERR_AAC_RECOVERABLE = 1,
//     ERR_AAC_STREAM_EMPTY = 2,
//     ERR_AAC_INCORRECT_ADTS_SYNCWORD = 3,
//     ERR_AAC_INVALID_ADTS_HEADER = 4,
//     ERR_AAC_SBR_BITSTREAM = 5,
//     ERR_AAC_SBR_DATA = 6,
//     ERR_AAC_UNSUPPORT_SBR_SAMPLING_RATE = 7,
//     ERR_AAC_SBR_PCM_FORMAT = 8,
//     ERR_AAC_ADTS_BAD_HEADER = 9,
//     ERR_AAC_ADTS_CHANNEL_MAPPING_FAIL,
//     ERR_AAC_ADTS_UNSUPPORT_CHANNEL_MODE,
// };

static int format_m4a_destory(struct audio_codec *codec)
{
    int result = 0;
    struct format_m4a *m4a;

    debug_printf("%s:%d\n", __FUNCTION__, __LINE__);

    m4a = (struct format_m4a *) codec;
    if (m4a)
    {
        if (m4a->decoder)
        {
            rt_free(m4a->decoder);
            debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        result = -1;
    }

    // when destory, stream set to NULL
    // fixed the bug that the second time play m4a, stream is not NULL, 
    // but stream->ops is NULL.  by jiewu
    stream = NULL;

    return result;
}

static struct audio_codec_ops _m4a_ops =
{
    format_m4a_create, 
    format_m4a_run,
    format_m4a_destory,
};

int player_codec_beken_aac_register(void)
{
    return audio_codec_register(CODEC_AAC, &_m4a_ops); 
}

int player_codec_beken_m4a_register(void)
{
    return audio_codec_register(FORMAT_M4A, &_m4a_ops); 
}
