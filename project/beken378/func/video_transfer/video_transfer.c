#include "include.h"
#include "arm_arch.h"

#if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)
#include "FreeRTOS.h"
#include "task.h"
#include "bk_rtos_pub.h"
#include "error.h"
#include "co_list.h"

#include "video_transfer.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#include "board.h"

#include "spidma_intf_pub.h"
#include "camera_intf_pub.h"
#include "rtthread.h"

#if CFG_USE_APP_DEMO_VIDEO_TRANSFER
#include "app_demo_tcp.h"
#include "app_demo_udp.h"
#endif

#if CFG_SUPPORT_TIANZHIHENG_DRONE
#include "app_lwip_tcp.h"
#include "app_lwip_udp.h"
#endif

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

#define TVIDEO_DEBUG                1
#include "uart_pub.h"
#if TVIDEO_DEBUG
#define TVIDEO_PRT                  os_printf
#define TVIDEO_WPRT                 warning_prf
#define TVIDEO_FATAL                fatal_prf
#else
#define TVIDEO_PRT                  null_prf
#define TVIDEO_WPRT                 null_prf
#define TVIDEO_FATAL                null_prf
#endif

#define TVIDEO_DROP_DATA_NONODE     0
#define TVIDEO_USE_HDR              1

#define TVIDEO_RXNODE_SIZE_UDP      1472
#define TVIDEO_RXNODE_SIZE_TCP      1460
#ifndef TVIDEO_RXNODE_SIZE
#define TVIDEO_RXNODE_SIZE          TVIDEO_RXNODE_SIZE_UDP
#endif

#define TVIDEO_DROP_DATA_FLAG       0x01

#if TVIDEO_DROP_DATA_NONODE
#define TVIDEO_POOL_LEN             (TVIDEO_RXNODE_SIZE * 38)  // 54KB
#else
#define TVIDEO_POOL_LEN             (TVIDEO_RXNODE_SIZE * 25)  // 7KB
#endif

#define TVIDEO_RXBUF_LEN            (TVIDEO_RXNODE_SIZE_UDP * 4)

UINT8 tvideo_rxbuf[TVIDEO_RXBUF_LEN];
TVIDEO_DESC_ST tvideo_st;

typedef struct tvideo_elem_st
{
    struct co_list_hdr hdr;
    void *buf_start;
    UINT32 buf_len;
} TVIDEO_ELEM_ST, *TVIDEO_ELEM_PTR;

typedef struct tvideo_pool_st
{
    //UINT8*  pool[TVIDEO_POOL_LEN];
    UINT8* pool;
    TVIDEO_ELEM_ST elem[TVIDEO_POOL_LEN / TVIDEO_RXNODE_SIZE];
    struct co_list free;
    struct co_list ready; 
    
    #if TVIDEO_DROP_DATA_NONODE
    struct co_list receiving;
    UINT32 drop_pkt_flag;    
    #endif

    UINT32 send_type;
    video_transfer_send_func send_func;
    video_transfer_start_cb start_cb;
    video_transfer_end_cb end_cb;

    #if(TVIDEO_USE_HDR && CFG_USE_CAMERA_INTF)
    UINT16 frame_id;
    UINT16 pkt_header_size;
    tvideo_add_pkt_header add_pkt_header;
    #endif
} TVIDEO_POOL_ST, *TVIDEO_POOL_PTR;

TVIDEO_POOL_ST tvideo_pool;

#define BUF_STATUS_START 0
#define BUF_STATUS_COPY  1
#define BUF_STATUS_DONE  2

struct video_buf_st
{
    UINT32 is_sof;
    UINT32 is_eof;
    UINT32 frame_id;
    UINT32 buf_status;
    UINT32 buf_len;
    UINT32 mempool_is_used;
    UINT32 is_request_data;
};

enum
{
	TV_INT_POLL          = 0, 
	TV_EXIT,
};

typedef struct tvideo_message 
{
	UINT32 data;
}TV_MSG_T;

#define TV_QITEM_COUNT      (60)
xTaskHandle  tvideo_thread_hdl = NULL;
beken_queue_t tvideo_msg_que = NULL;
struct video_buf_st video_buf = {0};

void tvideo_intfer_send_msg(UINT32 new_msg)
{
	OSStatus ret;
	TV_MSG_T msg;

    if(tvideo_msg_que) 
    {
    	msg.data = new_msg;
    	
    	ret = bk_rtos_push_to_queue(&tvideo_msg_que, &msg, BEKEN_NO_WAIT);
    	if(kNoErr != ret)
    	{
    		TVIDEO_FATAL("tvideo_intfer_send_msg failed\r\n");
    	}
    }
}

/*
* action : 1, 开始缓存图片，缓存成功，触发回调
* action : 0, 预缓存
*/
void tvideo_capture(UINT32 action)
{
    if(action == 1)
    {
        video_buf.is_request_data = 1;
    }
    else if(action == 0)
    {
        video_buf.mempool_is_used = 0;
    }
}

