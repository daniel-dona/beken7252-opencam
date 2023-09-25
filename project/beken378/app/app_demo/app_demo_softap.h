#ifndef __APP_SOFTAP_DEMO_H__
#define __APP_SOFTAP_DEMO_H__

enum
{
    DAP_TIMER_POLL          = 0, 
    DAP_WIFI_DISCONECTED,    
    DAP_WIFI_CONECTED,      
    DAP_APP_CONECTED,
    DAP_APP_DISCONECTED,      
    DAP_EXIT,
    DAP_START_OTA,
};

void app_demo_softap_send_msg(u32 new_msg, u32 new_data);

typedef struct tvideo_hdr_st
{
    UINT8 id;
    UINT8 is_eof;
    UINT8 pkt_cnt; 
    UINT8 size;
#if 0
    UINT32 a_bc;
#endif
}HDR_ST, *HDR_PTR;


typedef struct tvideo_ota_st
{
    const char *http_url;
    int    http_port;
    UINT32 http_timeout;
}TV_OTA_ST, *TV_OTA_PTR;

#include "video_transfer.h"
void app_demo_add_pkt_header(TV_HDR_PARAM_PTR param);
int app_demo_softap_is_ota_doing(void);

#endif  // __APP_SOFTAP_DEMO_H__