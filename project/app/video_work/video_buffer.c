#include "include.h"

#if (CFG_USE_APP_DEMO_VIDEO_TRANSFER)
#include "video_transfer_config.h"

// this moudule only support camera in chip, with sccb interface.
#if ((CFG_USE_CAMERA_INTF) && (APP_DEMO_CFG_USE_VIDEO_BUFFER))
//#include "FreeRTOS.h"
//#include "task.h"
#include "rtos_pub.h"
#include "error.h"

#include "uart_pub.h"
#include "mem_pub.h"
#include "str_pub.h"

#include "video_transfer.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

typedef struct vbuf_hdr_st
{
    UINT8 id;
    UINT8 is_eof;
    UINT8 pkt_cnt;
    UINT8 pkt_seq;
} VB_HDR_ST, *VB_HDR_PTR;

#define BUF_STA_DONE        0
#define BUF_STA_COPY        1
#define BUF_STA_FULL        2

typedef struct video_buffer_st
{
    beken_semaphore_t aready_semaphore;

    UINT8 *buf_base;  // handler in usr thread
    UINT32 buf_len;

    UINT32 frame_id;
    UINT32 frame_pkt_cnt;

    UINT8 *buf_ptr;
    UINT32 frame_len;
    UINT32 start_buf;
} VBUF_ST, *VBUF_PTR;

VBUF_PTR g_vbuf = NULL;
static UINT32 g_pkt_seq = 0;

static void video_buffer_add_pkt_header(TV_HDR_PARAM_PTR param)
{
    VB_HDR_PTR elem_tvhdr = (VB_HDR_PTR)param->ptk_ptr;

    g_pkt_seq++;
    elem_tvhdr->id = (UINT8)param->frame_id;
    elem_tvhdr->is_eof = param->is_eof;
    elem_tvhdr->pkt_cnt = param->frame_len;
    elem_tvhdr->pkt_seq = g_pkt_seq;

    //os_printf("i:%d,%d\r\n", param->frame_id, g_pkt_seq);

    if (param->is_eof)
    {
        //os_printf("eof\r\n");
        g_pkt_seq = 0;
    }
}

