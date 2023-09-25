#include "include.h"
#include "app_demo_config.h"

#if CFG_USE_APP_DEMO_VIDEO_TRANSFER
#include "common.h"
#include "uart_pub.h"
#include "mem_pub.h"

#include "task.h"
#include "bk_rtos_pub.h"
#include "error.h"

#include "wlan_ui_pub.h"
#include "ieee802_11_demo.h"
#include "rw_pub.h"
#include "app_demo_softap.h"

#include "param_config.h"
#include "drv_model_pub.h"
#include "flash_pub.h"

#include <lwip/inet.h>
#include "ieee802_11_defs.h"
#include "BkDriverFlash.h"

#include "app_demo_tcp.h"
#include "app_demo_udp.h"

#if (CFG_SUPPORT_RTT)
#include <finsh.h>
#endif

#define APP_DEMO_SOFTAP_DEBUG        1
#if APP_DEMO_SOFTAP_DEBUG
#define APP_DEMO_SOFTAP_PRT      warning_prf
#define APP_DEMO_SOFTAP_WARN     warning_prf
#define APP_DEMO_SOFTAP_FATAL    fatal_prf
#else
#define APP_DEMO_SOFTAP_PRT      null_prf
#define APP_DEMO_SOFTAP_WARN     null_prf
#define APP_DEMO_SOFTAP_FATAL    null_prf
#endif

#define APP_DEMO_SOFTAP_QITEM_COUNT           4
#define APP_DEMO_SOFTAP_TIMER_INTVAL          500  // ms

typedef enum
{
    APS_WIFI_DISCONECTED          = 0, 
    APS_WIFI_CONECTED,
    APS_APP_DISCONECTED,
    APS_APP_CONECTED, 
    APS_APP_OTA_DOING,
    APS_APP_OTA_DONE,
} DEMO_SOFTAP_STATUS;

typedef struct temp_message 
{
    u32 dmsg;
    u32 data;
}DRONE_MSG_T;

typedef struct app_demo_softap_st
{
    xTaskHandle thread_hdl;
    beken_queue_t msg_que; 
    u32 status;
}APP_DEMO_SOFTAP_ST, *APP_DEMO_SOFTAP_PTR;

enum{
    ARCH_CONFIG_GENERAL,
    ARCH_CONFIG_AP,
    ARCH_CONFIG_STA,
    ARCH_CONFIG_MISC,
};

enum{
    GENERAL_TYPE_ROLE,
    GENERAL_TYPE_DHCP,
    GENERAL_TYPE_IP,
    GENERAL_TYPE_MASK,
    GENERAL_TYPE_GW,
};

enum{    
    AP_TYPE_BSSID,
    AP_TYPE_SSID,
    AP_TYPE_CHANNEL,
    AP_TYPE_MODE,
    AP_TYPE_PASSWD,
};

typedef struct head_param{
    u32 type;
    u32 len;
}head_param_t;

APP_DEMO_SOFTAP_PTR g_demo_softap = NULL;

static int app_demo_softap_get_general_paramters(general_param_t *general);
static int app_demo_softap_get_ap_paramters(ap_param_t *ap);

void app_demo_softap_send_msg(u32 new_msg, u32 new_data)
{
    OSStatus ret;
    DRONE_MSG_T msg;

    if(g_demo_softap && g_demo_softap->msg_que) 
    {
        msg.dmsg = new_msg;
        
        ret = bk_rtos_push_to_queue(&g_demo_softap->msg_que, &msg, BEKEN_NO_WAIT);
        if(kNoErr != ret)
        {
            os_printf("app_demo_softap_send_msg failed\r\n");
        }
    }
}

static int app_demo_softap_rw_event_func(enum rw_evt_type evt_type, void *data)
{
    struct rw_evt_payload *evt_payload = (struct rw_evt_payload *)data;

    if(evt_type == RW_EVT_AP_CONNECTED) {
        APP_DEMO_SOFTAP_PRT("RW_EVT_AP_CONNECTED-(mac="MACSTR")\r\n",  MAC2STR(evt_payload->mac));
        //if(g_demo_softap->status == DS_WIFI_DISCONECTED)
            app_demo_softap_send_msg(DAP_WIFI_CONECTED, 0);          
    } else if(evt_type == RW_EVT_AP_DISCONNECTED) {
        APP_DEMO_SOFTAP_PRT("RW_EVT_AP_DISCONNECTED-(mac="MACSTR")\r\n",  MAC2STR(evt_payload->mac));
        //if(g_demo_softap->status != DS_WIFI_DISCONECTED)
            app_demo_softap_send_msg(DAP_WIFI_DISCONECTED, 0);  
    }

    return 0;
}

