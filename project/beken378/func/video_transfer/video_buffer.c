#include "include.h"

#if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF) 
#include "FreeRTOS.h"
#include "task.h"
#include "bk_rtos_pub.h"
#include "error.h"

#include "uart_pub.h"
#include "mem_pub.h"
#include "str_pub.h"

#include "video_transfer.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

#define BUF_STA_DONE        0
#define BUF_STA_COPY        1
#define BUF_STA_FULL        2

typedef struct video_buffer_st
{
    UINT32 is_running;
    beken_semaphore_t aready_semaphore; 
    
    UINT8 *buf_base;  // handler in usr thread
    UINT32 buf_len;
    
    UINT32 is_sof;  // handler in add_pkt_header(isr)
    UINT32 is_eof;
    UINT32 frame_id;

    UINT8 *buf_ptr;
    UINT32 frame_len;
    UINT32 start_buf;
} VBUF_ST, *VBUF_PTR;

VBUF_ST g_vbuf = 
{
    0
};

static void video_buffer_add_pkt_header(TV_HDR_PARAM_PTR param)
{  
    UINT32 eof_flag = 0;
    if(g_vbuf.frame_id != param->frame_id)
    {
        eof_flag = 1;
        g_vbuf.frame_id = param->frame_id;
    }
    
    if(g_vbuf.buf_base == NULL)
        return;

    g_vbuf.is_sof = eof_flag;

    if(param->is_eof)
    {
        g_vbuf.is_eof = 1;       
    }
    else
    {
        g_vbuf.is_eof = 0;
    }
}

static int video_buffer_recv_video_data(UINT8 *data, UINT32 len)
{
    if(g_vbuf.buf_base)
    {
        UINT32 copy_len, left_len;

        //os_printf("b: %d,%d\r\n", g_vbuf.is_sof, g_vbuf.is_eof);
        
        if(g_vbuf.is_sof == 1)
        {
            g_vbuf.buf_ptr = g_vbuf.buf_base;
            g_vbuf.frame_len = 0;
            g_vbuf.start_buf = BUF_STA_COPY;
        }

        if(g_vbuf.start_buf == BUF_STA_COPY)
        {
            left_len = g_vbuf.buf_len - g_vbuf.frame_len;
            if(len <= left_len)
            {
                #if CFG_GENERAL_DMA
                gdma_memcpy(g_vbuf.buf_ptr, data, len);
                #else
                os_memcpy(g_vbuf.buf_ptr, data, len);
                #endif
                g_vbuf.frame_len += len;
                g_vbuf.buf_ptr += len;

                if(g_vbuf.is_eof)
                {
                    //os_printf("set ph\r\n");
                    g_vbuf.start_buf = BUF_STA_DONE;
                    // all frame data have received, wakeup usr thread
                    bk_rtos_set_semaphore(&g_vbuf.aready_semaphore);
                }
            }
            else
            {
                os_printf("vbuf full!\r\n");
                g_vbuf.start_buf = BUF_STA_FULL;
            } 
        }
    }

	return len;
}

void video_buffer_open (void)
{
    if(g_vbuf.is_running == 0) 
    {
        GLOBAL_INT_DECLARATION();
        TVIDEO_SETUP_DESC_ST setup;

        if(bk_rtos_init_semaphore(&g_vbuf.aready_semaphore, 1) != kNoErr)
        {
            os_printf("vbuf init semaph failed\r\n");
            return;
        }

        setup.send_type = TVIDEO_SND_BUFFER;
        setup.send_func = video_buffer_recv_video_data;
        setup.start_cb = NULL;
        setup.end_cb = NULL;

        setup.pkt_header_size = 0;
        setup.add_pkt_header = video_buffer_add_pkt_header;
       
        video_transfer_init(&setup);

        GLOBAL_INT_DISABLE();
        g_vbuf.buf_base = NULL;
        g_vbuf.buf_len = 0;

        g_vbuf.frame_len = 0;
        g_vbuf.buf_ptr = NULL;
        g_vbuf.start_buf = BUF_STA_DONE;
        
        g_vbuf.is_sof = 0;
        g_vbuf.is_eof = 0;
        g_vbuf.frame_id = 0xffffffff;
        
        g_vbuf.is_running = 1;
        GLOBAL_INT_RESTORE();

        os_printf("vbuf opened\r\n");

    }
}

void video_buffer_close (void)
{
    if(g_vbuf.is_running == 1)
    {
        os_printf("voide close\r\n");

        video_transfer_deinit();

        if(g_vbuf.buf_base)
        {
            // user all video_buffer_read_frame and blocked, so wakeup it
            bk_rtos_set_semaphore(&g_vbuf.aready_semaphore);
            
            // wait until clear the buf flag
            while(g_vbuf.buf_base);
        }

        bk_rtos_deinit_semaphore(&g_vbuf.aready_semaphore);

        GLOBAL_INT_DECLARATION();

        GLOBAL_INT_DISABLE();
        g_vbuf.aready_semaphore = NULL;
        g_vbuf.is_running = 0;
        GLOBAL_INT_RESTORE();
    }

}

UINT32 video_buffer_read_frame(UINT8 *buf, UINT32 buf_len)
{
    UINT32 frame_len = 0;
    if((buf == NULL) || (buf_len == 0))
        return 0;
    
    if(g_vbuf.is_running)
    {
        if(g_vbuf.buf_base == NULL)
        {
            int ret;
            UINT32 timeout;

            // try to get semaphore, clear send by the previous frame 
            while(bk_rtos_get_semaphore(&g_vbuf.aready_semaphore, 0) == kNoErr);
            
            g_vbuf.buf_base = buf;
            g_vbuf.buf_len = buf_len;
            timeout = BEKEN_WAIT_FOREVER;

            ret = bk_rtos_get_semaphore(&g_vbuf.aready_semaphore, timeout);
            if(ret == kNoErr) 
            {
                frame_len = g_vbuf.frame_len;
                
            }
            else 
            {
                os_printf("read frame time out\r\n");
            }

            g_vbuf.buf_base = NULL;
            g_vbuf.buf_len = 0;
        }
    }

    return frame_len;
}

#endif //#if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF) 