static void tvideo_pool_init(void* data)
{
    UINT32 i = 0;
    TVIDEO_SETUP_DESC_PTR setup = (TVIDEO_SETUP_DESC_PTR)((int)data);

    if(tvideo_pool.pool == NULL)
    {
        tvideo_pool.pool = rt_malloc(sizeof(UINT8)* TVIDEO_POOL_LEN);
        if(tvideo_pool.pool == NULL)
        {
            TVIDEO_FATAL("tvideo_pool alloc failed\r\n");
            ASSERT(1);
        }
    }
    
    os_memset(&tvideo_pool.pool[0], 0, sizeof(UINT8)*TVIDEO_POOL_LEN);
    
    #if TVIDEO_DROP_DATA_NONODE
    co_list_init(&tvideo_pool.receiving);    
    tvideo_pool.drop_pkt_flag = 0;
    #endif

    TVIDEO_PRT("video transfer send type:%d\r\n", setup->send_type);

    tvideo_pool.send_type = setup->send_type;
    tvideo_pool.send_func = setup->send_func;
    tvideo_pool.start_cb = setup->start_cb;
    tvideo_pool.end_cb = setup->end_cb;

    #if(TVIDEO_USE_HDR && CFG_USE_CAMERA_INTF)
    if((setup->pkt_header_size % 4) != 0)
    {
        TVIDEO_WPRT("pkt header-size should 4byte-aligned, but:%d\r\n", 
            setup->pkt_header_size);
    }
    
    tvideo_pool.frame_id = 0;
    tvideo_pool.add_pkt_header = setup->add_pkt_header;
    tvideo_pool.pkt_header_size = setup->pkt_header_size;

    video_buf.is_sof = 0;
    video_buf.is_eof = 0;
    video_buf.frame_id = 0;
    video_buf.buf_status = BUF_STATUS_START;
    video_buf.mempool_is_used = 0;
    video_buf.buf_len = 0;
    video_buf.is_request_data = 0;
    #endif
}

static void tvideo_rx_handler(void *curptr, UINT32 newlen, UINT32 is_eof, UINT32 frame_len)
{
    TVIDEO_ELEM_PTR elem = NULL;
    rt_uint32_t recved;

    if(!newlen)
        return;
        
    #if TVIDEO_DROP_DATA_NONODE
    // drop pkt has happened, so drop it, until spidma timeout handler.
    if(tvideo_pool.drop_pkt_flag & TVIDEO_DROP_DATA_FLAG)
        break;
    #endif

    if(video_buf.frame_id != tvideo_pool.frame_id)
    {
        video_buf.is_sof = 1;
        video_buf.frame_id = tvideo_pool.frame_id;
    }
    else
    {
        video_buf.is_sof = 0;
        video_buf.frame_id = tvideo_pool.frame_id;
    }
        
    if(is_eof)
    {
        video_buf.is_eof = 1;
    }
    else
    {
        video_buf.is_eof = 0;
    }
        
    if(newlen > tvideo_st.node_len)
        newlen = tvideo_st.node_len;
        
    if(video_buf.mempool_is_used == 1)
    {
        // video_buf.is_request_data = 0;
        switch(video_buf.buf_status)
        {
            case BUF_STATUS_START:
                video_buf.buf_len = 0;
                if(video_buf.is_sof == 1)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                        video_buf.buf_status =  BUF_STATUS_COPY;
                    }
                }
                else if(video_buf.is_sof == 0)
                {
                    return;
                }
                break;
            case BUF_STATUS_COPY:
                if(video_buf.is_eof == 1)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                    }
                    video_buf.buf_status = BUF_STATUS_DONE;
                    tvideo_pool.send_func(tvideo_pool.pool, 0, video_buf.buf_len, 0);
                }
                else if(video_buf.is_eof == 0)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                    }
                }
                break;
            case BUF_STATUS_DONE:
                return;
                break;
        }
    }
    else if(video_buf.mempool_is_used == 0)
    {
        switch(video_buf.buf_status)
        {
            case BUF_STATUS_START:
                video_buf.buf_len = 0;
                if(video_buf.is_sof == 1)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                    }
                    video_buf.buf_status = BUF_STATUS_COPY;
                    if(video_buf.is_request_data == 1)
                    {
                        video_buf.mempool_is_used = 1;   
                        video_buf.is_request_data = 0;
                    }
                    // video_buf.is_sof = 0;
                }
                else if(video_buf.is_sof == 0)
                {
                    //do not copy data;
                    return;
                }
                break;
            case BUF_STATUS_COPY:
                if(video_buf.is_request_data == 1)
                {
                    video_buf.mempool_is_used = 1;
                    video_buf.is_request_data = 0;
                }
                if(video_buf.is_eof == 1)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                    }
                    video_buf.buf_status = BUF_STATUS_START;
                }
                else if(video_buf.is_eof == 0)
                {
                    if(video_buf.buf_len + newlen < TVIDEO_POOL_LEN)
                    {
                        memcpy(tvideo_pool.pool + video_buf.buf_len, curptr, newlen);
                        video_buf.buf_len += newlen;
                    }
                }
                break;
            case BUF_STATUS_DONE:
                video_buf.buf_status = BUF_STATUS_START;
                break; 
        }
    }

}