#if CFG_SUPPORT_HTTP_OTA
#include "utils_httpc.h"
#include "wdt_pub.h"
static void app_demo_softap_start_http_ota(int ota_param)
{   
    #define HTTP_RESP_CONTENT_LEN   (256)
    #define HTTPS_DEF_URL               "url://192.168.1.100/beken7231_fullcode.rbl"
    #define HTTPS_DEF_PORT              (80)
    #define HTTPS_DEF_TIMEOUT_MS        (5000)

    TV_OTA_PTR ota = (TV_OTA_PTR)ota_param;

    int ret;
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;
    char http_content[HTTP_RESP_CONTENT_LEN];
    const char *http_url;
    int    http_port;
    UINT32 http_timeout;

    os_memset(&httpclient, 0, sizeof(httpclient_t));
    os_memset(&httpclient_data, 0, sizeof(httpclient_data));
    os_memset(&http_content, 0, sizeof(HTTP_RESP_CONTENT_LEN));
    
    httpclient.header = "Accept: text/xml,text/html,\r\n"; 
    httpclient_data.response_buf = http_content; 
    httpclient_data.response_content_len = HTTP_RESP_CONTENT_LEN;

    if((ota == NULL) || (ota->http_url == NULL))
    {
        http_url = HTTPS_DEF_URL;
        http_port = HTTPS_DEF_PORT;
        http_timeout = HTTPS_DEF_TIMEOUT_MS;
    }
    else
    {
        http_url = ota->http_url;
        http_port = ota->http_port;
        http_timeout = ota->http_timeout;
    }

    APP_DEMO_SOFTAP_WARN("do http ota--url:%s\r\n", http_url);
    APP_DEMO_SOFTAP_WARN("port:%d, timeout:%d\r\n", http_port, http_timeout);
    
    ret = httpclient_common(&httpclient,
        http_url,  
        http_port,  
        NULL,
        HTTPCLIENT_GET, 
        http_timeout,
        &httpclient_data); 
    
    if (0 != ret) 
    { 
        APP_DEMO_SOFTAP_WARN("http_ota failed\r\n");
    } 
    else 
    {  
        APP_DEMO_SOFTAP_WARN("http_ota sucessed\r\n");
        
        GLOBAL_INT_DECLARATION();
        ret = 100;
        os_printf("wdt reboot\r\n");
        sddev_control(WDT_DEV_NAME, WCMD_SET_PERIOD, &ret);
        sddev_control(WDT_DEV_NAME, WCMD_POWER_UP, NULL);

        GLOBAL_INT_DISABLE();
        while(1);
        GLOBAL_INT_RESTORE(); // never come here
    }
}
#endif //#if CFG_SUPPORT_HTTP_OTA

