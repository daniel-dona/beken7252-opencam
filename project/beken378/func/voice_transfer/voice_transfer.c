#include "include.h"
#include "arm_arch.h"

#if (CFG_USE_AUDIO && CFG_USE_AUD_ADC)
#include "task.h"
#include "bk_rtos_pub.h"
#include "error.h"
#include "co_list.h"

#include "voice_transfer.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#include "board.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

//#include <finsh.h>
#include <rtdevice.h>
#include <drivers/audio.h>

#define TVOICE_DEBUG                1
#include "uart_pub.h"
#if TVOICE_DEBUG
#define TVOICE_PRT                  os_printf
#define TVOICE_WPRT                 warning_prf
#define TVOICE_FATAL                fatal_prf
#else
#define TVOICE_PRT                  null_prf
#define TVOICE_WPRT                 null_prf
#define TVOICE_FATAL                null_prf
#endif

#define TVOICE_USE_HDR              1

#define TVOICE_RXNODE_SIZE_UDP      1472//32//1472
#define TVOICE_RXNODE_SIZE_TCP      1460//20//1460
#ifndef TVOICE_RXNODE_SIZE
#define TVOICE_RXNODE_SIZE          TVOICE_RXNODE_SIZE_UDP
#endif

#define TVOICE_POOL_LEN             (TVOICE_RXNODE_SIZE * 3)  
#define TVOICE_MSG_WAIT_TIME        (50)  // 1460 * (1/8000/2) = 0.09175 = 90ms 

typedef struct tvoice_hdr_st
{
    UINT32 id;
}TVO_HDR_ST, *TVO_HDR_PTR;

typedef struct tvoice_elem_st
{
    struct co_list_hdr hdr;
    void *buf_start;
    UINT32 buf_len;
} TVOICE_ELEM_ST, *TVOICE_ELEM_PTR;

typedef struct tvoice_desc
{
    //UINT8  pool[TVOICE_POOL_LEN];
    UINT8* pool;
    TVOICE_ELEM_ST elem[TVOICE_POOL_LEN / TVOICE_RXNODE_SIZE];
    struct co_list free;
    struct co_list ready; 
    int elem_len;

    video_transfer_send_func send_func;
    #if(TVOICE_USE_HDR)
    TVO_HDR_ST tv_hdr;
    #endif

    rt_device_t adc_device;
    int n_channel;
} TVOICE_DESC_ST, *TVOICE_DESC_PTR;

TVOICE_DESC_ST tvoice_st;

enum
{
	TVO_EXIT     = 0,
};

typedef struct tvoice_message 
{
	UINT32 data;
}TVO_MSG_T;

#define TVO_QITEM_COUNT      (2)
xTaskHandle tvoice_thread_hdl = NULL;
beken_queue_t tvoice_msg_que = NULL;

void tvoice_intfer_send_msg(UINT32 new_msg)
{
	OSStatus ret;
	TVO_MSG_T msg;

    if(tvoice_msg_que) 
    {
    	msg.data = new_msg;
    	
    	ret = bk_rtos_push_to_queue(&tvoice_msg_que, &msg, BEKEN_NO_WAIT);
    	if(kNoErr != ret)
    	{
    		//TVOICE_WPRT("tvoice send_msg failed\r\n");
    	}
    }
}

static void tvoice_pool_init(void* data)
{
    UINT32 i = 0;

    if(tvoice_st.pool == NULL)
    {
        tvoice_st.pool = sdram_malloc(sizeof(UINT8)* TVOICE_POOL_LEN);
        if(tvoice_st.pool == NULL)
        {
            TVOICE_WPRT("tvoice_pool alloc failed\r\n");
            ASSERT(1);
        }
    }
    
    co_list_init(&tvoice_st.free);
    co_list_init(&tvoice_st.ready);

    for(i = 0; i < (TVOICE_POOL_LEN / TVOICE_RXNODE_SIZE); i++)
    {
        tvoice_st.elem[i].buf_start =
            (void *)&tvoice_st.pool[i * TVOICE_RXNODE_SIZE];
        tvoice_st.elem[i].buf_len = 0;

        co_list_push_back(&tvoice_st.free,
                          (struct co_list_hdr *)&tvoice_st.elem[i].hdr);
    }

    tvoice_st.send_func = (video_transfer_send_func)data;
    tvoice_st.elem_len = TVOICE_RXNODE_SIZE;

    #if(TVOICE_USE_HDR)
    tvoice_st.tv_hdr.id = 0;
    tvoice_st.elem_len -= sizeof(TVO_HDR_ST);
    #endif
}

static void tvoice_send_audio_adc(void)
{
    UINT32 send_len;
    TVOICE_ELEM_PTR elem = NULL;

    do{
        elem = (TVOICE_ELEM_PTR)co_list_pick(&tvoice_st.ready);        
        if(elem) {
            if(tvoice_st.send_func) {
                send_len = tvoice_st.send_func(elem->buf_start, elem->buf_len);
                if(send_len != elem->buf_len) {
                    break;
                }
            }
            
            co_list_pop_front(&tvoice_st.ready);
            co_list_push_back(&tvoice_st.free, (struct co_list_hdr *)&elem->hdr);
        }
    }
    while(elem);
}

