#include "include.h"

#include "FreeRTOS.h"
#include "task.h"
#include "bk_rtos_pub.h"
#include "error.h"

#include "uart_pub.h"
#include "mem_pub.h"
#include "str_pub.h"

#include "app_video_intf.h"
#include "video_transfer.h"
#include "spidma_intf_pub.h"
#include "camera_intf_pub.h"

int app_video_intf_send_packet (UINT8 *data, UINT32 len)
{
    //os_printf("voide send:%p, %p\r\n", data, len);
	return len;
}

void app_video_intf_open (void)
{
    os_printf("voide open\r\n");
    #if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF) 
    TVIDEO_SETUP_DESC_ST setup;
    
    setup.send_type = TVIDEO_SND_INTF;
    setup.send_func = app_video_intf_send_packet;
    setup.start_cb = NULL;
    setup.end_cb = NULL;

    setup.pkt_header_size = 0;
    setup.add_pkt_header = NULL;
   
    video_transfer_init(&setup);
    #endif
}

void app_video_intf_close (void)
{
    os_printf("voide close\r\n");
    #if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)  
    video_transfer_deinit();  
    #endif
}

#if CFG_SUPPORT_RTT
void video(int argc, char **argv)
{
    if(0 == os_strcmp(argv[1], "open"))
    {
        app_video_intf_open();
    }
    else if(0 == os_strcmp(argv[1], "close"))
    {
        app_video_intf_close();
    }
    else
    {
        os_printf("video open/close\r\n");
    }
}
FINSH_FUNCTION_EXPORT_ALIAS(video, __cmd_video, video);
#endif  //  CFG_SUPPORT_RTT