static int video_buffer_recv_video_data(UINT8 *data, UINT32 len)
{
    if (g_vbuf->buf_base)
    {
        VB_HDR_PTR hdr = (VB_HDR_PTR)data;
        UINT32 org_len, left_len;
        GLOBAL_INT_DECLARATION();

        if (len < sizeof(VB_HDR_ST))
        {
            os_printf("unknow err!\r\n");
            return len;
        }

        org_len = len - sizeof(VB_HDR_ST);
        data = data + sizeof(VB_HDR_ST);

        if ((hdr->id != g_vbuf->frame_id) && (hdr->pkt_seq == 1))
        {
            // start of frame;
            GLOBAL_INT_DISABLE();
            g_vbuf->frame_id = hdr->id;
            g_vbuf->frame_len = 0;
            g_vbuf->frame_pkt_cnt = 0;
            g_vbuf->buf_ptr = g_vbuf->buf_base;
            g_vbuf->start_buf = BUF_STA_COPY;
            GLOBAL_INT_RESTORE();
            //os_printf("sof:%d\r\n", g_vbuf->frame_id);
        }

        //os_printf("%d-%d: %d-%d: %d\r\n", hdr->id, g_vbuf->frame_id,
        //    hdr->pkt_seq, (g_vbuf->frame_pkt_cnt + 1), g_vbuf->start_buf);

        if ((hdr->id == g_vbuf->frame_id)
                && ((g_vbuf->frame_pkt_cnt + 1) == hdr->pkt_seq)
                && (g_vbuf->start_buf == BUF_STA_COPY))
        {
            left_len = g_vbuf->buf_len - g_vbuf->frame_len;
            if (org_len <= left_len)
            {
                #if CFG_GENERAL_DMA
                gdma_memcpy(g_vbuf->buf_ptr, data, org_len);
                #else
                os_memcpy(g_vbuf->buf_ptr, data, org_len);
                #endif

                GLOBAL_INT_DISABLE();
                g_vbuf->frame_len += org_len;
                g_vbuf->buf_ptr += org_len;
                g_vbuf->frame_pkt_cnt += 1;
                GLOBAL_INT_RESTORE();

                if (hdr->is_eof == 1)
                {
                    UINT8 *sof_ptr, *eof_ptr, *crc_ptr;
                    UINT32 p_len, right_image = 0;

                    sof_ptr = g_vbuf->buf_base;
                    eof_ptr = g_vbuf->buf_base + (g_vbuf->frame_len - 7);
                    crc_ptr = eof_ptr + 3;

                    if (((sof_ptr[0] == 0xff) && (sof_ptr[1] == 0xd8)) &&
                            ((eof_ptr[0] == 0xff) && (eof_ptr[1] == 0xd9)))
                    {
                        p_len = crc_ptr[0] + (crc_ptr[1] << 8)
                                + (crc_ptr[2] << 16) + (crc_ptr[3] << 24);

                        //os_printf("vb,len:%d - %d\r\n", p_len, (g_vbuf->frame_len - 5));
                        if (p_len == (g_vbuf->frame_len - 5))
                        {
                            right_image = 1;
                        }
                    }

                    if (right_image)
                    {
                        //os_printf("set ph\r\n");
                        GLOBAL_INT_DISABLE();
                        g_vbuf->start_buf = BUF_STA_DONE;
                        GLOBAL_INT_RESTORE();

                        // all frame data have received, wakeup usr thread
                        rtos_set_semaphore(&g_vbuf->aready_semaphore);
                    }

                }
            }
            else
            {
                os_printf("vbuf full!\r\n");
                GLOBAL_INT_DISABLE();
                g_vbuf->start_buf = BUF_STA_FULL;
                GLOBAL_INT_RESTORE();

            }
        }

        return len;
    }
    else
    {
        // keep data
        //os_printf("kp\r\n");
        return len;
    }
}

int video_buffer_open(void){
    if (g_vbuf == NULL)
    {
        int ret;
        GLOBAL_INT_DECLARATION();
        TVIDEO_SETUP_DESC_ST setup;

        g_vbuf = (VBUF_PTR)os_malloc(sizeof(VBUF_ST));
        if (g_vbuf == NULL)
        {
            os_printf("Unable to reserve video buffer memory.\r\n");
            return 0;
        }

        if (rtos_init_semaphore(&g_vbuf->aready_semaphore, 1) != kNoErr)
        {
            os_printf("Semaphore 'aready' failed.\r\n");
            os_free(g_vbuf);
            g_vbuf = NULL;
            return 0;
        }

        GLOBAL_INT_DISABLE();
        g_vbuf->buf_base = NULL;
        g_vbuf->buf_len = 0;

        g_vbuf->frame_len = 0;
        g_vbuf->buf_ptr = NULL;
        g_vbuf->start_buf = BUF_STA_DONE;

        g_vbuf->frame_id = 0xffff;
        g_vbuf->frame_pkt_cnt = 0;
        GLOBAL_INT_RESTORE();

        setup.open_type = TVIDEO_OPEN_SCCB;
        setup.send_type = TVIDEO_SND_INTF;
        setup.send_func = video_buffer_recv_video_data;
        setup.start_cb = NULL;
        setup.end_cb = NULL;

        setup.pkt_header_size = sizeof(VB_HDR_ST);
        setup.add_pkt_header = video_buffer_add_pkt_header;

        ret = video_transfer_init(&setup);
        if (ret != 0)
        {
            os_printf("Video transfer initialization failed\r\n");
            rtos_deinit_semaphore(&g_vbuf->aready_semaphore);
            os_free(g_vbuf);
            g_vbuf = NULL;
            return ret;
        }

        os_printf("Video buffer opened.\r\n");
    }

    return 0;
}