static rt_size_t tvoice_sound_read(char *buffer, rt_size_t size)
{
    rt_size_t read_bytes = 0;

    while (read_bytes < size)
    {
        rt_size_t rb = rt_device_read(tvoice_st.adc_device, 0,
            (buffer + read_bytes), (size - read_bytes));

        if (rb == 0)
            break;

        read_bytes += rb;
    }

    return read_bytes;
}

static int tvoice_read_audio_adc(void)
{
    TVOICE_ELEM_PTR elem = NULL;
    rt_size_t read_bytes = 0;

    elem = (TVOICE_ELEM_PTR)co_list_pick(&tvoice_st.free);
    if(elem) 
    {  
        /* read data from sound device */
        if(tvoice_st.n_channel == 2) 
        {
            read_bytes = tvoice_sound_read(elem->buf_start, tvoice_st.elem_len);
        } 
        else if(tvoice_st.n_channel == 1) 
        {
            read_bytes = tvoice_sound_read(elem->buf_start, tvoice_st.elem_len);
        }

        if(read_bytes > 0)
        {
            elem->buf_len = read_bytes;
            co_list_pop_front(&tvoice_st.free);
            co_list_push_back(&tvoice_st.ready, (struct co_list_hdr *)&elem->hdr);            
        }
    } else {
        TVOICE_WPRT("sound no node\r\n");
    }

    return read_bytes;
}

static int tvoice_init_audio_adc(void)
{
    int sample_rate = 8000;
    int channel = 1;
    
    tvoice_st.adc_device = rt_device_find("mic");
    if (!tvoice_st.adc_device)
    {
        TVOICE_FATAL("mic not found\n");
        return -1;
    }
    
    tvoice_st.n_channel = channel;
    rt_device_control(tvoice_st.adc_device, CODEC_CMD_SAMPLERATE, (void *)&sample_rate); 
    rt_device_control(tvoice_st.adc_device, CODEC_CMD_SET_CHANNEL, (void *)&channel); 

    rt_device_open(tvoice_st.adc_device, RT_DEVICE_OFLAG_RDONLY);
    
    return 0;
}

static void tvoice_deinit_audio_adc(void)
{
    rt_device_close(tvoice_st.adc_device);
}

/*---------------------------------------------------------------------------*/
static void tvoice_transfer_main( beken_thread_arg_t data )
{
    OSStatus err;
    TVOICE_PRT("tvoice main\r\n");

    os_memset(&tvoice_st, 0, sizeof(TVOICE_DESC_ST));
    tvoice_pool_init(data);
    
    if(tvoice_init_audio_adc())
        goto tvoice_exit;

    while(1)
    {
        TVO_MSG_T msg;
        err = bk_rtos_pop_from_queue(&tvoice_msg_que, &msg, TVOICE_MSG_WAIT_TIME);
        if(kNoErr == err)
        {
        	switch(msg.data) 
        	{ 
                case TVO_EXIT:
                    goto tvoice_exit;
                    break;
                    
                default:
                    break;
            }
        }

        tvoice_read_audio_adc();
        tvoice_send_audio_adc();
    }

    tvoice_deinit_audio_adc();
    
tvoice_exit:
    TVOICE_PRT("video_transfer_main exit\r\n");

    if(tvoice_st.pool)
    {
        sdram_free(tvoice_st.pool);
        tvoice_st.pool = NULL;
    }
    os_memset(&tvoice_st, 0, sizeof(TVOICE_DESC_ST));
    
    bk_rtos_deinit_queue(&tvoice_msg_que);
    tvoice_msg_que = NULL;

    tvoice_thread_hdl = NULL;
    bk_rtos_delete_thread(NULL);
}

UINT32 tvoice_transfer_init(video_transfer_send_func send_func)
{
    int ret;

    TVOICE_PRT("tvoice init %p\r\n", send_func);
    if((!tvoice_thread_hdl) && (!tvoice_msg_que))
    {

    	ret = bk_rtos_init_queue(&tvoice_msg_que, 
    							"tvoice_queue",
    							sizeof(TVO_MSG_T),
    							TVO_QITEM_COUNT);
    	if (kNoErr != ret) 
    	{
    		TVOICE_FATAL("ceate queue failed\r\n");
            return kGeneralErr;
    	}
        
        ret = bk_rtos_create_thread(&tvoice_thread_hdl,
                                      4,
                                      "tvoice_intf",
                                      (beken_thread_function_t)tvoice_transfer_main,
                                      1024,
                                      (beken_thread_arg_t)send_func);
        if (ret != kNoErr)
        {
            bk_rtos_deinit_queue(&tvoice_msg_que);
            tvoice_msg_que = NULL;
            tvoice_thread_hdl = NULL;
            TVOICE_FATAL("Error: Failed to create tvoice: %d\r\n", ret);
            return kGeneralErr;
        }
    }

    return kNoErr;
}

UINT32 tvoice_transfer_deinit(void)
{
    TVOICE_PRT("tvoice deinit\r\n");
    
    tvoice_intfer_send_msg(TVO_EXIT);

    while(tvoice_thread_hdl)
        bk_rtos_delay_milliseconds(10);
}
#endif  // (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)


