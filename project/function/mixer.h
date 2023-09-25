#ifndef _MIXER_H_
#define _MIXER_H_

#include "rtthread.h"
#include "include.h"
#include "doubly_list.h"

#define MIXER_DEBUG_PRTF             0
#if MIXER_DEBUG_PRTF
#define MIXER_PRINTF                 rt_kprintf
#define MIXER_WARNING_LOG            rt_kprintf
#define MIXER_LOG(...)
#else
#define MIXER_PRINTF(...)
#define MIXER_WARNING_LOG(...)            
#define MIXER_LOG(...)
#endif //MIXER_DEBUG_PRTF

#define MIXER_MUX_NEW_STRATEGY      (0)

#define MIXER_FAILURE               (1)
#define MIXER_SUCCESS               (0)

#define CFG_SUPPORT_SINGLE_CHANNEL  (1)

#define MIXER_DEFAULT_SAMPLE_RATE   (8000)

#define MIXER_STACK_SIZE            (1024 * 2)
#define MIXER_THREAD_PRIORITY       (21)
#define MSG_SIGNATURE               (0x5a5a)
#define DATA_SIGNATURE              (0xa5a5)
#define MX_MSG_QUEUE_NAME            "mixer_mq"
#define MX_MSG_QUEUE_TIMEOUT         (4)
#define MX_MSG_QUEUE_COUNT           (10)
#define MX_MUTEX_NAME                "mx_mutex"
#define MX_THREAD_NAME               "mx_thread"

#define OUTSIDE_SRC_EXCHANGE_BUF_SIZE        ((1024 + 4) * 8)

typedef struct _adc_device_
{
    uint32_t rate;
    uint32_t channel;

    rt_device_t dev;
} ADC_DEV_T;

typedef struct _dac_device_
{
    uint32_t rate;

    rt_device_t dev;
} DAC_DEV_T;

/* OUTSIDE_SRC_EXCHANGE_BUF_SIZE = OUTSIDE_MP_BLOCK_SIZE * OUTSIDE_MP_BLOCK_SIZE*/
#define OUTSIDE_MP_BLOCK_SIZE            (1024)
#define OUTSIDE_MP_BLOCK_COUNT            (8)

#define MP_BLOCK_SIZE            (1024)
#define MP_BLOCK_COUNT            (9)

#if MIXER_MUX_NEW_STRATEGY
typedef struct _mixer_mempool_
{
    uint32_t blk_size;
    uint32_t blk_count;
    struct rt_mempool pool;
} MP_T;
#else
typedef struct _mixer_mempool_
{
    uint32_t blk_size;
    uint32_t blk_count;
    rt_mp_t pool;
} MP_T;
#endif

typedef struct _music_src_node_
{
    LIST_HEADER_T hdr;

    uint8_t *buf;
    uint32_t len;
    uint32_t use_pos;
} MUSIC_SRC_NODE_T;

typedef struct _mixer_
{
    uint32_t is_mux_src;
    uint32_t is_thd_running;
    uint32_t is_thd_pause;

    struct rt_thread mx_thread;
    char mx_stack[MIXER_STACK_SIZE];

    rt_sem_t wait_end;
    rt_mutex_t mx_mutex;
    rt_mq_t mx_msg_queue;

    uint32_t audio_sample_rate;
    ADC_DEV_T adc_dev;

#if MIXER_MUX_NEW_STRATEGY
    /* OUTSIDE_SRC_EXCHANGE_BUF_SIZE, the length is equal to
       the memory cell length of audio_device_init at the mem
       pool. outside_src_buf will be used at mux src mode, and
       outside_mp will be used at single src mode*/
    char *outside_src_buf;
    uint32_t outside_src_pos;

    MP_T outside_mp;
#else
    MP_T mp;
#endif

    DAC_DEV_T dac_dev;

    LIST_HEADER_T music_src_list;
} MIXER_T;

typedef struct _mixer_msg_
{
    uint32_t sig;
    uint32_t detail;
    uint32_t len;
} MIXER_MSG_T;

enum
{
    MSG_TYPE_NULL = 0,
    MSG_TYPE_AUDIO_SRC_FLOW,
    MSG_TYPE_AUDIO_SRC_STATIC,
    MSG_TYPE_AUDIO_SET_SAMPLE_RATE,
    MSG_TYPE_PAUSE_MIXER,
    MSG_TYPE_REPLAY_MIXER,
};


enum
{
    MIX_ACT_PLAYING = 0,
    MIX_ACT_PAUSE,
};

uint32_t mixer_init(void);
uint32_t mixer_device_write(void *, int);
uint32_t mixer_device_set_rate(uint32_t);
uint32_t mixer_send_msg_audio_src_flow(void);
uint32_t mixer_send_msg_audio_src_static(void);

void mixer_pause(void);
void mixer_replay(void);

#endif // _MIXER_H_
// eof