int video_buffer_close(void)
{
    if (g_vbuf)
    {
        int ret;
        os_printf("Closing video buffer.\r\n");

        ret = video_transfer_deinit();
        if (ret != 0)
        {
            os_printf("Video buffer close failed.\r\n");
            return ret;
        }

        if (g_vbuf->buf_base)
        {
            // user all video_buffer_read_frame and blocked, so wakeup it
            rtos_set_semaphore(&g_vbuf->aready_semaphore);

            // wait until clear the buf flag
            while (g_vbuf->buf_base);
        }

        rtos_deinit_semaphore(&g_vbuf->aready_semaphore);

        GLOBAL_INT_DECLARATION();

        GLOBAL_INT_DISABLE();
        g_vbuf->aready_semaphore = NULL;
        GLOBAL_INT_RESTORE();

        os_free(g_vbuf);
        g_vbuf = NULL;
    }

    return 0;
}

UINT32 video_buffer_read_frame(UINT8 *buf, UINT32 buf_len)
{
    UINT32 frame_len = 0;
    GLOBAL_INT_DECLARATION();

    if ((buf == NULL) || (buf_len == 0))
    {
        return 0;
    }

    if (g_vbuf && (g_vbuf->buf_base == NULL))
    {
        int ret;
        UINT32 timeout;

        // try to get semaphore, clear send by the previous frame
        while (rtos_get_semaphore(&g_vbuf->aready_semaphore, 0) == kNoErr);

        GLOBAL_INT_DISABLE();
        g_vbuf->buf_base = buf;
        g_vbuf->buf_len = buf_len;
        GLOBAL_INT_RESTORE();

        timeout = BEKEN_WAIT_FOREVER;

        ret = rtos_get_semaphore(&g_vbuf->aready_semaphore, timeout);
        if (ret == kNoErr)
        {
            frame_len = g_vbuf->frame_len;
        }
        else
        {
            os_printf("read frame time out\r\n");
        }

        GLOBAL_INT_DISABLE();
        g_vbuf->buf_base = NULL;
        g_vbuf->buf_len = 0;
        GLOBAL_INT_RESTORE();
    }

    return frame_len;
}

#define VIDEO_BUFFER_CMD        0

#if VIDEO_BUFFER_CMD
#include "string.h"
#include "stdlib.h"

void video_buffer(int argc, char **argv)
{
    if (strcmp(argv[1], "open") == 0)
    {
        video_buffer_open();
    }
    else if (strcmp(argv[1], "read") == 0)

    {
        uint8_t *mybuf;
        uint32_t my_len;

        my_len = atoi(argv[2]);
        mybuf = os_malloc(my_len);

        if (mybuf == NULL)
        {
            os_printf("vbuf test no buff\r\n");
            return;
        }

        my_len = video_buffer_read_frame(mybuf, my_len);
        os_printf("frame_len: %d\r\n", my_len);

        if (1)
        {
            for (int i = 0; i < my_len; i++)
            {
                os_printf("%02x,", mybuf[i]);
                if ((i + 1) % 32 == 0)
                {
                    os_printf("\r\n");
                }
            }
        }
        os_free(mybuf);
    }
    else if (strcmp(argv[1], "close") == 0)
    {
        video_buffer_close();
    }
    else
    {
        os_printf("vbuf open/read len/close/\r\n");
    }
}

void video_buffer_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    video_buffer(argc, argv);
}
#if CFG_SUPPORT_RTT
FINSH_FUNCTION_EXPORT_ALIAS(video_buffer, __cmd_video_buffer, video buffer);
#endif
#else // VIDEO_BUFFER_CMD
void video_buffer_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{

}
#endif // VIDEO_BUFFER_CMD

#endif // ((CFG_USE_CAMERA_INTF) && (APP_DEMO_CFG_USE_VIDEO_BUFFER))
#endif //#if (CFG_USE_APP_DEMO_VIDEO_TRANSFER)