static int app_demo_softap_setup(void)
{   
    general_param_t general;
    ap_param_t ap_info;
    network_InitTypeDef_st wNetConfig;
    int len;
    u8 *mac;

    //efuse_disable_spi_download_mode();
    //efuse_disable_jtag_debug_mode();
    
    os_memset(&general, 0, sizeof(general_param_t));
    os_memset(&ap_info, 0, sizeof(ap_param_t)); 
    os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));  
    
    if(app_demo_softap_get_general_paramters(&general) == -1) {
        APP_DEMO_SOFTAP_PRT("no flash configuration, use default\r\n");
        general.role = 1,
        general.dhcp_enable = 1,

        os_strcpy((char *)wNetConfig.local_ip_addr, APP_DEMO_SOFTAP_DEF_NET_IP);
        os_strcpy((char *)wNetConfig.net_mask, APP_DEMO_SOFTAP_DEF_NET_MASK);
        os_strcpy((char *)wNetConfig.gateway_ip_addr, APP_DEMO_SOFTAP_DEF_NET_GW);
        os_strcpy((char *)wNetConfig.dns_server_ip_addr, APP_DEMO_SOFTAP_DEF_NET_GW);
    } else {
        APP_DEMO_SOFTAP_PRT("read flash configuration:\r\n");
        APP_DEMO_SOFTAP_PRT("general: %d,%d,%08x,%08x,%08x\r\n",
            general.role,
            general.dhcp_enable,
            general.ip_addr,
            general.ip_mask,
            general.ip_gw);

        general.ip_addr = htonl(general.ip_addr);
        general.ip_mask = htonl(general.ip_mask);
        general.ip_gw = htonl(general.ip_gw);

        os_strcpy((char *)wNetConfig.local_ip_addr, inet_ntoa(general.ip_addr));
        os_strcpy((char *)wNetConfig.net_mask, inet_ntoa(general.ip_mask));
        os_strcpy((char *)wNetConfig.gateway_ip_addr, inet_ntoa(general.ip_gw));
        os_strcpy((char *)wNetConfig.dns_server_ip_addr, inet_ntoa(general.ip_gw));
    }

    if(app_demo_softap_get_ap_paramters(&ap_info) == -1) {
        APP_DEMO_SOFTAP_PRT("no flash configuration, use default\r\n");
        mac = (u8*)&ap_info.bssid.array;
        wifi_get_mac_address(mac, CONFIG_ROLE_AP);
        ap_info.chann = APP_DEMO_SOFTAP_DEF_CHANNEL;
        ap_info.cipher_suite = 0;
        os_memcpy(ap_info.ssid.array, APP_DEMO_SOFTAP_DEF_SSID, os_strlen(APP_DEMO_SOFTAP_DEF_SSID));
        ap_info.key_len = 0;
        os_memset(&ap_info.key, 0, 65);   
    } else {
        // first load mac addr from flash
        wifi_get_mac_address(NULL, 0);
        mac = (u8*)&ap_info.bssid.array;
        APP_DEMO_SOFTAP_PRT("ap: %02x:%02x:%02x:%02x:%02x:%02x,%d,%d,%s,%d,%s\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            ap_info.chann,
            ap_info.cipher_suite,
            ap_info.ssid.array,
            ap_info.key_len,
            ap_info.key);
            
        wifi_set_mac_address(mac);
    }
    bk_wlan_ap_set_default_channel(ap_info.chann);

    len = os_strlen(ap_info.ssid.array);
    if(SSID_MAX_LEN < len)
    {
        APP_DEMO_SOFTAP_FATAL("ssid name more than 32 Bytes\r\n");
        return -1;
    }

    os_strcpy((char *)wNetConfig.wifi_ssid, ap_info.ssid.array);
    os_strcpy((char *)wNetConfig.wifi_key, ap_info.key);
    
    wNetConfig.wifi_mode = SOFT_AP;
    wNetConfig.dhcp_mode = DHCP_SERVER;
    wNetConfig.wifi_retry_interval = 100;
    
    APP_DEMO_SOFTAP_PRT("set ip info: %s,%s,%s\r\n",
            wNetConfig.local_ip_addr,
            wNetConfig.net_mask,
            wNetConfig.dns_server_ip_addr);
    
    APP_DEMO_SOFTAP_PRT("ssid:%s  key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
    bk_wlan_start(&wNetConfig);

    return 0;    
}