static void tvideo_end_frame_handler(void)
{
    #if TVIDEO_DROP_DATA_NONODE
    // reset drop flag, new pkt can receive
    tvideo_pool.drop_pkt_flag &= (~TVIDEO_DROP_DATA_FLAG);
    if(!co_list_is_empty(&tvideo_pool.receiving)) {        
        co_list_concat(&tvideo_pool.ready, &tvideo_pool.receiving);  
    }
    #endif

    #if(TVIDEO_USE_HDR && CFG_USE_CAMERA_INTF)
    tvideo_pool.frame_id++;
    #endif
}

static void tvideo_config_desc(void)
{
    UINT32 node_len = TVIDEO_RXNODE_SIZE_TCP;

    if(tvideo_pool.send_type == TVIDEO_SND_UDP) 
    {
        #if(TVIDEO_USE_HDR && CFG_USE_CAMERA_INTF)
        node_len = TVIDEO_RXNODE_SIZE_UDP - tvideo_pool.pkt_header_size;    
        #else
        node_len = TVIDEO_RXNODE_SIZE_UDP;
        #endif
    }
    else if(tvideo_pool.send_type == TVIDEO_SND_TCP)
    {
        node_len = TVIDEO_RXNODE_SIZE_TCP; 
    }
    else if(tvideo_pool.send_type == TVIDEO_SND_INTF)
    {
        #if(TVIDEO_USE_HDR && CFG_USE_CAMERA_INTF)
        node_len = TVIDEO_RXNODE_SIZE_UDP - tvideo_pool.pkt_header_size;    
        #else
        node_len = TVIDEO_RXNODE_SIZE_UDP;
        #endif
    }
    else 
    {
        TVIDEO_WPRT("Err snd tpye in spidma\r\n"); 
    }
    
    tvideo_st.rxbuf = &tvideo_rxbuf[0];
    tvideo_st.rxbuf_len = node_len * 4;
    tvideo_st.node_len = node_len;
    tvideo_st.rx_read_len = 0;

    tvideo_st.node_full_handler = tvideo_rx_handler;
    tvideo_st.data_end_handler = tvideo_end_frame_handler;
}

// static void tvideo_poll_handler(void)
// {
//     UINT32 send_len;
//     TVIDEO_ELEM_PTR elem = NULL;

//     do
//     {
//         elem = (TVIDEO_ELEM_PTR)co_list_pick(&tvideo_pool.ready);        
//         if(elem) 
//         {
//             if(tvideo_pool.send_func) 
//             {
//                 //REG_WRITE((0x00802800+(18*4)), 0x02);
//                 send_len = tvideo_pool.send_func(elem->buf_start, elem->buf_len);
//                 //REG_WRITE((0x00802800+(18*4)), 0x00);
//                 if(send_len != elem->buf_len) 
//                 {
//                     break;
//                 }
//             }
            
//             co_list_pop_front(&tvideo_pool.ready);
//             co_list_push_back(&tvideo_pool.free, (struct co_list_hdr *)&elem->hdr);
//         }
//     }
//     while(elem);
// }

/*---------------------------------------------------------------------------*/
static void video_transfer_main( beken_thread_arg_t data )
{
    OSStatus err;
    TVIDEO_PRT("video_transfer_main entry\r\n");
    
    tvideo_pool_init(data);
    tvideo_config_desc();

    {
        #if CFG_USE_SPIDMA
        spidma_intfer_init(&tvideo_st);
        #elif CFG_USE_CAMERA_INTF
        camera_intfer_init(&tvideo_st);
        #endif
    }

    if(tvideo_pool.start_cb != NULL)
        tvideo_pool.start_cb();

}

static int is_start = 0;
TVIDEO_SETUP_DESC_ST video_transfer_setup_bak = {0};
UINT32 video_transfer_init(TVIDEO_SETUP_DESC_PTR setup_cfg)
{
    int ret;

    TVIDEO_PRT("video_transfer_init %d\r\n", setup_cfg->send_type);
    if(is_start == 0)
    {
        // bakup setup_cfg, because of that 'setup_cfg' may not static value.
        os_memcpy(&video_transfer_setup_bak, setup_cfg, sizeof(TVIDEO_SETUP_DESC_ST));
        video_transfer_main((beken_thread_arg_t)&video_transfer_setup_bak);
        is_start = 1;
    }

    return kNoErr;
}

UINT32 video_transfer_deinit(void)
{
    TVIDEO_PRT("video_transfer_deinit\r\n");

    is_start = 0;
    
    if(tvideo_pool.pool)
    {
        rt_free(tvideo_pool.pool);
        tvideo_pool.pool = NULL;
    }

    #if CFG_USE_SPIDMA
    spidma_intfer_deinit();
    #elif CFG_USE_CAMERA_INTF
    camera_intfer_deinit();
    #endif

    // bk_rtos_deinit_queue(&tvideo_msg_que);
    // tvideo_msg_que = NULL;

    // tvideo_thread_hdl = NULL;
    // bk_rtos_delete_thread(NULL);

    // while(tvideo_thread_hdl)
    //     bk_rtos_delay_milliseconds(10);
}
#endif  // (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)