static void app_demo_softap_main( beken_thread_arg_t data )
{
    OSStatus err;
    int ret = 0;
    u32 status;

    //app_led_init();
    //app_led_send_msg(POWER_ON);

    rw_evt_set_callback(RW_EVT_AP_CONNECTED, app_demo_softap_rw_event_func);
    rw_evt_set_callback(RW_EVT_AP_DISCONNECTED, app_demo_softap_rw_event_func);

    app_demo_softap_setup();

    g_demo_softap->status = APS_WIFI_DISCONECTED;

    while(1) 
    {
        DRONE_MSG_T msg;
        status = g_demo_softap->status;
        
        err = bk_rtos_pop_from_queue(&g_demo_softap->msg_que, &msg, BEKEN_WAIT_FOREVER);
        if(kNoErr == err)
        {
            switch(msg.dmsg) 
            {
                case DAP_WIFI_DISCONECTED:
                    if(g_demo_softap->status != APS_WIFI_DISCONECTED) {
                        g_demo_softap->status = APS_WIFI_DISCONECTED;
                        //app_led_send_msg(LED_DISCONNECT);
                        #if APP_DEMO_CFG_USE_UDP
                        app_demo_udp_deinit();
                        #endif

                        #if APP_DEMO_CFG_USE_TCP
                        app_demo_tcp_deinit();
                        #endif
                        APP_DEMO_SOFTAP_PRT("wifi disconnected!\r\n");
                    }
                    break;                    

                case DAP_WIFI_CONECTED:
                    if(g_demo_softap->status == APS_WIFI_DISCONECTED) {
                        g_demo_softap->status = APS_WIFI_CONECTED;
                        //app_led_send_msg(LED_CONNECT);
                        #if APP_DEMO_CFG_USE_UDP
                        app_demo_udp_init();
                        #endif

                        #if APP_DEMO_CFG_USE_TCP
                        app_demo_tcp_init();
                        #endif
                        APP_DEMO_SOFTAP_PRT("wifi connected!\r\n");
                    }
                    break;

                case DAP_APP_CONECTED:
                    APP_DEMO_SOFTAP_PRT("app connected!\r\n");
                    g_demo_softap->status = APS_APP_CONECTED;
                    break;

                case DAP_APP_DISCONECTED:
                    APP_DEMO_SOFTAP_PRT("app disconnected!\r\n");
                    g_demo_softap->status = APS_APP_DISCONECTED;
                    break;                    

                case DAP_EXIT:
                    goto app_demo_softap_exit;
                    break;
                    
                #if CFG_SUPPORT_HTTP_OTA
                case DAP_START_OTA:
                    {
                        g_demo_softap->status = APS_APP_OTA_DOING;
                        app_demo_softap_start_http_ota(msg.data);
                        g_demo_softap->status = APS_APP_OTA_DONE;
                        g_demo_softap->status = status;
                    }
                    break;
                #endif
                    
                default:
                    break;
            }
        } 
    }

app_demo_softap_exit:

    bk_rtos_deinit_queue(&g_demo_softap->msg_que);

    os_free(g_demo_softap);
    g_demo_softap = NULL;
    
    bk_rtos_delete_thread(NULL);
}

int app_demo_softap_is_ota_doing(void)
{
    if(g_demo_softap == NULL)
        return 0;
    
    return (g_demo_softap->status == APS_APP_OTA_DOING)? 1 : 0;
}

void video_demo( void )
{
    int ret;

    APP_DEMO_SOFTAP_PRT("app_demo_softap_init\r\n");
#if 1
    if(!g_demo_softap) 
    {
        g_demo_softap = os_malloc(sizeof(APP_DEMO_SOFTAP_ST));
        if(!g_demo_softap) {
            APP_DEMO_SOFTAP_FATAL("app_demo_softap_init malloc failed\r\n");
            return;
        }

        ret = bk_rtos_init_queue(&g_demo_softap->msg_que, 
                                "app_demo_softap_queue",
                                sizeof(DRONE_MSG_T),
                                APP_DEMO_SOFTAP_QITEM_COUNT);
        if (kNoErr != ret) 
        {
            APP_DEMO_SOFTAP_FATAL("app_demo_softap_init ceate queue failed\r\n");
            os_free(g_demo_softap);
            g_demo_softap = NULL;
            return;
        }   
        
        ret = bk_rtos_create_thread(&g_demo_softap->thread_hdl,
                                      BEKEN_DEFAULT_WORKER_PRIORITY,
                                      "app_demo_softap",
                                      (beken_thread_function_t)app_demo_softap_main,
                                      2048,
                                      (beken_thread_arg_t)NULL);
        if (ret != kNoErr)
        {
            APP_DEMO_SOFTAP_FATAL("Error: Failed to create spidma_intfer: %d\r\n", ret);

            bk_rtos_deinit_queue(&g_demo_softap->msg_que);
            os_free(g_demo_softap);
            g_demo_softap = NULL;
            return;
        }

    }
#endif
    bk_rtos_delete_thread(NULL);

}
#if ((CFG_SUPPORT_RTT)&&(CFG_USE_APP_DEMO_VIDEO_TRANSFER))
MSH_CMD_EXPORT(video_demo,video demo);
#endif

static UINT32 search_opt_tab(UINT32 start_addr)
{
    UINT32 ret = 0, status;
    DD_HANDLE flash_handle;
    head_param_t head;
    #define BK_TLV_HEADER           (0x00564c54)   // ASIC TLV

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), start_addr);
    if(BK_TLV_HEADER == head.type){
        ret = 1;
    } else {
        ret = 0;
        APP_DEMO_SOFTAP_FATAL("BK_TLV_HEADER not found\r\n");
    }
    ddev_close(flash_handle);

    return ret;
}

static UINT32 search_by_type(UINT32 type, UINT32 start_addr)
{
    UINT32 status, addr, end_addr;
    DD_HANDLE flash_handle;
    head_param_t head;

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), start_addr);
    addr = start_addr + sizeof(head_param_t);
    end_addr = addr + head.len;
    while(addr < end_addr){
        ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
        if(type == head.type){
            break;
        }else{
            addr += sizeof(head_param_t);
            addr += head.len;
        }
    }

    if(addr >= end_addr){
        addr = 0;
    }
    ddev_close(flash_handle);
    
    return addr;
}

static int app_demo_softap_get_general_paramters(general_param_t *general)
{
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    head_param_t head;

    bk_logic_partition_t *pt = bk_flash_get_info(BK_PARTITION_USR_CONFIG);

    if(search_opt_tab(pt->partition_start_addr) == 0) {
        return -1;
    }

    if(!general){
        APP_DEMO_SOFTAP_FATAL("get_general_paramters is null\r\n");
        return -1;
    }
    
    addr_start = search_by_type(ARCH_CONFIG_GENERAL, pt->partition_start_addr);
    if(!addr_start){
        APP_DEMO_SOFTAP_FATAL("SEARCH GENERAL CLASS FAIL\r\n");
        return -1;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

    addr = search_by_type(GENERAL_TYPE_ROLE, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH ROLE FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&general->role, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(GENERAL_TYPE_DHCP, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH DHCP FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&general->dhcp_enable, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(GENERAL_TYPE_IP, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH IP FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&general->ip_addr, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(GENERAL_TYPE_MASK, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH MASK FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&general->ip_mask, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(GENERAL_TYPE_GW, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH GW FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&general->ip_gw, head.len, addr+sizeof(head_param_t));

    ddev_close(flash_handle);
    return 0;
}

static int app_demo_softap_get_ap_paramters(ap_param_t *ap)
{
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    head_param_t head;
    bk_logic_partition_t *pt = bk_flash_get_info(BK_PARTITION_USR_CONFIG);

    if(search_opt_tab(pt->partition_start_addr) == 0) {
        return -1;
    }

    if(!ap) {
        APP_DEMO_SOFTAP_FATAL("get_ap_paramters is null\r\n");
        return -1;
    }

    addr_start = search_by_type(ARCH_CONFIG_AP, pt->partition_start_addr);
    if(!addr_start){
        APP_DEMO_SOFTAP_FATAL("SEARCH AP CLASS FAIL\r\n");
        return -1;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

    addr = search_by_type(AP_TYPE_BSSID, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH BSSID FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&ap->bssid, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(AP_TYPE_SSID, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH SSID FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ap->ssid.length = head.len;
    ddev_read(flash_handle, (char *)ap->ssid.array, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(AP_TYPE_CHANNEL, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH CHANNEL FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&ap->chann, head.len, addr+sizeof(head_param_t));

    addr = search_by_type(AP_TYPE_MODE, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH MODE FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&ap->cipher_suite, head.len, addr+sizeof(head_param_t));

#if 1
    addr = search_by_type(AP_TYPE_PASSWD, addr_start);
    if(!addr){
        APP_DEMO_SOFTAP_FATAL("SEARCH PASSWORD FAIL\r\n");
        return 0;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ap->key_len = head.len;
    ddev_read(flash_handle, (char *)ap->key, head.len, addr+sizeof(head_param_t));
#endif
    return 0;
}

void app_demo_add_pkt_header(TV_HDR_PARAM_PTR param)
{
    HDR_PTR elem_tvhdr = (HDR_PTR)param->ptk_ptr;

    elem_tvhdr->id = (UINT8)param->frame_id;
    elem_tvhdr->is_eof = param->is_eof;
    elem_tvhdr->pkt_cnt = param->frame_len;
    elem_tvhdr->size = 0;
}
#endif  // CFG_USE_APP_DEMO_VIDEO_TRANSFER

